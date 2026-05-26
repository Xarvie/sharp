/*
 * sharpc.c — Sharp compiler driver (clang-compatible CLI).
 *
 * Sharp is a superset of C (cf. C++ ⊃ C).  The same driver compiles
 * `.sp` Sharp source and `.c` C source — input is always parsed as
 * Sharp.  The driver can preprocess, compile, assemble, link, or
 * compile+link in one invocation — just like clang.
 *
 * Usage (clang-compatible):
 *   sharpc <input> [-o <output>] [options]          # compile + link
 *   sharpc -c <input> [-o <obj>]                    # compile to .o
 *   sharpc -S <input> [-o <asm>]                    # emit .s
 *   sharpc -E <input> [-o <pp>]                     # preprocess only
 *   sharpc file1.sp file2.sp -o app                  # multi-file link
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
 * Finds {sharp_root}/sharp/std/ by locating the directory of the
 * sharpc executable and walking up to its project root.
 *
 * Resolution order (first found wins):
 *   1. <exe_dir>/../sharp/std/
 *   2. <exe_dir>/sharp/std/               (exe is at repo root)
 *   3. SHARP_ROOT environment variable
 *
 * Returns a malloc'd string (caller frees), or NULL if not found.
 * Returns an allocated path even if the directory doesn't exist —
 * callers can create it or warn.
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
        "sharp/std",      /* <exe_dir>/sharp/std/ */
        "../sharp/std",   /* <exe_dir>/../sharp/std/ */
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
        int n = snprintf(candidate, sizeof(candidate), "%s/sharp/std", env);
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
    char       *tmp_c;         /* generated .c text (NULL if not yet generated) */
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
    g_sess.tmp_files = realloc(g_sess.tmp_files, (g_sess.n_tmp_files + 1) * sizeof *g_sess.tmp_files);
    if (!g_sess.tmp_files) return;
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
    char *buf = malloc(strlen(prefix) + strlen(suffix) + 16);
    if (!buf) return NULL;
    snprintf(buf, strlen(prefix) + strlen(suffix) + 16,
             "/tmp/sharp-XXXXXX%s", suffix);
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
 * Uses system() + file redirect to avoid pipe-inheritance deadlock.
 * zig cc spawns sub-processes (compiler_rt) that inherit pipe handles;
 * with system(), all output goes through a temp file — no pipes.      */
static int run_cmd(const char **argv) {

    /* Build command line string from argv */
    char cmd[4096];
    size_t pos = 0;
    for (int i = 0; argv[i]; i++) {
        if (i > 0 && pos + 1 < sizeof(cmd)) cmd[pos++] = ' ';
        const char *a = argv[i];
        int need_q = (strchr(a, ' ') != NULL || a[0] == '\0');
        if (need_q && pos + 2 < sizeof(cmd)) cmd[pos++] = '"';
        size_t alen = strlen(a);
        if (pos + alen + (need_q ? 2 : 1) < sizeof(cmd)) {
            memcpy(cmd + pos, a, alen);
            pos += alen;
        }
        if (need_q && pos + 1 < sizeof(cmd)) cmd[pos++] = '"';
    }
    cmd[pos] = '\0';

    /* Temp file for stderr (per-thread unique) */
    char err_path[512];
#ifdef _WIN32
    const char *tmp = getenv("TEMP");
    if (!tmp) tmp = ".";
    snprintf(err_path, sizeof(err_path), "%s\\sharpc_err_%lu_%lu.tmp",
             tmp, (unsigned long)GetCurrentProcessId(),
             (unsigned long)GetCurrentThreadId());
#else
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp";
    snprintf(err_path, sizeof(err_path), "%s/sharpc_err_%d_%lu.tmp",
             tmp, (int)getpid(), (unsigned long)pthread_self());
#endif

    char full[8192];
    if (g_sess.verbose)
        snprintf(full, sizeof(full), "%s 2> \"%s\"", cmd, err_path);
    else
        snprintf(full, sizeof(full), "%s > NUL 2> \"%s\"", cmd, err_path);

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

static char *replace_ext(const char *path, const char *new_ext) {
    /* e.g. /a/b/foo.sp → /a/b/foo.o */
    const char *dot = strrchr(path, '.');
    if (!dot) {
        char *r = malloc(strlen(path) + strlen(new_ext) + 1);
        if (!r) return NULL;
        sprintf(r, "%s%s", path, new_ext);
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

/* Recognise compilation-only flags that the driver should accept and
 * ignore (so `sharpc` can be substituted for `cc` without the build
 * system having to filter them out).
 * Returns number of args to skip (0 = not ignored, 1 = skip just flag,
 * 2 = skip flag + its argument). */
static int is_ignored_flag(const char *a) {
    if (strncmp(a, "-W", 2) == 0)    return 1;   /* any -W... */
    if (strncmp(a, "-f", 2) == 0)    return 1;   /* any -f... */
    if (strncmp(a, "-m", 2) == 0)    return 1;   /* any -m... (e.g. -m64) */
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
"  <file>.sp/.c/.i     source files (multiple inputs supported)\n"
"  -                   read from stdin\n"
"\n"
"Output:\n"
"  -o <file>           output file (auto-detects .exe/.o/.s based on action)\n"
"\n"
"Preprocessor (gcc-compatible):\n"
"  -I <dir>            add user #include search path\n"
"  -isystem <dir>      add system #include search path\n"
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
"Misc:\n"
"  --target <triple>   install target macros (default: x86_64-linux-gnu)\n"
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

    const char *p = src;
    while (*p) {
        /* Skip string literals — raw strings inside are not syntax */
        if (*p == '"') {
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            while (*p && *p != '"') {
                if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
                if (*p == '\\' && p[1]) { out[out_len++] = *p++; }
                out[out_len++] = *p++;
            }
            if (*p == '"') { out[out_len++] = *p++; }
            continue;
        }
        /* Skip char literals */
        if (*p == '\'') {
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            while (*p && *p != '\'') {
                if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
                if (*p == '\\' && p[1]) { out[out_len++] = *p++; }
                out[out_len++] = *p++;
            }
            if (*p == '\'') { out[out_len++] = *p++; }
            continue;
        }
        /* Skip line comments */
        if (p[0] == '/' && p[1] == '/') {
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            while (*p && *p != '\n') {
                if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
                out[out_len++] = *p++;
            }
            continue;
        }
        /* Skip block comments */
        if (p[0] == '/' && p[1] == '*') {
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
            out[out_len++] = *p++;
            while (*p) {
                if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
                if (p[0] == '*' && p[1] == '/') { out[out_len++] = *p++; out[out_len++] = *p++; break; }
                out[out_len++] = *p++;
            }
            continue;
        }

        /* Look for r followed by #s and " — must be at token boundary */
        int at_boundary = (p == src) || !(((p[-1] >= 'a' && p[-1] <= 'z') || (p[-1] >= 'A' && p[-1] <= 'Z') ||
                                           (p[-1] >= '0' && p[-1] <= '9') || p[-1] == '_'));
        if (at_boundary && p[0] == 'r' && p[1] == '"') {
            /* Plain raw string: r"..." — zero hashes */
            const char *after_q = p + 2;
            const char *close = strchr(after_q, '"');
            if (close) {
                size_t content_len = (size_t)(close - after_q);
                if (out_len + content_len * 2 + 8 >= out_cap) {
                    out_cap = (out_len + content_len * 2 + 8) * 2;
                    out = realloc(out, out_cap);
                    if (!out) return NULL;
                }
                out[out_len++] = '"';
                for (size_t i = 0; i < content_len; i++) {
                    unsigned char c = (unsigned char)after_q[i];
                    if (c == '\\')      { out[out_len++] = '\\'; out[out_len++] = '\\'; }
                    else if (c == '"')  { out[out_len++] = '\\'; out[out_len++] = '"';  }
                    else if (c == '\n') { out[out_len++] = '\\'; out[out_len++] = 'n';  }
                    else if (c == '\t') { out[out_len++] = '\\'; out[out_len++] = 't';  }
                    else if (c == '\r') { out[out_len++] = '\\'; out[out_len++] = 'r';  }
                    else out[out_len++] = c;
                }
                out[out_len++] = '"';
                p = close + 1;
                continue;
            }
            /* No closing quote, treat as literal. */
        }
        /* Look for r#..."#... pattern */
        if (at_boundary && p[0] == 'r' && p[1] == '#') {
            /* Count hashes */
            const char *h = p + 1;
            while (*h == '#') h++;
            size_t nhash = (size_t)(h - p - 1);
            if (*h == '"') {
                /* Valid raw string start. Find closing "#..."# */
                const char *content_start = h + 1;
                const char *cs = content_start;
                const char *close = NULL;
                while (*cs) {
                    if (*cs == '"') {
                        /* Check for matching hashes */
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
                    size_t content_len = (size_t)(close - content_start);
                    if (out_len + content_len * 2 + 8 >= out_cap) {
                        out_cap = (out_len + content_len * 2 + 8) * 2;
                        out = realloc(out, out_cap);
                        if (!out) return NULL;
                    }
                    out[out_len++] = '"';
                    for (size_t i = 0; i < content_len; i++) {
                        unsigned char c = (unsigned char)content_start[i];
                        if (c == '\\')      { out[out_len++] = '\\'; out[out_len++] = '\\'; }
                        else if (c == '"')  { out[out_len++] = '\\'; out[out_len++] = '"';  }
                        else if (c == '\n') { out[out_len++] = '\\'; out[out_len++] = 'n';  }
                        else if (c == '\t') { out[out_len++] = '\\'; out[out_len++] = 't';  }
                        else if (c == '\r') { out[out_len++] = '\\'; out[out_len++] = 'r';  }
                        else out[out_len++] = c;
                    }
                    out[out_len++] = '"';
                    p = close + 1 + nhash;  /* skip closing "#...# */
                    continue;
                }
            }
            /* Not valid, copy 'r' as literal. */
        }
        /* Regular character. */
        if (out_len + 1 >= out_cap) { out_cap *= 2; out = realloc(out, out_cap); if (!out) return NULL; }
        out[out_len++] = *p++;
    }
    if (out_len >= out_cap) { out = realloc(out, out_len + 1); if (!out) return NULL; }
    out[out_len] = '\0';
    return out;
}
static char *preprocess_one_file(const char *input,
                                 StrVec *user_inc, StrVec *sys_inc,
                                 MacroVec *macros, long lang_std,
                                 const char *target,
                                 InputFile *inf) {
    char *src = read_file(input);
    if (!src) return NULL;

    /* Convert R"(...)" raw strings to standard C string literals. */
    char *src2 = convert_raw_strings(src, input);
    free(src);
    if (!src2) return NULL;
    src = src2;

    CppCtx *cctx = cpp_ctx_new();
    cpp_probe_zig_macros(cctx, target);
    cpp_emit_linemarkers(cctx, true);
    if (lang_std >= 0) cpp_set_lang_std(cctx, lang_std);

    for (size_t i = 0; i < user_inc->len; i++)
        cpp_add_user_include(cctx, user_inc->data[i]);
    for (size_t i = 0; i < sys_inc->len; i++)
        cpp_add_sys_include(cctx, sys_inc->data[i]);

    for (size_t i = 0; i < macros->len; i++) {
        if (macros->data[i].is_undef) {
            cpp_undefine(cctx, macros->data[i].spec);
        } else if (apply_define(cctx, macros->data[i].spec) < 0) {
            cpp_ctx_free(cctx); free(src); return NULL;
        }
    }

    CppResult r = cpp_run_buf(cctx, src, strlen(src), input);
    free(src);

    bool cpp_had_error = false;
    for (size_t i = 0; i < r.ndiags; i++) {
        CppDiag *d = &r.diags[i];
        if (d->level == CPP_DIAG_NOTE) continue;
        const char *fname = d->loc.file ? d->loc.file : input;
        fprintf(stderr, "%s:%u:%u: %s: %s\n",
                fname, d->loc.line, d->loc.col,
                d->level == CPP_DIAG_FATAL   ? "fatal" :
                d->level == CPP_DIAG_ERROR   ? "error" :
                d->level == CPP_DIAG_WARNING ? "warning" : "note",
                d->msg ? d->msg : "");
        if (d->level == CPP_DIAG_ERROR || d->level == CPP_DIAG_FATAL)
            cpp_had_error = true;
    }
    if (cpp_had_error) {
        if (inf) { inf->included_files = r.included_files; inf->nincluded = r.nincluded; inf->included_is_sys = r.included_is_sys; r.included_files = NULL; r.nincluded = 0; r.included_is_sys = NULL; }
        cpp_result_free(&r);
        cpp_ctx_free(cctx);
        return NULL;
    }

    char *out = malloc(r.text_len + 1);
    if (!out) {
        if (inf) { inf->included_files = r.included_files; inf->nincluded = r.nincluded; inf->included_is_sys = r.included_is_sys; r.included_files = NULL; r.nincluded = 0; r.included_is_sys = NULL; }
        cpp_result_free(&r); cpp_ctx_free(cctx); return NULL;
    }
    if (r.text_len) memcpy(out, r.text, r.text_len);
    out[r.text_len] = '\0';
    if (inf) { inf->included_files = r.included_files; inf->nincluded = r.nincluded; inf->included_is_sys = r.included_is_sys; r.included_files = NULL; r.nincluded = 0; r.included_is_sys = NULL; }
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
                              InputFile *inf) {
    char *src = read_file(input);
    if (!src) return NULL;

    /* Convert r#"..."# raw strings to standard C string literals
     * BEFORE the preprocessor runs.  Only for Sharp source files. */
    const char *input_ext = strrchr(input, '.');
    int is_sharp_input = input_ext && (strcmp(input_ext, ".sp") == 0 ||
                                        strcmp(input_ext, ".sph") == 0);
    if (is_sharp_input) {
        char *src2 = convert_raw_strings(src, input);
        free(src);
        if (!src2) return NULL;
        src = src2;
    }

    CppCtx *cctx = cpp_ctx_new();
    cpp_probe_zig_macros(cctx, target);
    cpp_emit_linemarkers(cctx, false);
    if (lang_std >= 0) cpp_set_lang_std(cctx, lang_std);

    /* Auto-detect system include paths by querying zig cc directly.
     * Returns the compiler's built-in search directories with correct
     * priority for the given target. */
    cpp_detect_zig_sys_paths(cctx, target);

    for (size_t i = 0; i < user_inc->len; i++)
        cpp_add_user_include(cctx, user_inc->data[i]);
    for (size_t i = 0; i < sys_inc->len; i++)
        cpp_add_sys_include(cctx, sys_inc->data[i]);

    for (size_t i = 0; i < macros->len; i++) {
        if (macros->data[i].is_undef) {
            cpp_undefine(cctx, macros->data[i].spec);
        } else if (apply_define(cctx, macros->data[i].spec) < 0) {
            cpp_ctx_free(cctx); free(src); return NULL;
        }
    }

    CppResult r = cpp_run_buf(cctx, src, strlen(src), input);
    free(src);

    /* Check cpp diagnostics */
    bool cpp_had_error = false;
    for (size_t i = 0; i < r.ndiags; i++) {
        CppDiag *d = &r.diags[i];
        if (d->level == CPP_DIAG_NOTE) continue;
        const char *fname = d->loc.file ? d->loc.file : input;
        fprintf(stderr, "%s:%u:%u: %s: %s\n",
                fname, d->loc.line, d->loc.col,
                d->level == CPP_DIAG_FATAL   ? "fatal" :
                d->level == CPP_DIAG_ERROR   ? "error" :
                d->level == CPP_DIAG_WARNING ? "warning" : "note",
                d->msg ? d->msg : "");
        if (d->level == CPP_DIAG_ERROR || d->level == CPP_DIAG_FATAL)
            cpp_had_error = true;
    }
    if (cpp_had_error) {
        if (inf) { inf->included_files = r.included_files; inf->nincluded = r.nincluded; inf->included_is_sys = r.included_is_sys; r.included_files = NULL; r.nincluded = 0; r.included_is_sys = NULL; }
        cpp_result_free(&r);
        cpp_ctx_free(cctx);
        return NULL;
    }

    FeDiagArr ld = {0}, pd = {0}, sd = {0}, sema_d = {0};

    size_t ntoks = 0;
    SharpTok *toks = lex_run(r.tokens, r.ntokens, input, &ld, &ntoks);

    AstNode *ast = parse_file_with_typedefs(toks, ntoks, input, &pd,
                                            NULL);

    /* Extract #include directives from the original source file */
    if (ast && toks && ntoks > 0) {
        const char **_seen = NULL;
        size_t _nseen = 0, _seen_cap = 0;
        for (size_t _ti = 0; _ti < ntoks; _ti++) {
            const char *_f = toks[_ti].loc.file;
            if (!_f || !*_f || *_f == '<' || strcmp(_f, input) == 0) continue;
            bool _dup = false;
            for (size_t _k = 0; _k < _nseen && !_dup; _k++)
                if (_seen[_k] == _f) _dup = true;
            if (_dup) continue;
            if (_nseen >= _seen_cap) {
                _seen_cap = _seen_cap ? _seen_cap * 2 : 64;
                _seen = realloc(_seen, _seen_cap * sizeof *_seen);
            }
            _seen[_nseen++] = _f;
        }

        FILE *_src_fp = fopen(input, "r");
        if (_src_fp) {
            char _line[1024];
            while (fgets(_line, sizeof _line, _src_fp)) {
                char *_p = _line;
                while (*_p == ' ' || *_p == '\t') _p++;
                if (*_p != '#') continue;
                _p++;
                while (*_p == ' ' || *_p == '\t') _p++;
                if (strncmp(_p, "include", 7) != 0) continue;
                _p += 7;
                while (*_p == ' ' || *_p == '\t') _p++;
                if (*_p != '<' && *_p != '"') continue;

                char _fname[512];
                char _delim_end = (*_p == '<') ? '>' : '"';
                const char *_s = _p + 1;
                const char *_e = strchr(_s, _delim_end);
                if (!_e) continue;
                size_t _flen = (size_t)(_e - _s);
                if (_flen == 0 || _flen >= sizeof _fname) continue;
                memcpy(_fname, _s, _flen); _fname[_flen] = '\0';

                const char *_bn = strrchr(_fname, '/');
                _bn = _bn ? _bn + 1 : _fname;

                bool _found = false;
                for (size_t _k = 0; _k < _nseen && !_found; _k++) {
                    const char *_sbn = strrchr(_seen[_k], '/');
                    _sbn = _sbn ? _sbn + 1 : _seen[_k];
                    if (strcmp(_sbn, _bn) == 0) _found = true;
                }
                if (!_found) continue;

                char *_end2 = _p + strlen(_p) - 1;
                while (_end2 > _p && (*_end2 == '\n' || *_end2 == '\r' ||
                                      *_end2 == ' '  || *_end2 == '\t'))
                    *_end2-- = '\0';
                char _inc[1024];
                snprintf(_inc, sizeof _inc, "#include %s", _p);
                file_add_include(ast, _inc);
            }
            fclose(_src_fp);
        }
        free(_seen);
    }

    /* Linemarkers */
    if (ast && r.text) {
        const char *p = r.text;
        const char *stk[256];
        int stkdepth = 0;
        stk[stkdepth++] = input;
        while (*p) {
            if (*p != '#') { while (*p && *p != '\n') p++; if (*p) p++; continue; }
            const char *line_start = p++;
            while (*p == ' ' || *p == '\t') p++;
            if (*p < '0' || *p > '9') { while (*p && *p != '\n') p++; if (*p) p++; continue; }
            while (*p >= '0' && *p <= '9') p++;
            while (*p == ' ' || *p == '\t') p++;
            if (*p != '"') { while (*p && *p != '\n') p++; if (*p) p++; continue; }
            p++;
            const char *fname_start = p;
            while (*p && *p != '"') p++;
            size_t fname_len = (size_t)(p - fname_start);
            if (*p) p++;
            int flags = 0;
            while (*p == ' ' || *p == '\t') p++;
            while (*p >= '0' && *p <= '9') { flags |= (1 << (*p - '1')); p++; while (*p == ' ' || *p == '\t') p++; }
            (void)line_start;
            bool entering  = (flags & 1) != 0;
            bool returning = (flags & 2) != 0;
            bool is_sys    = (flags & 4) != 0;
            char fname[512];
            if (fname_len >= sizeof fname) fname_len = sizeof fname - 1;
            memcpy(fname, fname_start, fname_len);
            fname[fname_len] = '\0';
            if (entering && !returning) {
                bool from_root = (stkdepth == 1 &&
                                  strcmp(stk[0], input) == 0);
                bool is_builtin = (fname[0] == '<' || fname[0] == '\0' ||
                                   strcmp(fname, "<built-in>") == 0 ||
                                   strcmp(fname, "<command-line>") == 0);
                if (from_root && !is_sys && !is_builtin) {
                    char inc_str[600];
                    snprintf(inc_str, sizeof inc_str,
                             "#include \"%s\"", fname);
                    file_add_include(ast, inc_str);
                }
                if (stkdepth < (int)(sizeof stk / sizeof *stk))
                    stk[stkdepth++] = r.text + (fname_start - r.text);
            } else if (returning && stkdepth > 0) {
                stkdepth--;
            }
            while (*p && *p != '\n') p++;
            if (*p) p++;
        }
    }

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
        if (sys_inc->len > 0)
            cg_set_sys_dirs(cg, (StrArr *)sys_inc);

        c_out = cg_generate(cg, ast);

        /* Rebuild #include directives from system headers consumed by
         * the preprocessor.  Rebuilds the preamble of needed system includes.
         * Guards are applied for uid_t/gid_t, sa_handler, and socklen_t. */
        if (sys_inc->len > 0 && c_out) {
            StrArr needed = {0};

            /* (a) Type-tracked includes from CG */
            const StrArr *cg_inc = cg_needed_includes(cg);
            if (cg_inc) {
                for (size_t i = 0; i < cg_inc->len; i++)
                    strarr_push(&needed, xstrdup(cg_inc->data[i]));
            }

            /* (b) Scan CPP tokens to find which system headers were touched */
            for (size_t ti = 0; ti < r.ntokens; ti++) {
                const char *file = r.tokens[ti].loc.file;
                if (!file || !file[0]) continue;
                for (size_t di = 0; di < sys_inc->len; di++) {
                    const char *dir = sys_inc->data[di];
                    size_t dlen = strlen(dir);
                    if (strncmp(file, dir, dlen) != 0 || file[dlen] != '/')
                        continue;
                    const char *inc = file + dlen + 1;
                    if (strncmp(inc, "bits/", 5) == 0) break;
                    if (strncmp(inc, "gnu/", 4) == 0) break;
                    if (strncmp(inc, "asm/", 4) == 0) break;
                    if (strncmp(inc, "asm-generic/", 12) == 0) break;
                    if (strncmp(inc, "linux/", 6) == 0) break;
                    if (strncmp(inc, "sysdeps/", 8) == 0) break;
                    if (strncmp(inc, "x86_64-linux-gnu/", 18) == 0) break;
                    if (strncmp(inc, "rpc/", 4) == 0) break;
                    if (strcmp(inc, "features.h") == 0) break;
                    if (strcmp(inc, "stubs.h") == 0) break;
                    if (strcmp(inc, "stubs-64.h") == 0) break;
                    if (strcmp(inc, "sys/cdefs.h") == 0) break;
                    if (strncmp(inc, "c++/14.2.0/", 11) == 0) break;
                    if (strncmp(inc, "c++/", 4) == 0) break;
                    if (strcmp(inc, "c++defs.h") == 0) break;
                    if (strcmp(inc, "libc-header-start.h") == 0) break;
                    if (strncmp(inc, "_mingw", 6) == 0) break;
                    if (strcmp(inc, "vadefs.h") == 0) break;
                    if (strcmp(inc, "corecrt.h") == 0) break;
                    if (strncmp(inc, "corecrt_", 8) == 0) break;
                    if (strncmp(inc, "msvcrt/", 7) == 0) break;
                    if (strcmp(inc, "crtdefs.h") == 0) break;
                    if (strcmp(inc, "vcruntime.h") == 0) break;
                    if (strcmp(inc, "concurrencysal.h") == 0) break;
                    if (strncmp(inc, "sal.h", 5) == 0) break;
                    if (strncmp(inc, "codeanalysis/", 13) == 0) break;
                    bool dup = false;
                    for (size_t ni = 0; ni < needed.len; ni++)
                        if (strcmp(needed.data[ni], inc) == 0) { dup = true; break; }
                    if (!dup) strarr_push(&needed, xstrdup(inc));
                    break;
                }
            }

            /* Guard variable detection */
            bool has_socket = false, has_netinet = false, has_signal = false;
            for (size_t i = 0; i < needed.len; i++) {
                const char *inc = needed.data[i];
                if (strcmp(inc, "sys/socket.h") == 0) has_socket = true;
                else if (strcmp(inc, "netinet/in.h") == 0) has_netinet = true;
                else if (strcmp(inc, "signal.h") == 0) has_signal = true;
            }


            /* uid_t/gid_t guard: scan for `typedef X uid_t;` or `typedef X gid_t;` */
            bool need_uid_guard = false;
            if (c_out) {
                /* Only guard when user code defines uid_t/gid_t as a TYPEDEF
                 * TARGET: `typedef X uid_t;` or `typedef X gid_t;`.
                 * Cases like `typedef uid_t X;` or `typedef gid_t X;` are
                 * harmless — uid_t/gid_t is the SOURCE type there. */
                for (const char *p = c_out; (p = strstr(p, "typedef")) != NULL; ) {
                    const char *end = strchr(p, '\n');
                    if (!end) end = p + strlen(p);
                    /* Skip "typedef", whitespace, first identifier (source type) */
                    const char *q = p + 7;
                    while (q < end && (*q == ' ' || *q == '\t' || *q == '*')) q++;
                    while (q < end && (*q == '_' || (*q >= 'a' && *q <= 'z') ||
                                       (*q >= 'A' && *q <= 'Z') ||
                                       (*q >= '0' && *q <= '9') || *q == '*')) q++;
                    while (q < end && (*q == ' ' || *q == '\t')) q++;
                    /* q now points at the typedef target */
                    if ((strncmp(q, "uid_t", 5) == 0 || strncmp(q, "gid_t", 5) == 0) &&
                        (q + 5 >= end || *(q + 5) == ';' || *(q + 5) == ' ' ||
                         *(q + 5) == '\t' || *(q + 5) == ',' || *(q + 5) == '[' ||
                         *(q + 5) == '(' || *(q + 5) == ')' || *(q + 5) == '=')) {
                        need_uid_guard = true;
                        break;
                    }
                    p = end + (end[0] != '\0' ? 1 : 0);
                }
            }

            /* Ordered list of known system headers (conventional C order).
             * Each is emitted iff present in the needed set. */
            static const char *known[] = {
                "stddef.h", "stdint.h", "stdbool.h", "stdarg.h",
                "stdio.h", "stdlib.h", "string.h", "strings.h",
                "ctype.h", "setjmp.h", "math.h", "locale.h", "errno.h",
                "time.h", "sys/time.h", "fcntl.h", "unistd.h",
                "signal.h", "pthread.h", "sys/types.h",
                "sys/socket.h", "sys/stat.h", "sys/ioctl.h", "sys/mman.h",
                "sys/uio.h", "sys/eventfd.h", "sys/timerfd.h",
                "sys/epoll.h", "poll.h", "sys/select.h",
                "netinet/in.h", "netinet/tcp.h", "arpa/inet.h",
                "netdb.h", "ifaddrs.h", "malloc.h", "intrin.h",
            };
            size_t nknown = sizeof known / sizeof known[0];

            char hdr[4096];
            size_t pos = 0;
            #define W(lit) do { \
                size_t n = sizeof(lit)-1; \
                if (pos + n > sizeof hdr) break; \
                memcpy(hdr+pos, lit, n); pos+=n; \
            } while(0)

            if (need_uid_guard) {
                W("#define uid_t __uid_t\n");
                W("#define gid_t __gid_t\n");
                W("#define __uid_t_defined\n");
                W("#define __gid_t_defined\n");
            }

            for (size_t ki = 0; ki < nknown; ki++) {
                for (size_t ni = 0; ni < needed.len; ni++) {
                    if (strcmp(needed.data[ni], known[ki]) == 0) {
                        char buf[256];
                        int blen = snprintf(buf, sizeof buf,
                                           "#include <%s>\n", known[ki]);
                        if (blen > 0 && pos + (size_t)blen <= sizeof hdr) {
                            memcpy(hdr + pos, buf, (size_t)blen);
                            pos += (size_t)blen;
                        }
                        break;
                    }
                }
            }

            if (has_signal) {
                W("#undef sa_handler\n");
                W("#undef sa_sigaction\n");
            }

            /* Emit headers not in the known list (project-specific, etc.) */
            for (size_t ni = 0; ni < needed.len; ni++) {
                bool found = false;
                for (size_t ki = 0; ki < nknown; ki++)
                    if (strcmp(needed.data[ni], known[ki]) == 0)
                        { found = true; break; }
                if (!found) {
                    char buf[256];
                    int blen = snprintf(buf, sizeof buf,
                                       "#include <%s>\n", needed.data[ni]);
                    if (blen > 0 && pos + (size_t)blen <= sizeof hdr) {
                        memcpy(hdr + pos, buf, (size_t)blen);
                        pos += (size_t)blen;
                    }
                }
            }

            if (need_uid_guard) {
                W("#undef uid_t\n");
                W("#undef gid_t\n");
                W("#undef __uid_t_defined\n");
                W("#undef __gid_t_defined\n");
            }
            if (has_socket || has_netinet) {
                W("#ifndef __socklen_t_defined\n");
                W("typedef unsigned int socklen_t;\n");
                W("#endif\n");
            }
            W("\n");

            #undef W

            size_t clen = strlen(c_out);
            char *new_out = malloc(pos + clen + 1);
            if (new_out) {
                memcpy(new_out, hdr, pos);
                memcpy(new_out + pos, c_out, clen + 1);
                free(c_out);
                c_out = new_out;
            }

            strarr_free_contents(&needed);
        }

        cg_ctx_free(cg);
    }

    /* Cleanup */
    sema_ctx_free(sema); ty_store_free(ts);
    scope_free_chain(scope); ast_node_free(ast); lex_free(toks);
    if (inf) { inf->included_files = r.included_files; inf->nincluded = r.nincluded; inf->included_is_sys = r.included_is_sys; r.included_files = NULL; r.nincluded = 0; r.included_is_sys = NULL; }
    cpp_result_free(&r);
    for (int a = 0; a < 4; a++) {
        for (size_t ii = 0; ii < all_diag[a]->len; ii++) free(all_diag[a]->data[ii].msg);
        free(all_diag[a]->data);
    }
    cpp_ctx_free(cctx);

    return c_out;
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
    StrVec      user_inc    = {0};
    StrVec      sys_inc     = {0};
    MacroVec    macros      = {0};
    StrVec      link_libs   = {0};   /* -l<lib> */
    StrVec      link_paths  = {0};   /* -L<dir> */
    StrVec      link_other  = {0};   /* extra linker flags (-shared, -static, etc.) */
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
            fprintf(stdout, "sharpc %s\n", SHARP_VERSION);
            goto cleanup;
        } else if (strcmp(a, "-v") == 0) {
            g_sess.verbose = 1;

        /* ── Target ──────────────────────────────────────────────── */
        } else if (strcmp(a, "--target") == 0 && i + 1 < argc) {
            target = argv[++i];

        /* ── --optimize=<mode> ──────────────────────────────────── */
        } else if (strcmp(a, "--optimize") == 0 && i + 1 < argc) {
            optimize = argv[++i];
        } else if (strncmp(a, "--optimize=", 11) == 0) {
            optimize = a + 11;

        /* ── -I ──────────────────────────────────────────────────── */
        } else if (strcmp(a, "-I") == 0 && i + 1 < argc) {
            if (sv_push(&user_inc, argv[++i]) < 0) goto oom;
        } else if (strncmp(a, "-I", 2) == 0 && a[2]) {
            if (sv_push(&user_inc, a + 2) < 0) goto oom;

        /* ── -isystem ────────────────────────────────────────────── */
        } else if (strcmp(a, "-isystem") == 0 && i + 1 < argc) {
            if (sv_push(&sys_inc, argv[++i]) < 0) goto oom;
        } else if (strncmp(a, "-isystem", 8) == 0 && a[8]) {
            if (sv_push(&sys_inc, a + 8) < 0) goto oom;

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
        } else {
            int skip = is_ignored_flag(a);
            if (skip) {
                if (skip >= 2 && i + 1 < argc) i++;   /* skip flag's argument */
            } else {
            fprintf(stderr, "sharpc: unknown option '%s'\n", a);
            usage(stderr);
            ret = 2;
            goto cleanup;
        }
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

    /* System include paths: detect for preamble injection and CG tracking.
     * Query zig cc directly to get the correct search paths with priority. */
    CppCtx *path_probe = cpp_ctx_new();
    cpp_detect_zig_sys_paths(path_probe, target);
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

    /* Find sharp/std/ relative to the sharpc executable */
    {
        char *std_dir = sharp_find_std_dir();
        if (std_dir) {
            sv_push(&user_inc, std_dir);
        } else {
            fprintf(stderr,
                "sharpc: warning: cannot locate sharp/std/ directory.\n"
                "  Set SHARP_ROOT env var pointing to your sharp repository root.\n"
                "  Example: export SHARP_ROOT=/path/to/sharp\n");
        }
    }

    /* ── Phase 1: preprocess each input to C ──────────────────────── */
    int had_error = 0;
    for (size_t fi = 0; fi < inputs.len; fi++) {
        InputFile *inf = &inputs.data[fi];

        /* .o / .obj files are pre-built objects — skip preprocessing */
        if (strcmp(file_ext(inf->path), ".o") == 0 || strcmp(file_ext(inf->path), ".obj") == 0) continue;

        if (action == ACTION_PREPROCESS) {
            /* -E: preprocess and output to stdout or -o file */
            char *pp_out = preprocess_one_file(inf->path, &user_inc, &sys_inc,
                                               &macros, lang_std, target, inf);
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
                                       output, inf);
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
            /* Write .c temp file */
            char *tmp_c = make_tmp_name(inf->path, ".c");
            if (!tmp_c) { ret = 2; goto cleanup; }
            FILE *f = fopen(tmp_c, "w");
            if (!f) { perror(tmp_c); ret = 2; goto cleanup; }
            fputs(inf->tmp_c, f);
            fclose(f);

            const char *out_file = (inputs.len == 1 && output) ? output :
                                   replace_ext(inf->path, ".s");

            const char *zig_argv[] = {zig_exe, "cc", "-S", tmp_c, "-o", out_file, NULL};
            if (g_sess.verbose) {
                fprintf(stderr, "%s", zig_argv[0]);
                for (int ai = 1; zig_argv[ai]; ai++) fprintf(stderr, " %s", zig_argv[ai]);
                fprintf(stderr, "\n");
            }
            if (run_cmd(zig_argv) != 0) {
                fprintf(stderr, "sharpc: assembly generation failed for %s\n", inf->path);
                ret = 3;
                goto cleanup;
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
         * or emit C text if -o target has .c extension. */
        const char *zig_exe = cpp_find_zig_exe();
        if (!zig_exe) {
            fprintf(stderr, "sharpc: zig not found\n");
            ret = 3; goto cleanup;
        }
        /* Check if any -o target has .c extension */
        bool emit_c_only = false;
        if (output && strcmp(file_ext(output), ".c") == 0)
            emit_c_only = true;

        if (emit_c_only && inputs.len == 1) {
            /* Single file, -o *.c → emit C text directly */
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
            char *tmp_c = make_tmp_name(inf->path, ".c");
            if (!tmp_c) { ret = 2; goto cleanup; }
            FILE *f = fopen(tmp_c, "w");
            if (!f) { perror(tmp_c); ret = 2; goto cleanup; }
            fputs(inf->tmp_c, f);
            fclose(f);

            const char *obj_out = (inputs.len == 1 && output &&
                                   strcmp(file_ext(output), ".c") != 0)
                                  ? output :
                                  replace_ext(inf->path, ".o");

            /* Build zig cc argv */
            StrVec zig_args = {0};
            sv_push(&zig_args, zig_exe);
            sv_push(&zig_args, "cc");
            if (target && target[0]) { sv_push(&zig_args, "-target"); sv_push(&zig_args, (char *)target); }
            sv_push(&zig_args, "-c");
            sv_push(&zig_args, tmp_c);
            sv_push(&zig_args, "-o");
            sv_push(&zig_args, obj_out);
            for (size_t li = 0; li < link_other.len; li++)
                sv_push(&zig_args, link_other.data[li]);
            sv_push(&zig_args, NULL);

            if (g_sess.verbose) {
                fprintf(stderr, "%s", zig_args.data[0]);
                for (int ai = 1; zig_args.data[ai]; ai++) fprintf(stderr, " %s", zig_args.data[ai]);
                fprintf(stderr, "\n");
            }
            if (run_cmd(zig_args.data) != 0) {
                fprintf(stderr, "sharpc: compilation failed for %s\n", inf->path);
                free(zig_args.data);
                ret = 3;
                goto cleanup;
            }
            if (g_sess.verbose)
                fprintf(stderr, "sharpc: wrote %s\n", obj_out);
            if (inputs.len != 1 || !output)
                free((void *)obj_out);
            free(zig_args.data);
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

        /* Pre-built object files — add directly to link list */
        if (strcmp(file_ext(inf->path), ".o") == 0 || strcmp(file_ext(inf->path), ".obj") == 0) {
            sv_push(&obj_files, inf->path);
            continue;
        }

        if (!inf->tmp_c) continue;
        char *tmp_c = make_tmp_name(inf->path, ".c");
        if (!tmp_c) { ret = 2; goto cleanup; }
        FILE *f = fopen(tmp_c, "w");
        if (!f) { perror(tmp_c); ret = 2; goto cleanup; }
        fputs(inf->tmp_c, f);
        fclose(f);

        const char *obj_tmp = make_tmp_name(inf->path, ".o");
        if (!obj_tmp) { ret = 2; goto cleanup; }

        StrVec zig_args = {0};
        sv_push(&zig_args, zig_exe);
        sv_push(&zig_args, "cc");
        if (target && target[0]) { sv_push(&zig_args, "-target"); sv_push(&zig_args, (char *)target); }
        sv_push(&zig_args, "-c");
        sv_push(&zig_args, tmp_c);
        sv_push(&zig_args, "-o");
        sv_push(&zig_args, obj_tmp);
        for (size_t li = 0; li < link_other.len; li++)
            sv_push(&zig_args, link_other.data[li]);
        sv_push(&zig_args, NULL);

        if (g_sess.verbose) {
            fprintf(stderr, "[sharpc] ");
            for (int ai = 0; zig_args.data[ai]; ai++) fprintf(stderr, "%s ", zig_args.data[ai]);
            fprintf(stderr, "\n");
        }
        if (run_cmd(zig_args.data) != 0) {
            fprintf(stderr, "sharpc: compilation failed for %s\n", inf->path);
            free(zig_args.data);
            ret = 3;
            goto cleanup;
        }
        free(zig_args.data);

        /* Add obj to link list */
        sv_push(&obj_files, obj_tmp);
    }

    /* ── Link ─────────────────────────────────────────────────────── */
    if (!output && inputs.len == 1) {
        output = replace_ext(inputs.data[0].path, "");
    } else if (!output) {
        output = "a.out";
    }

    StrVec ld_args = {0};
    sv_push(&ld_args, zig_exe);
    sv_push(&ld_args, "cc");
    if (target && target[0]) { sv_push(&ld_args, "-target"); sv_push(&ld_args, (char *)target); }
    sv_push(&ld_args, "-o");
    sv_push(&ld_args, output);
    for (size_t oi = 0; oi < obj_files.len; oi++)
        sv_push(&ld_args, obj_files.data[oi]);
    for (size_t li = 0; li < link_paths.len; li++)
        sv_push(&ld_args, link_paths.data[li]);
    for (size_t li = 0; li < link_libs.len; li++)
        sv_push(&ld_args, link_libs.data[li]);
    for (size_t li = 0; li < link_other.len; li++)
        sv_push(&ld_args, link_other.data[li]);
    sv_push(&ld_args, NULL);

    if (g_sess.verbose) {
        fprintf(stderr, "[sharpc] ");
        for (int ai = 0; ld_args.data[ai]; ai++) fprintf(stderr, "%s ", ld_args.data[ai]);
        fprintf(stderr, "\n");
    }
    if (run_cmd(ld_args.data) != 0) {
        fprintf(stderr, "sharpc: link failed\n");
        free(ld_args.data);
        ret = 3;
        goto cleanup;
    }
    free(ld_args.data);
    free(obj_files.data);

    if (g_sess.verbose)
        fprintf(stderr, "sharpc: wrote %s\n", output);

cleanup:
    free(user_inc.data);
    free(sys_inc.data);
    free(macros.data);
    free(link_libs.data);
    free(link_paths.data);
    free(link_other.data);
    for (size_t i = 0; i < inputs.len; i++) {
        free(inputs.data[i].tmp_c);
        /* included_files may be NULL if depfile block transferred ownership */
        if (inputs.data[i].included_files) {
            for (size_t j = 0; j < inputs.data[i].nincluded; j++)
                free(inputs.data[i].included_files[j]);
            free(inputs.data[i].included_files);
        }
        free(inputs.data[i].included_is_sys);
    }
    free(inputs.data);
    cleanup_tmp_files();
    return ret;

oom:
    fprintf(stderr, "sharpc: out of memory\n");
    free(user_inc.data);
    free(sys_inc.data);
    free(macros.data);
    free(link_libs.data);
    free(link_paths.data);
    free(link_other.data);
    for (size_t i = 0; i < inputs.len; i++) {
        free(inputs.data[i].tmp_c);
        if (inputs.data[i].included_files) {
            for (size_t j = 0; j < inputs.data[i].nincluded; j++)
                free(inputs.data[i].included_files[j]);
            free(inputs.data[i].included_files);
        }
        free(inputs.data[i].included_is_sys);
    }
    free(inputs.data);
    cleanup_tmp_files();
    return 2;
}
