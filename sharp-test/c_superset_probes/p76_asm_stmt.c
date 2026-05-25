/* p76_asm_stmt.c
 * Phase R7: __asm__ as a standalone statement (GCC extended inline asm).
 * zstd uses __asm__("cpuid" : ...) for CPU feature detection.
 * sharp-fe must accept and discard it without error.
 * The test verifies that other code in the function still executes. */
static int detect(void) {
    int result = 0;
#if defined(__x86_64__) || defined(__i386__)
    __asm__("" : : : "memory");  /* memory barrier — simplest valid asm */
#endif
    result = 42;
    return result;
}
int main(void) {
    return (detect() == 42) ? 0 : 1;
}
