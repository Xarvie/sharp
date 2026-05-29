/*
 * sharpc bug: struct field array dimension lost in comma-declaration group
 *
 * Pattern: `uint32_t A, B, C, X[16];` inside a struct — the CG's
 * cg_emit_field_group continuation handler only collected AST_TYPE_PTR
 * wrappers, ignoring AST_TYPE_ARRAY. The result was `X` instead of `X[16]`.
 */

typedef unsigned int uint32_t;
typedef long long int64_t;

int main(void) {
    /* Original test case: single array at end of comma group */
    struct {
        uint32_t A, B, C, X[16];
    } local;

    local.X[0] = 1;
    local.A = local.X[0];
    local.X[15] = 99;

    /* Extension 1: multiple array fields in same comma group */
    struct {
        int a, b[3], c, d[5];
    } multi;

    multi.b[0] = 10;
    multi.b[2] = 12;
    multi.d[4] = 50;

    /* Extension 2: array at the beginning of comma group */
    struct {
        uint32_t arr[8], single;
    } beg;

    beg.arr[7] = 77;
    beg.single = beg.arr[0];

    /* Extension 3: nested arrays in comma group */
    struct {
        int m[4][2], n;
    } nested;

    nested.m[3][1] = 42;

    /* Extension 4: const array in comma group */
    struct {
        int x, y;
        const int lut[4];
    } lookup;

    /* Extension 5: pointer and array mix in comma group */
    struct {
        uint32_t *ptr, val, buf[10];
    } mixed;

    mixed.val = 5;
    mixed.buf[9] = 99;
    mixed.ptr = &mixed.val;

    /* Suppress unused warnings */
    (void)local; (void)multi; (void)beg; (void)nested; (void)lookup; (void)mixed;

    return local.A == 1 && multi.b[0] == 10 && beg.arr[7] == 77 &&
           nested.m[3][1] == 42 && mixed.buf[9] == 99 ? 0 : 1;
}
