/*
 * tokcmp.c  —  Token comparator for Sharp/C round-trip verification.
 *
 * Two workflows, selected by file extension:
 *
 *   .c  file   (C superset check):
 *     Path A:  source.c  ──────────────── gcc -E ──▶ tokens_A
 *     Path B:  source.c  ── sharpc ──▶ out.c ── gcc -E ──▶ tokens_B
 *     Result:  IDENTICAL | DIFFER | ERROR
 *
 *   .sp  file  (Sharp pipeline check):
 *     Path B:  source.sp ── sharpc ──▶ out.c ── gcc -E ──▶ tokens_B
 *     Path A:  --ref=ref.c ─────────── gcc -E ──▶ tokens_A   (optional)
 *              (if no --ref: just show token count, report success/fail)
 *     Result:  IDENTICAL | DIFFER | COMPILED | ERROR
 *
 * Usage:
 *   ./tokcmp [options] FILE
 *   ./tokcmp [options] --batch DIR
 *
 * Options:
 *   -v, --verbose       show token diff on differ
 *   -q, --quiet         suppress per-file output (only summary)
 *   --ref=FILE          reference C file for .sp comparison
 *   --sharpc=PATH       path to sharpc binary (default: ./sharpc)
 *   --isystem=DIR       add -isystem DIR to gcc and sharpc invocations
 *                       (may be repeated; default: auto-detect gcc paths)
 *   --batch=DIR         compare all .c and .sp files in DIR
 *   --stop-first        stop at first non-identical result
 *   --no-filter-sys     include system-header tokens in comparison
 *   --show-tokens       dump token list for PATH_B (debug)
 *
 * Exit codes:
 *   0  all files identical (or all compiled for .sp without ref)
 *   1  at least one file differs or errors
 *   2  bad arguments / tool not found
 *
 * Token grammar recognised (mirrors gcc -E output):
 *   - identifiers:  [A-Za-z_][A-Za-z_0-9]*
 *   - int literals: 0x… / 0b… / decimal, with uUlL suffixes
 *   - float lits:   digits.digits [eE…] [fFlL]
 *   - strings:      L?"…" (handles \-escapes, no newline)
 *   - char lits:    L?'…'
 *   - operators:    all C multi-char ops (>>=, <<=, ->, ++, --, …)
 *   - punctuators:  all single-char C punctuators
 *   - line markers: # <digits> "file" [flags]  — filtered, used for origin tracking
 *   - comments:     /* … * /  and  //…\n  — always filtered
 *   - whitespace:   always filtered
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <fcntl.h>   /* open() */
#include <stdint.h>  /* uintptr_t, SIZE_MAX */



/* =========================================================================
 * Configuration / limits
 * ======================================================================== */
#define MAX_TOKENS   (1 << 20)   /* 1M tokens per file — enough for sqlite */
#define MAX_ISYS     32          /* max -isystem dirs */
#define MAX_FILES    4096        /* max files in --batch */
#define TOK_BUF_CAP  (64 << 20) /* 64 MB token text pool */

/* =========================================================================
 * Token store
 * ======================================================================== */
typedef struct {
    size_t  *toks;       /* array of byte-offsets into buf (NOT pointers: buf may realloc) */
    size_t   ntoks;
    size_t   toks_cap;
    char    *buf;        /* interned text pool */
    size_t   buf_len;
    size_t   buf_cap;
} TokStore;

/* Resolve offset i to a C string pointer.  Must be called after all pushes
 * that could realloc buf are done for the lifetime of the pointer. */
#define TS_TOK(ts, i)  ((ts)->buf + (ts)->toks[i])

static TokStore *ts_new(void) {
    TokStore *ts = calloc(1, sizeof *ts);
    if (!ts) abort();
    ts->toks_cap = 4096;
    ts->toks     = malloc(ts->toks_cap * sizeof *ts->toks);
    ts->buf_cap  = 256 * 1024;
    ts->buf      = malloc(ts->buf_cap);
    if (!ts->toks || !ts->buf) abort();
    return ts;
}
static void ts_free(TokStore *ts) {
    if (!ts) return;
    free(ts->toks);
    free(ts->buf);
    free(ts);
}

/* Intern a string and push an offset into the token array.
 * Fix (Phase tokcmp-fix): store buf-offsets, not char* pointers.
 * Previously buf could be realloc'd to a new address after pointers were
 * already stored in toks[], leaving every earlier entry dangling. */
static void ts_push(TokStore *ts, const char *s, size_t len) {
    /* grow token offset array */
    if (ts->ntoks == ts->toks_cap) {
        ts->toks_cap *= 2;
        ts->toks = realloc(ts->toks, ts->toks_cap * sizeof *ts->toks);
        if (!ts->toks) abort();
    }
    /* grow text buffer — safe because toks[] holds offsets, not pointers */
    if (ts->buf_len + len + 1 >= ts->buf_cap) {
        ts->buf_cap = ts->buf_cap * 2 + len + 1;
        ts->buf = realloc(ts->buf, ts->buf_cap);
        if (!ts->buf) abort();
    }
    ts->toks[ts->ntoks++] = ts->buf_len;   /* store offset, not pointer */
    memcpy(ts->buf + ts->buf_len, s, len);
    ts->buf[ts->buf_len + len] = '\0';
    ts->buf_len += len + 1;
}

/* =========================================================================
 * Tokeniser  (hand-written, mirrors gcc -E token grammar)
 * ======================================================================== */

/* Return true if path looks like a system/compiler path we should filter. */
static int is_sys_path(const char *p) {
    if (!p || !*p) return 1;
    if (p[0] == '<') return 1;
    if (strncmp(p, "/usr/", 5) == 0) return 1;
    if (strncmp(p, "/lib/", 5) == 0) return 1;
    if (strncmp(p, "/include/", 9) == 0) return 1;
    if (strcmp(p, "<built-in>") == 0 || strcmp(p, "<command-line>") == 0)
        return 1;
    return 0;
}

/*
 * Parse a gcc -E line marker:   # <digits> "<file>" [flags…]
 * Returns 1 if this line is a line marker (caller should skip it),
 * updates *cur_file and *cur_is_user.
 */
static int parse_linemarker(const char *line, char *cur_file, int *cur_is_user,
                             const char *root_path, int filter_sys) {
    const char *p = line;
    if (*p != '#') return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p < '0' || *p > '9') return 0;   /* not a line number → not a marker */
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return 0;
    p++;
    const char *fn_start = p;
    while (*p && *p != '"') p++;
    size_t fn_len = (size_t)(p - fn_start);
    if (fn_len > 0 && fn_len < PATH_MAX - 1) {
        memcpy(cur_file, fn_start, fn_len);
        cur_file[fn_len] = '\0';
    }
    if (!filter_sys) {
        *cur_is_user = 1;
    } else {
        /* User if: matches root_path, or non-system path */
        *cur_is_user = (!is_sys_path(cur_file) ||
                        strcmp(cur_file, root_path) == 0);
    }
    return 1;
}

/* Lex a single C token starting at *pp; advance *pp past the token.
 * Returns pointer to token start and sets *out_len.
 * Returns NULL at end of input. */
static const char *lex_one(const char **pp, size_t *out_len) {
    const char *p = *pp;

    /* skip whitespace */
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (!*p) { *pp = p; return NULL; }

    const char *start = p;

    /* line marker / preprocessor directive */
    if (*p == '#') {
        while (*p && *p != '\n') p++;
        *pp = p;
        *out_len = 0; /* caller discards markers */
        return start; /* signal: it's a # line (length 0 = skip) */
    }

    /* // comment */
    if (p[0] == '/' && p[1] == '/') {
        p += 2;
        while (*p && *p != '\n') p++;
        *pp = p; *out_len = 0; return start;
    }
    /* block comment */
    if (p[0] == '/' && p[1] == '*') {
        p += 2;
        while (*p && !(p[0] == '*' && p[1] == '/')) p++;
        if (*p) p += 2;
        *pp = p; *out_len = 0; return start;
    }

    /* string literal  L?"…" */
    if (*p == 'L' && p[1] == '"') p++;
    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && p[1]) p += 2; else p++;
        }
        if (*p) p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }

    /* char literal  L?'…' */
    if (*p == 'L' && p[1] == '\'') p++;
    if (*p == '\'') {
        p++;
        while (*p && *p != '\'') {
            if (*p == '\\' && p[1]) p += 2; else p++;
        }
        if (*p) p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }

    /* hex / binary / octal / decimal integer literal */
    if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
        while (isxdigit((unsigned char)*p)) p++;
        while (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L') p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }
    if (*p == '0' && (p[1] == 'b' || p[1] == 'B')) {
        p += 2;
        while (*p == '0' || *p == '1') p++;
        while (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L') p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }

    /* float / int starting with digit */
    if (isdigit((unsigned char)*p) || (*p == '.' && isdigit((unsigned char)p[1]))) {
        while (isdigit((unsigned char)*p)) p++;
        if (*p == '.') {
            p++;
            while (isdigit((unsigned char)*p)) p++;
        }
        if (*p == 'e' || *p == 'E') {
            p++;
            if (*p == '+' || *p == '-') p++;
            while (isdigit((unsigned char)*p)) p++;
        }
        while (*p == 'f' || *p == 'F' || *p == 'l' || *p == 'L' ||
               *p == 'u' || *p == 'U') p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }

    /* identifier / keyword */
    if (isalpha((unsigned char)*p) || *p == '_') {
        while (isalnum((unsigned char)*p) || *p == '_') p++;
        *pp = p; *out_len = (size_t)(p - start); return start;
    }

    /* multi-char operators (order matters: longest first) */
    static const char * const MULTI[] = {
        ">>=", "<<=",
        "->", "++", "--",
        "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
        "==", "!=", "<=", ">=",
        "&&", "||",
        "<<", ">>",
        "##", "...",
        NULL
    };
    for (int i = 0; MULTI[i]; i++) {
        size_t n = strlen(MULTI[i]);
        if (strncmp(p, MULTI[i], n) == 0) {
            p += n;
            *pp = p; *out_len = n; return start;
        }
    }

    /* single-char punctuator */
    p++;
    *pp = p; *out_len = 1; return start;
}

/*
 * Tokenise gcc -E output (text), filtering to tokens that originate
 * from root_path (when filter_sys is true).
 * Returns a newly-allocated TokStore.
 */
static TokStore *tokenise(const char *text, const char *root_path,
                           int filter_sys) {
    TokStore *ts = ts_new();
    const char *p = text;
    char cur_file[PATH_MAX] = {0};
    strncpy(cur_file, root_path, PATH_MAX - 1);
    int cur_is_user = 1;

    /*
     * Main loop.  We MUST skip whitespace before checking for '#' so that
     * line markers on the line after a blank line are processed correctly.
     *
     * The bug without this: after "# 1 /tmp/out.c\n\n# 1 /usr/stddef.h\n",
     * the blank '\n' is whitespace; the old code called lex_one() which
     * consumed the blank AND the entire "# 1 /usr/stddef.h" line (returning
     * length 0) WITHOUT calling parse_linemarker().  Result: cur_is_user
     * remained 1, and the subsequent "typedef long int ptrdiff_t;" was
     * incorrectly included as a user token.
     *
     * Fix: skip whitespace explicitly first, then check for '#'.  Any '#'
     * after whitespace in gcc -E output is always a line marker or
     * preprocessor directive, never a user-code '#' token (gcc -E has
     * already expanded all macros and removed all # directives).
     */
    while (*p) {
        /* Skip whitespace first so '#' at start of any line is always caught */
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (!*p) break;

        /* Line marker or preprocessor directive — update cur_file / cur_is_user */
        if (*p == '#') {
            const char *line_start = p;
            while (*p && *p != '\n') p++;
            size_t llen = (size_t)(p - line_start);
            char *line = malloc(llen + 1);
            if (!line) abort();
            memcpy(line, line_start, llen);
            line[llen] = '\0';
            parse_linemarker(line, cur_file, &cur_is_user, root_path, filter_sys);
            free(line);
            if (*p) p++; /* skip newline */
            continue;
        }

        /* Regular content token — lex_one never sees '#' here since we just
         * checked for it above (and lex_one's own '#' handling becomes dead
         * code for well-formed gcc -E output). */
        size_t tlen = 0;
        const char *tok = lex_one(&p, &tlen);
        if (!tok) break;
        if (tlen == 0) continue; /* comment or other skipped sequence */
        if (!cur_is_user) continue;
        ts_push(ts, tok, tlen);
    }
    return ts;
}

/* =========================================================================
 * Subprocess helpers
 * ======================================================================== */

/* Build an argv array from a format-like description.
 * Returns heap-allocated argv (caller must free argv[0] area separately). */
typedef struct {
    char **argv;
    int    argc;
    int    cap;
    char  *strpool;  /* owned strings */
    size_t sp_len, sp_cap;
} ArgVec;

static void av_init(ArgVec *av) {
    memset(av, 0, sizeof *av);
    av->cap = 16;
    av->argv = malloc(av->cap * sizeof *av->argv);
    av->sp_cap = 4096;
    av->strpool = malloc(av->sp_cap);
    if (!av->argv || !av->strpool) abort();
}
static void av_free(ArgVec *av) {
    free(av->argv);
    free(av->strpool);
}
static void av_push(ArgVec *av, const char *s) {
    if (av->argc + 1 >= av->cap) {
        av->cap *= 2;
        av->argv = realloc(av->argv, av->cap * sizeof *av->argv);
        if (!av->argv) abort();
    }
    size_t len = strlen(s);
    if (av->sp_len + len + 1 >= av->sp_cap) {
        av->sp_cap = av->sp_cap * 2 + len + 1;
        /* NOTE: realloc may move strpool; pointers already in argv[] dangle.
         * Use offset-based approach: store offsets not pointers, resolve later. */
        av->strpool = realloc(av->strpool, av->sp_cap);
        if (!av->strpool) abort();
        /* Refix all existing argv[i] pointers relative to new strpool base.
         * We track offsets by storing them as (char *)(uintptr_t)offset. */
    }
    /* Store offset into strpool as a uintptr */
    av->argv[av->argc++] = (char *)(uintptr_t)av->sp_len;
    memcpy(av->strpool + av->sp_len, s, len + 1);
    av->sp_len += len + 1;
}
/* Resolve all argv[i] offsets to real pointers.  Call once before execvp. */
static void av_resolve(ArgVec *av) {
    for (int i = 0; i < av->argc; i++)
        av->argv[i] = av->strpool + (uintptr_t)av->argv[i];
    av->argv[av->argc] = NULL;
}

/*
 * Run a command, capture stdout.
 * Returns heap-allocated stdout text (caller frees), or NULL on exec error.
 * Sets *exit_code.
 */
static char *run_capture(const char **argv, int *exit_code) {
    int pfd[2];
    if (pipe(pfd) < 0) { perror("pipe"); *exit_code = -1; return NULL; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); *exit_code = -1; return NULL; }
    if (pid == 0) {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        /* Redirect stderr to /dev/null */
        int devnull = open("/dev/null", 1);
        if (devnull >= 0) { dup2(devnull, STDERR_FILENO); close(devnull); }
        execvp(argv[0], (char *const *)argv);
        _exit(127);
    }
    close(pfd[1]);

    /* Read stdout into a growing buffer */
    size_t cap = 65536, len = 0;
    char *buf = malloc(cap);
    if (!buf) abort();
    ssize_t n;
    while ((n = read(pfd[0], buf + len, cap - len - 1)) > 0) {
        len += (size_t)n;
        if (len + 1 >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
            if (!buf) abort();
        }
    }
    buf[len] = '\0';
    close(pfd[0]);

    int status = 0;
    waitpid(pid, &status, 0);
    *exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    return buf;
}

/* =========================================================================
 * Global configuration
 * ======================================================================== */
typedef struct {
    const char  *sharpc;            /* path to sharpc binary */
    const char  *isys[MAX_ISYS];    /* -isystem dirs */
    int          nisys;
    int          filter_sys;        /* 1 = filter system headers */
    int          verbose;
    int          quiet;
    int          show_tokens;
    int          stop_first;
} Config;

static Config g_cfg;

static void cfg_init(Config *c) {
    memset(c, 0, sizeof *c);
    c->sharpc     = "./sharpc";
    c->filter_sys = 1;
}

/* Detect default gcc system include paths at startup.
 * Runs: gcc -E -v -x c /dev/null and parses stderr for include dirs.
 * Lines starting with " /" that have no embedded spaces are include dirs.
 * The cc1 invocation line is skipped because it contains spaces.
 */
static void cfg_detect_isys(Config *c) {
    if (c->nisys > 0) return;

    int pfd[2];
    if (pipe(pfd) < 0) return;
    pid_t pid = fork();
    if (pid == 0) {
        close(pfd[0]);
        dup2(pfd[1], STDERR_FILENO);
        close(pfd[1]);
        int dn = open("/dev/null", O_WRONLY);
        if (dn >= 0) { dup2(dn, STDOUT_FILENO); close(dn); }
        char *av[] = {"gcc","-E","-v","-x","c","/dev/null",NULL};
        execvp("gcc", av);
        _exit(127);
    }
    close(pfd[1]);

    /* Read all stderr into a growing buffer */
    size_t cap = 8192, len = 0;
    char *buf = malloc(cap);
    if (!buf) { close(pfd[0]); waitpid(pid, NULL, 0); return; }
    ssize_t nr;
    while ((nr = read(pfd[0], buf + len, cap - len - 1)) > 0) {
        len += (size_t)nr;
        if (len + 1 >= cap) {
            cap *= 2; buf = realloc(buf, cap);
            if (!buf) { close(pfd[0]); waitpid(pid, NULL, 0); return; }
        }
    }
    buf[len] = '\0';
    close(pfd[0]);
    waitpid(pid, NULL, 0);

    /* Parse lines starting with " /" (space + slash) that have no spaces
     * in the path itself (the cc1 invocation line has spaces). */
    char *p = buf;
    while (*p && c->nisys < MAX_ISYS) {
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';
        if (p[0] == ' ' && p[1] == '/' && !strchr(p + 1, ' ')) {
            char *dir = p + 1;
            char *end = dir + strlen(dir) - 1;
            while (end > dir && (*end == ' ' || *end == '\t' || *end == '\r'))
                *end-- = '\0';
            struct stat st;
            if (stat(dir, &st) == 0 && S_ISDIR(st.st_mode))
                c->isys[c->nisys++] = strdup(dir);
        }
        p = nl ? nl + 1 : p + strlen(p);
    }
    free(buf);
}


static void av_push_isys(ArgVec *av, const Config *cfg) {
    for (int i = 0; i < cfg->nisys; i++) {
        av_push(av, "-isystem");
        av_push(av, cfg->isys[i]);
    }
}

/*
 * Run gcc -E on `src_path` and tokenise the output.
 * Extra flags appended before src_path.
 * Returns TokStore* or NULL on error (sets errmsg).
 */
static TokStore *gcc_e_tokenise(const char *src_path, const Config *cfg,
                                 char *errmsg, size_t errmsg_cap) {
    ArgVec av; av_init(&av);
    av_push(&av, "gcc");
    av_push(&av, "-E");
    av_push(&av, "-std=gnu11")  /* tokcmp-fix: use gnu11 so __STRICT_ANSI__ not defined, matching sharpc preprocessor */;
    av_push_isys(&av, cfg);
    av_push(&av, "-o"); av_push(&av, "-");
    av_push(&av, (char *)src_path);
    av_resolve(&av);

    int ec = 0;
    char *out = run_capture((const char **)av.argv, &ec);
    av_free(&av);

    if (ec != 0 || !out) {
        snprintf(errmsg, errmsg_cap, "gcc -E failed (exit %d) on %s", ec, src_path);
        free(out);
        return NULL;
    }
    TokStore *ts = tokenise(out, src_path, cfg->filter_sys);
    free(out);
    return ts;
}

/*
 * Run sharpc on src_path → tmp_c_path, then gcc -E on tmp_c_path.
 * Returns TokStore* or NULL on error.
 */
static TokStore *sharpc_gcc_e_tokenise(const char *src_path,
                                        const char *tmp_c_path,
                                        const Config *cfg,
                                        char *errmsg, size_t errmsg_cap,
                                        int *sharpc_failed) {
    *sharpc_failed = 0;
    /* Step 1: sharpc */
    {
        ArgVec av; av_init(&av);
        av_push(&av, cfg->sharpc);
        av_push_isys(&av, cfg);
        av_push(&av, (char *)src_path);
        av_push(&av, "-o");
        av_push(&av, (char *)tmp_c_path);
        av_resolve(&av);
        int ec = 0;
        char *out = run_capture((const char **)av.argv, &ec);
        av_free(&av);
        free(out);
        if (ec != 0) {
            *sharpc_failed = 1;
            snprintf(errmsg, errmsg_cap,
                     "sharpc failed (exit %d) on %s", ec, src_path);
            return NULL;
        }
    }
    /* Step 2: gcc -E on the generated C */
    {
        ArgVec av; av_init(&av);
        av_push(&av, "gcc");
        av_push(&av, "-E");
        av_push(&av, "-std=gnu11")  /* tokcmp-fix: use gnu11 so __STRICT_ANSI__ not defined, matching sharpc preprocessor */;
        av_push_isys(&av, cfg);
        av_push(&av, "-o"); av_push(&av, "-");
        av_push(&av, (char *)tmp_c_path);
        av_resolve(&av);
        int ec = 0;
        char *out = run_capture((const char **)av.argv, &ec);
        av_free(&av);
        if (ec != 0 || !out) {
            snprintf(errmsg, errmsg_cap,
                     "gcc -E on sharpc output failed (exit %d) for %s",
                     ec, src_path);
            free(out);
            return NULL;
        }
        TokStore *ts = tokenise(out, tmp_c_path, cfg->filter_sys);
        free(out);
        return ts;
    }
}

/* Minimal differ: count changed tokens without external diff.
 * Uses LCS length heuristic for small sequences. */
static int count_differ_lines(const TokStore *a, const TokStore *b) {
    /* Simple approach: count non-matching tokens in zipper walk */
    size_t ai = 0, bi = 0, diff = 0;
    while (ai < a->ntoks && bi < b->ntoks) {
        if (strcmp(TS_TOK(a, ai), TS_TOK(b, bi)) == 0) {
            ai++; bi++;
        } else {
            diff++;
            /* advance whichever side is "shorter" by peeking ahead */
            int adv_a = (ai + 1 < a->ntoks &&
                         strcmp(TS_TOK(a, ai+1), TS_TOK(b, bi)) == 0) ? 1 : 0;
            int adv_b = (bi + 1 < b->ntoks &&
                         strcmp(TS_TOK(a, ai), TS_TOK(b, bi+1)) == 0) ? 1 : 0;
            if (adv_a) ai++; else if (adv_b) bi++; else { ai++; bi++; }
        }
    }
    diff += (a->ntoks - ai) + (b->ntoks - bi);
    return (int)(diff > INT_MAX ? INT_MAX : diff);
}

/* Print a unified-diff-style excerpt of the first N differences */
static void print_diff_excerpt(const TokStore *a, const TokStore *b,
                                int max_diff_toks) {
    size_t ai = 0, bi = 0;
    int shown = 0;
    int ctx_before = 3, ctx_after = 3;
    while ((ai < a->ntoks || bi < b->ntoks) && shown < max_diff_toks) {
        if (ai < a->ntoks && bi < b->ntoks &&
            strcmp(TS_TOK(a, ai), TS_TOK(b, bi)) == 0) {
            ai++; bi++;
            continue;
        }
        /* diff region found — print context + changed tokens */
        size_t ctx_start_a = ai > (size_t)ctx_before ? ai - ctx_before : 0;
        printf("  @@ A:%zu B:%zu @@\n", ai, bi);
        for (size_t k = ctx_start_a; k < ai; k++)
            printf("     %s\n", TS_TOK(a, k));
        while (ai < a->ntoks && bi < b->ntoks &&
               strcmp(TS_TOK(a, ai), TS_TOK(b, bi)) != 0 &&
               shown < max_diff_toks) {
            if (ai < a->ntoks) {
                printf("  -  %s\n", TS_TOK(a, ai++));
                shown++;
            }
            if (bi < b->ntoks) {
                printf("  +  %s\n", TS_TOK(b, bi++));
                shown++;
            }
        }
        /* print ctx_after context */
        int after = 0;
        while (ai < a->ntoks && bi < b->ntoks &&
               strcmp(TS_TOK(a, ai), TS_TOK(b, bi)) == 0 &&
               after < ctx_after) {
            printf("     %s\n", TS_TOK(a, ai++));
            bi++; after++;
        }
        printf("\n");
    }
    if (ai < a->ntoks && bi == b->ntoks)
        printf("  ... +%zu tokens only in A\n", a->ntoks - ai);
    if (bi < b->ntoks && ai == a->ntoks)
        printf("  ... +%zu tokens only in B\n", b->ntoks - bi);
}

/* =========================================================================
 * Comparison engine — types
 * ======================================================================== */

typedef enum {
    CMP_IDENTICAL,
    CMP_DIFFER,
    CMP_COMPILED,   /* .sp only: compiled successfully, no reference to compare */
    CMP_ERR_SHARPC,
    CMP_ERR_GCC_A,
    CMP_ERR_GCC_B,
    CMP_ERR_REF,
} CmpStatus;

typedef struct {
    CmpStatus  status;
    size_t     ntoks_a;
    size_t     ntoks_b;
    long       delta;    /* ntoks_b - ntoks_a */
    int        ndiff_lines;
    char       errmsg[512];
} CmpResult;

/*
 * Compare src_path according to its extension.
 * tmp_dir is a writable scratch directory.
 */
static CmpResult compare_file(const char *src_path, const char *ref_path,
                               const char *tmp_dir, const Config *cfg) {
    CmpResult r = {0};
    char tmp_c[PATH_MAX];
    snprintf(tmp_c, sizeof tmp_c, "%s/out.c", tmp_dir); tmp_c[sizeof tmp_c - 1] = '\0';

    /* Determine mode from extension */
    size_t slen = strlen(src_path);
    int is_sp = (slen >= 3 &&
                 src_path[slen-3] == '.' &&
                 src_path[slen-2] == 's' &&
                 src_path[slen-1] == 'p');

    if (is_sp) {
        /* .sp mode: Path B = sharpc → gcc -E */
        int sharpc_failed = 0;
        TokStore *b = sharpc_gcc_e_tokenise(src_path, tmp_c, cfg,
                                             r.errmsg, sizeof r.errmsg,
                                             &sharpc_failed);
        if (!b) {
            r.status = sharpc_failed ? CMP_ERR_SHARPC : CMP_ERR_GCC_B;
            return r;
        }
        r.ntoks_b = b->ntoks;

        if (ref_path) {
            /* Compare against reference C file */
            TokStore *a = gcc_e_tokenise(ref_path, cfg, r.errmsg, sizeof r.errmsg);
            if (!a) {
                ts_free(b);
                r.status = CMP_ERR_REF;
                return r;
            }
            r.ntoks_a  = a->ntoks;
            r.delta    = (long)b->ntoks - (long)a->ntoks;
            if (a->ntoks == b->ntoks) {
                int same = 1;
                for (size_t i = 0; i < a->ntoks && same; i++)
                    if (strcmp(TS_TOK(a, i), TS_TOK(b, i)) != 0) same = 0;
                r.status = same ? CMP_IDENTICAL : CMP_DIFFER;
            } else {
                r.status = CMP_DIFFER;
            }
            if (r.status == CMP_DIFFER) {
                r.ndiff_lines = count_differ_lines(a, b);
                if (cfg->verbose) print_diff_excerpt(a, b, 80);
            }
            if (cfg->show_tokens)
                for (size_t i = 0; i < b->ntoks; i++)
                    printf("  B[%zu] %s\n", i, TS_TOK(b, i));
            ts_free(a);
        } else {
            /* No reference: just report token count */
            r.status = CMP_COMPILED;
            if (cfg->show_tokens)
                for (size_t i = 0; i < b->ntoks; i++)
                    printf("  B[%zu] %s\n", i, TS_TOK(b, i));
        }
        ts_free(b);
    } else {
        /* .c mode: Path A = direct gcc -E, Path B = sharpc → gcc -E */
        TokStore *a = gcc_e_tokenise(src_path, cfg, r.errmsg, sizeof r.errmsg);
        if (!a) { r.status = CMP_ERR_GCC_A; return r; }

        int sharpc_failed = 0;
        TokStore *b = sharpc_gcc_e_tokenise(src_path, tmp_c, cfg,
                                             r.errmsg, sizeof r.errmsg,
                                             &sharpc_failed);
        if (!b) {
            ts_free(a);
            r.status = sharpc_failed ? CMP_ERR_SHARPC : CMP_ERR_GCC_B;
            return r;
        }
        r.ntoks_a = a->ntoks;
        r.ntoks_b = b->ntoks;
        r.delta   = (long)b->ntoks - (long)a->ntoks;

        if (a->ntoks == b->ntoks) {
            int same = 1;
            for (size_t i = 0; i < a->ntoks && same; i++)
                if (strcmp(TS_TOK(a, i), TS_TOK(b, i)) != 0) same = 0;
            r.status = same ? CMP_IDENTICAL : CMP_DIFFER;
        } else {
            r.status = CMP_DIFFER;
        }
        if (r.status == CMP_DIFFER) {
            r.ndiff_lines = count_differ_lines(a, b);
            if (cfg->verbose) print_diff_excerpt(a, b, 80);
        }
        if (cfg->show_tokens) {
            for (size_t i = 0; i < a->ntoks; i++) printf("  A[%zu] %s\n", i, TS_TOK(a, i));
            printf("  ---\n");
            for (size_t i = 0; i < b->ntoks; i++) printf("  B[%zu] %s\n", i, TS_TOK(b, i));
        }
        ts_free(a);
        ts_free(b);
    }
    return r;
}

/* =========================================================================
 * Output formatting
 * ======================================================================== */

static const char *status_icon(CmpStatus s) {
    switch (s) {
    case CMP_IDENTICAL:   return "✅";
    case CMP_COMPILED:    return "🔷";
    case CMP_DIFFER:      return "❌";
    case CMP_ERR_SHARPC:  return "⚠️ ";
    case CMP_ERR_GCC_A:   return "⚠️ ";
    case CMP_ERR_GCC_B:   return "⚠️ ";
    case CMP_ERR_REF:     return "⚠️ ";
    default:              return "? ";
    }
}
static const char *status_name(CmpStatus s) {
    switch (s) {
    case CMP_IDENTICAL:  return "identical";
    case CMP_COMPILED:   return "compiled";
    case CMP_DIFFER:     return "differ";
    case CMP_ERR_SHARPC: return "sharpc_error";
    case CMP_ERR_GCC_A:  return "gcc_error_A";
    case CMP_ERR_GCC_B:  return "gcc_error_B";
    case CMP_ERR_REF:    return "ref_error";
    default:             return "unknown";
    }
}

static void print_result(const char *name, const CmpResult *r, int is_sp) {
    const char *icon = status_icon(r->status);
    if (r->status == CMP_IDENTICAL) {
        printf("  %s  %-42s  %zu tokens\n", icon, name, r->ntoks_a);
    } else if (r->status == CMP_COMPILED) {
        printf("  %s  %-42s  %zu tokens (no ref)\n", icon, name, r->ntoks_b);
    } else if (r->status == CMP_DIFFER) {
        char sign = r->delta >= 0 ? '+' : ' ';
        printf("  %s  %-42s  A=%zu B=%zu (%c%ld) Δtok=%d\n",
               icon, name,
               r->ntoks_a, r->ntoks_b,
               sign, r->delta,
               r->ndiff_lines);
    } else {
        printf("  %s  %-42s  [%s] %s\n",
               icon, name, status_name(r->status), r->errmsg);
    }
    (void)is_sp;
}

/* =========================================================================
 * Batch mode
 * ======================================================================== */

static int collect_files(const char *dir, char **files, int max_files) {
    DIR *d = opendir(dir);
    if (!d) { perror(dir); return 0; }
    struct dirent *e;
    int n = 0;
    while ((e = readdir(d)) && n < max_files) {
        const char *name = e->d_name;
        size_t nlen = strlen(name);
        int is_c  = nlen > 2 && name[nlen-2] == '.' && name[nlen-1] == 'c';
        int is_sp = nlen > 3 && name[nlen-3] == '.' && name[nlen-2] == 's'
                              && name[nlen-1] == 'p';
        if (!is_c && !is_sp) continue;
        char path[PATH_MAX];
        snprintf(path, sizeof path, "%s/%s", dir, name);
        files[n++] = strdup(path);
    }
    closedir(d);
    /* sort for deterministic output */
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (strcmp(files[i], files[j]) > 0) {
                char *tmp = files[i]; files[i] = files[j]; files[j] = tmp;
            }
    return n;
}

/* =========================================================================
 * Temp directory management
 * ======================================================================== */
static char g_tmp_dir[PATH_MAX];

static void mk_tmp_dir(void) {
    snprintf(g_tmp_dir, sizeof g_tmp_dir, "/tmp/tokcmp_%d", (int)getpid());
    mkdir(g_tmp_dir, 0700);
}
static void rm_tmp_dir(void) {
    /* simple: remove out.c only */
    char p[PATH_MAX];
    snprintf(p, sizeof p, "%s/out.c", g_tmp_dir); p[sizeof p - 1] = '\0';
    unlink(p);
    rmdir(g_tmp_dir);
}

/* =========================================================================
 * Argument parsing + main
 * ======================================================================== */

static void usage(const char *prog) {
    fprintf(stderr,
"Usage: %s [options] FILE\n"
"       %s [options] --batch DIR\n"
"\n"
"Options:\n"
"  -v, --verbose          show token diff on differ\n"
"  -q, --quiet            suppress per-file output\n"
"  --ref=FILE             reference C file for .sp comparison\n"
"  --sharpc=PATH          path to sharpc (default: ./sharpc)\n"
"  --isystem=DIR          add -isystem DIR (repeatable)\n"
"  --batch=DIR            compare all .c and .sp files in DIR\n"
"  --stop-first           stop at first non-identical result\n"
"  --no-filter-sys        include system-header tokens\n"
"  --show-tokens          dump tokenised output (debug)\n"
"\n"
"Exit 0 = all identical/compiled, 1 = differ/error, 2 = arg error.\n",
        prog, prog);
}

int main(int argc, char *argv[]) {
    cfg_init(&g_cfg);

    const char *batch_dir = NULL;
    const char *single    = NULL;
    const char *ref_file  = NULL;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        if (!strcmp(a, "-v") || !strcmp(a, "--verbose"))  { g_cfg.verbose=1; continue; }
        if (!strcmp(a, "-q") || !strcmp(a, "--quiet"))    { g_cfg.quiet=1; continue; }
        if (!strcmp(a, "--no-filter-sys"))                 { g_cfg.filter_sys=0; continue; }
        if (!strcmp(a, "--show-tokens"))                   { g_cfg.show_tokens=1; continue; }
        if (!strcmp(a, "--stop-first"))                    { g_cfg.stop_first=1; continue; }
        if (!strncmp(a, "--sharpc=", 9))  { g_cfg.sharpc = a + 9; continue; }
        if (!strncmp(a, "--ref=", 6))     { ref_file = a + 6; continue; }
        if (!strncmp(a, "--isystem=", 10)) {
            if (g_cfg.nisys < MAX_ISYS) g_cfg.isys[g_cfg.nisys++] = a + 10;
            continue;
        }
        if (!strncmp(a, "--batch=", 8)) { batch_dir = a + 8; continue; }
        if (!strcmp(a, "--batch") && i+1 < argc) { batch_dir = argv[++i]; continue; }
        if (a[0] == '-') { fprintf(stderr, "unknown option: %s\n", a); usage(argv[0]); return 2; }
        single = a;
    }

    if (!single && !batch_dir) { usage(argv[0]); return 2; }

    /* Verify sharpc exists */
    if (access(g_cfg.sharpc, X_OK) != 0) {
        fprintf(stderr, "tokcmp: sharpc not found at '%s' (build with 'make sharpc')\n",
                g_cfg.sharpc);
        return 2;
    }

    /* Auto-detect system include paths if none given */
    if (g_cfg.nisys == 0) cfg_detect_isys(&g_cfg);

    mk_tmp_dir();

    int ret = 0;

    if (single) {
        CmpResult r = compare_file(single, ref_file, g_tmp_dir, &g_cfg);
        print_result(single, &r, 0);
        if (r.status != CMP_IDENTICAL && r.status != CMP_COMPILED)
            ret = 1;
    } else {
        /* Batch mode */
        char *files[MAX_FILES];
        int nf = collect_files(batch_dir, files, MAX_FILES);
        if (nf == 0) {
            fprintf(stderr, "tokcmp: no .c or .sp files found in %s\n", batch_dir);
            ret = 1;
        } else {
            int identical = 0, compiled = 0, differ = 0, errors = 0;
            printf("tokcmp: comparing %d files in %s\n\n", nf, batch_dir);
            for (int i = 0; i < nf; i++) {
                /* Auto-detect per-file reference: foo.sp → foo.ref.c */
                const char *this_ref = ref_file;
                char auto_ref[4096];
                if (!this_ref) {
                    size_t flen = strlen(files[i]);
                    if (flen > 3 && strcmp(files[i] + flen - 3, ".sp") == 0) {
                        snprintf(auto_ref, sizeof auto_ref, "%.*s.ref.c",
                                 (int)(flen - 3), files[i]);
                        struct stat _st;
                        if (stat(auto_ref, &_st) == 0) this_ref = auto_ref;
                    }
                }
                CmpResult r = compare_file(files[i], this_ref, g_tmp_dir, &g_cfg);
                const char *name = strrchr(files[i], '/');
                name = name ? name + 1 : files[i];
                if (!g_cfg.quiet) print_result(name, &r, 0);
                switch (r.status) {
                case CMP_IDENTICAL: identical++; break;
                case CMP_COMPILED:  compiled++;  break;
                case CMP_DIFFER:    differ++;    break;
                default:            errors++;    break;
                }
                free(files[i]);
                if (g_cfg.stop_first &&
                    r.status != CMP_IDENTICAL && r.status != CMP_COMPILED)
                    break;
            }
            printf("\n  identical=%d  compiled=%d  differ=%d  errors=%d  total=%d\n",
                   identical, compiled, differ, errors, nf);
            if (differ > 0 || errors > 0) ret = 1;
        }
    }

    rm_tmp_dir();
    return ret;
}
