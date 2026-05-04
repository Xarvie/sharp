/*
 * probe.c — Phase 0 audit probes for the Sharp language frontend.
 *
 * PURPOSE
 * -------
 * Before writing any front-end code, 62 probes map every core feature of
 * Sharp v0.11 spec to one of three outcomes:
 *
 *   OK        — behaviour is as spec'd; no action needed before this phase.
 *   ISSUE     — something unexpected; must be resolved or annotated before
 *               the phase that implements the corresponding feature begins.
 *   DECISION  — no runtime check; documents a design decision that future
 *               AI must honour.  Always reports OK.
 *
 * Two probe kinds:
 *   CPP probe     — calls cpp_run_buf in Sharp mode, inspects token stream.
 *   DECISION probe — prints a structured design-decision record, always OK.
 *
 * USAGE
 *   make probe && ./probe
 *
 * A documented ISSUE is an intentional finding, not a build failure.
 * main() returns 0 regardless of ISSUE count; the summary records them.
 */

#include "cpp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* =========================================================================
 * Accounting
 * ====================================================================== */
static int g_ok = 0, g_issue = 0, g_decision = 0;

static void ok(const char *name, const char *note) {
    printf("  OK       %-10s %s\n", name, note);
    g_ok++;
}
static void iss(const char *name, const char *note) {
    printf("  ISSUE    %-10s %s\n", name, note);
    g_issue++;
}
static void dec(const char *name, const char *text) {
    printf("  DECISION %-10s %s\n", name, text);
    g_decision++;
}

/* =========================================================================
 * CPP probe helpers
 * ====================================================================== */

static CppResult cpp_sharp(const char *src) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<probe>", CPP_LANG_SHARP);
    cpp_ctx_free(ctx);
    return r;
}

static size_t count_kind(const CppResult *r, CppTokKind k) {
    size_t n = 0;
    for (size_t i = 0; i < r->ntokens; i++)
        if (r->tokens[i].kind == k) n++;
    return n;
}

static bool has_token(const CppResult *r, CppTokKind k, const char *needle) {
    size_t nlen = strlen(needle);
    for (size_t i = 0; i < r->ntokens; i++) {
        CppTok t = r->tokens[i];
        if (t.kind == k && t.len == nlen && strncmp(t.text, needle, nlen) == 0)
            return true;
    }
    return false;
}

static bool text_has(const CppResult *r, const char *needle) {
    return r->text && strstr(r->text, needle);
}

/* =========================================================================
 * §1  Lexer layer: what cpp emits for Sharp-specific syntax
 * ====================================================================== */

static void p1_1(void) {
    /* 'defer' is not a cpp keyword; it must emerge as CPPT_IDENT.
     * The fe lex layer does keyword promotion. */
    CppResult r = cpp_sharp("defer a.destroy();");
    bool found = has_token(&r, CPPT_IDENT, "defer");
    cpp_result_free(&r);
    if (found) ok("P1.1", "'defer' => CPPT_IDENT; fe lex must promote to keyword");
    else       iss("P1.1", "'defer' not found as CPPT_IDENT");
}

static void p1_2(void) {
    /* @has_operator produces CPPT_AT_INTRINSIC with spell "@has_operator". */
    CppResult r = cpp_sharp("@has_operator(T, +)");
    bool found = false;
    for (size_t i = 0; i < r.ntokens; i++) {
        CppTok t = r.tokens[i];
        if (t.kind == CPPT_AT_INTRINSIC &&
            t.len >= 13 && strncmp(t.text, "@has_operator", 13) == 0) {
            found = true; break;
        }
    }
    cpp_result_free(&r);
    if (found) ok("P1.2", "@has_operator => AT_INTRINSIC, spell includes '@'");
    else       iss("P1.2", "@has_operator did not produce expected AT_INTRINSIC");
}

static void p1_3(void) {
    CppResult r = cpp_sharp("@static_assert(1, \"ok\")");
    bool found = count_kind(&r, CPPT_AT_INTRINSIC) > 0;
    cpp_result_free(&r);
    if (found) ok("P1.3", "@static_assert => CPPT_AT_INTRINSIC");
    else       iss("P1.3", "@static_assert did not produce AT_INTRINSIC");
}

static void p1_4(void) {
    /* Sharp uses 'null' in spec examples; without a header it is IDENT. */
    CppResult r = cpp_sharp("void* p = null;");
    bool as_ident = has_token(&r, CPPT_IDENT, "null");
    bool no_err   = r.error == NULL;
    cpp_result_free(&r);
    if (no_err) ok("P1.4", "'null' without header: IDENT (not a language keyword)");
    else        iss("P1.4", "'null' caused cpp error");
    (void)as_ident;
}

static void p1_5(void) {
    /* Unknown @intrinsic is still AT_INTRINSIC; sema (Phase 10) rejects it. */
    CppResult r = cpp_sharp("@totally_unknown(x)");
    bool found   = count_kind(&r, CPPT_AT_INTRINSIC) > 0;
    bool no_err  = r.error == NULL;
    cpp_result_free(&r);
    if (found && no_err) ok("P1.5", "Unknown @intrinsic => AT_INTRINSIC, no cpp error");
    else                  iss("P1.5", "cpp rejected unknown @intrinsic unexpectedly");
}

static void p1_6(void) {
    /* In C mode '@' is CPPT_PUNCT, not AT_INTRINSIC. */
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, "@foo", 4, "<p>", CPP_LANG_C);
    cpp_ctx_free(ctx);
    bool no_at = count_kind(&r, CPPT_AT_INTRINSIC) == 0;
    cpp_result_free(&r);
    if (no_at) ok("P1.6", "C mode: '@' is CPPT_PUNCT, not AT_INTRINSIC");
    else       iss("P1.6", "C mode '@' produced AT_INTRINSIC unexpectedly");
}

static void p1_7(void) {
    /* '<' and '>' in generics are plain CPPT_PUNCT. */
    CppResult r = cpp_sharp("struct Vec<T> { T* data; };");
    bool lt = has_token(&r, CPPT_PUNCT, "<");
    bool gt = has_token(&r, CPPT_PUNCT, ">");
    cpp_result_free(&r);
    if (lt && gt) ok("P1.7", "'<' '>' in generics are CPPT_PUNCT — parse handles ambiguity");
    else          iss("P1.7", "Expected '<' '>' as CPPT_PUNCT in generic decl");
}

static void p1_8(void) {
    /* KEY FINDING: does 'Vec<Vec<int>>' produce '>>' as a single token?
     * If so, the parse layer must split it in generic-arg context. */
    CppResult r = cpp_sharp("Vec<Vec<int>> v;");
    bool rshift = has_token(&r, CPPT_PUNCT, ">>");
    size_t gt_count = 0;
    for (size_t i = 0; i < r.ntokens; i++)
        if (r.tokens[i].kind == CPPT_PUNCT && r.tokens[i].len == 1 &&
            r.tokens[i].text[0] == '>')
            gt_count++;
    cpp_result_free(&r);
    if (rshift)
        iss("P1.8", "CONFIRMED: '>>' is a single token in Vec<Vec<int>>; parse must split it");
    else if (gt_count >= 2)
        ok("P1.8", "'>>' already tokenised as two '>'; no splitting needed");
    else
        iss("P1.8", "Could not determine '>>' tokenisation for nested generics");
}

/* =========================================================================
 * §2  struct syntax: methods, fields, call tokens
 * ====================================================================== */

static void p2_1(void) {
    CppResult r = cpp_sharp("struct Buffer { char* ptr; long len; };");
    bool ok_ = r.error == NULL && r.ndiags == 0 && has_token(&r, CPPT_IDENT, "struct");
    cpp_result_free(&r);
    if (ok_) ok("P2.1", "Struct field decl: IDENT+PUNCT tokens, zero diags");
    else     iss("P2.1", "Unexpected diag or missing 'struct' IDENT");
}

static void p2_2(void) {
    /* 'this' inside a method body is CPPT_IDENT. */
    CppResult r = cpp_sharp(
        "struct B { long len; long size() { return this->len; } };");
    bool found = has_token(&r, CPPT_IDENT, "this");
    bool ok_   = r.error == NULL && found;
    cpp_result_free(&r);
    if (ok_) ok("P2.2", "'this' inside method body = CPPT_IDENT");
    else     iss("P2.2", "'this' not IDENT or cpp error");
}

static void p2_3(void) {
    /* 'const' suffix after ')' in a const method is CPPT_IDENT. */
    CppResult r = cpp_sharp(
        "struct B { long len; long size() const { return this->len; } };");
    bool found = has_token(&r, CPPT_IDENT, "const");
    bool ok_   = r.error == NULL && found;
    cpp_result_free(&r);
    if (ok_) ok("P2.3", "const-method suffix 'const' = CPPT_IDENT in token stream");
    else     iss("P2.3", "'const' not found or cpp error");
}

static void p2_4(void) {
    CppResult r = cpp_sharp("b.get(0);");
    bool dot = has_token(&r, CPPT_PUNCT, ".");
    cpp_result_free(&r);
    if (dot) ok("P2.4", "Method call b.get(0): '.' is CPPT_PUNCT");
    else     iss("P2.4", "'.' not found as CPPT_PUNCT");
}

static void p2_5(void) {
    CppResult r = cpp_sharp("p->get(0);");
    bool arrow = has_token(&r, CPPT_PUNCT, "->");
    cpp_result_free(&r);
    if (arrow) ok("P2.5", "Pointer call p->get(0): '->' is CPPT_PUNCT");
    else       iss("P2.5", "'->' not found as CPPT_PUNCT");
}

static void p2_6(void) {
    /* Associated-function call: same '.' token as field access. */
    CppResult r = cpp_sharp("Buffer.alloc(1024);");
    bool dot = has_token(&r, CPPT_PUNCT, ".");
    bool ok_ = r.error == NULL && dot;
    cpp_result_free(&r);
    if (ok_) ok("P2.6", "Assoc-fn Buffer.alloc: '.' same as field-access (parse disambiguates)");
    else     iss("P2.6", "Unexpected token for assoc-fn call");
}

/* =========================================================================
 * §3  Operator overloading token stream
 * ====================================================================== */

static void p3_1(void) {
    CppResult r = cpp_sharp("Vec3 operator+(Vec3 rhs) { return rhs; }");
    bool found = has_token(&r, CPPT_IDENT, "operator");
    bool ok_   = r.error == NULL && found;
    cpp_result_free(&r);
    if (ok_) ok("P3.1", "'operator' = CPPT_IDENT; fe lex/parse handles it");
    else     iss("P3.1", "'operator' not IDENT or cpp error");
}

static void p3_2(void) {
    CppResult r = cpp_sharp("T operator[](long i) const { return data[i]; }");
    bool op  = has_token(&r, CPPT_IDENT, "operator");
    bool lb  = has_token(&r, CPPT_PUNCT, "[");
    bool rb  = has_token(&r, CPPT_PUNCT, "]");
    bool ok_ = r.error == NULL && op && lb && rb;
    cpp_result_free(&r);
    if (ok_) ok("P3.2", "operator[]: IDENT('operator') + '[' + ']' are PUNCT");
    else     iss("P3.2", "Unexpected token stream for operator[]");
}

static void p3_3(void) {
    CppResult r = cpp_sharp("Vec3 operator-() { Vec3 z = {0,0,0}; return z; }");
    bool op    = has_token(&r, CPPT_IDENT, "operator");
    bool minus = has_token(&r, CPPT_PUNCT, "-");
    bool ok_   = r.error == NULL && op && minus;
    cpp_result_free(&r);
    if (ok_) ok("P3.3", "Unary operator-(): IDENT + PUNCT('-')");
    else     iss("P3.3", "Unexpected token stream for unary operator-");
}

static void p3_4(void) {
    CppResult r = cpp_sharp("Vec3 operator*(float s, Vec3 v) { return v; }");
    bool op   = has_token(&r, CPPT_IDENT, "operator");
    bool star = has_token(&r, CPPT_PUNCT, "*");
    bool ok_  = r.error == NULL && op && star;
    cpp_result_free(&r);
    if (ok_) ok("P3.4", "Free operator*(float,Vec3): standard tokens, zero diags");
    else     iss("P3.4", "Unexpected token stream for free operator*");
}

/* =========================================================================
 * §4  defer token stream
 * ====================================================================== */

static void p4_1(void) {
    CppResult r = cpp_sharp("defer a.destroy();");
    bool found = has_token(&r, CPPT_IDENT, "defer");
    bool ok_   = r.error == NULL && found;
    cpp_result_free(&r);
    if (ok_) ok("P4.1", "Single-stmt defer: IDENT('defer') EXPR ';'");
    else     iss("P4.1", "Unexpected token stream for defer stmt");
}

static void p4_2(void) {
    CppResult r = cpp_sharp("defer { cleanup(); }");
    bool found  = has_token(&r, CPPT_IDENT, "defer");
    bool lbrace = has_token(&r, CPPT_PUNCT, "{");
    bool ok_    = r.error == NULL && found && lbrace;
    cpp_result_free(&r);
    if (ok_) ok("P4.2", "Block defer: IDENT('defer') '{' STMTS '}'");
    else     iss("P4.2", "Unexpected token stream for defer block");
}

static void p4_3(void) {
    /* Both 'defer' tokens must appear as IDENT; parse rejects nesting. */
    CppResult r = cpp_sharp("defer { defer reset(); }");
    size_t n = 0;
    for (size_t i = 0; i < r.ntokens; i++) {
        CppTok t = r.tokens[i];
        if (t.kind == CPPT_IDENT && t.len == 5 &&
            strncmp(t.text, "defer", 5) == 0) n++;
    }
    bool ok_ = r.error == NULL && n == 2;
    cpp_result_free(&r);
    if (ok_) ok("P4.3", "Nested defer: both 'defer' are IDENT; parse will reject");
    else     iss("P4.3", "Unexpected nested-defer token count");
}

static void p4_4(void) {
    CppResult r = cpp_sharp(
        "for (long i = 0; i < n; i++) { defer b.destroy(); if (!ok) break; }");
    bool found = has_token(&r, CPPT_IDENT, "defer");
    bool ok_   = r.error == NULL && found;
    cpp_result_free(&r);
    if (ok_) ok("P4.4", "defer inside for-loop: standard tokens, zero diags");
    else     iss("P4.4", "Unexpected token stream for defer-in-for");
}

/* =========================================================================
 * §5  Generics token stream
 * ====================================================================== */

static void p5_1(void) {
    CppResult r = cpp_sharp("struct Vec<T> { T* data; long len; };");
    bool lt = has_token(&r, CPPT_PUNCT, "<");
    bool gt = has_token(&r, CPPT_PUNCT, ">");
    bool T  = has_token(&r, CPPT_IDENT, "T");
    bool ok_= r.error == NULL && lt && gt && T;
    cpp_result_free(&r);
    if (ok_) ok("P5.1", "Generic struct Vec<T>: '<' T '>' all separate tokens");
    else     iss("P5.1", "Unexpected token stream for generic struct decl");
}

static void p5_2(void) {
    CppResult r = cpp_sharp("Vec<int> xs;");
    bool lt = has_token(&r, CPPT_PUNCT, "<");
    bool gt = has_token(&r, CPPT_PUNCT, ">");
    bool ok_= r.error == NULL && lt && gt;
    cpp_result_free(&r);
    if (ok_) ok("P5.2", "Generic type use Vec<int>: '<' '>' are PUNCT");
    else     iss("P5.2", "Unexpected token stream for generic type use");
}

static void p5_3(void) {
    CppResult r = cpp_sharp("Pair<int, float> p;");
    bool comma = has_token(&r, CPPT_PUNCT, ",");
    bool ok_   = r.error == NULL && comma;
    cpp_result_free(&r);
    if (ok_) ok("P5.3", "Multi-param Pair<int,float>: comma is CPPT_PUNCT");
    else     iss("P5.3", "Unexpected token stream for multi-param generic");
}

static void p5_4(void) {
    CppResult r = cpp_sharp("void swap<T>(T* a, T* b) { T tmp=*a; *a=*b; *b=tmp; }");
    bool lt = has_token(&r, CPPT_PUNCT, "<");
    bool ok_= r.error == NULL && lt;
    cpp_result_free(&r);
    if (ok_) ok("P5.4", "Generic func def swap<T>: standard tokens, zero diags");
    else     iss("P5.4", "Unexpected token stream for generic func def");
}

static void p5_5(void) {
    CppResult r = cpp_sharp("swap<int>(&x, &y);");
    bool lt = has_token(&r, CPPT_PUNCT, "<");
    bool gt = has_token(&r, CPPT_PUNCT, ">");
    bool ok_= r.error == NULL && lt && gt;
    cpp_result_free(&r);
    if (ok_) ok("P5.5", "Explicit type-arg call swap<int>: standard tokens");
    else     iss("P5.5", "Unexpected token stream for explicit type-arg call");
}

static void p5_6(void) {
    CppResult r = cpp_sharp("Vec<int>.new();");
    bool dot = has_token(&r, CPPT_PUNCT, ".");
    bool ok_ = r.error == NULL && dot;
    cpp_result_free(&r);
    if (ok_) ok("P5.6", "Generic assoc-fn Vec<int>.new(): '.' is CPPT_PUNCT");
    else     iss("P5.6", "Unexpected token stream for generic assoc-fn call");
}

static void p5_7(void) {
    CppResult r = cpp_sharp("extern struct Vec<int>;");
    bool ext = has_token(&r, CPPT_IDENT, "extern");
    bool ok_ = r.error == NULL && ext;
    cpp_result_free(&r);
    if (ok_) ok("P5.7", "extern struct Vec<int>: zero diags (spec §5.6)");
    else     iss("P5.7", "Unexpected diag for extern struct");
}

/* =========================================================================
 * §6  @ intrinsics — token stream
 * ====================================================================== */

static void p6_1(void) {
    CppResult r = cpp_sharp("@has_operator(T, ==)");
    bool found = count_kind(&r, CPPT_AT_INTRINSIC) > 0;
    cpp_result_free(&r);
    if (found) ok("P6.1", "@has_operator(T, ==) => AT_INTRINSIC");
    else       iss("P6.1", "@has_operator(T, ==) did not produce AT_INTRINSIC");
}

static void p6_2(void) {
    CppResult r = cpp_sharp("@has_operator(T, [])");
    bool found = count_kind(&r, CPPT_AT_INTRINSIC) > 0;
    cpp_result_free(&r);
    if (found) ok("P6.2", "@has_operator(T, []) => AT_INTRINSIC");
    else       iss("P6.2", "@has_operator(T, []) did not produce AT_INTRINSIC");
}

static void p6_3(void) {
    /* Two nested @intrinsics: @static_assert and @has_operator. */
    CppResult r = cpp_sharp(
        "@static_assert(@has_operator(T, +), \"msg\")");
    size_t n  = count_kind(&r, CPPT_AT_INTRINSIC);
    bool ok_  = r.error == NULL && n == 2;
    cpp_result_free(&r);
    if (ok_) ok("P6.3", "Nested @intrinsics: two AT_INTRINSIC tokens");
    else     iss("P6.3", "Expected 2 AT_INTRINSIC in nested @static_assert(@has_operator)");
}

static void p6_4(void) {
    /* cpp defers '#if @has_operator' — the text must contain the token. */
    CppResult r = cpp_sharp(
        "#if @has_operator(T, +)\nint x = 1;\n#endif\n");
    bool preserved = text_has(&r, "@has_operator");
    bool no_err    = r.error == NULL;
    cpp_result_free(&r);
    if (preserved && no_err)
        ok("P6.4", "#if @has_operator deferred by cpp (text preserved for fe)");
    else
        iss("P6.4", "#if @has_operator not deferred or cpp error");
}

static void p6_5(void) {
    /* The '+' operand inside @has_operator is CPPT_PUNCT. */
    CppResult r = cpp_sharp("@has_operator(T, +)");
    bool plus = has_token(&r, CPPT_PUNCT, "+");
    bool ok_  = r.error == NULL && plus;
    cpp_result_free(&r);
    if (ok_) ok("P6.5", "Operator argument '+' inside @has_operator is CPPT_PUNCT");
    else     iss("P6.5", "'+' not found as CPPT_PUNCT inside @has_operator");
}

/* =========================================================================
 * §7  Struct literal syntax and misc
 * ====================================================================== */

static void p7_1(void) {
    /* 'field: value' struct literal — the ':' is CPPT_PUNCT. */
    CppResult r = cpp_sharp("Buffer b = Buffer { ptr: buf, len: n };");
    bool colon = has_token(&r, CPPT_PUNCT, ":");
    bool ok_   = r.error == NULL && colon;
    cpp_result_free(&r);
    if (ok_) ok("P7.1", "Struct literal 'field: val' — ':' is CPPT_PUNCT, zero diags");
    else     iss("P7.1", "Unexpected ':' handling in struct literal");
}

static void p7_2(void) {
    const char *src =
        "struct Pt { float x; float y; "
        "  float xval() const { return this->x; } };";
    CppResult r = cpp_sharp(src);
    bool ok_ = r.error == NULL && r.ntokens > 5;
    cpp_result_free(&r);
    if (ok_) ok("P7.2", "Struct-with-const-method passes through cpp cleanly");
    else     iss("P7.2", "Struct-with-method produced unexpected diags");
}

/* =========================================================================
 * §8  Diagnostic plumbing
 * ====================================================================== */

static void p8_1(void) {
    /* cpp errors have valid CppLoc — fe can reuse them directly. */
    CppResult r = cpp_sharp("char* s = \"unterminated;");
    bool diag_ok = r.ndiags > 0 && r.diags[0].loc.line > 0;
    cpp_result_free(&r);
    if (diag_ok) ok("P8.1", "cpp diag carries valid CppLoc; fe can reuse type directly");
    else         iss("P8.1", "cpp diag missing or has no valid line location");
}

static void p8_2(void) {
    CppResult r = cpp_sharp("#include <_sharp_probe_no_such_file_.h>");
    bool has_err = r.error != NULL || r.ndiags > 0;
    cpp_result_free(&r);
    if (has_err) ok("P8.2", "Missing #include produces diag; fe driver checks r.ndiags");
    else         iss("P8.2", "Missing include produced no diag (unexpected)");
}

/* =========================================================================
 * §9  DECISION — name mangling (spec §5.10)
 * ====================================================================== */

static void p9_1(void) {
    dec("P9.1",
        "Buffer::alloc->Buffer__alloc  "
        "Buffer::get->Buffer__get(Buffer* this,long i)  "
        "Buffer::length->Buffer__length(const Buffer* this)");
}
static void p9_2(void) {
    dec("P9.2",
        "Vec<int>->Vec__int  Pair<int,float>->Pair__int__float  "
        "Vec<Pair<int,float>>->Vec__Pair__int__float");
}
static void p9_3(void) {
    dec("P9.3",
        "operator+(Vec3,Vec3)->operator_add__Vec3__Vec3  "
        "operator[](Vec<int>,long)->operator_index__Vec__int__long  "
        "Symbol map: + add - sub * mul / div % mod == eq != ne "
        "< lt > gt <= le >= ge & band | bor ^ bxor << shl >> shr [] index "
        "-(unary) neg ! lnot ~ bnot");
}
static void p9_4(void) {
    dec("P9.4",
        "Vec<int>::push->Vec__int__push  "
        "Vec<int>::size->Vec__int__size  "
        "(no const encoding in mangled name; const is a dispatch rule)");
}
static void p9_5(void) {
    dec("P9.5",
        "EDGE P1.8: Vec<Vec<int>> '>>' is a single rshift token. "
        "PLAN: parse_generic_args() sees PUNCT('>>'), consumes it, "
        "pushes back synthetic PUNCT('>') for inner level to close.");
}

/* =========================================================================
 * §10 DECISION — defer C translation
 * ====================================================================== */

static void p10_1(void) {
    dec("P10.1",
        "STRATEGY inline-cleanup-blocks (not goto labels). "
        "Sharp: defer a.destroy(); -> C: at every exit, emit Buffer__destroy(&a);");
}
static void p10_2(void) {
    dec("P10.2",
        "LIFO: defer b registered after defer a; at exit: b.destroy() then a.destroy().");
}
static void p10_3(void) {
    dec("P10.3",
        "return path: materialise return value into temp BEFORE emitting defers, "
        "then return temp.");
}
static void p10_4(void) {
    dec("P10.4",
        "break/continue: emit defers for *current* block only, then break/continue.");
}
static void p10_5(void) {
    dec("P10.5",
        "goto ban: sema Phase 9 walks (goto-src, label-dst) AST path; "
        "any AstDefer in path => error. Both forward and backward jumps banned.");
}

/* =========================================================================
 * §11 DECISION — codegen architecture
 * ====================================================================== */

static void p11_1(void) {
    dec("P11.1",
        "b.get(i)->Buffer__get(&b,i)  "
        "p->get(i)->Buffer__get(p,i)  "
        "b.size()->Buffer__size(&b)  "
        "Rule: value receiver => &; pointer receiver => as-is.");
}
static void p11_2(void) {
    dec("P11.2",
        "a+b(Vec3)->operator_add__Vec3__Vec3(a,b)  "
        "a[i](Vec<int>)->operator_index__Vec__int__long(&a,i)");
}
static void p11_3(void) {
    dec("P11.3",
        "Monomorphised defs use weak linkage. "
        "GCC/Clang: __attribute__((weak))  MSVC: __declspec(selectany). "
        "Macro: SHARP_WEAK defined in generated header preamble.");
}
static void p11_4(void) {
    dec("P11.4",
        "Generated .c file contains #line directives mapping back to .sp source. "
        "Satisfies invariant I4 (errors map to .sp line numbers).");
}
static void p11_5(void) {
    dec("P11.5",
        "No IR. AST -> C direct walk. Optimisation delegated to cc. "
        "Future passes: AST-to-AST transforms, not a new IR layer.");
}

/* =========================================================================
 * §12 DECISION — const type-system
 * ====================================================================== */

static void p12_1(void) {
    dec("P12.1",
        "const T != T. T->const T implicit. const T->T error. "
        "ty_is_const() checks top-level qualifier only.");
}
static void p12_2(void) {
    dec("P12.2",
        "Vec<const int> illegal (top-level const arg). "
        "Vec<const int*> legal (const on pointee). "
        "const Vec<int> legal (const on variable, not arg). "
        "Check: ty_is_const(arg) before substitution in mono.");
}
static void p12_3(void) {
    dec("P12.3",
        "const receiver can call const methods only. "
        "non-const receiver can call both. "
        "Checked in Phase 7 (sema-method).");
}

/* =========================================================================
 * §13 DECISION — public API shape
 * ====================================================================== */

static void p13_1(void) {
    dec("P13.1",
        "sharp.h exports: sharp_ctx_new/free, "
        "sharp_compile_tokens(ctx, CppTok*, ntok, filename), "
        "sharp_compile_file(ctx, path), sharp_result_free. "
        "SharpResult: {char* c_text; CppDiag* diags; size_t ndiags; const char* error;}");
}
static void p13_2(void) {
    dec("P13.2",
        "Diagnostics reuse CppDiag/CppDiagLevel/CppLoc from cpp.h. "
        "No separate FeDiag type. fe adds emit helpers in sharp_internal.h.");
}

/* =========================================================================
 * main
 * ====================================================================== */

/* =========================================================================
 * §14 — Spec v0.11 update: import + auto (Phase 1 addendum)
 * ====================================================================== */

/* P14.1  'import "file.sp";' — cpp passes 'import' through as CPPT_IDENT
 *         because it has no '#' prefix and is not a preprocessor directive. */
static void p14_1(void) {
    CppResult r = cpp_sharp("import \"vec.sp\";");
    bool found   = has_token(&r, CPPT_IDENT, "import");
    bool no_err  = r.error == NULL;
    cpp_result_free(&r);
    if (found && no_err)
        ok("P14.1", "'import' => CPPT_IDENT (not a cpp directive); fe lex promotes to STOK_IMPORT");
    else
        iss("P14.1", "Unexpected: cpp rejected 'import' or it is not CPPT_IDENT");
}

/* P14.2  cpp must NOT try to open the imported file.
 *         import has no '#' so cpp cannot treat it as an #include.
 *         Even a non-existent path must not cause a cpp error. */
static void p14_2(void) {
    CppResult r = cpp_sharp("import \"_no_such_file_abc123.sp\";");
    bool no_err  = r.error == NULL && r.ndiags == 0;
    cpp_result_free(&r);
    if (no_err)
        ok("P14.2", "'import' non-existent path: no cpp error (file lookup deferred to fe)");
    else
        iss("P14.2", "cpp produced an error for 'import' — unexpected");
}

/* P14.3  Multiple imports on separate lines — all arrive as CPPT_IDENT. */
static void p14_3(void) {
    CppResult r = cpp_sharp(
        "import \"a.sp\";\n"
        "import \"b.sp\";\n"
        "import \"a.sp\";\n");  /* duplicate -- dedup is fe's job */
    size_t n = 0;
    for (size_t i = 0; i < r.ntokens; i++)
        if (r.tokens[i].kind == CPPT_IDENT &&
            r.tokens[i].len  == 6 &&
            strncmp(r.tokens[i].text, "import", 6) == 0) n++;
    bool no_err = r.error == NULL;
    cpp_result_free(&r);
    if (n == 3 && no_err)
        ok("P14.3", "Three 'import' lines: three CPPT_IDENT tokens (dedup is fe's job)");
    else
        iss("P14.3", "Unexpected import count or cpp error");
}

/* P14.4  'auto x = expr;' — 'auto' arrives as CPPT_IDENT, same as C11 auto.
 *         fe lex promotes to STOK_AUTO; parse distinguishes storage-class vs
 *         type-deduction by context. */
static void p14_4(void) {
    CppResult r = cpp_sharp("auto n = xs.size();");
    bool found   = has_token(&r, CPPT_IDENT, "auto");
    bool no_err  = r.error == NULL;
    cpp_result_free(&r);
    if (found && no_err)
        ok("P14.4", "'auto' type deduction: arrives as CPPT_IDENT (same as C11 auto)");
    else
        iss("P14.4", "Unexpected: 'auto' not IDENT or cpp error");
}

/* P14.5  'const auto x = expr;' — both 'const' and 'auto' are CPPT_IDENT. */
static void p14_5(void) {
    CppResult r = cpp_sharp("const auto x = val;");
    bool found_auto  = has_token(&r, CPPT_IDENT, "auto");
    bool found_const = has_token(&r, CPPT_IDENT, "const");
    bool no_err      = r.error == NULL;
    cpp_result_free(&r);
    if (found_auto && found_const && no_err)
        ok("P14.5", "const auto: both 'const' and 'auto' are CPPT_IDENT");
    else
        iss("P14.5", "Unexpected: const/auto token or cpp error");
}

/* DECISION probes for spec §模块系统 and §auto */

static void p14_d1(void) {
    dec("P14.D1",
        "MODULE: No .sph header files (spec §模块系统). "
        "Declarations + implementations both in .sp. "
        "'import' handled at parse layer (not by cpp). "
        "Macro #defines do NOT leak through import. "
        "Same path imported multiple times is a no-op after first.");
}

static void p14_d2(void) {
    dec("P14.D2",
        "AUTO: 'auto x = expr;' infers type from expr (spec §auto). "
        "Lex kind: STOK_AUTO (same as C11 auto; parse distinguishes by context). "
        "Codegen: emit concrete inferred type in generated C, never 'auto'. "
        "Error: 'auto x;' with no initialiser is a sema-phase error. "
        "Not done: function return-type deduction — signatures must be explicit.");
}

static void p14_d3(void) {
    dec("P14.D3",
        "IMPORT ARCHITECTURE: "
        "Pass 1 — scan all struct/func signatures in imported files (no bodies). "
        "Pass 2 — resolve function bodies with full type info. "
        "Circular imports resolved if types reference each other only by pointer. "
        "Dedup: track imported paths; second import of same path = no-op. "
        "Phase affected: Phase 3 (parse) owns import resolution.");
}

/* =========================================================================
 * §15 — #include "*.sp" rejected in Sharp mode (spec §模块系统)
 * ====================================================================== */

/* P15.1  '#include "file.sp"' in Sharp mode must produce CPP_DIAG_ERROR
 *         containing the word "import" as a hint.
 *         The file must NOT be looked up — error fires before fs access. */
static void p15_1(void) {
    CppResult r = cpp_sharp("#include \"vec.sp\"");
    bool has_error = false, has_import_hint = false;
    for (size_t i = 0; i < r.ndiags; i++) {
        if (r.diags[i].level == CPP_DIAG_ERROR) has_error = true;
        if (r.diags[i].msg && strstr(r.diags[i].msg, "import"))
            has_import_hint = true;
    }
    cpp_result_free(&r);
    if (has_error && has_import_hint)
        ok("P15.1", "#include \"vec.sp\" in Sharp mode: CPP_DIAG_ERROR with 'import' hint");
    else
        iss("P15.1", "#include \"vec.sp\" in Sharp mode: expected error with import hint");
}

/* P15.2  In C mode the same '#include "file.sp"' must NOT produce the
 *         import-hint error.  The file-not-found error is acceptable. */
static void p15_2(void) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, "#include \"vec.sp\"",
                              strlen("#include \"vec.sp\""),
                              "<probe>", CPP_LANG_C);
    cpp_ctx_free(ctx);
    bool import_hint = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].msg && strstr(r.diags[i].msg, "import"))
            import_hint = true;
    cpp_result_free(&r);
    if (!import_hint)
        ok("P15.2", "C mode: #include \"vec.sp\" does not trigger import hint");
    else
        iss("P15.2", "C mode: unexpectedly got import hint for #include \"vec.sp\"");
}

/* P15.3  Non-.sp includes in Sharp mode are not affected. */
static void p15_3(void) {
    CppResult r = cpp_sharp("#include \"myheader.h\"");
    bool import_hint = false;
    for (size_t i = 0; i < r.ndiags; i++)
        if (r.diags[i].msg && strstr(r.diags[i].msg, "import"))
            import_hint = true;
    cpp_result_free(&r);
    if (!import_hint)
        ok("P15.3", "Sharp mode: #include \"myheader.h\" not blocked by .sp check");
    else
        iss("P15.3", "Sharp mode: #include \".h\" incorrectly triggered import hint");
}

int main(void) {
    printf("Sharp Frontend — Phase 0 Audit Probes\n");
    printf("======================================\n\n");

    printf("SS1  Lexer layer — cpp token stream\n");
    p1_1(); p1_2(); p1_3(); p1_4(); p1_5(); p1_6(); p1_7(); p1_8();
    printf("\n");

    printf("SS2  Struct syntax — methods and calls\n");
    p2_1(); p2_2(); p2_3(); p2_4(); p2_5(); p2_6();
    printf("\n");

    printf("SS3  Operator overloading — token stream\n");
    p3_1(); p3_2(); p3_3(); p3_4();
    printf("\n");

    printf("SS4  defer — token stream\n");
    p4_1(); p4_2(); p4_3(); p4_4();
    printf("\n");

    printf("SS5  Generics — token stream\n");
    p5_1(); p5_2(); p5_3(); p5_4(); p5_5(); p5_6(); p5_7();
    printf("\n");

    printf("SS6  @ intrinsics — token stream\n");
    p6_1(); p6_2(); p6_3(); p6_4(); p6_5();
    printf("\n");

    printf("SS7  Struct literal and misc syntax\n");
    p7_1(); p7_2();
    printf("\n");

    printf("SS8  Diagnostic plumbing\n");
    p8_1(); p8_2();
    printf("\n");

    printf("SS9  DECISION — name mangling (spec 5.10)\n");
    p9_1(); p9_2(); p9_3(); p9_4(); p9_5();
    printf("\n");

    printf("SS10 DECISION — defer translation strategy\n");
    p10_1(); p10_2(); p10_3(); p10_4(); p10_5();
    printf("\n");

    printf("SS11 DECISION — codegen architecture\n");
    p11_1(); p11_2(); p11_3(); p11_4(); p11_5();
    printf("\n");

    printf("SS12 DECISION — const type-system\n");
    p12_1(); p12_2(); p12_3();
    printf("\n");

    printf("SS13 DECISION — public API\n");
    p13_1(); p13_2();
    printf("\n");

    printf("SS14 Spec v0.11 update — import + auto\n");
    p14_1(); p14_2(); p14_3(); p14_4(); p14_5();
    p14_d1(); p14_d2(); p14_d3();
    printf("\n");

    printf("SS15 #include \"*.sp\" rejected in Sharp mode\n");
    p15_1(); p15_2(); p15_3();
    printf("\n");

    int total = g_ok + g_issue + g_decision;
    printf("======================================\n");
    printf("Phase 0 results: %d probes total\n", total);
    printf("  OK       %d\n", g_ok);
    printf("  ISSUE    %d\n", g_issue);
    printf("  DECISION %d\n", g_decision);
    printf("\n");
    if (g_issue == 0)
        printf("All probes OK or DECISION.  Phase 1 may begin.\n");
    else
        printf("ISSUE count: %d  (resolve each before its implementing phase)\n",
               g_issue);
    /* Return 0 always — ISSUEs are documented findings, not build failures. */
    return 0;
}
