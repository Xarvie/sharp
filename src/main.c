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
  #include <libtcc.h>
#else
  #include <dlfcn.h>
  #include "libtcc.h"
#endif

/* ------------------------------------------------------------------ *
 * libtcc function pointer types (dynamic loading)                    *
 * ------------------------------------------------------------------ */
typedef TCCState* (*tcc_new_fn)(void);
typedef void      (*tcc_delete_fn)(TCCState*);
typedef int       (*tcc_set_output_type_fn)(TCCState*, int);
typedef int       (*tcc_compile_string_fn)(TCCState*, const char*);
typedef int       (*tcc_add_file_fn)(TCCState*, const char*);
typedef int       (*tcc_output_file_fn)(TCCState*, const char*);
typedef int       (*tcc_add_include_path_fn)(TCCState*, const char*);
typedef int       (*tcc_add_sysinclude_path_fn)(TCCState*, const char*);
typedef int       (*tcc_add_library_path_fn)(TCCState*, const char*);
typedef int       (*tcc_add_library_fn)(TCCState*, const char*);
typedef void      (*tcc_set_lib_path_fn)(TCCState*, const char*);
typedef void      (*tcc_set_error_func_fn)(TCCState*, void*, void(*)(void*,const char*));

typedef struct {
#ifdef _WIN32
    HMODULE dll;
#else
    void*   dl;
#endif
    tcc_new_fn              tcc_new;
    tcc_delete_fn           tcc_delete;
    tcc_set_output_type_fn  tcc_set_output_type;
    tcc_compile_string_fn   tcc_compile_string;
    tcc_add_file_fn         tcc_add_file;
    tcc_output_file_fn      tcc_output_file;
    tcc_add_include_path_fn tcc_add_include_path;
    tcc_add_sysinclude_path_fn tcc_add_sysinclude_path;
    tcc_add_library_path_fn tcc_add_library_path;
    tcc_add_library_fn      tcc_add_library;
    tcc_set_lib_path_fn     tcc_set_lib_path;
    tcc_set_error_func_fn   tcc_set_error_func;
} TccLib;

static void tcc_error_cb(void* opaque, const char* msg) {
    (void)opaque;
    fprintf(stderr, "libtcc: %s\n", msg);
}

#ifdef _WIN32
static int load_libtcc(TccLib* lib, const char* tcc_dir) {
    char dll_path[512];
    snprintf(dll_path, sizeof(dll_path), "%s\\libtcc.dll", tcc_dir);
    lib->dll = LoadLibraryA(dll_path);
    if (!lib->dll) {
        fprintf(stderr, "sharpc: cannot load %s (error %lu)\n", dll_path, GetLastError());
        return -1;
    }
    lib->tcc_new              = (tcc_new_fn)              GetProcAddress(lib->dll, "tcc_new");
    lib->tcc_delete           = (tcc_delete_fn)           GetProcAddress(lib->dll, "tcc_delete");
    lib->tcc_set_output_type  = (tcc_set_output_type_fn)  GetProcAddress(lib->dll, "tcc_set_output_type");
    lib->tcc_compile_string   = (tcc_compile_string_fn)   GetProcAddress(lib->dll, "tcc_compile_string");
    lib->tcc_add_file         = (tcc_add_file_fn)         GetProcAddress(lib->dll, "tcc_add_file");
    lib->tcc_output_file      = (tcc_output_file_fn)      GetProcAddress(lib->dll, "tcc_output_file");
    lib->tcc_add_include_path = (tcc_add_include_path_fn) GetProcAddress(lib->dll, "tcc_add_include_path");
    lib->tcc_add_sysinclude_path = (tcc_add_sysinclude_path_fn)GetProcAddress(lib->dll, "tcc_add_sysinclude_path");
    lib->tcc_add_library_path = (tcc_add_library_path_fn) GetProcAddress(lib->dll, "tcc_add_library_path");
    lib->tcc_add_library      = (tcc_add_library_fn)      GetProcAddress(lib->dll, "tcc_add_library");
    lib->tcc_set_lib_path     = (tcc_set_lib_path_fn)     GetProcAddress(lib->dll, "tcc_set_lib_path");
    lib->tcc_set_error_func   = (tcc_set_error_func_fn)   GetProcAddress(lib->dll, "tcc_set_error_func");
    return (lib->tcc_new && lib->tcc_delete && lib->tcc_compile_string && lib->tcc_output_file) ? 0 : -1;
}
static void unload_libtcc(TccLib* lib) { if (lib->dll) FreeLibrary(lib->dll); }
#else
static int load_libtcc(TccLib* lib, const char* tcc_dir) {
    (void)tcc_dir;
    lib->dl = dlopen("libtcc.so", RTLD_NOW);
    if (!lib->dl) { fprintf(stderr, "sharpc: cannot load libtcc.so: %s\n", dlerror()); return -1; }
    lib->tcc_new              = (tcc_new_fn)              dlsym(lib->dl, "tcc_new");
    lib->tcc_delete           = (tcc_delete_fn)           dlsym(lib->dl, "tcc_delete");
    lib->tcc_set_output_type  = (tcc_set_output_type_fn)  dlsym(lib->dl, "tcc_set_output_type");
    lib->tcc_compile_string   = (tcc_compile_string_fn)   dlsym(lib->dl, "tcc_compile_string");
    lib->tcc_add_file         = (tcc_add_file_fn)         dlsym(lib->dl, "tcc_add_file");
    lib->tcc_output_file      = (tcc_output_file_fn)      dlsym(lib->dl, "tcc_output_file");
    lib->tcc_add_include_path = (tcc_add_include_path_fn) dlsym(lib->dl, "tcc_add_include_path");
    lib->tcc_add_sysinclude_path = (tcc_add_sysinclude_path_fn)dlsym(lib->dl, "tcc_add_sysinclude_path");
    lib->tcc_add_library_path = (tcc_add_library_path_fn) dlsym(lib->dl, "tcc_add_library_path");
    lib->tcc_add_library      = (tcc_add_library_fn)      dlsym(lib->dl, "tcc_add_library");
    lib->tcc_set_lib_path     = (tcc_set_lib_path_fn)     dlsym(lib->dl, "tcc_set_lib_path");
    lib->tcc_set_error_func   = (tcc_set_error_func_fn)   dlsym(lib->dl, "tcc_set_error_func");
    return (lib->tcc_new && lib->tcc_delete && lib->tcc_compile_string && lib->tcc_output_file) ? 0 : -1;
}
static void unload_libtcc(TccLib* lib) { if (lib->dl) dlclose(lib->dl); }
#endif

/* ========================================================================
 * Target triple system (inspired by Zig: arch-os-abi)
 * Examples: x86_64-windows-msvc, x86_64-linux-gnu, aarch64-macos-none
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
    /* arch */
    if (strcmp(parts[0], "x86_64") == 0 || strcmp(parts[0], "amd64") == 0) out->arch = ARCH_X86_64;
    else if (strcmp(parts[0], "x86") == 0 || strcmp(parts[0], "i386") == 0 || strcmp(parts[0], "i686") == 0) out->arch = ARCH_X86;
    else if (strcmp(parts[0], "aarch64") == 0 || strcmp(parts[0], "arm64") == 0) out->arch = ARCH_AARCH64;
    else if (strncmp(parts[0], "arm", 3) == 0) out->arch = ARCH_ARM;
    else if (strcmp(parts[0], "wasm32") == 0) out->arch = ARCH_WASM32;
    else if (strcmp(parts[0], "riscv64") == 0) out->arch = ARCH_RISCV64;
    else return false;
    /* os */
    if (strcmp(parts[1], "windows") == 0 || strcmp(parts[1], "win32") == 0) out->os = OS_WINDOWS;
    else if (strcmp(parts[1], "linux") == 0) out->os = OS_LINUX;
    else if (strcmp(parts[1], "macos") == 0 || strcmp(parts[1], "darwin") == 0) out->os = OS_MACOS;
    else if (strcmp(parts[1], "none") == 0 || strcmp(parts[1], "freestanding") == 0) out->os = OS_NONE;
    else if (strcmp(parts[1], "wasi") == 0) out->os = OS_WASI;
    else return false;
    /* abi (optional) */
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
    /* Architecture */
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
    /* OS */
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
    /* MSVC version macros (MSVC ABI) */
    if (target->abi == ABI_MSVC) {
        cpp_define(cpp, "_MSC_VER",        "1940");
        cpp_define(cpp, "_MSC_FULL_VER",   "194033519");
        cpp_define(cpp, "_MSC_BUILD",      "1");
        cpp_define(cpp, "_MSC_EXTENSIONS", "1");
        cpp_define(cpp, "_INTEGRAL_MAX_BITS", "64");
        cpp_define(cpp, "_MT",            "1");
        /* Calling convention aliases */
        cpp_define(cpp, "__CRTDECL",              "__cdecl");
        cpp_define(cpp, "__CLRCALL_PURE_OR_CDECL","__cdecl");
        cpp_define(cpp, "__CLRCALL_OR_CDECL",     "__cdecl");
    }
    /* Standard C */
    cpp_define(cpp, "__STDC__",           "1");
    cpp_define(cpp, "__STDC_HOSTED__",    "1");
    cpp_define(cpp, "__STDC_VERSION__",   "201112L");
    cpp_define(cpp, "__COUNTER__",        "0");
    /* Size types based on arch */
    int is_64 = (target->arch == ARCH_X86_64 || target->arch == ARCH_AARCH64 ||
                 target->arch == ARCH_RISCV64);
    cpp_define(cpp, "_SIZE_T_DEFINED",     "1");
    cpp_define(cpp, "__SIZE_TYPE__",       is_64 ? "unsigned long long" : "unsigned int");
    cpp_define(cpp, "__PTRDIFF_TYPE__",    is_64 ? "long long" : "int");
    cpp_define(cpp, "__INTPTR_TYPE__",     is_64 ? "long long" : "int");
    cpp_define(cpp, "__WCHAR_TYPE__",      is_64 ? "unsigned short" : "unsigned short");

    /* MSVC ABI: SAL annotations and decoration macros — defined empty.
     * GCC and clang use the same approach when compiling MSVC headers. */
    if (target->abi == ABI_MSVC) {
        /* Calling convention macros */
        cpp_define(cpp, "__CRTDECL",              "__cdecl");
        cpp_define(cpp, "__CLRCALL_PURE_OR_CDECL","__cdecl");
        cpp_define(cpp, "__CLRCALL_OR_CDECL",     "__cdecl");

        /* Import/export decoration */
        cpp_define(cpp, "_CRTIMP",      "");
        cpp_define(cpp, "_CRTIMP1",     "");
        cpp_define(cpp, "_ACRTIMP",     "");
        cpp_define(cpp, "_DCRTIMP",     "");
        cpp_define(cpp, "_CRTIMP2",     "");
        cpp_define(cpp, "_CRTIMP2_PURE","");
        cpp_define(cpp, "_ACRTIMP_ALT", "");

        /* Inline function marker */
        cpp_define(cpp, "_CRT_STDIO_INLINE",    "__inline");
        cpp_define(cpp, "_NO_CRT_STDIO_INLINE", "");
        cpp_define(cpp, "_ACRTIMP_INLINE",      "__inline");

        /* Pragma passthrough */
        cpp_define(cpp, "__pragma(x)",  "");
        cpp_define(cpp, "__pragma",     "");

        /* SAL annotations — parameter checks */
        cpp_define(cpp, "_In_",            "");
        cpp_define(cpp, "_In_z_",          "");
        cpp_define(cpp, "_In_opt_",        "");
        cpp_define(cpp, "_In_opt_z_",      "");
        cpp_define(cpp, "_In_reads_(x)",   "");
        cpp_define(cpp, "_In_reads_opt_(x)", "");
        cpp_define(cpp, "_In_reads_bytes_(x)", "");
        cpp_define(cpp, "_In_count_(x)",   "");
        cpp_define(cpp, "_In_range_(x,y)", "");
        cpp_define(cpp, "_Out_",           "");
        cpp_define(cpp, "_Out_z_",         "");
        cpp_define(cpp, "_Out_opt_",       "");
        cpp_define(cpp, "_Out_writes_(x)", "");
        cpp_define(cpp, "_Out_writes_z_(x)", "");
        cpp_define(cpp, "_Out_writes_opt_(x)", "");
        cpp_define(cpp, "_Out_writes_to_(x,y)", "");
        cpp_define(cpp, "_Out_writes_bytes_(x)", "");
        cpp_define(cpp, "_Outptr_",        "");
        cpp_define(cpp, "_Outptr_opt_",    "");
        cpp_define(cpp, "_Outptr_result_z_", "");
        cpp_define(cpp, "_Outptr_result_buffer_(x)", "");
        cpp_define(cpp, "_Inout_",         "");
        cpp_define(cpp, "_Inout_opt_",     "");
        cpp_define(cpp, "_Inout_z_",       "");
        cpp_define(cpp, "_Inout_updates_(x)", "");
        cpp_define(cpp, "_Ret_z_",         "");
        cpp_define(cpp, "_Ret_opt_z_",     "");
        cpp_define(cpp, "_Ret_writes_(x)", "");
        cpp_define(cpp, "_Deref_out_",     "");
        cpp_define(cpp, "_Deref_out_z_",   "");
        cpp_define(cpp, "_Deref_opt_out_", "");
        cpp_define(cpp, "_Frees_ptr_",     "");
        cpp_define(cpp, "_Frees_ptr_opt_", "");

        /* SAL annotations — pre/post conditions */
        cpp_define(cpp, "_Pre_",           "");
        cpp_define(cpp, "_Post_",          "");
        cpp_define(cpp, "_Pre_z_",         "");
        cpp_define(cpp, "_Post_z_",        "");
        cpp_define(cpp, "_Post_valid_",    "");
        cpp_define(cpp, "_Pre_valid_",     "");
        cpp_define(cpp, "_Pre_writable_byte_size_(x)", "");
        cpp_define(cpp, "_Pre_readable_byte_size_(x)", "");
        cpp_define(cpp, "_Post_writable_byte_size_(x)", "");
        cpp_define(cpp, "_Readable_bytes_(x)", "");
        cpp_define(cpp, "_Writable_bytes_(x)", "");
        cpp_define(cpp, "_Null_terminated_", "");
        cpp_define(cpp, "_NullNull_terminated_", "");
        cpp_define(cpp, "_Pre_notnull_",   "");
        cpp_define(cpp, "_Pre_null_",      "");
        cpp_define(cpp, "_Post_null_",     "");

        /* SAL annotations — return and format */
        cpp_define(cpp, "_Success_(x)",    "");
        cpp_define(cpp, "_Return_type_success_(x)", "");
        cpp_define(cpp, "_Check_return_",  "");
        cpp_define(cpp, "_Check_return_opt_", "");
        cpp_define(cpp, "_Check_return_wat_", "");
        cpp_define(cpp, "_Printf_format_string_", "");
        cpp_define(cpp, "_Scanf_format_string_", "");
        cpp_define(cpp, "_Scanf_s_format_string_", "");

        /* SAL annotations — control flow */
        cpp_define(cpp, "_Always_(x)",     "");
        cpp_define(cpp, "_When_(x,y)",     "");
        cpp_define(cpp, "_At_(target, annos)", "");
        cpp_define(cpp, "_Analysis_assume_(x)", "");

        /* SAL annotations — struct fields */
        cpp_define(cpp, "_Field_size_(x)", "");
        cpp_define(cpp, "_Field_size_opt_(x)", "");
        cpp_define(cpp, "_Field_range_(x,y)", "");
        cpp_define(cpp, "_Struct_size_bytes_(x)", "");

        /* UCRT internal */
        cpp_define(cpp, "_ACRTIMP_NOALIAS", "");
        cpp_define(cpp, "_ACRTIMP_NONALIAS_RET", "");
        cpp_define(cpp, "_CRTRESTRICT",     "");
        cpp_define(cpp, "_CRT_HYBRIDPATCHABLE__", "");
        cpp_define(cpp, "_SECURECRT_FILL_BUFFER", "");
        cpp_define(cpp, "_CRT_JIT_ENTRY",   "");
    }
}

static char* read_file(const char* path);

/* Compile a C source file to an executable using libtcc. Returns 0 on success. */
static int compile_with_tcc(const char* c_file, const char* exe_file, const char* tcc_dir) {
    TccLib lib = {0};
    if (load_libtcc(&lib, tcc_dir) != 0) return -1;

    TCCState* s = lib.tcc_new();
    lib.tcc_set_error_func(s, NULL, tcc_error_cb);
    lib.tcc_set_lib_path(s, tcc_dir);

    if (lib.tcc_set_output_type(s, TCC_OUTPUT_EXE) != 0) {
        lib.tcc_delete(s);
        unload_libtcc(&lib);
        return -1;
    }

    /* Add TCC's own include and library paths */
    lib.tcc_add_include_path(s, tcc_dir);
    lib.tcc_add_include_path(s, tcc_dir);
    lib.tcc_add_sysinclude_path(s, tcc_dir);

    /* Add library path for linking */
    char lib_dir[512];
    snprintf(lib_dir, sizeof(lib_dir), "%s/lib", tcc_dir);
    lib.tcc_add_library_path(s, lib_dir);

    /* Compile the C source */
    char* c_src = read_file(c_file);
    if (!c_src) {
        lib.tcc_delete(s);
        unload_libtcc(&lib);
        return -1;
    }

    int rc = lib.tcc_compile_string(s, c_src);
    free(c_src);

    if (rc != 0) {
        lib.tcc_delete(s);
        unload_libtcc(&lib);
        return -1;
    }

    /* Output executable */
    rc = lib.tcc_output_file(s, exe_file);
    lib.tcc_delete(s);
    unload_libtcc(&lib);
    return rc;
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
    /* replace trailing .sp with .c, otherwise append .c */
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
        out[n - 2] = 0; /* strip .sp, bare executable */
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

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_path = argv[++i];
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
        fprintf(stderr, "usage: sharpc <file.sp> [-o out[.c|.exe]] [-dump-hir] [-no-link] [--target <triple>]\n");
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

    /* Compute TCC directory path for system headers */
    const char* tcc_dir_env = getenv("SHARPC_TCC_DIR");
    char tcc_dir_buf[512];
    const char* tcc_dir = NULL;
    if (tcc_dir_env) {
        tcc_dir = tcc_dir_env;
    } else {
#ifdef _WIN32
        char exe_dir[512];
        GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir));
        char* last_slash = strrchr(exe_dir, '\\');
        if (last_slash) *last_slash = 0;
        last_slash = strrchr(exe_dir, '\\');
        if (last_slash) *last_slash = 0;
        snprintf(tcc_dir_buf, sizeof(tcc_dir_buf), "%s\\third_party\\tcc", exe_dir);
#else
        snprintf(tcc_dir_buf, sizeof(tcc_dir_buf), "/usr/lib/tcc");
#endif
        tcc_dir = tcc_dir_buf;
    }

    /* ------------------------------------------------------------------ *
     * Preprocessing pass (translation phases 1-6)                        *
     * ------------------------------------------------------------------ */
    CppCtx *cpp_ctx = cpp_ctx_new();

    /* Forward -I / -D / -U flags from argv */
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

    /* Apply target triple: define macros */
    apply_target_macros(cpp_ctx, &target);

    /* System include paths — search order:
     * 1. User -I paths (added above)
     * 2. TCC headers (default — parser-friendly)
     * 3. MinGW fallback (Windows)
     * 4. /usr/include (Linux/macOS)
     *
     * Note: UCRT/MSVC paths are disabled by default because UCRT headers
     * contain complex SAL annotation patterns that our parser cannot fully
     * handle yet. The SAL macros are defined as empty in apply_target_macros()
     * for compatibility when users explicitly #include UCRT headers via -I. */

    /* TCC headers (parser-friendly, proven to work) */
    char tcc_inc[512];
    snprintf(tcc_inc, sizeof(tcc_inc), "%s/include", tcc_dir);
    cpp_add_sys_include(cpp_ctx, tcc_inc);

    /* MinGW fallback for additional Windows headers */
#ifdef _WIN32
    cpp_add_sys_include(cpp_ctx, "C:/msys64/mingw64/include");
    cpp_add_sys_include(cpp_ctx, "C:/msys64/ucrt64/include");
    cpp_add_sys_include(cpp_ctx, "C:/mingw64/include");
#else
    cpp_add_sys_include(cpp_ctx, "/usr/include");
    cpp_add_sys_include(cpp_ctx, "/usr/local/include");
#endif

    /* TCC specific */
    cpp_define(cpp_ctx, "__TINYC__", "1");

    /* Floating-point constants used by system headers (math.h, etc.) */
    cpp_define(cpp_ctx, "INFINITY",  "((float)(1e+37*1e+37))");
    cpp_define(cpp_ctx, "NAN",       "((float)(0.0f))");
    /* Emit line markers so error messages map back to original source lines.
     * The Sharp lexer (lexer.c:try_consume_linemarker) absorbs these silently. */
    CppResult pp = cpp_run(cpp_ctx, in_path, CPP_LANG_SHARP);
    if (pp.error) {
        cpp_print_diags(&pp);
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 1;
    }

    /* Debug: dump preprocessed output if SHARPC_DUMP_PP is set */
    if (getenv("SHARPC_DUMP_PP")) {
        FILE* pf = fopen("preprocessed.out", "w");
        if (pf) {
            fputs(pp.text, pf);
            fclose(pf);
            fprintf(stderr, "[debug] preprocessed output written to preprocessed.out\n");
        }
    }

    /* For diagnostic source display, read the original file so error context
     * lines match what the user typed rather than the preprocessed output.
     * If the read fails, fall back to the preprocessed text.               */
    char* orig_src = read_file(in_path);
    diag_set_source(in_path, orig_src ? orig_src : pp.text);

    char* src = pp.text;   /* NUL-terminated; owned by pp — fed to lexer   */

    Arena* arena = NULL;
    Lexer lx;
    lex_init(&lx, src, in_path);
    Node* prog = parse_program(&lx, &arena);

    /* Always run sema, even if parse reported errors: the AST is still
     * structurally valid (parser's error recovery guarantees that), and
     * sema may have additional diagnostics the user wants to see in the
     * same pass — duplicate destructors, malformed operator signatures,
     * impls for unknown structs, etc. We only bail BEFORE code generation,
     * because cgen assumes a well-formed AST. */
    SymTable* st = sema_build(prog, &arena);

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d error%s; aborting\n",
                g_error_count, g_error_count == 1 ? "" : "s");
        arena_free_all(&arena);
        free(orig_src);
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 1;
    }

    /* Leap C: AST lowering — inject explicit ND_DROP nodes for RAII so
     * cgen doesn't have to track scopes at emission time. Must run after
     * sema (needs SymTable for dtor lookup) and before cgen. */
    lower_program(prog, st, &arena);

    /* Leap D: Build the Control Flow Graph.
     * Runs after lower_program so every ND_MATCH / ND_PROPAGATE /
     * ND_RETURN is already in its final desugared form.
     *
     * The HIR is built in parallel with cgen — it does not replace the
     * existing code-generation path yet.  Two analysis passes run here:
     *
     *   hir_mark_reachable()  — BFS from each function's entry block.
     *   hir_check_returns()   — Diagnose non-void functions that may
     *                           fall off the end without returning.
     *
     * Additional passes (live-variable analysis, use-before-init, Result
     * typestate) will be added in Leap E. */
    HirProg* hir = hir_build(prog, st, &arena);
    hir_mark_reachable(hir);
    hir_check_returns(hir);

    if (dump_hir) hir_dump(hir, stderr);

    hir_free(hir);

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d error%s; aborting\n",
                g_error_count, g_error_count == 1 ? "" : "s");
        free(orig_src);
        arena_free_all(&arena);
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 1;
    }

    /* ------------------------------------------------------------------ *
     * Code generation + optional libtcc linking (zero disk I/O)          *
     * ------------------------------------------------------------------ */
    bool out_alloced = false;
    if (!out_path) { out_path = default_out_path(in_path); out_alloced = true; }
    bool should_link = (strcmp(out_path, "-") != 0) && !no_link;

    if (strcmp(out_path, "-") == 0) {
        cgen_c(prog, st, stdout);
    } else if (!should_link) {
        FILE* out = fopen(out_path, "wb");
        if (!out) {
            fprintf(stderr, "cannot write '%s'\n", out_path);
            free(orig_src);
            arena_free_all(&arena);
            return 1;
        }
        cgen_c(prog, st, out);
        fclose(out);
    } else {
        StrBuf cbuf = {0};
        cgen_buf(prog, st, &cbuf);

        if (g_error_count) {
            sb_free(&cbuf);
            fprintf(stderr, "sharpc: %d error%s; aborting\n",
                    g_error_count, g_error_count == 1 ? "" : "s");
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            return 1;
        }

        /* Determine TCC directory */
        const char* env_tcc = getenv("SHARP_TCC_DIR");
        char tcc_dir_buf[512];
        const char* tcc_dir;
        if (env_tcc) {
            tcc_dir = env_tcc;
        } else {
#ifdef _WIN32
            char exe_dir[512];
            GetModuleFileNameA(NULL, exe_dir, sizeof(exe_dir));
            char* last_slash = strrchr(exe_dir, '\\');
            if (last_slash) *last_slash = 0;
            char* last_slash2 = strrchr(exe_dir, '\\');
            if (last_slash2) *last_slash2 = 0;
            snprintf(tcc_dir_buf, sizeof(tcc_dir_buf), "%s\\third_party\\tcc", exe_dir);
#else
            snprintf(tcc_dir_buf, sizeof(tcc_dir_buf), "/usr/lib/tcc");
#endif
            tcc_dir = tcc_dir_buf;
        }

        /* Determine output executable name */
        const char* exe_path = NULL;
        bool exe_alloced = false;
        bool is_exe_output = false;
        if (out_path && strcmp(out_path, "-") != 0) {
            size_t ol = strlen(out_path);
            is_exe_output = (ol >= 4 && strcmp(out_path + ol - 4, ".exe") == 0) ||
                            (ol >= 4 && strcmp(out_path + ol - 4, ".out") == 0);
        }
        if (is_exe_output) {
            exe_path = out_path;
        } else {
            exe_path = default_exe_path(in_path);
            exe_alloced = true;
        }

        /* Compile from memory to binary */
        // fprintf(stderr, "sharpc: %s -> %s via libtcc (zero disk I/O)\n", in_path, exe_path);

        /* Use libtcc to compile C code from memory */
        TccLib lib = {0};
        if (load_libtcc(&lib, tcc_dir) != 0) {
            sb_free(&cbuf);
            fprintf(stderr, "sharpc: cannot load libtcc\n");
            if (exe_alloced) free((char*)exe_path);
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            return 1;
        }

        TCCState* s = lib.tcc_new();
        lib.tcc_set_error_func(s, NULL, tcc_error_cb);
        lib.tcc_set_lib_path(s, tcc_dir);
        lib.tcc_set_output_type(s, TCC_OUTPUT_EXE);
        char tcc_inc[512];
        snprintf(tcc_inc, sizeof(tcc_inc), "%s/include", tcc_dir);
        lib.tcc_add_include_path(s, tcc_inc);
        lib.tcc_add_sysinclude_path(s, tcc_inc);
        char lib_dir_path[512];
        snprintf(lib_dir_path, sizeof(lib_dir_path), "%s/lib", tcc_dir);
        lib.tcc_add_library_path(s, lib_dir_path);

        /* Debug: write cbuf to temp file for inspection */
        {
            FILE* dbg = fopen("debug_tcc_input.c", "wb");
            if (dbg) { fwrite(cbuf.data, 1, cbuf.len, dbg); fclose(dbg); }
        }

        int tcc_rc = lib.tcc_compile_string(s, cbuf.data);
        sb_free(&cbuf);

        if (tcc_rc != 0) {
            fprintf(stderr, "sharpc: libtcc compilation failed\n");
            lib.tcc_delete(s);
            unload_libtcc(&lib);
            remove(exe_path);
            if (exe_alloced) free((char*)exe_path);
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            cpp_result_free(&pp);
            cpp_ctx_free(cpp_ctx);
            return 1;
        }

        tcc_rc = lib.tcc_output_file(s, exe_path);
        lib.tcc_delete(s);
        unload_libtcc(&lib);

        if (tcc_rc != 0) {
            fprintf(stderr, "sharpc: libtcc link failed\n");
            remove(exe_path);
            if (exe_alloced) free((char*)exe_path);
            if (out_alloced) free((char*)out_path);
            free(orig_src);
            arena_free_all(&arena);
            cpp_result_free(&pp);
            cpp_ctx_free(cpp_ctx);
            return 1;
        }
        fprintf(stderr, "sharpc: %s built successfully\n", exe_path);
        if (exe_alloced) free((char*)exe_path);
    }

    if (g_error_count) {
        fprintf(stderr, "sharpc: %d error%s; aborting\n",
                g_error_count, g_error_count == 1 ? "" : "s");
        if (out_alloced) free((char*)out_path);
        free(orig_src);
        arena_free_all(&arena);
        cpp_result_free(&pp);
        cpp_ctx_free(cpp_ctx);
        return 1;
    }

    if (out_alloced)   free((char*)out_path);
    free(orig_src);
    arena_free_all(&arena);
    cpp_result_free(&pp);
    cpp_ctx_free(cpp_ctx);
    return 0;
}
