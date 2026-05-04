/*
 * sharp_cli.c — minimal command-line driver for sharp-cpp.
 *
 * Used by the Lua integration smoke tests to preprocess each .c file
 * and report diagnostic counts.  This is a *test fixture*, not a
 * supported public CLI — production users should call cpp_run_buf()
 * via the C API directly.
 *
 * Usage:
 *   sharp_cli <file.c> [-I dir]... [-D name[=value]]...
 *
 * Exit codes:
 *   0  — preprocessed cleanly (fatal=0, error=0)
 *   1  — at least one fatal or error diagnostic
 *   2  — bad command-line arguments / cannot read file
 */
#define _POSIX_C_SOURCE 200809L
#include "cpp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    struct stat st; fstat(fileno(f), &st);
    char *buf = (char*)malloc((size_t)st.st_size + 1);
    size_t nread = fread(buf, 1, (size_t)st.st_size, f);
    buf[nread] = '\0';
    *out_len = nread;
    fclose(f);
    return buf;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file.c> [-I dir]... [-D name[=value]]...\n", argv[0]);
        return 2;
    }
    CppCtx *ctx = cpp_ctx_new();
    cpp_install_target_macros(ctx, "x86_64-linux-gnu");
    cpp_emit_linemarkers(ctx, true);
    const char *outfile = NULL;
    bool dump_tokens = false;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-I") == 0 && i + 1 < argc) {
            cpp_add_user_include(ctx, argv[++i]);
        } else if (strncmp(argv[i], "-I", 2) == 0) {
            cpp_add_user_include(ctx, argv[i] + 2);
        } else if (strcmp(argv[i], "-isystem") == 0 && i + 1 < argc) {
            cpp_add_sys_include(ctx, argv[++i]);
        } else if (strncmp(argv[i], "-isystem", 8) == 0 && argv[i][8] != '\0') {
            /* Phase R18: -isystem/path (concatenated, same as gcc) */
            cpp_add_sys_include(ctx, argv[i] + 8);
        } else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
            char *eq = strchr(argv[++i], '=');
            if (eq) { *eq = '\0'; cpp_define(ctx, argv[i], eq + 1); }
            else      cpp_define(ctx, argv[i], "1");
        } else if (strncmp(argv[i], "-D", 2) == 0) {
            char *spec = argv[i] + 2;
            char *eq = strchr(spec, '=');
            if (eq) { *eq = '\0'; cpp_define(ctx, spec, eq + 1); }
            else      cpp_define(ctx, spec, "1");
        } else if (strcmp(argv[i], "-ansi") == 0 ||
                   strcmp(argv[i], "-std=c89") == 0 ||
                   strcmp(argv[i], "-std=c90") == 0 ||
                   strcmp(argv[i], "-std=iso9899:1990") == 0) {
            /* Phase R9: pre-C95 standard mode.  __STDC_VERSION__ stays
             * undefined; #line range falls back to C90's [1..32767]. */
            cpp_set_lang_std(ctx, 0);
        } else if (strcmp(argv[i], "-std=c99") == 0 ||
                   strcmp(argv[i], "-std=iso9899:1999") == 0) {
            cpp_set_lang_std(ctx, 199901L);
        } else if (strcmp(argv[i], "-std=c11") == 0 ||
                   strcmp(argv[i], "-std=iso9899:2011") == 0) {
            cpp_set_lang_std(ctx, 201112L);
        } else if (strcmp(argv[i], "-P") == 0) {
            /* Suppress linemarker output (gcc -E -P compatible) */
            cpp_emit_linemarkers(ctx, false);
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outfile = argv[++i];
        } else if (strcmp(argv[i], "--tokens") == 0) {
            dump_tokens = true;
        } else if (strcmp(argv[i], "-std=c17") == 0 ||
                   strcmp(argv[i], "-std=c18") == 0 ||
                   strcmp(argv[i], "-std=iso9899:2017") == 0) {
            cpp_set_lang_std(ctx, 201710L);
        } else if (strcmp(argv[i], "-pedantic-errors") == 0 ||
                   strcmp(argv[i], "-pedantic") == 0 ||
                   strcmp(argv[i], "-Wpedantic") == 0) {
            /* Phase R9: silently accept — sharp-cpp's default warning
             * level already covers what GCC's -Wpedantic flags.  A
             * future enhancement could upgrade certain warnings to
             * errors under -pedantic-errors, but the most useful
             * effect (forcing strict ISO C lookup of __STDC_VERSION__)
             * is achieved by combining it with -ansi above.        */
        }
    }
    size_t len; char *src = read_file(argv[1], &len);
    if (!src) { cpp_ctx_free(ctx); return 2; }
    CppResult r = cpp_run_buf(ctx, src, len, argv[1], CPP_LANG_SHARP);
    int errors = 0, warnings = 0, fatals = 0, notes = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        switch (r.diags[i].level) {
        case CPP_DIAG_FATAL:   fatals++; break;
        case CPP_DIAG_ERROR:   errors++; break;
        case CPP_DIAG_WARNING: warnings++; break;
        case CPP_DIAG_NOTE:    notes++; break;
        }
        fprintf(stderr, "%s:%d:%d: %s: %s\n",
            r.diags[i].loc.file ? r.diags[i].loc.file : "?",
            r.diags[i].loc.line, r.diags[i].loc.col,
            r.diags[i].level == CPP_DIAG_FATAL ? "fatal" :
            r.diags[i].level == CPP_DIAG_ERROR ? "error" :
            r.diags[i].level == CPP_DIAG_WARNING ? "warning" : "note",
            r.diags[i].msg ? r.diags[i].msg : "");
    }
    if (r.text) {
        FILE *out = outfile ? fopen(outfile, "wb") : stdout;
        if (out) { fwrite(r.text, 1, r.text_len, out); if (outfile) fclose(out); }
    }
    if (dump_tokens) {
        for (size_t i = 0; i < r.ntokens; i++)
            fprintf(stderr, "[%zu] kind=%d text=%.*s\n",
                    i, (int)r.tokens[i].kind,
                    (int)r.tokens[i].len,
                    r.tokens[i].text ? r.tokens[i].text : "");
    }
    fprintf(stderr, "\n[diagnostics] fatal=%d error=%d warning=%d note=%d output=%zu bytes\n",
            fatals, errors, warnings, notes, r.text_len);
    int rc = (fatals || errors) ? 1 : 0;
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
    free(src);
    return rc;
}
