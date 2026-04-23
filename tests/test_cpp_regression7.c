/*
 * tests/test_cpp_regression7.c — Seventh round regression tests
 * for deep ISO C11 compliance edge cases found by code review.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cpp.h"

static int g_pass = 0, g_fail = 0;

#define CHECK(desc, expected, input) \
    check_impl(desc, input, strlen(input), expected, __LINE__)

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
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

#define CHECK_NO_ERROR(desc, input) \
    check_no_error_impl(desc, input, strlen(input), __LINE__)

static void check_no_error_impl(const char *desc, const char *input, size_t input_len, int test_line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    if (!has_error) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  unexpected error\n", desc, test_line);
        if (r.ndiags > 0) {
            for (size_t i = 0; i < r.ndiags && i < 3; i++)
                printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
        }
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

#define CHECK_ERROR(desc, input) \
    check_error_impl(desc, input, strlen(input), __LINE__)

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
        printf("  FAIL  %s:%d  expected error\n", desc, test_line);
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

#define DBG(desc, input) \
    debug_print(desc, input, strlen(input), __LINE__)

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
 * 1. Ternary short-circuit: 1 ? 1 : (1/0) — should NOT divide by zero
 * ================================================================ */
static void test_ternary_shortcircuit(void) {
    printf("\n--- 1: Ternary short-circuit ---\n");
    
    /* cond ? a : b — if cond is true, b should NOT be evaluated */
    CHECK_NO_ERROR("ternary true: skip false branch div-by-zero",
        "#if 1 ? 1 : (1/0)\nint yes=1;\n#endif\n");
    
    CHECK_NO_ERROR("ternary false: skip true branch div-by-zero",
        "#if 0 ? (1/0) : 1\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 2. Unary minus of INTMAX_MIN
 * ================================================================ */
static void test_unary_neg_overflow(void) {
    printf("\n--- 2: Unary minus INTMAX_MIN ---\n");
    
    /* -INTMAX_MIN is UB in C. Preprocessor should handle it gracefully. */
    /* This tests that we don't crash or produce wrong results */
    DBG("unary neg INTMAX_MIN",
        "#if -9223372036854775807LL-1LL < 0\nint yes=1;\n#endif\n");
    
    /* The negation of INTMAX_MIN may wrap, but we should at least not crash */
    CHECK_NO_ERROR("unary neg large value no crash",
        "#if -9223372036854775807LL < 0\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 3. Octal escape in character constant
 * ================================================================ */
static void test_octal_escape(void) {
    printf("\n--- 3: Octal escape in char const ---\n");
    
    /* '\101' should be character 'A' = 65 */
    CHECK("octal \\101 equals 'A'", "yes",
        "#if '\\101' == 'A'\nint yes=1;\n#endif\n");
    
    /* '\0' = 0 */
    CHECK("octal \\0 = 0", "yes",
        "#if '\\0' == 0\nint yes=1;\n#endif\n");
    
    /* \377 = 255 */
    CHECK("octal \\377 = 255", "yes",
        "#if '\\377' == 255\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 4. Shift count out of range
 * ================================================================ */
static void test_shift_range(void) {
    printf("\n--- 4: Shift count handling ---\n");
    
    /* In C, shift count >= width of type is UB. Preprocessor should
     * handle it without crashing. */
    DBG("large shift count",
        "#if 1<<63\nint yes=1;\n#endif\n");
    
    CHECK("1<<63 non-zero", "yes",
        "#if 1<<63\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 5. Nested ternary with short-circuit
 * ================================================================ */
static void test_nested_ternary(void) {
    printf("\n--- 5: Nested ternary short-circuit ---\n");
    
    /* 1 ? (0 ? (1/0) : 2) : 3 = 2, should not divide */
    CHECK_NO_ERROR("nested ternary: skip inner div-by-zero",
        "#if 1 ? (0 ? (1/0) : 2) : 3\nint yes=1;\n#endif\n");
    
    /* 0 ? 1 : (1 ? 2 : (1/0)) = 2, should not divide */
    CHECK_NO_ERROR("nested ternary: skip else div-by-zero",
        "#if 0 ? 1 : (1 ? 2 : (1/0))\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 6. Macro expansion in #if with defined()
 * ================================================================ */
static void test_defined_no_expand(void) {
    printf("\n--- 6: defined() no macro expansion ---\n");
    
    /* defined() should NOT expand its argument */
    const char *src =
        "#define FOO BAR\n"
        "#if defined(FOO)\n"
        "int yes=1;\n"
        "#endif\n";
    
    CHECK("defined(FOO) with FOO defined", "yes", src);
    
    /* Even if BAR is defined, defined(FOO) checks FOO not BAR */
    const char *src2 =
        "#define BAR 1\n"
        "#define FOO BAR\n"
        "#if defined(FOO) && !defined(BAZ)\n"
        "int yes=1;\n"
        "#endif\n";
    
    CHECK("defined(FOO) not expanded to defined(BAR)", "yes", src2);
}

/* ================================================================
 * 7. Division by zero in #if produces error
 * ================================================================ */
static void test_div_zero(void) {
    printf("\n--- 7: Division by zero ---\n");
    
    CHECK_ERROR("1/0 produces error",
        "#if 1/0\n#endif\n");
    
    CHECK_ERROR("1%0 produces error",
        "#if 1%0\n#endif\n");
}

/* ================================================================
 * 8. Logical operators short-circuit
 * ================================================================ */
static void test_logical_shortcircuit(void) {
    printf("\n--- 8: Logical operator short-circuit ---\n");
    
    /* 0 && (1/0) — false AND anything = false, right side not evaluated */
    CHECK_NO_ERROR("0 && (1/0) no div-by-zero",
        "#if 0 && (1/0)\nint no=1;\n#else\nint yes=1;\n#endif\n");
    
    /* 1 || (1/0) — true OR anything = true, right side not evaluated */
    CHECK_NO_ERROR("1 || (1/0) no div-by-zero",
        "#if 1 || (1/0)\nint yes=1;\n#endif\n");
    
    /* Check the actual values */
    CHECK("0 && 1 = 0", "no",
        "#if 0 && 1\nint yes=1;\n#else\nint no=1;\n#endif\n");
    
    CHECK("1 || 0 = 1", "yes",
        "#if 1 || 0\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 9. Comma operator in #if (not part of standard but common)
 * ================================================================ */
static void test_comma_in_if(void) {
    printf("\n--- 9: Comma in #if expression ---\n");
    
    /* ISO C11 does NOT allow comma operator in #if expressions.
     * The comma is a top-level separator in function-like macro calls.
     * But we test that expressions with commas are handled gracefully. */
    DBG("comma in #if (non-standard)",
        "#if (1,2)==2\nint yes=1;\n#endif\n");
    
    /* Parenthesized comma expression: (1,2) = 2 is NOT standard C11
     * but some preprocessors accept it. Let's see what ours does. */
}

/* ================================================================
 * 10. Hex escape in character constant
 * ================================================================ */
static void test_hex_escape(void) {
    printf("\n--- 10: Hex escape in char const ---\n");
    
    /* '\x41' should equal 'A' */
    CHECK("hex \\x41 equals 'A'", "yes",
        "#if '\\x41' == 'A'\nint yes=1;\n#endif\n");
    
    /* \x0 = 0 */
    CHECK("hex \\x0 = 0", "yes",
        "#if '\\x0' == 0\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 11. Large pp-number in #if (overflow check)
 * ================================================================ */
static void test_large_ppnumber(void) {
    printf("\n--- 11: Large pp-number ---\n");
    
    /* Very large hex number */
    DBG("large hex number",
        "#if 0xFFFFFFFFFFFFFFFFULL\nint yes=1;\n#endif\n");
    
    CHECK("large hex non-zero", "yes",
        "#if 0xFFFFFFFFFFFFFFFFULL\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 12. Undef in dead branch
 * ================================================================ */
static void test_undef_dead_branch(void) {
    printf("\n--- 12: #undef in dead branch ---\n");
    
    /* #undef in dead branch should NOT execute */
    const char *src =
        "#define FOO 1\n"
        "#if 0\n"
        "#undef FOO\n"
        "#endif\n"
        "#if FOO\n"
        "int yes=1;\n"
        "#endif\n";
    
    CHECK("#undef in dead branch ignored", "yes", src);
}

/* ================================================================
 * 13. Include guard interaction
 * ================================================================ */
static void test_include_guard(void) {
    printf("\n--- 13: Include guard ---\n");
    
    FILE *hf = fopen("r7_guard.sp", "w");
    if (hf) {
        fprintf(hf,
            "#ifndef R7_GUARD_H\n"
            "#define R7_GUARD_H\n"
            "int guarded = 1;\n"
            "#endif\n");
        fclose(hf);
        
        const char *src =
            "#include \"r7_guard.sp\"\n"
            "#include \"r7_guard.sp\"\n"
            "int x = guarded;\n";
        
        /* Should only define guarded once */
        CHECK("include guard prevents double-def", "= 1", src);
        remove("r7_guard.sp");
    } else {
        printf("  SKIP  (cannot create temp file)\n");
    }
}

/* ================================================================
 * 14. Stringification of empty argument
 * ================================================================ */
static void test_stringify_empty(void) {
    printf("\n--- 14: Stringify empty arg ---\n");
    
    /* #X where X is empty → "" */
    CHECK("stringify empty arg = \"\"", "\"\"",
        "#define S(x) #x\nconst char *p = S();\n");
}

/* ================================================================
 * 15. Token paste at boundaries
 * ================================================================ */
static void test_paste_boundary(void) {
    printf("\n--- 15: Token paste boundary ---\n");
    
    /* A ## B where A is first token in body */
    CHECK("paste at start of body", "1",
        "#define P(a,b) a##b\nint x=P(,1);\n");
    
    /* A ## B where B is last token in body */
    CHECK("paste at end of body", "1",
        "#define Q(a) a##\nint x=Q(1);\n");
}

/* ================================================================
 * 16. Multiple defined() in single #if
 * ================================================================ */
static void test_multiple_defined(void) {
    printf("\n--- 16: Multiple defined() ---\n");
    
    CHECK_NO_ERROR("multiple defined()",
        "#if defined(__STDC__) && defined(__STDC_VERSION__)\nint yes=1;\n#endif\n");
    
    CHECK("multiple defined() = true", "yes",
        "#if defined(__STDC__) && defined(__STDC_VERSION__)\nint yes=1;\n#endif\n");
    
    CHECK_NO_ERROR("defined() or defined()",
        "#if defined(NONEXISTENT) || defined(__STDC__)\nint yes=1;\n#endif\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 7 ===\n");
    
    test_ternary_shortcircuit();
    test_unary_neg_overflow();
    test_octal_escape();
    test_shift_range();
    test_nested_ternary();
    test_defined_no_expand();
    test_div_zero();
    test_logical_shortcircuit();
    test_comma_in_if();
    test_hex_escape();
    test_large_ppnumber();
    test_undef_dead_branch();
    test_include_guard();
    test_stringify_empty();
    test_paste_boundary();
    test_multiple_defined();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
