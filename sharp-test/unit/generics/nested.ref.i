
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

#line 7 "sharp-test/unit/generics/nested.ce"
typedef long isize;

#line 8 "sharp-test/unit/generics/nested.ce"
typedef unsigned long usize;

#line 11 "sharp-test/unit/generics/nested.ce"

#line 46 "sharp-test/unit/generics/nested.ce"

#line 11 "sharp-test/unit/generics/nested.ce"
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


typedef struct Vec__Vec__int Vec__Vec__int;
struct Vec__Vec__int {
    Vec__int * data;
    isize len;
    isize cap;
};


typedef struct Vec__Vec__Vec__int Vec__Vec__Vec__int;
struct Vec__Vec__Vec__int {
    Vec__Vec__int * data;
    isize len;
    isize cap;
};



#line 46 "sharp-test/unit/generics/nested.ce"
typedef struct Pair__int__Vec__int Pair__int__Vec__int;
struct Pair__int__Vec__int {
    int first;
    Vec__int second;
};



#line 17 "sharp-test/unit/generics/nested.ce"
void Vec__int__push(Vec__int * this, int val);

#line 29 "sharp-test/unit/generics/nested.ce"
long Vec__int__size(Vec__int * this);

#line 33 "sharp-test/unit/generics/nested.ce"
int Vec__int__get(Vec__int * this, long i);

#line 38 "sharp-test/unit/generics/nested.ce"
void Vec__int__destroy(Vec__int * this);

#line 17 "sharp-test/unit/generics/nested.ce"
void Vec__float__push(Vec__float * this, float val);

#line 29 "sharp-test/unit/generics/nested.ce"
long Vec__float__size(Vec__float * this);

#line 33 "sharp-test/unit/generics/nested.ce"
float Vec__float__get(Vec__float * this, long i);

#line 38 "sharp-test/unit/generics/nested.ce"
void Vec__float__destroy(Vec__float * this);

#line 17 "sharp-test/unit/generics/nested.ce"
void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val);
void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val);

#line 29 "sharp-test/unit/generics/nested.ce"
long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this);

#line 33 "sharp-test/unit/generics/nested.ce"
Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i);

#line 29 "sharp-test/unit/generics/nested.ce"
long Vec__Vec__int__size(Vec__Vec__int * this);

#line 33 "sharp-test/unit/generics/nested.ce"
Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i);

#line 38 "sharp-test/unit/generics/nested.ce"
void Vec__Vec__int__destroy(Vec__Vec__int * this);
void Vec__Vec__Vec__int__destroy(Vec__Vec__Vec__int * this);

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

#line 17 "sharp-test/unit/generics/nested.ce"

#line 29 "sharp-test/unit/generics/nested.ce"

#line 33 "sharp-test/unit/generics/nested.ce"

#line 38 "sharp-test/unit/generics/nested.ce"

#line 51 "sharp-test/unit/generics/nested.ce"
int main() {

#line 53 "sharp-test/unit/generics/nested.ce"
    Vec__int v = { 0 };

#line 54 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&v, 42);

#line 55 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&v, 73);

#line 56 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__size(&v) != 2) 
        return 1;

#line 57 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&v, 0) != 42) 
        return 2;

#line 58 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&v, 1) != 73) 
        return 3;

#line 59 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&v);

#line 62 "sharp-test/unit/generics/nested.ce"
    Vec__float fv = { 0 };

#line 63 "sharp-test/unit/generics/nested.ce"
    Vec__float__push(&fv, 1.5f);

#line 64 "sharp-test/unit/generics/nested.ce"
    Vec__float__push(&fv, 2.5f);

#line 65 "sharp-test/unit/generics/nested.ce"
    if (Vec__float__size(&fv) != 2) 
        return 4;

#line 66 "sharp-test/unit/generics/nested.ce"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 5;

#line 67 "sharp-test/unit/generics/nested.ce"
    Vec__float__destroy(&fv);

#line 70 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__Vec__int v3d = { 0 };

#line 71 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int inner2d = { 0 };

#line 72 "sharp-test/unit/generics/nested.ce"
    Vec__int inner1d = { 0 };

#line 73 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&inner1d, 42);

#line 74 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&inner1d, 73);

#line 75 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__push(&inner2d, inner1d);

#line 76 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__Vec__int__push(&v3d, inner2d);

#line 78 "sharp-test/unit/generics/nested.ce"
    if (Vec__Vec__Vec__int__size(&v3d) != 1) 
        return 6;

#line 79 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int r2 = Vec__Vec__Vec__int__get(&v3d, 0);

#line 80 "sharp-test/unit/generics/nested.ce"
    if (Vec__Vec__int__size(&r2) != 1) 
        return 7;

#line 81 "sharp-test/unit/generics/nested.ce"
    Vec__int r1 = Vec__Vec__int__get(&r2, 0);

#line 82 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__size(&r1) != 2) 
        return 8;

#line 83 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&r1, 0) != 42) 
        return 9;

#line 84 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&r1);

#line 85 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__destroy(&r2);

#line 86 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__Vec__int__destroy(&v3d);

#line 89 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int m2d = { 0 };

#line 90 "sharp-test/unit/generics/nested.ce"
    Vec__int row0 = { 0 };

#line 91 "sharp-test/unit/generics/nested.ce"
    Vec__int row1 = { 0 };

#line 92 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row0, 1);

#line 93 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row1, 10);

#line 94 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row1, 20);

#line 95 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__push(&m2d, row0);

#line 96 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__push(&m2d, row1);

#line 98 "sharp-test/unit/generics/nested.ce"
    if (Vec__Vec__int__size(&m2d) != 2) 
        return 10;

#line 99 "sharp-test/unit/generics/nested.ce"
    Vec__int c0 = Vec__Vec__int__get(&m2d, 0);

#line 100 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&c0, 0) != 1) 
        return 11;

#line 101 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&c0);

#line 102 "sharp-test/unit/generics/nested.ce"
    Vec__int c1 = Vec__Vec__int__get(&m2d, 1);

#line 103 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&c1, 1) != 20) 
        return 12;

#line 104 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&c1);

#line 105 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__destroy(&m2d);

#line 108 "sharp-test/unit/generics/nested.ce"
    Pair__int__Vec__int pv = { 0 };

#line 109 "sharp-test/unit/generics/nested.ce"
    pv.first = 42;

#line 112 "sharp-test/unit/generics/nested.ce"
    Vec__int row_a = { 0 };

#line 113 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row_a, 1);

#line 114 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row_a, 2);

#line 115 "sharp-test/unit/generics/nested.ce"
    Vec__int row_b = { 0 };

#line 116 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row_b, 3);

#line 117 "sharp-test/unit/generics/nested.ce"
    Vec__int__push(&row_b, 4);

#line 118 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int matrix = { 0 };

#line 119 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__push(&matrix, row_a);

#line 120 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__push(&matrix, row_b);

#line 121 "sharp-test/unit/generics/nested.ce"
    if (Vec__Vec__int__size(&matrix) != 2) 
        return 13;

#line 122 "sharp-test/unit/generics/nested.ce"
    Vec__int r0 = Vec__Vec__int__get(&matrix, 0);

#line 123 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__size(&r0) != 2) 
        return 14;

#line 124 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&r0, 1) != 2) 
        return 15;

#line 125 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&r0);

#line 126 "sharp-test/unit/generics/nested.ce"
    Vec__int mr1 = Vec__Vec__int__get(&matrix, 1);

#line 127 "sharp-test/unit/generics/nested.ce"
    if (Vec__int__get(&mr1, 0) != 3) 
        return 16;

#line 128 "sharp-test/unit/generics/nested.ce"
    Vec__int__destroy(&mr1);

#line 129 "sharp-test/unit/generics/nested.ce"
    Vec__Vec__int__destroy(&matrix);

#line 131 "sharp-test/unit/generics/nested.ce"
    return 0;
}

#line 17 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 18 "sharp-test/unit/generics/nested.ce"
    if (this->len >= this->cap) {

#line 19 "sharp-test/unit/generics/nested.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "sharp-test/unit/generics/nested.ce"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 21 "sharp-test/unit/generics/nested.ce"
        if (!nd) 
            __builtin_trap();

#line 22 "sharp-test/unit/generics/nested.ce"
        this->data = nd;

#line 23 "sharp-test/unit/generics/nested.ce"
        this->cap = new_cap;
    }

#line 25 "sharp-test/unit/generics/nested.ce"
    this->data[this->len] = val;

#line 26 "sharp-test/unit/generics/nested.ce"
    this->len = this->len + 1;
}


#line 29 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 30 "sharp-test/unit/generics/nested.ce"
    return this->len;
}


#line 33 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 34 "sharp-test/unit/generics/nested.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "sharp-test/unit/generics/nested.ce"
    return this->data[i];
}


#line 38 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 39 "sharp-test/unit/generics/nested.ce"
    free(this->data);

#line 40 "sharp-test/unit/generics/nested.ce"
    this->data = (int *)0;

#line 41 "sharp-test/unit/generics/nested.ce"
    this->len = 0;

#line 42 "sharp-test/unit/generics/nested.ce"
    this->cap = 0;
}


#line 17 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 18 "sharp-test/unit/generics/nested.ce"
    if (this->len >= this->cap) {

#line 19 "sharp-test/unit/generics/nested.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "sharp-test/unit/generics/nested.ce"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 21 "sharp-test/unit/generics/nested.ce"
        if (!nd) 
            __builtin_trap();

#line 22 "sharp-test/unit/generics/nested.ce"
        this->data = nd;

#line 23 "sharp-test/unit/generics/nested.ce"
        this->cap = new_cap;
    }

#line 25 "sharp-test/unit/generics/nested.ce"
    this->data[this->len] = val;

#line 26 "sharp-test/unit/generics/nested.ce"
    this->len = this->len + 1;
}


#line 29 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 30 "sharp-test/unit/generics/nested.ce"
    return this->len;
}


#line 33 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 34 "sharp-test/unit/generics/nested.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "sharp-test/unit/generics/nested.ce"
    return this->data[i];
}


#line 38 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 39 "sharp-test/unit/generics/nested.ce"
    free(this->data);

#line 40 "sharp-test/unit/generics/nested.ce"
    this->data = (float *)0;

#line 41 "sharp-test/unit/generics/nested.ce"
    this->len = 0;

#line 42 "sharp-test/unit/generics/nested.ce"
    this->cap = 0;
}


#line 17 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val) {

#line 18 "sharp-test/unit/generics/nested.ce"
    if (this->len >= this->cap) {

#line 19 "sharp-test/unit/generics/nested.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "sharp-test/unit/generics/nested.ce"
        Vec__int * nd = (Vec__int *)realloc(this->data, sizeof(Vec__int) * new_cap);

#line 21 "sharp-test/unit/generics/nested.ce"
        if (!nd) 
            __builtin_trap();

#line 22 "sharp-test/unit/generics/nested.ce"
        this->data = nd;

#line 23 "sharp-test/unit/generics/nested.ce"
        this->cap = new_cap;
    }

#line 25 "sharp-test/unit/generics/nested.ce"
    this->data[this->len] = val;

#line 26 "sharp-test/unit/generics/nested.ce"
    this->len = this->len + 1;
}


#line 17 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val) {

#line 18 "sharp-test/unit/generics/nested.ce"
    if (this->len >= this->cap) {

#line 19 "sharp-test/unit/generics/nested.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "sharp-test/unit/generics/nested.ce"
        Vec__Vec__int * nd = (Vec__Vec__int *)realloc(this->data, sizeof(Vec__Vec__int) * new_cap);

#line 21 "sharp-test/unit/generics/nested.ce"
        if (!nd) 
            __builtin_trap();

#line 22 "sharp-test/unit/generics/nested.ce"
        this->data = nd;

#line 23 "sharp-test/unit/generics/nested.ce"
        this->cap = new_cap;
    }

#line 25 "sharp-test/unit/generics/nested.ce"
    this->data[this->len] = val;

#line 26 "sharp-test/unit/generics/nested.ce"
    this->len = this->len + 1;
}


#line 29 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this) {

#line 30 "sharp-test/unit/generics/nested.ce"
    return this->len;
}


#line 33 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i) {

#line 34 "sharp-test/unit/generics/nested.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "sharp-test/unit/generics/nested.ce"
    return this->data[i];
}


#line 29 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) long Vec__Vec__int__size(Vec__Vec__int * this) {

#line 30 "sharp-test/unit/generics/nested.ce"
    return this->len;
}


#line 33 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i) {

#line 34 "sharp-test/unit/generics/nested.ce"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "sharp-test/unit/generics/nested.ce"
    return this->data[i];
}


#line 38 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__Vec__int__destroy(Vec__Vec__int * this) {

#line 39 "sharp-test/unit/generics/nested.ce"
    free(this->data);

#line 40 "sharp-test/unit/generics/nested.ce"
    this->data = (Vec__int *)0;

#line 41 "sharp-test/unit/generics/nested.ce"
    this->len = 0;

#line 42 "sharp-test/unit/generics/nested.ce"
    this->cap = 0;
}


#line 38 "sharp-test/unit/generics/nested.ce"
__attribute__((weak)) void Vec__Vec__Vec__int__destroy(Vec__Vec__Vec__int * this) {

#line 39 "sharp-test/unit/generics/nested.ce"
    free(this->data);

#line 40 "sharp-test/unit/generics/nested.ce"
    this->data = (Vec__Vec__int *)0;

#line 41 "sharp-test/unit/generics/nested.ce"
    this->len = 0;

#line 42 "sharp-test/unit/generics/nested.ce"
    this->cap = 0;
}

