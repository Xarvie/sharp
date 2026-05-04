/* p74_alignof.c
 * Phase R7: __alignof / __alignof__ / _Alignof keyword aliases.
 * zstd uses ZSTD_ALIGNOF(T) which expands to __alignof(T) on GCC.
 * All three spellings must produce the correct alignment value. */
typedef struct { char c; int i; } S;

int main(void) {
    /* ISO C11 */
    if (_Alignof(S) < 4) return 1;
    /* GCC aliases — both spellings must work */
    if (__alignof__(int) != _Alignof(int)) return 2;
    if (__alignof(char)  != 1) return 3;
    return 0;
}
