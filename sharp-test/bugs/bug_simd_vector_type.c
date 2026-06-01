typedef float __m128 __attribute__((vector_size(16)));
typedef float __m256 __attribute__((vector_size(32)));
typedef double __m128d __attribute__((vector_size(16)));
typedef double __m256d __attribute__((vector_size(32)));
typedef long long __m128i __attribute__((vector_size(16)));
typedef long long __m256i __attribute__((vector_size(32)));

__m128 test_m128(void) {
    __m128 a = {1.0f, 2.0f, 3.0f, 4.0f};
    __m128 b = {5.0f, 6.0f, 7.0f, 8.0f};
    return a + b;
}

__m256 test_m256(void) {
    __m256 a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    return a;
}

int main(void) {
    __m128 r = test_m128();
    (void)r;
    return 0;
}
