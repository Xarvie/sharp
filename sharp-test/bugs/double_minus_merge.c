/*
 * sharpc bug: double minus merged into -- (decrement) after macro expansion
 *
 * Pattern: `#define NEG -0x6200` then `-NEG` → text `- -0x6200` (correct)
 * but the AST → CG output produced `--0x6200` which zig cc rejected as
 * an attempt to decrement a non-lvalue.
 *
 * Root cause: cg.c's AST_UNARY handler did not insert spacing when the
 * operand was another unary operator, so `-(-x)` emitted `--x`.
 */

/* --- Original test case --- */

#define NEG_VAL -0x6200

int main(void) {
    int x = -NEG_VAL;
    return x == 0x6200 ? 0 : 1;
}

/* --- Extension: direct expression (no macro) --- */
void test_direct(void) {
    int a = -(-5);
    int b = +(+3);
    (void)a; (void)b;
}

/* --- Extension: chained unary ops --- */
void test_chained(void) {
    int x = -(-(-7));
    int y = +(+2);
    int z = !(!1);
    int w = ~(~0xFF);
    (void)x; (void)y; (void)z; (void)w;
}

/* --- Extension: mixed unary ops --- */
void test_mixed(void) {
    int x = -+5;
    int y = +-3;
    (void)x; (void)y;
}

/* --- Extension: unary with parens --- */
void test_parens(void) {
    int a = -((-10));
    int b = +((+8));
    (void)a; (void)b;
}

/* --- Extension: double minus with macro producing unary --- */
#define ADD_ONE +1
#define SUB_ONE -1

void test_macro_unary(void) {
    int a = -SUB_ONE;   /* -(-1) */
    int b = +ADD_ONE;   /* +(+1) */
    int c = -ADD_ONE;   /* -(+1) */
    int d = +SUB_ONE;   /* +(-1) */
    (void)a; (void)b; (void)c; (void)d;
}
