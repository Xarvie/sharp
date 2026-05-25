/* p87_typeof_type.c
 * Phase R12: __typeof__ / typeof in type declarations.
 * Used in GCC macros; verbatim pass-through to cc. */
int main(void) {
    int val = 7;
    __typeof__(val) copy = val;
    __typeof__(int) x = 42;
    return (copy == 7 && x == 42) ? 0 : 1;
}
