typedef float __m128 __attribute__((vector_size(16)));

static __inline__ __m128 test_bitcast_float(float f) {
    return __builtin_bit_cast(__m128, (float[4]){f,f,f,f});
}

static __inline__ int test_bitcast_int(__m128 v) {
    return __builtin_bit_cast(int, v[0]);
}

int main(void) {
    __m128 r = test_bitcast_float(1.0f);
    int i = test_bitcast_int(r);
    (void)r; (void)i;
    return 0;
}
