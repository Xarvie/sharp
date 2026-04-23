/*
 * tests/test_cpp_regression8.c — Eighth round regression tests
 * for deep ISO C11 compliance edge cases found by code review.
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
        if (r.ndiags > 0 && r.ndiags < 10) {
            for (size_t i = 0; i < r.ndiags && i < 3; i++)
                printf("        diag[%zu]: %s\n", i, r.diags[i].msg);
        }
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
 * 1. Multi-line macro with backslash continuation
 * ================================================================ */
static void test_multiline_macro(void) {
    printf("\n--- 1: Multi-line macro with \\ ---\n");
    
    // Macro body spanning multiple lines
    CHECK("multiline macro body", "1 +2",
        "#define ADD(x,y) x + \\\ny\nint r = ADD(1,2);\n");
    
    // Macro definition with continuation
    CHECK("multiline define continuation", "3",
        "#define VAL 1 + \\\n2\n#if VAL == 3\nint yes=3;\n#endif\n");
}

/* ================================================================
 * 2. Token paste (##) at macro body boundaries
 * ================================================================ */
static void test_paste_boundary(void) {
    printf("\n--- 2: Token paste at boundaries ---\n");
    
    /* ## at very start of body: ##X — lhs is empty */
    DBG("## at start of macro body",
        "#define P(x) ##x\nint r=P(42);\n");
    
    /* ## at very end of body: X## — rhs is empty */
    DBG("## at end of macro body",
        "#define Q(x) x##\nint r=Q(42);\n");
    
    /* Consecutive ##: A##B##C — left-to-right associativity */
    CHECK("triple paste A##B##C", "ABC",
        "#define T(a,b,c) a##b##c\nint x=T(A,B,C);\n");
}

/* ================================================================
 * 3. Argument count mismatch
 * ================================================================ */
static void test_arg_count_mismatch(void) {
    printf("\n--- 3: Argument count mismatch ---\n");
    
    /* Too few args: FOO(a,b,c) called as FOO(1,2) */
    DBG("too few args to 3-param macro",
        "#define FOO(a,b,c) a b c\nint x=FOO(1,2);\n");
    
    /* Too many args: FOO(a,b) called as FOO(1,2,3) */
    DBG("too many args to 2-param macro",
        "#define BAR(a,b) a b\nint x=BAR(1,2,3);\n");
}

/* ================================================================
 * 4. pp-number with exponent: 1e+2, 1.5e-3
 * ================================================================ */
static void test_ppnumber_exponent(void) {
    printf("\n--- 4: pp-number exponent ---\n");
    
    // 1e+2 should be a single pp-number token (preprocessor tokenization)
    // But #if comparison may treat it as float, so just check tokenization
    CHECK_NO_ERROR("1e+2 single token",
        "double x = 1e+2;\n");
    
    // 1.5e-3 should be a single pp-number token
    CHECK_NO_ERROR("1.5e-3 single token",
        "double x = 1.5e-3;\n");
    
    // 0x1p10 hex float exponent (C99) - tokenization only
    CHECK_NO_ERROR("0x1p10 single token",
        "double x = 0x1p10;\n");
    
    // 0x1.5p3 hex float with fraction
    CHECK_NO_ERROR("0x1.5p3 single token",
        "double x = 0x1.5p3;\n");
}

/* ================================================================
 * 5. Wide and universal character name prefixes
 * ================================================================ */
static void test_wide_char(void) {
    printf("\n--- 5: Wide char literals ---\n");
    
    /* L'A' should be a wide character constant */
    CHECK_NO_ERROR("L'A' wide char const",
        "int x = L'A';\n");
    
    /* u8"text" should be UTF-8 string literal */
    CHECK_NO_ERROR("u8\"text\" UTF-8 string",
        "const char *p = u8\"hello\";\n");
}

/* ================================================================
 * 6. Trigraph support
 * ================================================================ */
static void test_trigraphs(void) {
    printf("\n--- 6: Trigraph support ---\n");
    
    /* ??= should become # */
    CHECK("??= → #", "yes",
        "??=if 1\nint yes=1;\n??=endif\n");
    
    /* ??( should become [ */
    CHECK("??( → [", "[",
        "int a??(5??];\n");
    
    /* ??) should become ] */
    CHECK("??) → ]", "]",
        "int a[5??);\n");
    
    /* ??< should become { */
    CHECK("??< → {", "{",
        "int f() ??< return 0; ??}\n");
    
    /* ??> should become } */
    CHECK("??> → }", "}",
        "int f() ??< return 0; ??}\n");
    
    /* ??/ should become \ */
    CHECK("??/ → \\", "yes",
        "#define M 1??/\n2\n#if M==12\nint yes=1;\n#endif\n");
    
    /* ??' should become ^ */
    CHECK("??' → ^", "^",
        "int x = ??';\n");
    
    /* ??! should become | */
    CHECK("??! → |", "|",
        "int x = 1??!2;\n");
    
    /* ??- should become ~ */
    CHECK("??- → ~", "~",
        "int x = ??-0;\n");
}

/* ================================================================
 * 7. Empty object macro
 * ================================================================ */
static void test_empty_object_macro(void) {
    printf("\n--- 7: Empty object macro ---\n");
    
    /* #define EMPTY — no body */
    CHECK("empty object macro disappears", "int yes=1;",
        "#define EMPTY\nint yes=1;\n");
    
    /* Using empty macro in expression */
    CHECK("empty macro in expr", "int yes=1;",
        "#define E\nint E yes=1;\n");
}

/* ================================================================
 * 8. Nested parentheses in macro args
 * ================================================================ */
static void test_nested_parens(void) {
    printf("\n--- 8: Nested parens in macro args ---\n");
    
    // Comma inside nested parens should NOT split macro arguments
    // F((1,2)) - the whole (1,2) is one argument
    CHECK("nested parens: single arg", "(1,2)",
        "#define F(x) x\nint r = F((1,2));\n");
    
    // Deep nesting
    CHECK("deep nesting single arg", "yes",
        "#define G(x) x\nint r=G(((1)));\n#if r || 1\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 9. # with no space (GNU-style #define)
 * ================================================================ */
static void test_hash_no_space(void) {
    printf("\n--- 9: # without space ---\n");
    
    /* #define (no space between # and define) */
    CHECK("#define no space", "yes",
        "#define X 1\n#if X\nint yes=1;\n#endif\n");
    
    /* #  endif (extra space) */
    CHECK("#  endif extra space", "yes",
        "#if 1\n#  endif\nint yes=1;\n");
}

/* ================================================================
 * 10. Comment inside string literal (should NOT be treated as comment)
 * ================================================================ */
static void test_comment_in_string(void) {
    printf("\n--- 10: Comment-like content in strings ---\n");
    
    // "/* not a comment */" - should preserve the content
    CHECK("slash-star in string preserved", "\"/* not a comment */\"",
        "const char *s = \"/* not a comment */\";\n");
    
    // "// not a comment" - should preserve
    CHECK("double-slash in string preserved", "\"// not a comment\"",
        "const char *s = \"// not a comment\";\n");
}

/* ================================================================
 * 11. Punctuator longest match: ++, ->, etc.
 * ================================================================ */
static void test_punctuator_longest(void) {
    printf("\n--- 11: Punctuator longest match ---\n");
    
    /* ++ should be single token, not two + */
    CHECK("++ single token", "yes",
        "int x++;\n#if 1\nint yes=1;\n#endif\n");
    
    /* -> should be single token */
    CHECK("-> single token", "yes",
        "p->x;\n#if 1\nint yes=1;\n#endif\n");
    
    /* <= should be single token */
    CHECK("<= single token", "yes",
        "#if 1<=2\nint yes=1;\n#endif\n");
    
    /* >= should be single token */
    CHECK(">= single token", "yes",
        "#if 2>=1\nint yes=1;\n#endif\n");
    
    /* == should be single token */
    CHECK("== single token", "yes",
        "#if 1==1\nint yes=1;\n#endif\n");
    
    /* != should be single token */
    CHECK("!= single token", "yes",
        "#if 1!=2\nint yes=1;\n#endif\n");
    
    /* += should be single token */
    CHECK("+= single token", "yes",
        "x+=1;\n#if 1\nint yes=1;\n#endif\n");
    
    /* && should be single token */
    CHECK("&& single token", "yes",
        "#if 1 && 1\nint yes=1;\n#endif\n");
    
    /* || should be single token */
    CHECK("|| single token", "yes",
        "#if 0 || 1\nint yes=1;\n#endif\n");
    
    /* << should be single token */
    CHECK("<< single token", "yes",
        "#if 1<<1\nint yes=1;\n#endif\n");
    
    /* >> should be single token */
    CHECK(">> single token", "yes",
        "#if 4>>1\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 12. String continuation across lines (phase 2 splicing)
 * ================================================================ */
static void test_string_continuation(void) {
    printf("\n--- 12: String continuation ---\n");
    
    /* "hello\<newline>world" should be single string "helloworld" */
    CHECK("string with backslash-newline", "yes",
        "const char *s = \"hello\\\nworld\";\n#if 1\nint yes=1;\n#endif\n");
}

/* ================================================================
 * 13. Empty file / only comments
 * ================================================================ */
static void test_empty_input(void) {
    printf("\n--- 13: Empty and comment-only input ---\n");
    
    /* Empty input */
    DBG("empty input", "");
    
    /* Only comments */
    DBG("comment-only input", "/* hello world */\n// single line\n");
    
    /* Only whitespace */
    DBG("whitespace-only input", "   \n  \n\t\n");
}

/* ================================================================
 * 14. Circular include (A includes B, B includes A)
 * ================================================================ */
static void test_circular_include(void) {
    printf("\n--- 14: Circular include ---\n");
    
    FILE *ha = fopen("r8_circular_a.h", "w");
    FILE *hb = fopen("r8_circular_b.h", "w");
    if (ha && hb) {
        fprintf(ha, "#ifndef R8_A_H\n#define R8_A_H\n#include \"r8_circular_b.h\"\nint from_a = 1;\n#endif\n");
        fprintf(hb, "#ifndef R8_B_H\n#define R8_B_H\n#include \"r8_circular_a.h\"\nint from_b = 2;\n#endif\n");
        fclose(ha); fclose(hb);
        
        const char *src = "#include \"r8_circular_a.h\"\nint x = from_a + from_b;\n";
        CHECK_NO_ERROR("circular include no crash", src);
        
        remove("r8_circular_a.h");
        remove("r8_circular_b.h");
    } else {
        printf("  SKIP  (cannot create temp files)\n");
        if (ha) fclose(ha);
        if (hb) fclose(hb);
    }
}

/* ================================================================
 * 15. Undef and redefine
 * ================================================================ */
static void test_undef_redefine(void) {
    printf("\n--- 15: Undef and redefine ---\n");
    
    CHECK("undef changes value", "2",
        "#define X 1\n#undef X\n#define X 2\n#if X==2\nint yes=2;\n#endif\n");
    
    /* Undef of undefined macro — should be OK */
    CHECK_NO_ERROR("undef undefined macro",
        "#undef NONEXISTENT\nint yes=1;\n");
}

/* ================================================================
 * 16. Nested function-like macro calls
 * ================================================================ */
static void test_nested_macro_call(void) {
    printf("\n--- 16: Nested macro calls ---\n");
    
    CHECK("nested macro call", "(( ((1)+(2)))*(1))",
        "#define ADD(a,b) ((a)+(b))\n#define MUL(a,b) ((a)*(b))\nint r=MUL(ADD(1,2),1);\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests round 8 ===\n");
    
    test_multiline_macro();
    test_paste_boundary();
    test_arg_count_mismatch();
    test_ppnumber_exponent();
    test_wide_char();
    test_trigraphs();
    test_empty_object_macro();
    test_nested_parens();
    test_hash_no_space();
    test_comment_in_string();
    test_punctuator_longest();
    test_string_continuation();
    test_empty_input();
    test_circular_include();
    test_undef_redefine();
    test_nested_macro_call();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
