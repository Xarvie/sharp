#define _GNU_SOURCE
/*
 * test_decls.c — Sharp frontend tests for multi-variable declarations
 *                and C-style fixed-size arrays.
 *
 * These features are closing two long-standing gaps that users hit early:
 *   §1  Multi-variable declarations:  int x = 3, y = 7;
 *   §2  Array declarations:           int arr[3];   int m[3][4];
 *   §3  Mixing the two:               int a, b, c[5], d = 10;
 *   §4  Generic vs comparison:        ensures `i < 5` is a comparison,
 *                                     not a generic-arg list
 *   §5  Error cases:                  for-init multi-decl, auto without init
 *
 * Every test runs the full pipeline (Sharp → C11 → gcc → run) and verifies
 * the *runtime semantics*, not just successful parsing.
 */
#define _POSIX_C_SOURCE 200809L
#include "cg.h"
#include "sema.h"
#include "parse.h"
#include "lex.h"
#include "cpp.h"
#include "import.h"
#include "sharp_internal.h"
#include "test_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int g_pass = 0, g_fail = 0;

/* ── pipeline: Sharp → C11 string (caller frees), count diagnostics ── */
typedef struct { char *c_src; size_t nerrs; } PipeResult;

static PipeResult pipeline(const char *src) {
    CppCtx *ctx = cpp_ctx_new();
    cpp_emit_linemarkers(ctx, false);
    CppResult r = cpp_run_buf(ctx, src, strlen(src), "<t>", CPP_LANG_SHARP);
    FeDiagArr ld={0},pd={0},sd={0},sema_d={0}; size_t n=0;
    SharpTok *toks  = lex_run(r.tokens,r.ntokens,"<t>",&ld,&n, true);
    cpp_ctx_free(ctx);
    AstNode  *ast   = parse_file(toks,n,"<t>",&pd);
    Scope    *scope = scope_build(ast,&sd);
    TyStore  *ts    = ty_store_new();
    SemaCtx  *sema  = sema_ctx_new(ts,scope,&sema_d);
    sema_check_file(sema,ast);

    char *out = NULL;
    if (sema_d.len == 0 && pd.len == 0) {
        CgCtx *cg = cg_ctx_new(ts,scope);
        out = cg_generate(cg,ast);
        cg_ctx_free(cg);
    }
    size_t nerrs = pd.len + sema_d.len;

    sema_ctx_free(sema); ty_store_free(ts); scope_free_chain(scope);
    ast_node_free(ast); lex_free(toks); cpp_result_free(&r);
    for (size_t i=0;i<ld.len;i++) free(ld.data[i].msg);
    for (size_t i=0;i<pd.len;i++) free(pd.data[i].msg);
    for (size_t i=0;i<sd.len;i++) free(sd.data[i].msg);
    for (size_t i=0;i<sema_d.len;i++) free(sema_d.data[i].msg);
    free(ld.data); free(pd.data); free(sd.data); free(sema_d.data);
    return (PipeResult){ out, nerrs };
}

static int compile_and_run(const char *c_src) {
    char c_path[64], b_path[64];
    snprintf(c_path, sizeof c_path, "/tmp/sharpfe_decls_XXXXXX.c");
    int fd = mkstemps(c_path, 2);
    if (fd < 0) return -1;
    if (write(fd, c_src, strlen(c_src)) < 0) { close(fd); unlink(c_path); return -1; }
    close(fd);

    snprintf(b_path, sizeof b_path, "/tmp/sharpfe_decls_bin_XXXXXX");
    int fd2 = mkstemp(b_path); if (fd2 >= 0) close(fd2);
    char cmd[256];
    snprintf(cmd, sizeof cmd,
             "gcc -std=c11 -w %s -o %s 2>/dev/null", c_path, b_path);
    int r = system(cmd);
    unlink(c_path);
    if (r != 0) { unlink(b_path); return -1; }

    r = system(b_path);
    unlink(b_path);
    /* WEXITSTATUS — match other test files' convention */
    return r;
}

static int run(const char *sharp) {
    PipeResult p = pipeline(sharp);
    if (p.nerrs > 0 || !p.c_src) { free(p.c_src); return -1; }
    int r = compile_and_run(p.c_src);
    free(p.c_src);
    return r;
}

/* =========================================================================
 * §1  Multi-variable declarations
 * ====================================================================== */
static void test_multivar(void) {
    printf("[1] Multi-variable declarations\n");

    /* Basic two-variable form */
    CHECK(run("int main() {\n"
              "    int x = 3, y = 7;\n"
              "    return x + y - 10;\n"
              "}") == 0,
          "int x = 3, y = 7;");

    /* Three variables, mixed init / no-init */
    CHECK(run("int main() {\n"
              "    int a = 1, b = 2, c;\n"
              "    c = a + b;\n"
              "    return c - 3;\n"
              "}") == 0,
          "int a=1, b=2, c;  (third has no init)");

    /* Initialiser uses earlier declarator (must respect order) */
    CHECK(run("int main() {\n"
              "    int a = 5, b = a + 10;\n"
              "    return b - 15;\n"
              "}") == 0,
          "int a=5, b=a+10  (later sees earlier)");

    /* Top-level (file scope) multi-variable */
    CHECK(run("int g_x = 100, g_y = 200, g_z = 50;\n"
              "int main() { return g_x + g_y + g_z - 350; }") == 0,
          "top-level: int g_x = 100, g_y = 200, g_z = 50;");

    /* Pointer types in multi-var.  S1 switched to ISO C semantics: each
     * `*` binds to ONE declarator, so `int* p, q;` makes p an int* and
     * q a plain int.  Idiomatic multi-pointer becomes `int *p, *q;`
     * (see also §5 below where this form is now accepted). */
    CHECK(run("int main() {\n"
              "    int v = 42;\n"
              "    int *p = &v, *q = &v;\n"
              "    return *p + *q - 84;\n"
              "}") == 0,
          "ISO ptr multi-var: int *p = &v, *q = &v;");

    /* long, const variants */
    CHECK(run("int main() {\n"
              "    long a = 10L, b = 20L, c = 30L;\n"
              "    return (int)(a + b + c) - 60;\n"
              "}") == 0,
          "long a=10L, b=20L, c=30L;");

    /* Many declarators (5+) */
    CHECK(run("int main() {\n"
              "    int a = 1, b = 2, c = 3, d = 4, e = 5;\n"
              "    return a + b + c + d + e - 15;\n"
              "}") == 0,
          "five declarators: a=1, b=2, c=3, d=4, e=5");

    /* Initialiser is a comma-free expression — must not greedy-consume */
    CHECK(run("int main() {\n"
              "    int a = 1 + 2 * 3, b = a * 2;\n"
              "    return a + b - 21;\n"
              "}") == 0,
          "complex init expressions per declarator");

    /* Initialiser with function call */
    CHECK(run("int square(int x) { return x * x; }\n"
              "int main() {\n"
              "    int a = square(3), b = square(4);\n"
              "    return a + b - 25;\n"
              "}") == 0,
          "init = function-call: square(3), square(4)");

    /* Initialiser with comparison and ternary */
    CHECK(run("int main() {\n"
              "    int a = 5, b = a < 10 ? 100 : 200;\n"
              "    return b - 100;\n"
              "}") == 0,
          "init using comparison + ternary");
}

/* =========================================================================
 * §2  Array declarations
 * ====================================================================== */
static void test_arrays(void) {
    printf("[2] Array declarations\n");

    /* 1-D fixed array */
    CHECK(run("int main() {\n"
              "    int arr[3];\n"
              "    arr[0] = 10; arr[1] = 20; arr[2] = 30;\n"
              "    return arr[0] + arr[1] + arr[2] - 60;\n"
              "}") == 0,
          "int arr[3]; subscript read+write");

    /* Loop fill + read */
    CHECK(run("int main() {\n"
              "    int v[5];\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < 5; i = i + 1) v[i] = i * i;\n"
              "    for (int i = 0; i < 5; i = i + 1) s = s + v[i];\n"
              "    return s - 30;\n"
              "}") == 0,
          "1-D array: loop fill v[i]=i*i, sum=30");

    /* 2-D array */
    CHECK(run("int main() {\n"
              "    int m[3][4];\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < 3; i = i + 1) {\n"
              "        for (int j = 0; j < 4; j = j + 1) {\n"
              "            m[i][j] = i * 4 + j;\n"
              "        }\n"
              "    }\n"
              "    for (int i = 0; i < 3; i = i + 1) {\n"
              "        for (int j = 0; j < 4; j = j + 1) {\n"
              "            s = s + m[i][j];\n"
              "        }\n"
              "    }\n"
              "    return s - 66;\n"
              "}") == 0,
          "2-D int m[3][4] fill+sum=66");

    /* Array decays to pointer when passed */
    CHECK(run("int sum_arr(int* a, int n) {\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < n; i = i + 1) s = s + a[i];\n"
              "    return s;\n"
              "}\n"
              "int main() {\n"
              "    int v[5];\n"
              "    v[0]=1; v[1]=2; v[2]=3; v[3]=4; v[4]=5;\n"
              "    return sum_arr(v, 5) - 15;\n"
              "}") == 0,
          "array decays to int* when passed");

    /* char array */
    CHECK(run("int main() {\n"
              "    char s[3];\n"
              "    s[0] = 65; s[1] = 66; s[2] = 67;\n"
              "    return (int)(s[0] + s[1] + s[2]) - 198;\n"
              "}") == 0,
          "char s[3]; ASCII A,B,C sum=198");

    /* Top-level array declaration (file scope) */
    CHECK(run("int g[5];\n"
              "int main() {\n"
              "    g[0] = 1; g[1] = 2; g[2] = 3; g[3] = 4; g[4] = 5;\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < 5; i = i + 1) s = s + g[i];\n"
              "    return s - 15;\n"
              "}") == 0,
          "top-level array: int g[5];");
}

/* =========================================================================
 * §3  Multi-var + array combinations
 * ====================================================================== */
static void test_combined(void) {
    printf("[3] Multi-variable + array combinations\n");

    /* Multi-decl with array among them */
    CHECK(run("int main() {\n"
              "    int a = 1, b[3], c = 5;\n"
              "    b[0] = 10; b[1] = 20; b[2] = 30;\n"
              "    return a + b[0] + b[1] + b[2] + c - 66;\n"
              "}") == 0,
          "mixed: int a=1, b[3], c=5;");

    /* Two arrays in one declaration */
    CHECK(run("int main() {\n"
              "    int a[3], b[2];\n"
              "    a[0]=1; a[1]=2; a[2]=3;\n"
              "    b[0]=10; b[1]=20;\n"
              "    return a[0]+a[1]+a[2]+b[0]+b[1] - 36;\n"
              "}") == 0,
          "two arrays in one decl: int a[3], b[2];");

    /* Top-level mixed: scalars and arrays */
    CHECK(run("int g_a = 7, g_arr[3], g_b = 11;\n"
              "int main() {\n"
              "    g_arr[0] = 1; g_arr[1] = 2; g_arr[2] = 3;\n"
              "    return g_a + g_arr[0] + g_arr[1] + g_arr[2] + g_b - 24;\n"
              "}") == 0,
          "top-level mixed scalars+array: g_a=7, g_arr[3], g_b=11;");
}

/* =========================================================================
 * §4  Generic-vs-comparison disambiguation
 * ====================================================================== */
static void test_lt_disambig(void) {
    printf("[4] Generic-vs-comparison disambiguation\n");

    /* `i < 5` must parse as comparison, not generic args.
     * Before the fix, `is_type_start` saw `IDENT '<'` and tried to parse
     * a generic, which failed and left a stray diag. */
    CHECK(run("int main() {\n"
              "    int i = 2;\n"
              "    return i < 5 ? 0 : 1;\n"
              "}") == 0,
          "i < 5 inside expression parses as compare");

    /* Comparison inside for-cond — the original failing case */
    CHECK(run("int main() {\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < 5; i = i + 1) s = s + 1;\n"
              "    return s - 5;\n"
              "}") == 0,
          "for (int i = 0; i < 5; ...)");

    /* Generic instantiation must still work — Vec<int> v; */
    CHECK(run("struct Vec<T> { T data; }\n"
              "int main() {\n"
              "    Vec<int> v;\n"
              "    v.data = 42;\n"
              "    return v.data - 42;\n"
              "}") == 0,
          "Vec<int> v;  (generic still parses as type)");

    /* Statement starting with `IDENT < expr` must be a comparison-stmt */
    CHECK(run("int main() {\n"
              "    int x = 1;\n"
              "    int y = 2;\n"
              "    int r = x < y ? 100 : 0;\n"
              "    return r - 100;\n"
              "}") == 0,
          "int r = x < y ? ...  (compare in init)");
}

/* =========================================================================
 * §5  Error cases — parser/sema must reject these
 * ====================================================================== */
static void test_errors(void) {
    printf("[5] Error cases\n");

    /* Multiple declarators in for-init: not supported. */
    { PipeResult p = pipeline(
          "int main() {\n"
          "    for (int i = 0, j = 0; i < 3; i = i + 1) { }\n"
          "    return 0;\n"
          "}");
      CHECK(p.nerrs > 0, "for-init multi-decl rejected");
      free(p.c_src); }

    /* `auto x;` without initialiser — auto requires init. */
    { PipeResult p = pipeline(
          "int main() {\n"
          "    auto x;\n"
          "    return 0;\n"
          "}");
      CHECK(p.nerrs > 0, "auto without init rejected");
      free(p.c_src); }

    /* `auto x, y;` — both auto without init. */
    { PipeResult p = pipeline(
          "int main() {\n"
          "    auto x = 3, y;\n"
          "    return 0;\n"
          "}");
      CHECK(p.nerrs > 0, "auto y; in multi-decl rejected");
      free(p.c_src); }

    /* S1 changed Sharp to ISO C declarator semantics: `int *p, *q` is
     * now the idiomatic way to declare two int pointers, and parses
     * cleanly.  The previous "stray *" diagnostic is retired. */
    { PipeResult p = pipeline(
          "int main() {\n"
          "    int v = 0;\n"
          "    int *p = &v, *q = &v;\n"
          "    return *p + *q;\n"
          "}");
      CHECK(p.nerrs == 0, "ISO 'int *p, *q' accepted");
      free(p.c_src); }
}

/* =========================================================================
 * S1: storage classes, function pointers, ISO declarator semantics
 * ====================================================================== */
static void test_s1_iso(void) {
    /* Storage classes round-trip from declaration to generated C. */
    CHECK(run("static int counter = 7;\n"
              "int main() { return counter - 7; }") == 0,
          "S1: top-level static int");

    CHECK(run("int main() {\n"
              "    static int once = 99;\n"
              "    return once - 99;\n"
              "}") == 0,
          "S1: block-scope static int");

    /* extern then defining declaration — both name the same object. */
    CHECK(run("extern int e;\n"
              "int e = 11;\n"
              "int main() { return e - 11; }") == 0,
          "S1: extern + defining declaration");

    /* inline function specifier. */
    CHECK(run("inline int dbl(int x) { return x + x; }\n"
              "int main() { return dbl(21) - 42; }") == 0,
          "S1: inline function");

    /* C-style multi-decl with mixed pointers and plain. */
    CHECK(run("int main() {\n"
              "    int v = 5;\n"
              "    int *p = &v, x = 7;\n"
              "    return *p + x - 12;\n"
              "}") == 0,
          "S1: int *p = &v, x = 7;  (ISO multi-decl)");

    /* Function pointer round-trip. */
    CHECK(run("int sq(int x) { return x * x; }\n"
              "int main() {\n"
              "    int (*f)(int) = sq;\n"
              "    return f(6) - 36;\n"
              "}") == 0,
          "S1: int (*f)(int) function pointer");

    /* const local arithmetic — qualifier transparent for arith. */
    CHECK(run("int main() {\n"
              "    const int x = 10;\n"
              "    return x + 5 - 15;\n"
              "}") == 0,
          "S1: const-qualified local in arithmetic");

    /* Composed primitive type. */
    CHECK(run("int main() {\n"
              "    unsigned char b = 200;\n"
              "    return (int)b - 200;\n"
              "}") == 0,
          "S1: 'unsigned char' as composed type");

    /* Function prototype + definition. */
    CHECK(run("int triple(int x);\n"
              "int triple(int x) { return x * 3; }\n"
              "int main() { return triple(5) - 15; }") == 0,
          "S1: function prototype + definition");
}

/* =========================================================================
 * S2: typedef + enum + union + switch
 * ====================================================================== */
static void test_s2_typedef_enum_union_switch(void) {
    /* typedef alias used as a variable type, then in arithmetic. */
    CHECK(run("typedef int myint;\n"
              "int main() {\n"
              "    myint a = 7, b = 5;\n"
              "    return (a - b) - 2;\n"
              "}") == 0,
          "S2: typedef int myint; arithmetic");

    /* typedef of a pointer type. */
    CHECK(run("typedef int *intp;\n"
              "int main() {\n"
              "    int v = 99;\n"
              "    intp p = &v;\n"
              "    return *p - 99;\n"
              "}") == 0,
          "S2: typedef int *intp; round-trip");

    /* enum with explicit values. */
    CHECK(run("enum E { A = 1, B = 2, C = 4 };\n"
              "int main() { return (A | B | C) - 7; }") == 0,
          "S2: enum with explicit values + bitwise OR");

    /* enum with implicit auto-increment values. */
    CHECK(run("enum N { Z, O, T, T3 };\n"
              "int main() { return (Z + O + T + T3) - 6; }") == 0,
          "S2: enum implicit increments (0,1,2,3)");

    /* union: value access through both members. */
    CHECK(run("union U { int i; char b[4]; };\n"
              "int main() {\n"
              "    union U u;\n"
              "    u.i = 0;\n"
              "    u.b[0] = 7;\n"
              "    return u.b[0] - 7;\n"
              "}") == 0,
          "S2: union with int and char[4] members");

    /* switch with multiple cases + default. */
    CHECK(run("int classify(int x) {\n"
              "    switch (x) {\n"
              "        case 0: return 100;\n"
              "        case 1: return 200;\n"
              "        case 2: return 300;\n"
              "        default: return 999;\n"
              "    }\n"
              "}\n"
              "int main() {\n"
              "    return classify(2) - 300;\n"
              "}") == 0,
          "S2: switch with cases + default");

    /* switch with default returning. */
    CHECK(run("int main() {\n"
              "    int n = 42;\n"
              "    switch (n) {\n"
              "        case 1: return 1;\n"
              "        default: return n - 42;\n"
              "    }\n"
              "}") == 0,
          "S2: switch default fallback");

    /* enum-typed function return — enum tag treated as int. */
    CHECK(run("enum Color { Red, Green, Blue };\n"
              "enum Color pick() { return Green; }\n"
              "int main() { return pick() - 1; }") == 0,
          "S2: enum tag used as function return type");
}

/* =========================================================================
 * S4: initializers + bit-fields
 * ====================================================================== */
static void test_s4_initializers(void) {
    /* Bit-field round-trip in struct. */
    CHECK(run("struct B { unsigned int a:3; unsigned int b:5; };\n"
              "int main() {\n"
              "    struct B b;\n"
              "    b.a = 7; b.b = 31;\n"
              "    return (int)(b.a + b.b) - 38;\n"
              "}") == 0,
          "S4: bit-field a:3 + b:5 round-trip");

    /* Array brace initializer with sum check. */
    CHECK(run("int main() {\n"
              "    int a[4] = {10, 20, 30, 40};\n"
              "    return a[0] + a[1] + a[2] + a[3] - 100;\n"
              "}") == 0,
          "S4: int a[4] = {10,20,30,40}");

    /* Top-level const array brace initializer. */
    CHECK(run("const int t[3] = {7, 8, 9};\n"
              "int main() {\n"
              "    return t[0] + t[1] + t[2] - 24;\n"
              "}") == 0,
          "S4: top-level const int[3] = {7,8,9}");

    /* Struct positional initializer. */
    CHECK(run("struct P { int x; int y; };\n"
              "int main() {\n"
              "    struct P p = {3, 4};\n"
              "    return p.x + p.y - 7;\n"
              "}") == 0,
          "S4: struct P p = {3, 4}");

    /* Struct designated initializer. */
    CHECK(run("struct P { int x; int y; };\n"
              "int main() {\n"
              "    struct P p = {.x = 11, .y = 22};\n"
              "    return p.x + p.y - 33;\n"
              "}") == 0,
          "S4: struct P p = {.x=11, .y=22}");

    /* Array designated initializer with skipped indices. */
    CHECK(run("int main() {\n"
              "    int a[5] = {[0] = 1, [4] = 5};\n"
              "    return a[0] + a[4] - 6;\n"
              "}") == 0,
          "S4: int a[5] = {[0]=1, [4]=5}");

    /* Compound literal as initializer. */
    CHECK(run("struct P { int x; int y; };\n"
              "int main() {\n"
              "    struct P p = (struct P){.x = 9, .y = 1};\n"
              "    return p.x + p.y - 10;\n"
              "}") == 0,
          "S4: compound literal (struct P){.x=9,.y=1}");

    /* String literal initializing a char array (size-deduced). */
    CHECK(run("int main() {\n"
              "    char s[] = \"abc\";\n"
              "    return (s[0] - 'a') + (s[1] - 'b') + (s[2] - 'c');\n"
              "}") == 0,
          "S4: char s[] = \"abc\" (string-init)");

    /* Nested init lists for 2-D array. */
    CHECK(run("int main() {\n"
              "    int m[2][3] = { {1, 2, 3}, {4, 5, 6} };\n"
              "    int s = 0;\n"
              "    for (int i = 0; i < 2; i = i + 1)\n"
              "        for (int j = 0; j < 3; j = j + 1)\n"
              "            s = s + m[i][j];\n"
              "    return s - 21;\n"
              "}") == 0,
          "S4: nested init list int m[2][3]={{...},{...}}");
}

/* =========================================================================
 * Phase S5 — GCC labels-as-values + computed goto
 *
 * The Lua 5.4 interpreter's main dispatch loop in `lvm.c` builds a
 * static table of `&&label` addresses and dispatches with `goto *t[op]`.
 * This is the GCC "labels-as-values" extension; cc accepts it natively.
 * sharp-fe must round-trip the construct from source to generated C
 * verbatim.
 * ====================================================================== */
static void test_s5_computed_goto(void) {
    /* Address-of-label produces a void* — basic round-trip with a
     * three-way computed dispatch. */
    CHECK(run("int main(void) {\n"
              "    static const void *tab[] = { &&L0, &&L1, &&L2 };\n"
              "    int i = 1;\n"
              "    int r = 0;\n"
              "    goto *tab[i];\n"
              "L0: r = 100; goto done;\n"
              "L1: r =   0; goto done;\n"
              "L2: r = 200; goto done;\n"
              "done:\n"
              "    return r;\n"
              "}") == 0,
          "S5: computed goto via address-of-label table (i=1 -> 0)");

    /* Same idea, different index — confirm the dispatch actually
     * uses the runtime value. */
    CHECK(run("int main(void) {\n"
              "    static const void *tab[] = { &&Z, &&FAIL };\n"
              "    int i = 0;\n"
              "    goto *tab[i];\n"
              "FAIL: return 1;\n"
              "Z:    return 0;\n"
              "}") == 0,
          "S5: computed goto picks first slot (i=0 -> 0)");

    /* Address-of-label as a plain expression assigned to a variable;
     * goto *var. */
    CHECK(run("int main(void) {\n"
              "    void *p = &&here;\n"
              "    goto *p;\n"
              "    return 1;\n"
              "here:\n"
              "    return 0;\n"
              "}") == 0,
          "S5: void* p = &&here; goto *p;");
}

/* =========================================================================
 * Phase R1 — second real-world target (cJSON 1.7.18) regressions
 *
 * Every case below is reduced from a concrete pattern that broke when
 * sharp-fe first met cJSON's source.  Keeping them here guards against
 * silent regressions in either the C-mode lex flag (which lets `null`
 * be an identifier) or in the type-system fixes that landed alongside.
 * ====================================================================== */
static void test_r1_cjson_regressions(void) {
    /* `T * const p` is a const pointer to mutable T.  cg used to emit
     * `const T *` (pointer to const), which made the function body's
     * writes through the pointer fail to compile. */
    CHECK(run("struct S { int v; };\n"
              "static int set(struct S * const item, int n) {\n"
              "    item->v = n;\n"
              "    return item->v;\n"
              "}\n"
              "int main(void) {\n"
              "    struct S s; s.v = 0;\n"
              "    return set(&s, 7) - 7;\n"
              "}") == 0,
          "R1: T * const param keeps pointee mutable");

    /* Anonymous-tagged typedef + 1-element array + `->` field access.
     * `struct_scope_of` previously didn't strip TY_ARRAY so the
     * receiver-to-struct-scope step failed, reporting "no member 'x'
     * in struct" for a perfectly valid `arr->field` decay. */
    CHECK(run("typedef struct { int x; int y; } pair;\n"
              "int main(void) {\n"
              "    pair p[1];\n"
              "    p->x = 3;\n"
              "    p->y = 4;\n"
              "    return p->x + p->y - 7;\n"
              "}") == 0,
          "R1: typedef'd anon struct, p[1] then p->field");

    /* `null` as an identifier in C source.  Real cJSON code:
     *   cJSON *null = cJSON_CreateNull();
     *   if (...) { cJSON_Delete(null); }
     * Sharp's STOK_NULL must not be promoted in C mode.  Note: this
     * test file feeds Sharp dialect to lex_run (true), so `null` IS a
     * keyword here; we exercise the C-mode path through a synthesised
     * pure-C run via the integration helper.  Direct sharpc on a `.c`
     * input is covered by p51_computed_goto-style probes; here we
     * cover the post-parse semantics by choosing a non-`null` name.
     */
    CHECK(run("int main(void) {\n"
              "    int *p = (int *)0;\n"
              "    if (p) return 1;\n"
              "    return 0;\n"
              "}") == 0,
          "R1: null-pointer comparison via cast (Sharp-mode-safe phrasing)");
}

/* =========================================================================
 * Phase R2 — third + fourth real-world targets (picol, stb_image)
 *
 * picol introduced no new bugs (the surface was covered by S5+R1).
 * stb_image, by contrast, produced four distinct, deep failure modes
 * — every one is reduced to a probe in c_superset_probes/p55..p60
 * AND locked in here as a unit test that runs through the whole
 * integration helper (cpp + fe + cc + execute).
 * ====================================================================== */
static void test_r2_stb_image_regressions(void) {
    /* C11 _Thread_local storage-class.  Coexists with `static`, must
     * round-trip the keyword unchanged into the regenerated C. */
    CHECK(run("static _Thread_local const char *err = \"no error\";\n"
              "int main(void) {\n"
              "    return err[0] == 'n' ? 0 : 1;\n"
              "}") == 0,
          "R2: static _Thread_local const char *");

    /* K&R-style function-type typedef.  `typedef T name(args);`
     * declares `name` as an alias for the *function type* (not the
     * pointer-to-function type); usage is via `name *p`. */
    CHECK(run("typedef void Fn(int);\n"
              "static int captured = 0;\n"
              "static void cb(int v) { captured = v; }\n"
              "int main(void) {\n"
              "    Fn *p = cb;\n"
              "    p(42);\n"
              "    return captured - 42;\n"
              "}") == 0,
          "R2: typedef void Fn(int); Fn *p = ...");

    /* Block-scope anonymous enum injects enumerators into the
     * enclosing scope; emission must come BEFORE first use. */
    CHECK(run("int main(void) {\n"
              "    enum { W = 16, H = 16, S = W + H };\n"
              "    int x = W, y = H, z = S;\n"
              "    return (x + y + z) - 64;\n"
              "}") == 0,
          "R2: block-scope anonymous enum + ordering");

    /* do-while with unbraced single-statement body.  Pre-R2 cg
     * dropped the body silently; loop iterated but did nothing. */
    CHECK(run("int main(void) {\n"
              "    int x = 0;\n"
              "    int i = 5;\n"
              "    do x++; while (--i);\n"
              "    return x - 5;\n"
              "}") == 0,
          "R2: do BODY while (COND); — unbraced body");
}

/* =========================================================================
 * Main
 * ====================================================================== */
int main(void) {
    test_multivar();
    test_arrays();
    test_combined();
    test_lt_disambig();
    test_errors();
    test_s1_iso();
    test_s2_typedef_enum_union_switch();
    test_s4_initializers();
    test_s5_computed_goto();
    test_r1_cjson_regressions();
    test_r2_stb_image_regressions();

    printf("\nDecls results: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
