/* p75_sizeof_array_type.c
 * Phase R7: sizeof(T[expr]) — array type inside sizeof operand.
 * The DEBUG_STATIC_ASSERT macro in zstd expands to
 * (void)sizeof(char[(cond)?1:-1]) which requires parsing the
 * array type as a sizeof operand. */
static void check(void) {
    /* sizeof of a fixed-size array type */
    if (sizeof(int[4]) != 16) __builtin_trap();
    /* sizeof of a VLA-style array type (size from expression) */
    int n = 3;
    (void)sizeof(char[n]);          /* valid C99 VLA-in-sizeof */
    /* Classic static-assert pattern */
    (void)sizeof(char[(1 == 1) ? 1 : -1]);
}
int main(void) { check(); return 0; }
