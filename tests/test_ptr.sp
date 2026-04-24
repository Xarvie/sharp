/* TDD-Expr-1: pointer, address-of, dereference */

i32 main() {
    i32 x = 42;
    i32* p = &x;
    return *p; /* 42 */
}
