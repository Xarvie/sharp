/*
 * sharpc.c — Sharp compiler driver (clang-compatible CLI).
 *
 * Sharp is a superset of C (cf. C++ ⊃ C).  The same driver compiles
 * `.ce` Sharp source and `.c` C source — input is always parsed as
 * Sharp.  The driver can preprocess, compile, assemble, link, or
 * compile+link in one invocation — just like clang.
 *
 * Usage (clang-compatible):
 *   sharpc <input> [-o <output>] [options]          # compile + link
 *   sharpc -c <input> [-o <obj>]                    # compile to .o
 *   sharpc -S <input> [-o <asm>]                    # emit .s
 *   sharpc -E <input> [-o <pp>]                     # preprocess only
 *   sharpc file1.ce file2.ce -o app                  # multi-file link
 *
 *   sharpc -                              # read from stdin
 *
 * Exit codes:
 *   0 = success
 *   1 = compile errors in source
 *   2 = I/O error or bad command line
 *   3 = link failure
 */
#define _POSIX_C_SOURCE 200809L  /* strdup, readlink */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#ifndef _WIN32
  #include <unistd.h>
  #include <sys/wait.h>
  #include <pthread.h>
  #ifdef __APPLE__
    #include <mach-o/dyld.h>
  #endif
#else
  #include <windows.h>
  #include <io.h>
  #include <process.h>
  #include <direct.h>
#endif
#include <fcntl.h>
#include "cg.h"
#include "sema.h"
#include "parse.h"
#include "lex.h"
#include "cpp.h"
#include "cpp_sys_paths.h"
#include "sharp_internal.h"

/* ── Std library path: derived from sharpc executable location ────── */
/*
 * Finds {sharp_root}/std/ by locating the directory of the
 * sharpc executable and walking up to its project root.
 *
 * Resolution order (first found wins):
 *   1. <exe_dir>/std/               (exe is at repo root)
 *   2. <exe_dir>/../std/            (exe is in build/)
 *   3. SHARP_ROOT environment variable
 *
 * Returns a malloc'd string (caller frees), or NULL if not found.
 */
static char *sharp_find_std_dir(void) {
#ifdef _WIN32
    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
    if (len == 0 || len >= sizeof(exe_path)) return NULL;
#elif defined(__APPLE__)
    char exe_path[4096];
    uint32_t size = sizeof(exe_path);
    if (_NSGetExecutablePath(exe_path, &size) != 0) return NULL;
    char *rp = realpath(exe_path, NULL);
    if (!rp) return NULL;
    /* copy to stack since we need to free rp */
    size_t rplen = strlen(rp);
    if (rplen >= sizeof(exe_path)) { free(rp); return NULL; }
    memcpy(exe_path, rp, rplen + 1);
    free(rp);
#else
    char exe_path[4096];
    ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len <= 0 || (size_t)len >= sizeof(exe_path)) return NULL;
    exe_path[len] = '\0';
#endif

    /* Get the directory containing the executable */
    char exe_dir[4096];
    {
        /* Find last separator */
        const char *sep = NULL;
#ifdef _WIN32
        const char *bs = strrchr(exe_path, '\\');
        const char *fs = strrchr(exe_path, '/');
        sep = (bs > fs) ? bs : fs;
#else
        sep = strrchr(exe_path, '/');
#endif
        if (!sep) return NULL;
        size_t dirlen = (size_t)(sep - exe_path);
        if (dirlen >= sizeof(exe_dir)) return NULL;
        memcpy(exe_dir, exe_path, dirlen);
        exe_dir[dirlen] = '\0';
    }

    /* Candidate paths to try */
    const char *candidates[] = {
        "std",      /* <exe_dir>/std/ */
        "../std",   /* <exe_dir>/../std/ */
    };

    for (int i = 0; i < 2; i++) {
        char candidate[4096];
        int n = snprintf(candidate, sizeof(candidate),
                        "%s/%s", exe_dir, candidates[i]);
        if (n <= 0 || (size_t)n >= sizeof(candidate)) continue;
#ifdef _WIN32
        DWORD attr = GetFileAttributesA(candidate);
        if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
#else
        struct stat st;
        if (stat(candidate, &st) == 0 && S_ISDIR(st.st_mode)) {
#endif
            return strdup(candidate);
        }
    }

    /* SHARP_ROOT env var fallback */
    const char *env = getenv("SHARP_ROOT");
    if (env && env[0]) {
        char candidate[4096];
        int n = snprintf(candidate, sizeof(candidate), "%s/std", env);
        if (n > 0 && (size_t)n < sizeof(candidate)) {
            return strdup(candidate);
        }
    }

    return NULL;
}

/* ── Compilation action ────────────────────────────────────────────── */

typedef enum { ACTION_LINK, ACTION_COMPILE_ONLY, ACTION_ASSEMBLY, ACTION_PREPROCESS } Action;

/* ── Small string-pointer vector ─────────────────────────────────────── */

typedef struct {
    const char **data;
    size_t       len, cap;
} StrVec;

static int sv_push(StrVec *v, const char *s) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        const char **nd = realloc(v->data, nc * sizeof *nd);
        if (!nd) return -1;
        v->data = nd; v->cap = nc;
    }
    v->data[v->len++] = s;
    return 0;
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

/* ── Input file descriptor ────────────────────────────────────────── */

typedef struct {
    const char *path;          /* original path (or "-" for stdin) */
    char       *tmp_c;         /* generated .c text (NULL if not yet generated);
                                  for .S files this is preprocessed assembly */
    bool        tmp_is_asm;    /* true if tmp_c is assembly (.S), not C */
    bool        tmp_is_zig_direct; /* true if .cpp/.m/.mm — pass directly to zig cc */
    char      **included_files; /* files opened via #include (owned, freed on cleanup) */
    size_t      nincluded;
    bool       *included_is_sys; /* parallel to included_files, true = system header */
} InputFile;

typedef struct {
    InputFile *data;
    size_t     len, cap;
} InputVec;

static InputFile *iv_push(InputVec *v) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 4;
        InputFile *nd = realloc(v->data, nc * sizeof *nd);
        if (!nd) return NULL;
        v->data = nd; v->cap = nc;
    }
    v->data[v->len].path = NULL;
    v->data[v->len].tmp_c = NULL;
    v->data[v->len].tmp_is_asm = false;
    v->data[v->len].tmp_is_zig_direct = false;
    v->data[v->len].included_files = NULL;
    v->data[v->len].nincluded = 0;
    v->data[v->len].included_is_sys = NULL;
    return &v->data[v->len++];
}

/* ── Global session ─────────────────────────────────────────────── */

SharpSess g_sess = {0};

/* ── Temp file cleanup ────────────────────────────────────────────── */

static void register_tmp(const char *path) {
    if (!path) return;
    const char **newp = realloc(g_sess.tmp_files, (g_sess.n_tmp_files + 1) * sizeof *g_sess.tmp_files);
    if (!newp) return;
    g_sess.tmp_files = newp;
    g_sess.tmp_files[g_sess.n_tmp_files++] = path;
}

static void cleanup_tmp_files(void) {
    const char *keep_env = getenv("SHARPC_KEEP_TMP");
    for (size_t i = 0; i < g_sess.n_tmp_files; i++) {
        if (keep_env) {
            const char *src = g_sess.tmp_files[i];
            FILE *sf = fopen(src, "r");
            if (sf) {
                /* Find basename for the kept file */
                const char *bn = strrchr(src, '/');
                bn = bn ? bn + 1 : src;
                char keep_path[512];
                snprintf(keep_path, sizeof keep_path, "/tmp/SHARPC_KEPT_%s", bn);
                FILE *df = fopen(keep_path, "w");
                if (df) {
                    int ch;
                    while ((ch = fgetc(sf)) != EOF) fputc(ch, df);
                    fclose(df);
                    fprintf(stderr, "sharpc: kept -> %s\n", keep_path);
                }
                fclose(sf);
            }
        }
        unlink(g_sess.tmp_files[i]);
        free((void *)g_sess.tmp_files[i]);
    }
    free(g_sess.tmp_files);
    g_sess.tmp_files = NULL;
    g_sess.n_tmp_files = 0;
}

#ifndef _WIN32
static int fill_random_x(char *tmpl, int suffix_len) {
    const char letters[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    size_t nletters = sizeof letters - 1;
    size_t total    = strlen(tmpl);
    size_t x_start  = total - 6 - (size_t)suffix_len;
    int    fd_rnd   = open("/dev/urandom", O_RDONLY);
    if (fd_rnd < 0) return -1;
    for (int attempt = 0; attempt < 128; attempt++) {
        unsigned char rnd[6];
        if (read(fd_rnd, rnd, 6) != 6) break;
        for (int i = 0; i < 6; i++)
            tmpl[x_start + i] = letters[rnd[i] % nletters];
        int fd = open(tmpl, O_CREAT | O_EXCL | O_RDWR, 0600);
        if (fd >= 0) { close(fd_rnd); return fd; }
        if (errno != EEXIST) break;
    }
    close(fd_rnd);
    return -1;
}

static char *make_tmp_name(const char *prefix, const char *suffix) {
    size_t bufsz = strlen(prefix) + strlen(suffix) + 32;
    char *buf = malloc(bufsz);
    if (!buf) return NULL;
    const char *base = strrchr(prefix, '/');
    base = base ? base + 1 : prefix;
    snprintf(buf, bufsz, "/tmp/sharp_%s-XXXXXX%s", base, suffix);
    int fd = fill_random_x(buf, (int)strlen(suffix));
    if (fd < 0) { free(buf); return NULL; }
    close(fd);
    register_tmp(buf);
    return buf;
}
#else
static char *make_tmp_name(const char *prefix, const char *suffix) {
    const char *tmpdir = getenv("TMP");
    if (!tmpdir) tmpdir = getenv("TEMP");
    if (!tmpdir) tmpdir = ".";

    char *buf = malloc(strlen(tmpdir) + 64 + strlen(suffix));
    if (!buf) return NULL;

    static unsigned counter = 0;
    const char *base = strrchr(prefix, '\\');
    const char *base2 = strrchr(prefix, '/');
    base = (base2 && (!base || base2 > base)) ? base2 : base;
    base = base ? base + 1 : prefix;
    const char *dot = strrchr(base, '.');
    size_t baselen = dot ? (size_t)(dot - base) : strlen(base);
    if (baselen > 32) baselen = 32;

    snprintf(buf, strlen(tmpdir) + 64 + strlen(suffix),
             "%s\\sharp-%.*s-%u%s", tmpdir, (int)baselen, base, ++counter, suffix);
    register_tmp(buf);
    return buf;
}
#endif

/* ── Run external command (gcc/ld/zig cc) ────────────────────────────
 *
 * Windows: CreateProcess with stderr redirect to temp file.
 * Unix: fork/exec with stderr redirect to temp file.                */
static int run_cmd(const char **argv) {
    if (!argv || !argv[0]) return -1;

#ifdef _WIN32
    char err_path[512];
    const char *tmp = getenv("TEMP");
    if (!tmp) tmp = ".";
    snprintf(err_path, sizeof(err_path), "%s\\sharpc_err_%lu_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetCurrentThreadId());

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hErr = CreateFileA(err_path, GENERIC_WRITE, 0, &sa,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hErr == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "sharpc: cannot create temp error file %s\n", err_path);
        return -1;
    }

    STARTUPINFOA si = {0};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = hErr;

    if (!g_sess.verbose) {
        si.hStdOutput = hErr;
    }

    size_t cmdlen = 0;
    for (int i = 0; argv[i]; i++)
        cmdlen += strlen(argv[i]) * 2 + 4;
    char *cmdline = malloc(cmdlen + 1);
    if (!cmdline) { CloseHandle(hErr); return -1; }
    size_t pos = 0;
    for (int i = 0; argv[i]; i++) {
        if (i > 0) cmdline[pos++] = ' ';
        const char *a = argv[i];
        int needs_q = (a[0] == '\0' || strpbrk(a, " \t\n\"\\") != NULL);
        if (!needs_q) {
            size_t alen = strlen(a);
            memcpy(cmdline + pos, a, alen);
            pos += alen;
        } else {
            cmdline[pos++] = '"';
            for (const char *p = a; *p; p++) {
                if (*p == '"') {
                    cmdline[pos++] = '\\';
                    cmdline[pos++] = '"';
                } else if (*p == '\\') {
                    cmdline[pos++] = '\\';
                    cmdline[pos++] = '\\';
                } else {
                    cmdline[pos++] = *p;
                }
            }
            cmdline[pos++] = '"';
        }
    }
    cmdline[pos] = '\0';

    if (g_sess.verbose)
        fprintf(stderr, "[sharpc] %s\n", cmdline);

    PROCESS_INFORMATION pi = {0};
    BOOL ok = CreateProcessA(NULL, cmdline, NULL, NULL, TRUE,
                             0, NULL, NULL, &si, &pi);
    free(cmdline);
    CloseHandle(hErr);

    if (!ok) {
        fprintf(stderr, "sharpc: CreateProcess failed (error %lu)\n",
                GetLastError());
        remove(err_path);
        return -1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode != 0) {
        FILE *ef = fopen(err_path, "r");
        if (ef) {
            char buf[4096];
            while (fgets(buf, sizeof(buf), ef))
                fputs(buf, stderr);
            fclose(ef);
        }
    }
    remove(err_path);
    return (int)exitCode;

#else
    char err_path[512];
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    snprintf(err_path, sizeof(err_path), "%s/sharpc_err_%d_%lu.tmp",
             tmp, (int)getpid(), (unsigned long)pthread_self());

    size_t total = 0;
    for (int i = 0; argv[i]; i++)
        total += strlen(argv[i]) * 2 + 4;

    char *cmd = malloc(total);
    if (!cmd) return -1;
    size_t pos = 0;

    for (int i = 0; argv[i]; i++) {
        if (i > 0) cmd[pos++] = ' ';
        const char *a = argv[i];
        int needs_q = (a[0] == '\0' || strpbrk(a, " \t\n\"'$`\\*?[]{}|&;()<>#!~") != NULL);
        if (!needs_q) {
            size_t alen = strlen(a);
            if (pos + alen >= total) {
                total = (pos + alen) * 2 + 32;
                char *nc = realloc(cmd, total);
                if (!nc) { free(cmd); return -1; }
                cmd = nc;
            }
            memcpy(cmd + pos, a, alen);
            pos += alen;
        } else {
            if (pos + 1 >= total) {
                total = total * 2 + 32;
                char *nc = realloc(cmd, total);
                if (!nc) { free(cmd); return -1; }
                cmd = nc;
            }
            cmd[pos++] = '\'';
            for (const char *p = a; *p; p++) {
                if (*p == '\'') {
                    size_t need = 4;
                    if (pos + need >= total) {
                        total = (pos + need) * 2 + 32;
                        char *nc = realloc(cmd, total);
                        if (!nc) { free(cmd); return -1; }
                        cmd = nc;
                    }
                    cmd[pos++] = '\'';
                    cmd[pos++] = '\\';
                    cmd[pos++] = '\'';
                    cmd[pos++] = '\'';
                } else {
                    if (pos + 1 >= total) {
                        total = total * 2 + 32;
                        char *nc = realloc(cmd, total);
                        if (!nc) { free(cmd); return -1; }
                        cmd = nc;
                    }
                    cmd[pos++] = *p;
                }
            }
            if (pos + 1 >= total) {
                total = total * 2 + 32;
                char *nc = realloc(cmd, total);
                if (!nc) { free(cmd); return -1; }
                cmd = nc;
            }
            cmd[pos++] = '\'';
        }
    }
    cmd[pos] = '\0';

    char *full = malloc(pos + 512);
    if (!full) { free(cmd); return -1; }
    if (g_sess.verbose)
        snprintf(full, pos + 512, "%s 2> \"%s\"", cmd, err_path);
    else
        snprintf(full, pos + 512, "%s > /dev/null 2> \"%s\"", cmd, err_path);

    int rc = system(full);

    if (rc != 0) {
        FILE *ef = fopen(err_path, "r");
        if (ef) {
            char buf[4096];
            while (fgets(buf, sizeof(buf), ef))
                fputs(buf, stderr);
            fclose(ef);
        }
    }
    remove(err_path);
    free(full);
    free(cmd);
    return rc;
#endif
}

/* ── Zig cc invocation helpers ──────────────────────────────────────
 *
 * Most compilation actions involve constructing a zig cc command line
 * with common prefix (zig_exe, "cc", -target, --sysroot, link_other),
 * then appending mode-specific arguments and calling run_cmd.          */

/* Build the common zig cc prefix: zig_exe, "cc", -target, --sysroot,
 * and link_other flags.  Returns an StrVec that the caller extends. */
static StrVec zig_cc_prefix(const char *zig_exe, const char *target,
                             const char *sysroot, StrVec *link_other) {
    StrVec a = {0};
    sv_push(&a, zig_exe);
    sv_push(&a, "cc");
    if (target && target[0]) { sv_push(&a, "-target"); sv_push(&a, (char *)target); }
    if (sysroot && sysroot[0]) { sv_push(&a, "--sysroot"); sv_push(&a, (char *)sysroot); }
    for (size_t i = 0; i < link_other->len; i++)
        sv_push(&a, link_other->data[i]);
    return a;
}

/* NULL-terminate, optionally print verbose output, run the command,
 * and free the args array.  Returns 0 on success, non-zero on failure. */
static int zig_cc_run(StrVec *args) {
    sv_push(args, NULL);
    if (g_sess.verbose) {
        fprintf(stderr, "[sharpc] ");
        for (int i = 0; args->data[i]; i++) fprintf(stderr, "%s ", args->data[i]);
        fprintf(stderr, "\n");
    }
    int rc = run_cmd(args->data);
    free(args->data);
    args->data = NULL;
    return rc;
}

/* ── File I/O ────────────────────────────────────────────────────────── */

static char *read_stdin(void) {
    size_t cap = 4096, len = 0;
    char *buf = malloc(cap);
    if (!buf) return NULL;
    size_t n;
    while ((n = fread(buf + len, 1, cap - len, stdin)) > 0) {
        len += n;
        if (len == cap) {
            cap *= 2;
            char *nb = realloc(buf, cap);
            if (!nb) { free(buf); return NULL; }
            buf = nb;
        }
    }
    buf[len] = '\0';
    return buf;
}

static char *read_file(const char *path) {
    if (strcmp(path, "-") == 0) return read_stdin();
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f); rewind(f);
    if (sz < 0) { fclose(f); return NULL; }
    char *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    buf[rd] = '\0';
    fclose(f);
    return buf;
}

/* ── Extension helpers ─────────────────────────────────────────────── */

static const char *file_ext(const char *p) {
    const char *dot = strrchr(p, '.');
    return dot ? dot : "";
}

/* File types that skip sharp preprocessing/compilation and are passed
 * directly to zig cc (assembler / linker). */
static bool is_linkable_input(const char *path) {
    const char *ext = file_ext(path);
    return strcmp(ext, ".o")  == 0 || strcmp(ext, ".obj") == 0 ||
           strcmp(ext, ".a")  == 0 || strcmp(ext, ".so")  == 0 ||
           strcmp(ext, ".s")  == 0;
}

static bool is_dot_S(const char *path) {
    return strcmp(file_ext(path), ".S") == 0;
}

static bool is_zig_forward_source(const char *path) {
    const char *ext = file_ext(path);
    return strcmp(ext, ".cpp") == 0 ||
           strcmp(ext, ".cxx") == 0 ||
           strcmp(ext, ".cc")  == 0 ||
           strcmp(ext, ".m")   == 0 ||
           strcmp(ext, ".mm")  == 0;
}

static char *replace_ext(const char *path, const char *new_ext) {
    /* e.g. /a/b/foo.ce → /a/b/foo.o */
    const char *dot = strrchr(path, '.');
    if (!dot) {
        size_t rsz = strlen(path) + strlen(new_ext) + 1;
        char *r = malloc(rsz);
        if (!r) return NULL;
        snprintf(r, rsz, "%s%s", path, new_ext);
        return r;
    }
    size_t base_len = (size_t)(dot - path);
    char *r = malloc(base_len + strlen(new_ext) + 1);
    if (!r) return NULL;
    memcpy(r, path, base_len);
    strcpy(r + base_len, new_ext);
    return r;
}

/* ── -D / -U specs (in command-line order) ───────────────────────────── */

typedef struct {
    /* spec for -D is "NAME" or "NAME=VAL"; for -U it is just "NAME". */
    const char *spec;
    int         is_undef;   /* 0 = -D, 1 = -U */
} MacroSpec;

typedef struct {
    MacroSpec *data;
    size_t     len, cap;
} MacroVec;

static int mv_push(MacroVec *v, const char *spec, int is_undef) {
    if (v->len == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 8;
        MacroSpec *nd = realloc(v->data, nc * sizeof *nd);
        if (!nd) return -1;
        v->data = nd; v->cap = nc;
    }
    v->data[v->len].spec     = spec;
    v->data[v->len].is_undef = is_undef;
    v->len++;
    return 0;
}

/* Apply one -D spec to ctx.  spec is "NAME" or "NAME=VAL". */
static int apply_define(CppCtx *ctx, const char *spec) {
    char *dup = xstrdup(spec);
    if (!dup) return -1;
    char *eq  = strchr(dup, '=');
    if (eq) { *eq = '\0'; cpp_define(ctx, dup, eq + 1); }
    else      cpp_define(ctx, dup, "1");
    free(dup);
    return 0;
}

/* ── Argument parsing ────────────────────────────────────────────────── */

/* Recognise flags that the driver should accept and truly ignore
 * (they affect diagnostics or C-standard mode selection that sharp
 * does not implement, and would be meaningless / harmful to pass
 * through to the backend).  Code-generation flags (-m, -f, -W) are
 * NOT ignored — they are passed through to zig in the main loop.
 * Returns: 0 = not ignored, 1 = skip just this flag. */
static int is_ignored_flag(const char *a) {
    if (strcmp(a, "-pedantic")        == 0) return 1;
    if (strcmp(a, "-pedantic-errors") == 0) return 1;
    if (strcmp(a, "-ansi")            == 0) return 1;
    return 0;
}

static void usage(FILE *out) {
    fputs(
"Usage: sharpc <input> [options]                      compile + link (default)\n"
"       sharpc -c <input> [options]                   compile to object file (.o)\n"
"       sharpc -S <input> [options]                   compile to assembly (.s)\n"
"       sharpc -E <input> [options]                   preprocess only\n"
"\n"
"Input files:\n"
"  <file>.ce/.c/.i     source files (compiled via Sharp frontend)\n"
"  <file>.cpp/.cxx/.cc  C++ source files (passed directly to zig cc)\n"
"  <file>.m/.mm         Objective-C / Objective-C++ (passed to zig cc)\n"
"  <file>.S             assembly with preprocessor (cpp + zig cc)\n"
"  <file>.o/.obj        pre-built objects (passed to linker)\n"
"  <file>.a/.so         static / shared libraries (passed to linker)\n"
"  <file>.s             assembly without preprocessor (passed to zig cc)\n"
"  -                   read from stdin\n"
"\n"
"Output:\n"
"  -o <file>           output file (auto-detects .exe/.o/.s based on action)\n"
"\n"
"Preprocessor (gcc-compatible):\n"
"  -I <dir>            add user #include search path\n"
"  -isystem <dir>      add system #include search path\n"
"  -include <file>     include file before processing the primary source\n"
"  -D <NAME>[=V]       define macro\n"
"  -U <NAME>           undefine macro\n"
"  -P                  suppress linemarkers\n"
"  -std=c11            language standard (C11 only)\n"
"\n"
"Compilation control:\n"
"  -E                  preprocess only; do not compile, assemble or link\n"
"  -S                  compile to assembly; do not assemble or link\n"
"  -c                  compile to object; do not link\n"
"  -g                  generate debug info (passed to system compiler)\n"
"  -O<n>               optimisation level (passed to system compiler)\n"
"\n"
"Linker:\n"
"  -l<lib>             link with library (e.g. -lm)\n"
"  -L<dir>             add library search path\n"
"  -shared             produce shared library\n"
"  -static             static linking\n"
"\n"
"Code generation (passed through to system compiler):\n"
"  -m<flag>            machine-specific flags (e.g. -msse4.2, -mavx2)\n"
"  -m<key>=<val>       machine param (e.g. -march=native, -mtune=generic)\n"
"  -f<flag>            code-gen flags (e.g. -fPIC, -fsanitize=address)\n"
"  -W<flag>            warning flags (e.g. -Wall, -Wextra)\n"
"  -pthread            POSIX threads support\n"
"  -pipe               use pipes between compilation stages\n"
"\n"
"Misc:\n"
"  --target <triple>   install target macros (default: host triple)\n"
"  -v                  verbose: print commands\n"
"  --help / -h         show this help\n", out);
}

/* ── Preprocess one input file to text (true -E mode) ───────────────── */
/* ── Raw-string preprocessor (Rust style) ──────────────────────────────
 *
 * Converts r#"..."# raw strings to standard C string literals before
 * feeding to the C preprocessor.
 *
 * Syntax: r#"content"#          (1 hash)
 *         r##"content"##        (2 hashes)
 *         r###"content"###      (3 hashes) — up to 255 hashes
 *
 * content may contain any character except " followed by the same
 * number of # that opened the string.
 * Backslashes and quotes are preserved verbatim.
 *
 * The output is a standard C string: "line1\nline2\n..." where
 * internal \n becomes \\n, " becomes \", \ becomes \\.
 * ==================================================================== */

static char *convert_raw_strings(const char *src, const char *input) {
    size_t out_cap = strlen(src) + 256;
    char *out = malloc(out_cap);
    if (!out) return NULL;
    size_t out_len = 0;

#define NEED(n) do { \
    if (out_len + (n) >= out_cap) { \
        out_cap = (out_len + (n)) * 2; \
        out = realloc(out, out_cap); \
        if (!out) return NULL; \
    } \
} while (0)

#define EMIT1(c) do { NEED(1); out[out_len++] = (c); } while (0)

#define EMIT_ESCAPED(start, len) do { \
    NEED((len) * 2 + 4); \
    out[out_len++] = '"'; \
    for (size_t _i = 0; _i < (len); _i++) { \
        unsigned char _c = (unsigned char)(start)[_i]; \
        if (_c == '\\')      { out[out_len++] = '\\'; out[out_len++] = '\\'; } \
        else if (_c == '"')  { out[out_len++] = '\\'; out[out_len++] = '"';  } \
        else if (_c == '\n') { out[out_len++] = '\\'; out[out_len++] = 'n';  } \
        else if (_c == '\t') { out[out_len++] = '\\'; out[out_len++] = 't';  } \
        else if (_c == '\r') { out[out_len++] = '\\'; out[out_len++] = 'r';  } \
        else out[out_len++] = _c; \
    } \
    out[out_len++] = '"'; \
} while (0)

    const char *p = src;
    while (*p) {
        if (*p == '"') {
            EMIT1(*p++);
            while (*p && *p != '"') {
                if (*p == '\\' && p[1]) { EMIT1(*p++); }
                EMIT1(*p++);
            }
            if (*p == '"') EMIT1(*p++);
            continue;
        }
        if (*p == '\'') {
            EMIT1(*p++);
            while (*p && *p != '\'') {
                if (*p == '\\' && p[1]) { EMIT1(*p++); }
                EMIT1(*p++);
            }
            if (*p == '\'') EMIT1(*p++);
            continue;
        }
        if (p[0] == '/' && p[1] == '/') {
            EMIT1(*p++); EMIT1(*p++);
            while (*p && *p != '\n') EMIT1(*p++);
            continue;
        }
        if (p[0] == '/' && p[1] == '*') {
            EMIT1(*p++); EMIT1(*p++);
            while (*p) {
                if (p[0] == '*' && p[1] == '/') { EMIT1(*p++); EMIT1(*p++); break; }
                EMIT1(*p++);
            }
            continue;
        }

        int at_boundary = (p == src) || !(((p[-1] >= 'a' && p[-1] <= 'z') || (p[-1] >= 'A' && p[-1] <= 'Z') ||
                                           (p[-1] >= '0' && p[-1] <= '9') || p[-1] == '_'));
        if (at_boundary && p[0] == 'r' && p[1] == '"') {
            const char *after_q = p + 2;
            const char *close = strchr(after_q, '"');
            if (close) {
                EMIT_ESCAPED(after_q, (size_t)(close - after_q));
                p = close + 1;
                continue;
            }
        }
        if (at_boundary && p[0] == 'r' && p[1] == '#') {
            const char *h = p + 1;
            while (*h == '#') h++;
            size_t nhash = (size_t)(h - p - 1);
            if (*h == '"') {
                const char *content_start = h + 1;
                const char *cs = content_start;
                const char *close = NULL;
                while (*cs) {
                    if (*cs == '"') {
                        size_t nh = 0;
                        while (cs[1 + nh] == '#') nh++;
                        if (nh == nhash && cs[1 + nh] != '#') {
                            close = cs;
                            break;
                        }
                    }
                    cs++;
                }
                if (close) {
                    EMIT_ESCAPED(content_start, (size_t)(close - content_start));
                    p = close + 1 + nhash;
                    continue;
                }
            }
        }
        EMIT1(*p++);
    }
#undef NEED
#undef EMIT1
#undef EMIT_ESCAPED
    if (out_len >= out_cap) { out = realloc(out, out_len + 1); if (!out) return NULL; }
    out[out_len] = '\0';
    return out;
}

/* Prepend "#include \"file\"\n" for each force-include, then append src.
 * The first file gets a linemarker pointing back to the original input. */
static char *prepend_force_includes(const char *input, char *src,
                                    StrVec *force_includes) {
    size_t total_extra = 0;
    for (size_t i = 0; i < force_includes->len; i++)
        total_extra += 11 + strlen(force_includes->data[i]); /* #include "f"\n */
    /* linemarker: # 1 "input" 1\n = 17 chars approx (max path len) */
    size_t lm_extra = 0;
    if (force_includes->len > 0)
        lm_extra = 6 + strlen(input) + 4; /* # 1 "input"\n */
    size_t src_len = strlen(src);
    char *buf = malloc(lm_extra + total_extra + src_len + 1);
    if (!buf) { free(src); return NULL; }
    char *p = buf;
    /* Emit linemarker so diagnostics from injected includes report
     * the original file. */
    p += sprintf(p, "# 1 \"%s\"\n", input);
    for (size_t i = 0; i < force_includes->len; i++)
        p += sprintf(p, "#include \"%s\"\n", force_includes->data[i]);
    memcpy(p, src, src_len + 1);
    free(src);
    return buf;
}

/* ── Shared preprocessor helpers ───────────────────────────────────── */

/* Set up a CppCtx with all common flags: target macros, standard,
 * include paths, and -D/-U macros.  Returns a fully configured ctx,
 * or NULL on OOM (from -D apply_define). */
static CppCtx *setup_cpp_context(const char *target, long lang_std,
                                  StrVec *user_inc, StrVec *sys_inc,
                                  MacroVec *macros, bool linemarkers) {
    CppCtx *cctx = cpp_ctx_new();
    cpp_probe_zig_macros(cctx, target);
    cpp_emit_linemarkers(cctx, linemarkers);
    if (lang_std >= 0) cpp_set_lang_std(cctx, lang_std);

    for (size_t i = 0; i < user_inc->len; i++)
        cpp_add_user_include(cctx, user_inc->data[i]);
    for (size_t i = 0; i < sys_inc->len; i++)
        cpp_add_sys_include(cctx, sys_inc->data[i]);

    for (size_t i = 0; i < macros->len; i++) {
        if (macros->data[i].is_undef)
            cpp_undefine(cctx, macros->data[i].spec);
        else if (apply_define(cctx, macros->data[i].spec) < 0) {
            cpp_ctx_free(cctx); return NULL;
        }
    }
    return cctx;
}

/* Print all diagnostics from a CppResult, returning true if any
 * error or fatal diagnostic was present. */
static bool print_cpp_diags(CppResult *r, const char *filename) {
    bool had_error = false;
    for (size_t i = 0; i < r->ndiags; i++) {
        CppDiag *d = &r->diags[i];
        if (d->level == CPP_DIAG_NOTE) continue;
        const char *fname = d->loc.file ? d->loc.file : filename;
        fprintf(stderr, "%s:%u:%u: %s: %s\n",
                fname, d->loc.line, d->loc.col,
                d->level == CPP_DIAG_FATAL   ? "fatal" :
                d->level == CPP_DIAG_ERROR   ? "error" :
                d->level == CPP_DIAG_WARNING ? "warning" : "note",
                d->msg ? d->msg : "");
        if (d->level == CPP_DIAG_ERROR || d->level == CPP_DIAG_FATAL)
            had_error = true;
    }
    return had_error;
}

/* Transfer ownership of included_files from CppResult to InputFile.
 * The result's pointers are set to NULL so cpp_result_free won't free them. */
static void transfer_included_files(CppResult *r, InputFile *inf) {
    if (!inf) return;
    inf->included_files = r->included_files;
    inf->nincluded = r->nincluded;
    inf->included_is_sys = r->included_is_sys;
    r->included_files = NULL;
    r->nincluded = 0;
    r->included_is_sys = NULL;
}

/* Run sharp's C preprocessor on src with all flags applied.
 * Does NOT free src — the caller owns it.
 * Returns preprocessed text (caller frees), or NULL on error. */
static char *preprocess_one_file(const char *input,
                                 StrVec *user_inc, StrVec *sys_inc,
                                 MacroVec *macros, long lang_std,
                                 const char *target,
                                 InputFile *inf,
                                 StrVec *force_includes,
                                 bool convert_raw,
                                 bool linemarkers) {
    char *src = read_file(input);
    if (!src) return NULL;

    if (convert_raw) {
        char *src2 = convert_raw_strings(src, input);
        free(src);
        if (!src2) return NULL;
        src = src2;
    }

    if (force_includes && force_includes->len > 0) {
        src = prepend_force_includes(input, src, force_includes);
        if (!src) return NULL;
    }

    CppCtx *cctx = setup_cpp_context(target, lang_std, user_inc, sys_inc,
                                      macros, linemarkers);
    if (!cctx) { free(src); return NULL; }

    CppResult r = cpp_run_buf(cctx, src, strlen(src), input);
    free(src);

    bool cpp_had_error = print_cpp_diags(&r, input);
    if (cpp_had_error) {
        transfer_included_files(&r, inf);
        cpp_result_free(&r);
        cpp_ctx_free(cctx);
        return NULL;
    }

    char *out = malloc(r.text_len + 1);
    if (!out) {
        transfer_included_files(&r, inf);
        cpp_result_free(&r); cpp_ctx_free(cctx); return NULL;
    }
    if (r.text_len) memcpy(out, r.text, r.text_len);
    out[r.text_len] = '\0';
    transfer_included_files(&r, inf);
    cpp_result_free(&r);
    cpp_ctx_free(cctx);
    return out;
}

/* ── Compile one input file to C text via the Sharp frontend ────────── */

static char *compile_one_file(const char *input,
                              StrVec *user_inc, StrVec *sys_inc,
                              MacroVec *macros, long lang_std,
                              const char *target,
                              const char *output,
                              InputFile *inf,
                              StrVec *force_includes) {
    char *src = read_file(input);
    if (!src) return NULL;

    /* Prepend -include files BEFORE raw-string conversion so that
     * #include directives in -include files also get processed. */
    if (force_includes && force_includes->len > 0) {
        src = prepend_force_includes(input, src, force_includes);
        if (!src) return NULL;
    }

    /* Convert r#"..."# raw strings to standard C string literals
     * BEFORE the preprocessor runs.  Only for Sharp source files. */
    const char *input_ext = strrchr(input, '.');
    int is_sharp_input = input_ext && (strcmp(input_ext, ".ce") == 0 ||
                                        strcmp(input_ext, ".he") == 0);
    if (is_sharp_input) {
        char *src2 = convert_raw_strings(src, input);
        free(src);
        if (!src2) return NULL;
        src = src2;
    }

    CppCtx *cctx = setup_cpp_context(target, lang_std, user_inc, sys_inc,
                                      macros, true);
    if (!cctx) { free(src); return NULL; }

    CppResult r = cpp_run_buf(cctx, src, strlen(src), input);
    free(src);

    /* Check cpp diagnostics */
    bool cpp_had_error = print_cpp_diags(&r, input);
    if (cpp_had_error) {
        transfer_included_files(&r, inf);
        cpp_result_free(&r);
        cpp_ctx_free(cctx);
        return NULL;
    }

    FeDiagArr ld = {0}, pd = {0}, sd = {0}, sema_d = {0};

    size_t ntoks = 0;
    SharpTok *toks = lex_run(r.tokens, r.ntokens, input, &ld, &ntoks);

    AstNode *ast = parse_file_with_typedefs(toks, ntoks, input, &pd,
                                            NULL);

    Scope *scope = scope_build_with_prelude(ast, &sd, NULL);

    /* Type analysis + semantic check */
    TyStore *ts   = ty_store_new();
    SemaCtx *sema = sema_ctx_new(ts, scope, &sema_d);
    sema_check_file(sema, ast);

    /* Collect all diagnostics */
    FeDiagArr *all_diag[] = { &ld, &pd, &sd, &sema_d };
    bool had_error = false;
    for (int a = 0; a < 4; a++) {
        for (size_t ii = 0; ii < all_diag[a]->len; ii++) {
            CppDiag *d = &all_diag[a]->data[ii];
            if (d->level == CPP_DIAG_NOTE) continue;
            const char *fname = d->loc.file ? d->loc.file : input;
            fprintf(stderr, "%s:%u:%u: %s: %s\n",
                    fname, d->loc.line, d->loc.col,
                    d->level == CPP_DIAG_ERROR ? "error" : "warning",
                    d->msg);
            if (d->level == CPP_DIAG_ERROR) had_error = true;
        }
    }

    char *c_out = NULL;
    if (!had_error) {
        CgCtx *cg = cg_ctx_new(ts, scope);
        c_out = cg_generate(cg, ast);
        cg_ctx_free(cg);
    }

    /* Cleanup */
    sema_ctx_free(sema); ty_store_free(ts);
    scope_free_chain(scope); ast_node_free(ast); lex_free(toks);
    transfer_included_files(&r, inf);
    cpp_result_free(&r);
    for (int a = 0; a < 4; a++) {
        for (size_t ii = 0; ii < all_diag[a]->len; ii++) free(all_diag[a]->data[ii].msg);
        free(all_diag[a]->data);
    }
    cpp_ctx_free(cctx);

    return c_out;
}

/* ── Temp file writing helper ─────────────────────────────────────── */

static char *write_tmp_file(const char *prefix, const char *suffix,
                             const char *content) {
    char *path = make_tmp_name(prefix, suffix);
    if (!path) return NULL;
    FILE *f = fopen(path, "w");
    if (!f) { perror(path); free(path); return NULL; }
    fputs(content, f);
    fclose(f);
    return path;
}

/* ── Compile one InputFile to a .o via zig cc ──────────────────────
 *
 * Handles all input types: Sharp-compiled C text (.i), preprocessed
 * assembly (.S → .s), and zig-forward sources (.cpp, .m, etc.).
 *
 * Returns: malloc'd path to the .o file (caller frees), or NULL on error.
 * On error, an appropriate message is printed to stderr.              */
static char *compile_input_to_obj(const char *zig_exe, const char *target,
                                   const char *sysroot, StrVec *link_other,
                                   StrVec *user_inc, StrVec *sys_inc,
                                   InputFile *inf) {
    const char *obj_path = make_tmp_name(inf->path, ".o");
    if (!obj_path) return NULL;

    if (inf->tmp_is_zig_direct) {
        StrVec args = zig_cc_prefix(zig_exe, target, sysroot, link_other);
        for (size_t i = 0; i < user_inc->len; i++) {
            sv_push(&args, "-I");
            sv_push(&args, user_inc->data[i]);
        }
        for (size_t i = 0; i < sys_inc->len; i++) {
            sv_push(&args, "-isystem");
            sv_push(&args, sys_inc->data[i]);
        }
        sv_push(&args, "-c");
        sv_push(&args, inf->tmp_c);
        sv_push(&args, "-o");
        sv_push(&args, obj_path);
        if (zig_cc_run(&args) != 0) {
            fprintf(stderr, "sharpc: compilation failed for %s\n", inf->path);
            return NULL;
        }
        return (char *)obj_path;
    }

    if (inf->tmp_is_asm) {
        char *tmp_s = write_tmp_file(inf->path, ".s", inf->tmp_c);
        if (!tmp_s) return NULL;

        StrVec args = zig_cc_prefix(zig_exe, target, sysroot, link_other);
        sv_push(&args, "-c");
        sv_push(&args, tmp_s);
        sv_push(&args, "-o");
        sv_push(&args, obj_path);
        if (zig_cc_run(&args) != 0) {
            fprintf(stderr, "sharpc: assembly failed for %s\n", inf->path);
            return NULL;
        }
        return (char *)obj_path;
    }

    /* Normal Sharp-compiled C text */
    char *tmp_i = write_tmp_file(inf->path, ".i", inf->tmp_c);
    if (!tmp_i) return NULL;

    StrVec args = zig_cc_prefix(zig_exe, target, sysroot, link_other);
    sv_push(&args, "-x");
    sv_push(&args, "c");
    sv_push(&args, "-c");
    sv_push(&args, tmp_i);
    sv_push(&args, "-o");
    sv_push(&args, obj_path);
    if (zig_cc_run(&args) != 0) {
        fprintf(stderr, "sharpc: compilation failed for %s\n", inf->path);
        return NULL;
    }
    return (char *)obj_path;
}

/* ── Session cleanup (shared by cleanup: and oom: paths) ─────────── */

static void cleanup_session(StrVec *user_inc, StrVec *sys_inc,
                             MacroVec *macros, StrVec *link_libs,
                             StrVec *link_paths, StrVec *link_other,
                             StrVec *force_includes, InputVec *inputs) {
    for (size_t i = 0; i < user_inc->len; i++) free((void *)user_inc->data[i]);
    free(user_inc->data);
    for (size_t i = 0; i < sys_inc->len; i++) free((void *)sys_inc->data[i]);
    free(sys_inc->data);
    free(macros->data);
    free(link_libs->data);
    free(link_paths->data);
    free(link_other->data);
    for (size_t i = 0; i < force_includes->len; i++) free((void *)force_includes->data[i]);
    free(force_includes->data);
    for (size_t i = 0; i < inputs->len; i++) {
        free(inputs->data[i].tmp_c);
        if (inputs->data[i].included_files) {
            for (size_t j = 0; j < inputs->data[i].nincluded; j++)
                free(inputs->data[i].included_files[j]);
            free(inputs->data[i].included_files);
        }
        free(inputs->data[i].included_is_sys);
    }
    free(inputs->data);
    cleanup_tmp_files();
}

/* ── Main driver ──────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    Action      action          = ACTION_LINK;
    const char *output          = NULL;
    const char *optimize        = NULL;
    int         gen_deps        = 0;
    int         gen_deps_no_sys = 0;   /* -MMD: exclude system headers */
    const char *depfile         = NULL;
    const char *dep_target      = NULL; /* -MT <target> */
#if defined(_WIN32)
    const char *target      = "x86_64-windows-gnu";
#elif defined(__APPLE__)
    const char *target      = "aarch64-macos";
#else
    const char *target      = "x86_64-linux-gnu";
#endif
    const char *sysroot        = NULL;
    StrVec      user_inc    = {0};
    StrVec      sys_inc     = {0};
    MacroVec    macros      = {0};
    StrVec      link_libs   = {0};   /* -l<lib> */
    StrVec      link_paths  = {0};   /* -L<dir> */
    StrVec      link_other  = {0};   /* extra linker flags (-shared, -static, etc.) */
    StrVec      force_includes = {0}; /* -include <file> */
    long        lang_std    = -1;
    InputVec    inputs      = {0};
    int         ret         = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];

        /* ── Action flags ────────────────────────────────────────── */
        if (strcmp(a, "-E") == 0) {
            action = ACTION_PREPROCESS;
        } else if (strcmp(a, "-S") == 0) {
            action = ACTION_ASSEMBLY;
        } else if (strcmp(a, "-c") == 0) {
            action = ACTION_COMPILE_ONLY;
        } else if (strcmp(a, "-o") == 0 && i + 1 < argc) {
            output = argv[++i];

        /* ── Help / version / verbose ──────────────────────────────── */
        } else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            usage(stdout); goto cleanup;
        } else if (strcmp(a, "--version") == 0) {
            fprintf(stdout, "sharpc 1.0.0\n");
            goto cleanup;
        } else if (strcmp(a, "-v") == 0) {
            g_sess.verbose = 1;

        /* ── Target ──────────────────────────────────────────────── */
        } else if (strcmp(a, "--target") == 0 && i + 1 < argc) {
            target = argv[++i];

        /* ── --sysroot=<path> ───────────────────────────────────── */
        } else if (strcmp(a, "--sysroot") == 0 && i + 1 < argc) {
            sysroot = argv[++i];
        } else if (strncmp(a, "--sysroot=", 10) == 0) {
            sysroot = a + 10;

        /* ── --optimize=<mode> ──────────────────────────────────── */
        } else if (strcmp(a, "--optimize") == 0 && i + 1 < argc) {
            optimize = argv[++i];
        } else if (strncmp(a, "--optimize=", 11) == 0) {
            optimize = a + 11;

        /* ── -I ──────────────────────────────────────────────────── */
        } else if (strcmp(a, "-I") == 0 && i + 1 < argc) {
            char *d = xstrdup(argv[++i]); if (!d) goto oom;
            if (sv_push(&user_inc, d) < 0) { free(d); goto oom; }
        } else if (strncmp(a, "-I", 2) == 0 && a[2]) {
            char *d = xstrdup(a + 2); if (!d) goto oom;
            if (sv_push(&user_inc, d) < 0) { free(d); goto oom; }

        /* ── -isystem ────────────────────────────────────────────── */
        } else if (strcmp(a, "-isystem") == 0 && i + 1 < argc) {
            char *d = xstrdup(argv[++i]); if (!d) goto oom;
            if (sv_push(&sys_inc, d) < 0) { free(d); goto oom; }
        } else if (strncmp(a, "-isystem", 8) == 0 && a[8]) {
            char *d = xstrdup(a + 8); if (!d) goto oom;
            if (sv_push(&sys_inc, d) < 0) { free(d); goto oom; }

        /* ── -include ────────────────────────────────────────────── */
        } else if (strcmp(a, "-include") == 0 && i + 1 < argc) {
            char *d = xstrdup(argv[++i]); if (!d) goto oom;
            if (sv_push(&force_includes, d) < 0) { free(d); goto oom; }

        /* ── -D / -U ─────────────────────────────────────────────── */
        } else if (strcmp(a, "-D") == 0 && i + 1 < argc) {
            if (mv_push(&macros, argv[++i], 0) < 0) goto oom;
        } else if (strncmp(a, "-D", 2) == 0 && a[2]) {
            if (mv_push(&macros, a + 2, 0) < 0) goto oom;
        } else if (strcmp(a, "-U") == 0 && i + 1 < argc) {
            if (mv_push(&macros, argv[++i], 1) < 0) goto oom;
        } else if (strncmp(a, "-U", 2) == 0 && a[2]) {
            if (mv_push(&macros, a + 2, 1) < 0) goto oom;

        /* ── -P (suppress linemarkers) ───────────────────────────── */
        } else if (strcmp(a, "-P") == 0) {
            /* sharpc always emits -P-style output; no-op */

        /* ── Dependency generation ────────────────────────────────── */
        } else if (strcmp(a, "-MD") == 0) {
            gen_deps = 1;
        } else if (strcmp(a, "-MMD") == 0) {
            gen_deps = 1;
            gen_deps_no_sys = 1;
        } else if (strcmp(a, "-MF") == 0 && i + 1 < argc) {
            depfile = argv[++i];
        } else if (strcmp(a, "-MT") == 0 && i + 1 < argc) {
            dep_target = argv[++i];

        /* ── -std=... ────────────────────────────────────────────── */
        } else if (strncmp(a, "-std=", 5) == 0) {
            const char *s = a + 5;
            if (strcmp(s, "c11") == 0 || strcmp(s, "gnu11") == 0 ||
                strcmp(s, "iso9899:2011") == 0)
                lang_std = 201112L;
            else
                fprintf(stderr, "sharpc: only -std=c11 is supported, ignoring -std=%s\n", s);

        /* ── Linker flags (-l, -L, -shared, -static) ─────────────── */
        } else if (strncmp(a, "-l", 2) == 0 && a[2]) {
            sv_push(&link_libs, a);
        } else if (strncmp(a, "-L", 2) == 0 && a[2]) {
            sv_push(&link_paths, a);
        } else if (strcmp(a, "-shared") == 0 || strcmp(a, "-static") == 0) {
            sv_push(&link_other, a);
        } else if (strcmp(a, "-g") == 0) {
            sv_push(&link_other, a);
            sv_push(&link_other, "-gdwarf-4");  /* DWARF debug info for GDB compatibility */
            sv_push(&link_other, "-O0");  /* disable optimization for debug builds */
        } else if (strncmp(a, "-O", 2) == 0) {
            sv_push(&link_other, a);

        /* ── Code-generation flags → pass through to zig ────────────
         * -m<flag>   machine-specific (e.g. -msse4.2, -mavx2)
         * -m<key>=<v> (e.g. -march=native)
         * -f<flag>   code-gen / semantic (e.g. -fPIC, -fno-strict-aliasing)
         * -f<key>=<v> (e.g. -fsanitize=address)
         * -W<flag>   warning flags (e.g. -Wall, -Wextra)
         * -pthread   POSIX threads
         * -pipe      use pipes between compilation stages           */
        } else if (strncmp(a, "-m", 2) == 0 || strncmp(a, "-f", 2) == 0 ||
                   strncmp(a, "-W", 2) == 0) {
            sv_push(&link_other, a);
        } else if (strcmp(a, "-pthread") == 0 || strcmp(a, "-pipe") == 0) {
            sv_push(&link_other, a);

        /* ── Stdin / positional input ────────────────────────────── */
        } else if (strcmp(a, "-") == 0) {
            InputFile *f = iv_push(&inputs);
            if (!f) goto oom;
            f->path = "-";
        } else if (a[0] != '-' && !is_ignored_flag(a)) {
            InputFile *f = iv_push(&inputs);
            if (!f) goto oom;
            f->path = a;

        /* ── Silently accepted no-ops ────────────────────────────── */
        } else if (!is_ignored_flag(a)) {
            fprintf(stderr, "sharpc: unknown option '%s'\n", a);
            usage(stderr);
            ret = 2;
            goto cleanup;
        }
    }

    /* Apply --optimize=<mode> overrides */
    if (optimize) {
        /* Remove existing -g/-O flags from link_other */
        StrVec cleaned = {0};
        for (size_t li = 0; li < link_other.len; li++) {
            const char *f = link_other.data[li];
            if (strcmp(f, "-g") == 0 || strcmp(f, "-gdwarf-4") == 0 ||
                (strncmp(f, "-O", 2) == 0 && f[2] >= '0' && f[2] <= '9')) {
                /* drop */
            } else {
                sv_push(&cleaned, f);
            }
        }
        free(link_other.data);
        link_other = cleaned;

        if (strcmp(optimize, "Debug") == 0) {
            sv_push(&link_other, "-g");
            sv_push(&link_other, "-gdwarf-4");
            sv_push(&link_other, "-O0");
        } else if (strcmp(optimize, "ReleaseFast") == 0) {
            sv_push(&link_other, "-O2");
            sv_push(&link_other, "-DNDEBUG");
        } else if (strcmp(optimize, "ReleaseSafe") == 0) {
            sv_push(&link_other, "-O2");
        } else if (strcmp(optimize, "ReleaseSmall") == 0) {
            sv_push(&link_other, "-Os");
            sv_push(&link_other, "-DNDEBUG");
        } else {
            fprintf(stderr,
                "sharpc: unknown --optimize mode '%s'\n"
                "  Valid modes: Debug, ReleaseFast, ReleaseSafe, ReleaseSmall\n",
                optimize);
            ret = 2; goto cleanup;
        }
    }

    if (inputs.len == 0) {
        fprintf(stderr, "sharpc: no input file\n");
        usage(stderr);
        ret = 2;
        goto cleanup;
    }

    /* Stdin input only makes sense for single-file actions */
    if (inputs.len > 1) {
        for (size_t i = 0; i < inputs.len; i++) {
            if (strcmp(inputs.data[i].path, "-") == 0) {
                fprintf(stderr, "sharpc: stdin input ('-') only valid with a single file\n");
                ret = 2;
                goto cleanup;
            }
        }
    }

    /* System include paths: directly parse zig cc -E -v output to get
     * exactly the same search paths that zig cc uses. */
    CppCtx *path_probe = cpp_ctx_new();
    cpp_detect_zig_sys_paths_from_zig(path_probe, target);
    for (size_t i = 0; i < cpp_sys_include_count(path_probe); i++)
        sv_push(&sys_inc, xstrdup(cpp_sys_include(path_probe, i)));
    cpp_ctx_free(path_probe);

    /* Check zig availability early — give a clear hint if missing. */
    {
        const char *zig_early = cpp_find_zig_exe();
        if (!zig_early) {
            fprintf(stderr,
                "sharpc: error: zig not found.\n"
                "  zig is required as the C compiler backend.\n"
                "  Place 'zig' next to sharpc, or in the parent directory,\n"
                "  or install zig from https://ziglang.org/download/\n"
                "  and add it to your PATH.\n");
            ret = 3;
            goto cleanup;
        }
    }

    /* Find std/ relative to the sharpc executable */
    {
        char *std_dir = sharp_find_std_dir();
        if (std_dir) {
            sv_push(&user_inc, std_dir);
        } else {
            fprintf(stderr,
                "sharpc: warning: cannot locate std/ directory.\n"
                "  Set SHARP_ROOT env var pointing to your sharp repository root.\n"
                "  Example: export SHARP_ROOT=/path/to/sharp\n");
        }
    }

    /* ── Phase 1: preprocess each input to C ──────────────────────── */
    int had_error = 0;
    for (size_t fi = 0; fi < inputs.len; fi++) {
        InputFile *inf = &inputs.data[fi];

        /* .o/.obj/.a/.so/.s — skip sharp preprocessing/compilation entirely */
        if (is_linkable_input(inf->path)) continue;

        /* .S — preprocess with sharp's cpp (flags apply), skip frontend */
        if (is_dot_S(inf->path)) {
            char *pp = preprocess_one_file(inf->path, &user_inc, &sys_inc,
                                           &macros, lang_std, target, inf,
                                           &force_includes, false, false);
            if (!pp) { had_error = 1; continue; }
            inf->tmp_c = pp;
            inf->tmp_is_asm = true;
            if (action == ACTION_PREPROCESS) {
                FILE *out = output ? fopen(output, "w") : stdout;
                if (!out) { perror(output ? output : "stdout"); ret = 2; goto cleanup; }
                fputs(pp, out);
                if (output) fclose(out);
                goto cleanup;
            }
            continue;
        }

        if (is_zig_forward_source(inf->path)) {
            inf->tmp_c = strdup(inf->path);
            inf->tmp_is_zig_direct = true;
            continue;
        }

        if (action == ACTION_PREPROCESS) {
            char *pp_out = preprocess_one_file(inf->path, &user_inc, &sys_inc,
                                               &macros, lang_std, target, inf,
                                               &force_includes, true, true);
            if (!pp_out) { had_error = 1; continue; }
            FILE *out = output ? fopen(output, "w") : stdout;
            if (!out) { perror(output ? output : "stdout"); free(pp_out); ret = 2; goto cleanup; }
            fputs(pp_out, out);
            if (output) fclose(out);
            free(pp_out);
            goto cleanup;
        }

        char *c_out = compile_one_file(inf->path, &user_inc, &sys_inc,
                                       &macros, lang_std, target,
                                       output, inf,
                                       &force_includes);
        if (!c_out) { had_error = 1; continue; }
        inf->tmp_c = c_out;
    }
    if (had_error) { ret = 1; goto cleanup; }

    /* ── Write -MD/-MMD dependency files ────────────────────────── */
    if (gen_deps) {
        for (size_t fi = 0; fi < inputs.len; fi++) {
            InputFile *inf = &inputs.data[fi];
            if (!inf->included_files) continue;

            /* Target name: use -MT arg, or derive .o from path */
            char *target_name;
            char *target_owned = NULL;
            if (dep_target) {
                target_name = (char *)dep_target;
            } else {
                target_owned = replace_ext(inf->path, ".o");
                target_name = target_owned;
            }
            if (!target_name) continue;

            /* Resolve depfile name: use -MF arg, or derive from target */
            const char *dfname = depfile;
            char       *df_owned = NULL;
            if (!dfname) {
                df_owned = replace_ext(target_name, ".d");
                dfname = df_owned;
            }

            FILE *df = fopen(dfname, "w");
            if (!df) {
                perror(dfname);
                free(df_owned); free(target_owned);
                continue;
            }
            fprintf(df, "%s: %s", target_name, inf->path);
            for (size_t di = 0; di < inf->nincluded; di++) {
                /* -MMD: skip system headers */
                if (gen_deps_no_sys && inf->included_is_sys &&
                    inf->included_is_sys[di])
                    continue;
                fprintf(df, " %s", inf->included_files[di]);
            }
            fprintf(df, "\n");
            fclose(df);

            free(df_owned);
            free(target_owned);

            /* Prevent cleanup from double-freeing */
            inf->included_files = NULL;
            inf->nincluded = 0;
        }
    }

    if (action == ACTION_ASSEMBLY) {
        /* -S: emit .s for each input via zig cc -S */
        const char *zig_exe = cpp_find_zig_exe();
        if (!zig_exe) {
            fprintf(stderr, "sharpc: zig not found\n");
            ret = 3; goto cleanup;
        }
        if (!output && inputs.len == 1) {
            output = replace_ext(inputs.data[0].path, ".s");
        }
        for (size_t fi = 0; fi < inputs.len; fi++) {
            InputFile *inf = &inputs.data[fi];
            if (!inf->tmp_c) continue;

            const char *out_file = (inputs.len == 1 && output) ? output :
                                   replace_ext(inf->path, ".s");

            if (inf->tmp_is_zig_direct) {
                StrVec zig_args = zig_cc_prefix(zig_exe, target, sysroot, &link_other);
                sv_push(&zig_args, "-S");
                sv_push(&zig_args, inf->tmp_c);
                sv_push(&zig_args, "-o");
                sv_push(&zig_args, out_file);

                if (zig_cc_run(&zig_args) != 0) {
                    fprintf(stderr, "sharpc: assembly failed for %s\n", inf->path);
                    ret = 3; goto cleanup;
                }
            } else if (inf->tmp_is_asm) {
                FILE *f = fopen(out_file, "w");
                if (!f) { perror(out_file); ret = 2; goto cleanup; }
                fputs(inf->tmp_c, f);
                fclose(f);
            } else {
                char *tmp_i = write_tmp_file(inf->path, ".i", inf->tmp_c);
                if (!tmp_i) { ret = 2; goto cleanup; }

                StrVec zig_args = zig_cc_prefix(zig_exe, target, sysroot, &link_other);
                sv_push(&zig_args, "-x");
                sv_push(&zig_args, "c");
                sv_push(&zig_args, "-S");
                sv_push(&zig_args, tmp_i);
                sv_push(&zig_args, "-o");
                sv_push(&zig_args, out_file);

                if (zig_cc_run(&zig_args) != 0) {
                    fprintf(stderr, "sharpc: assembly generation failed for %s\n", inf->path);
                    ret = 3; goto cleanup;
                }
            }
            if (g_sess.verbose)
                fprintf(stderr, "sharpc: wrote %s\n", out_file);
            if (inputs.len != 1 || !output)
                free((void *)out_file);
        }
        goto cleanup;
    }

    if (action == ACTION_COMPILE_ONLY) {
        /* -c: compile to .o for each input via zig cc -c,
         * or emit C text if -o target has .i extension. */
        const char *zig_exe = cpp_find_zig_exe();
        if (!zig_exe) {
            fprintf(stderr, "sharpc: zig not found\n");
            ret = 3; goto cleanup;
        }
        bool emit_text_only = false;
        if (output) {
            const char *ext = file_ext(output);
            if (strcmp(ext, ".i") == 0 || strcmp(ext, ".c") == 0)
                emit_text_only = true;
        }

        if (emit_text_only && inputs.len == 1) {
            FILE *f = fopen(output, "w");
            if (!f) { perror(output); ret = 2; goto cleanup; }
            fputs(inputs.data[0].tmp_c, f);
            fclose(f);
            if (g_sess.verbose)
                fprintf(stderr, "sharpc: wrote %s\n", output);
            goto cleanup;
        }

        for (size_t fi = 0; fi < inputs.len; fi++) {
            InputFile *inf = &inputs.data[fi];
            if (!inf->tmp_c) continue;

            const char *obj_out = (inputs.len == 1 && output)
                                  ? output :
                                  replace_ext(inf->path, ".o");

            char *obj_tmp = compile_input_to_obj(zig_exe, target, sysroot,
                                                  &link_other,
                                                  &user_inc, &sys_inc, inf);
            if (!obj_tmp) { ret = 3; goto cleanup; }

            if (strcmp(obj_tmp, obj_out) != 0) {
                if (g_sess.verbose)
                    fprintf(stderr, "sharpc: rename %s -> %s\n", obj_tmp, obj_out);
                rename(obj_tmp, obj_out);
            }
            if (g_sess.verbose)
                fprintf(stderr, "sharpc: wrote %s\n", obj_out);
            if (inputs.len != 1 || !output)
                free((void *)obj_out);
        }
        goto cleanup;
    }

    /* ── Compile inputs ─────────────────────────────────────────── */
    const char *zig_exe = cpp_find_zig_exe();
    if (!zig_exe) {
        fprintf(stderr, "sharpc: zig not found\n");
        ret = 3; goto cleanup;
    }
    StrVec obj_files = {0};
    for (size_t fi = 0; fi < inputs.len; fi++) {
        InputFile *inf = &inputs.data[fi];

        if (is_linkable_input(inf->path)) {
            sv_push(&obj_files, inf->path);
            continue;
        }

        if (!inf->tmp_c) continue;

        char *obj_tmp = compile_input_to_obj(zig_exe, target, sysroot,
                                              &link_other,
                                              &user_inc, &sys_inc, inf);
        if (!obj_tmp) { ret = 3; goto cleanup; }

        sv_push(&obj_files, obj_tmp);
    }

    /* ── Link ─────────────────────────────────────────────────────── */
    if (!output && inputs.len == 1) {
        output = replace_ext(inputs.data[0].path, "");
    } else if (!output) {
        output = "a.out";
    }

    StrVec ld_args = zig_cc_prefix(zig_exe, target, sysroot, &link_other);
    sv_push(&ld_args, "-o");
    sv_push(&ld_args, output);
    for (size_t oi = 0; oi < obj_files.len; oi++)
        sv_push(&ld_args, obj_files.data[oi]);
    for (size_t li = 0; li < link_paths.len; li++)
        sv_push(&ld_args, link_paths.data[li]);
    for (size_t li = 0; li < link_libs.len; li++)
        sv_push(&ld_args, link_libs.data[li]);

    if (zig_cc_run(&ld_args) != 0) {
        fprintf(stderr, "sharpc: link failed\n");
        ret = 3;
        goto cleanup;
    }
    free(obj_files.data);

    if (g_sess.verbose)
        fprintf(stderr, "sharpc: wrote %s\n", output);

cleanup:
    cleanup_session(&user_inc, &sys_inc, &macros, &link_libs, &link_paths,
                     &link_other, &force_includes, &inputs);
    return ret;

oom:
    fprintf(stderr, "sharpc: out of memory\n");
    cleanup_session(&user_inc, &sys_inc, &macros, &link_libs, &link_paths,
                     &link_other, &force_includes, &inputs);
    return 2;
}
