
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

#line 47 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memcpy(void * __restrict __dest, const void * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 51 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memmove(void * __dest, const void * __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 65 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memset(void * __s, int __c, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 75 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int memcmp(const void * __s1, const void * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 91 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int __memcmpeq(const void * __s1, const void * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 118 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memchr(const void * __s, int __c, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 156 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strcpy(char * __restrict __dest, const char * __restrict __src) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 159 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strncpy(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 164 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strcat(char * __restrict __dest, const char * __restrict __src) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 167 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strncat(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 171 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strcmp(const char * __s1, const char * __s2) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 174 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strncmp(const char * __s1, const char * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 178 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strcoll(const char * __s1, const char * __s2) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 181 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strxfrm(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(2)));

#line 261 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strchr(const char * __s, int __c) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 292 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strrchr(const char * __s, int __c) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 316 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strcspn(const char * __s, const char * __reject) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 320 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strspn(const char * __s, const char * __accept) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 346 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strpbrk(const char * __s, const char * __accept) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 377 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strstr(const char * __haystack, const char * __needle) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 388 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strtok(char * __restrict __s, const char * __restrict __delim) __attribute__((__nothrow__)) __attribute__((__nonnull__(2)));

#line 393 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * __strtok_r(char * __restrict __s, const char * __restrict __delim, char * * __restrict __save_ptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(2,3)));

#line 439 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strlen(const char * __s) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 451 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strerror(int __errnum) __attribute__((__nothrow__));

#line 8 "sharp-test/unit/codegen/struct_layout.ce"
typedef struct MixedFields MixedFields;
struct MixedFields {
    int id;
    char tag;
    int * ptr;
    char name[8];
};

#line 15 "sharp-test/unit/codegen/struct_layout.ce"
int MixedFields__tag_val(MixedFields * this);

#line 17 "sharp-test/unit/codegen/struct_layout.ce"
void MixedFields__set_id(MixedFields * this, int v);

#line 20 "sharp-test/unit/codegen/struct_layout.ce"
typedef struct Inner Inner;
struct Inner {
    int a;
    int b;
};

#line 25 "sharp-test/unit/codegen/struct_layout.ce"
typedef struct Outer Outer;
struct Outer {
    Inner first;
    Inner second;
    int total;
};

#line 31 "sharp-test/unit/codegen/struct_layout.ce"
int Outer__compute_total(Outer * this);

#line 36 "sharp-test/unit/codegen/struct_layout.ce"
struct BitFlags {
    unsigned int x : 4;
    unsigned int y : 8;
    unsigned int z : 1;
};

#line 43 "sharp-test/unit/codegen/struct_layout.ce"
typedef struct Color Color;
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#line 63 "sharp-test/unit/codegen/struct_layout.ce"
unsigned char Color__luminance(Color * this);

#line 68 "sharp-test/unit/codegen/struct_layout.ce"
typedef struct Rect Rect;
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

#line 75 "sharp-test/unit/codegen/struct_layout.ce"
int Rect__area(Rect * this);

#line 92 "sharp-test/unit/codegen/struct_layout.ce"
int Rect__contains(Rect * this, int px, int py);

#line 100 "sharp-test/unit/codegen/struct_layout.ce"
union __anon_union_3 {
    int i_val;
    double d_val;
};

#line 98 "sharp-test/unit/codegen/struct_layout.ce"
struct TaggedValue {
    int kind;
    union {
    int i_val;
    double d_val;
    };
};

#line 108 "sharp-test/unit/codegen/struct_layout.ce"
struct __anon_struct_6 {
    float x;
    float y;
};

#line 107 "sharp-test/unit/codegen/struct_layout.ce"
union __anon_union_5 {
    struct {
    float x;
    float y;
    };
    float data[2];
};

#line 106 "sharp-test/unit/codegen/struct_layout.ce"
struct Coords {
    union {
    struct {
    float x;
    float y;
    };
    float data[2];
    };
};

#line 117 "sharp-test/unit/codegen/struct_layout.ce"
struct FlexHeader {
    int count;
    int items[];
};

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

#line 47 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memcpy(void * __restrict __dest, const void * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 51 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memmove(void * __dest, const void * __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 65 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memset(void * __s, int __c, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1)));

#line 75 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int memcmp(const void * __s1, const void * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 91 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int __memcmpeq(const void * __s1, const void * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 118 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern void * memchr(const void * __s, int __c, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 156 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strcpy(char * __restrict __dest, const char * __restrict __src) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 159 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strncpy(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 164 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strcat(char * __restrict __dest, const char * __restrict __src) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 167 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strncat(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(1,2)));

#line 171 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strcmp(const char * __s1, const char * __s2) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 174 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strncmp(const char * __s1, const char * __s2, size_t __n) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 178 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern int strcoll(const char * __s1, const char * __s2) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 181 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strxfrm(char * __restrict __dest, const char * __restrict __src, size_t __n) __attribute__((__nothrow__)) __attribute__((__nonnull__(2)));

#line 261 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strchr(const char * __s, int __c) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 292 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strrchr(const char * __s, int __c) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 316 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strcspn(const char * __s, const char * __reject) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 320 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strspn(const char * __s, const char * __accept) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 346 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strpbrk(const char * __s, const char * __accept) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 377 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strstr(const char * __haystack, const char * __needle) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1,2)));

#line 388 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strtok(char * __restrict __s, const char * __restrict __delim) __attribute__((__nothrow__)) __attribute__((__nonnull__(2)));

#line 393 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * __strtok_r(char * __restrict __s, const char * __restrict __delim, char * * __restrict __save_ptr) __attribute__((__nothrow__)) __attribute__((__nonnull__(2,3)));

#line 439 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern size_t strlen(const char * __s) __attribute__((__nothrow__)) __attribute__((__pure__)) __attribute__((__nonnull__(1)));

#line 451 "/root/code/sharp/zig/lib/libc/include/generic-glibc/string.h"
extern char * strerror(int __errnum) __attribute__((__nothrow__));

#line 15 "sharp-test/unit/codegen/struct_layout.ce"
int MixedFields__tag_val(MixedFields * this) {
    return (int)this->tag;
}

#line 17 "sharp-test/unit/codegen/struct_layout.ce"
void MixedFields__set_id(MixedFields * this, int v) {
    this->id = v;
}

#line 31 "sharp-test/unit/codegen/struct_layout.ce"
int Outer__compute_total(Outer * this) {

#line 32 "sharp-test/unit/codegen/struct_layout.ce"
    return this->first.a + this->first.b + this->second.a + this->second.b;
}

#line 49 "sharp-test/unit/codegen/struct_layout.ce"
Color make_color(unsigned char r, unsigned char g, unsigned char b) {

#line 50 "sharp-test/unit/codegen/struct_layout.ce"
    Color c;

#line 51 "sharp-test/unit/codegen/struct_layout.ce"
    c.r = r;
    c.g = g;
    c.b = b;

#line 52 "sharp-test/unit/codegen/struct_layout.ce"
    return c;
}

#line 55 "sharp-test/unit/codegen/struct_layout.ce"
Color brighten(Color c, int amount) {

#line 56 "sharp-test/unit/codegen/struct_layout.ce"
    Color result;

#line 57 "sharp-test/unit/codegen/struct_layout.ce"
    result.r = (unsigned char)(c.r + amount > 255 ? 255 : c.r + amount);

#line 58 "sharp-test/unit/codegen/struct_layout.ce"
    result.g = (unsigned char)(c.g + amount > 255 ? 255 : c.g + amount);

#line 59 "sharp-test/unit/codegen/struct_layout.ce"
    result.b = (unsigned char)(c.b + amount > 255 ? 255 : c.b + amount);

#line 60 "sharp-test/unit/codegen/struct_layout.ce"
    return result;
}

#line 63 "sharp-test/unit/codegen/struct_layout.ce"
unsigned char Color__luminance(Color * this) {

#line 64 "sharp-test/unit/codegen/struct_layout.ce"
    return (unsigned char)((this->r * 77 + this->g * 150 + this->b * 29) / 256);
}

#line 75 "sharp-test/unit/codegen/struct_layout.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 77 "sharp-test/unit/codegen/struct_layout.ce"
Rect make_rect(int x, int y, int w, int h) {

#line 78 "sharp-test/unit/codegen/struct_layout.ce"
    Rect r;

#line 79 "sharp-test/unit/codegen/struct_layout.ce"
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;

#line 80 "sharp-test/unit/codegen/struct_layout.ce"
    return r;
}

#line 83 "sharp-test/unit/codegen/struct_layout.ce"
Rect translate_rect(Rect r, int dx, int dy) {

#line 84 "sharp-test/unit/codegen/struct_layout.ce"
    Rect out;

#line 85 "sharp-test/unit/codegen/struct_layout.ce"
    out.x = r.x + dx;

#line 86 "sharp-test/unit/codegen/struct_layout.ce"
    out.y = r.y + dy;

#line 87 "sharp-test/unit/codegen/struct_layout.ce"
    out.w = r.w;

#line 88 "sharp-test/unit/codegen/struct_layout.ce"
    out.h = r.h;

#line 89 "sharp-test/unit/codegen/struct_layout.ce"
    return out;
}

#line 92 "sharp-test/unit/codegen/struct_layout.ce"
int Rect__contains(Rect * this, int px, int py) {

#line 93 "sharp-test/unit/codegen/struct_layout.ce"
    return px >= this->x && px < this->x + this->w && py >= this->y && py < this->y + this->h;
}

#line 122 "sharp-test/unit/codegen/struct_layout.ce"
int main(void) {

#line 124 "sharp-test/unit/codegen/struct_layout.ce"
    MixedFields mf;

#line 125 "sharp-test/unit/codegen/struct_layout.ce"
    int val = 42;

#line 126 "sharp-test/unit/codegen/struct_layout.ce"
    mf.id = 10;

#line 127 "sharp-test/unit/codegen/struct_layout.ce"
    mf.tag = 'A';

#line 128 "sharp-test/unit/codegen/struct_layout.ce"
    mf.ptr = &val;

#line 129 "sharp-test/unit/codegen/struct_layout.ce"
    mf.name[0] = 't';
    mf.name[1] = 'e';
    mf.name[2] = 's';
    mf.name[3] = 't';

#line 130 "sharp-test/unit/codegen/struct_layout.ce"
    mf.name[4] = '\0';

#line 131 "sharp-test/unit/codegen/struct_layout.ce"
    if (mf.id != 10) 
        return 1;

#line 132 "sharp-test/unit/codegen/struct_layout.ce"
    if (MixedFields__tag_val(&mf) != (int)'A') 
        return 2;

#line 133 "sharp-test/unit/codegen/struct_layout.ce"
    if (mf.ptr[0] != 42) 
        return 3;

#line 134 "sharp-test/unit/codegen/struct_layout.ce"
    if (mf.name[0] != 't') 
        return 4;

#line 135 "sharp-test/unit/codegen/struct_layout.ce"
    MixedFields__set_id(&mf, 99);

#line 136 "sharp-test/unit/codegen/struct_layout.ce"
    if (mf.id != 99) 
        return 5;

#line 139 "sharp-test/unit/codegen/struct_layout.ce"
    Outer o;

#line 140 "sharp-test/unit/codegen/struct_layout.ce"
    o.first.a = 1;
    o.first.b = 2;

#line 141 "sharp-test/unit/codegen/struct_layout.ce"
    o.second.a = 3;
    o.second.b = 4;

#line 142 "sharp-test/unit/codegen/struct_layout.ce"
    o.total = Outer__compute_total(&o);

#line 143 "sharp-test/unit/codegen/struct_layout.ce"
    if (o.total != 10) 
        return 6;

#line 144 "sharp-test/unit/codegen/struct_layout.ce"
    if (o.first.a + o.second.b != 5) 
        return 7;

#line 147 "sharp-test/unit/codegen/struct_layout.ce"
    struct BitFlags bf;

#line 148 "sharp-test/unit/codegen/struct_layout.ce"
    bf.x = 12;

#line 149 "sharp-test/unit/codegen/struct_layout.ce"
    bf.y = 200;

#line 150 "sharp-test/unit/codegen/struct_layout.ce"
    bf.z = 1;

#line 151 "sharp-test/unit/codegen/struct_layout.ce"
    if (bf.x != 12) 
        return 8;

#line 152 "sharp-test/unit/codegen/struct_layout.ce"
    if (bf.y != 200) 
        return 9;

#line 153 "sharp-test/unit/codegen/struct_layout.ce"
    if (bf.z != 1) 
        return 10;

#line 154 "sharp-test/unit/codegen/struct_layout.ce"
    bf.x = 31;

#line 155 "sharp-test/unit/codegen/struct_layout.ce"
    if (bf.x != 15) 
        return 11;

#line 156 "sharp-test/unit/codegen/struct_layout.ce"
    bf.y = 300;

#line 157 "sharp-test/unit/codegen/struct_layout.ce"
    if (bf.y != 44) 
        return 12;

#line 160 "sharp-test/unit/codegen/struct_layout.ce"
    Color c1 = make_color(100, 150, 200);

#line 161 "sharp-test/unit/codegen/struct_layout.ce"
    Color c2 = c1;

#line 162 "sharp-test/unit/codegen/struct_layout.ce"
    if (c2.r != 100) 
        return 13;

#line 163 "sharp-test/unit/codegen/struct_layout.ce"
    if (c2.g != 150) 
        return 14;

#line 164 "sharp-test/unit/codegen/struct_layout.ce"
    if (c2.b != 200) 
        return 15;

#line 165 "sharp-test/unit/codegen/struct_layout.ce"
    c2.r = 0;

#line 166 "sharp-test/unit/codegen/struct_layout.ce"
    if (c1.r != 100) 
        return 16;

#line 167 "sharp-test/unit/codegen/struct_layout.ce"
    Color c3 = brighten(c1, 50);

#line 168 "sharp-test/unit/codegen/struct_layout.ce"
    if (c3.r != 150) 
        return 17;

#line 169 "sharp-test/unit/codegen/struct_layout.ce"
    if (c3.g != 200) 
        return 18;

#line 170 "sharp-test/unit/codegen/struct_layout.ce"
    if (c3.b != 250) 
        return 19;

#line 171 "sharp-test/unit/codegen/struct_layout.ce"
    unsigned char lum = Color__luminance(&c1);

#line 172 "sharp-test/unit/codegen/struct_layout.ce"
    if (lum == 0) 
        return 20;

#line 175 "sharp-test/unit/codegen/struct_layout.ce"
    Rect r1 = make_rect(10, 20, 30, 40);

#line 176 "sharp-test/unit/codegen/struct_layout.ce"
    if (Rect__area(&r1) != 1200) 
        return 21;

#line 177 "sharp-test/unit/codegen/struct_layout.ce"
    if (!Rect__contains(&r1, 15, 25)) 
        return 22;

#line 178 "sharp-test/unit/codegen/struct_layout.ce"
    if (Rect__contains(&r1, 5, 25)) 
        return 23;

#line 179 "sharp-test/unit/codegen/struct_layout.ce"
    Rect r2 = translate_rect(r1, 5, -10);

#line 180 "sharp-test/unit/codegen/struct_layout.ce"
    if (r2.x != 15) 
        return 24;

#line 181 "sharp-test/unit/codegen/struct_layout.ce"
    if (r2.y != 10) 
        return 25;

#line 182 "sharp-test/unit/codegen/struct_layout.ce"
    if (r2.w != 30) 
        return 26;

#line 183 "sharp-test/unit/codegen/struct_layout.ce"
    if (r2.h != 40) 
        return 27;

#line 186 "sharp-test/unit/codegen/struct_layout.ce"
    struct TaggedValue tv;

#line 187 "sharp-test/unit/codegen/struct_layout.ce"
    tv.kind = 0;

#line 188 "sharp-test/unit/codegen/struct_layout.ce"
    tv.i_val = 42;

#line 189 "sharp-test/unit/codegen/struct_layout.ce"
    if (tv.i_val != 42) 
        return 28;

#line 190 "sharp-test/unit/codegen/struct_layout.ce"
    tv.kind = 1;

#line 191 "sharp-test/unit/codegen/struct_layout.ce"
    tv.d_val = 3.14;

#line 192 "sharp-test/unit/codegen/struct_layout.ce"
    if (tv.d_val < 3.13 || tv.d_val > 3.15) 
        return 29;

#line 194 "sharp-test/unit/codegen/struct_layout.ce"
    struct Coords co;

#line 195 "sharp-test/unit/codegen/struct_layout.ce"
    co.x = 1.0f;

#line 196 "sharp-test/unit/codegen/struct_layout.ce"
    co.y = 2.0f;

#line 197 "sharp-test/unit/codegen/struct_layout.ce"
    if (co.data[0] < 0.99f || co.data[0] > 1.01f) 
        return 30;

#line 198 "sharp-test/unit/codegen/struct_layout.ce"
    if (co.data[1] < 1.99f || co.data[1] > 2.01f) 
        return 31;

#line 199 "sharp-test/unit/codegen/struct_layout.ce"
    co.data[0] = 5.0f;

#line 200 "sharp-test/unit/codegen/struct_layout.ce"
    co.data[1] = 6.0f;

#line 201 "sharp-test/unit/codegen/struct_layout.ce"
    if (co.x < 4.99f || co.x > 5.01f) 
        return 32;

#line 202 "sharp-test/unit/codegen/struct_layout.ce"
    if (co.y < 5.99f || co.y > 6.01f) 
        return 33;

#line 205 "sharp-test/unit/codegen/struct_layout.ce"
    struct FlexHeader * fh = (struct FlexHeader *)malloc(sizeof(struct FlexHeader) + 3 * sizeof(int));

#line 206 "sharp-test/unit/codegen/struct_layout.ce"
    fh->count = 3;

#line 207 "sharp-test/unit/codegen/struct_layout.ce"
    fh->items[0] = 10;

#line 208 "sharp-test/unit/codegen/struct_layout.ce"
    fh->items[1] = 20;

#line 209 "sharp-test/unit/codegen/struct_layout.ce"
    fh->items[2] = 30;

#line 210 "sharp-test/unit/codegen/struct_layout.ce"
    if (fh->count != 3) 
        return 34;

#line 211 "sharp-test/unit/codegen/struct_layout.ce"
    if (fh->items[0] != 10) 
        return 35;

#line 212 "sharp-test/unit/codegen/struct_layout.ce"
    if (fh->items[1] != 20) 
        return 36;

#line 213 "sharp-test/unit/codegen/struct_layout.ce"
    if (fh->items[2] != 30) 
        return 37;

#line 214 "sharp-test/unit/codegen/struct_layout.ce"
    free(fh);

#line 216 "sharp-test/unit/codegen/struct_layout.ce"
    return 0;
}
