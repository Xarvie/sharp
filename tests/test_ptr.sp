/* compiler: any */
/* TDD-Expr-1: pointer, address-of, dereference */

int main() {
    int x = 42;
    int* p = &x;
    return *p; /* 42 */
}