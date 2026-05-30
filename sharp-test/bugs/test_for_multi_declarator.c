/*
 * test_for_multi_declarator.c — Bug: for-loop multi-declarator init
 * loses pointer/array/function-pointer suffixes on continuation
 * declarators.
 *
 * Root cause: cg.c AST_FOR / AST_BLOCK path emitted only the variable
 * name for continuation declarators (i > 0), omitting the `*` pointer
 * prefix, `[N]` array suffix, and `(*)(params)` function-pointer
 * declarator.  This produced invalid C like `const char *spot, next`
 * instead of `const char *spot, *next`.
 *
 * Fix: use cg_emit_comma_cont_declarator() for continuation declarators
 * in the for-loop AST_BLOCK code path, matching the existing block-level
 * comma-continuation emission.
 */

typedef unsigned int Uint32;

/* === Case 1: Original bug — const char * with for-loop multi-declarator === */
int test_const_char_ptr_for(void) {
    const char *hint = "x";
    for (const char *spot = hint, *next; *spot; spot = next) {
        Uint32 spot_mask;
        const char *end = spot + 1;
        if (!*end) {
            spot_mask = 1;
        }
        next = end + 1;
        (void)spot_mask;
    }
    return 0;
}

/* === Case 2: Two pointers, both with initializers === */
int test_two_ptr_init(void) {
    for (const char *a = "hello", *b = "world"; *a; a++, b++) {
        (void)a;
        (void)b;
    }
    return 0;
}

/* === Case 3: Mixed pointer and non-pointer === */
int test_mixed_ptr_nonptr(void) {
    for (int *p = (int[]){1,2,3}, count = 3; count > 0; p++, count--) {
        (void)p;
    }
    return 0;
}

/* === Case 4: Double pointer ** === */
int test_double_ptr(void) {
    int x = 1;
    int *px = &x;
    for (int **pp = &px, **qq = &px; pp; pp = 0) {
        (void)pp;
        (void)qq;
    }
    return 0;
}

/* === Case 5: Pointer with const qualifier on the pointer itself === */
int test_ptr_const_ptr(void) {
    int val = 42;
    for (int * const p = &val, * const q = &val; *p; ) {
        (void)p;
        (void)q;
        break;
    }
    return 0;
}

/* === Case 6: Array declarator in for-loop init === */
int test_array_declarator(void) {
    for (int arr[3] = {1,2,3}, idx = 0; idx < 3; idx++) {
        (void)arr;
    }
    return 0;
}

/* === Case 7: Three declarators in for-loop === */
int test_three_declarators(void) {
    int a = 1;
    for (int *p = &a, *q = &a, *r = &a; *p; ) {
        (void)p;
        (void)q;
        (void)r;
        break;
    }
    return 0;
}

/* === Case 8: Typedef'd type with pointer in for-loop === */
int test_typedef_ptr_for(void) {
    typedef unsigned int U32;
    U32 val = 10;
    for (U32 *p = &val, *q = &val; *p; ) {
        (void)p;
        (void)q;
        break;
    }
    return 0;
}

/* === Case 9: Non-for-loop multi-declarator (regression check) === */
int test_block_multi_declarator(void) {
    const char *a = "x", *b = "y";
    (void)a;
    (void)b;
    return 0;
}

/* === Case 10: Single pointer declarator in for-loop (regression) === */
int test_single_ptr_for(void) {
    int val = 1;
    for (int *p = &val; *p; ) {
        (void)p;
        break;
    }
    return 0;
}

/* === Case 11: volatile pointer in for-loop multi-declarator === */
int test_volatile_ptr(void) {
    volatile int x = 1;
    for (volatile int *p = &x, *q = &x; *p; ) {
        (void)p;
        (void)q;
        break;
    }
    return 0;
}

/* === Case 12: Pointer-to-const in for-loop multi-declarator === */
int test_ptr_to_const(void) {
    for (const int *p = (const int[]){1,2,3}, *q = (const int[]){4,5,6}; *p; p++) {
        (void)p;
        (void)q;
        break;
    }
    return 0;
}

/* === Case 13: char * with string literal init in for-loop === */
int test_char_ptr_string(void) {
    for (char *s = "abc", *t = "def"; *s; s++, t++) {
        (void)s;
        (void)t;
    }
    return 0;
}

/* === Case 14: Nested for-loop with multi-declarator === */
int test_nested_for(void) {
    for (int *outer = (int[]){1,2,3}, oi = 0; oi < 3; oi++) {
        for (int *inner = (int[]){4,5,6}, ii = 0; ii < 3; ii++) {
            (void)outer;
            (void)inner;
        }
    }
    return 0;
}

/* === Case 15: while loop with multi-declarator before it (regression) === */
int test_while_multi_decl(void) {
    int a = 1;
    int *p = &a, *q = &a;
    while (*p) {
        (void)p;
        (void)q;
        break;
    }
    return 0;
}

int main(void) {
    test_const_char_ptr_for();
    test_two_ptr_init();
    test_mixed_ptr_nonptr();
    test_double_ptr();
    test_ptr_const_ptr();
    test_array_declarator();
    test_three_declarators();
    test_typedef_ptr_for();
    test_block_multi_declarator();
    test_single_ptr_for();
    test_volatile_ptr();
    test_ptr_to_const();
    test_char_ptr_string();
    test_nested_for();
    test_while_multi_decl();
    return 0;
}
