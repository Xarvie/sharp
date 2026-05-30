/*
 * test_anon_union_global.c — Bug: union with variable declarators at top level
 * not parsed correctly.
 *
 * Root cause: parse.c's top-level union handler (STOK_UNION branch) only
 * parsed the union definition and consumed the trailing semicolon, without
 * checking for variable declarators after the closing brace.  The struct
 * handler had this logic (BUG-004 guard + parse_init_declarator_list), but
 * the union handler did not.
 *
 * This meant `union { Uint64 u64; double d; } inf = { ... };` at top level
 * would fail with "unexpected token 'inf' at top level".
 *
 * Fix: add the same variable-declarator handling to the union branch as
 * exists in the struct branch, including the BUG-004 typedef-name guard.
 */

typedef unsigned long long Uint64;

/* === Case 1: Original bug — anonymous union with initializer === */
union {
    Uint64 u64;
    double d;
} inf = { 0x7ff0000000000000ULL };

int test_anon_union_global(void) {
    if (inf.d > 1.0) return 1;
    return 0;
}

/* === Case 2: Tagged union with variable declarator === */
union UData {
    int i;
    double d;
};

union UData g_udata = { .i = 42 };

int test_tagged_union_var(void) {
    return g_udata.i;
}

/* === Case 3: Anonymous union with multiple declarators === */
union {
    int val;
    unsigned int uval;
} a_val = { .val = 10 }, b_val = { .val = 20 };

int test_multi_union_decls(void) {
    return a_val.val + b_val.val;
}

/* === Case 4: Anonymous union with pointer declarator === */
union {
    int *pi;
    char *pc;
} ptr_holder = { .pi = (int[]){1,2,3} };

int test_union_ptr_decl(void) {
    return ptr_holder.pi != 0 ? 1 : 0;
}

/* === Case 5: Tagged union with pointer declarator === */
union PtrUnion {
    int *ip;
    double *dp;
};

union PtrUnion g_pu = { .ip = (int[]){5} };

int test_tagged_union_ptr(void) {
    return g_pu.ip != 0 ? 1 : 0;
}

/* === Case 6: Anonymous struct with variable declarator (regression) === */
struct {
    int x;
    int y;
} point = { 3, 4 };

int test_anon_struct_var(void) {
    return point.x + point.y;
}

/* === Case 7: Anonymous union without initializer === */
union {
    char c;
    unsigned char uc;
} byte_holder;

int test_union_no_init(void) {
    byte_holder.c = 'A';
    return byte_holder.uc;
}

/* === Case 8: Nested anonymous union inside struct with global var === */
struct Vec2 {
    union {
        struct { float x, y; };
        float data[2];
    };
};

struct Vec2 g_vec = { .x = 1.0f, .y = 2.0f };

int test_nested_anon_union_global(void) {
    return g_vec.x > 0.0f ? 1 : 0;
}

/* === Case 9: Tagged union with array declarator === */
union IntOrFloat {
    int i;
    float f;
};

union IntOrFloat iof_arr[2] = { { .i = 1 }, { .f = 2.0f } };

int test_union_array_decl(void) {
    return iof_arr[0].i;
}

/* === Case 10: Anonymous union with designated initializer === */
union {
    struct { unsigned int lo, hi; };
    unsigned long long full;
} parts = { .lo = 0xABCD, .hi = 0xEF01 };

int test_designated_init(void) {
    return parts.lo != 0;
}

int main(void) {
    int result = 0;
    result += test_anon_union_global();
    result += test_tagged_union_var();
    result += test_multi_union_decls();
    result += test_union_ptr_decl();
    result += test_tagged_union_ptr();
    result += test_anon_struct_var();
    result += test_union_no_init();
    result += test_nested_anon_union_global();
    result += test_union_array_decl();
    result += test_designated_init();
    return result > 0 ? 0 : 1;
}
