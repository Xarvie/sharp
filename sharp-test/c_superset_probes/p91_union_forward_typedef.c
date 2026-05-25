/* p91_union_forward_typedef.c
 * Phase R13 fix: `typedef union Foo T;` followed by `union Foo { ... };`
 * must correctly promote the forward stub to the union body.
 * Previously the S5 is_union mismatch blocked promotion, causing
 * "redefinition" errors and "no member" on field access. */
typedef union my_union my_union_t;

union my_union {
    int   as_int;
    float as_float;
    char  bytes[4];
};

int main(void) {
    my_union_t u;
    u.as_int = 42;
    u.as_float = 0.0f;
    union my_union v;
    v.as_int = 7;
    return (u.as_int == 0 && v.as_int == 7) ? 0 : 1;
}
