/* p77_sizeof_noparen.c
 * Phase R8: `sizeof expr` without parentheses (ISO C 6.5.3.4).
 * Parens are required only for type operands; expressions do not need them.
 * Brotli's platform.h uses `sizeof t` where `t` is a local variable. */
int main(void) {
    int   x = 42;
    float f = 1.5f;
    char  buf[16];
    if (sizeof x   != sizeof(int))   return 1;
    if (sizeof f   != sizeof(float)) return 2;
    if (sizeof buf != 16)            return 3;
    int *p = &x;
    if (sizeof *p != sizeof(int))    return 4;
    return 0;
}
