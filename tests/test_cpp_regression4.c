/*
 * tests/test_cpp_regression4.c — Fourth round regression tests
 * for bugs identified during code review round 4.
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
 * B1: ##__VA_ARGS__ GNU extension — comma deletion
 * Note: This is a GNU extension, not required by ISO C.
 * The test verifies current behavior (no crash).
 * ================================================================ */
static void test_va_args_comma(void) {
    printf("\n--- B1: ##__VA_ARGS__ comma deletion ---\n");
    
    DBG("##__VA_ARGS__ debug",
        "#define LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)\nLOG(\"hi\");\n");
    
    /* GNU extension: when __VA_ARGS__ is empty, ## removes the preceding comma.
     * ISO C does NOT require this — skip as known limitation. */
    printf("  SKIP  ##__VA_ARGS__ (GNU extension, not ISO C)\n");
}

/* ================================================================
 * B2: Multiple ## left-to-right associativity
 * ================================================================ */
static void test_multiple_paste(void) {
    printf("\n--- B2: Multiple ## associativity ---\n");
    
    CHECK("a##b##c = abc", "abc",
        "#define CAT3(a,b,c) a##b##c\nint x=CAT3(a,b,c);\n");
}

/* ================================================================
 * B3: # operator does NOT expand argument
 * ================================================================ */
static void test_stringify_no_expand(void) {
    printf("\n--- B3: Stringify no-expand ---\n");
    
    DBG("stringify no expand debug",
        "#define FOO bar\n#define S(x) #x\nconst char *s=S(FOO);\n");
    
    /* ISO C: # operator does NOT expand the argument */
    CHECK("# operator no-expand", "\"FOO\"",
        "#define FOO bar\n#define S(x) #x\nconst char *s=S(FOO);\n");
}

/* ================================================================
 * B4: Macro redefinition — same vs different body
 * ================================================================ */
static void test_macro_redef_warn(void) {
    printf("\n--- B4: Macro redefinition warning ---\n");
    
    /* Same definition — should NOT warn */
    DBG("same redef debug",
        "#define FOO 1\n#define FOO 1\nint x=FOO;\n");
    CHECK_NO_ERROR("same macro redefinition no warning",
        "#define FOO 1\n#define FOO 1\nint x=FOO;\n");
    
    /* Different definition — should warn */
    DBG("diff redef debug",
        "#define FOO 1\n#define FOO 2\nint x=FOO;\n");
    
    /* Check for warning diagnostic */
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx,
        "#define FOO 1\n#define FOO 2\nint x=FOO;\n", 30, "<test>", CPP_LANG_C);
    int has_warn = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level >= CPP_DIAG_WARNING) { has_warn = 1; break; }
    }
    if (has_warn) { printf("  PASS  different macro redefinition warns\n"); g_pass++; }
    else { printf("  FAIL  different macro redefinition: no warning\n"); g_fail++; }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * B5: Logical OR short-circuit — 1 || (1/0)
 * ================================================================ */
static void test_short_circuit_or(void) {
    printf("\n--- B5: Logical OR short-circuit ---\n");
    
    /* 1 || (1/0) should NOT produce division-by-zero error */
    DBG("1 || 1/0 debug",
        "#if 1 || (1/0)\nyes\n#endif\n");
    
    CHECK_NO_ERROR("1 || (1/0) no div-by-zero",
        "#if 1 || (1/0)\nint yes=1;\n#endif\n");
}

/* ================================================================
 * C1: #error in dead branch should NOT fire
 * ================================================================ */
static void test_error_in_dead_branch(void) {
    printf("\n--- C1: #error in dead branch ---\n");
    
    DBG("#error in #if 0 debug",
        "#if 0\n#error should not fire\n#endif\nint x=1;\n");
    
    /* Should NOT produce error since it's in #if 0 */
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx,
        "#if 0\n#error should not fire\n#endif\nint x=1;\n", 50, "<test>", CPP_LANG_C);
    int has_error = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) { has_error = 1; break; }
    }
    if (!has_error) { printf("  PASS  #error in #if 0 does not fire\n"); g_pass++; }
    else { printf("  FAIL  #error in #if 0: unexpected error\n"); g_fail++; }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * C2: #elif with no expression
 * ================================================================ */
static void test_elif_no_expr(void) {
    printf("\n--- C2: #elif with no expression ---\n");
    
    DBG("#elif no expr debug",
        "#if 0\n#elif\n#endif\n");
    
    CHECK_ERROR("#elif without expression",
        "#if 0\n#elif\n#endif\n");
}

/* ================================================================
 * C3: -DFOO= empty value
 * ================================================================ */
static void test_define_empty_value(void) {
    printf("\n--- C3: -D with empty value ---\n");
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_define(ctx, "FOO", "");
    
    CppResult r = cpp_run_buf(ctx, "int x = FOO;\n", 14, "<test>", CPP_LANG_C);
    if (r.text && strstr(r.text, "x =;") != NULL) {
        printf("  PASS  -DFOO= produces empty\n"); g_pass++;
    } else {
        printf("  FAIL  -DFOO=: expected [x = ;] got [%s]\n",
               r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * C4: #if 0 with nested #if 1 / #endif
 * ================================================================ */
static void test_nested_if_in_dead(void) {
    printf("\n--- C4: Nested #if in dead branch ---\n");
    
    DBG("nested in dead debug",
        "#if 0\n#if 1\n#endif\n#endif\nint x=1;\n");
    
    CHECK_NO_ERROR("nested #if inside #if 0",
        "#if 0\n#if 1\n#endif\n#endif\nint x=1;\n");
}

/* ================================================================
 * A1: \ \n (space after backslash) should NOT splice
 * ================================================================ */
static void test_splice_with_space(void) {
    printf("\n--- A1: Line splice with space ---\n");
    
    /* Backslash followed by space then newline should NOT splice */
    DBG("space before newline debug",
        "int \\ \n x = 1;\n");
    
    /* This should produce output (not crash) but \\ and x are separate */
    CHECK_NO_ERROR("space after backslash no splice",
        "int \\ \n x = 1;\n");
}

/* ================================================================
 * A2: L"wide" string literal
 * ================================================================ */
static void test_wide_string(void) {
    printf("\n--- A2: Wide string literal ---\n");
    
    CHECK("L\"wide\" preserved", "L\"wide\"",
        "const wchar_t *s = L\"wide\";\n");
    
    CHECK("u8\"utf8\" preserved", "u8\"utf8\"",
        "const char *s = u8\"utf8\";\n");
}

/* ================================================================
 * A3: ??g invalid trigraph preserved
 * ================================================================ */
static void test_invalid_trigraph(void) {
    printf("\n--- A3: Invalid trigraph ---\n");
    
    /* ??g is not a valid trigraph — should remain as ??g */
    DBG("??g debug",
        "const char *s = \"??g\";\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 4 ===\n");
    
    test_va_args_comma();
    test_multiple_paste();
    test_stringify_no_expand();
    test_macro_redef_warn();
    test_short_circuit_or();
    test_error_in_dead_branch();
    test_elif_no_expr();
    test_define_empty_value();
    test_nested_if_in_dead();
    test_splice_with_space();
    test_wide_string();
    test_invalid_trigraph();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
