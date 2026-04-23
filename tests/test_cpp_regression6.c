/*
 * tests/test_cpp_regression6.c — Sixth round regression tests
 * for complex ISO C11 edge cases.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define CHECK_NO_ERROR(desc, input) \
    check_no_error_impl(desc, input, strlen(input), __LINE__)

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
 * 1. #if 0 with #define inside — FOO should NOT be defined
 * ================================================================ */
static void test_if0_define(void) {
    printf("\n--- 1: #if 0 with #define ---\n");
    
    DBG("#if 0 #define debug",
        "#if 0\n#define DEAD 1\n#endif\nint x=DEAD;\n");
    
    /* DEAD should not be expanded since it's in dead branch */
    CHECK("#if 0 #define not active", "DEAD",
        "#if 0\n#define DEAD 1\n#endif\nint x=DEAD;\n");
}

/* ================================================================
 * 2. #if 1 with #define inside — FOO should be defined
 * ================================================================ */
static void test_if1_define(void) {
    printf("\n--- 2: #if 1 with #define ---\n");
    
    DBG("#if 1 #define debug",
        "#if 1\n#define ALIVE 1\n#endif\nint x=ALIVE;\n");
    
    CHECK("#if 1 #define active", "= 1",
        "#if 1\n#define ALIVE 1\n#endif\nint x=ALIVE;\n");
}

/* ================================================================
 * 3. Nested #if across include boundaries
 * ================================================================ */
static void test_nested_if_include(void) {
    printf("\n--- 3: Nested #if across includes ---\n");
    
    /* Create a header with #endif */
    FILE *hf = fopen("r6_inner.sp", "w");
    if (hf) {
        fprintf(hf, "#define INNER 1\n");
        fclose(hf);
        
        /* Outer file: #if 1 \n #include "r6_inner.sp" \n #endif */
        const char *src =
            "#if 1\n"
            "#include \"r6_inner.sp\"\n"
            "#endif\n"
            "int x = INNER;\n";
        
        DBG("nested #if across include debug",
            "#if 1\n#include \"r6_inner.sp\"\n#endif\nint x=INNER;\n");
    
        CHECK("nested #if across include", "= 1", src);
        remove("r6_inner.sp");
    } else {
        printf("  SKIP  (cannot create temp file)\n");
    }
}

/* ================================================================
 * 4. #ifdef inside #if 0 — should not error
 * ================================================================ */
static void test_ifdef_in_dead(void) {
    printf("\n--- 4: #ifdef inside #if 0 ---\n");
    
    DBG("#ifdef in dead debug",
        "#if 0\n#ifdef FOO\n#endif\n#endif\nint x=1;\n");
    
    CHECK_NO_ERROR("#ifdef inside #if 0",
        "#if 0\n#ifdef FOO\n#endif\n#endif\nint x=1;\n");
}

/* ================================================================
 * 5. #include with macro expanding to quoted string
 * ================================================================ */
static void test_include_macro_string(void) {
    printf("\n--- 5: #include with macro to string ---\n");
    
    FILE *hf = fopen("r6_hdr.sp", "w");
    if (hf) {
        fprintf(hf, "int macro_inc = 42;\n");
        fclose(hf);
        
        const char *src =
            "#define H \"r6_hdr.sp\"\n"
            "#include H\n"
            "int x = macro_inc;\n";
        
        CHECK("#include macro string", "= 42", src);
        remove("r6_hdr.sp");
    } else {
        printf("  SKIP  (cannot create temp file)\n");
    }
}

/* ================================================================
 * 6. #include with macro expanding to angle brackets
 * ================================================================ */
static void test_include_macro_angle(void) {
    printf("\n--- 6: #include with macro to <...> ---\n");
    
    /* Create a temp header file */
    FILE *hf = fopen("r6_sys_hdr.sp", "w");
    if (!hf) {
        printf("  SKIP  (cannot create temp file)\n");
        return;
    }
    fprintf(hf, "int angle_inc = 77;\n");
    fclose(hf);
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_add_sys_include(ctx, ".");
    
    const char *src =
        "#define H <r6_sys_hdr.sp>\n"
        "#include H\n"
        "int x = angle_inc;\n";
    
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
    if (r.text && strstr(r.text, "= 77") != NULL) {
        printf("  PASS  #include macro angle brackets\n"); g_pass++;
    } else {
        printf("  FAIL  #include macro angle brackets:193  expected=[= 77]\n");
        printf("        text=[%s]\n", r.text ? r.text : "(null)");
        if (r.ndiags > 0) {
            for (size_t i = 0; i < r.ndiags && i < 3; i++)
                printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
        }
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
    remove("r6_sys_hdr.sp");
}

/* ================================================================
 * 7. #line affects __LINE__
 * ================================================================ */
static void test_line_affects_line(void) {
    printf("\n--- 7: #line affects __LINE__ ---\n");
    
    DBG("#line affects __LINE__ debug",
        "#line 100\nint x = __LINE__;\n");
    
    /* After #line 100, the next line is numbered 100 per ISO C11 §6.10.4p2 */
    CHECK("#line 100 then __LINE__ = 100", "= 100",
        "#line 100\nint x = __LINE__;\n");
}

/* ================================================================
 * 8. #error diagnostic message
 * ================================================================ */
static void test_error_message(void) {
    printf("\n--- 8: #error message ---\n");
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx,
        "#error this is my error message\n", 38, "<test>", CPP_LANG_C);
    int has_error = 0;
    const char *msg = NULL;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) {
            has_error = 1;
            msg = r.diags[i].msg;
            break;
        }
    }
    if (has_error && msg && strstr(msg, "this is my error message")) {
        printf("  PASS  #error message correct\n"); g_pass++;
    } else {
        printf("  FAIL  #error message: [%s]\n", msg ? msg : "(none)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * 9. #warning vs #error level
 * ================================================================ */
static void test_warning_vs_error(void) {
    printf("\n--- 9: #warning vs #error level ---\n");
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx,
        "#warning just a warning\n", 24, "<test>", CPP_LANG_C);
    int has_warn = 0, has_err = 0;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_WARNING) has_warn = 1;
        if (r.diags[i].level == CPP_DIAG_ERROR) has_err = 1;
    }
    if (has_warn && !has_err) {
        printf("  PASS  #warning is warning level\n"); g_pass++;
    } else {
        printf("  FAIL  #warning: warn=%d err=%d\n", has_warn, has_err);
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * 10. #undef of macro defined with -D
 * ================================================================ */
static void test_undef_cmdline(void) {
    printf("\n--- 10: #undef cmdline macro ---\n");
    
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_define(ctx, "FOO", "1");
    
    const char *src =
        "#undef FOO\n"
        "int x = FOO;\n";
    
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
    if (r.text && strstr(r.text, "FOO") != NULL) {
        printf("  PASS  #undef -D macro works\n"); g_pass++;
    } else {
        printf("  FAIL  #undef -D macro: [%s]\n", r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * 11. Empty directive: just newline after #
 * ================================================================ */
static void test_empty_directive(void) {
    printf("\n--- 11: Empty directive ---\n");
    
    DBG("empty directive debug",
        "#\nint x=1;\n");
    
    CHECK_NO_ERROR("empty directive (just #)",
        "#\nint x=1;\n");
}

/* ================================================================
 * 12. Directive with leading whitespace
 * ================================================================ */
static void test_directive_leading_ws(void) {
    printf("\n--- 12: Directive with leading whitespace ---\n");
    
    DBG("leading ws debug",
        "   #define FOO 1\nint x=FOO;\n");
    
    CHECK("#define with leading ws", "= 1",
        "   #define FOO 1\nint x=FOO;\n");
}

/* ================================================================
 * 13. Operator precedence: 1 | 2 & 3
 * ================================================================ */
static void test_prec_bitwise(void) {
    printf("\n--- 13: Bitwise precedence ---\n");
    
    /* 1 | 2 & 3 = 1 | (2 & 3) = 1 | 2 = 3 */
    DBG("1|2&3 debug",
        "#if 1|2&3==3\nint yes=1;\n#endif\n");
    
    CHECK("1|2&3 == 3 (& before |)", "yes",
        "#if 1|2&3==3\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 14. Ternary associativity: 0 ? 1 : 0 ? 2 : 3 = 3
 * ================================================================ */
static void test_ternary_assoc(void) {
    printf("\n--- 14: Ternary associativity ---\n");
    
    DBG("ternary assoc debug",
        "#if 0?1:0?2:3\nint yes=1;\n#endif\n");
    
    /* 0 ? 1 : (0 ? 2 : 3) = 0 ? 1 : 3 = 3 (true) */
    CHECK("ternary right-associative", "yes",
        "#if 0?1:0?2:3\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 15. UNDEFINED + 1 in #if
 * ================================================================ */
static void test_undefined_plus_one(void) {
    printf("\n--- 15: Undefined identifier + 1 ---\n");
    
    DBG("UNDEF+1 debug",
        "#if UNDEF+1==1\nint yes=1;\n#endif\n");
    
    CHECK("UNDEF+1 == 1", "yes",
        "#if UNDEF+1==1\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 16. Nested parens: ((1+2)*3) = 9
 * ================================================================ */
static void test_nested_parens(void) {
    printf("\n--- 16: Nested parens ---\n");
    
    DBG("((1+2)*3) debug",
        "#if ((1+2)*3)==9\nint yes=1;\n#endif\n");
    
    CHECK("((1+2)*3) == 9", "yes",
        "#if ((1+2)*3)==9\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 17. Large number: 1000000 * 1000000
 * ================================================================ */
static void test_large_mult(void) {
    printf("\n--- 17: Large multiplication ---\n");
    
    DBG("1M*1M debug",
        "#if 1000000*1000000==1000000000000LL\nint yes=1;\n#endif\n");
    
    /* intmax_t should handle this */
    CHECK_NO_ERROR("1000000*1000000 no crash",
        "#if 1000000*1000000\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 18. __VA_ARGS__ with commas
 * ================================================================ */
static void test_va_args_commas(void) {
    printf("\n--- 18: __VA_ARGS__ with commas ---\n");
    
    DBG("__VA_ARGS__ commas debug",
        "#define V(...) __VA_ARGS__\nint x=V(a,b,c);\n");
    
    CHECK("__VA_ARGS__ preserves commas", "a,b,c",
        "#define V(...) __VA_ARGS__\nint x=V(a,b,c);\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 6 ===\n");
    
    test_if0_define();
    test_if1_define();
    test_nested_if_include();
    test_ifdef_in_dead();
    test_include_macro_string();
    test_include_macro_angle();
    test_line_affects_line();
    test_error_message();
    test_warning_vs_error();
    test_undef_cmdline();
    test_empty_directive();
    test_directive_leading_ws();
    test_prec_bitwise();
    test_ternary_assoc();
    test_undefined_plus_one();
    test_nested_parens();
    test_large_mult();
    test_va_args_commas();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
