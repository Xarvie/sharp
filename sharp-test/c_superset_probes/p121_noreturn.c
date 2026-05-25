/* p121 — _Noreturn function specifier (C11 6.7.4)
 * KNOWN-ERROR: sharpc parses _Noreturn as unknown token at top level
 * _Noreturn is a C11 standard keyword that must pass through verbatim.
 * Expected: _Noreturn void fatal ( void ) ;
 * Actual:   sharpc exits with error
 */
_Noreturn void fatal(const char *msg);
int main(void) { return 0; }
