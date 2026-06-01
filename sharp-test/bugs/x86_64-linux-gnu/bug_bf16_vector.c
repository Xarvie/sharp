typedef __bf16 __m128bh __attribute__((vector_size(16)));
typedef __bf16 __m256bh __attribute__((vector_size(32)));

static __inline __m256bh test_setzero_pbh(void) {
    return (__m256bh){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
}

int main(void) {
    __m256bh v = test_setzero_pbh();
    __m128bh u = {0};
    (void)v;
    (void)u;
    return 0;
}
