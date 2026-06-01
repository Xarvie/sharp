
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

#line 16 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_stdio_config.h"
unsigned long long * __local_stdio_printf_options(void);

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_stdio_config.h"
unsigned long long * __local_stdio_scanf_options(void);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
struct _iobuf {
    void * _Placeholder;
};

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
typedef struct _iobuf FILE;

#line 5 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw_off_t.h"
typedef long _off_t;

#line 7 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw_off_t.h"
typedef long off32_t;

#line 13 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw_off_t.h"
__extension__ typedef long long _off64_t;

#line 15 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw_off_t.h"
__extension__ typedef long long off64_t;

#line 26 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\_mingw_off_t.h"
typedef off32_t off_t;

#line 101 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * __acrt_iob_func(unsigned index);

#line 103 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * __iob_func(void);

#line 112 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__extension__ typedef long long fpos_t;

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsscanf(const char * __restrict__ _Str, const char * __restrict__ Format, va_list argp);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_scanf(const char * __restrict__ _Format, ...);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vscanf(const char * __restrict__ Format, va_list argp);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfscanf(FILE * __restrict__ fp, const char * __restrict__ Format, va_list argp);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnprintf(char * __restrict__ _DstBuf, size_t _MaxCount, const char * __restrict__ _Format, va_list _ArgList);

#line 181 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snprintf(char * __restrict__ s, size_t n, const char * __restrict__ format, ...);

#line 184 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_printf(const char * __restrict__, ...) __attribute__((__nothrow__));

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vprintf(const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fprintf(FILE * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sprintf(char * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 199 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsprintf(char * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 202 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_asprintf(char * * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 205 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_vasprintf(char * * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 209 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...) __asm__("sscanf");

#line 213 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsscanf(const char * __restrict__ _Str, const char * __restrict__ _Format, va_list argp) __asm__("vsscanf");

#line 217 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_scanf(const char * __restrict__ _Format, ...) __asm__("scanf");

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vscanf(const char * __restrict__ _Format, va_list argp) __asm__("vscanf");

#line 225 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...) __asm__("fscanf");

#line 229 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list argp) __asm__("vfscanf");

#line 234 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_printf(const char * __restrict__, ...) __asm__("printf") __attribute__((__nothrow__));

#line 238 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vprintf(const char * __restrict__, va_list) __asm__("vprintf") __attribute__((__nothrow__));

#line 242 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fprintf(FILE * __restrict__, const char * __restrict__, ...) __asm__("fprintf") __attribute__((__nothrow__));

#line 246 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __asm__("vfprintf") __attribute__((__nothrow__));

#line 251 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sprintf(char * __restrict__, const char * __restrict__, ...) __asm__("sprintf") __attribute__((__nothrow__));

#line 255 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsprintf(char * __restrict__, const char * __restrict__, va_list) __asm__("vsprintf") __attribute__((__nothrow__));

#line 259 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snprintf(char * __restrict__, size_t, const char * __restrict__, ...) __asm__("snprintf") __attribute__((__nothrow__));

#line 263 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnprintf(char * __restrict__, size_t, const char * __restrict__, va_list) __asm__("vsnprintf") __attribute__((__nothrow__));

#line 267 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf(unsigned long long options, char * str, size_t len, const char * format, _locale_t locale, va_list valist);

#line 268 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 269 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsscanf(unsigned long long options, const char * input, size_t length, const char * format, _locale_t locale, va_list valist);

#line 270 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfscanf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 448 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 450 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) printf(const char * __restrict__ _Format, ...);

#line 452 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, ...);

#line 455 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vfprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list _ArgList);

#line 457 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) vprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 459 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vsprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, va_list _Args);

#line 463 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 466 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) scanf(const char * __restrict__ _Format, ...);

#line 469 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 478 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vfscanf(FILE * __stream, const char * __format, __builtin_va_list __local_argv);

#line 481 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vsscanf(const char * __restrict__ __source, const char * __restrict__ __format, __builtin_va_list __local_argv);

#line 483 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int vscanf(const char * __format, __builtin_va_list __local_argv);

#line 535 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _filbuf(FILE * _File);

#line 536 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flsbuf(int _Ch, FILE * _File);

#line 540 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _fsopen(const char * _Filename, const char * _Mode, int _ShFlag);

#line 542 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) clearerr(FILE * _File);

#line 543 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fclose(FILE * _File);

#line 544 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fcloseall(void);

#line 548 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _fdopen(int _FileHandle, const char * _Mode);

#line 550 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) feof(FILE * _File);

#line 551 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) ferror(FILE * _File);

#line 552 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fflush(FILE * _File);

#line 553 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetc(FILE * _File);

#line 554 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetchar(void);

#line 555 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetpos(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 556 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetpos64(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 557 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * fgets(char * __restrict__ _Buf, int _MaxCount, FILE * __restrict__ _File);

#line 558 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fileno(FILE * _File);

#line 566 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) char * _tempnam(const char * _DirName, const char * _FilePrefix);

#line 570 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flushall(void);

#line 571 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode);

#line 572 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fopen64(const char * __restrict__ filename, const char * __restrict__ mode);

#line 573 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputc(int _Ch, FILE * _File);

#line 574 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputchar(int _Ch);

#line 575 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputs(const char * __restrict__ _Str, FILE * __restrict__ _File);

#line 576 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
size_t __attribute__((__cdecl__)) fread(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 577 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * freopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode, FILE * __restrict__ _File);

#line 578 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fsetpos(FILE * _File, const fpos_t * _Pos);

#line 579 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fsetpos64(FILE * _File, const fpos_t * _Pos);

#line 580 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fseek(FILE * _File, long _Offset, int _Origin);

#line 581 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
long __attribute__((__cdecl__)) ftell(FILE * _File);

#line 585 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64(FILE * _File, long long _Offset, int _Origin);

#line 586 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64(FILE * _File);

#line 622 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
size_t __attribute__((__cdecl__)) fwrite(const void * __restrict__ _Str, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 623 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getc(FILE * _File);

#line 624 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getchar(void);

#line 625 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getmaxstdio(void);

#line 626 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * gets(char * _Buffer) __attribute__((__warning__("Using gets() is always unsafe - use fgets() instead")));

#line 628 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _getw(FILE * _File);

#line 631 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 634 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _pclose(FILE * _File);

#line 635 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _popen(const char * _Command, const char * _Mode);

#line 641 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putc(int _Ch, FILE * _File);

#line 642 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putchar(int _Ch);

#line 643 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) puts(const char * _Str);

#line 644 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putw(int _Word, FILE * _File);

#line 647 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) remove(const char * _Filename);

#line 648 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) rename(const char * _OldFilename, const char * _NewFilename);

#line 649 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _unlink(const char * _Filename);

#line 651 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) unlink(const char * _Filename);

#line 654 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) rewind(FILE * _File);

#line 655 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _rmtmp(void);

#line 656 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) setbuf(FILE * __restrict__ _File, char * __restrict__ _Buffer);

#line 657 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _setmaxstdio(int _Max);

#line 658 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_output_format(unsigned int _Format);

#line 659 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _get_output_format(void);

#line 660 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) setvbuf(FILE * __restrict__ _File, char * __restrict__ _Buf, int _Mode, size_t _Size);

#line 664 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) _scprintf(const char * __restrict__ _Format, ...);

#line 666 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snscanf(const char * __restrict__ _Src, size_t _MaxCount, const char * __restrict__ _Format, ...);

#line 676 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) __attribute__((__dllimport__)) int __attribute__((__cdecl__)) _vscprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 677 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * tmpfile(void);

#line 678 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * tmpnam(char * _Buffer);

#line 679 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) ungetc(int _Ch, FILE * _File);

#line 683 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _vsnprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, va_list _Args);

#line 685 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, ...);

#line 741 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int vsnprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, va_list __local_argv);

#line 744 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int snprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, ...);

#line 905 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_printf_count_output(int _Value);

#line 906 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _get_printf_count_output(void);

#line 912 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 914 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vswscanf(const wchar_t * __restrict__ _Str, const wchar_t * __restrict__ Format, va_list argp);

#line 916 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wscanf(const wchar_t * __restrict__ _Format, ...);

#line 918 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwscanf(const wchar_t * __restrict__ Format, va_list argp);

#line 920 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 922 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwscanf(FILE * __restrict__ fp, const wchar_t * __restrict__ Format, va_list argp);

#line 925 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 927 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wprintf(const wchar_t * __restrict__ _Format, ...);

#line 929 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 931 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 933 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 935 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 937 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...);

#line 939 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 942 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...) __asm__("swscanf");

#line 945 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vswscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, va_list) __asm__("vswscanf");

#line 948 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wscanf(const wchar_t * __restrict__ _Format, ...) __asm__("wscanf");

#line 951 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwscanf(const wchar_t * __restrict__ _Format, va_list) __asm__("vwscanf");

#line 954 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...) __asm__("fwscanf");

#line 957 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list) __asm__("vfwscanf");

#line 961 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 965 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wprintf(const wchar_t * __restrict__ _Format, ...) __asm__("wprintf");

#line 968 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vfwprintf");

#line 971 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vwprintf");

#line 974 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("swprintf");

#line 977 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vswprintf");

#line 980 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("snwprintf");

#line 983 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vsnwprintf");

#line 987 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf(unsigned long long options, wchar_t * str, size_t len, const wchar_t * format, _locale_t locale, va_list valist);

#line 988 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 989 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswscanf(unsigned long long options, const wchar_t * input, size_t length, const wchar_t * format, _locale_t locale, va_list valist);

#line 990 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwscanf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 1102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1104 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 1106 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) wscanf(const wchar_t * __restrict__ _Format, ...);

#line 1108 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int vfwscanf(FILE * __stream, const wchar_t * __format, va_list __local_argv);

#line 1111 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int vswscanf(const wchar_t * __restrict__ __source, const wchar_t * __restrict__ __format, va_list __local_argv);

#line 1113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int vwscanf(const wchar_t * __format, va_list __local_argv);

#line 1115 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1116 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) wprintf(const wchar_t * __restrict__ _Format, ...);

#line 1117 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1118 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1150 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfsopen(const wchar_t * _Filename, const wchar_t * _Mode, int _ShFlag);

#line 1153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) fgetwc(FILE * _File);

#line 1154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwchar(void);

#line 1155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) fputwc(wchar_t _Ch, FILE * _File);

#line 1156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwchar(wchar_t _Ch);

#line 1157 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) getwc(FILE * _File);

#line 1158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) getwchar(void);

#line 1159 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) putwc(wchar_t _Ch, FILE * _File);

#line 1160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) putwchar(wchar_t _Ch);

#line 1161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) ungetwc(wint_t _Ch, FILE * _File);

#line 1162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wchar_t * fgetws(wchar_t * __restrict__ _Dst, int _SizeInWords, FILE * __restrict__ _File);

#line 1163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputws(const wchar_t * __restrict__ _Str, FILE * __restrict__ _File);

#line 1164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _getws(wchar_t * _String);

#line 1165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putws(const wchar_t * _Str);

#line 1178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _snwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1179 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _vsnwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) swprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vswprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1185 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 1186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vsnwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, va_list arg);

#line 1249 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtempnam(const wchar_t * _Directory, const wchar_t * _FilePrefix);

#line 1253 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _snwscanf(const wchar_t * __restrict__ _Src, size_t _MaxCount, const wchar_t * __restrict__ _Format, ...);

#line 1254 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfdopen(int _FileHandle, const wchar_t * _Mode);

#line 1255 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode);

#line 1256 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfreopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode, FILE * __restrict__ _OldFile);

#line 1260 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 1262 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wpopen(const wchar_t * _Command, const wchar_t * _Mode);

#line 1267 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wremove(const wchar_t * _Filename);

#line 1268 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtmpnam(wchar_t * _Buffer);

#line 1270 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwc_nolock(FILE * _File);

#line 1271 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwc_nolock(wchar_t _Ch, FILE * _File);

#line 1272 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _ungetwc_nolock(wint_t _Ch, FILE * _File);

#line 1297 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetc_nolock(FILE * _File);

#line 1298 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputc_nolock(int _Char, FILE * _File);

#line 1299 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getc_nolock(FILE * _File);

#line 1300 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putc_nolock(int _Char, FILE * _File);

#line 1312 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _lock_file(FILE * _File);

#line 1313 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _unlock_file(FILE * _File);

#line 1315 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fclose_nolock(FILE * _File);

#line 1316 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fflush_nolock(FILE * _File);

#line 1317 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 1318 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseek_nolock(FILE * _File, long _Offset, int _Origin);

#line 1319 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _ftell_nolock(FILE * _File);

#line 1320 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64_nolock(FILE * _File, long long _Offset, int _Origin);

#line 1321 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64_nolock(FILE * _File);

#line 1322 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fwrite_nolock(const void * __restrict__ _DstBuf, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 1323 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _ungetc_nolock(int _Ch, FILE * _File);

#line 1334 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * tempnam(const char * _Directory, const char * _FilePrefix);

#line 1338 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fcloseall(void);

#line 1339 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fdopen(int _FileHandle, const char * _Format);

#line 1340 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetchar(void);

#line 1341 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fileno(FILE * _File);

#line 1342 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) flushall(void);

#line 1343 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputchar(int _Ch);

#line 1344 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getw(FILE * _File);

#line 1345 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putw(int _Ch, FILE * _File);

#line 1346 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) rmtmp(void);

#line 1363 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __mingw_str_wide_utf8(const wchar_t * const wptr, char * * mbptr, size_t * buflen);

#line 1377 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __mingw_str_utf8_wide(const char * const mbptr, wchar_t * * wptr, size_t * buflen);

#line 1386 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) __mingw_str_free(void * ptr);

#line 1393 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnl(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1394 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnle(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1395 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlp(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1396 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlpe(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1397 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnv(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1398 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnve(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1399 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvp(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1400 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvpe(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1417 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnv(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1418 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnve(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 1419 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvp(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1420 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvpe(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 29 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) clearerr_s(FILE * _File);

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
size_t __attribute__((__cdecl__)) fread_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_s(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_p(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnprintf_s(unsigned long long _Options, char * _Str, size_t _Len, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_s(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_p(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 589 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) fopen_s(FILE * * _File, const char * _Filename, const char * _Mode);

#line 590 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) freopen_s(FILE * * _File, const char * _Filename, const char * _Mode, FILE * _Stream);

#line 592 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) char * gets_s(char *, rsize_t);

#line 595 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpfile_s(FILE * * _File);

#line 597 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpnam_s(char *, rsize_t);

#line 603 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) wchar_t * _getws_s(wchar_t * _Str, size_t _SizeInWords);

#line 607 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 608 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnwprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 609 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf_s(unsigned long long _Options, FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 867 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode);

#line 868 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfreopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode, FILE * _OldFile);

#line 870 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wtmpnam_s(wchar_t * _DstBuf, size_t _SizeInWords);

#line 912 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 12 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_list.h"
typedef __builtin_va_list va_list;

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

#line 16 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_stdio_config.h"
unsigned long long * __local_stdio_printf_options(void);

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\corecrt_stdio_config.h"
unsigned long long * __local_stdio_scanf_options(void);

#line 101 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * __acrt_iob_func(unsigned index);

#line 103 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * __iob_func(void);

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsscanf(const char * __restrict__ _Str, const char * __restrict__ Format, va_list argp);

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_scanf(const char * __restrict__ _Format, ...);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vscanf(const char * __restrict__ Format, va_list argp);

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfscanf(FILE * __restrict__ fp, const char * __restrict__ Format, va_list argp);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnprintf(char * __restrict__ _DstBuf, size_t _MaxCount, const char * __restrict__ _Format, va_list _ArgList);

#line 181 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snprintf(char * __restrict__ s, size_t n, const char * __restrict__ format, ...);

#line 184 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_printf(const char * __restrict__, ...) __attribute__((__nothrow__));

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vprintf(const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fprintf(FILE * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sprintf(char * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 199 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsprintf(char * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 202 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_asprintf(char * * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 205 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_vasprintf(char * * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 209 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...) __asm__("sscanf");

#line 213 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsscanf(const char * __restrict__ _Str, const char * __restrict__ _Format, va_list argp) __asm__("vsscanf");

#line 217 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_scanf(const char * __restrict__ _Format, ...) __asm__("scanf");

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vscanf(const char * __restrict__ _Format, va_list argp) __asm__("vscanf");

#line 225 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...) __asm__("fscanf");

#line 229 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list argp) __asm__("vfscanf");

#line 234 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_printf(const char * __restrict__, ...) __asm__("printf") __attribute__((__nothrow__));

#line 238 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vprintf(const char * __restrict__, va_list) __asm__("vprintf") __attribute__((__nothrow__));

#line 242 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fprintf(FILE * __restrict__, const char * __restrict__, ...) __asm__("fprintf") __attribute__((__nothrow__));

#line 246 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __asm__("vfprintf") __attribute__((__nothrow__));

#line 251 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sprintf(char * __restrict__, const char * __restrict__, ...) __asm__("sprintf") __attribute__((__nothrow__));

#line 255 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsprintf(char * __restrict__, const char * __restrict__, va_list) __asm__("vsprintf") __attribute__((__nothrow__));

#line 259 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snprintf(char * __restrict__, size_t, const char * __restrict__, ...) __asm__("snprintf") __attribute__((__nothrow__));

#line 263 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnprintf(char * __restrict__, size_t, const char * __restrict__, va_list) __asm__("vsnprintf") __attribute__((__nothrow__));

#line 267 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf(unsigned long long options, char * str, size_t len, const char * format, _locale_t locale, va_list valist);

#line 268 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 269 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsscanf(unsigned long long options, const char * input, size_t length, const char * format, _locale_t locale, va_list valist);

#line 270 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfscanf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 448 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 450 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) printf(const char * __restrict__ _Format, ...);

#line 452 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, ...);

#line 455 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vfprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list _ArgList);

#line 457 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) vprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 459 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vsprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, va_list _Args);

#line 463 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 466 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) scanf(const char * __restrict__ _Format, ...);

#line 469 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 478 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vfscanf(FILE * __stream, const char * __format, __builtin_va_list __local_argv);

#line 481 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vsscanf(const char * __restrict__ __source, const char * __restrict__ __format, __builtin_va_list __local_argv);

#line 483 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int vscanf(const char * __format, __builtin_va_list __local_argv);

#line 535 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _filbuf(FILE * _File);

#line 536 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flsbuf(int _Ch, FILE * _File);

#line 540 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _fsopen(const char * _Filename, const char * _Mode, int _ShFlag);

#line 542 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) clearerr(FILE * _File);

#line 543 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fclose(FILE * _File);

#line 544 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fcloseall(void);

#line 548 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _fdopen(int _FileHandle, const char * _Mode);

#line 550 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) feof(FILE * _File);

#line 551 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) ferror(FILE * _File);

#line 552 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fflush(FILE * _File);

#line 553 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetc(FILE * _File);

#line 554 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetchar(void);

#line 555 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetpos(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 556 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetpos64(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 557 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * fgets(char * __restrict__ _Buf, int _MaxCount, FILE * __restrict__ _File);

#line 558 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fileno(FILE * _File);

#line 566 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) char * _tempnam(const char * _DirName, const char * _FilePrefix);

#line 570 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flushall(void);

#line 571 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode);

#line 572 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fopen64(const char * __restrict__ filename, const char * __restrict__ mode);

#line 573 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputc(int _Ch, FILE * _File);

#line 574 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputchar(int _Ch);

#line 575 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputs(const char * __restrict__ _Str, FILE * __restrict__ _File);

#line 576 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
size_t __attribute__((__cdecl__)) fread(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 577 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * freopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode, FILE * __restrict__ _File);

#line 578 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fsetpos(FILE * _File, const fpos_t * _Pos);

#line 579 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fsetpos64(FILE * _File, const fpos_t * _Pos);

#line 580 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fseek(FILE * _File, long _Offset, int _Origin);

#line 581 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
long __attribute__((__cdecl__)) ftell(FILE * _File);

#line 585 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64(FILE * _File, long long _Offset, int _Origin);

#line 586 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64(FILE * _File);

#line 588 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int fseeko(FILE * _File, _off_t _Offset, int _Origin) {

#line 589 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return fseek(_File, _Offset, _Origin);
}

#line 591 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int fseeko64(FILE * _File, _off64_t _Offset, int _Origin) {

#line 592 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return _fseeki64(_File, _Offset, _Origin);
}

#line 594 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ _off_t ftello(FILE * _File) {

#line 595 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return ftell(_File);
}

#line 597 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ _off64_t ftello64(FILE * _File) {

#line 598 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return _ftelli64(_File);
}

#line 622 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
size_t __attribute__((__cdecl__)) fwrite(const void * __restrict__ _Str, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 623 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getc(FILE * _File);

#line 624 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getchar(void);

#line 625 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getmaxstdio(void);

#line 626 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * gets(char * _Buffer) __attribute__((__warning__("Using gets() is always unsafe - use fgets() instead")));

#line 628 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _getw(FILE * _File);

#line 631 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 634 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _pclose(FILE * _File);

#line 635 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _popen(const char * _Command, const char * _Mode);

#line 641 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putc(int _Ch, FILE * _File);

#line 642 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putchar(int _Ch);

#line 643 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) puts(const char * _Str);

#line 644 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putw(int _Word, FILE * _File);

#line 647 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) remove(const char * _Filename);

#line 648 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) rename(const char * _OldFilename, const char * _NewFilename);

#line 649 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _unlink(const char * _Filename);

#line 651 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) unlink(const char * _Filename);

#line 654 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) rewind(FILE * _File);

#line 655 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _rmtmp(void);

#line 656 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) setbuf(FILE * __restrict__ _File, char * __restrict__ _Buffer);

#line 657 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _setmaxstdio(int _Max);

#line 658 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_output_format(unsigned int _Format);

#line 659 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _get_output_format(void);

#line 660 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) setvbuf(FILE * __restrict__ _File, char * __restrict__ _Buf, int _Mode, size_t _Size);

#line 664 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) _scprintf(const char * __restrict__ _Format, ...);

#line 666 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__scanf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snscanf(const char * __restrict__ _Src, size_t _MaxCount, const char * __restrict__ _Format, ...);

#line 676 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) __attribute__((__dllimport__)) int __attribute__((__cdecl__)) _vscprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 677 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * tmpfile(void);

#line 678 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * tmpnam(char * _Buffer);

#line 679 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) ungetc(int _Ch, FILE * _File);

#line 683 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _vsnprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, va_list _Args);

#line 685 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, ...);

#line 741 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int vsnprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, va_list __local_argv);

#line 744 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int snprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, ...);

#line 905 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_printf_count_output(int _Value);

#line 906 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _get_printf_count_output(void);

#line 912 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 914 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vswscanf(const wchar_t * __restrict__ _Str, const wchar_t * __restrict__ Format, va_list argp);

#line 916 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wscanf(const wchar_t * __restrict__ _Format, ...);

#line 918 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwscanf(const wchar_t * __restrict__ Format, va_list argp);

#line 920 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 922 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwscanf(FILE * __restrict__ fp, const wchar_t * __restrict__ Format, va_list argp);

#line 925 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 927 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wprintf(const wchar_t * __restrict__ _Format, ...);

#line 929 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 931 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 933 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 935 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 937 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...);

#line 939 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 942 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...) __asm__("swscanf");

#line 945 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vswscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, va_list) __asm__("vswscanf");

#line 948 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wscanf(const wchar_t * __restrict__ _Format, ...) __asm__("wscanf");

#line 951 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwscanf(const wchar_t * __restrict__ _Format, va_list) __asm__("vwscanf");

#line 954 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...) __asm__("fwscanf");

#line 957 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list) __asm__("vfwscanf");

#line 961 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 965 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wprintf(const wchar_t * __restrict__ _Format, ...) __asm__("wprintf");

#line 968 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vfwprintf");

#line 971 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vwprintf");

#line 974 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("swprintf");

#line 977 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vswprintf");

#line 980 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("snwprintf");

#line 983 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vsnwprintf");

#line 987 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf(unsigned long long options, wchar_t * str, size_t len, const wchar_t * format, _locale_t locale, va_list valist);

#line 988 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 989 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswscanf(unsigned long long options, const wchar_t * input, size_t length, const wchar_t * format, _locale_t locale, va_list valist);

#line 990 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwscanf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 1102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1104 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 1106 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) wscanf(const wchar_t * __restrict__ _Format, ...);

#line 1108 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int vfwscanf(FILE * __stream, const wchar_t * __format, va_list __local_argv);

#line 1111 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(2))) int vswscanf(const wchar_t * __restrict__ __source, const wchar_t * __restrict__ __format, va_list __local_argv);

#line 1113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__nonnull__(1))) int vwscanf(const wchar_t * __format, va_list __local_argv);

#line 1115 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1116 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) wprintf(const wchar_t * __restrict__ _Format, ...);

#line 1117 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1118 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1150 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfsopen(const wchar_t * _Filename, const wchar_t * _Mode, int _ShFlag);

#line 1153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) fgetwc(FILE * _File);

#line 1154 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwchar(void);

#line 1155 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) fputwc(wchar_t _Ch, FILE * _File);

#line 1156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwchar(wchar_t _Ch);

#line 1157 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) getwc(FILE * _File);

#line 1158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) getwchar(void);

#line 1159 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) putwc(wchar_t _Ch, FILE * _File);

#line 1160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) putwchar(wchar_t _Ch);

#line 1161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wint_t __attribute__((__cdecl__)) ungetwc(wint_t _Ch, FILE * _File);

#line 1162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
wchar_t * fgetws(wchar_t * __restrict__ _Dst, int _SizeInWords, FILE * __restrict__ _File);

#line 1163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputws(const wchar_t * __restrict__ _Str, FILE * __restrict__ _File);

#line 1164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _getws(wchar_t * _String);

#line 1165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putws(const wchar_t * _Str);

#line 1169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scwprintf(const wchar_t * __restrict__ _Format, ...) {

#line 1171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_list __ap;

#line 1172 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    int __ret;

#line 1173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_start(__ap, _Format);

#line 1174 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __ret = __stdio_common_vswprintf((*__local_stdio_printf_options()) | 0x0002ULL, ((void *)0), 0, _Format, ((void *)0), __ap);

#line 1175 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_end(__ap);

#line 1176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return __ret;
}

#line 1178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _snwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1179 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) _vsnwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) swprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vswprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1185 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 1186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) vsnwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, va_list arg);

#line 1190 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swprintf(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Format, ...) {

#line 1192 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_list __ap;

#line 1193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    int __ret;

#line 1194 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_start(__ap, _Format);

#line 1195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __ret = __stdio_common_vswprintf((*__local_stdio_printf_options()), _Dest, (size_t)-1, _Format, ((void *)0), __ap);

#line 1196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    __builtin_va_end(__ap);

#line 1197 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return __ret;
}

#line 1200 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswprintf(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Format, va_list _Args) {

#line 1202 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return __stdio_common_vswprintf((*__local_stdio_printf_options()), _Dest, (size_t)-1, _Format, ((void *)0), _Args);
}

#line 1206 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) {

#line 1208 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    int _Result = __stdio_common_vswprintf((*__local_stdio_printf_options()) | 0x0002ULL, ((void *)0), 0, _Format, ((void *)0), _ArgList);

#line 1209 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
    return _Result < 0 ? -1 : _Result;
}

#line 1249 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtempnam(const wchar_t * _Directory, const wchar_t * _FilePrefix);

#line 1253 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _snwscanf(const wchar_t * __restrict__ _Src, size_t _MaxCount, const wchar_t * __restrict__ _Format, ...);

#line 1254 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfdopen(int _FileHandle, const wchar_t * _Mode);

#line 1255 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode);

#line 1256 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wfreopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode, FILE * __restrict__ _OldFile);

#line 1260 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 1262 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) FILE * _wpopen(const wchar_t * _Command, const wchar_t * _Mode);

#line 1267 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wremove(const wchar_t * _Filename);

#line 1268 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtmpnam(wchar_t * _Buffer);

#line 1270 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwc_nolock(FILE * _File);

#line 1271 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwc_nolock(wchar_t _Ch, FILE * _File);

#line 1272 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _ungetwc_nolock(wint_t _Ch, FILE * _File);

#line 1297 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetc_nolock(FILE * _File);

#line 1298 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputc_nolock(int _Char, FILE * _File);

#line 1299 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getc_nolock(FILE * _File);

#line 1300 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putc_nolock(int _Char, FILE * _File);

#line 1312 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _lock_file(FILE * _File);

#line 1313 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _unlock_file(FILE * _File);

#line 1315 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fclose_nolock(FILE * _File);

#line 1316 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fflush_nolock(FILE * _File);

#line 1317 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 1318 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseek_nolock(FILE * _File, long _Offset, int _Origin);

#line 1319 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _ftell_nolock(FILE * _File);

#line 1320 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64_nolock(FILE * _File, long long _Offset, int _Origin);

#line 1321 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64_nolock(FILE * _File);

#line 1322 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fwrite_nolock(const void * __restrict__ _DstBuf, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 1323 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _ungetc_nolock(int _Ch, FILE * _File);

#line 1334 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
char * tempnam(const char * _Directory, const char * _FilePrefix);

#line 1338 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fcloseall(void);

#line 1339 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
FILE * fdopen(int _FileHandle, const char * _Format);

#line 1340 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fgetchar(void);

#line 1341 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fileno(FILE * _File);

#line 1342 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) flushall(void);

#line 1343 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) fputchar(int _Ch);

#line 1344 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) getw(FILE * _File);

#line 1345 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) putw(int _Ch, FILE * _File);

#line 1346 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) rmtmp(void);

#line 1363 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __mingw_str_wide_utf8(const wchar_t * const wptr, char * * mbptr, size_t * buflen);

#line 1377 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
int __attribute__((__cdecl__)) __mingw_str_utf8_wide(const char * const mbptr, wchar_t * * wptr, size_t * buflen);

#line 1386 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
void __attribute__((__cdecl__)) __mingw_str_free(void * ptr);

#line 1393 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnl(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1394 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnle(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1395 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlp(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1396 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlpe(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1397 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnv(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1398 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnve(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1399 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvp(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1400 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvpe(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1417 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnv(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1418 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnve(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 1419 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvp(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1420 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvpe(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 29 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) clearerr_s(FILE * _File);

#line 31 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
size_t __attribute__((__cdecl__)) fread_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_s(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_p(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 36 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnprintf_s(unsigned long long _Options, char * _Str, size_t _Len, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 37 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_s(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_p(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 40 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfscanf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 42 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfscanf(0x0001ULL, _File, _Format, _Locale, _ArgList);
}

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfscanf_s(FILE * _File, const char * _Format, va_list _ArgList) {

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfscanf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscanf_s_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vscanf_s(const char * _Format, va_list _ArgList) {

#line 57 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);
}

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fscanf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 62 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 63 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 65 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_s_l(_File, _Format, _Locale, _ArgList);

#line 66 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 67 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 70 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fscanf_s(FILE * _File, const char * _Format, ...) {

#line 72 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 73 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 74 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 76 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 77 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 80 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scanf_s_l(const char * _Format, _locale_t _Locale, ...) {

#line 82 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 85 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 86 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 90 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) scanf_s(const char * _Format, ...) {

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 93 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 95 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);

#line 96 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 97 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 100 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfscanf_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfscanf(0, _File, _Format, _Locale, _ArgList);
}

#line 105 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscanf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfscanf_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 110 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fscanf_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 112 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 113 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 114 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 115 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_l(_File, _Format, _Locale, _ArgList);

#line 116 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 117 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 119 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scanf_l(const char * _Format, _locale_t _Locale, ...) {

#line 121 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 122 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 123 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 124 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfscanf_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 125 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 126 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 129 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsscanf_s_l(const char * _Src, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 131 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsscanf(0x0001ULL, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 133 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsscanf_s(const char * _Src, const char * _Format, va_list _ArgList) {

#line 135 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsscanf_s_l(_Src, _Format, ((void *)0), _ArgList);
}

#line 137 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sscanf_s_l(const char * _Src, const char * _Format, _locale_t _Locale, ...) {

#line 139 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 140 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 141 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 142 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsscanf_s_l(_Src, _Format, _Locale, _ArgList);

#line 143 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 144 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) sscanf_s(const char * _Src, const char * _Format, ...) {

#line 148 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 149 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 150 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 151 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsscanf_s_l(_Src, _Format, ((void *)0), _ArgList);

#line 152 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 153 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsscanf_l(const char * _Src, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsscanf(0, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 160 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sscanf_l(const char * _Src, const char * _Format, _locale_t _Locale, ...) {

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 163 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 164 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsscanf_l(_Src, _Format, _Locale, _ArgList);

#line 166 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 167 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_s_l(const char * _Src, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 174 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 175 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = __stdio_common_vsscanf(0x0001ULL, _Src, _MaxCount, _Format, _Locale, _ArgList);

#line 177 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 180 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_s(const char * _Src, size_t _MaxCount, const char * _Format, ...) {

#line 182 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 183 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 184 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 185 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = __stdio_common_vsscanf(0x0001ULL, _Src, _MaxCount, _Format, ((void *)0), _ArgList);

#line 186 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 187 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 191 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_l(const char * _Src, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 193 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 194 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 195 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 196 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = __stdio_common_vsscanf(0, _Src, _MaxCount, _Format, _Locale, _ArgList);

#line 197 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 198 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 202 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 204 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfprintf_s((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 206 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfprintf_s(FILE * _File, const char * _Format, va_list _ArgList) {

#line 208 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 210 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_s_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 212 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 214 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vprintf_s(const char * _Format, va_list _ArgList) {

#line 216 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 218 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 220 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 221 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 222 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 223 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_s_l(_File, _Format, _Locale, _ArgList);

#line 224 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 225 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 227 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_s_l(const char * _Format, _locale_t _Locale, ...) {

#line 229 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 230 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 231 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 232 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 233 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 234 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 236 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fprintf_s(FILE * _File, const char * _Format, ...) {

#line 238 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 239 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 240 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 241 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 242 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 243 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 245 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) printf_s(const char * _Format, ...) {

#line 247 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 248 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 249 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 250 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 251 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 252 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 255 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_c_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 257 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf((*__local_stdio_printf_options()), _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 259 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_c(char * _DstBuf, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 261 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 263 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_c_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 265 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 266 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 267 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 268 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 269 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 270 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 272 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_c(char * _DstBuf, size_t _MaxCount, const char * _Format, ...) {

#line 274 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 275 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 276 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 277 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, ((void *)0), _ArgList);

#line 278 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 279 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 282 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_s_l(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 284 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsnprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);
}

#line 286 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsnprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 288 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 290 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 292 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 294 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_s_l(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 296 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 297 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 298 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 299 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);

#line 300 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 301 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 303 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, ...) {

#line 305 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 306 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 307 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 308 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);

#line 309 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 310 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 313 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_s_l(char * _DstBuf, size_t _DstSize, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 315 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _Format, _Locale, _ArgList);
}

#line 317 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsprintf_s(char * _DstBuf, size_t _Size, const char * _Format, va_list _ArgList) {

#line 319 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsprintf_s_l(_DstBuf, _Size, _Format, ((void *)0), _ArgList);
}

#line 321 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_s_l(char * _DstBuf, size_t _DstSize, const char * _Format, _locale_t _Locale, ...) {

#line 323 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 324 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 325 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 326 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsprintf_s_l(_DstBuf, _DstSize, _Format, _Locale, _ArgList);

#line 327 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 328 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 330 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) sprintf_s(char * _DstBuf, size_t _DstSize, const char * _Format, ...) {

#line 332 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 333 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 334 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 335 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);

#line 336 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 337 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 340 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_p_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 342 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfprintf_p((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 344 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_p(FILE * _File, const char * _Format, va_list _ArgList) {

#line 346 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_p_l(_File, _Format, ((void *)0), _ArgList);
}

#line 348 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_p_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 350 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_p_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 352 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_p(const char * _Format, va_list _ArgList) {

#line 354 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_p_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 356 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_p_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 358 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 359 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 360 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 361 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = __stdio_common_vfprintf_p((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);

#line 362 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 363 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 365 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_p(FILE * _File, const char * _Format, ...) {

#line 367 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 368 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 369 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 370 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_p_l(_File, _Format, ((void *)0), _ArgList);

#line 371 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 372 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 374 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_p_l(const char * _Format, _locale_t _Locale, ...) {

#line 376 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 377 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 378 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 379 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_p_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 380 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 381 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 383 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_p(const char * _Format, ...) {

#line 385 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 386 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 387 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 388 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_p_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 389 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 390 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 393 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_p_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 395 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf_p((*__local_stdio_printf_options()), _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 397 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_p(char * _Dst, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 399 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsprintf_p_l(_Dst, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 401 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_p_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 403 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 404 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 405 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 406 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsprintf_p_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 407 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 408 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 410 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_p(char * _Dst, size_t _MaxCount, const char * _Format, ...) {

#line 412 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 413 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 414 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 415 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsprintf_p_l(_Dst, _MaxCount, _Format, ((void *)0), _ArgList);

#line 416 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 417 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 420 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_p_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 422 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf_p(0x0002ULL, ((void *)0), 0, _Format, _Locale, _ArgList);
}

#line 424 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_p(const char * _Format, va_list _ArgList) {

#line 426 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vscprintf_p_l(_Format, ((void *)0), _ArgList);
}

#line 428 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_p_l(const char * _Format, _locale_t _Locale, ...) {

#line 430 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 431 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 432 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 433 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vscprintf_p_l(_Format, _Locale, _ArgList);

#line 434 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 435 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 437 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_p(const char * _Format, ...) {

#line 439 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 440 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 441 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 442 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vscprintf_p_l(_Format, ((void *)0), _ArgList);

#line 443 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 444 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 447 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 449 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfprintf((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 451 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 453 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfprintf_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 455 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 457 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 458 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 459 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 460 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_l(_File, _Format, _Locale, _ArgList);

#line 461 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 462 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 464 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_l(const char * _Format, _locale_t _Locale, ...) {

#line 466 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 467 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 468 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 469 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfprintf_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 470 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 471 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 474 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 476 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf(0x0001ULL, _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 478 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 480 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 481 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 482 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 483 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnprintf_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 484 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 485 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 487 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_l(char * _DstBuf, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 489 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsnprintf_l(_DstBuf, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 491 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_l(char * _DstBuf, const char * _Format, _locale_t _Locale, ...) {

#line 493 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 494 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 495 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 496 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsprintf_l(_DstBuf, _Format, _Locale, _ArgList);

#line 497 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 498 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 501 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 503 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsprintf(0x0002ULL, ((void *)0), 0, _Format, _Locale, _ArgList);
}

#line 505 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_l(const char * _Format, _locale_t _Locale, ...) {

#line 507 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 508 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 509 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 510 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vscprintf_l(_Format, _Locale, _ArgList);

#line 511 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 512 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 589 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) fopen_s(FILE * * _File, const char * _Filename, const char * _Mode);

#line 590 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) freopen_s(FILE * * _File, const char * _Filename, const char * _Mode, FILE * _Stream);

#line 592 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) char * gets_s(char *, rsize_t);

#line 595 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpfile_s(FILE * * _File);

#line 597 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpnam_s(char *, rsize_t);

#line 603 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) wchar_t * _getws_s(wchar_t * _Str, size_t _SizeInWords);

#line 607 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 608 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnwprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 609 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf_s(unsigned long long _Options, FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 611 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfwscanf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 613 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfwscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _File, _Format, _Locale, _ArgList);
}

#line 616 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfwscanf_s(FILE * _File, const wchar_t * _Format, va_list _ArgList) {

#line 618 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwscanf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 621 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vwscanf_s_l(const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 623 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 626 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vwscanf_s(const wchar_t * _Format, va_list _ArgList) {

#line 628 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);
}

#line 631 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fwscanf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 633 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 634 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 635 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 636 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwscanf_s_l(_File, _Format, _Locale, _ArgList);

#line 637 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 638 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 641 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fwscanf_s(FILE * _File, const wchar_t * _Format, ...) {

#line 643 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 644 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 645 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 646 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwscanf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 647 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 648 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 651 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _wscanf_s_l(const wchar_t * _Format, _locale_t _Locale, ...) {

#line 653 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 654 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 655 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 656 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 657 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 658 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 661 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) wscanf_s(const wchar_t * _Format, ...) {

#line 663 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 664 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 665 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 666 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);

#line 667 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 668 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 671 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswscanf_s_l(const wchar_t * _Src, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 673 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vswscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 676 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vswscanf_s(const wchar_t * _Src, const wchar_t * _Format, va_list _ArgList) {

#line 678 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vswscanf_s_l(_Src, _Format, ((void *)0), _ArgList);
}

#line 681 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swscanf_s_l(const wchar_t * _Src, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 683 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 684 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 685 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 686 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vswscanf_s_l(_Src, _Format, _Locale, _ArgList);

#line 687 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 688 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 690 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) swscanf_s(const wchar_t * _Src, const wchar_t * _Format, ...) {

#line 692 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 693 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 694 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 695 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vswscanf_s_l(_Src, _Format, ((void *)0), _ArgList);

#line 696 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 697 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 700 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwscanf_s_l(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 702 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vswscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _Src, _MaxCount, _Format, _Locale, _ArgList);
}

#line 704 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwscanf_s_l(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 706 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 707 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 708 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 709 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnwscanf_s_l(_Src, _MaxCount, _Format, _Locale, _ArgList);

#line 710 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 711 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 713 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwscanf_s(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, ...) {

#line 715 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 716 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 717 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 718 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnwscanf_s_l(_Src, _MaxCount, _Format, ((void *)0), _ArgList);

#line 719 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 720 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 723 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfwprintf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 725 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vfwprintf_s((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 727 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vwprintf_s_l(const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 729 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 731 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfwprintf_s(FILE * _File, const wchar_t * _Format, va_list _ArgList) {

#line 733 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwprintf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 735 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vwprintf_s(const wchar_t * _Format, va_list _ArgList) {

#line 737 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vfwprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 739 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fwprintf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 741 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 742 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 743 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 744 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwprintf_s_l(_File, _Format, _Locale, _ArgList);

#line 745 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 746 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 748 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _wprintf_s_l(const wchar_t * _Format, _locale_t _Locale, ...) {

#line 750 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 751 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 752 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 753 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 754 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 755 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 757 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fwprintf_s(FILE * _File, const wchar_t * _Format, ...) {

#line 759 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 760 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 761 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 762 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwprintf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 763 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 764 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 766 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) wprintf_s(const wchar_t * _Format, ...) {

#line 768 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 769 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 770 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 771 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vfwprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 772 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 773 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 776 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 778 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vswprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _Format, _Locale, _ArgList);
}

#line 780 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vswprintf_s(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, va_list _ArgList) {

#line 782 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vswprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);
}

#line 784 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 786 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 787 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 788 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 789 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vswprintf_s_l(_DstBuf, _DstSize, _Format, _Locale, _ArgList);

#line 790 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 791 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 793 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) swprintf_s(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, ...) {

#line 795 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 796 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 797 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 798 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vswprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);

#line 799 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 800 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 803 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 805 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return __stdio_common_vsnwprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);
}

#line 807 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwprintf_s(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, va_list _ArgList) {

#line 809 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 811 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 813 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 814 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 815 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 816 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);

#line 817 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 818 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 820 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwprintf_s(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, ...) {

#line 822 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_list _ArgList;

#line 823 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    int _Ret;

#line 824 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 825 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    _Ret = _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);

#line 826 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    __builtin_va_end(_ArgList);

#line 827 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
    return _Ret;
}

#line 867 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode);

#line 868 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfreopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode, FILE * _OldFile);

#line 870 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wtmpnam_s(wchar_t * _DstBuf, size_t _SizeInWords);

#line 912 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\libc\\include\\any-windows-any\\sec_api\\stdio_s.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 7 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int sum_ints(int count, ...) {

#line 8 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 10 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int total = 0;

#line 11 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 12 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        total = total + __builtin_va_arg(ap ,int);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 14 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return total;
}

#line 18 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
double sum_doubles(int count, ...) {

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 21 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    double total = 0.0;

#line 22 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 23 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        total = total + __builtin_va_arg(ap ,double);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 25 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return total;
}

#line 29 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int max_int(int count, ...) {

#line 30 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 32 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int result = __builtin_va_arg(ap ,int);

#line 33 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 1; i < count; i = i + 1) {

#line 34 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        int val = __builtin_va_arg(ap ,int);

#line 35 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        if (val > result) 
            result = val;
    }

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 38 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return result;
}

#line 44 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int parse_fmt(const char * fmt, ...) {

#line 45 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, fmt);

#line 47 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int count = 0;

#line 48 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; fmt[i] != '\0'; i = i + 1) {

#line 49 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        if (fmt[i] == 'd') {

#line 50 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
            int v = __builtin_va_arg(ap ,int);

#line 51 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
            (void)v;

#line 52 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
            count = count + 1;
        }
        else 

#line 53 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
            if (fmt[i] == 'f') {

#line 54 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                double v = __builtin_va_arg(ap ,double);

#line 55 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                (void)v;

#line 56 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                count = count + 1;
            }
            else 

#line 57 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                if (fmt[i] == 's') {

#line 58 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                    const char * v = __builtin_va_arg(ap ,const char *);

#line 59 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                    (void)v;

#line 60 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
                    count = count + 1;
                }
    }

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 64 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return count;
}

#line 68 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int str_lengths(int count, ...) {

#line 69 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 71 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int total = 0;

#line 72 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) {

#line 73 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        const char * s = __builtin_va_arg(ap ,const char *);

#line 74 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        int len = 0;

#line 75 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        while (s[len] != '\0') 
            len = len + 1;

#line 76 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        total = total + len;
    }

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 79 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return total;
}

#line 83 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int sum_then_max(int count, ...) {

#line 84 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 87 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap2;

#line 11 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_copy.h"
    __builtin_va_copy(ap2, ap);

#line 90 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int sum = 0;

#line 91 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 92 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        sum = sum + __builtin_va_arg(ap ,int);

#line 94 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int mx = 0;

#line 95 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) {

#line 96 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        int val = __builtin_va_arg(ap2 ,int);

#line 97 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        if (i == 0 || val > mx) 
            mx = val;
    }

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);
    __builtin_va_end(ap2);

#line 102 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return sum * 1000 + mx;
}

#line 106 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int first_arg(int n, ...) {

#line 107 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, n);

#line 109 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    int result = __builtin_va_arg(ap ,int);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 111 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return result;
}

#line 115 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
long long sum_longs(int count, ...) {

#line 116 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    va_list ap;

#line 17 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_start(ap, count);

#line 118 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    long long total = 0;

#line 119 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 120 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
        total = total + __builtin_va_arg(ap ,long long);

#line 19 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\zig\\lib\\include\\__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 122 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return total;
}

#line 125 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
int main(void) {

#line 127 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_ints(0) != 0) 
        return 1;

#line 128 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_ints(1, 42) != 42) 
        return 2;

#line 129 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_ints(3, 10, 20, 30) != 60) 
        return 3;

#line 130 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_ints(5, 1, 2, 3, 4, 5) != 15) 
        return 4;

#line 133 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_doubles(0) < -0.001 || sum_doubles(0) > 0.001) 
        return 5;

#line 134 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_doubles(2, 1.5, 2.5) < 3.99 || sum_doubles(2, 1.5, 2.5) > 4.01) 
        return 6;

#line 135 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_doubles(3, 0.1, 0.2, 0.3) < 0.59 || sum_doubles(3, 0.1, 0.2, 0.3) > 0.61) 
        return 7;

#line 138 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (max_int(1, 7) != 7) 
        return 8;

#line 139 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (max_int(3, 5, 9, 3) != 9) 
        return 9;

#line 140 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (max_int(4, -1, -5, -3, -2) != -1) 
        return 10;

#line 143 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (parse_fmt("") != 0) 
        return 11;

#line 144 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (parse_fmt("d", 10) != 1) 
        return 12;

#line 145 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (parse_fmt("ddf", 1, 2, 3.0) != 3) 
        return 13;

#line 146 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (parse_fmt("dsf", 42, "hello", 1.0) != 3) 
        return 14;

#line 147 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (parse_fmt("dds", 1, 2, "abc") != 3) 
        return 15;

#line 150 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (str_lengths(0) != 0) 
        return 16;

#line 151 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (str_lengths(1, "abc") != 3) 
        return 17;

#line 152 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (str_lengths(3, "ab", "cde", "f") != 6) 
        return 18;

#line 156 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_then_max(3, 10, 20, 30) != 60030) 
        return 19;

#line 158 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_then_max(4, 5, 1, 8, 3) != 17008) 
        return 20;

#line 161 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (first_arg(1, 99) != 99) 
        return 21;

#line 162 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (first_arg(3, 77, 88, 99) != 77) 
        return 22;

#line 165 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_longs(0) != 0) 
        return 23;

#line 166 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (sum_longs(2, 1000000000LL, 2000000000LL) != 3000000000LL) 
        return 24;

#line 169 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("%d\n", 42) < 1) 
        return 25;

#line 170 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("%s %d\n", "hello", 123) < 1) 
        return 26;

#line 171 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("%f\n", 3.14) < 1) 
        return 27;

#line 172 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("%c %x %o\n", 'A', 255, 8) < 1) 
        return 28;

#line 173 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("%ld %lld\n", (long)100, (long long)999) < 1) 
        return 29;

#line 176 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    if (printf("") != 0) 
        return 30;

#line 178 "C:\\Users\\ftp\\Desktop\\sharp-proj\\sharp\\sharp-test\\unit\\cinterop\\variadic.ce"
    return 0;
}
