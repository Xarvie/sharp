/* probe.c — search for edge cases / bugs in the existing preprocessor.
 *
 * Each probe runs an expectation against the existing code; mismatches are
 * reported.  This is read-only diagnostic, not a test suite.                */
#include "cpp.h"
#include "cpp_internal.h"
#include "pptok.h"
#include "directive.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static int problems = 0;

static CppResult run(const char *src, CppLang lang) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<probe>", lang);
    cpp_ctx_free(ctx);
    return r;
}

static int count_kind(const CppResult *r, CppTokKind k) {
    int n = 0;
    for (size_t i = 0; i < r->ntokens; i++)
        if (r->tokens[i].kind == k) n++;
    return n;
}

#define PROBE(name) printf("\n--- %s ---\n", name)
#define BAD(fmt, ...) do { problems++; printf("  ISSUE: " fmt "\n", ##__VA_ARGS__); } while(0)
#define OK(fmt, ...)  printf("  ok: " fmt "\n", ##__VA_ARGS__)

static void dump_tokens(const CppResult *r) {
    for (size_t i = 0; i < r->ntokens; i++) {
        printf("    [%zu] %s = '%.*s'\n", i,
               cpp_tok_kind_name(r->tokens[i].kind),
               (int)r->tokens[i].len, r->tokens[i].text);
    }
}

static void probe_wide_string(void) {
    PROBE("wide string literal L\"foo\"");
    CppResult r = run("L\"foo\"\n", CPP_LANG_C);
    int sl = count_kind(&r, CPPT_STRING_LIT);
    int id = count_kind(&r, CPPT_IDENT);
    if (sl != 1) BAD("expected 1 STRING_LIT, got %d (L\"foo\" tokenized as separate tokens?)", sl);
    if (id != 0) BAD("L should be part of string, not standalone identifier (got %d idents)", id);
    if (sl == 1) {
        const CppTok *t = NULL;
        for (size_t i = 0; i < r.ntokens; i++)
            if (r.tokens[i].kind == CPPT_STRING_LIT) { t = &r.tokens[i]; break; }
        if (t && t->len > 0 && t->text[0] != 'L')
            BAD("STRING_LIT spell missing L prefix: '%.*s'", (int)t->len, t->text);
    }
    if (sl != 1 || id != 0) dump_tokens(&r);
    cpp_result_free(&r);
}

static void probe_u8_string(void) {
    PROBE("UTF-8 string literal u8\"foo\"");
    CppResult r = run("u8\"foo\"\n", CPP_LANG_C);
    int sl = count_kind(&r, CPPT_STRING_LIT);
    int id = count_kind(&r, CPPT_IDENT);
    if (sl != 1) BAD("expected 1 STRING_LIT, got %d", sl);
    if (id != 0) BAD("u8 should be part of string, got %d standalone idents", id);
    cpp_result_free(&r);
}

static void probe_wide_char(void) {
    PROBE("wide char literal L'a'");
    CppResult r = run("L'a'\n", CPP_LANG_C);
    int cc = count_kind(&r, CPPT_CHAR_CONST);
    if (cc != 1) BAD("expected 1 CHAR_CONST, got %d", cc);
    cpp_result_free(&r);
}

static void probe_hex_float(void) {
    PROBE("hex float pp-number 0x1.fp10");
    CppResult r = run("double x = 0x1.fp10;\n", CPP_LANG_C);
    /* Should be one PP_NUMBER token */
    int found = 0;
    for (size_t i = 0; i < r.ntokens; i++) {
        if (r.tokens[i].kind == CPPT_PP_NUMBER &&
            r.tokens[i].len >= 8 &&
            memcmp(r.tokens[i].text, "0x1.fp10", 8) == 0) {
            found++;
        }
    }
    if (found != 1) BAD("0x1.fp10 not preserved as a single pp-number (found %d)", found);
    cpp_result_free(&r);
}

static void probe_stringify_quotes(void) {
    PROBE("stringification: # X with string content");
    CppResult r = run(
        "#define STR(x) #x\n"
        "STR(\"hello\\n\")\n", CPP_LANG_C);
    /* The macro should emit "\"hello\\n\"" — i.e. the inner quotes and
     * backslash must be escaped. */
    bool ok = strstr(r.text, "\\\"hello\\\\n\\\"") != NULL;
    if (!ok) BAD("STR(\"hello\\n\") should yield \\\"hello\\\\n\\\" in output, got: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_paste_validity(void) {
    PROBE("token pasting: A ## B should produce single ident");
    CppResult r = run(
        "#define PASTE(a,b) a##b\n"
        "PASTE(foo,bar)\n", CPP_LANG_C);
    int idents = count_kind(&r, CPPT_IDENT);
    if (idents != 1) BAD("PASTE(foo,bar) should yield single ident foobar, got %d idents", idents);
    if (!strstr(r.text, "foobar")) BAD("missing 'foobar' in: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_va_opt_empty(void) {
    PROBE("__VA_OPT__: empty __VA_ARGS__ → empty expansion");
    CppResult r = run(
        "#define LOG(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)\n"
        "LOG(\"hello\")\n", CPP_LANG_C);
    /* "hello" is a single token, no comma should appear before printf args */
    /* Output should look like: printf("hello"  )  — the comma is gone */
    bool comma_present = false;
    for (size_t i = 0; i < r.ntokens; i++) {
        if (r.tokens[i].kind == CPPT_PUNCT &&
            r.tokens[i].len == 1 && r.tokens[i].text[0] == ',') {
            comma_present = true; break;
        }
    }
    if (comma_present) BAD("__VA_OPT__(,) should NOT emit a comma when args are empty: %s", r.text);
    else OK("no comma — got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_va_opt_nonempty(void) {
    PROBE("__VA_OPT__: non-empty __VA_ARGS__ → emits");
    CppResult r = run(
        "#define LOG(fmt, ...) printf(fmt __VA_OPT__(,) __VA_ARGS__)\n"
        "LOG(\"v=%d\", 42)\n", CPP_LANG_C);
    /* Should have a comma between fmt and 42 */
    if (!strstr(r.text, ",")) BAD("__VA_OPT__(,) should emit comma when args present: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_self_recursion(void) {
    PROBE("self-recursive macro: should not infinite loop");
    CppResult r = run(
        "#define A A+1\n"
        "x = A;\n", CPP_LANG_C);
    /* The standard mandates: A is replaced by 'A+1' once; the 'A' inside
     * is blue-painted and not re-expanded.  Result: x = A+1; */
    if (!strstr(r.text, "A+1")) BAD("self-recursive macro botched: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_mutual_recursion(void) {
    PROBE("mutual recursion: A→B, B→A — both blue-painted on second touch");
    CppResult r = run(
        "#define A B\n"
        "#define B A\n"
        "x = A;\n", CPP_LANG_C);
    /* A → B → A (now hidden) → A.  Result: x = A; */
    if (!strstr(r.text, " A;") && !strstr(r.text, "= A;"))
        BAD("mutual-rec macros: expected x=A, got: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_charconst_in_if(void) {
    PROBE("#if 'A' should evaluate as 65");
    CppResult r = run(
        "#if 'A' == 65\n"
        "yes\n"
        "#else\n"
        "no\n"
        "#endif\n", CPP_LANG_C);
    if (!strstr(r.text, "yes")) BAD("'A'==65 should be true: %s", r.text);
    else OK("got correct branch");
    cpp_result_free(&r);
}

static void probe_octal_escape(void) {
    PROBE("#if '\\101' (octal A) == 65");
    CppResult r = run(
        "#if '\\101' == 65\n"
        "yes\n"
        "#endif\n", CPP_LANG_C);
    if (!strstr(r.text, "yes")) BAD("octal escape failed: %s", r.text);
    else OK("got correct value");
    cpp_result_free(&r);
}

static void probe_hex_escape(void) {
    PROBE("#if '\\x41' == 65");
    CppResult r = run(
        "#if '\\x41' == 65\n"
        "yes\n"
        "#endif\n", CPP_LANG_C);
    if (!strstr(r.text, "yes")) BAD("hex escape failed: %s", r.text);
    else OK("got correct value");
    cpp_result_free(&r);
}

static void probe_negative_unsigned(void) {
    PROBE("#if (-1U) > 0 (unsigned promotion)");
    CppResult r = run(
        "#if (-1U) > 0\n"
        "yes\n"
        "#endif\n", CPP_LANG_C);
    if (!strstr(r.text, "yes")) BAD("(-1U)>0 should be true: %s", r.text);
    else OK("unsigned arithmetic ok");
    cpp_result_free(&r);
}

static void probe_div_by_zero(void) {
    PROBE("#if 1/0 should diagnose, not crash");
    CppResult r = run(
        "#if 1/0\n"
        "yes\n"
        "#endif\n", CPP_LANG_C);
    bool err = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level == CPP_DIAG_ERROR) { err = true; break; }
    if (!err) BAD("1/0 in #if should produce a diagnostic");
    else OK("diagnostic produced");
    cpp_result_free(&r);
}

static void probe_unterminated_string(void) {
    PROBE("unterminated string literal should diagnose");
    CppResult r = run("\"hello\n", CPP_LANG_C);
    bool err = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level >= CPP_DIAG_WARNING) { err = true; break; }
    if (!err) BAD("unterminated string did not diagnose");
    else OK("diagnostic produced");
    cpp_result_free(&r);
}

static void probe_unterminated_comment(void) {
    PROBE("unterminated /* comment should diagnose");
    CppResult r = run("/* foo bar baz", CPP_LANG_C);
    bool err = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level >= CPP_DIAG_WARNING) { err = true; break; }
    if (!err) BAD("unterminated comment did not diagnose");
    else OK("diagnostic produced");
    cpp_result_free(&r);
}

static void probe_trigraph(void) {
    PROBE("trigraph ??= → #");
    CppResult r = run("??=define X 1\nX\n", CPP_LANG_C);
    if (!strstr(r.text, "1")) BAD("trigraph ??= failed: %s", r.text);
    else OK("trigraph translated");
    cpp_result_free(&r);
}

static void probe_line_splice_in_string(void) {
    PROBE("backslash-newline inside identifier: ide\\\\\\nntifier == identifier");
    CppResult r = run("#define X 1\nide\\\nntifier\n", CPP_LANG_C);
    /* Tokens should be: 'identifier' as one token after splicing.
     * Actually macros: `ide\<NL>ntifier` becomes `identifier` and we don't
     * have a #define for it so it should just appear in output as `identifier`. */
    if (!strstr(r.text, "identifier")) BAD("line splice in identifier failed: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_redefinition_warning(void) {
    PROBE("redefining a macro with different body should warn");
    CppResult r = run(
        "#define X 1\n"
        "#define X 2\n", CPP_LANG_C);
    bool warn = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level >= CPP_DIAG_WARNING) { warn = true; break; }
    if (!warn) BAD("incompatible redefinition should warn");
    else OK("warning produced");
    cpp_result_free(&r);
}

static void probe_redefinition_same(void) {
    PROBE("redefining a macro with SAME body should NOT warn");
    CppResult r = run(
        "#define X 1\n"
        "#define X 1\n", CPP_LANG_C);
    bool warn = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level >= CPP_DIAG_WARNING) { warn = true; break; }
    if (warn) BAD("identical redefinition should NOT warn");
    else OK("no diagnostic — correct");
    cpp_result_free(&r);
}

static void probe_undef_undefined(void) {
    PROBE("#undef of an undefined macro should NOT diagnose");
    CppResult r = run("#undef NOPE\n", CPP_LANG_C);
    if (r.ndiags != 0) BAD("#undef of undefined emitted %zu diags", r.ndiags);
    else OK("clean");
    cpp_result_free(&r);
}

static void probe_macro_paste_empty(void) {
    PROBE("token pasting with empty argument (placemarker)");
    CppResult r = run(
        "#define CAT(a,b) a##b\n"
        "CAT(foo,)\n"
        "CAT(,bar)\n", CPP_LANG_C);
    if (!strstr(r.text, "foo")) BAD("CAT(foo,) lost foo: %s", r.text);
    if (!strstr(r.text, "bar")) BAD("CAT(,bar) lost bar: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_func_macro_no_paren(void) {
    PROBE("function-like macro without parens — pass through");
    CppResult r = run(
        "#define F(x) x+1\n"
        "F\n", CPP_LANG_C);
    /* C says: f-l macro name not followed by '(' is just the identifier */
    if (!strstr(r.text, "F")) BAD("F should pass through: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_pp_number_signs(void) {
    PROBE("pp-number 1e+10 should be ONE token");
    CppResult r = run("1e+10\n", CPP_LANG_C);
    int n = count_kind(&r, CPPT_PP_NUMBER);
    if (n != 1) BAD("1e+10 split into %d pp-numbers", n);
    cpp_result_free(&r);
}

static void probe_dollar_ident(void) {
    PROBE("$ in identifier (extension)");
    CppResult r = run("foo$bar\n", CPP_LANG_C);
    int n = count_kind(&r, CPPT_IDENT);
    if (n != 1) BAD("foo$bar should be one ident, got %d", n);
    cpp_result_free(&r);
}

static void probe_digraph(void) {
    PROBE("digraphs <% %> <: :> %: %:%:");
    CppResult r = run("a<%b%>c<:0:>d %:e %:%: f\n", CPP_LANG_C);
    /* These should be tokenized as their canonical equivalents { } [ ] # ## */
    if (!strstr(r.text, "<%")) {
        OK("digraphs preserved as text");  /* Either way is acceptable */
    }
    cpp_result_free(&r);
}

static void probe_keep_whitespace(void) {
    PROBE("cpp_keep_whitespace: spaces preserved between tokens");
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_keep_whitespace(ctx, true);
    const char *src = "int x;\n";
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<probe>", CPP_LANG_C);
    int nspaces = count_kind(&r, CPPT_SPACE);
    if (nspaces == 0) BAD("expected SPACE tokens with keep_whitespace=true");
    else OK("got %d space tokens", nspaces);
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

static void probe_pragma_once_via_include(void) {
    PROBE("#pragma once: define a tmp file twice");
    /* Skipped — needs a real file; just ensure the API exists */
    OK("(skipped — needs filesystem test)");
}

static void probe_concat_strings(void) {
    PROBE("phase 6 string literal concat: \"foo\" \"bar\" → \"foobar\"");
    CppResult r = run("\"foo\" \"bar\"\n", CPP_LANG_C);
    if (!strstr(r.text, "\"foobar\"")) BAD("concat failed: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_concat_strings_mixed(void) {
    PROBE("phase 6: L\"a\" \"b\" → L\"ab\"");
    CppResult r = run("L\"a\" \"b\"\n", CPP_LANG_C);
    if (!strstr(r.text, "L\"ab\"") && !strstr(r.text, "\"ab\""))
        BAD("L+plain concat failed: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_macro_in_include(void) {
    PROBE("#include with macro expansion");
    /* Skipped — requires filesystem */
    OK("(skipped — needs filesystem test)");
}

static void probe_directive_after_directives(void) {
    PROBE("multiple consecutive directives shouldn't blow up");
    CppResult r = run(
        "#define A 1\n"
        "#define B 2\n"
        "#define C 3\n"
        "A B C\n", CPP_LANG_C);
    if (r.ndiags > 0) BAD("got %zu diagnostics", r.ndiags);
    if (!strstr(r.text, "1") || !strstr(r.text, "2") || !strstr(r.text, "3"))
        BAD("missing macro expansions: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_stringify_space_collapse(void) {
    PROBE("stringification: spaces between tokens collapse to single space");
    CppResult r = run(
        "#define S(x) #x\n"
        "S(  a   b   c  )\n", CPP_LANG_C);
    /* Per §6.10.3.2 leading/trailing whitespace is removed; runs of
     * whitespace collapsed to single space. */
    if (!strstr(r.text, "\"a b c\"")) BAD("expected \"a b c\", got: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_function_macro_args_with_commas_in_parens(void) {
    PROBE("F((a,b),c) — comma inside parens does NOT split args");
    CppResult r = run(
        "#define F(x,y) x|y\n"
        "F((a,b),c)\n", CPP_LANG_C);
    if (!strstr(r.text, "(a,b)") || !strstr(r.text, "|c"))
        BAD("expected '(a,b)|c', got: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_predefined_file_line(void) {
    PROBE("__FILE__ / __LINE__");
    CppResult r = run(
        "__FILE__\n"
        "__LINE__\n", CPP_LANG_C);
    if (!strstr(r.text, "<probe>")) BAD("__FILE__ should be \"<probe>\": %s", r.text);
    if (!strstr(r.text, "1") || !strstr(r.text, "2"))
        BAD("__LINE__ should give 1 then 2: %s", r.text);
    cpp_result_free(&r);
}

static void probe_empty_input(void) {
    PROBE("empty input should produce empty result");
    CppResult r = run("", CPP_LANG_C);
    if (r.ndiags > 0) BAD("got %zu diags from empty input", r.ndiags);
    else OK("no diagnostics");
    cpp_result_free(&r);
}

static void probe_only_whitespace(void) {
    PROBE("whitespace-only input");
    CppResult r = run("   \n   \n   ", CPP_LANG_C);
    if (r.ndiags > 0) BAD("got %zu diags from whitespace-only input", r.ndiags);
    else OK("no diagnostics");
    cpp_result_free(&r);
}

static void probe_extremely_long_token(void) {
    PROBE("very long identifier (10000 chars)");
    char *src = malloc(10100);
    memset(src, 'a', 10000);
    src[10000] = '\n';
    src[10001] = '\0';
    CppResult r = run(src, CPP_LANG_C);
    int idents = count_kind(&r, CPPT_IDENT);
    if (idents != 1) BAD("long ident: expected 1 ident, got %d", idents);
    else OK("handled long identifier");
    cpp_result_free(&r);
    free(src);
}

static void probe_crlf_lines(void) {
    PROBE("CRLF line endings");
    CppResult r = run("#define X 1\r\nint x = X;\r\n", CPP_LANG_C);
    if (r.ndiags > 0) BAD("CRLF: %zu diags", r.ndiags);
    if (!strstr(r.text, "x = 1")) BAD("CRLF: expansion failed: %s", r.text);
    else OK("CRLF handled");
    cpp_result_free(&r);
}

static void probe_pragma_unknown(void) {
    PROBE("#pragma unknown — should pass through with no error");
    CppResult r = run("#pragma GCC poison foo\nbar\n", CPP_LANG_C);
    bool err = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].level >= CPP_DIAG_ERROR) { err = true; break; }
    if (err) BAD("unknown pragma should not error");
    else OK("no error for unknown pragma");
    cpp_result_free(&r);
}

static void probe_cmdline_define(void) {
    PROBE("cpp_define then expand");
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_define(ctx, "FOO", "42");
    const char *src = "FOO\n";
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<probe>", CPP_LANG_C);
    if (!strstr(r.text, "42")) BAD("FOO should expand to 42: %s", r.text);
    else OK("cmdline define ok");
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

static void probe_cmdline_undef_builtin(void) {
    PROBE("cpp_undefine on a built-in should suppress it");
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    cpp_undefine(ctx, "__STDC__");
    const char *src = "#ifdef __STDC__\nyes\n#else\nno\n#endif\n";
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<probe>", CPP_LANG_C);
    if (strstr(r.text, "yes")) BAD("__STDC__ should be undefined after -U: %s", r.text);
    else OK("undef of built-in ok");
    cpp_result_free(&r);
    cpp_ctx_free(ctx);
}

static void probe_va_args_alone(void) {
    PROBE("variadic macro: #define X(...) and X(a,b,c)");
    CppResult r = run(
        "#define X(...) [__VA_ARGS__]\n"
        "X(1,2,3)\n", CPP_LANG_C);
    if (!strstr(r.text, "[1,2,3]") && !strstr(r.text, "[1, 2, 3]"))
        BAD("VA expansion: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_recursive_arg_expansion(void) {
    PROBE("argument expanded before substitution");
    CppResult r = run(
        "#define F(x) [x]\n"
        "#define A 1\n"
        "F(A)\n", CPP_LANG_C);
    if (!strstr(r.text, "[1]")) BAD("F(A) should yield [1]: %s", r.text);
    else OK("got: %s", r.text);
    cpp_result_free(&r);
}

static void probe_no_arg_expansion_in_paste(void) {
    PROBE("argument NOT expanded when used with ## (correctness check)");
    /* Per §6.10.3.3p2: an argument used with # or ## is NOT pre-expanded.
     * #define A 1
     * #define X(a) a##_suf
     * X(A)  →  A_suf  (NOT 1_suf)
     */
    CppResult r = run(
        "#define A 1\n"
        "#define X(a) a##_suf\n"
        "X(A)\n", CPP_LANG_C);
    if (strstr(r.text, "1_suf")) BAD("## should NOT pre-expand arg: got 1_suf in: %s", r.text);
    else if (strstr(r.text, "A_suf")) OK("correctly produced A_suf");
    else BAD("unexpected: %s", r.text);
    cpp_result_free(&r);
}

static void probe_no_arg_expansion_in_stringify(void) {
    PROBE("argument NOT expanded when used with # (stringify)");
    CppResult r = run(
        "#define A 1\n"
        "#define S(a) #a\n"
        "S(A)\n", CPP_LANG_C);
    if (strstr(r.text, "\"1\"")) BAD("# should NOT pre-expand arg: got \"1\" in: %s", r.text);
    else if (strstr(r.text, "\"A\"")) OK("correctly produced \"A\"");
    else BAD("unexpected: %s", r.text);
    cpp_result_free(&r);
}

static void probe_pp_number_dot_first(void) {
    PROBE("pp-number .5e-3");
    CppResult r = run(".5e-3\n", CPP_LANG_C);
    int n = count_kind(&r, CPPT_PP_NUMBER);
    if (n != 1) BAD(".5e-3 should be one pp-number, got %d", n);
    cpp_result_free(&r);
}

static void probe_at_in_string_c(void) {
    PROBE("'@' inside a string literal in C mode is not special");
    CppResult r = run("\"@foo\"\n", CPP_LANG_C);
    if (!strstr(r.text, "\"@foo\"")) BAD("@ in string mangled: %s", r.text);
    else OK("preserved");
    cpp_result_free(&r);
}

static void probe_at_in_string_sharp(void) {
    PROBE("'@foo' inside a string literal in Sharp mode is not @-intrinsic");
    CppResult r = run("\"@foo\"\n", CPP_LANG_SHARP);
    int ai = count_kind(&r, CPPT_AT_INTRINSIC);
    if (ai != 0) BAD("@ inside string was tokenized as @-intrinsic");
    else OK("preserved as string content");
    cpp_result_free(&r);
}

static void probe_paste_into_pp_number(void) {
    PROBE("token paste forming pp-number");
    CppResult r = run(
        "#define N(a,b) a##b\n"
        "N(1,2)\n", CPP_LANG_C);
    if (!strstr(r.text, "12")) BAD("paste 1##2 should give 12: %s", r.text);
    int nn = count_kind(&r, CPPT_PP_NUMBER);
    if (nn != 1) BAD("12 should be one pp-number, got %d", nn);
    cpp_result_free(&r);
}

int main(void) {
    printf("=== Preprocessor probe (looking for gaps) ===\n");

    /* Tokenizer */
    probe_wide_string();
    probe_u8_string();
    probe_wide_char();
    probe_hex_float();
    probe_pp_number_signs();
    probe_pp_number_dot_first();
    probe_dollar_ident();
    probe_digraph();
    probe_unterminated_string();
    probe_unterminated_comment();
    probe_trigraph();
    probe_line_splice_in_string();
    probe_crlf_lines();
    probe_extremely_long_token();

    /* Macro expansion */
    probe_stringify_quotes();
    probe_paste_validity();
    probe_va_opt_empty();
    probe_va_opt_nonempty();
    /* Phase 2 territory — mutual recursion crashes the macro engine.
     * Skipping until Phase 2 is implemented. */
    probe_mutual_recursion();
    probe_self_recursion();
    probe_redefinition_warning();
    probe_redefinition_same();
    probe_undef_undefined();
    probe_macro_paste_empty();
    probe_func_macro_no_paren();
    probe_va_args_alone();
    probe_recursive_arg_expansion();
    probe_no_arg_expansion_in_paste();
    probe_no_arg_expansion_in_stringify();
    probe_function_macro_args_with_commas_in_parens();
    probe_paste_into_pp_number();
    probe_stringify_space_collapse();

    /* #if expressions */
    probe_charconst_in_if();
    probe_octal_escape();
    probe_hex_escape();
    probe_negative_unsigned();
    probe_div_by_zero();

    /* Directives */
    probe_directive_after_directives();
    probe_pragma_unknown();
    probe_pragma_once_via_include();
    probe_macro_in_include();
    probe_predefined_file_line();

    /* Phase 6 */
    probe_concat_strings();
    probe_concat_strings_mixed();

    /* API */
    probe_keep_whitespace();
    probe_cmdline_define();
    probe_cmdline_undef_builtin();

    /* Misc */
    probe_empty_input();
    probe_only_whitespace();
    probe_at_in_string_c();
    probe_at_in_string_sharp();

    printf("\n=== %d issues found ===\n", problems);
    return problems > 0 ? 1 : 0;
}
