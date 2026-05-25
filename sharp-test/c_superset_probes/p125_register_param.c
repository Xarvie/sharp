/* p125 — register storage class on function parameters
 * KNOWN-DIFFER: sharpc drops 'register' from parameter declarations
 * Expected: int add ( register int a , register int b ) { ... }
 * Actual:   int add ( int a , int b ) { ... }
 */
int add(register int a, register int b) { return a + b; }
int scale(register int x, register int factor) { return x * factor; }
int main(void) { return add(1, 2) - 3; }
