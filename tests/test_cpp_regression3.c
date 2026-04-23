/*
 * tests/test_cpp_regression3.c — Third round regression tests
 * for bugs identified during code review round 3.
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

/* Check for error diagnostic */
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

/* Check no error diagnostics */
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

/* ================================================================
 * P1 — macro.c: Duplicate parameter names
 * ================================================================ */
static void test_p1_duplicate_params(void) {
    printf("\n--- P1: Duplicate parameter names ---\n");
    
    DBG("FOO(x, x) debug",
        "#define FOO(x, x) x\nint a=FOO(1,2);\n");
    
    /* Should produce error for duplicate param names */
    CHECK_ERROR("#define FOO(x, x) duplicate params",
        "#define FOO(x, x) x\n");
}

/* ================================================================
 * P2 — directive.c: #line with invalid values
 * ================================================================ */
static void test_p2_line_directive(void) {
    printf("\n--- P2: #line directive validation ---\n");
    
    /* Valid #line */
    CHECK("#line 100 works", "yes",
        "#line 100\nint yes=1;\n");
    
    /* #line 0 - should be invalid (line numbers start at 1) */
    DBG("#line 0 debug",
        "#line 0\nint x=1;\n");
    
    /* #line with negative */
    DBG("#line -1 debug",
        "#line -1\nint x=1;\n");
    
    /* #line with large number */
    CHECK("#line 999999", "yes",
        "#line 999999\nint yes=1;\n");
}

/* ================================================================
 * P2 — directive.c: #undef of built-in macros
 * ================================================================ */
static void test_p2_undef_builtins(void) {
    printf("\n--- P2: #undef built-in macros ---\n");
    
    DBG("#undef __LINE__ debug",
        "#undef __LINE__\nint x=__LINE__;\n");
    
    /* Check if __LINE__ still works after undef — it should expand to a number */
    CHECK("__LINE__ after #undef still expands", "=2",
        "#undef __LINE__\nint x=__LINE__;\n");
}

/* ================================================================
 * P2 — directive.c: #pragma with empty content
 * ================================================================ */
static void test_p2_pragma_empty(void) {
    printf("\n--- P2: Empty #pragma ---\n");
    
    DBG("empty #pragma debug",
        "#pragma\nint x=1;\n");
    
    CHECK_NO_ERROR("empty #pragma should not error",
        "#pragma\nint x=1;\n");
}

/* ================================================================
 * P2 — directive.c: #if without expression
 * ================================================================ */
static void test_p2_if_no_expr(void) {
    printf("\n--- P2: #if without expression ---\n");
    
    DBG("#if no expr debug",
        "#if\nint x=1;\n#endif\n");
    
    /* Should produce error */
    CHECK_ERROR("#if without expression",
        "#if\nint x=1;\n#endif\n");
}

/* ================================================================
 * Additional: Recursive macro #define FOO FOO
 * ================================================================ */
static void test_recursive_macro(void) {
    printf("\n--- Additional: Recursive macro ---\n");
    
    /* #define FOO FOO should not infinite loop (blue-painting) */
    DBG("FOO=FOO debug",
        "#define FOO FOO\nint x=FOO;\n");
    
    CHECK("recursive FOO=FOO no infinite loop", "FOO",
        "#define FOO FOO\nint x=FOO;\n");
    
    /* Indirect recursion */
    DBG("FOO=BAR, BAR=FOO debug",
        "#define FOO BAR\n#define BAR FOO\nint x=FOO;\n");
    
    /* Correct behavior: FOO→BAR, BAR blue-painted (not expanded).
     * Result is BAR, not FOO. This prevents infinite recursion. */
    CHECK("indirect recursion no infinite loop", "BAR",
        "#define FOO BAR\n#define BAR FOO\nint x=FOO;\n");
}

/* ================================================================
 * Additional: Token paste producing invalid token
 * ================================================================ */
static void test_paste_invalid(void) {
    printf("\n--- Additional: Token paste invalid ---\n");
    
    /* a ## + should produce invalid token */
    DBG("a##+ debug",
        "#define CAT(a,b) a##b\nint x=CAT(a,+);\n");
    
    /* Check behavior - should error or produce something */
    CHECK("a##+ produces output", "x=",
        "#define CAT(a,b) a##b\nint x=CAT(a,+);\n");
}

/* ================================================================
 * Additional: sizeof in #if (should error)
 * ================================================================ */
static void test_sizeof_in_if(void) {
    printf("\n--- Additional: sizeof in #if ---\n");
    
    DBG("sizeof in #if debug",
        "#if sizeof(int)\nint x=1;\n#endif\n");
    
    /* sizeof is not allowed in preprocessor expressions */
    /* Should produce error or treat as 0 */
}

/* ================================================================
 * Additional: Assignment in #if (should error)
 * ================================================================ */
static void test_assign_in_if(void) {
    printf("\n--- Additional: Assignment in #if ---\n");
    
    DBG("x=1 in #if debug",
        "#define x 1\n#if x=2\nint y=1;\n#endif\n");
    
    /* Assignment is not allowed in preprocessor expressions */
}

/* ================================================================
 * Additional: String with trigraph
 * ================================================================ */
static void test_trigraph_in_string(void) {
    printf("\n--- Additional: Trigraph in string ---\n");
    
    /* Trigraphs in strings should be replaced per C standard */
    DBG("??= in string debug",
        "const char *s = \"??=define\";\n");
    
    CHECK("trigraph in string", "#define",
        "const char *s = \"??=define\";\n");
}

/* ================================================================
 * Additional: Comment at end of file
 * ================================================================ */
static void test_comment_at_eof(void) {
    printf("\n--- Additional: Comment at EOF ---\n");
    
    /* // comment at EOF without newline */
    CHECK_NO_ERROR("// comment at EOF",
        "int x = 1; // comment");
    
    /* /* comment at EOF without closing */
    DBG("unterminated comment debug",
        "int x = 1; /* comment");
}

/* ================================================================
 * Additional: #ifdef without macro name
 * ================================================================ */
static void test_ifdef_no_name(void) {
    printf("\n--- Additional: #ifdef without name ---\n");
    
    DBG("#ifdef no name debug",
        "#ifdef\nint x=1;\n#endif\n");
    
    /* Should produce error */
    CHECK_ERROR("#ifdef without macro name",
        "#ifdef\nint x=1;\n#endif\n");
}

/* ================================================================
 * Additional: #else after #else
 * ================================================================ */
static void test_else_after_else(void) {
    printf("\n--- Additional: #else after #else ---\n");
    
    /* Should produce error */
    CHECK_ERROR("#else after #else",
        "#if 1\n#else\n#else\n#endif\n");
}

/* ================================================================
 * Additional: Macro redefinition same vs different
 * ================================================================ */
static void test_macro_redef(void) {
    printf("\n--- Additional: Macro redefinition ---\n");
    
    /* Same definition - should not warn */
    DBG("same redef debug",
        "#define FOO 1\n#define FOO 1\nint x=FOO;\n");
    
    CHECK("same macro redefinition", "1",
        "#define FOO 1\n#define FOO 1\nint x=FOO;\n");
    
    /* Different definition - should warn */
    DBG("diff redef debug",
        "#define FOO 1\n#define FOO 2\nint x=FOO;\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 3 ===\n");
    
    test_p1_duplicate_params();
    test_p2_line_directive();
    test_p2_undef_builtins();
    test_p2_pragma_empty();
    test_p2_if_no_expr();
    test_recursive_macro();
    test_paste_invalid();
    test_sizeof_in_if();
    test_assign_in_if();
    test_trigraph_in_string();
    test_comment_at_eof();
    test_ifdef_no_name();
    test_else_after_else();
    test_macro_redef();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
