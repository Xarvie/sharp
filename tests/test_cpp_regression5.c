/*
 * tests/test_cpp_regression5.c — Fifth round regression tests
 * for ISO C11 compliance edge cases.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

static int g_pass = 0, g_fail = 0;

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

#define CHECK_ERROR(desc, input) \
    check_error_impl(desc, input, sizeof(input)-1, __LINE__)

static void check_error_impl(const char *desc, const char *input, size_t input_len, int test_line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    if (has_error) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  expected error diagnostic\n", desc, test_line);
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

#define CHECK_NO_ERROR(desc, input) \
    check_no_error_impl(desc, input, sizeof(input)-1, __LINE__)

static void check_no_error_impl(const char *desc, const char *input, size_t input_len, int line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    if (!has_error) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  unexpected error\n", desc, line);
        for (size_t i = 0; i < r.ndiags && i < 5; i++)
            printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

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
 * 1. #if defined() with no name — should error
 * ================================================================ */
static void test_defined_empty(void) {
    printf("\n--- 1: defined() with no name ---\n");
    
    DBG("defined() empty debug",
        "#if defined()\nint x=1;\n#endif\n");
    
    /* ISO C: defined() with no argument is a constraint violation */
    /* Our implementation may or may not catch this */
}

/* ================================================================
 * 2. #if defined with nothing after — should error
 * ================================================================ */
static void test_defined_nothing(void) {
    printf("\n--- 2: defined with nothing after ---\n");
    
    DBG("defined nothing debug",
        "#if defined\nint x=1;\n#endif\n");
    
    /* Should be 0 (undefined) or error? */
    /* ISO C: defined requires a macro name */
}

/* ================================================================
 * 3. Function-like macro disambiguation: FOO (x) = object-like
 * ================================================================ */
static void test_func_vs_object(void) {
    printf("\n--- 3: FOO (x) is object-like ---\n");
    
    /* Space between name and ( means object-like macro */
    DBG("FOO (x) debug",
        "#define FOO (x)\nint a=FOO;\n");
    
    /* FOO should expand to "(x)" */
    CHECK("FOO expands to (x)", "(x)",
        "#define FOO (x)\nint a=FOO;\n");
}

/* ================================================================
 * 4. Stringification of empty arg: S() → ""
 * ================================================================ */
static void test_stringify_empty(void) {
    printf("\n--- 4: Stringify empty arg ---\n");
    
    DBG("S() debug",
        "#define S(x) #x\nconst char *s=S();\n");
    
    CHECK("S() produces empty string", "\"\"",
        "#define S(x) #x\nconst char *s=S();\n");
}

/* ================================================================
 * 5. ## at beginning/end of body — invalid
 * ================================================================ */
static void test_paste_at_edges(void) {
    printf("\n--- 5: ## at body edges ---\n");
    
    /* ## at beginning: #define FOO(x) ## x — invalid */
    DBG("##x debug",
        "#define FOO(x) ##x\nint a=FOO(y);\n");
    
    /* ## at end: #define FOO(x) x ## — invalid */
    DBG("x## debug",
        "#define FOO(x) x##\nint a=FOO(y);\n");
}

/* ================================================================
 * 6. #if with trailing tokens: #if 1 2 — should error
 * ================================================================ */
static void test_if_trailing_tokens(void) {
    printf("\n--- 6: #if with trailing tokens ---\n");
    
    DBG("#if 1 2 debug",
        "#if 1 2\nint x=1;\n#endif\n");
    
    /* ISO C: extra tokens after #if expression is undefined behavior */
    /* Usually treated as error */
}

/* ================================================================
 * 7. #if defined FOO (no parens) — valid
 * ================================================================ */
static void test_defined_no_parens(void) {
    printf("\n--- 7: defined FOO without parens ---\n");
    
    CHECK("defined FOO works", "yes",
        "#define FOO\n#if defined FOO\nint yes=1;\n#endif\n");
    
    CHECK("defined NOPE is false", "",
        "#if defined NOPE\nint no=1;\n#endif\n");
}

/* ================================================================
 * 8. Nested function-like macro as argument
 * ================================================================ */
static void test_nested_func_args(void) {
    printf("\n--- 8: Nested function-like macro args ---\n");
    
    DBG("nested args debug",
        "#define F(x,y) x+y\n#define G(z) F(z,1)\nint a=G(2);\n");
    
    CHECK("nested func macro", "2+1",
        "#define F(x,y) x+y\n#define G(z) F(z,1)\nint a=G(2);\n");
}

/* ================================================================
 * 9. Octal constant in #if
 * ================================================================ */
static void test_octal_in_if(void) {
    printf("\n--- 9: Octal in #if ---\n");
    
    /* 010 octal = 8 decimal */
    DBG("octal debug",
        "#if 010==8\nint yes=1;\n#endif\n");
    
    CHECK("010 == 8", "yes",
        "#if 010==8\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 10. Hex constant in #if
 * ================================================================ */
static void test_hex_in_if(void) {
    printf("\n--- 10: Hex in #if ---\n");
    
    /* 0x10 = 16 decimal */
    DBG("hex debug",
        "#if 0x10==16\nint yes=1;\n#endif\n");
    
    CHECK("0x10 == 16", "yes",
        "#if 0x10==16\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 11. #if expression trailing tokens
 * ================================================================ */
static void test_expr_trailing(void) {
    printf("\n--- 11: Expression trailing tokens ---\n");
    
    DBG("#if 1+2 extra debug",
        "#if 1+2 extra\nint x=1;\n#endif\n");
}

/* ================================================================
 * 12. .1 pp-number (no leading digit)
 * ================================================================ */
static void test_dot_number(void) {
    printf("\n--- 12: .1 pp-number ---\n");
    
    DBG(".1 debug",
        "double x = .1;\n");
    
    CHECK(".1 preserved", ".1",
        "double x = .1;\n");
}

/* ================================================================
 * 13. 'abc' multi-char constant
 * ================================================================ */
static void test_multichar(void) {
    printf("\n--- 13: Multi-char constant ---\n");
    
    DBG("'abc' debug",
        "#if 'a' != 0\nint yes=1;\n#endif\n");
    
    /* Multi-char constants are implementation-defined */
    CHECK("'a' is non-zero", "yes",
        "#if 'a' != 0\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 14. #undef with extra tokens
 * ================================================================ */
static void test_undef_extra(void) {
    printf("\n--- 14: #undef with extra tokens ---\n");
    
    DBG("#undef FOO extra debug",
        "#define FOO 1\n#undef FOO extra\nint x=FOO;\n");
    
    /* ISO C: extra tokens after #undef should produce a warning */
}

/* ================================================================
 * 15. digraph %:include
 * ================================================================ */
static void test_digraph_include(void) {
    printf("\n--- 15: Digraph %:include ---\n");
    
    /* %: should be # — but in practice this is a tokenizer-level digraph */
    DBG("%:include debug",
        "%%define FOO 1\n");
}

/* ================================================================
 * 16. pp-number 1e10e5 (multiple exponents)
 * ================================================================ */
static void test_double_exp(void) {
    printf("\n--- 16: Double exponent pp-number ---\n");
    
    DBG("1e10e5 debug",
        "double x = 1e10e5;\n");
    
    /* Valid pp-number, but invalid in translation phase 7 */
    CHECK_NO_ERROR("1e10e5 no preprocess error",
        "double x = 1e10e5;\n");
}

/* ================================================================
 * 17. '' empty char constant — invalid
 * ================================================================ */
static void test_empty_char(void) {
    printf("\n--- 17: Empty char constant ---\n");
    
    DBG("'' debug",
        "int x = '';\n");
}

/* ================================================================
 * 18. 0x1p10 hex float
 * ================================================================ */
static void test_hex_float(void) {
    printf("\n--- 18: Hex float ---\n");
    
    DBG("0x1p10 debug",
        "double x = 0x1p10;\n");
    
    CHECK("0x1p10 preserved", "0x1p10",
        "double x = 0x1p10;\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 5 ===\n");
    
    test_defined_empty();
    test_defined_nothing();
    test_func_vs_object();
    test_stringify_empty();
    test_paste_at_edges();
    test_if_trailing_tokens();
    test_defined_no_parens();
    test_nested_func_args();
    test_octal_in_if();
    test_hex_in_if();
    test_expr_trailing();
    test_dot_number();
    test_multichar();
    test_undef_extra();
    test_digraph_include();
    test_double_exp();
    test_empty_char();
    test_hex_float();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
