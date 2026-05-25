/* p123 — __typeof__ inside GNU statement expression ({ })
 * KNOWN-ERROR: sema reports "unary arithmetic on non-arithmetic type"
 * The typeof expression inside ({ }) should be handled as opaque
 * and not checked by sema (it is a GCC extension, not standard C11).
 * Expected: compiles, token-identical
 * Actual:   sharpc sema error
 */
#define ABS(x) ({ __typeof__(x) _t = (x); _t < 0 ? -_t : _t; })
int main(void) { int r = ABS(-5); return r - 5; }
