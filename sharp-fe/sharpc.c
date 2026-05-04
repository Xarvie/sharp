#define _GNU_SOURCE
/*
 * sharpc.c — Sharp compiler driver.
 *
 * Sharp is a superset of C (cf. C++ ⊃ C).  The same driver compiles
 * `.sp` Sharp source and `.c` C source — input is always parsed as
 * Sharp.  The driver runs preprocess → lex → parse → import → scope →
 * sema → cg and writes ISO C11 to `-o out.c` (or stdout).
 *
 * Usage:
 *   sharpc input [-o out.c] [-I dir] [-isystem dir] [-D NAME[=V]] [-U NAME]
 *                [-P] [-std=c11|c99|c17|...] [--target TRIPLE] [-]
 *
 *   sharpc -                              # read from stdin
 *
 * Preprocessor flags mirror the gcc -E command line, so a typical C
 * build can switch `cc -E` for `sharpc` without restructuring its
 * Makefile.  The following flags are silently accepted as no-ops, so
 * `sharpc` can act as a drop-in compile driver where the build system
 * passes compilation-only flags:
 *   -w -Wall -Wextra -Wpedantic -pedantic -ansi  -O0..-O3  -g  -fPIC
 *
 * Exit codes:
 *   0 = success (generated C11)
 *   1 = compile errors in source
 *   2 = I/O error or bad command line
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "import.h"
#include "cg.h"
#include "sema.h"
#include "parse.h"
#include "lex.h"
#include "cpp.h"
#include "sharp_internal.h"

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
    char *dup = strdup(spec);
    if (!dup) return -1;
    char *eq  = strchr(dup, '=');
    if (eq) { *eq = '\0'; cpp_define(ctx, dup, eq + 1); }
    else      cpp_define(ctx, dup, "1");
    free(dup);
    return 0;
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

/* ── Argument parsing ────────────────────────────────────────────────── */

/* Recognise compilation-only flags that the driver should accept and
 * ignore (so `sharpc` can be substituted for `cc -E` without the build
 * system having to filter them out). */
static int is_ignored_flag(const char *a) {
    if (strcmp(a, "-w")          == 0) return 1;
    if (strcmp(a, "-g")          == 0) return 1;
    if (strcmp(a, "-fPIC")       == 0) return 1;
    if (strcmp(a, "-fpic")       == 0) return 1;
    if (strncmp(a, "-O",   2) == 0)    return 1;   /* -O0..-O3, -Os, -Og */
    if (strncmp(a, "-W",   2) == 0)    return 1;   /* any -W... */
    if (strncmp(a, "-f",   2) == 0)    return 1;   /* any -f... */
    if (strncmp(a, "-m",   2) == 0)    return 1;   /* any -m... (e.g. -m64) */
    if (strcmp(a, "-pedantic")        == 0) return 1;
    if (strcmp(a, "-pedantic-errors") == 0) return 1;
    if (strcmp(a, "-ansi")            == 0) return 1;
    return 0;
}

static void usage(FILE *out) {
    fputs(
"Usage: sharpc input [-o out.c] [options]\n"
"\n"
"Preprocessor options (gcc-compatible):\n"
"  -I dir          add user #include \"...\" search path\n"
"  -isystem dir    add system #include <...> search path\n"
"  -D NAME[=V]     define macro\n"
"  -U NAME         undefine macro\n"
"  -P              suppress linemarkers (default for sharpc)\n"
"  -std=c11|c99|c17|c89|gnu99|gnu11|...\n"
"\n"
"Driver options:\n"
"  -o file         write output to file (default: stdout)\n"
"  --target trip   install target macros (default: x86_64-linux-gnu)\n"
"  -               read source from stdin\n"
"\n"
"Compile-only flags accepted as no-ops: -w -O* -W* -f* -m* -g -fPIC\n"
"  -pedantic -pedantic-errors -ansi\n", out);
}

int main(int argc, char *argv[]) {
    const char *input  = NULL;
    const char *output = NULL;
    const char *target = "x86_64-linux-gnu";
    StrVec      user_inc = {0};   /* -I */
    StrVec      sys_inc  = {0};   /* -isystem */
    MacroVec    macros   = {0};   /* -D / -U */
    long        lang_std = -1;    /* unset → libcpp default */
    int         ret      = 0;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];

        /* ── Driver options ──────────────────────────────────────── */
        if (strcmp(a, "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        } else if (strcmp(a, "--target") == 0 && i + 1 < argc) {
            target = argv[++i];
        } else if (strcmp(a, "--help") == 0 || strcmp(a, "-h") == 0) {
            usage(stdout); ret = 0; goto cleanup;

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

        /* ── -P (suppress linemarkers) — sharpc default ──────────── */
        } else if (strcmp(a, "-P") == 0) {
            /* sharpc always emits -P-style output; this is a no-op. */

        /* ── -std=... ────────────────────────────────────────────── */
        } else if (strncmp(a, "-std=", 5) == 0) {
            const char *s = a + 5;
            if      (!strcmp(s, "c89") || !strcmp(s, "c90") ||
                     !strcmp(s, "iso9899:1990"))                 lang_std = 0;
            else if (!strcmp(s, "c99") || !strcmp(s, "gnu99") ||
                     !strcmp(s, "iso9899:1999"))                 lang_std = 199901L;
            else if (!strcmp(s, "c11") || !strcmp(s, "gnu11") ||
                     !strcmp(s, "iso9899:2011"))                 lang_std = 201112L;
            else if (!strcmp(s, "c17") || !strcmp(s, "c18") ||
                     !strcmp(s, "gnu17") || !strcmp(s, "gnu18") ||
                     !strcmp(s, "iso9899:2017"))                 lang_std = 201710L;
            else {
                fprintf(stderr, "sharpc: unrecognised -std=%s, defaulting to c11\n", s);
                lang_std = 201112L;
            }

        /* ── Stdin / positional input ────────────────────────────── */
        } else if (strcmp(a, "-") == 0) {
            input = a;
        } else if (a[0] != '-') {
            input = a;

        /* ── Silently accepted no-ops ────────────────────────────── */
        } else if (is_ignored_flag(a)) {
            /* drop */

        } else {
            fprintf(stderr, "sharpc: unknown option '%s'\n", a);
            usage(stderr);
            ret = 2;
            goto cleanup;
        }
    }
    if (!input) {
        fprintf(stderr, "sharpc: no input file\n");
        usage(stderr);
        ret = 2;
        goto cleanup;
    }

    char *src = read_file(input);
    if (!src) { ret = 2; goto cleanup; }

    /* ── Pipeline ────────────────────────────────────────────────── */
    CppCtx *cctx = cpp_ctx_new();
    if (cpp_install_target_macros(cctx, target) != 0) {
        fprintf(stderr, "sharpc: unknown target triple '%s'\n", target);
        cpp_ctx_free(cctx); free(src); ret = 2; goto cleanup;
    }
    cpp_emit_linemarkers(cctx, false);
    if (lang_std >= 0) cpp_set_lang_std(cctx, lang_std);

    /* Include paths.  User paths apply only to "..." includes; system
     * paths apply only to <...> includes (libcpp policy). */
    for (size_t i = 0; i < user_inc.len; i++)
        cpp_add_user_include(cctx, user_inc.data[i]);
    for (size_t i = 0; i < sys_inc.len; i++)
        cpp_add_sys_include(cctx, sys_inc.data[i]);

    /* Macros, in command-line order — later -D overrides earlier. */
    for (size_t i = 0; i < macros.len; i++) {
        if (macros.data[i].is_undef) {
            cpp_undefine(cctx, macros.data[i].spec);
        } else if (apply_define(cctx, macros.data[i].spec) < 0) {
            cpp_ctx_free(cctx); free(src); goto oom;
        }
    }

    CppResult r = cpp_run_buf(cctx, src, strlen(src), input, CPP_LANG_SHARP);
    /* NOTE: cpp_ctx_free is deferred to the very end.  Tokens carry
     * CppLoc.file pointers into the cpp string interner; lex_run copies
     * the token text but only shallow-copies CppLoc, so all front-end
     * diagnostics flowing out of lex/parse/sema also point into the
     * interner.  Freeing the ctx here would dangle every loc.file in
     * the diag arrays. */
    free(src);

    /* Print preprocessor diagnostics first.  If any are errors, we stop
     * before lex/parse — the token stream is unreliable past that point. */
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
        cpp_result_free(&r);
        cpp_ctx_free(cctx);
        ret = 1;
        goto cleanup;
    }

    FeDiagArr ld = {0}, pd = {0}, impd = {0}, sd = {0}, sema_d = {0};
    size_t ntoks = 0;
    /* Phase R1: pick lex mode by file extension.  `.sp` is Sharp source
     * (promote `null`/`defer`/`operator`/`this`/`import` to keywords);
     * any other extension (`.c`, `.i`, `.h`, …) is C source where those
     * names must remain identifiers. */
    bool sharp_mode = false;
    {
        size_t ilen = strlen(input);
        if (ilen >= 3 && input[ilen-3] == '.' &&
            input[ilen-2] == 's' && input[ilen-1] == 'p')
            sharp_mode = true;
    }
    SharpTok *toks = lex_run(r.tokens, r.ntokens, input, &ld, &ntoks, sharp_mode);
    AstNode *ast = parse_file(toks, ntoks, input, &pd);

    /* Sharp's `import` statement uses the same -I list as the C
     * preprocessor's "..." search path. */
    ImportCtx *ictx = import_ctx_new();
    for (size_t i = 0; i < user_inc.len; i++)
        import_ctx_add_search_path(ictx, user_inc.data[i]);
    import_resolve(ictx, ast, input, &impd);
    import_ctx_free(ictx);

    Scope *scope = scope_build(ast, &sd);
    TyStore *ts    = ty_store_new();
    SemaCtx *sema  = sema_ctx_new(ts, scope, &sema_d);
    sema_check_file(sema, ast);

    bool had_error = false;
    FeDiagArr *all[] = { &ld, &pd, &impd, &sd, &sema_d };
    for (int a = 0; a < 5; a++) {
        for (size_t i = 0; i < all[a]->len; i++) {
            CppDiag *d = &all[a]->data[i];
            const char *fname = d->loc.file ? d->loc.file : input;
            fprintf(stderr, "%s:%u:%u: %s: %s\n",
                    fname, d->loc.line, d->loc.col,
                    d->level == CPP_DIAG_ERROR ? "error" : "warning",
                    d->msg);
            if (d->level == CPP_DIAG_ERROR) had_error = true;
        }
    }

    int   exit_code = had_error ? 1 : 0;
    char *c_out     = NULL;
    if (!had_error) {
        CgCtx *cg = cg_ctx_new(ts, scope);
        c_out = cg_generate(cg, ast);
        cg_ctx_free(cg);
    }

    sema_ctx_free(sema); ty_store_free(ts);
    scope_free_chain(scope); ast_node_free(ast); lex_free(toks);
    cpp_result_free(&r);
    for (int a = 0; a < 5; a++) {
        for (size_t i = 0; i < all[a]->len; i++) free(all[a]->data[i].msg);
        free(all[a]->data);
    }
    /* Now safe — all diagnostics have been printed and freed, no more
     * references into the cpp string interner remain. */
    cpp_ctx_free(cctx);

    if (c_out) {
        FILE *out = output ? fopen(output, "w") : stdout;
        if (!out) { perror(output ? output : "stdout"); free(c_out); ret = 2; goto cleanup; }
        fputs(c_out, out);
        if (output) fclose(out);
        free(c_out);
    }
    ret = exit_code;

cleanup:
    free(user_inc.data);
    free(sys_inc.data);
    free(macros.data);
    return ret;

oom:
    fprintf(stderr, "sharpc: out of memory\n");
    free(user_inc.data);
    free(sys_inc.data);
    free(macros.data);
    return 2;
}
