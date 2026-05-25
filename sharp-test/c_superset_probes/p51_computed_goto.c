/* p51_computed_goto.c — labels-as-values + computed goto (GCC extension).
 *
 * Real-world pattern: Lua 5.4 interpreter's main dispatch loop in lvm.c
 * uses a static array of label addresses (`&&label`) and dispatches with
 * `goto *table[op];`.  This probe is a minimal model of that idiom.
 *
 * Pass criterion: round-trips through sharp-fe to C that compiles under
 * cc and exits 0.  cc must accept the extension (gcc and clang both do).
 */
int main(void) {
    static const void *const tab[] = { &&L0, &&L1, &&L2 };
    int i = 1;
    int r = 0;
    goto *tab[i];
L0: r = 100; goto done;
L1: r =   0; goto done;
L2: r = 200; goto done;
done:
    return r;
}
