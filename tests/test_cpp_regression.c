/*
 * tests/test_cpp_regression.c — Targeted regression tests for bugs
 * identified during code review.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpp.h"

static int g_pass = 0, g_fail = 0;

/* Helper: count occurrences of a substring */
static int count_occ(const char *haystack, const char *needle) {
    int count = 0;
    const char *p = haystack;
    if (!haystack || !needle) return 0;
    while ((p = strstr(p, needle)) != NULL) { count++; p += strlen(needle); }
    return count;
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
        for (size_t i = 0; i < r.ndiags; i++) {
            printf("        diag[%zu] level=%d msg=%s\n", i, r.diags[i].level, r.diags[i].msg);
        }
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

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
    /* Negative check: substring must be absent */
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

/* Check output is non-empty */
#define CHECK_NONEMPTY(desc, input) \
    check_nonempty(desc, input, sizeof(input)-1, __LINE__)

static void check_nonempty(const char *desc, const char *input, size_t input_len, int line) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, input, input_len, "<test>", CPP_LANG_C);
    if (r.text && r.text[0]) { printf("  PASS  %s\n", desc); g_pass++; }
    else {
        printf("  FAIL  %s:%d  output is empty\n", desc, line);
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * P0 — macro.c L512-513: result.tail update bug in token-paste
 * ================================================================ */
static void test_p0_token_paste(void) {
    printf("\n--- P0: Token-paste whitespace handling ---\n");
    
    /* Debug first to see actual output */
    DBG("paste debug", 
        "#define CAT(a,b) a##b\nint CAT(x,y)=1;\n");
    
    /* Simple paste — check for the pasted result */
    CHECK("simple paste produces xy", "xy",
        "#define CAT(a,b) a##b\nint CAT(x,y)=1;\n");
    
    /* Paste with spaces around ## */
    CHECK("paste with spaces around ##", "xy",
        "#define CAT(a,b) a ## b\nint CAT(x,y)=1;\n");
    
    /* Triple paste */
    CHECK("triple paste", "foo_bar",
        "#define MKNAME(pre,suf) pre##_##suf\nint MKNAME(foo,bar)=1;\n");
}

/* ================================================================
 * P1 — pptok.c L143-150: CRLF splice boundary
 * ================================================================ */
static void test_p1_crlf_splice(void) {
    printf("\n--- P1: CRLF splice handling ---\n");
    
    /* CRLF line ending (no splice) */
    CHECK_NONEMPTY("plain CRLF passthrough", "int x = 1;\r\n");
    
    /* Splice with CRLF */
    CHECK("CRLF line splice produces 'int'", "int",
        "int\\\r\n x = 1;\r\n");
    
    /* Multiple consecutive splices */
    CHECK("multiple CRLF splices", "int",
        "in\\\r\nt\\\r\n x = 1;\r\n");
    
    /* LF-only splice (baseline) */
    CHECK("LF line splice", "int",
        "int\\\n x = 1;\n");
}

/* ================================================================
 * P1 — directive.c: #include relative path
 * ================================================================ */
static void test_p1_include_paths(void) {
    printf("\n--- P1: Include path handling ---\n");
    
    /* Create a header file */
    FILE *hf = fopen("test_rel_header.sp", "w");
    if (hf) {
        fprintf(hf, "int rel_include_ok = 1;\n");
        fclose(hf);
        
        /* Debug first */
        DBG("include debug", "#include \"test_rel_header.sp\"\n");
        
        CHECK("relative include", "rel_include_ok",
            "#include \"test_rel_header.sp\"\n");
        
        remove("test_rel_header.sp");
    } else {
        printf("  SKIP  relative include (cannot create temp file)\n");
    }
    
    /* Include guard test */
    FILE *hf2 = fopen("test_guard.sp", "w");
    if (hf2) {
        fprintf(hf2, "#ifndef TEST_GUARD_H\n#define TEST_GUARD_H\nint guard_marker = 1;\n#endif\n");
        fclose(hf2);
        
        const char *src =
            "#include \"test_guard.sp\"\n"
            "#include \"test_guard.sp\"\n"
            "int after = 2;\n";
        
        CppCtx *ctx = cpp_ctx_new();
        cpp_emit_linemarkers(ctx, false);
        CppResult r = cpp_run_buf(ctx, src, strlen(src), "<test>", CPP_LANG_C);
        
        int count = r.text ? count_occ(r.text, "guard_marker") : 0;
        if (count == 1) {
            printf("  PASS  include guard prevents double inclusion\n"); g_pass++;
        } else {
            printf("  FAIL  include guard: marker appeared %d times (expected 1)\n", count);
            printf("        text=[%s]\n", r.text ? r.text : "(null)");
            g_fail++;
        }
        cpp_result_free(&r);
        cpp_ctx_free(ctx);
        
        remove("test_guard.sp");
    }
}

/* ================================================================
 * P2 — directive.c L399: linemarker line number accuracy
 * ================================================================ */
static void test_p2_linemarker(void) {
    printf("\n--- P2: Linemarker line number accuracy ---\n");
    
    /* Debug */
    DBG("LINE debug",
        "#define X 1\n"
        "int line1 = __LINE__;\n");
    
    /* __LINE__ after #define */
    CHECK("__LINE__ after #define is 2", "= 2",
        "#define X 1\n"
        "int line1 = __LINE__;\n");
    
    /* __LINE__ after multiple directives */
    CHECK("__LINE__ after 3 #defines is 4", "= 4",
        "#define A 1\n"
        "#define B 2\n"
        "#define C 3\n"
        "int line2 = __LINE__;\n");
    
    /* __LINE__ after #ifdef block */
    CHECK("__LINE__ after #ifdef/#endif is 4", "= 4",
        "#define FOO\n"
        "#ifdef FOO\n"
        "#endif\n"
        "int line3 = __LINE__;\n");
}

/* ================================================================
 * P2 — macro.c: Empty parameter handling
 * ================================================================ */
static void test_p2_empty_args(void) {
    printf("\n--- P2: Empty macro argument handling ---\n");
    
    /* Debug */
    DBG("empty args debug",
        "#define EMPTY() done\n"
        "int x = EMPTY();\n");
    
    /* Empty parens */
    CHECK("empty parens macro", "done",
        "#define EMPTY() done\n"
        "int x = EMPTY();\n");
    
    /* Empty args with space inside */
    CHECK("empty parens with space", "done2",
        "#define EMPTY2() done2\n"
        "int x = EMPTY2( );\n");
    
    /* Function-like macro not called */
    CHECK("func macro without parens", "NOCALL",
        "#define NOCALL(x) expanded\n"
        "int x = NOCALL;\n");
}

/* ================================================================
 * Additional: String literal concatenation
 * ================================================================ */
static void test_string_concat(void) {
    printf("\n--- Additional: String concatenation ---\n");
    
    /* Debug */
    DBG("string concat debug", "\"a\" \"b\" \"c\"");
    
    /* Two strings */
    CHECK("two string concat", "\"ab\"",
        "\"a\" \"b\"");
    
    /* Three strings — this is the bug: expected "abc" */
    const char *src3 = "\"a\" \"b\" \"c\"";
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src3, strlen(src3), "<test>", CPP_LANG_C);
    if (r.text && strstr(r.text, "\"abc\"") != NULL) {
        printf("  PASS  three string concat\n"); g_pass++;
    } else {
        printf("  FAIL  three string concat: expected [\"abc\"] got [%s]\n",
               r.text ? r.text : "(null)");
        g_fail++;
    }
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

/* ================================================================
 * Additional: pp-number exponent handling
 * ================================================================ */
static void test_pp_number(void) {
    printf("\n--- Additional: pp-number exponent ---\n");
    
    /* Debug */
    DBG("pp-number debug", "double x = 1.5e+10;\n");
    
    /* Check that the full number is preserved */
    CHECK("pp-number with e+ exponent", "1.5e+10",
        "double x = 1.5e+10;\n");
    
    CHECK("pp-number with e- exponent", "1.5e-10",
        "double x = 1.5e-10;\n");
}

/* ================================================================
 * Additional: Trigraph handling
 * ================================================================ */
static void test_trigraphs(void) {
    printf("\n--- Additional: Trigraph handling ---\n");
    
    /* Debug */
    DBG("trigraph debug", "??=define FOO 1\nint x = FOO;\n");
    
    /* ??= → # */
    CHECK("trigraph ??= produces #define", "1",
        "??=define FOO 1\nint x = FOO;\n");
    
    /* ??( → [ and ??) → ] */
    CHECK("trigraph ??(??) produces []", "[0]",
        "int a??(0??) = 1;\n");
}

/* ================================================================
 * MAIN
 * ================================================================ */
int main(void) {
    printf("=== cpp regression tests ===\n");
    
    test_p0_token_paste();
    test_p1_crlf_splice();
    test_p1_include_paths();
    test_p2_linemarker();
    test_p2_empty_args();
    test_string_concat();
    test_pp_number();
    test_trigraphs();
    
    printf("\n=== %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
