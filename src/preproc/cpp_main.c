/*
 * cpp_main.c — Standalone Sharp/C Preprocessor driver
 *
 * Usage:
 *   sharp-cpp [options] <input-file>
 *
 * Options:
 *   -o <file>          Write preprocessed output to <file> (default: stdout)
 *   -I <dir>           Add <dir> to user include search path
 *   -isystem <dir>     Add <dir> to system include search path
 *   -D<name>[=<val>]   Define macro (default value: 1)
 *   -U<name>           Undefine macro
 *   -P                 Suppress line markers in output
 *   -C                 Keep comments in output
 *   -W                 Keep whitespace tokens
 *   --sharp            Force Sharp language mode (default: inferred from extension)
 *   --c                Force C language mode
 *   --tokens           Dump token list to stderr (diagnostic)
 *   -h / --help        Print this help
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options] <input>\n"
        "\n"
        "Options:\n"
        "  -o <file>          Output file (default: stdout)\n"
        "  -I <dir>           User include path\n"
        "  -isystem <dir>     System include path\n"
        "  -D<name>[=<val>]   Define macro\n"
        "  -U<name>           Undefine macro\n"
        "  -P                 Suppress line markers\n"
        "  -C                 Keep comments\n"
        "  -W                 Keep whitespace tokens\n"
        "  --sharp            Force Sharp language mode\n"
        "  --c                Force C language mode\n"
        "  --tokens           Dump token list to stderr\n"
        "  -h / --help        Print this help\n",
        prog);
}

int main(int argc, char **argv) {
    const char *input_file  = NULL;
    const char *output_file = NULL;
    bool dump_tokens    = false;
    CppLang lang        = (CppLang)-1; /* -1 = auto-detect from extension */

    CppCtx *ctx = cpp_ctx_new();

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];

        if (strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
            usage(argv[0]); cpp_ctx_free(ctx); return 0;
        } else if (strcmp(a, "-o") == 0) {
            if (++i >= argc) { fprintf(stderr, "-o requires an argument\n"); return 1; }
            output_file = argv[i];
        } else if (strcmp(a, "-I") == 0) {
            if (++i >= argc) { fprintf(stderr, "-I requires an argument\n"); return 1; }
            cpp_add_user_include(ctx, argv[i]);
        } else if (strncmp(a, "-I", 2) == 0 && a[2]) {
            cpp_add_user_include(ctx, a + 2);
        } else if (strcmp(a, "-isystem") == 0) {
            if (++i >= argc) { fprintf(stderr, "-isystem requires an argument\n"); return 1; }
            cpp_add_sys_include(ctx, argv[i]);
        } else if (strncmp(a, "-D", 2) == 0 && a[2]) {
            const char *def  = a + 2;
            const char *eq   = strchr(def, '=');
            if (eq) {
                char name[256];
                size_t nlen = (size_t)(eq - def);
                if (nlen >= sizeof name) nlen = sizeof name - 1;
                memcpy(name, def, nlen); name[nlen] = '\0';
                cpp_define(ctx, name, eq + 1);
            } else {
                cpp_define(ctx, def, NULL);
            }
        } else if (strncmp(a, "-U", 2) == 0 && a[2]) {
            cpp_undefine(ctx, a + 2);
        } else if (strcmp(a, "-P") == 0) {
            cpp_emit_linemarkers(ctx, false);
        } else if (strcmp(a, "-C") == 0) {
            cpp_keep_comments(ctx, true);
        } else if (strcmp(a, "-W") == 0) {
            cpp_keep_whitespace(ctx, true);
        } else if (strcmp(a, "--sharp") == 0) {
            lang = CPP_LANG_SHARP;
        } else if (strcmp(a, "--c") == 0) {
            lang = CPP_LANG_C;
        } else if (strcmp(a, "--tokens") == 0) {
            dump_tokens = true;
        } else if (a[0] == '-') {
            fprintf(stderr, "unknown option: %s\n", a);
            usage(argv[0]); cpp_ctx_free(ctx); return 1;
        } else {
            if (input_file) {
                fprintf(stderr, "multiple input files not supported\n");
                cpp_ctx_free(ctx); return 1;
            }
            input_file = a;
        }
    }

    if (!input_file) {
        fprintf(stderr, "no input file\n");
        usage(argv[0]); cpp_ctx_free(ctx); return 1;
    }

    /* Auto-detect language from extension */
    if ((int)lang == -1) {
        const char *ext = strrchr(input_file, '.');
        if (ext && (strcmp(ext, ".sp") == 0 || strcmp(ext, ".sharp") == 0))
            lang = CPP_LANG_SHARP;
        else
            lang = CPP_LANG_C;
    }

    CppResult res = cpp_run(ctx, input_file, lang);
    cpp_print_diags(&res);

    int exit_code = 0;
    if (res.error) {
        exit_code = 1;
    } else {
        FILE *out = stdout;
        if (output_file) {
            out = fopen(output_file, "wb");
            if (!out) {
                perror(output_file);
                cpp_result_free(&res);
                cpp_ctx_free(ctx);
                return 1;
            }
        }
        fwrite(res.text, 1, res.text_len, out);
        if (output_file) fclose(out);

        if (dump_tokens) {
            for (size_t i = 0; i < res.ntokens; i++) {
                const CppTok *t = &res.tokens[i];
                fprintf(stderr, "[%s] %s:%d:%d  %.*s\n",
                        cpp_tok_kind_name(t->kind),
                        t->loc.file ? t->loc.file : "?",
                        t->loc.line, t->loc.col,
                        (int)t->len, t->text);
            }
        }
    }

    cpp_result_free(&res);
    cpp_ctx_free(ctx);
    return exit_code;
}
