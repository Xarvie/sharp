/* p46 — `__asm__("name")` declarator-suffix symbol rename.
 *
 * Used by glibc's `<sys/cdefs.h>` __REDIRECT macros to give a symbol a
 * different linker name from its source-level identifier:
 *
 *   extern int foo(int) __asm__("real_foo");
 *
 * Sharp accepts and silently discards the asm-rename in the front end.
 * The transpiled C output therefore contains a plain prototype with
 * the source-level name; cc-side codegen can re-derive the rename
 * from the original headers when present.  Here we declare and define
 * with the same source-level name so the linker is satisfied without
 * exercising the rename behaviour itself.
 */

extern int probe_dup(int x) __asm__("probe_dup");
extern int probe_triple(int x) __asm("probe_triple");
extern int probe_quad(int x) asm("probe_quad");

int probe_dup(int x)    { return x + x; }
int probe_triple(int x) { return x * 3; }
int probe_quad(int x)   { return x * 4; }

int main(void) {
    return (probe_dup(3)    - 6)
         + (probe_triple(2) - 6)
         + (probe_quad(2)   - 8);
}
