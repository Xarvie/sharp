typedef __bf16 __m128bh __attribute__((vector_size(16)));
typedef __bf16 __m256bh __attribute__((vector_size(32)));
typedef __bf16 __m512bh __attribute__((vector_size(64)));
typedef float __m128 __attribute__((vector_size(16)));
typedef float __m256 __attribute__((vector_size(32)));
typedef double __m128d __attribute__((vector_size(16)));
typedef double __m256d __attribute__((vector_size(32)));
typedef long long __m128i __attribute__((vector_size(16)));
typedef long long __m256i __attribute__((vector_size(32)));

__m256bh test_bf16_setzero(void) {
    return (__m256bh){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
}

__m128 test_float_cast(__m128 a) {
    return a + a;
}

int main(void) {
    __m256bh v = test_bf16_setzero();
    __m128bh u = {0};
    __m128  f = test_float_cast(f);
    __bf16  s = {0};
    (void)v; (void)u; (void)f; (void)s;
    return 0;
}
