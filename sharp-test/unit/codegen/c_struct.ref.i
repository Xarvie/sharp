
#line 18 "/root/code/sharp/zig/lib/include/__stddef_size_t.h"
typedef unsigned long int size_t;

#line 24 "/root/code/sharp/zig/lib/include/__stddef_wchar_t.h"
typedef int wchar_t;

#line 83 "/root/code/sharp/zig/lib/libc/include/x86-linux-gnu/bits/floatn.h"
typedef _Complex float __cfloat128 __attribute__((__mode__(__TC__)));

#line 97 "/root/code/sharp/zig/lib/libc/include/x86-linux-gnu/bits/floatn.h"
typedef __float128 _Float128;

#line 214 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/floatn-common.h"
typedef float _Float32;

#line 251 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/floatn-common.h"
typedef double _Float64;

#line 268 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/floatn-common.h"
typedef double _Float32x;

#line 285 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/floatn-common.h"
typedef long double _Float64x;

#line 63 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"

#line 67 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
typedef struct {
    int quot;
    int rem;
} div_t;

#line 71 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"

#line 75 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
typedef struct {
    long int quot;
    long int rem;
} ldiv_t;

#line 81 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"

#line 85 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
__extension__ typedef struct {
    long long int quot;
    long long int rem;
} lldiv_t;

#line 102 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t __ctype_get_mb_cur_max(void) __attribute__((__nothrow__));

#line 106 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern double atof(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 109 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int atoi(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 112 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int atol(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 117 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int atoll(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 122 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern double strtod(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 128 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern float strtof(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 131 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long double strtold(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 181 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int strtol(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 185 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern unsigned long int strtoul(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 205 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int strtoll(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 210 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern unsigned long long int strtoull(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 577 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int rand(void) __attribute__((__nothrow__));

#line 579 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void srand(unsigned int __seed) __attribute__((__nothrow__));

#line 682 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * malloc(size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__));

#line 685 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * calloc(size_t __nmemb, size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__));

#line 693 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * realloc(void * __ptr, size_t __size) __attribute__((__nothrow__)) __attribute__((__warn_unused_result__));

#line 697 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free(void * __ptr) __attribute__((__nothrow__));

#line 758 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * aligned_alloc(size_t __alignment, size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)));

#line 764 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void abort(void) __attribute__((__nothrow__)) __attribute__((__noreturn__)) __attribute__((__cold__));

#line 768 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int atexit(void (*__func)(void)) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 776 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int at_quick_exit(void (*__func)(void)) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 790 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 796 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void quick_exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 802 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void _Exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 807 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern char * getenv(const char * __name) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 957 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int system(const char * __command);

#line 982 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
typedef int (*__compar_fn_t)(const void *, const void *);

#line 994 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * bsearch(const void * __key, const void * __base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1,2,5)));

#line 1010 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void qsort(void * __base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1,4)));

#line 1020 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int abs(int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1021 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int labs(long int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1024 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int llabs(long long int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1038 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern div_t div(int __numer, int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1040 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern ldiv_t ldiv(long int __numer, long int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1044 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern lldiv_t lldiv(long long int __numer, long long int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1108 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int mblen(const char * __s, size_t __n) __attribute__((__nothrow__));

#line 1111 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int mbtowc(wchar_t * __restrict __pwc, const char * __restrict __s, size_t __n) __attribute__((__nothrow__));

#line 1115 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int wctomb(char * __s, wchar_t __wchar) __attribute__((__nothrow__));

#line 1119 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t mbstowcs(wchar_t * __restrict __pwcs, const char * __restrict __s, size_t __n) __attribute__((__nothrow__));

#line 1123 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t wcstombs(char * __restrict __s, const wchar_t * __restrict __pwcs, size_t __n) __attribute__((__nothrow__));

#line 7 "sharp-test/unit/codegen/c_struct.ce"
typedef long isize;

#line 10 "sharp-test/unit/codegen/c_struct.ce"
struct CIface {
    void * userdata;
    int refcount;
    struct CIface * next;
};

#line 29 "sharp-test/unit/codegen/c_struct.ce"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 33 "sharp-test/unit/codegen/c_struct.ce"
isize Str__size(Str * this);

#line 56 "sharp-test/unit/codegen/c_struct.ce"
typedef struct Vec_int Vec_int;
struct Vec_int {
    int * data;
    isize len;
    isize cap;
};

#line 61 "sharp-test/unit/codegen/c_struct.ce"
void Vec_int__push(Vec_int * this, int val);

#line 70 "sharp-test/unit/codegen/c_struct.ce"
int Vec_int__get(Vec_int * this, isize i);

#line 71 "sharp-test/unit/codegen/c_struct.ce"
isize Vec_int__size(Vec_int * this);

#line 72 "sharp-test/unit/codegen/c_struct.ce"
void Vec_int__destroy(Vec_int * this);

#line 102 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t __ctype_get_mb_cur_max(void) __attribute__((__nothrow__));

#line 106 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern double atof(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 109 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int atoi(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 112 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int atol(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 117 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int atoll(const char * __nptr) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 122 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern double strtod(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 128 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern float strtof(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 131 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long double strtold(const char * __restrict __nptr, char * * __restrict __endptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 181 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int strtol(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 185 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern unsigned long int strtoul(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 205 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int strtoll(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 210 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern unsigned long long int strtoull(const char * __restrict __nptr, char * * __restrict __endptr, int __base) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 577 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int rand(void) __attribute__((__nothrow__));

#line 579 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void srand(unsigned int __seed) __attribute__((__nothrow__));

#line 682 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * malloc(size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__));

#line 685 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * calloc(size_t __nmemb, size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__));

#line 693 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * realloc(void * __ptr, size_t __size) __attribute__((__nothrow__)) __attribute__((__warn_unused_result__));

#line 697 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free(void * __ptr) __attribute__((__nothrow__));

#line 758 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * aligned_alloc(size_t __alignment, size_t __size) __attribute__((__nothrow__)) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)));

#line 764 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void abort(void) __attribute__((__nothrow__)) __attribute__((__noreturn__)) __attribute__((__cold__));

#line 768 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int atexit(void (*__func)(void)) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 776 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int at_quick_exit(void (*__func)(void)) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 790 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 796 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void quick_exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 802 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void _Exit(int __status) __attribute__((__nothrow__)) __attribute__((__noreturn__));

#line 807 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern char * getenv(const char * __name) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 957 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int system(const char * __command);

#line 994 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void * bsearch(const void * __key, const void * __base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1,2,5)));

#line 1010 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void qsort(void * __base, size_t __nmemb, size_t __size, __compar_fn_t __compar) __attribute__((__nonnull__(1,4)));

#line 1020 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int abs(int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1021 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long int labs(long int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1024 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern long long int llabs(long long int __x) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1038 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern div_t div(int __numer, int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1040 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern ldiv_t ldiv(long int __numer, long int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1044 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern lldiv_t lldiv(long long int __numer, long long int __denom) __attribute__((__nothrow__)) __attribute__((__const__));

#line 1108 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int mblen(const char * __s, size_t __n) __attribute__((__nothrow__));

#line 1111 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int mbtowc(wchar_t * __restrict __pwc, const char * __restrict __s, size_t __n) __attribute__((__nothrow__));

#line 1115 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int wctomb(char * __s, wchar_t __wchar) __attribute__((__nothrow__));

#line 1119 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t mbstowcs(wchar_t * __restrict __pwcs, const char * __restrict __s, size_t __n) __attribute__((__nothrow__));

#line 1123 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t wcstombs(char * __restrict __s, const wchar_t * __restrict __pwcs, size_t __n) __attribute__((__nothrow__));

#line 16 "sharp-test/unit/codegen/c_struct.ce"
void reset_refcount(struct CIface * c) {

#line 17 "sharp-test/unit/codegen/c_struct.ce"
    c->refcount = 0;

#line 18 "sharp-test/unit/codegen/c_struct.ce"
    c->next = (struct CIface *)0;
}

#line 20 "sharp-test/unit/codegen/c_struct.ce"
struct CIface * create_c_iface(void) {

#line 21 "sharp-test/unit/codegen/c_struct.ce"
    struct CIface * p = (struct CIface *)malloc(sizeof(struct CIface));

#line 22 "sharp-test/unit/codegen/c_struct.ce"
    p->userdata = 0;

#line 23 "sharp-test/unit/codegen/c_struct.ce"
    p->refcount = 0;

#line 24 "sharp-test/unit/codegen/c_struct.ce"
    p->next = 0;

#line 25 "sharp-test/unit/codegen/c_struct.ce"
    return p;
}

#line 33 "sharp-test/unit/codegen/c_struct.ce"
isize Str__size(Str * this) {
    return this->len;
}

#line 34 "sharp-test/unit/codegen/c_struct.ce"
struct Str str_from_lit(const char * lit) {

#line 35 "sharp-test/unit/codegen/c_struct.ce"
    struct Str s;

#line 36 "sharp-test/unit/codegen/c_struct.ce"
    s.ptr = lit;

#line 37 "sharp-test/unit/codegen/c_struct.ce"
    s.len = 0;

#line 38 "sharp-test/unit/codegen/c_struct.ce"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 39 "sharp-test/unit/codegen/c_struct.ce"
    return s;
}

#line 41 "sharp-test/unit/codegen/c_struct.ce"
struct Str str_from_parts(const char * p, isize len) {

#line 42 "sharp-test/unit/codegen/c_struct.ce"
    struct Str s;

#line 43 "sharp-test/unit/codegen/c_struct.ce"
    s.ptr = p;

#line 44 "sharp-test/unit/codegen/c_struct.ce"
    s.len = len;

#line 45 "sharp-test/unit/codegen/c_struct.ce"
    return s;
}

#line 47 "sharp-test/unit/codegen/c_struct.ce"
const char * to_cstr_manual(struct Str s) {

#line 48 "sharp-test/unit/codegen/c_struct.ce"
    static char buf[4096];

#line 49 "sharp-test/unit/codegen/c_struct.ce"
    isize i = 0;

#line 50 "sharp-test/unit/codegen/c_struct.ce"
    while (i < Str__size(&s)) {
        buf[i] = s.ptr[i];
        i = i + 1;
    }

#line 51 "sharp-test/unit/codegen/c_struct.ce"
    buf[i] = '\0';

#line 52 "sharp-test/unit/codegen/c_struct.ce"
    return buf;
}

#line 61 "sharp-test/unit/codegen/c_struct.ce"
void Vec_int__push(Vec_int * this, int val) {

#line 62 "sharp-test/unit/codegen/c_struct.ce"
    if (this->len >= this->cap) {

#line 63 "sharp-test/unit/codegen/c_struct.ce"
        isize nc = (this->cap == 0) ? 8 : this->cap * 2;

#line 64 "sharp-test/unit/codegen/c_struct.ce"
        this->data = (int *)realloc(this->data, nc * sizeof(int));

#line 65 "sharp-test/unit/codegen/c_struct.ce"
        this->cap = nc;
    }

#line 67 "sharp-test/unit/codegen/c_struct.ce"
    this->data[this->len] = val;

#line 68 "sharp-test/unit/codegen/c_struct.ce"
    this->len = this->len + 1;
}

#line 70 "sharp-test/unit/codegen/c_struct.ce"
int Vec_int__get(Vec_int * this, isize i) {
    return this->data[i];
}

#line 71 "sharp-test/unit/codegen/c_struct.ce"
isize Vec_int__size(Vec_int * this) {
    return this->len;
}

#line 72 "sharp-test/unit/codegen/c_struct.ce"
void Vec_int__destroy(Vec_int * this) {
    free(this->data);
}

#line 74 "sharp-test/unit/codegen/c_struct.ce"
int main(void) {

#line 76 "sharp-test/unit/codegen/c_struct.ce"
    struct CIface * c = create_c_iface();

#line 77 "sharp-test/unit/codegen/c_struct.ce"
    reset_refcount(c);

#line 78 "sharp-test/unit/codegen/c_struct.ce"
    free(c);

#line 81 "sharp-test/unit/codegen/c_struct.ce"
    struct Str hello = str_from_lit("Hello");

#line 82 "sharp-test/unit/codegen/c_struct.ce"
    if (Str__size(&hello) != 5) 
        return 1;

#line 83 "sharp-test/unit/codegen/c_struct.ce"
    const char * cstr = to_cstr_manual(hello);

#line 84 "sharp-test/unit/codegen/c_struct.ce"
    if (cstr[5] != '\0') 
        return 2;

#line 85 "sharp-test/unit/codegen/c_struct.ce"
    if (cstr[0] != 'H') 
        return 3;

#line 86 "sharp-test/unit/codegen/c_struct.ce"
    struct Str empty = str_from_parts(hello.ptr, 0);

#line 87 "sharp-test/unit/codegen/c_struct.ce"
    if (Str__size(&empty) != 0) 
        return 4;

#line 90 "sharp-test/unit/codegen/c_struct.ce"
    Vec_int vi;

#line 91 "sharp-test/unit/codegen/c_struct.ce"
    vi.data = ((void *)0);
    vi.len = 0;
    vi.cap = 0;

#line 92 "sharp-test/unit/codegen/c_struct.ce"
    Vec_int__push(&vi, 42);

#line 93 "sharp-test/unit/codegen/c_struct.ce"
    Vec_int__push(&vi, 73);

#line 94 "sharp-test/unit/codegen/c_struct.ce"
    if (Vec_int__size(&vi) != 2) 
        return 5;

#line 95 "sharp-test/unit/codegen/c_struct.ce"
    if (Vec_int__get(&vi, 0) != 42) 
        return 6;

#line 96 "sharp-test/unit/codegen/c_struct.ce"
    if (Vec_int__get(&vi, 1) != 73) 
        return 7;

#line 97 "sharp-test/unit/codegen/c_struct.ce"
    Vec_int__destroy(&vi);

#line 100 "sharp-test/unit/codegen/c_struct.ce"
    struct CIface head = { 0 };

#line 101 "sharp-test/unit/codegen/c_struct.ce"
    struct CIface second = { 0 };

#line 102 "sharp-test/unit/codegen/c_struct.ce"
    head.userdata = (void *)0;

#line 103 "sharp-test/unit/codegen/c_struct.ce"
    head.refcount = 1;

#line 104 "sharp-test/unit/codegen/c_struct.ce"
    head.next = &second;

#line 105 "sharp-test/unit/codegen/c_struct.ce"
    second.userdata = (void *)0;

#line 106 "sharp-test/unit/codegen/c_struct.ce"
    second.refcount = 2;

#line 107 "sharp-test/unit/codegen/c_struct.ce"
    second.next = (struct CIface *)0;

#line 108 "sharp-test/unit/codegen/c_struct.ce"
    if (head.next->refcount != 2) 
        return 8;

#line 109 "sharp-test/unit/codegen/c_struct.ce"
    if (head.next->next != (struct CIface *)0) 
        return 9;

#line 111 "sharp-test/unit/codegen/c_struct.ce"
    return 0;
}
