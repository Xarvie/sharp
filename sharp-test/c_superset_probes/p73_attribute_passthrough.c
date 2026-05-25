/* p73_attribute_passthrough.c
 * Phase R6: GCC __attribute__((...)) passthrough.
 *
 * Verifies that sharp-fe preserves function and variable
 * __attribute__((...)) specifiers in the generated C so that the
 * downstream C compiler sees them.
 *
 *  always_inline (leading position, before return type):
 *    Forces inlining — essential for lz4's correctness where the
 *    tableType enum must constant-propagate through the call chain.
 *
 *  noreturn (trailing position, after parameter list):
 *    Tells the compiler the function never returns; suppresses spurious
 *    "control reaches end of non-void function" warnings in callers.
 *
 *  unused (variable attribute, after declarator):
 *    Suppresses "set but not used" warnings from -Wall.
 *
 * The probe is self-contained (no #include) so sharpc can preprocess it
 * without a system include path.  __builtin_trap() is available as a
 * GCC builtin without declaration.
 */

/* Leading attribute: LZ4_FORCE_INLINE expands to exactly this form
 * after cpp: `static __inline__ __attribute__((always_inline))`.
 * We use `inline` (ISO keyword) to keep the probe portable. */
static inline __attribute__((always_inline)) int add(int a, int b) {
    return a + b;
}

/* Trailing attribute on a forward declaration — classic noreturn pattern
 * used by glibc's abort() and many library "die" helpers. */
static void panic(void) __attribute__((noreturn));
static void panic(void) {
    __builtin_trap();
}

/* Variable attribute: suppresses -Wunused-variable. */
static int guard __attribute__((unused)) = 0;

int main(void) {
    guard = 1;
    if (add(3, 4) != 7) panic();
    if (guard != 1)     panic();
    return 0;
}
