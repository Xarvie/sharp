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
 *   sharpc a.sp b.sp -o out.exe        # multi-file compile + link
 */
#include "sharp.h"
#include "hir.h"
#include "cpp.h"

#ifdef _WIN32
  #include <windows.h>
  #include <process.h>
  #include <direct.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
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

/* Add system include paths to cpp_ctx (MSVC/UCRT/SDK on Windows). */
static void setup_sys_includes(CppCtx *cpp_ctx) {
#ifdef _WIN32
    const char *inc_env = getenv("INCLUDE");
    if (inc_env && inc_env[0]) {
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
        bool found_vc = false;
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
        const char *sdk_base =
            "C:\\Program Files (x86)\\Windows Kits\\10\\Include";
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
#else
    cpp_add_sys_include(cpp_ctx, "/usr/include");
    cpp_add_sys_include(cpp_ctx, "/usr/local/include");
#endif
}

static bool detect_file_is_c(const char* path) {
    size_t n = strlen(path);
    return n >= 2 && strcmp(path + n - 2, ".c") == 0;
}

/* Process one file for -E mode: preprocess, convert linemarkers, write output.
 * Returns 0 on success, 1 on failure. */
static int process_one_file(CppCtx *cpp_ctx, const char* in, const char* out) {
    CppLang lang = detect_file_is_c(in) ? CPP_LANG_C : CPP_LANG_SHARP;
    CppResult pp = cpp_run(cpp_ctx, in, lang);
    cpp_print_diags(&pp);
    if (pp.error) { cpp_result_free(&pp); return 1; }

    {
        char* p = pp.text;
        while ((p = strstr(p, "# ")) != NULL) {
            char* after_hash = p + 1;
            if (after_hash[1] >= '0' && after_hash[1] <= '9') {
                /* Convert "# 123 ..." → "#line 123 ..."
                 * Need to shift everything after "# " right by 4 bytes
                 * to make room for "line" */
                size_t shift = 4;
                size_t tail_start = (size_t)(after_hash - pp.text);
                size_t tail_len = pp.text_len - tail_start;
                /* Grow the buffer by 4 bytes */
                char* new_buf = realloc(pp.text, pp.text_len + shift + 1);
                if (!new_buf) {
                    fprintf(stderr, "sharpc: realloc failed\n");
                    cpp_result_free(&pp);
                    return 1;
                }
                pp.text = new_buf;
                /* Recalculate pointers after realloc */
                after_hash = pp.text + tail_start;
                p = pp.text + (p - new_buf);
                /* Shift tail right by 4 bytes */
                memmove(after_hash + 1 + shift, after_hash + 1, tail_len);
                /* Insert "line" */
                after_hash[0] = 'l'; after_hash[1] = 'i';
                after_hash[2] = 'n'; after_hash[3] = 'e'; after_hash[4] = ' ';
                pp.text_len += shift;
                p = after_hash + 5;
            } else {
                p = after_hash;
            }
        }
    }

    FILE* fout = fopen(out, "wb");
    if (!fout) {
        fprintf(stderr, "sharpc: cannot open '%s' for writing\n", out);
        cpp_result_free(&pp);
        return 1;
    }
    if (pp.text && pp.text_len)
        fwrite(pp.text, 1, pp.text_len, fout);
    fclose(fout);
    cpp_result_free(&pp);
    fprintf(stderr, "sharpc: preprocessed output written to '%s'\n", out);
    return 0;
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

int main(int argc, char** argv) {
    const char* in_path  = NULL;
    const char* out_path = NULL;
    const char* target_str = NULL;
    bool        dump_hir = false;
    bool        preprocess_only = false;

    /* -E mode with paired args: sharpc -E in1.c out1.c in2.c out2.c */
    typedef struct { const char *in, *out; } EPair;
    EPair* e_pairs = NULL;
    int    e_count = 0;
    int    pair_start = -1; /* argv index where -E args start */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
        } else if (strcmp(argv[i], "-E") == 0) {
            preprocess_only = true;
        } else if (strcmp(argv[i], "-dump-hir") == 0) {
            dump_hir = true;
        } else if (strncmp(argv[i], "--target=", 9) == 0) {
            target_str = argv[i] + 9;
        } else if (strcmp(argv[i], "--target") == 0 && i + 1 < argc) {
            target_str = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] == 0) {
            out_path = "-";
        } else if (strncmp(argv[i], "-I", 2) == 0 && argv[i][2]) {
        } else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            i++;
        } else if (strncmp(argv[i], "-D", 2) == 0 && argv[i][2]) {
        } else if (strncmp(argv[i], "-U", 2) == 0 && argv[i][2]) {
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "unknown flag: %s\n", argv[i]);
            return 2;
        } else if (preprocess_only) {
            if (pair_start < 0) pair_start = i;
            /* -E mode: collect pairs of positional args (input output) */
            if (e_count % 2 == 0) {
                e_pairs = realloc(e_pairs, (e_count/2 + 1) * sizeof(EPair));
                e_pairs[e_count/2].in = argv[i];
                e_pairs[e_count/2].out = NULL;
                e_count++;
            } else {
                e_pairs[e_count/2].out = argv[i];
                e_count++;
            }
        } else if (!in_path) {
            in_path = argv[i];
        } else {
            fprintf(stderr, "multiple input files not supported\n");
            return 2;
        }
    }

    if (preprocess_only) {
        if (e_count == 0 || e_count % 2 != 0) {
            fprintf(stderr, "usage: sharpc -E <in1> <out1> [<in2> <out2> ...]\n");
            free(e_pairs);
            return 2;
        }
        CppCtx *cpp_ctx = cpp_ctx_new();
        for (int i = 1; i < pair_start; i++) {
            if (strncmp(argv[i], "-I", 2) == 0 && argv[i][2])
                cpp_add_user_include(cpp_ctx, argv[i] + 2);
            else if (strcmp(argv[i], "-I") == 0 && i + 1 < argc)
                cpp_add_user_include(cpp_ctx, argv[++i]);
            else if (strncmp(argv[i], "-D", 2) == 0 && argv[i][2]) {
                const char *def = argv[i] + 2;
                const char *eq  = strchr(def, '=');
                if (eq) {
                    char name[256]; size_t nl = (size_t)(eq - def);
                    if (nl >= sizeof name) nl = sizeof name - 1;
                    memcpy(name, def, nl); name[nl] = '\0';
                    cpp_define(cpp_ctx, name, eq + 1);
                } else { cpp_define(cpp_ctx, def, NULL); }
            } else if (strncmp(argv[i], "-U", 2) == 0 && argv[i][2])
                cpp_undefine(cpp_ctx, argv[i] + 2);
        }
        TargetTriple et = target_default();
        apply_target_macros(cpp_ctx, &et);
        setup_sys_includes(cpp_ctx);

        int rc = 0;
        for (int pi = 0; pi < e_count/2; pi++) {
            if (process_one_file(cpp_ctx, e_pairs[pi].in, e_pairs[pi].out) != 0)
                rc = 1;
        }
        cpp_ctx_free(cpp_ctx);
        free(e_pairs);
        return rc;
    }

    /* Collect all input files */
    const char** input_files = NULL;
    int ninputs = 0;
    int input_cap = 8;
    input_files = (const char**)calloc(input_cap, sizeof(const char*));

    /* Collect all non-option, non-preprocess-only arguments as inputs */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        if (ninputs >= input_cap) {
            input_cap *= 2;
            input_files = (const char**)realloc(input_files, input_cap * sizeof(const char*));
        }
        input_files[ninputs++] = argv[i];
    }

    if (ninputs == 0) {
        fprintf(stderr, "usage: sharpc <file1> [file2 ...] [-o out] [-dump-hir] [--target <triple>]\n");
        fprintf(stderr, "       sharpc -E <in1> <out1> [<in2> <out2> ...]\n");
        free(input_files);
        return 2;
    }

    /* Resolve target triple */
    TargetTriple target;
    if (target_str) {
        if (!parse_target_triple(target_str, &target)) {
            fprintf(stderr, "error: invalid target triple '%s'\n", target_str);
            free(input_files);
            return 2;
        }
    } else {
        target = target_default();
    }

    /* ------------------------------------------------------------------ *
     * Shared preprocessor context                                        *
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
    setup_sys_includes(cpp_ctx);

    /* Shared template registry for cross-file generic visibility */
    TemplateRegistry templates;
    tmpl_reg_init(&templates);

    /* ------------------------------------------------------------------ *
     * tmp/ directory management                                          *
     * ------------------------------------------------------------------ */
    const char* tmp_dir = "tmp";
#ifdef _WIN32
    _mkdir(tmp_dir);
#else
    mkdir(tmp_dir, 0755);
#endif

    /* Track generated .c files for later linking */
    const char** tmp_files = NULL;
    int ntmp_files = 0;
    int tmp_cap = 8;
    tmp_files = (const char**)calloc(tmp_cap, sizeof(const char*));

    /* ------------------------------------------------------------------ *
     * Per-File compilation loop                                          *
     * ------------------------------------------------------------------ */
    int compile_errors = 0;

    for (int fi = 0; fi < ninputs; fi++) {
        const char* in_path = input_files[fi];
        fprintf(stderr, "sharpc: compiling %s...\n", in_path);

        /* Detect language mode */
        CppLang lang = CPP_LANG_SHARP;
        {
            size_t plen = strlen(in_path);
            if (plen >= 2 && strcmp(in_path + plen - 2, ".c") == 0)
                lang = CPP_LANG_C;
        }

        /* Phase A: Preprocess */
        CppResult pp = cpp_run(cpp_ctx, in_path, lang);
        cpp_print_diags(&pp);
        if (pp.error) {
            fprintf(stderr, "sharpc: preprocessing failed for %s\n", in_path);
            cpp_result_free(&pp);
            compile_errors = 1;
            continue;
        }

        /* Convert "# 123" → "#line 123" in preprocessed output */
        {
            char* p = pp.text;
            while ((p = strstr(p, "# ")) != NULL) {
                char* after_hash = p + 1;
                if (after_hash[1] >= '0' && after_hash[1] <= '9') {
                    size_t shift = 4;
                    size_t tail_start = (size_t)(after_hash - pp.text);
                    size_t tail_len = pp.text_len - tail_start;
                    char* new_buf = realloc(pp.text, pp.text_len + shift + 1);
                    if (!new_buf) {
                        fprintf(stderr, "sharpc: realloc failed\n");
                        cpp_result_free(&pp);
                        compile_errors = 1;
                        goto cleanup;
                    }
                    pp.text = new_buf;
                    after_hash = pp.text + tail_start;
                    p = pp.text + (p - new_buf);
                    memmove(after_hash + 1 + shift, after_hash + 1, tail_len);
                    after_hash[0] = 'l'; after_hash[1] = 'i';
                    after_hash[2] = 'n'; after_hash[3] = 'e'; after_hash[4] = ' ';
                    pp.text_len += shift;
                    p = after_hash + 5;
                } else {
                    p = after_hash;
                }
            }
        }

        /* Phase B: lex → parse → sema → lower → codegen
         * ALL files (.sp/.c/.h) go through the SAME pipeline.
         * .c files are treated as Sharp source files (Sharp is a C superset). */
        Arena* arena = NULL;
        Lexer lx;
        lex_init(&lx, pp.text, in_path);
        Node* prog = parse_program(&lx, &arena);

        if (g_error_count) {
            fprintf(stderr, "sharpc: %d error(s) during parsing %s\n", g_error_count, in_path);
        }

        SymTable* st = sema_build(prog, &arena);

        if (g_error_count) {
            fprintf(stderr, "sharpc: %d error(s) during semantic analysis of %s\n", g_error_count, in_path);
        }

        lower_program(prog, st, &arena);

        HirProg* hir = hir_build(prog, st, &arena);
        hir_mark_reachable(hir);
        hir_check_returns(hir);
        if (dump_hir) hir_dump(hir, stderr);
        hir_free(hir);

        /* Phase C: Generate tmp/<basename>.c */
        char basename_buf[512];
        const char* base = strrchr(in_path, '/');
        if (!base) base = strrchr(in_path, '\\');
        base = base ? base + 1 : in_path;

        /* Strip extension */
        strncpy(basename_buf, base, sizeof(basename_buf) - 1);
        basename_buf[sizeof(basename_buf) - 1] = '\0';
        char* dot = strrchr(basename_buf, '.');
        if (dot) *dot = '\0';

        char tmp_path[768];
        snprintf(tmp_path, sizeof(tmp_path), "%s/%s.c", tmp_dir, basename_buf);

        FILE* out = fopen(tmp_path, "wb");
        if (!out) {
            fprintf(stderr, "sharpc: cannot write '%s'\n", tmp_path);
            compile_errors = 1;
            cpp_result_free(&pp);
            continue;
        }
        cgen_c(prog, st, out);
        fclose(out);

        /* Track tmp file for linking */
        if (ntmp_files >= tmp_cap) {
            tmp_cap *= 2;
            tmp_files = (const char**)realloc(tmp_files, tmp_cap * sizeof(const char*));
        }
        tmp_files[ntmp_files++] = strdup(tmp_path);

        /* Free per-file resources */
        char* orig_src = read_file(in_path);
        diag_set_source(in_path, orig_src ? orig_src : pp.text);
        free(orig_src);
        arena_free_all(&arena);
        cpp_result_free(&pp);

        fprintf(stderr, "sharpc: %s → %s\n", in_path, tmp_path);
    }

cleanup:
    if (compile_errors) {
        fprintf(stderr, "sharpc: compilation failed\n");
    } else {
        fprintf(stderr, "sharpc: %d file(s) compiled successfully\n", ntmp_files);
    }

    /* Free resources */
    for (int i = 0; i < ntmp_files; i++) free((char*)tmp_files[i]);
    free(tmp_files);
    tmpl_reg_free(&templates);
    cpp_ctx_free(cpp_ctx);
    free(input_files);

    return compile_errors ? 1 : 0;
}
