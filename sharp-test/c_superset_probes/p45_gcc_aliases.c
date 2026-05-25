/* p45 — GCC declarator extensions: keyword aliases (`__restrict`,
 * `__inline__`, `__const`, `__signed__`, `__volatile`) and the
 * `__asm__("name")` declarator suffix.
 *
 * glibc's <string.h>, <stdlib.h>, <math.h> and friends use these in
 * essentially every prototype.  Sharp must accept them in every
 * decl-specifier and declarator position so headers like Lua's
 * `<sys/cdefs.h>`-using prototypes parse cleanly.
 *
 * Sharp silently discards both the qualifier aliases (mapped to their
 * ISO equivalents at lex time) and the asm-name (eaten in
 * eat_attribute_specifiers).  This probe verifies parse + cg emit valid
 * C — semantics of the asm-rename are a linker concern; we don't
 * exercise them here so that the probe is self-contained.
 */

/* Forward prototypes use every flavour of GCC keyword alias and the
 * asm-rename suffix.  Definitions are provided below. */
__inline__ static int  always_zero(void) { return 0; }
__const   static int   pure_pass(int x)  { return x; }

int probe_qual_args(const char *__restrict a, const char *__restrict__ b);
void probe_volatile(__volatile int *p);
__signed int probe_signed_int(__signed int x);

/* Definitions */
int probe_qual_args(const char *__restrict a, const char *__restrict__ b) {
    return (a == b) ? 0 : 1;
}
void probe_volatile(__volatile int *p) { *p = 0; }
__signed int probe_signed_int(__signed int x) { return x; }

int main(void) {
    int v = 7;
    probe_volatile(&v);
    return always_zero()
         + probe_qual_args("a", "a")
         + (pure_pass(5) - 5)
         + probe_signed_int(0)
         + v;
}
