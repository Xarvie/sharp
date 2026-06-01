typedef float __v4sf __attribute__((vector_size(16)));
typedef float __v8sf __attribute__((vector_size(32)));
typedef double __v2df __attribute__((vector_size(16)));
typedef double __v4df __attribute__((vector_size(32)));
typedef long long __v2di __attribute__((vector_size(16)));
typedef long long __v4di __attribute__((vector_size(32)));
typedef unsigned char __v16qi __attribute__((vector_size(16)));
typedef unsigned char __v32qi __attribute__((vector_size(32)));

__v4sf test_v4sf(void) { return (__v4sf){1.0f,2.0f,3.0f,4.0f}; }
__v8sf test_v8sf(void) { return (__v8sf){1,2,3,4,5,6,7,8}; }
__v2df test_v2df(void) { return (__v2df){1.0,2.0}; }
__v4df test_v4df(void) { return (__v4df){1,2,3,4}; }
__v2di test_v2di(void) { return (__v2di){1LL,2LL}; }
__v4di test_v4di(void) { return (__v4di){1LL,2LL,3LL,4LL}; }

int main(void) {
    __v4sf a = test_v4sf();
    __v8sf b = test_v8sf();
    __v2df c = test_v2df();
    __v4df d = test_v4df();
    __v2di e = test_v2di();
    __v4di f = test_v4di();
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;
    return 0;
}
