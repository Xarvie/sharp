/* p96_asm_named_operands.c
 * Phase R15: GCC inline asm with named operands [name] and
 * volatile qualifier. mbedtls constant_time_impl.h uses this. */
static int opaque(int x) {
#if defined(__GNUC__)
    asm volatile ("" : [x] "+r" (x) :);
#endif
    return x;
}

int main(void) { return opaque(0); }
