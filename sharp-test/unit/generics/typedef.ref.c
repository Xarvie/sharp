
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

#line 7 "sharp-test/unit/generics/typedef.sp"
typedef long isize;

#line 8 "sharp-test/unit/generics/typedef.sp"
typedef unsigned long usize;

#line 11 "sharp-test/unit/generics/typedef.sp"

#line 32 "sharp-test/unit/generics/typedef.sp"

#line 72 "sharp-test/unit/generics/typedef.sp"

#line 11 "sharp-test/unit/generics/typedef.sp"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int data;
    int n;
};



#line 32 "sharp-test/unit/generics/typedef.sp"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
    isize len;
    isize cap;
};


typedef struct Vec__float Vec__float;
struct Vec__float {
    float * data;
    isize len;
    isize cap;
};



#line 72 "sharp-test/unit/generics/typedef.sp"
typedef struct Arr__int Arr__int;
struct Arr__int {
    int * data;
    int size;
};



#line 67 "sharp-test/unit/generics/typedef.sp"
typedef Vec__int IntVec;

#line 68 "sharp-test/unit/generics/typedef.sp"
typedef Vec__float FloatVec;

#line 69 "sharp-test/unit/generics/typedef.sp"
typedef IntVec MyIntVec;

#line 16 "sharp-test/unit/generics/typedef.sp"
Stack__int Stack__int__new(void);

#line 22 "sharp-test/unit/generics/typedef.sp"
void Stack__int__push(Stack__int * this, int v);

#line 27 "sharp-test/unit/generics/typedef.sp"
int Stack__int__top(Stack__int * this);

#line 38 "sharp-test/unit/generics/typedef.sp"
void Vec__int__push(Vec__int * this, int val);

#line 50 "sharp-test/unit/generics/typedef.sp"
long Vec__int__size(Vec__int * this);

#line 54 "sharp-test/unit/generics/typedef.sp"
int Vec__int__get(Vec__int * this, long i);

#line 59 "sharp-test/unit/generics/typedef.sp"
void Vec__int__destroy(Vec__int * this);

#line 38 "sharp-test/unit/generics/typedef.sp"
void Vec__float__push(Vec__float * this, float val);

#line 50 "sharp-test/unit/generics/typedef.sp"
long Vec__float__size(Vec__float * this);

#line 54 "sharp-test/unit/generics/typedef.sp"
float Vec__float__get(Vec__float * this, long i);

#line 59 "sharp-test/unit/generics/typedef.sp"
void Vec__float__destroy(Vec__float * this);

#line 77 "sharp-test/unit/generics/typedef.sp"
int Arr__int__len(Arr__int * this);

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

#line 16 "sharp-test/unit/generics/typedef.sp"

#line 22 "sharp-test/unit/generics/typedef.sp"

#line 27 "sharp-test/unit/generics/typedef.sp"

#line 38 "sharp-test/unit/generics/typedef.sp"

#line 50 "sharp-test/unit/generics/typedef.sp"

#line 54 "sharp-test/unit/generics/typedef.sp"

#line 59 "sharp-test/unit/generics/typedef.sp"

#line 77 "sharp-test/unit/generics/typedef.sp"

#line 81 "sharp-test/unit/generics/typedef.sp"
int main() {

#line 83 "sharp-test/unit/generics/typedef.sp"
    Stack__int s = Stack__int__new();

#line 84 "sharp-test/unit/generics/typedef.sp"
    Stack__int__push(&s, 42);

#line 85 "sharp-test/unit/generics/typedef.sp"
    if (Stack__int__top(&s) != 42) 
        return 1;

#line 86 "sharp-test/unit/generics/typedef.sp"
    if (s.n != 1) 
        return 2;

#line 89 "sharp-test/unit/generics/typedef.sp"
    IntVec v = { 0 };

#line 90 "sharp-test/unit/generics/typedef.sp"
    Vec__int__push(&v, 42);

#line 91 "sharp-test/unit/generics/typedef.sp"
    Vec__int__push(&v, 73);

#line 92 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__size(&v) != 2) 
        return 3;

#line 93 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__get(&v, 0) != 42) 
        return 4;

#line 94 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__get(&v, 1) != 73) 
        return 5;

#line 95 "sharp-test/unit/generics/typedef.sp"
    Vec__int__destroy(&v);

#line 98 "sharp-test/unit/generics/typedef.sp"
    MyIntVec v2 = { 0 };

#line 99 "sharp-test/unit/generics/typedef.sp"
    Vec__int__push(&v2, 99);

#line 100 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__size(&v2) != 1) 
        return 6;

#line 101 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__get(&v2, 0) != 99) 
        return 7;

#line 102 "sharp-test/unit/generics/typedef.sp"
    Vec__int__destroy(&v2);

#line 105 "sharp-test/unit/generics/typedef.sp"
    FloatVec fv = { 0 };

#line 106 "sharp-test/unit/generics/typedef.sp"
    Vec__float__push(&fv, 1.5f);

#line 107 "sharp-test/unit/generics/typedef.sp"
    if (Vec__float__size(&fv) != 1) 
        return 8;

#line 108 "sharp-test/unit/generics/typedef.sp"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 9;

#line 109 "sharp-test/unit/generics/typedef.sp"
    Vec__float__destroy(&fv);

#line 112 "sharp-test/unit/generics/typedef.sp"
    IntVec z = { 0 };

#line 113 "sharp-test/unit/generics/typedef.sp"
    if (Vec__int__size(&z) != 0) 
        return 10;

#line 116 "sharp-test/unit/generics/typedef.sp"
    Arr__int arr = { 0 };

#line 117 "sharp-test/unit/generics/typedef.sp"
    arr.size = 5;

#line 118 "sharp-test/unit/generics/typedef.sp"
    if (Arr__int__len(&arr) != 5) 
        return 11;

#line 120 "sharp-test/unit/generics/typedef.sp"
    return 0;
}

#line 16 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) Stack__int Stack__int__new(void) {

#line 17 "sharp-test/unit/generics/typedef.sp"
    Stack__int s;

#line 18 "sharp-test/unit/generics/typedef.sp"
    s.n = 0;

#line 19 "sharp-test/unit/generics/typedef.sp"
    return s;
}


#line 22 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) void Stack__int__push(Stack__int * this, int v) {

#line 23 "sharp-test/unit/generics/typedef.sp"
    this->data = v;

#line 24 "sharp-test/unit/generics/typedef.sp"
    this->n = this->n + 1;
}


#line 27 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) int Stack__int__top(Stack__int * this) {

#line 28 "sharp-test/unit/generics/typedef.sp"
    return this->data;
}


#line 38 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 39 "sharp-test/unit/generics/typedef.sp"
    if (this->len >= this->cap) {

#line 40 "sharp-test/unit/generics/typedef.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "sharp-test/unit/generics/typedef.sp"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 42 "sharp-test/unit/generics/typedef.sp"
        if (!nd) 
            __builtin_trap();

#line 43 "sharp-test/unit/generics/typedef.sp"
        this->data = nd;

#line 44 "sharp-test/unit/generics/typedef.sp"
        this->cap = new_cap;
    }

#line 46 "sharp-test/unit/generics/typedef.sp"
    this->data[this->len] = val;

#line 47 "sharp-test/unit/generics/typedef.sp"
    this->len = this->len + 1;
}


#line 50 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 51 "sharp-test/unit/generics/typedef.sp"
    return this->len;
}


#line 54 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 55 "sharp-test/unit/generics/typedef.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "sharp-test/unit/generics/typedef.sp"
    return this->data[i];
}


#line 59 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 60 "sharp-test/unit/generics/typedef.sp"
    free(this->data);

#line 61 "sharp-test/unit/generics/typedef.sp"
    this->data = (int *)0;

#line 62 "sharp-test/unit/generics/typedef.sp"
    this->len = 0;

#line 63 "sharp-test/unit/generics/typedef.sp"
    this->cap = 0;
}


#line 38 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 39 "sharp-test/unit/generics/typedef.sp"
    if (this->len >= this->cap) {

#line 40 "sharp-test/unit/generics/typedef.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "sharp-test/unit/generics/typedef.sp"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 42 "sharp-test/unit/generics/typedef.sp"
        if (!nd) 
            __builtin_trap();

#line 43 "sharp-test/unit/generics/typedef.sp"
        this->data = nd;

#line 44 "sharp-test/unit/generics/typedef.sp"
        this->cap = new_cap;
    }

#line 46 "sharp-test/unit/generics/typedef.sp"
    this->data[this->len] = val;

#line 47 "sharp-test/unit/generics/typedef.sp"
    this->len = this->len + 1;
}


#line 50 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 51 "sharp-test/unit/generics/typedef.sp"
    return this->len;
}


#line 54 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 55 "sharp-test/unit/generics/typedef.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "sharp-test/unit/generics/typedef.sp"
    return this->data[i];
}


#line 59 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 60 "sharp-test/unit/generics/typedef.sp"
    free(this->data);

#line 61 "sharp-test/unit/generics/typedef.sp"
    this->data = (float *)0;

#line 62 "sharp-test/unit/generics/typedef.sp"
    this->len = 0;

#line 63 "sharp-test/unit/generics/typedef.sp"
    this->cap = 0;
}


#line 77 "sharp-test/unit/generics/typedef.sp"
__attribute__((weak)) int Arr__int__len(Arr__int * this) {

#line 78 "sharp-test/unit/generics/typedef.sp"
    return this->size;
}

