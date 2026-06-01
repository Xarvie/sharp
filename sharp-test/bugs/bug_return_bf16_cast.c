typedef __bf16 __v8bf __attribute__((vector_size(16)));

static __inline__ __bf16 test_cvtness(float __A) {
    __v8bf __R = {0, 0, 0, 0, 0, 0, 0, 0};
    return (__bf16)__R[0];
}

int main(void) {
    __bf16 v = test_cvtness(1.0f);
    (void)v;
    return 0;
}
