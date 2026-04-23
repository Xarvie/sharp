/*
 * tests/test_cpp_regression2.c — Second round regression tests
 * for bugs identified during code review round 2.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

static int g_pass = 0, g_fail = 0;

/* Flexible check: substring must be present */
#define CHECK(desc, expected, input) \
    check_impl(desc, input, sizeof(input)-1, expected, __LINE__)

static void check_impl(const char *desc, const char *input, size_t input_len,
                       const char *expected, int test_line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int ok = r.text && strstr(r.text, expected) != NULL;
    if (ok) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  expected=[%s]\n", desc, test_line, expected);
        printf("        text=[%s]\n", r.text ? r.text : "(null)");
        if (r.ndiags > 0) {
            for (size_t i = 0; i < r.ndiags && i < 5; i++)
                printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
        }
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* Negative check: substring must NOT be present */
#define CHECK_NOT(desc, unexpected, input) \
    check_not_impl(desc, input, sizeof(input)-1, unexpected, __LINE__)

static void check_not_impl(const char *desc, const char *input, size_t input_len,
                           const char *unexpected, int test_line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int absent = !r.text || strstr(r.text, unexpected) == NULL;
    if (absent) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  unexpected=[%s] was found\n", desc, test_line, unexpected);
        printf("        text=[%s]\n", r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* Debug print: show raw output */
#define DBG(desc, input) \
    debug_print(desc, input, sizeof(input)-1, __LINE__)

static void debug_print(const char *desc, const char *input, size_t input_len, int line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    printf("  DBG   %s:%d\n", desc, line);
    printf("        text=[%s]\n", r.text ? r.text : "(null)");
    if (r.ndiags > 0) {
        for (size_t i = 0; i < r.ndiags && i < 5; i++)
            printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * P0 — macro.c L682-696: Empty argument detection for FOO()
 * ================================================================ */
static void test_p0_empty_args(void) {
    printf("\n--- P0: Empty argument detection ---\n");
    
    /* Simple empty args */
    DBG("FOO() debug",
        "#define FOO() done\nint x=FOO();\n");
    
    CHECK("FOO() empty parens", "done",
        "#define FOO() done\nint x=FOO();\n");
    
    /* Empty args with space: FOO( ) */
    CHECK("FOO( ) empty with space", "done",
        "#define FOO() done\nint x=FOO( );\n");
    
    /* Multi-param with empty first: FOO(,1) */
    DBG("FOO(,1) debug",
        "#define FOO(a,b) a+b\nint x=FOO(,1);\n");
    
    CHECK("FOO(,1) empty first param", "+1",
        "#define FOO(a,b) a+b\nint x=FOO(,1);\n");
    
    /* Multi-param with empty second: FOO(1,) */
    CHECK("FOO(1,) empty second param", "1+",
        "#define FOO(a,b) a+b\nint x=FOO(1,);\n");
    
    /* Multi-param with both empty: FOO(,) */
    CHECK("FOO(,) both params empty", "+",
        "#define FOO(a,b) a+b\nint x=FOO(,);\n");
}

/* ================================================================
 * P1 — pptok.c: 0b binary prefix support
 * ================================================================ */
static void test_p1_binary_prefix(void) {
    printf("\n--- P1: Binary number prefix ---\n");
    
    /* Debug */
    DBG("0b1010 debug", "int x = 0b1010;\n");
    
    /* Simple binary */
    CHECK("0b1010 should be present", "0b1010",
        "int x = 0b1010;\n");
    
    /* Binary with uppercase */
    CHECK("0B1010 should be present", "0B1010",
        "int x = 0B1010;\n");
    
    /* Binary in #if expression */
    CHECK("#if with 0b prefix", "yes",
        "#if 0b1010==10\nint yes=1;\n#endif\n");
}

/* ================================================================
 * P2 — pptok.c: Escape character handling in character constants
 * ================================================================ */
static void test_p2_escape_chars(void) {
    printf("\n--- P2: Escape character handling ---\n");
    
    /* Character constants in #if expressions */
    CHECK("'\\n' in #if", "yes",
        "#if '\\n'==10\nint yes=1;\n#endif\n");
    
    CHECK("'\\t' in #if", "yes",
        "#if '\\t'==9\nint yes=1;\n#endif\n");
    
    CHECK("'\\0' in #if", "yes",
        "#if '\\0'==0\nint yes=1;\n#endif\n");
    
    /* Hex escape */
    CHECK("'\\x41' == 'A'", "yes",
        "#if '\\x41'=='A'\nint yes=1;\n#endif\n");
    
    /* Octal escape */
    CHECK("'\\101' == 'A'", "yes",
        "#if '\\101'=='A'\nint yes=1;\n#endif\n");
}

/* ================================================================
 * Additional: << and >> operator parsing
 * ================================================================ */
static void test_shift_operators(void) {
    printf("\n--- Additional: Shift operators ---\n");
    
    CHECK("<< operator preserved", "<<",
        "int x = a << b;\n");
    
    CHECK(">> operator preserved", ">>",
        "int x = a >> b;\n");
    
    /* Test <<= and >>= are preserved as single tokens */
    DBG("<<= debug", "a <<= 1;\n");
    CHECK("<<= operator preserved", "<<=",
        "a <<= 1;\n");
    
    CHECK(">>= operator preserved", ">>=",
        "a >>= 1;\n");
}

/* ================================================================
 * Additional: #elif after #else
 * ================================================================ */
static void test_elif_after_else(void) {
    printf("\n--- Additional: #elif after #else ---\n");
    
    /* This should produce an error */
    const char *src =
        "#if 0\n"
        "#else\n"
        "#elif 1\n"  /* ERROR: #elif after #else */
        "#endif\n";
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
    
    /* Should have an error diagnostic */
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    
    if (has_error) {
        printf("  PASS  #elif after #else produces error\n"); g_pass++;
    } else {
        printf("  FAIL  #elif after #else: no error diagnostic\n");
        printf("        text=[%s]\n", r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * Additional: -D with = in value
 * ================================================================ */
static void test_define_with_equals(void) {
    printf("\n--- Additional: -D with = in value ---\n");
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    /* -DFOO=a=b should define FOO as "a=b" */
    cpp_define(ctx, "FOO", "a=b");
    
    const char *src = "int x = FOO;\n";
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
    
    if (r.text && strstr(r.text, "a=b") != NULL) {
        printf("  PASS  -DFOO=a=b produces 'a=b'\n"); g_pass++;
    } else {
        printf("  FAIL  -DFOO=a=b: expected [a=b] got [%s]\n",
               r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * Additional: __VA_ARGS__ with zero variadic args
 * ================================================================ */
static void test_va_args_empty(void) {
    printf("\n--- Additional: __VA_ARGS__ empty ---\n");
    
    CHECK("__VA_ARGS__ with no extra args", "hello",
        "#define LOG(msg, ...) msg\nLOG(hello);\n");
    
    /* __VA_OPT__ should expand to nothing when ... is empty */
    CHECK("__VA_OPT__ empty expands to nothing", "hello",
        "#define LOG(msg, ...) msg __VA_OPT__(,)\nLOG(hello);\n");
}

/* ================================================================
 * Additional: Division by zero handling
 * ================================================================ */
static void test_div_by_zero(void) {
    printf("\n--- Additional: Division by zero ---\n");
    
    /* 1/0 should produce error diagnostic, not crash */
    const char *src =
        "#if 1/0\n"
        "int x = 1;\n"
        "#endif\n";
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
    
    /* Should have an error diagnostic */
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    
    if (has_error) {
        printf("  PASS  1/0 produces error diagnostic\n"); g_pass++;
    } else {
        printf("  FAIL  1/0: no error diagnostic\n");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 2 ===\n");
    
    test_p0_empty_args();
    test_p1_binary_prefix();
    test_p2_escape_chars();
    test_shift_operators();
    test_elif_after_else();
    test_define_with_equals();
    test_va_args_empty();
    test_div_by_zero();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
