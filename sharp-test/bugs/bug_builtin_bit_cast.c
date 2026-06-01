typedef float __m128 __attribute__((vector_size(16)));

static __inline __m128 test_bitcast(float f) {
    return __builtin_bit_cast(__m128, (float[4]){f,f,f,f});
}

int main(void) {
    __m128 r = test_bitcast(1.0f);
    (void)r;
    return 0;
}
