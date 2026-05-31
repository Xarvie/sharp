/*
 * cpp_sys_paths.c — System include path discovery for sharp-cpp.
 *
 * Implements platform-specific heuristics to find system headers,
 * mirroring clang's built-in discovery logic.
 */
#define _POSIX_C_SOURCE 200809L
#include "cpp.h"
#include "cpp_sys_paths.h"
#include "cpp_internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

/* Helper: compare two version strings like "10.0.26100.0" */
static int compare_version(const char *a, const char *b) {
    unsigned long va[4] = {0}, vb[4] = {0};
    sscanf(a, "%lu.%lu.%lu.%lu", &va[0], &va[1], &va[2], &va[3]);
    sscanf(b, "%lu.%lu.%lu.%lu", &vb[0], &vb[1], &vb[2], &vb[3]);
    for (int i = 0; i < 4; i++) {
        if (va[i] < vb[i]) return -1;
        if (va[i] > vb[i]) return 1;
    }
    return 0;
}

/* Helper: append a sys include path if it exists and is unique */
static void add_sys_path_if_valid(CppCtx *ctx, const char *path) {
    if (!path || !*path) return;

#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        return;
#else
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode))
        return;
#endif

    /* Avoid duplicates */
    for (size_t i = 0; i < ctx->sys_include_paths.len; i++) {
        if (strcmp(ctx->sys_include_paths.data[i], path) == 0)
            return;
    }

    strarr_push(&ctx->sys_include_paths, cpp_xstrdup(path));
}

/* Add a zig libc include path (e.g. "any-windows-any", "generic-mingw")
 * to ctx->sys_include_paths. */
static void add_zig_sub_path(CppCtx *ctx, const char *rel) {
    const char *zdir = cpp_find_zig_install_dir();
    if (!zdir) return;
    char path[MAX_PATH];
#ifdef _WIN32
    snprintf(path, sizeof path, "%s\\%s", zdir, rel);
#else
    snprintf(path, sizeof path, "%s/%s", zdir, rel);
#endif
    add_sys_path_if_valid(ctx, path);
}

static void add_zig_libc_path(CppCtx *ctx, const char *suffix) {
    char rel[MAX_PATH];
    snprintf(rel, sizeof rel, "lib/libc/include/%s", suffix);
    add_zig_sub_path(ctx, rel);
}

/* Add zig's MinGW headers (<zig>/lib/libc/mingw/include). */
static void add_zig_libc_mingw(CppCtx *ctx) {
    add_zig_sub_path(ctx, "lib/libc/mingw/include");
}

/* Add zig's builtin include (<zig>/lib/include) for compiler headers
 * like <mm_malloc.h>, <__stdarg_va_copy.h>, etc. */
static void add_zig_lib_include(CppCtx *ctx) {
    add_zig_sub_path(ctx, "lib/include");
}

/* ======================================================================== */
/*                         Windows Implementation                            */
/* ======================================================================== */

#ifdef _WIN32

/* Helper: strip trailing filename from a Windows path (like PathRemoveFileSpecA) */
static void remove_file_spec_a(char *path) {
    char *last_slash = strrchr(path, '\\');
    if (last_slash) *last_slash = '\0';
    else {
        char *last_fwd = strrchr(path, '/');
        if (last_fwd) *last_fwd = '\0';
    }
}

/* Helper: canonicalize a Windows path (like PathCanonicalizeA) */
static void canonicalize_path_a(char *out, const char *in) {
    size_t len = strlen(in);
    if (len == 0) { out[0] = '\0'; return; }

    const char *sep_chars = "\\/";
    /* Copy input */
    char tmp[512];
    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
    strncpy(tmp, in, len);
    tmp[len] = '\0';

    /* Normalize all separators to backslash, collapse runs */
    size_t r = 0, w = 0;
    int had_sep = 0;
    if (tmp[0] && tmp[1] == ':') {
        out[w++] = tmp[0];
        out[w++] = tmp[1];
        r = 2;
        had_sep = 0;
    }
    for (; tmp[r]; r++) {
        if (tmp[r] == '/' || tmp[r] == '\\') {
            if (!had_sep) out[w++] = '\\';
            had_sep = 1;
        } else {
            had_sep = 0;
            out[w++] = tmp[r];
        }
    }
    if (w > 1 && out[w-1] == '\\') w--;
    out[w] = '\0';
}

/* Step 1: Clang builtin headers */
static void detect_clang_builtin_paths(CppCtx *ctx) {
    char clang_exe[MAX_PATH];
    if (!SearchPathA(NULL, "clang.exe", NULL, sizeof(clang_exe), clang_exe, NULL))
        return;

    char clang_dir[MAX_PATH];
    strncpy(clang_dir, clang_exe, sizeof(clang_dir) - 1);
    clang_dir[sizeof(clang_dir) - 1] = '\0';
    remove_file_spec_a(clang_dir);

    char search_root[MAX_PATH];
    strncpy(search_root, clang_dir, sizeof(search_root) - 1);
    search_root[sizeof(search_root) - 1] = '\0';

    for (int level = 0; level < 6; level++) {
        char raw[MAX_PATH];
        char candidate[MAX_PATH];
        snprintf(raw, sizeof(raw), "%s\\lib\\clang", search_root);
        canonicalize_path_a(candidate, raw);

        DWORD attrs = GetFileAttributesA(candidate);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            WIN32_FIND_DATAA fd;
            char pattern[MAX_PATH];
            snprintf(pattern, sizeof(pattern), "%s\\*", candidate);
            HANDLE h = FindFirstFileA(pattern, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                do {
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
                        char inc_path[MAX_PATH];
                        snprintf(inc_path, sizeof(inc_path), "%s\\%s\\include", candidate, fd.cFileName);
                        add_sys_path_if_valid(ctx, inc_path);
                    }
                } while (FindNextFileA(h, &fd));
                FindClose(h);
            }
            return;
        }

        remove_file_spec_a(search_root);
    }
}

/* Step 2: MSVC headers — find via VS setup registry or VSINSTALLDIR env */
static void detect_msvc_paths(CppCtx *ctx) {
    char msvc_inc[MAX_PATH] = {0};

    /* Try VSINSTALLDIR environment variable first */
    const char *vs_install = getenv("VSINSTALLDIR");
    if (vs_install && vs_install[0]) {
        /* Strip trailing backslash if present */
        char vs_base[MAX_PATH];
        strncpy(vs_base, vs_install, sizeof(vs_base) - 1);
        vs_base[sizeof(vs_base) - 1] = '\0';
        size_t vs_len = strlen(vs_base);
        if (vs_len > 0 && (vs_base[vs_len - 1] == '\\' || vs_base[vs_len - 1] == '/'))
            vs_base[vs_len - 1] = '\0';

        /* Typical: C:\Program Files\Microsoft Visual Studio\2022\Community */
        /* MSVC headers are under VC\Tools\MSVC\<version>\include */
        char search_pattern[MAX_PATH];
        snprintf(search_pattern, sizeof(search_pattern),
                 "%s\\VC\\Tools\\MSVC", vs_base);
        DWORD attrs = GetFileAttributesA(search_pattern);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            /* Find latest version subdirectory */
            WIN32_FIND_DATAA fd;
            char pattern[MAX_PATH];
            snprintf(pattern, sizeof(pattern), "%s\\*", search_pattern);
            HANDLE h = FindFirstFileA(pattern, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                char best_version[64] = {0};
                do {
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                        fd.cFileName[0] != '.' && fd.cFileName[0] != '$') {
                        /* Prefer numerically highest version */
                        if (compare_version(fd.cFileName, best_version) > 0) {
                            strncpy(best_version, fd.cFileName, sizeof(best_version) - 1);
                            best_version[sizeof(best_version) - 1] = '\0';
                        }
                    }
                } while (FindNextFileA(h, &fd));
                FindClose(h);

                if (best_version[0]) {
                    snprintf(msvc_inc, sizeof(msvc_inc),
                             "%s\\VC\\Tools\\MSVC\\%s\\include",
                             vs_install, best_version);
                    add_sys_path_if_valid(ctx, msvc_inc);
                    return;
                }
            }
        }
    }

    /* Fallback: scan common VS installation paths */
    const char *common_bases[] = {
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional",
        "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional",
        "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise",
    };
    for (size_t bi = 0; bi < sizeof(common_bases) / sizeof(common_bases[0]); bi++) {
        char search_pattern[MAX_PATH];
        snprintf(search_pattern, sizeof(search_pattern),
                 "%s\\VC\\Tools\\MSVC", common_bases[bi]);
        DWORD attrs = GetFileAttributesA(search_pattern);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            WIN32_FIND_DATAA fd;
            char pattern[MAX_PATH];
            snprintf(pattern, sizeof(pattern), "%s\\*", search_pattern);
            HANDLE h = FindFirstFileA(pattern, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                char best_version[64] = {0};
                do {
                    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                        fd.cFileName[0] != '.' && fd.cFileName[0] != '$') {
                        if (compare_version(fd.cFileName, best_version) > 0) {
                            strncpy(best_version, fd.cFileName, sizeof(best_version) - 1);
                            best_version[sizeof(best_version) - 1] = '\0';
                        }
                    }
                } while (FindNextFileA(h, &fd));
                FindClose(h);

                if (best_version[0]) {
                    snprintf(msvc_inc, sizeof(msvc_inc),
                             "%s\\VC\\Tools\\MSVC\\%s\\include",
                             common_bases[bi], best_version);
                    add_sys_path_if_valid(ctx, msvc_inc);
                    return;
                }
            }
        }
    }
}

/* Step 3: Windows SDK headers via registry */
static BOOL read_reg_str(HKEY root, const char *subkey, const char *value, char *out, DWORD out_size) {
    HKEY key;
    if (RegOpenKeyExA(root, subkey, 0, KEY_READ | KEY_WOW64_32KEY, &key) != ERROR_SUCCESS)
        return FALSE;
    DWORD type = 0, size = out_size;
    BOOL ok = (RegQueryValueExA(key, value, NULL, &type, (LPBYTE)out, &size) == ERROR_SUCCESS && type == REG_SZ);
    RegCloseKey(key);
    return ok;
}

static void detect_windows_sdk_paths(CppCtx *ctx) {
    char kits_root[MAX_PATH] = {0};
    char best_version[64] = {0};

    if (read_reg_str(HKEY_LOCAL_MACHINE,
                     "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0",
                     "InstallationFolder", kits_root, sizeof(kits_root))) {
        /* Strip trailing backslash */
        size_t kr_len = strlen(kits_root);
        if (kr_len > 0 && (kits_root[kr_len - 1] == '\\' || kits_root[kr_len - 1] == '/'))
            kits_root[kr_len - 1] = '\0';
        read_reg_str(HKEY_LOCAL_MACHINE,
                     "SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0",
                     "ProductVersion", best_version, sizeof(best_version));
    }

    if (!best_version[0]) {
        const char *env_sdk_dir = getenv("WindowsSdkDir");
        const char *env_sdk_ver = getenv("WindowsSDKVersion");
        if (env_sdk_dir && env_sdk_ver) {
            strncpy(kits_root, env_sdk_dir, sizeof(kits_root) - 1);
            kits_root[sizeof(kits_root) - 1] = '\0';
            size_t len = strlen(kits_root);
            if (len > 0 && (kits_root[len - 1] == '\\' || kits_root[len - 1] == '/'))
                kits_root[len - 1] = '\0';
            strncpy(best_version, env_sdk_ver, sizeof(best_version) - 1);
            best_version[sizeof(best_version) - 1] = '\0';
        }
    }

    /* Always scan the filesystem to find the exact directory version name.
     * Registry ProductVersion may be "10.0.26100" but the directory is
     * "10.0.26100.0" — we need the actual directory name.
     * Use the filesystem version if it's >= the registry version. */
    const char *scan_base = "C:\\Program Files (x86)\\Windows Kits\\10\\Include";
    DWORD attrs = GetFileAttributesA(scan_base);
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        WIN32_FIND_DATAA fd;
        char pattern[MAX_PATH];
        snprintf(pattern, sizeof(pattern), "%s\\*", scan_base);
        HANDLE h = FindFirstFileA(pattern, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
                    fd.cFileName[0] != '.' && strncmp(fd.cFileName, "10.0.", 5) == 0) {
                    /* If no version yet, or this is newer, use it */
                    if (!best_version[0] || compare_version(fd.cFileName, best_version) >= 0) {
                        strncpy(best_version, fd.cFileName, sizeof(best_version) - 1);
                        best_version[sizeof(best_version) - 1] = '\0';
                        strncpy(kits_root, "C:\\Program Files (x86)\\Windows Kits\\10", sizeof(kits_root) - 1);
                        kits_root[sizeof(kits_root) - 1] = '\0';
                    }
                }
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
    }

    if (kits_root[0] && best_version[0]) {
        char sdk_inc[MAX_PATH];
        const char *subdirs[] = {"ucrt", "um", "shared", "winrt", "cppwinrt"};
        for (size_t i = 0; i < sizeof(subdirs) / sizeof(subdirs[0]); i++) {
            snprintf(sdk_inc, sizeof(sdk_inc), "%s\\Include\\%s\\%s",
                     kits_root, best_version, subdirs[i]);
            add_sys_path_if_valid(ctx, sdk_inc);
        }
    }
}

/* Helper: run a command and capture all output (stdout + stderr merged) */
static char *run_command_and_capture(const char *cmd) {
#ifdef _WIN32
    /* On Windows, use a temp file to capture both stdout and stderr */
    char tmpfile[MAX_PATH];
    GetTempPathA(sizeof(tmpfile), tmpfile);
    char filename[MAX_PATH];
    GetTempFileNameA(tmpfile, "shp", 0, filename);
    
    /* Build command: cmd > tmpfile 2>&1 */
    char full_cmd[1024];
    snprintf(full_cmd, sizeof(full_cmd), "%s > \"%s\" 2>&1", cmd, filename);
    
    FILE *fp = popen(full_cmd, "r");
    if (fp) pclose(fp);
    
    /* Read the temp file */
    fp = fopen(filename, "rb");
    DeleteFileA(filename);
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buf = cpp_xmalloc(len + 1);
    
    fread(buf, 1, len, fp);
    buf[len] = '\0';
    fclose(fp);
    return buf;
#else
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;
    
    size_t cap = 8192, buf_len = 0;
    char *buf = cpp_xmalloc(cap);
    size_t nr;
    while ((nr = fread(buf + buf_len, 1, cap - buf_len - 1, fp)) > 0) {
        buf_len += nr;
        if (buf_len + 1 >= cap) {
            cap *= 2;
            buf = cpp_xrealloc(buf, cap);
        }
    }
    buf[buf_len] = '\0';
    pclose(fp);
    return buf;
#endif
}

/* Helper: find an executable in PATH or common locations */
static BOOL find_executable(const char *name, char *out, size_t out_size) {
    /* First try PATH */
    if (SearchPathA(NULL, name, NULL, (DWORD)out_size, out, NULL))
        return TRUE;
    
    /* Common MinGW locations */
    const char *common_paths[] = {
        "C:\\mingw64\\bin",
        "C:\\mingw\\bin",
        "C:\\msys64\\mingw64\\bin",
        "C:\\msys64\\mingw32\\bin",
        "C:\\Program Files\\mingw-w64\\bin",
    };
    for (size_t i = 0; i < sizeof(common_paths) / sizeof(common_paths[0]); i++) {
        char candidate[MAX_PATH];
        snprintf(candidate, sizeof(candidate), "%s\\%s", common_paths[i], name);
        DWORD attrs = GetFileAttributesA(candidate);
        if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
            strncpy(out, candidate, out_size - 1);
            out[out_size - 1] = '\0';
            return TRUE;
        }
    }
    return FALSE;
}

/* Step 4: MinGW GCC headers — derive paths from gcc.exe location */
static void detect_mingw_gcc_paths(CppCtx *ctx) {
    char gcc_exe[MAX_PATH];
    if (!find_executable("gcc.exe", gcc_exe, sizeof(gcc_exe)))
        return;
    
    /* Derive GCC prefix from executable location.
     * gcc.exe is typically at: <prefix>/bin/gcc.exe
     * Headers are at:
     *   <prefix>/lib/gcc/<target>/<version>/include
     *   <prefix>/<target>/include
     *   <prefix>/include
     */
    char gcc_dir[MAX_PATH];
    strncpy(gcc_dir, gcc_exe, sizeof(gcc_dir) - 1);
    gcc_dir[sizeof(gcc_dir) - 1] = '\0';
    remove_file_spec_a(gcc_dir);  /* remove gcc.exe → get bin/ */
    remove_file_spec_a(gcc_dir);  /* remove bin/ → get <prefix>/ */
    
    /* Scan <prefix>/lib/gcc/<target>/<version>/include */
    char lib_gcc[MAX_PATH];
    snprintf(lib_gcc, sizeof(lib_gcc), "%s\\lib\\gcc", gcc_dir);
    DWORD attrs = GetFileAttributesA(lib_gcc);
    if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        /* Scan for target triple dirs */
        WIN32_FIND_DATAA fd1;
        char pattern1[MAX_PATH];
        snprintf(pattern1, sizeof(pattern1), "%s\\*", lib_gcc);
        HANDLE h1 = FindFirstFileA(pattern1, &fd1);
        if (h1 != INVALID_HANDLE_VALUE) {
            do {
                if (fd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd1.cFileName[0] != '.') {
                    /* Found <target>, now scan for version */
                    char target_dir[MAX_PATH];
                    snprintf(target_dir, sizeof(target_dir), "%s\\%s", lib_gcc, fd1.cFileName);
                    
                    WIN32_FIND_DATAA fd2;
                    char pattern2[MAX_PATH];
                    snprintf(pattern2, sizeof(pattern2), "%s\\*", target_dir);
                    HANDLE h2 = FindFirstFileA(pattern2, &fd2);
                    if (h2 != INVALID_HANDLE_VALUE) {
                        do {
                            if (fd2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd2.cFileName[0] != '.') {
                                char inc[MAX_PATH];
                                snprintf(inc, sizeof(inc), "%s\\%s\\include", target_dir, fd2.cFileName);
                                add_sys_path_if_valid(ctx, inc);
                                snprintf(inc, sizeof(inc), "%s\\%s\\include-fixed", target_dir, fd2.cFileName);
                                add_sys_path_if_valid(ctx, inc);
                            }
                        } while (FindNextFileA(h2, &fd2));
                        FindClose(h2);
                    }
                }
            } while (FindNextFileA(h1, &fd1));
            FindClose(h1);
        }
    }
    
    /* Add <prefix>/<target>/include — scan for target triple */
    char prefix_include[MAX_PATH];
    snprintf(prefix_include, sizeof(prefix_include), "%s\\*", gcc_dir);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(prefix_include, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && fd.cFileName[0] != '.') {
                /* Check if this looks like a target triple (contains -) */
                if (strchr(fd.cFileName, '-')) {
                    char inc[MAX_PATH];
                    snprintf(inc, sizeof(inc), "%s\\%s\\include", gcc_dir, fd.cFileName);
                    add_sys_path_if_valid(ctx, inc);
                }
            }
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    
    /* Add <prefix>/include */
    char top_inc[MAX_PATH];
    snprintf(top_inc, sizeof(top_inc), "%s\\include", gcc_dir);
    add_sys_path_if_valid(ctx, top_inc);
}

void cpp_detect_sys_include_paths(CppCtx *ctx) {
    /* Order matters: clang builtin → MSVC → MinGW GCC → Windows SDK.
     * This matches clang's own search order on Windows. */
    detect_clang_builtin_paths(ctx);
    detect_msvc_paths(ctx);
    detect_mingw_gcc_paths(ctx);
    detect_windows_sdk_paths(ctx);

    /* Also add zig's builtin include for compiler-provided headers
     * like <mm_malloc.h>, <__stdarg_va_copy.h>, etc. */
    add_zig_lib_include(ctx);
    /* Also add zig's libc headers for native Windows target. */
    add_zig_libc_path(ctx, "any-windows-any");
    add_zig_libc_mingw(ctx);
}

/* ======================================================================== */
/*                         Linux Implementation                              */
/* ======================================================================== */

#elif defined(__linux__)

static void detect_linux_sys_paths(CppCtx *ctx) {
    add_sys_path_if_valid(ctx, "/usr/include");
    add_sys_path_if_valid(ctx, "/usr/local/include");

    FILE *fp = popen("gcc -print-file-name=include 2>/dev/null", "r");
    if (fp) {
        char buf[1024];
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = '\0';
            if (buf[0])
                add_sys_path_if_valid(ctx, buf);
        }
        pclose(fp);
    }
}

void cpp_detect_sys_include_paths(CppCtx *ctx) {
    detect_linux_sys_paths(ctx);
}

/* ======================================================================== */
/*                         macOS Implementation                              */
/* ======================================================================== */

#elif defined(__APPLE__)

static void detect_macos_sys_paths(CppCtx *ctx) {
    FILE *fp = popen("xcrun --show-sdk-path 2>/dev/null", "r");
    if (fp) {
        char buf[1024];
        if (fgets(buf, sizeof(buf), fp)) {
            buf[strcspn(buf, "\n")] = '\0';
            if (buf[0]) {
                char inc[1024];
                snprintf(inc, sizeof(inc), "%s/usr/include", buf);
                add_sys_path_if_valid(ctx, inc);
            }
        }
        pclose(fp);
    }

    add_sys_path_if_valid(ctx, "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include");
    add_sys_path_if_valid(ctx, "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include");
    add_sys_path_if_valid(ctx, "/usr/local/include");
    add_sys_path_if_valid(ctx, "/usr/include");
}

void cpp_detect_sys_include_paths(CppCtx *ctx) {
    detect_macos_sys_paths(ctx);
}

/* ======================================================================== */
/*                         Fallback                                          */
/* ======================================================================== */

#else

void cpp_detect_sys_include_paths(CppCtx *ctx) {
    (void)ctx;
}

#endif

/* ======================================================================== */
/*  Target-aware system include path detection (host-independent)           */
/*  Used for cross-compilation: discovers zig libc headers for the target   */
/*  OS regardless of which host we're running on.                           */
/* ======================================================================== */

/* Extract OS component from a zig target triple.
 * Examples: "x86_64-macos"       → "macos"
 *           "x86_64-linux-gnu"   → "linux"
 *           "x86_64-windows"     → "windows"
 *           "aarch64-ios-simulator" → "ios"
 */
static const char *extract_target_os(const char *target) {
    if (!target || !target[0]) return NULL;

    /* Target triple format: arch[-vendor]-os[-abi][-environment]
     * Split by '-' and find the OS component (typically 2nd or 3rd part). */
    const char *parts[4] = {0};
    int nparts = 0;
    const char *p = target;

    while (*p && nparts < 4) {
        parts[nparts++] = p;
        while (*p && *p != '-') p++;
        if (*p == '-') p++;
    }

    if (nparts < 2) return NULL;

    /* Heuristic: the OS is the last "known" component.
     * Known OS names in zig triples: */
    static const char *known_os[] = {
        "windows", "linux", "macos", "freebsd", "netbsd", "openbsd",
        "ios", "tvos", "watchos", "visionos",
        "wasi", "solaris", "haiku", "dragonfly", "none",
    };
    int nknown = (int)(sizeof known_os / sizeof known_os[0]);

    /* Check from last to second part (first is always arch) */
    for (int i = nparts - 1; i >= 1; i--) {
        for (int j = 0; j < nknown; j++) {
            size_t len = strlen(known_os[j]);
            if (strncmp(parts[i], known_os[j], len) == 0 &&
                (parts[i][len] == '\0' || parts[i][len] == '-')) {
                return known_os[j];
            }
        }
    }
    return NULL;
}

/* Map target OS name to zig libc include directory suffix. */
static const char *zig_include_dir_for_os(const char *os) {
    if (!os) return NULL;

    if (strcmp(os, "windows") == 0) return "any-windows-any";
    if (strcmp(os, "linux") == 0)   return "any-linux-any";
    if (strcmp(os, "macos") == 0)   return "any-darwin-any";
    if (strcmp(os, "ios") == 0)     return "any-darwin-any";
    if (strcmp(os, "tvos") == 0)    return "any-darwin-any";
    if (strcmp(os, "watchos") == 0) return "any-darwin-any";
    if (strcmp(os, "visionos") == 0) return "any-darwin-any";
    if (strcmp(os, "freebsd") == 0) return "generic-freebsd";
    if (strcmp(os, "netbsd") == 0)  return "generic-netbsd";
    if (strcmp(os, "openbsd") == 0) return "generic-openbsd";
    if (strcmp(os, "wasi") == 0)    return "wasm-wasi-musl";

    return NULL;
}

void cpp_detect_target_sys_paths(CppCtx *ctx, const char *target) {
    const char *target_os = extract_target_os(target);
    if (!target_os) return;

    const char *inc_suffix = zig_include_dir_for_os(target_os);
    if (!inc_suffix) return;

    /* 1. Cross-compile mode: find zig installation and add target-specific
     *    libc include directories. This works on any host for any target. */
    const char *zig_dir = cpp_find_zig_install_dir();
    if (zig_dir) {
        char path[MAX_PATH];

        /* Always add zig's builtin include for compiler-provided headers
         * like <mm_malloc.h>, <__stdarg_va_copy.h>, etc. */
#ifdef _WIN32
        snprintf(path, sizeof path, "%s\\lib\\include", zig_dir);
#else
        snprintf(path, sizeof path, "%s/lib/include", zig_dir);
#endif
        add_sys_path_if_valid(ctx, path);

        /* Add target-specific zig libc headers:
         *   <zig>/lib/libc/include/<platform-dir>/ */
#ifdef _WIN32
        snprintf(path, sizeof path, "%s\\lib\\libc\\include\\%s",
                 zig_dir, inc_suffix);
#else
        snprintf(path, sizeof path, "%s/lib/libc/include/%s",
                 zig_dir, inc_suffix);
#endif
        add_sys_path_if_valid(ctx, path);

        /* For Linux targets, also add libc-internal headers (like
         * <bits/...>, <asm/...> which live in generic-glibc or generic-musl) */
        if (strcmp(target_os, "linux") == 0) {
#ifdef _WIN32
            snprintf(path, sizeof path, "%s\\lib\\libc\\include\\generic-glibc",
                     zig_dir);
#else
            snprintf(path, sizeof path, "%s/lib/libc/include/generic-glibc",
                     zig_dir);
#endif
            add_sys_path_if_valid(ctx, path);
        }

        /* For Darwin targets, also add musl headers (provides stdbool.h etc.) */
        if (strcmp(target_os, "macos") == 0 ||
            strcmp(target_os, "ios") == 0 ||
            strcmp(target_os, "tvos") == 0 ||
            strcmp(target_os, "watchos") == 0) {
#ifdef _WIN32
            snprintf(path, sizeof path, "%s\\lib\\libc\\include\\generic-musl",
                     zig_dir);
#else
            snprintf(path, sizeof path, "%s/lib/libc/include/generic-musl",
                     zig_dir);
#endif
            add_sys_path_if_valid(ctx, path);
        }

        /* For Windows targets, add MinGW-specific headers */
        if (strcmp(target_os, "windows") == 0) {
#ifdef _WIN32
            snprintf(path, sizeof path, "%s\\lib\\libc\\mingw\\include",
                     zig_dir);
#else
            snprintf(path, sizeof path, "%s/lib/libc/mingw/include",
                     zig_dir);
#endif
            add_sys_path_if_valid(ctx, path);
        }
    }

    /* 2. Native Windows SDK / MSVC paths when targeting Windows */
    if (strcmp(target_os, "windows") == 0) {
#ifdef _WIN32
        detect_msvc_paths(ctx);
        detect_windows_sdk_paths(ctx);
#endif
    }
}

/* ======================================================================== */
/*  Hardcoded zig include paths (queried from zig cc -E -v)                  */
/*  These are the exact paths zig reports for each target.                   */
/*  Always prefixed with <zig_dir> so they work regardless of install loc.   */
/* ======================================================================== */

static void add_zig_base_include(CppCtx *ctx, const char *zig_dir) {
    char p[MAX_PATH];
#ifdef _WIN32
    snprintf(p, sizeof p, "%s\\lib\\include", zig_dir);
#else
    snprintf(p, sizeof p, "%s/lib/include", zig_dir);
#endif
    add_sys_path_if_valid(ctx, p);
}

static void add_zig_path(CppCtx *ctx, const char *zig_dir, const char *rel) {
    char p[MAX_PATH];
#ifdef _WIN32
    snprintf(p, sizeof p, "%s\\%s", zig_dir, rel);
#else
    snprintf(p, sizeof p, "%s/%s", zig_dir, rel);
#endif
    add_sys_path_if_valid(ctx, p);
}

/**
 * cpp_detect_zig_sys_paths — Add system include directories for the
 * given target using zig installation path heuristics.
 *
 * Queries the zig installation directory layout for target-specific
 * libc headers.  Falls back to host system include detection if zig
 * is not installed or the target is unknown.
 */
void cpp_detect_zig_sys_paths(CppCtx *ctx, const char *target) {
#ifdef _WIN32
    const char *zig_dir = cpp_find_zig_install_dir();
    if (!zig_dir) {
        cpp_detect_sys_include_paths(ctx);
        return;
    }

    /* Always add compiler builtin headers first (mm_malloc.h, etc.) */
    char p[MAX_PATH];
    snprintf(p, sizeof p, "%s\\lib\\include", zig_dir);
    add_sys_path_if_valid(ctx, p);

    const char *target_os = extract_target_os(target);

    if (!target_os) {
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\any-windows-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
        snprintf(p, sizeof p, "%s\\lib\\libc\\mingw\\include", zig_dir);
        add_sys_path_if_valid(ctx, p);
        return;
    }

    if (strcmp(target_os, "windows") == 0) {
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\any-windows-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
        snprintf(p, sizeof p, "%s\\lib\\libc\\mingw\\include", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "linux") == 0) {
        int is_x86 = target && (strncmp(target, "x86_64", 6) == 0 ||
                                strncmp(target, "x86-", 4) == 0);
        int is_aarch64 = target && strncmp(target, "aarch64", 7) == 0;
        int is_musl = target && strstr(target, "musl");

        if (is_x86 && is_musl) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86_64-linux-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_x86) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_aarch64 && is_musl) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\aarch64-linux-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\aarch64-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_aarch64) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\aarch64-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\aarch64-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\any-linux-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "macos") == 0) {
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\any-darwin-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "freebsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86_64-freebsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-freebsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "netbsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86_64-netbsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-netbsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "openbsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s\\lib\\libc\\include\\x86_64-openbsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-openbsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "wasi") == 0) {
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\wasm-wasi-musl", zig_dir);
        add_sys_path_if_valid(ctx, p);
        snprintf(p, sizeof p, "%s\\lib\\libc\\include\\generic-musl", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else {
        cpp_detect_sys_include_paths(ctx);
    }
#else
    /* Linux/macOS: find zig dir and construct Unix-style paths */
    const char *zig_dir = cpp_find_zig_install_dir();
    if (!zig_dir) {
        cpp_detect_sys_include_paths(ctx);
        return;
    }

    char p[MAX_PATH];
    snprintf(p, sizeof p, "%s/lib/include", zig_dir);
    add_sys_path_if_valid(ctx, p);

    const char *target_os = extract_target_os(target);

    if (!target_os) {
        cpp_detect_sys_include_paths(ctx);
        return;
    }

    if (strcmp(target_os, "linux") == 0) {
        int is_x86 = target && (strncmp(target, "x86_64", 6) == 0 ||
                                strncmp(target, "x86-", 4) == 0);
        int is_aarch64 = target && strncmp(target, "aarch64", 7) == 0;
        int is_musl = target && strstr(target, "musl");

        if (is_x86 && is_musl) {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86_64-linux-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/generic-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_x86) {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_aarch64 && is_musl) {
            snprintf(p, sizeof p, "%s/lib/libc/include/aarch64-linux-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/generic-musl", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/aarch64-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else if (is_aarch64) {
            snprintf(p, sizeof p, "%s/lib/libc/include/aarch64-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/aarch64-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        } else {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86-linux-gnu", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/generic-glibc", zig_dir);
            add_sys_path_if_valid(ctx, p);
            snprintf(p, sizeof p, "%s/lib/libc/include/x86-linux-any", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s/lib/libc/include/any-linux-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "macos") == 0 ||
               strcmp(target_os, "ios") == 0 ||
               strcmp(target_os, "tvos") == 0 ||
               strcmp(target_os, "watchos") == 0) {
        /* Platform-native SDK paths first (macOS SDK via xcrun).
         * These take priority over zig's libc headers to avoid
         * redefinition conflicts (e.g. _OSSwapInt16). */
        cpp_detect_sys_include_paths(ctx);
        snprintf(p, sizeof p, "%s/lib/libc/include/any-darwin-any", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "freebsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86_64-freebsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s/lib/libc/include/generic-freebsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "netbsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86_64-netbsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s/lib/libc/include/generic-netbsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "openbsd") == 0) {
        int is_x86_64 = target && strncmp(target, "x86_64", 6) == 0;
        if (is_x86_64) {
            snprintf(p, sizeof p, "%s/lib/libc/include/x86_64-openbsd-none", zig_dir);
            add_sys_path_if_valid(ctx, p);
        }
        snprintf(p, sizeof p, "%s/lib/libc/include/generic-openbsd", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else if (strcmp(target_os, "wasi") == 0) {
        snprintf(p, sizeof p, "%s/lib/libc/include/wasm-wasi-musl", zig_dir);
        add_sys_path_if_valid(ctx, p);
        snprintf(p, sizeof p, "%s/lib/libc/include/generic-musl", zig_dir);
        add_sys_path_if_valid(ctx, p);
    } else {
        cpp_detect_sys_include_paths(ctx);
    }
#endif
}

size_t cpp_sys_include_count(const CppCtx *ctx) {
    return ctx ? ctx->sys_include_paths.len : 0;
}

const char *cpp_sys_include(const CppCtx *ctx, size_t idx) {
    if (!ctx || idx >= ctx->sys_include_paths.len) return NULL;
    return ctx->sys_include_paths.data[idx];
}

/* -----------------------------------------------------------------
 * cpp_detect_zig_sys_paths_from_zig — Discover system include paths
 * by directly parsing `zig cc -E -v` output.
 *
 * This guarantees 100% consistency with zig cc's own search paths.
 * ----------------------------------------------------------------- */

/* Internal: capture stderr output from a command (zig -v goes to stderr) */
static char *
capture_zig_cc_verbose(const char *zig_exe, const char *target)
{
    char err_path[512];
    char cmd[2048];

#ifdef _WIN32
    char out_path[512];
    const char *tmp = getenv("TEMP");
    if (!tmp) tmp = ".";
    snprintf(out_path, sizeof(out_path), "%s\\zig_verbose_out_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId());
    snprintf(err_path, sizeof(err_path), "%s\\zig_verbose_err_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId());
    const char *empty_file = out_path;
    FILE *efp = fopen(empty_file, "w");
    if (!efp) return NULL;
    fclose(efp);
#else
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    snprintf(err_path, sizeof(err_path), "%s/zig_verbose_err_%d.tmp",
             tmp, (int)getpid());
    const char *empty_file = "/dev/null";
#endif

    /* zig cc -E -v outputs verbose info to stderr */
    snprintf(cmd, sizeof(cmd),
             "%s cc -E -v -std=c11%s%s%s -x c %s >\"%s\" 2>\"%s\"",
             zig_exe,
             target ? " -target " : "",
             target ? target : "",
             /* -fno-blocks for darwin */
             (target && (strstr(target, "macos") || strstr(target, "ios"))) ? " -fno-blocks" : "",
             empty_file,
#ifdef _WIN32
             out_path,
#else
             "/dev/null",
#endif
             err_path);

    int exit_code = system(cmd);

#ifdef _WIN32
    DeleteFileA(empty_file);
    DeleteFileA(out_path);
#endif

    if (exit_code != 0) {
        remove(err_path);
        return NULL;
    }

    /* Read stderr output */
    FILE *f = fopen(err_path, "r");
    if (!f) {
        remove(err_path);
        return NULL;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        fclose(f);
        remove(err_path);
        return NULL;
    }

    char *buf = cpp_xmalloc((size_t)fsize + 1);

    size_t nread = fread(buf, 1, (size_t)fsize, f);
    buf[nread] = '\0';
    fclose(f);
    remove(err_path);

    return buf;
}

/**
 * cpp_detect_zig_sys_paths_from_zig — Parse `zig cc -E -v` output to
 * discover system include directories.  This guarantees that sharpc
 * uses exactly the same include paths as zig cc.
 */
void cpp_detect_zig_sys_paths_from_zig(CppCtx *ctx, const char *target)
{
    const char *zig_exe = cpp_find_zig_exe();
    if (!zig_exe) {
        /* Fallback to host-native detection */
        cpp_detect_sys_include_paths(ctx);
        return;
    }

    char *verbose = capture_zig_cc_verbose(zig_exe, target);
    if (!verbose) {
        cpp_detect_sys_include_paths(ctx);
        return;
    }

    /* Parse the output for include paths.
     * Format:
     *   #include <...> search starts here:
     *    /path/to/include1
     *    /path/to/include2
     *   End of search list.
     */
    const char *start_marker = "#include <...> search starts here:";
    const char *end_marker = "End of search list.";

    const char *start = strstr(verbose, start_marker);
    if (!start) {
        free(verbose);
        cpp_detect_sys_include_paths(ctx);
        return;
    }
    start += strlen(start_marker);
    const char *end = strstr(start, end_marker);
    if (!end) end = verbose + strlen(verbose);

    /* Parse each line between start and end */
    const char *p = start;
    while (p < end) {
        /* Find next newline or end */
        const char *line_end = p;
        while (line_end < end && *line_end != '\n' && *line_end != '\r') line_end++;
        
        /* Extract line content */
        size_t line_len = (size_t)(line_end - p);
        if (line_len == 0) { p++; continue; }
        
        /* Skip leading spaces */
        const char *path_start = p;
        while (path_start < line_end && (*path_start == ' ' || *path_start == '\t')) path_start++;
        if (path_start >= line_end) { p = line_end + 1; continue; }
        
        size_t path_len = (size_t)(line_end - path_start);
        char path_buf[MAX_PATH];
        if (path_len >= sizeof(path_buf)) path_len = sizeof(path_buf) - 1;
        memcpy(path_buf, path_start, path_len);
        path_buf[path_len] = '\0';

        /* Resolve relative paths against current working directory.
         * zig cc -E -v outputs paths relative to the cwd when run. */
        if (path_buf[0] != '/') {
            char cwd[MAX_PATH];
            if (getcwd(cwd, sizeof(cwd))) {
                char full_path[MAX_PATH];
                snprintf(full_path, sizeof(full_path), "%s/%s", cwd, path_buf);
                add_sys_path_if_valid(ctx, full_path);
            }
        } else {
            add_sys_path_if_valid(ctx, path_buf);
        }
        
        /* Move to next line */
        p = line_end;
        while (p < end && (*p == '\n' || *p == '\r')) p++;
    }

    free(verbose);
}
