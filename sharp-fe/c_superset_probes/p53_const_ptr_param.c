/* p53_const_ptr_param.c — `T * const param` (const POINTER, mutable pointee).
 *
 * In ISO C the const can sit on either side of a pointer:
 *   const T *p   — pointer to const T (pointee is const)
 *   T * const p  — const pointer to T (the pointer itself is const, but
 *                  *p is a mutable T)
 *
 * cJSON 1.7.18 declares `static cJSON_bool parse_number(cJSON * const item, ...)`
 * and the body writes `item->valuedouble = number;`.  That's only valid
 * if cg places the const on the right side of the pointer; otherwise cc
 * rejects the assignment.
 *
 * Pass criterion: round-trips through sharp-fe to C that compiles
 * unchanged and exits 0.
 */
struct S { int v; };

static int func(struct S * const item, int n) {
    item->v = n;       /* must be legal: pointee is mutable */
    return item->v;
}

int main(void) {
    struct S s; s.v = 0;
    return func(&s, 7) - 7;
}
