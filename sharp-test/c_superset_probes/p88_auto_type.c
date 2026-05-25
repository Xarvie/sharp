/* p88_auto_type.c
 * Phase R12: __auto_type GCC type-inference extension.
 * Used in stdatomic.h macros. */
static int gval = 42;
int main(void) {
    __auto_type p = &gval;
    __auto_type v = *p;
    return (v == 42) ? 0 : 1;
}
