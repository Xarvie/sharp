/*
 * tests/test_cpp_smoke.c — Preprocessor unit smoke tests
 *
 * Links against the cpp/ static library; no file I/O required.
 * Exits 0 on success, non-zero on any failure.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

/* -------------------------------------------------------------------------
 * Minimal test framework
 * ---------------------------------------------------------------------- */
static int g_pass = 0, g_fail = 0;

static void check(const char *desc,
                  const char *input, size_t input_len,
                  const char *expected_substr) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r  = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int ok = r.text && strstr(r.text, expected_substr) != NULL;
    if (ok) {
        printf("  PASS  %s\n", desc);
        g_pass++;
    } else {
        printf("  FAIL  %-40s\n"
               "        expected substr: [%s]\n"
               "        got text:        [%s]\n",
               desc, expected_substr, r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* Run with a pre-define */
static void check_def(const char *desc, const char *def_name,
                      const char *def_val,
                      const char *input, size_t input_len,
                      const char *expected_substr) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_define(ctx, def_name, def_val);
    CppResult r  = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int ok = r.text && strstr(r.text, expected_substr) != NULL;
    if (ok) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %-40s  expected=[%s] got=[%s]\n",
               desc, expected_substr, r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* Run in Sharp mode */
static void check_sharp(const char *desc,
                        const char *input, size_t input_len,
                        const char *expected_substr) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r  = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_SHARP);
    int ok = r.text && strstr(r.text, expected_substr) != NULL;
    if (ok) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %-40s  expected=[%s] got=[%s]\n",
               desc, expected_substr, r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

#define S(literal)  (literal), (sizeof(literal) - 1)

int main(void) {
    printf("--- cpp smoke tests ---\n");

    /* Phase 1-3: tokenisation */
    check("plain passthrough",
          S("int x = 1;\n"), "int x = 1;");

    /* Object-like macro */
    check("object macro",
          S("#define N 42\nint x = N;\n"), "42");

    /* Multi-token body */
    check("multi-token body",
          S("#define PAIR 1,2\nint a[]={PAIR};\n"), "1,2");

    /* Function-like macro */
    check("func macro",
          S("#define SQ(x) ((x)*(x))\nint r=SQ(5);\n"), "((5)*(5))");

    /* Variadic macro */
    check("variadic macro",
          S("#define PR(f,...) printf(f,__VA_ARGS__)\nPR(\"%d\",9);\n"),
          "printf(\"%d\",9)");

    /* Stringification */
    check("stringify",
          S("#define S(x) #x\nconst char*p=S(hi);\n"), "\"hi\"");

    /* Token paste */
    check("token paste",
          S("#define CAT(a,b) a##b\nint CAT(x,y)=0;\n"), "xy");

    /* #undef */
    check("undef",
          S("#define V 7\n#undef V\nint z=V;\n"), "z=V");

    /* #ifdef true branch */
    check("ifdef true",
          S("#define FEAT\n#ifdef FEAT\nint yes=1;\n#endif\n"), "yes=1");

    /* #ifdef false branch suppressed */
    check("ifdef false suppressed",
          S("#ifdef NOPE\nint bad=1;\n#endif\nint ok=2;\n"), "ok=2");
    {
        /* Also verify 'bad' is absent */
        const char *src = "#ifdef NOPE\nint bad=1;\n#endif\nint ok=2;\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        if (r.text && strstr(r.text, "bad") == NULL) {
            printf("  PASS  ifdef false: dead code absent\n"); g_pass++;
        } else {
            printf("  FAIL  ifdef false: dead code leaked into output\n"); g_fail++;
        }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* #ifndef */
    check("ifndef",
          S("#ifndef MISSING\nint here=1;\n#endif\n"), "here=1");

    /* #if with arithmetic */
    check("#if arithmetic",
          S("#if 3*3==9\nint ok=1;\n#endif\n"), "ok=1");

    /* #if with defined() — defined() must be resolved before expansion */
    check("#if defined()",
          S("#define FOO\n#if defined(FOO)\nint yes=1;\n#endif\n"), "yes=1");

    /* #if with !defined() */
    check("#if !defined()",
          S("#if !defined(BAR)\nint yes=1;\n#endif\n"), "yes=1");

    /* #elif */
    check("elif",
          S("#define X 2\n#if X==1\nint a=1;\n#elif X==2\nint a=2;\n#else\nint a=3;\n#endif\n"),
          "a=2");

    /* #else */
    check("else",
          S("#if 0\nint a=1;\n#else\nint a=2;\n#endif\n"), "a=2");

    /* Nested conditionals */
    check("nested cond",
          S("#define A 1\n#if A\n#if A\nint x=1;\n#endif\n#endif\n"), "x=1");

    /* Blue-painting: no infinite recursion */
    check("blue paint",
          S("#define X X+1\nint a=X;\n"), "X+1");

    /* Macro calling macro */
    check("macro-in-macro",
          S("#define A 10\n#define B A+A\nint x=B;\n"), "10");  /* both 10s present */

    /* __LINE__ */
    check("__LINE__",
          S("int ln=__LINE__;\n"), "1");

    /* __FILE__ */
    check("__FILE__",
          S("const char*f=__FILE__;\n"), "\"<test>\"");

    /* __STDC__ */
    check("__STDC__",
          S("#if __STDC__\nint yes=1;\n#endif\n"), "yes=1");

    /* String literal concatenation (phase 6) */
    check("string concat",
          S("const char*s=\"hel\" \"lo\";\n"), "\"hello\"");

    /* #error in dead branch — must NOT fire */
    check("#error in dead branch",
          S("#if 0\n#error should not appear\n#endif\nint ok=1;\n"), "ok=1");

    /* #warning in live branch — text in diags, no abort */
    {
        const char *src = "#warning test warning\nint ok=1;\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        int warned = 0;
        for (size_t i = 0; i < r.ndiags; i++)
            if (r.diags[i].level == CPP_DIAG_WARNING) warned = 1;
        if (warned && r.text && strstr(r.text, "ok=1")) {
            printf("  PASS  #warning fires as warning, not fatal\n"); g_pass++;
        } else {
            printf("  FAIL  #warning handling\n"); g_fail++;
        }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* Command-line -D */
    check_def("-D flag", "N", "99",
              S("int x=N;\n"), "x= 99");

    /* Sharp mode: __SHARP__ defined */
    check_sharp("__SHARP__ in sharp mode",
                S("#if __SHARP__\nint s=1;\n#endif\n"), "s=1");

    /* Sharp mode: __SHARP__ not defined in C mode */
    {
        const char *src = "#if __SHARP__\nint s=1;\n#endif\nint c=2;\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        int has_s = r.text && strstr(r.text, "s=1") != NULL;
        int has_c = r.text && strstr(r.text, "c=2") != NULL;
        if (!has_s && has_c) {
            printf("  PASS  __SHARP__ absent in C mode\n"); g_pass++;
        } else {
            printf("  FAIL  __SHARP__ in C mode: has_s=%d has_c=%d text=[%s]\n",
                   has_s, has_c, r.text ? r.text : "(null)");
            g_fail++;
        }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* Token count sanity */
    {
        const char *src = "int x = 42;\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        /* Expect: int x = 42 ; = 5 tokens (whitespace suppressed by default) */
        if (r.ntokens == 5) {
            printf("  PASS  token count (5 tokens for 'int x = 42;')\n"); g_pass++;
        } else {
            printf("  FAIL  token count: expected 5, got %zu\n", r.ntokens); g_fail++;
        }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* __COUNTER__: sequential, starts at 0 */
    {
        const char *src =
            "int a=__COUNTER__;\n"
            "int b=__COUNTER__;\n"
            "int c=__COUNTER__;\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        int ok = r.text
            && strstr(r.text, "a= 0")   /* object macro emits leading space */
            && strstr(r.text, "b= 1")
            && strstr(r.text, "c= 2");
        /* fallback: check raw digits present in order */
        if (!ok && r.text) {
            /* Accept "a=0" or "a= 0" */
            char *p0 = strstr(r.text, "0");
            char *p1 = p0 ? strstr(p0+1, "1") : NULL;
            char *p2 = p1 ? strstr(p1+1, "2") : NULL;
            ok = (p0 && p1 && p2);
        }
        if (ok) { printf("  PASS  __COUNTER__ sequential\n"); g_pass++; }
        else     { printf("  FAIL  __COUNTER__  text=[%s]\n", r.text?r.text:"(null)"); g_fail++; }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* #pragma once: second inclusion of the same buffer is suppressed */
    {
        /* Write a temp header with #pragma once */
        const char *tmpfile = "_sharp_once_test.h";
        FILE *hf = fopen(tmpfile, "w");
        if (!hf) { printf("  SKIP  #pragma once (cannot create temp file)\n"); }
        else {
            fprintf(hf, "#pragma once\nint once_marker=1;\n");
            fclose(hf);

            char incbuf[256];
            snprintf(incbuf, sizeof incbuf,
                     "#include \"%s\"\n#include \"%s\"\nint done=2;\n",
                     tmpfile, tmpfile);

            CppCtx *ctx = cpp_ctx_new();
            cpp_emit_linemarkers(ctx, false);
            CppResult r = cpp_run_buf(ctx, incbuf, strlen(incbuf), "<t>", CPP_LANG_C);
            /* "once_marker" must appear exactly once */
            int count = 0;
            if (r.text) {
                char *p = r.text;
                while ((p = strstr(p, "once_marker")) != NULL) { count++; p++; }
            }
            if (count == 1) { printf("  PASS  #pragma once (single inclusion)\n"); g_pass++; }
            else { printf("  FAIL  #pragma once: marker appeared %d times\n", count); g_fail++; }
            cpp_result_free(&r); cpp_ctx_free(ctx);
            remove(tmpfile);
        }
    }

    /* __VA_OPT__: expands to tokens when ... is non-empty */
    {
        const char *src =
            "#define LOG(msg, ...) msg __VA_OPT__(,) __VA_ARGS__\n"
            "LOG(hello, 42);\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        int ok = r.text && strstr(r.text, "hello") && strstr(r.text, "42");
        if (ok) { printf("  PASS  __VA_OPT__ non-empty\n"); g_pass++; }
        else     { printf("  FAIL  __VA_OPT__ non-empty  text=[%s]\n", r.text?r.text:"(null)"); g_fail++; }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* __VA_OPT__: expands to nothing when ... is empty */
    {
        const char *src =
            "#define LOG(msg, ...) msg __VA_OPT__(,) __VA_ARGS__\n"
            "LOG(hello);\n";
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        /* Should NOT contain a trailing comma */
        int has_comma = r.text && strstr(r.text, ",") != NULL;
        int has_hello = r.text && strstr(r.text, "hello") != NULL;
        if (has_hello && !has_comma) { printf("  PASS  __VA_OPT__ empty\n"); g_pass++; }
        else { printf("  FAIL  __VA_OPT__ empty  text=[%s]\n", r.text?r.text:"(null)"); g_fail++; }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    /* Linemarker: __LINE__ reflects original source line */
    {
        /* After a #define block, __LINE__ should still be the correct line */
        const char *src =
            "#define X 1\n"
            "#define Y 2\n"
            "int line = __LINE__;\n";   /* line 3 in this buffer */
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_C);
        int ok = r.text && strstr(r.text, "3") != NULL;
        if (ok) { printf("  PASS  __LINE__ after #define\n"); g_pass++; }
        else     { printf("  FAIL  __LINE__ after #define  text=[%s]\n", r.text?r.text:"(null)"); g_fail++; }
        cpp_result_free(&r); cpp_ctx_free(ctx);
    }

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
