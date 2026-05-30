typedef float __v4sf __attribute__((__vector_size__(16)));
typedef float __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef int __v4si __attribute__((__vector_size__(16)));
typedef short __v8hi __attribute__((__vector_size__(16)));
typedef char __v16qi __attribute__((__vector_size__(16)));
typedef long long __v2di __attribute__((__vector_size__(16)));
typedef double __v2df __attribute__((__vector_size__(16)));
typedef unsigned char __v16qu __attribute__((__vector_size__(16)));
typedef unsigned short __v8hu __attribute__((__vector_size__(16)));
typedef unsigned int __v4su __attribute__((__vector_size__(16)));

__v4sf gvf;
__m128 gvm;
__v4si gvi;
__v2di gvd;
__v2df gvdf;

int test_vector_subscript(void) {
    return (int)gvf[0] + (int)gvm[1] + gvi[2];
}

__v4sf test_vector_arith(void) {
    __v4sf a = gvf;
    __v4sf b = gvf;
    __v4sf c = a + b;
    __v4sf d = a - b;
    __v4sf e = a * b;
    __v4sf f = a / b;
    __v4sf g = -a;
    (void)c; (void)d; (void)e; (void)f; (void)g;
    return a;
}

__v4sf test_vector_scalar_arith(void) {
    __v4sf a = gvf;
    __v4sf b = a + 1.0f;
    __v4sf c = 2.0f * a;
    (void)b; (void)c;
    return a;
}

int test_vector_assign(void) {
    __v4sf a = gvf;
    __v4sf b;
    b = a;
    return (int)b[0];
}

__v4sf test_vector_cast(void) {
    __v4si vi = gvi;
    __v4sf vf = (__v4sf)vi;
    return vf;
}

__v4sf test_vector_compound_lit(void) {
    __v4sf v = (__v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    return v;
}

int test_local_vector_typedef(void) {
    typedef float local_v2f __attribute__((__vector_size__(8)));
    local_v2f x;
    x[0] = 1.0f;
    x[1] = 2.0f;
    return (int)x[0] + (int)x[1];
}

int test_vector_bitwise(void) {
    __v4si a = gvi;
    __v4si b = ~a;
    __v4si c = a & b;
    __v4si d = a | b;
    __v4si e = a ^ b;
    (void)c; (void)d; (void)e;
    return (int)b[0];
}

__v4sf test_vector_ternary(int cond) {
    __v4sf a = gvf;
    __v4sf b = gvf;
    return cond ? a : b;
}

int test_vector_comparison(void) {
    __v4sf a = gvf;
    if (a[0] > 0.0f) return 1;
    return 0;
}

typedef short __v4hf __attribute__((__vector_size__(8)));

int test_small_vector(void) {
    __v4hf v;
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    v[3] = 4;
    return (int)v[0] + (int)v[3];
}

typedef long long __v1di __attribute__((__vector_size__(8)));

int test_single_elem_vector(void) {
    __v1di v;
    v[0] = 42;
    return (int)v[0];
}

typedef char __v32qi __attribute__((__vector_size__(32)));

int test_avx_vector(void) {
    __v32qi v;
    v[0] = 1;
    v[31] = 2;
    return (int)v[0] + (int)v[31];
}

__v4sf test_builtin_convertvector(void) {
    __v4si vi = gvi;
    return __builtin_convertvector(vi, __v4sf);
}

__v4sf test_builtin_shufflevector(void) {
    __v4sf a = gvf;
    return __builtin_shufflevector(a, a, 3, 2, 1, 0);
}

int test_vector_in_struct(void) {
    struct { __v4sf v; int tag; } s;
    s.v = gvf;
    s.tag = 1;
    return s.tag + (int)s.v[0];
}

typedef float __v2sf __attribute__((__vector_size__(8)));

int test_vector_field_assign(void) {
    __v2sf lo;
    __v2sf hi;
    lo[0] = 1.0f;
    lo[1] = 2.0f;
    hi[0] = 3.0f;
    hi[1] = 4.0f;
    return (int)lo[0] + (int)hi[1];
}

int test_vector_param(__v4sf v) {
    return (int)v[0];
}

__v4sf test_vector_return(void) {
    return gvf;
}

int main(void) {
    int r = 0;
    r += test_vector_subscript();
    r += test_vector_arith()[0] != 0.0f ? 1 : 0;
    r += test_vector_scalar_arith()[0] != 0.0f ? 1 : 0;
    r += test_vector_assign();
    r += test_vector_cast()[0] != 0.0f ? 1 : 0;
    r += test_vector_compound_lit()[0] != 0.0f ? 1 : 0;
    r += test_local_vector_typedef();
    r += test_vector_bitwise();
    r += test_vector_ternary(1)[0] != 0.0f ? 1 : 0;
    r += test_vector_comparison();
    r += test_small_vector();
    r += test_single_elem_vector();
    r += test_avx_vector();
    r += test_builtin_convertvector()[0] != 0.0f ? 1 : 0;
    r += test_builtin_shufflevector()[0] != 0.0f ? 1 : 0;
    r += test_vector_in_struct();
    r += test_vector_field_assign();
    r += test_vector_param(gvf);
    r += test_vector_return()[0] != 0.0f ? 1 : 0;
    return r;
}
