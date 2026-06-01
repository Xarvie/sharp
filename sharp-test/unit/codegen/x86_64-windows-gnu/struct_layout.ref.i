
#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\vadefs.h"
typedef __builtin_va_list __gnuc_va_list;

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\vadefs.h"
typedef __gnuc_va_list va_list;

#line 604 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
void __attribute__((__cdecl__)) __debugbreak(void);

#line 625 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code);

#line 665 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
const char * __mingw_get_crt_info(void);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef unsigned long long size_t;

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef long long ssize_t;

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef size_t rsize_t;

#line 62 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef long long intptr_t;

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef unsigned long long uintptr_t;

#line 88 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef long long ptrdiff_t;

#line 98 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef unsigned short wchar_t;

#line 106 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef unsigned short wint_t;

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef unsigned short wctype_t;

#line 113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef int errno_t;

#line 118 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef long __time32_t;

#line 123 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
__extension__ typedef long long __time64_t;

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef __time64_t time_t;

#line 430 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
struct threadlocaleinfostruct;

#line 431 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
struct threadmbcinfostruct;

#line 432 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef struct threadlocaleinfostruct * pthreadlocinfo;

#line 433 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef struct threadmbcinfostruct * pthreadmbcinfo;

#line 434 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
struct __lc_time_data;

#line 436 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"

#line 439 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef struct localeinfo_struct {
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
} _locale_tstruct;
typedef struct localeinfo_struct * _locale_t;

#line 443 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"

#line 447 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef struct tagLC_ID {
    unsigned short wLanguage;
    unsigned short wCountry;
    unsigned short wCodePage;
} LC_ID;
typedef struct tagLC_ID * LPLC_ID;

#line 452 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"

#line 482 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt.h"
typedef struct threadlocaleinfostruct {
    const unsigned short * _locale_pctype;
    int _locale_mb_cur_max;
    unsigned int _locale_lc_codepage;
} threadlocinfo;

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wdupenv_s(wchar_t * * _Buffer, size_t * _BufferSizeInWords, const wchar_t * _VarName);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itow_s(int _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 27 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltow_s(long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultow_s(unsigned long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wgetenv_s(size_t * _ReturnSize, wchar_t * _DstBuf, size_t _DstSizeInWords, const wchar_t * _VarName);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64tow_s(long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64tow_s(unsigned long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 39 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wmakepath_s(wchar_t * _PathResult, size_t _SizeInWords, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 42 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wputenv_s(const wchar_t * _Name, const wchar_t * _Value);

#line 44 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsearchenv_s(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath, size_t _SizeInWords);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsplitpath_s(const wchar_t * _FullPath, wchar_t * _Drive, size_t _DriveSizeInWords, wchar_t * _Dir, size_t _DirSizeInWords, wchar_t * _Filename, size_t _FilenameSizeInWords, wchar_t * _Ext, size_t _ExtSizeInWords);

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef int (*_onexit_t)(void);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct _div_t {
    int quot;
    int rem;
} div_t;

#line 65 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 68 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct _ldiv_t {
    long quot;
    long rem;
} ldiv_t;

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 77 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    unsigned char ld[10];
} _LDOUBLE;

#line 82 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    double x;
} _CRT_DOUBLE;

#line 86 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 88 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    float f;
} _CRT_FLOAT;

#line 93 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 95 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    long double x;
} _LONGDOUBLE;

#line 100 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"

#line 102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    unsigned char ld12[12];
} _LDBL12;

#line 113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) ___mb_cur_max_func(void);

#line 135 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef void (*_purecall_handler)(void);

#line 137 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_set_purecall_handler(_purecall_handler _Handler))(void);

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_get_purecall_handler(void))(void);

#line 140 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef void (*_invalid_parameter_handler)(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t);

#line 141 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_set_invalid_parameter_handler(_invalid_parameter_handler _Handler))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 142 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_get_invalid_parameter_handler(void))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) extern int * _errno(void);

#line 148 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _set_errno(int _Value);

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_errno(int * _Value);

#line 151 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long * __doserrno(void);

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _set_doserrno(unsigned long _Value);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_doserrno(unsigned long * _Value);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * __sys_errlist(void);

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __sys_nerr(void);

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * * __p___argv(void);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __p__fmode(void);

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __p___argc(void);

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p___wargv(void);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * __p__pgmptr(void);

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * __p__wpgmptr(void);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_pgmptr(char * * _Value);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_wpgmptr(wchar_t * * _Value);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_fmode(int _Mode);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_fmode(int * _PMode);

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * * __p__environ(void);

#line 222 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p__wenviron(void);

#line 234 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osplatform(void);

#line 235 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osver(void);

#line 236 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winver(void);

#line 237 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winmajor(void);

#line 238 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winminor(void);

#line 256 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osplatform(unsigned int * _Value);

#line 257 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osver(unsigned int * _Value);

#line 258 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winver(unsigned int * _Value);

#line 259 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winmajor(unsigned int * _Value);

#line 260 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winminor(unsigned int * _Value);

#line 274 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) exit(int _Code) __attribute__((__noreturn__));

#line 275 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) _exit(int _Code) __attribute__((__noreturn__));

#line 277 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) quick_exit(int _Code) __attribute__((__noreturn__));

#line 282 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) _Exit(int) __attribute__((__noreturn__));

#line 291 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) abort(void);

#line 296 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_abort_behavior(unsigned int _Flags, unsigned int _Mask);

#line 300 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) abs(int _X);

#line 301 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) labs(long _X);

#line 304 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) _abs64(long long);

#line 311 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) atexit(void (*)(void));

#line 313 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) at_quick_exit(void (*)(void));

#line 317 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) atof(const char * _String);

#line 318 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) _atof_l(const char * _String, _locale_t _Locale);

#line 320 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) atoi(const char * _Str);

#line 321 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoi_l(const char * _Str, _locale_t _Locale);

#line 322 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) atol(const char * _Str);

#line 323 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _atol_l(const char * _Str, _locale_t _Locale);

#line 326 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * bsearch(const void * _Key, const void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 327 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) qsort(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 329 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned short __attribute__((__cdecl__)) _byteswap_ushort(unsigned short _Short);

#line 330 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _byteswap_ulong(unsigned long _Long);

#line 331 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _byteswap_uint64(unsigned long long _Int64);

#line 332 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
div_t __attribute__((__cdecl__)) div(int _Numerator, int _Denominator);

#line 333 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * getenv(const char * _VarName);

#line 334 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _itoa(int _Value, char * _Dest, int _Radix);

#line 335 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _i64toa(long long _Val, char * _DstBuf, int _Radix);

#line 336 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ui64toa(unsigned long long _Val, char * _DstBuf, int _Radix);

#line 337 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64(const char * _String);

#line 338 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64_l(const char * _String, _locale_t _Locale);

#line 339 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64(const char * _String, char * * _EndPtr, int _Radix);

#line 340 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 341 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64(const char * _String, char * * _EndPtr, int _Radix);

#line 342 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 343 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
ldiv_t __attribute__((__cdecl__)) ldiv(long _Numerator, long _Denominator);

#line 344 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ltoa(long _Value, char * _Dest, int _Radix);

#line 345 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mblen(const char * _Ch, size_t _MaxCount);

#line 346 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mblen_l(const char * _Ch, size_t _MaxCount, _locale_t _Locale);

#line 347 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen(const char * _Str);

#line 348 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen_l(const char * _Str, _locale_t _Locale);

#line 349 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen(const char * _Str, size_t _MaxCount);

#line 350 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen_l(const char * _Str, size_t _MaxCount, _locale_t _Locale);

#line 351 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mbtowc(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes);

#line 352 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mbtowc_l(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes, _locale_t _Locale);

#line 353 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
size_t __attribute__((__cdecl__)) mbstowcs(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount);

#line 354 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstowcs_l(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 355 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mkstemp(char * template_name);

#line 356 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) rand(void);

#line 357 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_error_mode(int _Mode);

#line 358 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) srand(unsigned int _Seed);

#line 385 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtod(const char * __restrict__ _Str, char * * __restrict__ _EndPtr);

#line 386 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtof(const char * __restrict__ nptr, char * * __restrict__ endptr);

#line 388 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtold(const char * __restrict__, char * * __restrict__);

#line 392 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
extern double __attribute__((__cdecl__)) __attribute__((__nothrow__)) __strtod(const char * __restrict__, char * * __restrict__);

#line 400 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __mingw_strtof(const char * __restrict__, char * * __restrict__);

#line 401 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __mingw_strtod(const char * __restrict__, char * * __restrict__);

#line 402 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __mingw_strtold(const char * __restrict__, char * * __restrict__);

#line 404 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _strtof_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 405 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _strtod_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 406 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) strtol(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 407 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _strtol_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 408 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) strtoul(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 409 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _strtoul_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 412 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) system(const char * _Command);

#line 414 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ultoa(unsigned long _Value, char * _Dest, int _Radix);

#line 415 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) wctomb(char * _MbCh, wchar_t _WCh);

#line 416 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wctomb_l(char * _MbCh, wchar_t _WCh, _locale_t _Locale);

#line 417 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
size_t __attribute__((__cdecl__)) wcstombs(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount);

#line 418 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcstombs_l(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 452 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * calloc(size_t _NumOfElements, size_t _SizeOfElements);

#line 453 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) free(void * _Memory);

#line 454 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * malloc(size_t _Size);

#line 455 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * realloc(void * _Memory, size_t _NewSize);

#line 456 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _aligned_free(void * _Memory);

#line 457 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_malloc(size_t _Size, size_t _Alignment);

#line 458 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset);

#line 459 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_realloc(void * _Memory, size_t _Size, size_t _Alignment);

#line 460 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 461 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _recalloc(void * _Memory, size_t _Count, size_t _Size);

#line 462 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment);

#line 463 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment, size_t _Offset);

#line 464 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _aligned_msize(void * _Memory, size_t _Alignment, size_t _Offset);

#line 487 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _itow(int _Value, wchar_t * _Dest, int _Radix);

#line 488 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ltow(long _Value, wchar_t * _Dest, int _Radix);

#line 489 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ultow(unsigned long _Value, wchar_t * _Dest, int _Radix);

#line 491 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __mingw_wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 492 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __mingw_wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 493 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __mingw_wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 506 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 507 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 510 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 512 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wcstod_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 513 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _wcstof_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 514 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) wcstol(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 515 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wcstol_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 516 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) wcstoul(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 517 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _wcstoul_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 518 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wgetenv(const wchar_t * _VarName);

#line 521 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wsystem(const wchar_t * _Command);

#line 523 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof(const wchar_t * _Str);

#line 524 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof_l(const wchar_t * _Str, _locale_t _Locale);

#line 525 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi(const wchar_t * _Str);

#line 526 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi_l(const wchar_t * _Str, _locale_t _Locale);

#line 527 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol(const wchar_t * _Str);

#line 528 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol_l(const wchar_t * _Str, _locale_t _Locale);

#line 530 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _i64tow(long long _Val, wchar_t * _DstBuf, int _Radix);

#line 531 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ui64tow(unsigned long long _Val, wchar_t * _DstBuf, int _Radix);

#line 532 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64(const wchar_t * _Str);

#line 533 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64_l(const wchar_t * _Str, _locale_t _Locale);

#line 534 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 535 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 536 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 537 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 540 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putenv(const char * _EnvString);

#line 541 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wputenv(const wchar_t * _EnvString);

#line 550 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _fullpath(char * _FullPath, const char * _Path, size_t _SizeInBytes);

#line 555 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 556 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 557 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 558 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl(_CRT_DOUBLE * _Result, char * _Str);

#line 559 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl(_LDOUBLE * _Result, char * _Str);

#line 560 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt(_CRT_FLOAT * _Result, char * _Str);

#line 561 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl_l(_CRT_DOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 562 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl_l(_LDOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 563 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt_l(_CRT_FLOAT * _Result, char * _Str, _locale_t _Locale);

#line 579 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotl(unsigned long, int);

#line 580 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotr(unsigned long, int);

#line 586 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _makepath(char * _Path, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 587 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int (*_onexit(_onexit_t _Func))(void);

#line 591 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 597 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotl64(unsigned long long _Val, int _Shift);

#line 598 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long Value, int Shift);

#line 605 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotr(unsigned int _Val, int _Shift);

#line 606 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotl(unsigned int _Val, int _Shift);

#line 609 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long _Val, int _Shift);

#line 610 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _searchenv(const char * _Filename, const char * _EnvVar, char * _ResultPath);

#line 611 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _splitpath(const char * _FullPath, char * _Drive, char * _Dir, char * _Filename, char * _Ext);

#line 612 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 620 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wfullpath(wchar_t * _FullPath, const wchar_t * _Path, size_t _SizeInWords);

#line 624 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wmakepath(wchar_t * _ResultPath, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 627 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 629 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsearchenv(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath);

#line 630 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsplitpath(const wchar_t * _FullPath, wchar_t * _Drive, wchar_t * _Dir, wchar_t * _Filename, wchar_t * _Ext);

#line 633 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _beep(unsigned _Frequency, unsigned _Duration) __attribute__((__deprecated__));

#line 635 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _seterrormode(int _Mode) __attribute__((__deprecated__));

#line 636 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _sleep(unsigned long _Duration) __attribute__((__deprecated__));

#line 657 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 658 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 659 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 660 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * itoa(int _Val, char * _DstBuf, int _Radix);

#line 661 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ltoa(long _Val, char * _DstBuf, int _Radix);

#line 662 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) putenv(const char * _EnvString);

#line 666 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 669 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ultoa(unsigned long _Val, char * _Dstbuf, int _Radix);

#line 670 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int (*onexit(_onexit_t _Func))(void);

#line 676 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
typedef struct {
    long long quot, rem;
} lldiv_t;

#line 678 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
lldiv_t __attribute__((__cdecl__)) lldiv(long long, long long);

#line 680 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) llabs(long long);

#line 685 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) strtoll(const char * __restrict__, char * * __restrict, int);

#line 686 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) strtoull(const char * __restrict__, char * * __restrict__, int);

#line 689 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) atoll(const char *);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _dupenv_s(char * * _PBuffer, size_t * _PBufferSizeInBytes, const char * _VarName);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) void * bsearch_s(const void * _Key, const void * _Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 25 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) getenv_s(size_t * _ReturnSize, char * _DstBuf, rsize_t _DstSize, const char * _VarName);

#line 27 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix);

#line 29 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64toa_s(long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64toa_s(unsigned long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltoa_s(long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) mbstowcs_s(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _mbstowcs_s_l(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount, _locale_t _Locale);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultoa_s(unsigned long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 39 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wctomb_s(int * _SizeConverted, char * _MbCh, rsize_t _SizeInBytes, wchar_t _WCh);

#line 40 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wctomb_s_l(int * _SizeConverted, char * _MbCh, size_t _SizeInBytes, wchar_t _WCh, _locale_t _Locale);

#line 41 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcstombs_s(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes);

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcstombs_s_l(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes, _locale_t _Locale);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ecvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDights, int * _PtDec, int * _PtSign);

#line 48 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _fcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _gcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDigits);

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _makepath_s(char * _PathResult, size_t _Size, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _putenv_s(const char * _Name, const char * _Value);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _searchenv_s(const char * _Filename, const char * _EnvVar, char * _ResultPath, size_t _SizeInBytes);

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _splitpath_s(const char * _FullPath, char * _Drive, size_t _DriveSize, char * _Dir, size_t _DirSize, char * _Filename, size_t _FilenameSize, char * _Ext, size_t _ExtSize);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) qsort_s(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
typedef struct _heapinfo {
    int * _pentry;
    size_t _size;
    int _useflag;
} _HEAPINFO;

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) unsigned int * __p__amblksiz(void);

#line 129 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_malloc(size_t _Size, size_t _Alignment);

#line 130 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void __mingw_aligned_free(void * _Memory);

#line 131 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 132 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_offset_malloc(size_t, size_t, size_t);

#line 133 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_realloc(void * _Memory, size_t _Size, size_t _Offset);

#line 134 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
size_t __mingw_aligned_msize(void * memblock, size_t alignment, size_t offset);

#line 144 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _resetstkoflw(void);

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _set_malloc_crt_max_wait(unsigned long _NewValue);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) void * _expand(void * _Memory, size_t _NewSize);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _msize(void * _Memory);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _get_sbh_threshold(void);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_sbh_threshold(size_t _NewValue);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_amblksiz(size_t _Value);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_amblksiz(size_t * _Value);

#line 171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapadd(void * _Memory, size_t _Size);

#line 172 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapchk(void);

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapmin(void);

#line 174 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapset(unsigned int _Fill);

#line 175 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapwalk(_HEAPINFO * _EntryInfo);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _heapused(size_t * _Used, size_t * _Commit);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _get_heap_handle(void);

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) void * _memccpy(void * _Dst, const void * _Src, int _Val, size_t _MaxCount);

#line 46 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memchr(const void * _Buf, int _Val, size_t _MaxCount);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 48 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp_l(const void * _Buf1, const void * _Buf2, size_t _Size, _locale_t _Locale);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) memcmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memcpy(void * __restrict__ _Dst, const void * __restrict__ _Src, size_t _Size);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memcpy_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * mempcpy(void * _Dst, const void * _Src, size_t _Size);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memset(void * _Dst, int _Val, size_t _Size);

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memccpy(void * _Dst, const void * _Src, int _Val, size_t _Size);

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 59 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * _strset(char * _Str, int _Val);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * _strset_l(char * _Str, int _Val, _locale_t _Locale);

#line 61 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strcpy(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 62 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strcat(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcmp(const char * _Str1, const char * _Str2);

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strlen(const char * _Str);

#line 65 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strnlen(const char * _Str, size_t _MaxCount);

#line 66 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memmove(void * _Dst, const void * _Src, size_t _Size);

#line 71 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strdup(const char * _Src);

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strchr(const char * _Str, int _Val);

#line 76 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp(const char * _Str1, const char * _Str2);

#line 77 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcmpi(const char * _Str1, const char * _Str2);

#line 78 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 79 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcoll(const char * _Str1, const char * _Str2);

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 81 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll(const char * _Str1, const char * _Str2);

#line 82 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 86 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strcspn(const char * _Str, const char * _Control);

#line 88 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strerror(const char * _ErrMsg);

#line 89 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strerror(int);

#line 90 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strlwr(char * _String);

#line 91 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strlwr_l(char * _String, _locale_t _Locale);

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strncat(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 93 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strncmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 95 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 96 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strncpy(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 97 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strnset(char * _Str, int _Val, size_t _MaxCount);

#line 98 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strnset_l(char * str, int c, size_t count, _locale_t _Locale);

#line 99 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strpbrk(const char * _Str, const char * _Control);

#line 100 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strrchr(const char * _Str, int _Ch);

#line 101 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strrev(char * _Str);

#line 102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strspn(const char * _Str, const char * _Control);

#line 103 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strstr(const char * _Str, const char * _SubStr);

#line 104 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strtok(char * __restrict__ _Str, const char * __restrict__ _Delim);

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strtok_r(char * __restrict__ _Str, const char * __restrict__ _Delim, char * * __restrict__ __last);

#line 109 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strupr(char * _String);

#line 110 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strupr_l(char * _String, _locale_t _Locale);

#line 111 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strxfrm(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount);

#line 112 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _strxfrm_l(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 119 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strdup(const char * _Src);

#line 123 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcmpi(const char * _Str1, const char * _Str2);

#line 124 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) stricmp(const char * _Str1, const char * _Str2);

#line 125 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strlwr(char * _Str);

#line 126 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strnicmp(const char * _Str1, const char * _Str, size_t _MaxCount);

#line 127 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strncasecmp(const char *, const char *, size_t);

#line 128 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcasecmp(const char *, const char *);

#line 136 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strnset(char * _Str, int _Val, size_t _MaxCount);

#line 137 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strrev(char * _Str);

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strset(char * _Str, int _Val);

#line 139 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strupr(char * _Str);

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsdup(const wchar_t * _Str);

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcscat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcschr(const wchar_t * _Str, wchar_t _Ch);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcscmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcscpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 157 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcscspn(const wchar_t * _Str, const wchar_t * _Control);

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcslen(const wchar_t * _Str);

#line 159 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsnlen(const wchar_t * _Src, size_t _MaxCount);

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsncat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsncmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsncpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * _wcsncpy_l(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count, _locale_t _Locale);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcspbrk(const wchar_t * _Str, const wchar_t * _Control);

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsrchr(const wchar_t * _Str, wchar_t _Ch);

#line 166 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsspn(const wchar_t * _Str, const wchar_t * _Control);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsstr(const wchar_t * _Str, const wchar_t * _SubStr);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim, wchar_t * * __restrict__ _Ptr);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * _wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcserror(int _ErrNum);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * __wcserror(const wchar_t * _Str);

#line 178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 179 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 180 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 181 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsrev(wchar_t * _Str);

#line 184 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsset(wchar_t * _Str, wchar_t _Val);

#line 185 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr(wchar_t * _String);

#line 186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr_l(wchar_t * _String, _locale_t _Locale);

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr(wchar_t * _String);

#line 188 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr_l(wchar_t * _String, _locale_t _Locale);

#line 189 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsxfrm(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount);

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcsxfrm_l(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 191 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcscoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 192 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcscoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 194 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 197 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 198 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 205 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsdup(const wchar_t * _Str);

#line 210 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 211 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 212 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 213 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsrev(wchar_t * _Str);

#line 214 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsset(wchar_t * _Str, wchar_t _Val);

#line 215 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcslwr(wchar_t * _Str);

#line 216 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsupr(wchar_t * _Str);

#line 217 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strset_s(char * _Dst, size_t _DstSize, int _Value);

#line 26 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strerror_s(char * _Buf, size_t _SizeInBytes, const char * _ErrMsg);

#line 28 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) strerror_s(char * _Buf, size_t _SizeInBytes, int _ErrNum);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s(char * _Str, size_t _Size);

#line 32 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strnset_s(char * _Str, size_t _Size, int _Val, size_t _MaxCount);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s(char * _Str, size_t _Size);

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 41 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncat_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncat_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcpy_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncpy_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncpy_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) char * strtok_s(char * _Str, const char * _Delim, char * * _Context);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) char * _strtok_s_l(char * _Str, const char * _Delim, char * * _Context, _locale_t _Locale);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcat_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memmove_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) wchar_t * wcstok_s(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context);

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcserror_s(wchar_t * _Buf, size_t _SizeInWords, int _ErrNum);

#line 66 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) __wcserror_s(wchar_t * _Buffer, size_t _SizeInWords, const wchar_t * _ErrMsg);

#line 67 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s(wchar_t * _Dst, size_t _DstSizeInWords, wchar_t _Val, size_t _MaxCount);

#line 68 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s(wchar_t * _Str, size_t _SizeInWords, wchar_t _Val);

#line 69 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s(wchar_t * _Str, size_t _SizeInWords);

#line 71 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s_l(wchar_t * _Str, size_t _SizeInWords, _locale_t _Locale);

#line 73 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s(wchar_t * _Str, size_t _Size);

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s_l(wchar_t * _Str, size_t _Size, _locale_t _Locale);

#line 78 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscpy_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscat_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncat_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncat_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncpy_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 89 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncpy_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 91 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) wchar_t * _wcstok_s_l(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context, _locale_t _Locale);

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, _locale_t _Locale);

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, size_t _Count, _locale_t _Locale);

#line 8 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
typedef struct MixedFields MixedFields;
struct MixedFields {
    int id;
    char tag;
    int * ptr;
    char name[8];
};

#line 15 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int MixedFields__tag_val(MixedFields * this);

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
void MixedFields__set_id(MixedFields * this, int v);

#line 20 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
typedef struct Inner Inner;
struct Inner {
    int a;
    int b;
};

#line 25 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
typedef struct Outer Outer;
struct Outer {
    Inner first;
    Inner second;
    int total;
};

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Outer__compute_total(Outer * this);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
struct BitFlags {
    unsigned int x : 4;
    unsigned int y : 8;
    unsigned int z : 1;
};

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
typedef struct Color Color;
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
unsigned char Color__luminance(Color * this);

#line 68 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
typedef struct Rect Rect;
struct Rect {
    int x;
    int y;
    int w;
    int h;
};

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Rect__area(Rect * this);

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Rect__contains(Rect * this, int px, int py);

#line 100 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
union __anon_union_6 {
    int i_val;
    double d_val;
};

#line 98 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
struct TaggedValue {
    int kind;
    union {
    int i_val;
    double d_val;
    };
};

#line 108 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
struct __anon_struct_9 {
    float x;
    float y;
};

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
union __anon_union_8 {
    struct {
    float x;
    float y;
    };
    float data[2];
};

#line 106 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
struct Coords {
    union {
    struct {
    float x;
    float y;
    };
    float data[2];
    };
};

#line 117 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
struct FlexHeader {
    int count;
    int items[];
};

#line 604 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
void __attribute__((__cdecl__)) __debugbreak(void);

#line 605 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) void __attribute__((__cdecl__)) __debugbreak(void) {

#line 610 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
    __asm__ __volatile__ ( "int {$}3" : );
}

#line 625 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code);

#line 626 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code) {

#line 632 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
    __asm__ __volatile__ ( "int {$}0x29" : : "c" ( code ) );

#line 639 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
    __builtin_unreachable();
}

#line 665 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw.h"
const char * __mingw_get_crt_info(void);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wdupenv_s(wchar_t * * _Buffer, size_t * _BufferSizeInWords, const wchar_t * _VarName);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itow_s(int _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 27 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltow_s(long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultow_s(unsigned long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wgetenv_s(size_t * _ReturnSize, wchar_t * _DstBuf, size_t _DstSizeInWords, const wchar_t * _VarName);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64tow_s(long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64tow_s(unsigned long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 39 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wmakepath_s(wchar_t * _PathResult, size_t _SizeInWords, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 42 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wputenv_s(const wchar_t * _Name, const wchar_t * _Value);

#line 44 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsearchenv_s(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath, size_t _SizeInWords);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsplitpath_s(const wchar_t * _FullPath, wchar_t * _Drive, size_t _DriveSizeInWords, wchar_t * _Dir, size_t _DirSizeInWords, wchar_t * _Filename, size_t _FilenameSizeInWords, wchar_t * _Ext, size_t _ExtSizeInWords);

#line 113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) ___mb_cur_max_func(void);

#line 137 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_set_purecall_handler(_purecall_handler _Handler))(void);

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_get_purecall_handler(void))(void);

#line 141 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_set_invalid_parameter_handler(_invalid_parameter_handler _Handler))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 142 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void (*_get_invalid_parameter_handler(void))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) extern int * _errno(void);

#line 148 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _set_errno(int _Value);

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_errno(int * _Value);

#line 151 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long * __doserrno(void);

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _set_doserrno(unsigned long _Value);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_doserrno(unsigned long * _Value);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * __sys_errlist(void);

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __sys_nerr(void);

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * * __p___argv(void);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __p__fmode(void);

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int * __p___argc(void);

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p___wargv(void);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * __p__pgmptr(void);

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * __p__wpgmptr(void);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_pgmptr(char * * _Value);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_wpgmptr(wchar_t * * _Value);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_fmode(int _Mode);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_fmode(int * _PMode);

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * * * __p__environ(void);

#line 222 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p__wenviron(void);

#line 234 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osplatform(void);

#line 235 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osver(void);

#line 236 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winver(void);

#line 237 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winmajor(void);

#line 238 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winminor(void);

#line 256 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osplatform(unsigned int * _Value);

#line 257 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osver(unsigned int * _Value);

#line 258 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winver(unsigned int * _Value);

#line 259 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winmajor(unsigned int * _Value);

#line 260 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winminor(unsigned int * _Value);

#line 274 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) exit(int _Code) __attribute__((__noreturn__));

#line 275 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) _exit(int _Code) __attribute__((__noreturn__));

#line 277 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) quick_exit(int _Code) __attribute__((__noreturn__));

#line 282 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) _Exit(int) __attribute__((__noreturn__));

#line 291 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) abort(void);

#line 296 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_abort_behavior(unsigned int _Flags, unsigned int _Mask);

#line 300 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) abs(int _X);

#line 301 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) labs(long _X);

#line 304 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) _abs64(long long);

#line 306 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) long long __attribute__((__cdecl__)) _abs64(long long x) {

#line 307 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
    return __builtin_llabs(x);
}

#line 311 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) atexit(void (*)(void));

#line 313 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) at_quick_exit(void (*)(void));

#line 317 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) atof(const char * _String);

#line 318 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) _atof_l(const char * _String, _locale_t _Locale);

#line 320 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) atoi(const char * _Str);

#line 321 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoi_l(const char * _Str, _locale_t _Locale);

#line 322 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) atol(const char * _Str);

#line 323 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _atol_l(const char * _Str, _locale_t _Locale);

#line 326 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * bsearch(const void * _Key, const void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 327 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) qsort(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 329 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned short __attribute__((__cdecl__)) _byteswap_ushort(unsigned short _Short);

#line 330 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _byteswap_ulong(unsigned long _Long);

#line 331 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _byteswap_uint64(unsigned long long _Int64);

#line 332 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
div_t __attribute__((__cdecl__)) div(int _Numerator, int _Denominator);

#line 333 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * getenv(const char * _VarName);

#line 334 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _itoa(int _Value, char * _Dest, int _Radix);

#line 335 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _i64toa(long long _Val, char * _DstBuf, int _Radix);

#line 336 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ui64toa(unsigned long long _Val, char * _DstBuf, int _Radix);

#line 337 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64(const char * _String);

#line 338 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64_l(const char * _String, _locale_t _Locale);

#line 339 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64(const char * _String, char * * _EndPtr, int _Radix);

#line 340 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 341 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64(const char * _String, char * * _EndPtr, int _Radix);

#line 342 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 343 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
ldiv_t __attribute__((__cdecl__)) ldiv(long _Numerator, long _Denominator);

#line 344 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ltoa(long _Value, char * _Dest, int _Radix);

#line 345 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mblen(const char * _Ch, size_t _MaxCount);

#line 346 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mblen_l(const char * _Ch, size_t _MaxCount, _locale_t _Locale);

#line 347 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen(const char * _Str);

#line 348 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen_l(const char * _Str, _locale_t _Locale);

#line 349 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen(const char * _Str, size_t _MaxCount);

#line 350 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen_l(const char * _Str, size_t _MaxCount, _locale_t _Locale);

#line 351 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mbtowc(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes);

#line 352 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mbtowc_l(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes, _locale_t _Locale);

#line 353 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
size_t __attribute__((__cdecl__)) mbstowcs(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount);

#line 354 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstowcs_l(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 355 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) mkstemp(char * template_name);

#line 356 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) rand(void);

#line 357 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_error_mode(int _Mode);

#line 358 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) srand(unsigned int _Seed);

#line 385 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtod(const char * __restrict__ _Str, char * * __restrict__ _EndPtr);

#line 386 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtof(const char * __restrict__ nptr, char * * __restrict__ endptr);

#line 388 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtold(const char * __restrict__, char * * __restrict__);

#line 392 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
extern double __attribute__((__cdecl__)) __attribute__((__nothrow__)) __strtod(const char * __restrict__, char * * __restrict__);

#line 400 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __mingw_strtof(const char * __restrict__, char * * __restrict__);

#line 401 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __mingw_strtod(const char * __restrict__, char * * __restrict__);

#line 402 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __mingw_strtold(const char * __restrict__, char * * __restrict__);

#line 404 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _strtof_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 405 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _strtod_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 406 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) strtol(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 407 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _strtol_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 408 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) strtoul(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 409 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _strtoul_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 412 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) system(const char * _Command);

#line 414 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ultoa(unsigned long _Value, char * _Dest, int _Radix);

#line 415 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) wctomb(char * _MbCh, wchar_t _WCh);

#line 416 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wctomb_l(char * _MbCh, wchar_t _WCh, _locale_t _Locale);

#line 417 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
size_t __attribute__((__cdecl__)) wcstombs(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount);

#line 418 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcstombs_l(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 452 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * calloc(size_t _NumOfElements, size_t _SizeOfElements);

#line 453 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) free(void * _Memory);

#line 454 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * malloc(size_t _Size);

#line 455 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void * realloc(void * _Memory, size_t _NewSize);

#line 456 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _aligned_free(void * _Memory);

#line 457 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_malloc(size_t _Size, size_t _Alignment);

#line 458 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset);

#line 459 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_realloc(void * _Memory, size_t _Size, size_t _Alignment);

#line 460 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 461 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _recalloc(void * _Memory, size_t _Count, size_t _Size);

#line 462 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment);

#line 463 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment, size_t _Offset);

#line 464 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _aligned_msize(void * _Memory, size_t _Alignment, size_t _Offset);

#line 487 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _itow(int _Value, wchar_t * _Dest, int _Radix);

#line 488 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ltow(long _Value, wchar_t * _Dest, int _Radix);

#line 489 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ultow(unsigned long _Value, wchar_t * _Dest, int _Radix);

#line 491 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) __mingw_wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 492 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) __mingw_wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 493 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) __mingw_wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 506 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
double __attribute__((__cdecl__)) wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 507 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
float __attribute__((__cdecl__)) wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 510 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long double __attribute__((__cdecl__)) wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 512 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wcstod_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 513 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _wcstof_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 514 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long __attribute__((__cdecl__)) wcstol(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 515 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wcstol_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 516 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) wcstoul(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 517 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _wcstoul_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 518 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wgetenv(const wchar_t * _VarName);

#line 521 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wsystem(const wchar_t * _Command);

#line 523 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof(const wchar_t * _Str);

#line 524 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof_l(const wchar_t * _Str, _locale_t _Locale);

#line 525 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi(const wchar_t * _Str);

#line 526 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi_l(const wchar_t * _Str, _locale_t _Locale);

#line 527 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol(const wchar_t * _Str);

#line 528 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol_l(const wchar_t * _Str, _locale_t _Locale);

#line 530 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _i64tow(long long _Val, wchar_t * _DstBuf, int _Radix);

#line 531 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ui64tow(unsigned long long _Val, wchar_t * _DstBuf, int _Radix);

#line 532 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64(const wchar_t * _Str);

#line 533 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64_l(const wchar_t * _Str, _locale_t _Locale);

#line 534 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 535 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 536 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 537 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 540 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putenv(const char * _EnvString);

#line 541 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wputenv(const wchar_t * _EnvString);

#line 550 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _fullpath(char * _FullPath, const char * _Path, size_t _SizeInBytes);

#line 555 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 556 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 557 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) char * _gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 558 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl(_CRT_DOUBLE * _Result, char * _Str);

#line 559 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl(_LDOUBLE * _Result, char * _Str);

#line 560 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt(_CRT_FLOAT * _Result, char * _Str);

#line 561 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl_l(_CRT_DOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 562 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl_l(_LDOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 563 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt_l(_CRT_FLOAT * _Result, char * _Str, _locale_t _Locale);

#line 579 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotl(unsigned long, int);

#line 580 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotr(unsigned long, int);

#line 586 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _makepath(char * _Path, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 587 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int (*_onexit(_onexit_t _Func))(void);

#line 591 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 597 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotl64(unsigned long long _Val, int _Shift);

#line 598 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long Value, int Shift);

#line 605 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotr(unsigned int _Val, int _Shift);

#line 606 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotl(unsigned int _Val, int _Shift);

#line 609 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long _Val, int _Shift);

#line 610 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _searchenv(const char * _Filename, const char * _EnvVar, char * _ResultPath);

#line 611 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _splitpath(const char * _FullPath, char * _Drive, char * _Dir, char * _Filename, char * _Ext);

#line 612 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 620 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wfullpath(wchar_t * _FullPath, const wchar_t * _Path, size_t _SizeInWords);

#line 624 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wmakepath(wchar_t * _ResultPath, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 627 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 629 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsearchenv(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath);

#line 630 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsplitpath(const wchar_t * _FullPath, wchar_t * _Drive, wchar_t * _Dir, wchar_t * _Filename, wchar_t * _Ext);

#line 633 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _beep(unsigned _Frequency, unsigned _Duration) __attribute__((__deprecated__));

#line 635 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _seterrormode(int _Mode) __attribute__((__deprecated__));

#line 636 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _sleep(unsigned long _Duration) __attribute__((__deprecated__));

#line 657 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 658 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 659 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 660 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * itoa(int _Val, char * _DstBuf, int _Radix);

#line 661 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ltoa(long _Val, char * _DstBuf, int _Radix);

#line 662 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int __attribute__((__cdecl__)) putenv(const char * _EnvString);

#line 666 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
void __attribute__((__cdecl__)) swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 669 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
char * ultoa(unsigned long _Val, char * _Dstbuf, int _Radix);

#line 670 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
int (*onexit(_onexit_t _Func))(void);

#line 678 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
lldiv_t __attribute__((__cdecl__)) lldiv(long long, long long);

#line 680 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) llabs(long long);

#line 685 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) strtoll(const char * __restrict__, char * * __restrict, int);

#line 686 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
unsigned long long __attribute__((__cdecl__)) strtoull(const char * __restrict__, char * * __restrict__, int);

#line 689 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdlib.h"
long long __attribute__((__cdecl__)) atoll(const char *);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _dupenv_s(char * * _PBuffer, size_t * _PBufferSizeInBytes, const char * _VarName);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) void * bsearch_s(const void * _Key, const void * _Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 25 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) getenv_s(size_t * _ReturnSize, char * _DstBuf, rsize_t _DstSize, const char * _VarName);

#line 27 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix);

#line 29 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64toa_s(long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64toa_s(unsigned long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltoa_s(long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) mbstowcs_s(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _mbstowcs_s_l(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount, _locale_t _Locale);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultoa_s(unsigned long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 39 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wctomb_s(int * _SizeConverted, char * _MbCh, rsize_t _SizeInBytes, wchar_t _WCh);

#line 40 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wctomb_s_l(int * _SizeConverted, char * _MbCh, size_t _SizeInBytes, wchar_t _WCh, _locale_t _Locale);

#line 41 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcstombs_s(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes);

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcstombs_s_l(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes, _locale_t _Locale);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ecvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDights, int * _PtDec, int * _PtSign);

#line 48 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _fcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _gcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDigits);

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _makepath_s(char * _PathResult, size_t _Size, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _putenv_s(const char * _Name, const char * _Value);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _searchenv_s(const char * _Filename, const char * _EnvVar, char * _ResultPath, size_t _SizeInBytes);

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _splitpath_s(const char * _FullPath, char * _Drive, size_t _DriveSize, char * _Dir, size_t _DirSize, char * _Filename, size_t _FilenameSize, char * _Ext, size_t _ExtSize);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdlib_s.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) qsort_s(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);
__attribute__((__dllimport__)) unsigned int * __p__amblksiz(void);

#line 129 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_malloc(size_t _Size, size_t _Alignment);

#line 130 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void __mingw_aligned_free(void * _Memory);

#line 131 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 132 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_offset_malloc(size_t, size_t, size_t);

#line 133 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
void * __mingw_aligned_realloc(void * _Memory, size_t _Size, size_t _Offset);

#line 134 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
size_t __mingw_aligned_msize(void * memblock, size_t alignment, size_t offset);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
static __inline__ void * _mm_malloc(size_t __size, size_t __align) {

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    if (__align == 1) {

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
        return malloc(__size);
    }

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    if (!(__align & (__align - 1)) && __align < sizeof(void *)) 

#line 39 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
        __align = sizeof(void *);

#line 41 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    void * __mallocedMemory;

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    __mallocedMemory = __mingw_aligned_malloc(__size, __align);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    return __mallocedMemory;
}

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
static __inline__ void __attribute__((__always_inline__,__nodebug__)) _mm_free(void * __p) {

#line 58 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\mm_malloc.h"
    __mingw_aligned_free(__p);
}

#line 144 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _resetstkoflw(void);

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _set_malloc_crt_max_wait(unsigned long _NewValue);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) void * _expand(void * _Memory, size_t _NewSize);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _msize(void * _Memory);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _get_sbh_threshold(void);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_sbh_threshold(size_t _NewValue);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_amblksiz(size_t _Value);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_amblksiz(size_t * _Value);

#line 171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapadd(void * _Memory, size_t _Size);

#line 172 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapchk(void);

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapmin(void);

#line 174 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapset(unsigned int _Fill);

#line 175 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapwalk(_HEAPINFO * _EntryInfo);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _heapused(size_t * _Used, size_t * _Commit);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _get_heap_handle(void);

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
static __inline void * _MarkAllocaS(void * _Ptr, unsigned int _Marker) {

#line 191 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
    if (_Ptr) {

#line 192 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
        *((unsigned int *)_Ptr) = _Marker;

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
        _Ptr = (char *)_Ptr + 16;
    }

#line 195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
    return _Ptr;
}

#line 218 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
static __inline void __attribute__((__cdecl__)) _freea(void * _Memory) {

#line 219 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
    unsigned int _Marker;

#line 220 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
    if (_Memory) {

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
        _Memory = (char *)_Memory - 16;

#line 222 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
        _Marker = *(unsigned int *)_Memory;

#line 223 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
        if (_Marker == 0xDDDD) {

#line 224 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\malloc.h"
            free(_Memory);
        }
    }
}

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) void * _memccpy(void * _Dst, const void * _Src, int _Val, size_t _MaxCount);

#line 46 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memchr(const void * _Buf, int _Val, size_t _MaxCount);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 48 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp_l(const void * _Buf1, const void * _Buf2, size_t _Size, _locale_t _Locale);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) memcmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memcpy(void * __restrict__ _Dst, const void * __restrict__ _Src, size_t _Size);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memcpy_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * mempcpy(void * _Dst, const void * _Src, size_t _Size);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memset(void * _Dst, int _Val, size_t _Size);

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memccpy(void * _Dst, const void * _Src, int _Val, size_t _Size);

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 59 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * _strset(char * _Str, int _Val);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * _strset_l(char * _Str, int _Val, _locale_t _Locale);

#line 61 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strcpy(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 62 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strcat(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcmp(const char * _Str1, const char * _Str2);

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strlen(const char * _Str);

#line 65 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strnlen(const char * _Str, size_t _MaxCount);

#line 66 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
void * memmove(void * _Dst, const void * _Src, size_t _Size);

#line 71 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strdup(const char * _Src);

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strchr(const char * _Str, int _Val);

#line 76 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp(const char * _Str1, const char * _Str2);

#line 77 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcmpi(const char * _Str1, const char * _Str2);

#line 78 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 79 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcoll(const char * _Str1, const char * _Str2);

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 81 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll(const char * _Str1, const char * _Str2);

#line 82 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 86 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strcspn(const char * _Str, const char * _Control);

#line 88 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strerror(const char * _ErrMsg);

#line 89 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strerror(int);

#line 90 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strlwr(char * _String);

#line 91 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strlwr_l(char * _String, _locale_t _Locale);

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strncat(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 93 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strncmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 95 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 96 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strncpy(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 97 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strnset(char * _Str, int _Val, size_t _MaxCount);

#line 98 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strnset_l(char * str, int c, size_t count, _locale_t _Locale);

#line 99 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strpbrk(const char * _Str, const char * _Control);

#line 100 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strrchr(const char * _Str, int _Ch);

#line 101 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strrev(char * _Str);

#line 102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strspn(const char * _Str, const char * _Control);

#line 103 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strstr(const char * _Str, const char * _SubStr);

#line 104 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strtok(char * __restrict__ _Str, const char * __restrict__ _Delim);

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strtok_r(char * __restrict__ _Str, const char * __restrict__ _Delim, char * * __restrict__ __last);

#line 109 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strupr(char * _String);

#line 110 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) char * _strupr_l(char * _String, _locale_t _Locale);

#line 111 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) strxfrm(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount);

#line 112 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _strxfrm_l(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 119 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strdup(const char * _Src);

#line 123 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcmpi(const char * _Str1, const char * _Str2);

#line 124 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) stricmp(const char * _Str1, const char * _Str2);

#line 125 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strlwr(char * _Str);

#line 126 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strnicmp(const char * _Str1, const char * _Str, size_t _MaxCount);

#line 127 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strncasecmp(const char *, const char *, size_t);

#line 128 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) strcasecmp(const char *, const char *);

#line 136 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strnset(char * _Str, int _Val, size_t _MaxCount);

#line 137 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strrev(char * _Str);

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strset(char * _Str, int _Val);

#line 139 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
char * strupr(char * _Str);

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsdup(const wchar_t * _Str);

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcscat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcschr(const wchar_t * _Str, wchar_t _Ch);

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcscmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcscpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 157 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcscspn(const wchar_t * _Str, const wchar_t * _Control);

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcslen(const wchar_t * _Str);

#line 159 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsnlen(const wchar_t * _Src, size_t _MaxCount);

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsncat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsncmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsncpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * _wcsncpy_l(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count, _locale_t _Locale);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcspbrk(const wchar_t * _Str, const wchar_t * _Control);

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsrchr(const wchar_t * _Str, wchar_t _Ch);

#line 166 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsspn(const wchar_t * _Str, const wchar_t * _Control);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsstr(const wchar_t * _Str, const wchar_t * _SubStr);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim, wchar_t * * __restrict__ _Ptr);

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * _wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcserror(int _ErrNum);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * __wcserror(const wchar_t * _Str);

#line 178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 179 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 180 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 181 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsrev(wchar_t * _Str);

#line 184 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsset(wchar_t * _Str, wchar_t _Val);

#line 185 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr(wchar_t * _String);

#line 186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr_l(wchar_t * _String, _locale_t _Locale);

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr(wchar_t * _String);

#line 188 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr_l(wchar_t * _String, _locale_t _Locale);

#line 189 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
size_t __attribute__((__cdecl__)) wcsxfrm(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount);

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcsxfrm_l(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 191 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcscoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 192 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcscoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 194 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 197 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 198 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 205 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsdup(const wchar_t * _Str);

#line 210 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 211 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 212 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 213 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsrev(wchar_t * _Str);

#line 214 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsset(wchar_t * _Str, wchar_t _Val);

#line 215 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcslwr(wchar_t * _Str);

#line 216 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
wchar_t * wcsupr(wchar_t * _Str);

#line 217 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\string.h"
int __attribute__((__cdecl__)) wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 24 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strset_s(char * _Dst, size_t _DstSize, int _Value);

#line 26 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strerror_s(char * _Buf, size_t _SizeInBytes, const char * _ErrMsg);

#line 28 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) strerror_s(char * _Buf, size_t _SizeInBytes, int _ErrNum);

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s(char * _Str, size_t _Size);

#line 32 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strnset_s(char * _Str, size_t _Size, int _Val, size_t _MaxCount);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s(char * _Str, size_t _Size);

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 41 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncat_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 43 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncat_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcpy_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncpy_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncpy_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) char * strtok_s(char * _Str, const char * _Delim, char * * _Context);

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) char * _strtok_s_l(char * _Str, const char * _Delim, char * * _Context, _locale_t _Locale);

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcat_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) size_t __attribute__((__cdecl__)) strnlen_s(const char * _src, size_t _count) {

#line 57 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
    return _src ? strnlen(_src, _count) : 0;
}

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memmove_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) wchar_t * wcstok_s(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context);

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcserror_s(wchar_t * _Buf, size_t _SizeInWords, int _ErrNum);

#line 66 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) __wcserror_s(wchar_t * _Buffer, size_t _SizeInWords, const wchar_t * _ErrMsg);

#line 67 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s(wchar_t * _Dst, size_t _DstSizeInWords, wchar_t _Val, size_t _MaxCount);

#line 68 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s(wchar_t * _Str, size_t _SizeInWords, wchar_t _Val);

#line 69 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s(wchar_t * _Str, size_t _SizeInWords);

#line 71 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s_l(wchar_t * _Str, size_t _SizeInWords, _locale_t _Locale);

#line 73 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s(wchar_t * _Str, size_t _Size);

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s_l(wchar_t * _Str, size_t _Size, _locale_t _Locale);

#line 78 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscpy_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscat_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncat_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncat_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncpy_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 89 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncpy_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 91 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) wchar_t * _wcstok_s_l(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context, _locale_t _Locale);

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, _locale_t _Locale);

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, size_t _Count, _locale_t _Locale);

#line 97 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) size_t __attribute__((__cdecl__)) wcsnlen_s(const wchar_t * _src, size_t _count) {

#line 98 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\string_s.h"
    return _src ? wcsnlen(_src, _count) : 0;
}

#line 15 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int MixedFields__tag_val(MixedFields * this) {
    return (int)this->tag;
}

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
void MixedFields__set_id(MixedFields * this, int v) {
    this->id = v;
}

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Outer__compute_total(Outer * this) {

#line 32 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return this->first.a + this->first.b + this->second.a + this->second.b;
}

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
Color make_color(unsigned char r, unsigned char g, unsigned char b) {

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Color c;

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    c.r = r;
    c.g = g;
    c.b = b;

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return c;
}

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
Color brighten(Color c, int amount) {

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Color result;

#line 57 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    result.r = (unsigned char)(c.r + amount > 255 ? 255 : c.r + amount);

#line 58 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    result.g = (unsigned char)(c.g + amount > 255 ? 255 : c.g + amount);

#line 59 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    result.b = (unsigned char)(c.b + amount > 255 ? 255 : c.b + amount);

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return result;
}

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
unsigned char Color__luminance(Color * this) {

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return (unsigned char)((this->r * 77 + this->g * 150 + this->b * 29) / 256);
}

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 77 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
Rect make_rect(int x, int y, int w, int h) {

#line 78 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Rect r;

#line 79 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return r;
}

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
Rect translate_rect(Rect r, int dx, int dy) {

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Rect out;

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    out.x = r.x + dx;

#line 86 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    out.y = r.y + dy;

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    out.w = r.w;

#line 88 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    out.h = r.h;

#line 89 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return out;
}

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int Rect__contains(Rect * this, int px, int py) {

#line 93 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return px >= this->x && px < this->x + this->w && py >= this->y && py < this->y + this->h;
}

#line 122 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
int main(void) {

#line 124 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    MixedFields mf;

#line 125 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    int val = 42;

#line 126 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    mf.id = 10;

#line 127 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    mf.tag = 'A';

#line 128 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    mf.ptr = &val;

#line 129 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    mf.name[0] = 't';
    mf.name[1] = 'e';
    mf.name[2] = 's';
    mf.name[3] = 't';

#line 130 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    mf.name[4] = '\0';

#line 131 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (mf.id != 10) 
        return 1;

#line 132 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (MixedFields__tag_val(&mf) != (int)'A') 
        return 2;

#line 133 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (mf.ptr[0] != 42) 
        return 3;

#line 134 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (mf.name[0] != 't') 
        return 4;

#line 135 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    MixedFields__set_id(&mf, 99);

#line 136 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (mf.id != 99) 
        return 5;

#line 139 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Outer o;

#line 140 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    o.first.a = 1;
    o.first.b = 2;

#line 141 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    o.second.a = 3;
    o.second.b = 4;

#line 142 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    o.total = Outer__compute_total(&o);

#line 143 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (o.total != 10) 
        return 6;

#line 144 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (o.first.a + o.second.b != 5) 
        return 7;

#line 147 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    struct BitFlags bf;

#line 148 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    bf.x = 12;

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    bf.y = 200;

#line 150 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    bf.z = 1;

#line 151 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (bf.x != 12) 
        return 8;

#line 152 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (bf.y != 200) 
        return 9;

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (bf.z != 1) 
        return 10;

#line 154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    bf.x = 31;

#line 155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (bf.x != 15) 
        return 11;

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    bf.y = 300;

#line 157 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (bf.y != 44) 
        return 12;

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Color c1 = make_color(100, 150, 200);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Color c2 = c1;

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c2.r != 100) 
        return 13;

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c2.g != 150) 
        return 14;

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c2.b != 200) 
        return 15;

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    c2.r = 0;

#line 166 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c1.r != 100) 
        return 16;

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Color c3 = brighten(c1, 50);

#line 168 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c3.r != 150) 
        return 17;

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c3.g != 200) 
        return 18;

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (c3.b != 250) 
        return 19;

#line 171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    unsigned char lum = Color__luminance(&c1);

#line 172 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (lum == 0) 
        return 20;

#line 175 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Rect r1 = make_rect(10, 20, 30, 40);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (Rect__area(&r1) != 1200) 
        return 21;

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (!Rect__contains(&r1, 15, 25)) 
        return 22;

#line 178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (Rect__contains(&r1, 5, 25)) 
        return 23;

#line 179 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    Rect r2 = translate_rect(r1, 5, -10);

#line 180 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (r2.x != 15) 
        return 24;

#line 181 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (r2.y != 10) 
        return 25;

#line 182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (r2.w != 30) 
        return 26;

#line 183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (r2.h != 40) 
        return 27;

#line 186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    struct TaggedValue tv;

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    tv.kind = 0;

#line 188 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    tv.i_val = 42;

#line 189 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (tv.i_val != 42) 
        return 28;

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    tv.kind = 1;

#line 191 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    tv.d_val = 3.14;

#line 192 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (tv.d_val < 3.13 || tv.d_val > 3.15) 
        return 29;

#line 194 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    struct Coords co;

#line 195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    co.x = 1.0f;

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    co.y = 2.0f;

#line 197 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (co.data[0] < 0.99f || co.data[0] > 1.01f) 
        return 30;

#line 198 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (co.data[1] < 1.99f || co.data[1] > 2.01f) 
        return 31;

#line 199 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    co.data[0] = 5.0f;

#line 200 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    co.data[1] = 6.0f;

#line 201 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (co.x < 4.99f || co.x > 5.01f) 
        return 32;

#line 202 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (co.y < 5.99f || co.y > 6.01f) 
        return 33;

#line 205 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    struct FlexHeader * fh = (struct FlexHeader *)malloc(sizeof(struct FlexHeader) + 3 * sizeof(int));

#line 206 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    fh->count = 3;

#line 207 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    fh->items[0] = 10;

#line 208 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    fh->items[1] = 20;

#line 209 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    fh->items[2] = 30;

#line 210 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (fh->count != 3) 
        return 34;

#line 211 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (fh->items[0] != 10) 
        return 35;

#line 212 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (fh->items[1] != 20) 
        return 36;

#line 213 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    if (fh->items[2] != 30) 
        return 37;

#line 214 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    free(fh);

#line 216 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\codegen\\struct_layout.ce"
    return 0;
}
