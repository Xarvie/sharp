/* p122 — _Alignas alignment specifier (C11 6.7.5)
 * KNOWN-ERROR: sharpc parses _Alignas as unknown token at top level
 * _Alignas is a C11 standard keyword.
 * Expected: _Alignas ( 16 ) int x ;
 * Actual:   sharpc exits with error
 */
_Alignas(16) int x;
_Alignas(double) int y;
int main(void) { return 0; }
