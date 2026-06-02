
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

#line 282 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfromd(char * __dest, size_t __size, const char * __format, double __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

#line 286 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfromf(char * __dest, size_t __size, const char * __format, float __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

#line 290 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfroml(char * __dest, size_t __size, const char * __format, long double __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

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

#line 708 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free_sized(void * __ptr, size_t __size) __attribute__((__nothrow__));

#line 713 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free_aligned_sized(void * __ptr, size_t __alignment, size_t __size) __attribute__((__nothrow__));

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

#line 28 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/atomic_wide_counter.h"

#line 25 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/atomic_wide_counter.h"

#line 33 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/atomic_wide_counter.h"
typedef union {
    unsigned long long int __value64;
    struct {
    unsigned int __low;
    unsigned int __high;
} __value32;
} __atomic_wide_counter;

#line 51 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"

#line 55 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
typedef struct __pthread_internal_list {
    struct __pthread_internal_list * __prev;
    struct __pthread_internal_list * __next;
} __pthread_list_t;

#line 57 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"

#line 60 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
typedef struct __pthread_internal_slist {
    struct __pthread_internal_slist * __next;
} __pthread_slist_t;

#line 22 "/root/code/sharp/zig/lib/libc/include/x86-linux-gnu/bits/struct_mutex.h"
struct __pthread_mutex_s {
    int __lock;
    unsigned int __count;
    int __owner;
    unsigned int __nusers;
    int __kind;
    short __spins;
    short __unused;
    __pthread_list_t __list;
};

#line 23 "/root/code/sharp/zig/lib/libc/include/x86-linux-gnu/bits/struct_rwlock.h"
struct __pthread_rwlock_arch_t {
    unsigned int __readers;
    unsigned int __writers;
    unsigned int __wrphase_futex;
    unsigned int __writers_futex;
    unsigned int __pad3;
    unsigned int __pad4;
    int __cur_writer;
    int __shared;
    unsigned long int __pad1;
    unsigned long int __pad2;
    unsigned int __flags;
};

#line 94 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
struct __pthread_cond_s {
    __atomic_wide_counter __wseq;
    __atomic_wide_counter __g1_start;
    unsigned int __g_size[2];
    unsigned int __g1_orig_size;
    unsigned int __wrefs;
    unsigned int __g_signals[2];
    unsigned int __unused_initialized_1;
    unsigned int __unused_initialized_2;
};

#line 106 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
typedef unsigned int __tss_t;

#line 107 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
typedef unsigned long int __thrd_t;

#line 109 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"

#line 112 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/thread-shared-types.h"
typedef struct {
    int __data;
} __once_flag;

#line 24 "/root/code/sharp/zig/lib/libc/include/generic-glibc/bits/types/once_flag.h"
typedef __once_flag once_flag;

#line 1207 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void call_once(once_flag * __flag, void (*__func)(void));

#line 1211 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t memalignment(const void * __p);

#line 7 "sharp-test/unit/generics/typedef.ce"
typedef long isize;

#line 8 "sharp-test/unit/generics/typedef.ce"
typedef unsigned long usize;

#line 11 "sharp-test/unit/generics/typedef.ce"

#line 32 "sharp-test/unit/generics/typedef.ce"

#line 72 "sharp-test/unit/generics/typedef.ce"

#line 32 "sharp-test/unit/generics/typedef.ce"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
    isize len;
    isize cap;
};



#line 11 "sharp-test/unit/generics/typedef.ce"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int data;
    int n;
};



#line 32 "sharp-test/unit/generics/typedef.ce"
typedef struct Vec__float Vec__float;
struct Vec__float {
    float * data;
    isize len;
    isize cap;
};



#line 72 "sharp-test/unit/generics/typedef.ce"
typedef struct Arr__int Arr__int;
struct Arr__int {
    int * data;
    int size;
};



#line 67 "sharp-test/unit/generics/typedef.ce"
typedef Vec__int IntVec;

#line 68 "sharp-test/unit/generics/typedef.ce"
typedef Vec__float FloatVec;

#line 69 "sharp-test/unit/generics/typedef.ce"
typedef IntVec MyIntVec;

#line 38 "sharp-test/unit/generics/typedef.ce"
void Vec__int__push(Vec__int * this, int val);

#line 16 "sharp-test/unit/generics/typedef.ce"
Stack__int Stack__int__new(void);

#line 22 "sharp-test/unit/generics/typedef.ce"
void Stack__int__push(Stack__int * this, int v);

#line 27 "sharp-test/unit/generics/typedef.ce"
int Stack__int__top(Stack__int * this);

#line 50 "sharp-test/unit/generics/typedef.ce"
long Vec__int__size(Vec__int * this);

#line 54 "sharp-test/unit/generics/typedef.ce"
int Vec__int__get(Vec__int * this, long i);

#line 59 "sharp-test/unit/generics/typedef.ce"
void Vec__int__destroy(Vec__int * this);

#line 38 "sharp-test/unit/generics/typedef.ce"
void Vec__float__push(Vec__float * this, float val);

#line 50 "sharp-test/unit/generics/typedef.ce"
long Vec__float__size(Vec__float * this);

#line 54 "sharp-test/unit/generics/typedef.ce"
float Vec__float__get(Vec__float * this, long i);

#line 59 "sharp-test/unit/generics/typedef.ce"
void Vec__float__destroy(Vec__float * this);

#line 77 "sharp-test/unit/generics/typedef.ce"
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

#line 282 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfromd(char * __dest, size_t __size, const char * __format, double __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

#line 286 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfromf(char * __dest, size_t __size, const char * __format, float __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

#line 290 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern int strfroml(char * __dest, size_t __size, const char * __format, long double __f) __attribute__((__nothrow__)) __attribute__((__nonnull__(3)));

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

#line 708 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free_sized(void * __ptr, size_t __size) __attribute__((__nothrow__));

#line 713 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void free_aligned_sized(void * __ptr, size_t __alignment, size_t __size) __attribute__((__nothrow__));

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

#line 1207 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern void call_once(once_flag * __flag, void (*__func)(void));

#line 1211 "/root/code/sharp/zig/lib/libc/include/generic-glibc/stdlib.h"
extern size_t memalignment(const void * __p);

#line 16 "sharp-test/unit/generics/typedef.ce"

#line 22 "sharp-test/unit/generics/typedef.ce"

#line 27 "sharp-test/unit/generics/typedef.ce"

#line 38 "sharp-test/unit/generics/typedef.ce"

#line 50 "sharp-test/unit/generics/typedef.ce"

#line 54 "sharp-test/unit/generics/typedef.ce"

#line 59 "sharp-test/unit/generics/typedef.ce"

#line 77 "sharp-test/unit/generics/typedef.ce"

#line 81 "sharp-test/unit/generics/typedef.ce"
IntVec make_vec(int a, int b) {

#line 82 "sharp-test/unit/generics/typedef.ce"
    IntVec v = { 0 };

#line 83 "sharp-test/unit/generics/typedef.ce"
    Vec__int__push(&v, a);

#line 84 "sharp-test/unit/generics/typedef.ce"
    Vec__int__push(&v, b);

#line 85 "sharp-test/unit/generics/typedef.ce"
    return v;
}

#line 88 "sharp-test/unit/generics/typedef.ce"
int main() {

#line 90 "sharp-test/unit/generics/typedef.ce"
    Stack__int s = Stack__int__new();

#line 91 "sharp-test/unit/generics/typedef.ce"
    Stack__int__push(&s, 42);

#line 92 "sharp-test/unit/generics/typedef.ce"
    if (Stack__int__top(&s) != 42) 
        return 1;

#line 93 "sharp-test/unit/generics/typedef.ce"
    if (s.n != 1) 
        return 2;

#line 96 "sharp-test/unit/generics/typedef.ce"
    IntVec v = { 0 };

#line 97 "sharp-test/unit/generics/typedef.ce"
    Vec__int__push(&v, 42);

#line 98 "sharp-test/unit/generics/typedef.ce"
    Vec__int__push(&v, 73);

#line 99 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__size(&v) != 2) 
        return 3;

#line 100 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__get(&v, 0) != 42) 
        return 4;

#line 101 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__get(&v, 1) != 73) 
        return 5;

#line 102 "sharp-test/unit/generics/typedef.ce"
    Vec__int__destroy(&v);

#line 105 "sharp-test/unit/generics/typedef.ce"
    MyIntVec v2 = { 0 };

#line 106 "sharp-test/unit/generics/typedef.ce"
    Vec__int__push(&v2, 99);

#line 107 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__size(&v2) != 1) 
        return 6;

#line 108 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__get(&v2, 0) != 99) 
        return 7;

#line 109 "sharp-test/unit/generics/typedef.ce"
    Vec__int__destroy(&v2);

#line 112 "sharp-test/unit/generics/typedef.ce"
    FloatVec fv = { 0 };

#line 113 "sharp-test/unit/generics/typedef.ce"
    Vec__float__push(&fv, 1.5f);

#line 114 "sharp-test/unit/generics/typedef.ce"
    if (Vec__float__size(&fv) != 1) 
        return 8;

#line 115 "sharp-test/unit/generics/typedef.ce"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 9;

#line 116 "sharp-test/unit/generics/typedef.ce"
    Vec__float__destroy(&fv);

#line 119 "sharp-test/unit/generics/typedef.ce"
    IntVec z = { 0 };

#line 120 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__size(&z) != 0) 
        return 10;

#line 123 "sharp-test/unit/generics/typedef.ce"
    Arr__int arr = { 0 };

#line 124 "sharp-test/unit/generics/typedef.ce"
    arr.size = 5;

#line 125 "sharp-test/unit/generics/typedef.ce"
    if (Arr__int__len(&arr) != 5) 
        return 11;

#line 128 "sharp-test/unit/generics/typedef.ce"
    IntVec mv = make_vec(10, 20);

#line 129 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__size(&mv) != 2) 
        return 12;

#line 130 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__get(&mv, 0) != 10) 
        return 13;

#line 131 "sharp-test/unit/generics/typedef.ce"
    if (Vec__int__get(&mv, 1) != 20) 
        return 14;

#line 132 "sharp-test/unit/generics/typedef.ce"
    Vec__int__destroy(&mv);

#line 134 "sharp-test/unit/generics/typedef.ce"
    return 0;
}

#line 38 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 39 "sharp-test/unit/generics/typedef.ce"
    if (this->len >= this->cap) {

#line 40 "sharp-test/unit/generics/typedef.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "sharp-test/unit/generics/typedef.ce"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 42 "sharp-test/unit/generics/typedef.ce"
        if (!nd) 
            __builtin_trap();

#line 43 "sharp-test/unit/generics/typedef.ce"
        this->data = nd;

#line 44 "sharp-test/unit/generics/typedef.ce"
        this->cap = new_cap;
    }

#line 46 "sharp-test/unit/generics/typedef.ce"
    this->data[this->len] = val;

#line 47 "sharp-test/unit/generics/typedef.ce"
    this->len = this->len + 1;
}


#line 16 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) Stack__int Stack__int__new(void) {

#line 17 "sharp-test/unit/generics/typedef.ce"
    Stack__int s;

#line 18 "sharp-test/unit/generics/typedef.ce"
    s.n = 0;

#line 19 "sharp-test/unit/generics/typedef.ce"
    return s;
}


#line 22 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) void Stack__int__push(Stack__int * this, int v) {

#line 23 "sharp-test/unit/generics/typedef.ce"
    this->data = v;

#line 24 "sharp-test/unit/generics/typedef.ce"
    this->n = this->n + 1;
}


#line 27 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) int Stack__int__top(Stack__int * this) {

#line 28 "sharp-test/unit/generics/typedef.ce"
    return this->data;
}


#line 50 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 51 "sharp-test/unit/generics/typedef.ce"
    return this->len;
}


#line 54 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 55 "sharp-test/unit/generics/typedef.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "sharp-test/unit/generics/typedef.ce"
    return this->data[i];
}


#line 59 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 60 "sharp-test/unit/generics/typedef.ce"
    free(this->data);

#line 61 "sharp-test/unit/generics/typedef.ce"
    this->data = (int *)0;

#line 62 "sharp-test/unit/generics/typedef.ce"
    this->len = 0;

#line 63 "sharp-test/unit/generics/typedef.ce"
    this->cap = 0;
}


#line 38 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 39 "sharp-test/unit/generics/typedef.ce"
    if (this->len >= this->cap) {

#line 40 "sharp-test/unit/generics/typedef.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "sharp-test/unit/generics/typedef.ce"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 42 "sharp-test/unit/generics/typedef.ce"
        if (!nd) 
            __builtin_trap();

#line 43 "sharp-test/unit/generics/typedef.ce"
        this->data = nd;

#line 44 "sharp-test/unit/generics/typedef.ce"
        this->cap = new_cap;
    }

#line 46 "sharp-test/unit/generics/typedef.ce"
    this->data[this->len] = val;

#line 47 "sharp-test/unit/generics/typedef.ce"
    this->len = this->len + 1;
}


#line 50 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 51 "sharp-test/unit/generics/typedef.ce"
    return this->len;
}


#line 54 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 55 "sharp-test/unit/generics/typedef.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "sharp-test/unit/generics/typedef.ce"
    return this->data[i];
}


#line 59 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 60 "sharp-test/unit/generics/typedef.ce"
    free(this->data);

#line 61 "sharp-test/unit/generics/typedef.ce"
    this->data = (float *)0;

#line 62 "sharp-test/unit/generics/typedef.ce"
    this->len = 0;

#line 63 "sharp-test/unit/generics/typedef.ce"
    this->cap = 0;
}


#line 77 "sharp-test/unit/generics/typedef.ce"
__attribute__((weak)) int Arr__int__len(Arr__int * this) {

#line 78 "sharp-test/unit/generics/typedef.ce"
    return this->size;
}

