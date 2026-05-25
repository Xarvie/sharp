/* p85_int128.c
 * Phase R11: __int128, unsigned __int128, signed __int128 as types.
 * GCC/Clang built-in 128-bit integer — mapped to long long (best approx).
 * Also tests __signed__ __int128 which appears in linux/types.h. */
typedef __signed__ __int128          i128;
typedef unsigned __int128            u128;

static long long square(long long x) { return x * x; }

int main(void) {
    i128 a = 7;
    u128 b = 3;
    /* Cast expressions */
    long long c = (long long)((i128)(a + b) * 2);
    if (c != 20) return 1;
    /* Standalone unsigned __int128 in sizeof */
    if (sizeof(u128) < 4) return 2;
    (void)square((long long)a);
    return 0;
}
