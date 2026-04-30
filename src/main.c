/* main.c - driver for the Sharp compiler.
 *
 * Usage:
 *   sharpc <input.sp>                  # writes <input.c> + <input.exe>
 *   sharpc <input.sp> -o <out.exe>     # compile to executable
 *   sharpc <input.sp> -o <out.c>       # only emit C (if .c extension)
 *   sharpc <input.sp> -o <out.sp>      # auto-detect: emit C + binary
 *   sharpc <input.sp> -                # write C to stdout
 *   sharpc <input.sp> -dump-hir        # print CFG to stderr, then generate C
 *   sharpc <input.sp> -no-link         # skip binary generation
 */
#include "sharp.h"
#include "hir.h"
#include "cpp.h"

#ifdef _WIN32
  #include <windows.h>
  #include <process.h>
#else
  #include <unistd.h>
#endif

/* ========================================================================
 * Target triple system (inspired by Zig: arch-os-abi)
 * ======================================================================== */

typedef enum {
    ARCH_X86_64, ARCH_X86, ARCH_AARCH64, ARCH_ARM, ARCH_WASM32, ARCH_RISCV64,
} TargetArch;

typedef enum {
    OS_WINDOWS, OS_LINUX, OS_MACOS, OS_NONE, OS_WASI,
} TargetOS;

typedef enum {
    ABI_MSVC, ABI_GNU, ABI_GNUILP32, ABI_NONE,
} TargetABI;

typedef struct {
    TargetArch arch;
    TargetOS   os;
    TargetABI  abi;
} TargetTriple;

static TargetTriple target_default(void) {
    TargetTriple t = {0};
#ifdef _WIN32
    t.arch = ARCH_X86_64; t.os = OS_WINDOWS; t.abi = ABI_MSVC;
#elif __linux__
    t.arch = ARCH_X86_64; t.os = OS_LINUX;   t.abi = ABI_GNU;
#elif __APPLE__
    t.arch = ARCH_X86_64; t.os = OS_MACOS;   t.abi = ABI_GNU;
#else
    t.arch = ARCH_X86_64; t.os = OS_NONE;    t.abi = ABI_NONE;
#endif
    return t;
}

static bool parse_target_triple(const char* str, TargetTriple* out) {
    char buf[128]; strncpy(buf, str, sizeof(buf) - 1); buf[sizeof(buf) - 1] = '\0';
    char* p = buf; char* parts[4] = {0}; int np = 0;
    while (p && np < 4) { parts[np++] = p; p = strchr(p, '-'); if (p) *p++ = '\0'; }
    if (np < 2) return false;
    if (strcmp(parts[0], "x86_64") == 0 || strcmp(parts[0], "amd64") == 0) out->arch = ARCH_X86_64;
    else if (strcmp(parts[0], "x86") == 0 || strcmp(parts[0], "i386") == 0 || strcmp(parts[0], "i686") == 0) out->arch = ARCH_X86;
    else if (strcmp(parts[0], "aarch64") == 0 || strcmp(parts[0], "arm64") == 0) out->arch = ARCH_AARCH64;
    else if (strncmp(parts[0], "arm", 3) == 0) out->arch = ARCH_ARM;
    else if (strcmp(parts[0], "wasm32") == 0) out->arch = ARCH_WASM32;
    else if (strcmp(parts[0], "riscv64") == 0) out->arch = ARCH_RISCV64;
    else return false;
    if (strcmp(parts[1], "windows") == 0 || strcmp(parts[1], "win32") == 0) out->os = OS_WINDOWS;
    else if (strcmp(parts[1], "linux") == 0) out->os = OS_LINUX;
    else if (strcmp(parts[1], "macos") == 0 || strcmp(parts[1], "darwin") == 0) out->os = OS_MACOS;
    else if (strcmp(parts[1], "none") == 0 || strcmp(parts[1], "freestanding") == 0) out->os = OS_NONE;
    else if (strcmp(parts[1], "wasi") == 0) out->os = OS_WASI;
    else return false;
    out->abi = ABI_NONE;
    if (np >= 3) {
        if (strcmp(parts[2], "msvc") == 0) out->abi = ABI_MSVC;
        else if (strcmp(parts[2], "gnu") == 0) out->abi = ABI_GNU;
        else if (strcmp(parts[2], "gnux32") == 0) out->abi = ABI_GNUILP32;
    } else {
        switch (out->os) {
        case OS_WINDOWS: out->abi = ABI_MSVC; break;
        case OS_LINUX:   out->abi = ABI_GNU;  break;
        case OS_MACOS:   out->abi = ABI_GNU;  break;
        default:         out->abi = ABI_NONE; break;
        }
    }
    return true;
}

static void apply_target_macros(CppCtx* cpp, const TargetTriple* target) {
    switch (target->arch) {
    case ARCH_X86_64:
        cpp_define(cpp, "_M_X64",    "100"); cpp_define(cpp, "_M_AMD64", "100");
        cpp_define(cpp, "__x86_64__","1");
        break;
    case ARCH_X86:
        cpp_define(cpp, "_M_IX86",   "600"); cpp_define(cpp, "__i386__", "1");
        break;
    case ARCH_AARCH64:
        cpp_define(cpp, "_M_ARM64",  "1");   cpp_define(cpp, "__aarch64__", "1");
        break;
    case ARCH_ARM:
        cpp_define(cpp, "_M_ARM",    "7");   break;
    case ARCH_WASM32:
        cpp_define(cpp, "__wasm__",  "1");   cpp_define(cpp, "__wasm32__", "1");
        break;
    case ARCH_RISCV64:
        cpp_define(cpp, "__riscv",   "1");   cpp_define(cpp, "__riscv_xlen", "64");
        break;
    }
    switch (target->os) {
    case OS_WINDOWS:
        cpp_define(cpp, "_WIN32",    "1");
        if (target->arch == ARCH_X86_64 || target->arch == ARCH_AARCH64)
            cpp_define(cpp, "_WIN64","1");
        break;
    case OS_LINUX:
        cpp_define(cpp, "__linux__", "1"); cpp_define(cpp, "linux", "1");
        break;
    case OS_MACOS:
        cpp_define(cpp, "__APPLE__", "1"); cpp_define(cpp, "__MACH__", "1");
        break;
    case OS_WASI:
        cpp_define(cpp, "__wasi__",  "1");
        break;
    default: break;
    }
    if (target->abi == ABI_MSVC) {
        cpp_define(cpp, "_MSC_VER",        "1940");
        cpp_define(cpp, "_MSC_FULL_VER",   "194033519");
        cpp_define(cpp, "_MSC_BUILD",      "1");
        cpp_define(cpp, "_MSC_EXTENSIONS", "1");
        cpp_define(cpp, "_INTEGRAL_MAX_BITS", "64");
        cpp_define(cpp, "_MT",            "1");
        cpp_define(cpp, "__CRTDECL",              "__cdecl");
        cpp_define(cpp, "__CLRCALL_PURE_OR_CDECL","__cdecl");
        cpp_define(cpp, "__CLRCALL_OR_CDECL",     "__cdecl");
    }
    cpp_define(cpp, "__STDC__",           "1");
    cpp_define(cpp, "__STDC_HOSTED__",    "1");
    cpp_define(cpp, "__STDC_VERSION__",   "201112L");
    cpp_define(cpp, "__COUNTER__",        "0");
    int is_64 = (target->arch == ARCH_X86_64 || target->arch == ARCH_AARCH64 ||
                 target->arch == ARCH_RISCV64);
    cpp_define(cpp, "_SIZE_T_DEFINED",     "1");
    cpp_define(cpp, "__SIZE_TYPE__",       is_64 ? "unsigned long long" : "unsigned int");
    cpp_define(cpp, "__PTRDIFF_TYPE__",    is_64 ? "long long" : "int");
    cpp_define(cpp, "__INTPTR_TYPE__",     is_64 ? "long long" : "int");
    cpp_define(cpp, "__WCHAR_TYPE__",      is_64 ? "unsigned short" : "unsigned short");
    if (target->abi == ABI_MSVC) {
        cpp_define(cpp, "__CRTDECL",              "__cdecl");
        cpp_define(cpp, "__CLRCALL_PURE_OR_CDECL","__cdecl");
        cpp_define(cpp, "__CLRCALL_OR_CDECL",     "__cdecl");
        cpp_define(cpp, "_CRTIMP",      "");
        cpp_define(cpp, "_CRTIMP1",     "");
        cpp_define(cpp, "_ACRTIMP",     "");
        cpp_define(cpp, "_DCRTIMP",     "");
        cpp_define(cpp, "_CRTIMP2",     "");
        cpp_define(cpp, "_CRTIMP2_PURE","");
        cpp_define(cpp, "_ACRTIMP_ALT", "");
        cpp_define(cpp, "_CRT_STDIO_INLINE",    "__inline");
        cpp_define(cpp, "_NO_CRT_STDIO_INLINE", "");
        cpp_define(cpp, "_ACRTIMP_INLINE",      "__inline");
        cpp_define(cpp, "__pragma(x)",  "");
        cpp_define(cpp, "__pragma",     "");
        cpp_define(cpp, "_In_",            "");
        cpp_define(cpp, "_In_z_",          "");
        cpp_define(cpp, "_In_opt_",        "");
        cpp_define(cpp, "_Out_",           "");
        cpp_define(cpp, "_Out_z_",         "");
        cpp_define(cpp, "_Out_opt_",       "");
        cpp_define(cpp, "_Inout_",         "");
        cpp_define(cpp, "_Inout_opt_",     "");
        cpp_define(cpp, "_Success_(x)",    "");
        cpp_define(cpp, "_Check_return_",  "");
        cpp_define(cpp, "_Printf_format_string_", "");
        cpp_define(cpp, "_Field_size_(x)", "");
        cpp_define(cpp, "_Field_size_opt_(x)", "");
        cpp_define(cpp, "_ACRTIMP_NOALIAS", "");
        cpp_define(cpp, "_ACRTIMP_NONALIAS_RET", "");
        cpp_define(cpp, "_CRTRESTRICT",     "");
        cpp_define(cpp, "__CRT_INLINE",  "inline");
        cpp_define(cpp, "_CRT_INLINE",   "inline");
    }
}

static char* read_file(const char* path);

/* Compile a C source file to an executable using the system C compiler.
 * Windows: clang (MSVC-compatible)
 * Unix:    clang / gcc
 * Returns 0 on success.
 */
static int compile_c_to_exe(const char* c_file, const char* exe_file) {
#ifdef _WIN32
    /* On Windows, use clang-cl with MSVC/UCRT target for maximum compatibility */
    const char* compilers[] = {"clang-cl", "cl", NULL};
    for (int i = 0; compilers[i]; i++) {
        char check_cmd[256];
        snprintf(check_cmd, sizeof(check_cmd), "where %s >nul 2>&1", compilers[i]);
        if (system(check_cmd) != 0) continue;

        char cmd[1024];
        if (strcmp(compilers[i], "clang-cl") == 0) {
            snprintf(cmd, sizeof(cmd),
                "clang-cl --target=x86_64-pc-windows-msvc -O2 -o \"%s\" \"%s\"",
                exe_file, c_file);
        } else {
            /* MSVC cl.exe */
            snprintf(cmd, sizeof(cmd),
                "cl /O2 /Fe\"%s\" \"%s\"",
                exe_file, c_file);
        }
        fprintf(stderr, "sharpc: compiling with %s...\n", compilers[i]);
        int rc = system(cmd);
        if (rc == 0) return 0;
        fprintf(stderr, "sharpc: %s failed, trying next compiler...\n", compilers[i]);
    }
    fprintf(stderr, "sharpc: no suitable C compiler found (tried clang-cl, cl)\n");
    fprintf(stderr, "sharpc: Install clang (LLVM) or MSVC Build Tools\n");
    return -1;
#else
    const char* compilers[] = {"clang", "gcc", "cc", NULL};
    char cmd[1024];
    for (int i = 0; compilers[i]; i++) {
        char check_cmd[256];
        snprintf(check_cmd, sizeof(check_cmd), "which %s > /dev/null 2>&1", compilers[i]);
        if (system(check_cmd) != 0) continue;

        snprintf(cmd, sizeof(cmd), "%s -std=c11 -O2 -Wall -o '%s' '%s'",
                 compilers[i], exe_file, c_file);
        fprintf(stderr, "sharpc: compiling with %s...\n", compilers[i]);
        int rc = system(cmd);
        if (rc == 0) return 0;
        fprintf(stderr, "sharpc: %s failed, trying next compiler...\n", compilers[i]);
    }
    fprintf(stderr, "sharpc: no suitable C compiler found (tried clang, gcc, cc)\n");
    return -1;
#endif
}

static char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open '%s'\n", path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = 0;
    fclose(f);
    return buf;
}

static char* default_out_path(const char* in) {
    size_t n = strlen(in);
    char* out = (char*)malloc(n + 3);
    memcpy(out, in, n + 1);
    if (n >= 3 && strcmp(in + n - 3, ".sp") == 0) {
        out[n - 3] = '.'; out[n - 2] = 'c'; out[n - 1] = 0;
    } else {
        strcat(out, ".c");
    }
    return out;
}

static char* default_exe_path(const char* in_path) {
    size_t n = strlen(in_path);
    char* out = (char*)malloc(n + 5);
    memcpy(out, in_path, n + 1);
    if (n >= 3 && strcmp(in_path + n - 3, ".sp") == 0) {
#ifdef _WIN32
        out[n - 3] = '.'; out[n - 2] = 'e'; out[n - 1] = 'x'; out[n] = 'e'; out[n+1] = 0;
#else
        out[n - 2] = 0;
#endif
    } else {
#ifdef _WIN32
        strcat(out, ".exe");
#else
        strcat(out, ".out");
#endif
    }
    return out;
}

int main(int argc, char** argv) {
    const char* in_path  = NULL;
    const char* out_path = NULL;
    const char* target_str = NULL;
    bool        dump_hir = false;
    bool        no_link  = false;
    bool        preprocess_only = false;  /* -E flag */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
        } else if (strcmp(argv[i], "-E") == 0) {
            preprocess_only = true;
        } else if (strcmp(argv[i], "-dump-hir") == 0) {
            dump_hir = true;
        } else if (strcmp(argv[i], "-no-link") == 0) {
            no_link = true;
        } else if (strncmp(argv[i], "--target=", 9) == 0) {
            target_str = argv[i] + 9;
        } else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            target_str = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] == 0) {
            out_path = "-";
        } else if (strncmp(argv[i], "-I", 2) == 0 && argv[i][2]) {
            /* handled below via CppCtx */
        } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            i++; /* skip path; handled below */
        } else if (strncmp(argv[i], "-D", 2) == 0 && argv[i][2]) {
            /* handled below */
        } else if (strncmp(argv[i], "-U", 2) == 0 && argv[i][2]) {
            /* handled below */
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "unknown flag: %s\n", argv[i]);
            return 2;
        } else if (!in_path) {
            in_path = argv[i];
        } else {
            fprintf(stderr, "multiple input files not supported\n");
            return 2;
        }
    }
    if (!in_path) {
        fprintf(stderr, "usage: sharpc <file.[sp|c]> [-E] [-o out] [-dump-hir] [-no-link] [--target <triple>]\n");
        fprintf(stderr, "  -E            preprocess only, write to stdout (or -o file)\n");
        fprintf(stderr, "  target examples: x86_64-windows-msvc, x86_64-linux-gnu, aarch64-macos-none\n");
        return 2;
    }

    /* Resolve target triple */
    TargetTriple target;
    if (target_str) {
        if (!parse_target_triple(target_str, &target)) {
            fprintf(stderr, "error: invalid target triple '%s'\n", target_str);
            return 2;
        }
    } else {
        target = target_default();
    }

    /* ------------------------------------------------------------------ *
     * Preprocessing pass (translation phases 1-6)                        *
     * ------------------------------------------------------------------ */
    CppCtx *cpp_ctx = cpp_ctx_new();

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-I", 2) == 0 && argv[i][2]) {
            cpp_add_user_include(cpp_ctx, argv[i] + 2);
        } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            cpp_add_user_include(cpp_ctx, argv[++i]);
        } else if (strncmp(argv[i], "-D", 2) == 0 && argv[i][2]) {
            const char *def = argv[i] + 2;
            const char *eq  = strchr(def, '=');
            if (eq) {
                char name[256]; size_t nl = (size_t)(eq - def);
                if (nl >= sizeof name) nl = sizeof name - 1;
                memcpy(name, def, nl); name[nl] = '\0';
                cpp_define(cpp_ctx, name, eq + 1);
            } else { cpp_define(cpp_ctx, def, NULL); }
        } else if (strncmp(argv[i], "-U", 2) == 0 && argv[i][2]) {
            cpp_undefine(cpp_ctx, argv[i] + 2);
        }
    }

    apply_target_macros(cpp_ctx, &target);

    /* ------------------------------------------------------------------ *
     * System include paths
     *
     * Windows: MSVC + UCRT only.
     *   Priority: INCLUDE env var (vcvarsall/Developer Prompt) → VCToolsInstallDir
     *             → enumerate VS 2022 versioned dirs → Windows SDK dirs.
     *
     * Linux/macOS: system include dirs only.
     * ------------------------------------------------------------------ */
#ifdef _WIN32
    {
        const char *inc_env = getenv("INCLUDE");
        if (inc_env && inc_env[0]) {
            /* vcvarsall.bat already set INCLUDE — contains VC + SDK + UCRT. */
            char buf[8192];
            strncpy(buf, inc_env, sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
            char *p = buf;
            while (*p) {
                char *semi = strchr(p, ';');
                if (semi) *semi = '\0';
                if (*p) cpp_add_sys_include(cpp_ctx, p);
                if (!semi) break;
                p = semi + 1;
            }
        } else {
            /* No vcvarsall — discover VS install ourselves. */
            bool found_vc = false;

            /* VCToolsInstallDir is set by vcvarsall even without a full
             * Developer Prompt (e.g. cmake --build invoked from VS).     */
            const char *vc_tools = getenv("VCToolsInstallDir");
            if (vc_tools && vc_tools[0]) {
                char inc[512];
                snprintf(inc, sizeof(inc), "%s\\include", vc_tools);
                DWORD attr = GetFileAttributesA(inc);
                if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
                    cpp_add_sys_include(cpp_ctx, inc);
                    found_vc = true;
                }
            }

            if (!found_vc) {
                /* Enumerate versioned subdirs under known VS 2022 MSVC bases */
                const char *vc_bases[] = {
                    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC",
                    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\MSVC",
                    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC",
                    "C:\\Program Files\\Microsoft Visual Studio\\2022\\BuildTools\\VC\\Tools\\MSVC",
                    NULL
                };
                for (int bi = 0; vc_bases[bi] && !found_vc; bi++) {
                    char pattern[512];
                    snprintf(pattern, sizeof(pattern), "%s\\*", vc_bases[bi]);
                    WIN32_FIND_DATAA fd;
                    HANDLE hf = FindFirstFileA(pattern, &fd);
                    if (hf == INVALID_HANDLE_VALUE) continue;
                    do {
                        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                        if (fd.cFileName[0] == '.') continue;
                        char inc[512];
                        snprintf(inc, sizeof(inc), "%s\\%s\\include",
                                 vc_bases[bi], fd.cFileName);
                        DWORD attr = GetFileAttributesA(inc);
                        if (attr != INVALID_FILE_ATTRIBUTES &&
                            (attr & FILE_ATTRIBUTE_DIRECTORY)) {
                            cpp_add_sys_include(cpp_ctx, inc);
                            found_vc = true;
                            break;
                        }
                    } while (FindNextFileA(hf, &fd));
                    FindClose(hf);
                }
            }

            /* Windows SDK: pick the newest installed version and add
             * ucrt, shared, and um sub-directories.                  */
            const char *sdk_base =
                "C:\\Program Files (x86)\\Windows Kits\\10\\Include";
            {
                char sdk_pattern[512];
                snprintf(sdk_pattern, sizeof(sdk_pattern), "%s\\*", sdk_base);
                WIN32_FIND_DATAA sd;
                HANDLE hs = FindFirstFileA(sdk_pattern, &sd);
                char best_ver[64] = {0};
                if (hs != INVALID_HANDLE_VALUE) {
                    do {
                        if (!(sd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                        if (sd.cFileName[0] == '.') continue;
                        if (strcmp(sd.cFileName, best_ver) > 0)
                            strncpy(best_ver, sd.cFileName, sizeof(best_ver) - 1);
                    } while (FindNextFileA(hs, &sd));
                    FindClose(hs);
                }
                if (best_ver[0]) {
                    const char *subs[] = { "ucrt", "shared", "um", NULL };
                    for (int si = 0; subs[si]; si++) {
                        char p[512];
                        snprintf(p, sizeof(p), "%s\\%s\\%s",
                                 sdk_base, best_ver, subs[si]);
                        DWORD attr = GetFileAttributesA(p);
                        if (attr != INVALID_FILE_ATTRIBUTES &&
                            (attr & FILE_ATTRIBUTE_DIRECTORY))
                            cpp_add_sys_include(cpp_ctx, p);
                    }
                }
            }
        }
    }
#else
    /* Linux / macOS: system headers only. */
    cpp_add_sys_include(cpp_ctx, "/usr/include");
    cpp_add_sys_include(cpp_ctx, "/usr/local/include");
#endif

    /* Use C language mode when the input file ends in .c */
    CppLang lang = CPP_LANG_SHARP;
    {
        size_t plen = strlen(in_path);
        if (plen >= 2 && strcmp(in_path + plen - 2, ".c") == 0)
            lang = CPP_LANG_C;
    }

    CppResult pp = cpp_run(cpp_ctx, in_path, lang);
    cpp_print_diags(&pp);
    if (pp.error) {
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 1;
    }

    /* -E: preprocess only — write text to stdout (or -o file) and exit. */
    if (preprocess_only) {
        FILE *out = stdout;
        bool  close_out = false;
        if (out_path && strcmp(out_path, "-") != 0) {
            out = fopen(out_path, "wb");
            if (!out) {
                fprintf(stderr, "sharpc: cannot open '%s' for writing\n", out_path);
                cpp_result_free(&pp);
                cpp_ctx_free(cpp_ctx);
                return 1;
            }
            close_out = true;
        }
        if (pp.text && pp.text_len)
            fwrite(pp.text, 1, pp.text_len, out);
        if (close_out) fclose(out);
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 0;
    }

    if (getenv("SHARPC_DUMP_PP")) {
        FILE* pf = fopen("preprocessed.out", "w");
        if (pf) {
            fputs(pp.text, pf);
            fclose(pf);
            fprintf(stderr, "[debug] preprocessed output written to preprocessed.out\n");
        }
    }

    char* orig_src = read_file(in_path);
    diag_set_source(in_path, orig_src ? orig_src : pp.text);

    /* Remove GCC/clang line markers (# line "file" or # line) that
     * interfere with parsing. Replace them with empty lines to preserve
     * line numbers for diagnostics. */
    char* src = pp.text;
    {
        char* p = src;
        while ((p = strstr(p, "\n# ")) != NULL) {
            char* line_start = p + 1;  /* skip the '\n' */
            char* newline = strchr(line_start, '\n');
            if (newline) {
                /* Blank out the line marker, keep the newline */
                size_t len = (size_t)(newline - line_start);
                memset(line_start, ' ', len);
                p = newline;
            } else {
                /* Last line, no trailing newline */
                size_t len = strlen(line_start);
                memset(line_start, ' ', len);
                break;
            }
        }
    }

    Arena* arena = NULL;
    Lexer lx;
    lex_init(&lx, src, in_path);
    Node* prog = parse_program(&lx, &arena);

    SymTable* st = sema_build(prog, &arena);

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d warning(s) during semantic analysis\n",
                g_error_count);
        /* Don't abort — let C compiler handle semantic issues */
    }

    lower_program(prog, st, &arena);
    fprintf(stderr, "sharpc: lowering complete\n");

    HirProg* hir = hir_build(prog, st, &arena);
    fprintf(stderr, "sharpc: HIR build complete\n");
    hir_mark_reachable(hir);
    hir_check_returns(hir);

    if (dump_hir) hir_dump(hir, stderr);

    hir_free(hir);
    fprintf(stderr, "sharpc: HIR freed\n");

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d warning(s) during analysis\n",
                g_error_count);
        /* Don't abort — continue to generate C code */
    }

    /* ------------------------------------------------------------------ *
     * Code generation + compilation using system C compiler              *
     * ------------------------------------------------------------------ */
    bool out_alloced = false;
    if (!out_path) { out_path = default_out_path(in_path); out_alloced = true; }
    bool should_link = (strcmp(out_path, "-") != 0) && !no_link;

    if (strcmp(out_path, "-") == 0) {
        fprintf(stderr, "sharpc: generating C to stdout...\n");
        cgen_c(prog, st, stdout);
    } else if (!should_link) {
        fprintf(stderr, "sharpc: generating C to '%s'...\n", out_path);
        FILE* out = fopen(out_path, "wb");
        if (!out) {
            fprintf(stderr, "sharpc: cannot write '%s'\n", out_path);
            free(orig_src);
            arena_free_all(&arena);
            return 1;
        }
        cgen_c(prog, st, out);
        fprintf(stderr, "sharpc: C code generation complete, closing file...\n");
        fclose(out);
        fprintf(stderr, "sharpc: C code written to '%s'\n", out_path);
    } else {
        StrBuf cbuf = {0};
        cgen_buf(prog, st, &cbuf);

        /* Determine if output is an executable */
        const char* exe_path = NULL;
        bool exe_alloced = false;
        bool is_exe_output = false;
        if (out_path && strcmp(out_path, "-") != 0) {
            size_t ol = strlen(out_path);
            is_exe_output = (ol >= 4 && strcmp(out_path + ol - 4, ".exe") == 0) ||
                            (ol >= 4 && strcmp(out_path + ol - 4, ".out") == 0);
        }

        const char* c_temp_path;
        bool c_temp_alloced = false;
        if (is_exe_output) {
            /* Generate a temporary .c file for compilation */
            size_t len = strlen(out_path) + 3;
            char* tmp = (char*)malloc(len);
            strncpy(tmp, out_path, len - 1);
            tmp[len - 1] = '\0';
            /* Replace .exe/.out with .c */
            char* ext = strrchr(tmp, '.');
            if (ext) {
                strcpy(ext, ".c");
            } else {
                strcat(tmp, ".c");
            }
            c_temp_path = tmp;
            c_temp_alloced = true;
            exe_path = out_path;
        } else {
            c_temp_path = out_path;
            exe_path = default_exe_path(in_path);
            exe_alloced = true;
        }

        FILE* c_out = fopen(c_temp_path, "wb");
        if (!c_out) {
            sb_free(&cbuf);
            fprintf(stderr, "sharpc: cannot write '%s'\n", c_temp_path);
            if (c_temp_alloced) free((char*)c_temp_path);
            if (exe_alloced) free((char*)exe_path);
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            return 1;
        }
        fwrite(cbuf.data, 1, cbuf.len, c_out);
        fclose(c_out);
        sb_free(&cbuf);

        if (compile_c_to_exe(c_temp_path, exe_path) != 0) {
            fprintf(stderr, "sharpc: C compilation failed\n");
            remove(c_temp_path);
            if (c_temp_alloced) free((char*)c_temp_path);
            if (exe_alloced) free((char*)exe_path);
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            cpp_result_free(&pp);
            cpp_ctx_free(cpp_ctx);
            return 1;
        }

        fprintf(stderr, "sharpc: %s built successfully\n", exe_path);
        /* Clean up temporary C file */
        if (c_temp_alloced) {
            remove(c_temp_path);
            free((char*)c_temp_path);
        }
        if (exe_alloced) free((char*)exe_path);
    }

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d warning(s) in total\n", g_error_count);
    }

    if (out_alloced)   free((char*)out_path);
    free(orig_src);
    arena_free_all(&arena);
    cpp_result_free(&pp);
    cpp_ctx_free(cpp_ctx);
    return 0;
}
