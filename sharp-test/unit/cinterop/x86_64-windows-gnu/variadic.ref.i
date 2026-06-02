
#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/vadefs.h"
typedef __builtin_va_list __gnuc_va_list;

#line 31 "/root/code/sharp/zig/lib/libc/include/any-windows-any/vadefs.h"
typedef __gnuc_va_list va_list;

#line 604 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
void __attribute__((__cdecl__)) __debugbreak(void);

#line 625 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code);

#line 665 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
const char * __mingw_get_crt_info(void);

#line 35 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef unsigned long long size_t;

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef long long ssize_t;

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef size_t rsize_t;

#line 62 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef long long intptr_t;

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef unsigned long long uintptr_t;

#line 88 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef long long ptrdiff_t;

#line 98 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef unsigned short wchar_t;

#line 106 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef unsigned short wint_t;

#line 107 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef unsigned short wctype_t;

#line 113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef int errno_t;

#line 118 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef long __time32_t;

#line 123 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
__extension__ typedef long long __time64_t;

#line 138 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef __time64_t time_t;

#line 430 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
struct threadlocaleinfostruct;

#line 431 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
struct threadmbcinfostruct;

#line 432 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef struct threadlocaleinfostruct * pthreadlocinfo;

#line 433 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef struct threadmbcinfostruct * pthreadmbcinfo;

#line 434 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
struct __lc_time_data;

#line 436 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"

#line 439 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef struct localeinfo_struct {
    pthreadlocinfo locinfo;
    pthreadmbcinfo mbcinfo;
} _locale_tstruct;
typedef struct localeinfo_struct * _locale_t;

#line 443 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"

#line 447 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef struct tagLC_ID {
    unsigned short wLanguage;
    unsigned short wCountry;
    unsigned short wCodePage;
} LC_ID;
typedef struct tagLC_ID * LPLC_ID;

#line 452 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"

#line 482 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt.h"
typedef struct threadlocaleinfostruct {
    const unsigned short * _locale_pctype;
    int _locale_mb_cur_max;
    unsigned int _locale_lc_codepage;
} threadlocinfo;

#line 16 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_stdio_config.h"
unsigned long long * __local_stdio_printf_options(void);

#line 17 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_stdio_config.h"
unsigned long long * __local_stdio_scanf_options(void);

#line 33 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
struct _iobuf {
    void * _Placeholder;
};

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
typedef struct _iobuf FILE;

#line 5 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw_off_t.h"
typedef long _off_t;

#line 7 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw_off_t.h"
typedef long off32_t;

#line 13 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw_off_t.h"
__extension__ typedef long long _off64_t;

#line 15 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw_off_t.h"
__extension__ typedef long long off64_t;

#line 26 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw_off_t.h"
typedef off32_t off_t;

#line 101 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * __acrt_iob_func(unsigned index);

#line 103 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * __iob_func(void);

#line 112 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__extension__ typedef long long fpos_t;

#line 158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsscanf(const char * __restrict__ _Str, const char * __restrict__ Format, va_list argp);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_scanf(const char * __restrict__ _Format, ...);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vscanf(const char * __restrict__ Format, va_list argp);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfscanf(FILE * __restrict__ fp, const char * __restrict__ Format, va_list argp);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnprintf(char * __restrict__ _DstBuf, size_t _MaxCount, const char * __restrict__ _Format, va_list _ArgList);

#line 181 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snprintf(char * __restrict__ s, size_t n, const char * __restrict__ format, ...);

#line 184 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_printf(const char * __restrict__, ...) __attribute__((__nothrow__));

#line 187 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vprintf(const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fprintf(FILE * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sprintf(char * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 199 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsprintf(char * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 202 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_asprintf(char * * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 205 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_vasprintf(char * * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 209 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...) __asm__("sscanf");

#line 213 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsscanf(const char * __restrict__ _Str, const char * __restrict__ _Format, va_list argp) __asm__("vsscanf");

#line 217 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_scanf(const char * __restrict__ _Format, ...) __asm__("scanf");

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vscanf(const char * __restrict__ _Format, va_list argp) __asm__("vscanf");

#line 225 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...) __asm__("fscanf");

#line 229 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list argp) __asm__("vfscanf");

#line 234 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_printf(const char * __restrict__, ...) __asm__("printf") __attribute__((__nothrow__));

#line 238 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vprintf(const char * __restrict__, va_list) __asm__("vprintf") __attribute__((__nothrow__));

#line 242 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fprintf(FILE * __restrict__, const char * __restrict__, ...) __asm__("fprintf") __attribute__((__nothrow__));

#line 246 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __asm__("vfprintf") __attribute__((__nothrow__));

#line 251 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sprintf(char * __restrict__, const char * __restrict__, ...) __asm__("sprintf") __attribute__((__nothrow__));

#line 255 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsprintf(char * __restrict__, const char * __restrict__, va_list) __asm__("vsprintf") __attribute__((__nothrow__));

#line 259 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snprintf(char * __restrict__, size_t, const char * __restrict__, ...) __asm__("snprintf") __attribute__((__nothrow__));

#line 263 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnprintf(char * __restrict__, size_t, const char * __restrict__, va_list) __asm__("vsnprintf") __attribute__((__nothrow__));

#line 267 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf(unsigned long long options, char * str, size_t len, const char * format, _locale_t locale, va_list valist);

#line 268 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 269 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsscanf(unsigned long long options, const char * input, size_t length, const char * format, _locale_t locale, va_list valist);

#line 270 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfscanf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 448 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 450 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) printf(const char * __restrict__ _Format, ...);

#line 452 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, ...);

#line 455 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vfprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list _ArgList);

#line 457 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) vprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 459 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vsprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, va_list _Args);

#line 463 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 466 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) scanf(const char * __restrict__ _Format, ...);

#line 469 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 478 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vfscanf(FILE * __stream, const char * __format, __builtin_va_list __local_argv);

#line 481 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vsscanf(const char * __restrict__ __source, const char * __restrict__ __format, __builtin_va_list __local_argv);

#line 483 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int vscanf(const char * __format, __builtin_va_list __local_argv);

#line 535 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _filbuf(FILE * _File);

#line 536 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flsbuf(int _Ch, FILE * _File);

#line 540 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _fsopen(const char * _Filename, const char * _Mode, int _ShFlag);

#line 542 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) clearerr(FILE * _File);

#line 543 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fclose(FILE * _File);

#line 544 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fcloseall(void);

#line 548 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _fdopen(int _FileHandle, const char * _Mode);

#line 550 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) feof(FILE * _File);

#line 551 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) ferror(FILE * _File);

#line 552 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fflush(FILE * _File);

#line 553 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetc(FILE * _File);

#line 554 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetchar(void);

#line 555 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetpos(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 556 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetpos64(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 557 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * fgets(char * __restrict__ _Buf, int _MaxCount, FILE * __restrict__ _File);

#line 558 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fileno(FILE * _File);

#line 566 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) char * _tempnam(const char * _DirName, const char * _FilePrefix);

#line 570 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flushall(void);

#line 571 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode);

#line 572 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fopen64(const char * __restrict__ filename, const char * __restrict__ mode);

#line 573 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputc(int _Ch, FILE * _File);

#line 574 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputchar(int _Ch);

#line 575 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputs(const char * __restrict__ _Str, FILE * __restrict__ _File);

#line 576 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
size_t __attribute__((__cdecl__)) fread(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 577 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * freopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode, FILE * __restrict__ _File);

#line 578 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fsetpos(FILE * _File, const fpos_t * _Pos);

#line 579 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fsetpos64(FILE * _File, const fpos_t * _Pos);

#line 580 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fseek(FILE * _File, long _Offset, int _Origin);

#line 581 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
long __attribute__((__cdecl__)) ftell(FILE * _File);

#line 585 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64(FILE * _File, long long _Offset, int _Origin);

#line 586 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64(FILE * _File);

#line 622 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
size_t __attribute__((__cdecl__)) fwrite(const void * __restrict__ _Str, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 623 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getc(FILE * _File);

#line 624 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getchar(void);

#line 625 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getmaxstdio(void);

#line 626 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * gets(char * _Buffer) __attribute__((__warning__("Using gets() is always unsafe - use fgets() instead")));

#line 628 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _getw(FILE * _File);

#line 631 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 634 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _pclose(FILE * _File);

#line 635 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _popen(const char * _Command, const char * _Mode);

#line 641 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putc(int _Ch, FILE * _File);

#line 642 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putchar(int _Ch);

#line 643 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) puts(const char * _Str);

#line 644 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putw(int _Word, FILE * _File);

#line 647 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) remove(const char * _Filename);

#line 648 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) rename(const char * _OldFilename, const char * _NewFilename);

#line 649 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _unlink(const char * _Filename);

#line 651 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) unlink(const char * _Filename);

#line 654 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) rewind(FILE * _File);

#line 655 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _rmtmp(void);

#line 656 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) setbuf(FILE * __restrict__ _File, char * __restrict__ _Buffer);

#line 657 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _setmaxstdio(int _Max);

#line 658 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_output_format(unsigned int _Format);

#line 659 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _get_output_format(void);

#line 660 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) setvbuf(FILE * __restrict__ _File, char * __restrict__ _Buf, int _Mode, size_t _Size);

#line 664 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) _scprintf(const char * __restrict__ _Format, ...);

#line 666 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snscanf(const char * __restrict__ _Src, size_t _MaxCount, const char * __restrict__ _Format, ...);

#line 676 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) __attribute__((__dllimport__)) int __attribute__((__cdecl__)) _vscprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 677 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * tmpfile(void);

#line 678 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * tmpnam(char * _Buffer);

#line 679 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) ungetc(int _Ch, FILE * _File);

#line 683 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _vsnprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, va_list _Args);

#line 685 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, ...);

#line 741 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int vsnprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, va_list __local_argv);

#line 744 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int snprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, ...);

#line 905 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_printf_count_output(int _Value);

#line 906 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _get_printf_count_output(void);

#line 912 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 914 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vswscanf(const wchar_t * __restrict__ _Str, const wchar_t * __restrict__ Format, va_list argp);

#line 916 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wscanf(const wchar_t * __restrict__ _Format, ...);

#line 918 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwscanf(const wchar_t * __restrict__ Format, va_list argp);

#line 920 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 922 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwscanf(FILE * __restrict__ fp, const wchar_t * __restrict__ Format, va_list argp);

#line 925 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 927 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wprintf(const wchar_t * __restrict__ _Format, ...);

#line 929 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 931 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 933 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 935 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 937 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...);

#line 939 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 942 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...) __asm__("swscanf");

#line 945 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vswscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, va_list) __asm__("vswscanf");

#line 948 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wscanf(const wchar_t * __restrict__ _Format, ...) __asm__("wscanf");

#line 951 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwscanf(const wchar_t * __restrict__ _Format, va_list) __asm__("vwscanf");

#line 954 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...) __asm__("fwscanf");

#line 957 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list) __asm__("vfwscanf");

#line 961 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 965 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wprintf(const wchar_t * __restrict__ _Format, ...) __asm__("wprintf");

#line 968 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vfwprintf");

#line 971 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vwprintf");

#line 974 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("swprintf");

#line 977 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vswprintf");

#line 980 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("snwprintf");

#line 983 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vsnwprintf");

#line 987 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf(unsigned long long options, wchar_t * str, size_t len, const wchar_t * format, _locale_t locale, va_list valist);

#line 988 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 989 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswscanf(unsigned long long options, const wchar_t * input, size_t length, const wchar_t * format, _locale_t locale, va_list valist);

#line 990 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwscanf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 1102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1104 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 1106 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) wscanf(const wchar_t * __restrict__ _Format, ...);

#line 1108 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int vfwscanf(FILE * __stream, const wchar_t * __format, va_list __local_argv);

#line 1111 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int vswscanf(const wchar_t * __restrict__ __source, const wchar_t * __restrict__ __format, va_list __local_argv);

#line 1113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int vwscanf(const wchar_t * __format, va_list __local_argv);

#line 1115 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1116 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) wprintf(const wchar_t * __restrict__ _Format, ...);

#line 1117 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1118 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1150 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfsopen(const wchar_t * _Filename, const wchar_t * _Mode, int _ShFlag);

#line 1153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) fgetwc(FILE * _File);

#line 1154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwchar(void);

#line 1155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) fputwc(wchar_t _Ch, FILE * _File);

#line 1156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwchar(wchar_t _Ch);

#line 1157 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) getwc(FILE * _File);

#line 1158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) getwchar(void);

#line 1159 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) putwc(wchar_t _Ch, FILE * _File);

#line 1160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) putwchar(wchar_t _Ch);

#line 1161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) ungetwc(wint_t _Ch, FILE * _File);

#line 1162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wchar_t * fgetws(wchar_t * __restrict__ _Dst, int _SizeInWords, FILE * __restrict__ _File);

#line 1163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputws(const wchar_t * __restrict__ _Str, FILE * __restrict__ _File);

#line 1164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _getws(wchar_t * _String);

#line 1165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putws(const wchar_t * _Str);

#line 1178 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _snwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1179 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _vsnwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1182 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) swprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1183 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vswprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1185 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 1186 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vsnwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, va_list arg);

#line 1249 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtempnam(const wchar_t * _Directory, const wchar_t * _FilePrefix);

#line 1253 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _snwscanf(const wchar_t * __restrict__ _Src, size_t _MaxCount, const wchar_t * __restrict__ _Format, ...);

#line 1254 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfdopen(int _FileHandle, const wchar_t * _Mode);

#line 1255 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode);

#line 1256 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfreopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode, FILE * __restrict__ _OldFile);

#line 1260 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 1262 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wpopen(const wchar_t * _Command, const wchar_t * _Mode);

#line 1267 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wremove(const wchar_t * _Filename);

#line 1268 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtmpnam(wchar_t * _Buffer);

#line 1270 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwc_nolock(FILE * _File);

#line 1271 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwc_nolock(wchar_t _Ch, FILE * _File);

#line 1272 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _ungetwc_nolock(wint_t _Ch, FILE * _File);

#line 1297 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetc_nolock(FILE * _File);

#line 1298 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputc_nolock(int _Char, FILE * _File);

#line 1299 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getc_nolock(FILE * _File);

#line 1300 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putc_nolock(int _Char, FILE * _File);

#line 1312 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _lock_file(FILE * _File);

#line 1313 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _unlock_file(FILE * _File);

#line 1315 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fclose_nolock(FILE * _File);

#line 1316 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fflush_nolock(FILE * _File);

#line 1317 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 1318 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseek_nolock(FILE * _File, long _Offset, int _Origin);

#line 1319 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _ftell_nolock(FILE * _File);

#line 1320 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64_nolock(FILE * _File, long long _Offset, int _Origin);

#line 1321 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64_nolock(FILE * _File);

#line 1322 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fwrite_nolock(const void * __restrict__ _DstBuf, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 1323 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _ungetc_nolock(int _Ch, FILE * _File);

#line 1334 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * tempnam(const char * _Directory, const char * _FilePrefix);

#line 1338 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fcloseall(void);

#line 1339 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fdopen(int _FileHandle, const char * _Format);

#line 1340 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetchar(void);

#line 1341 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fileno(FILE * _File);

#line 1342 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) flushall(void);

#line 1343 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputchar(int _Ch);

#line 1344 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getw(FILE * _File);

#line 1345 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putw(int _Ch, FILE * _File);

#line 1346 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) rmtmp(void);

#line 1363 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __mingw_str_wide_utf8(const wchar_t * const wptr, char * * mbptr, size_t * buflen);

#line 1377 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __mingw_str_utf8_wide(const char * const mbptr, wchar_t * * wptr, size_t * buflen);

#line 1386 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) __mingw_str_free(void * ptr);

#line 1393 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnl(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1394 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnle(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1395 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlp(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1396 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlpe(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1397 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnv(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1398 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnve(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1399 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvp(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1400 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvpe(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1417 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnv(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1418 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnve(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 1419 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvp(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1420 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvpe(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 29 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) clearerr_s(FILE * _File);

#line 31 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
size_t __attribute__((__cdecl__)) fread_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 34 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_s(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 35 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_p(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnprintf_s(unsigned long long _Options, char * _Str, size_t _Len, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_s(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 38 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_p(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 589 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) fopen_s(FILE * * _File, const char * _Filename, const char * _Mode);

#line 590 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) freopen_s(FILE * * _File, const char * _Filename, const char * _Mode, FILE * _Stream);

#line 592 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) char * gets_s(char *, rsize_t);

#line 595 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpfile_s(FILE * * _File);

#line 597 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpnam_s(char *, rsize_t);

#line 603 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) wchar_t * _getws_s(wchar_t * _Str, size_t _SizeInWords);

#line 607 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 608 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnwprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 609 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf_s(unsigned long long _Options, FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 867 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode);

#line 868 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfreopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode, FILE * _OldFile);

#line 870 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wtmpnam_s(wchar_t * _DstBuf, size_t _SizeInWords);

#line 912 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 12 "/root/code/sharp/zig/lib/include/__stdarg_va_list.h"
typedef __builtin_va_list va_list;

#line 604 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
void __attribute__((__cdecl__)) __debugbreak(void);

#line 605 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) void __attribute__((__cdecl__)) __debugbreak(void) {

#line 610 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
    __asm__ __volatile__ ( "int {$}3" : );
}

#line 625 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code);

#line 626 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) void __attribute__((__cdecl__)) __attribute__((__noreturn__)) __fastfail(unsigned int code) {

#line 632 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
    __asm__ __volatile__ ( "int {$}0x29" : : "c" ( code ) );

#line 639 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
    __builtin_unreachable();
}

#line 665 "/root/code/sharp/zig/lib/libc/include/any-windows-any/_mingw.h"
const char * __mingw_get_crt_info(void);

#line 16 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_stdio_config.h"
unsigned long long * __local_stdio_printf_options(void);

#line 17 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_stdio_config.h"
unsigned long long * __local_stdio_scanf_options(void);

#line 101 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * __acrt_iob_func(unsigned index);

#line 103 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * __iob_func(void);

#line 158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsscanf(const char * __restrict__ _Str, const char * __restrict__ Format, va_list argp);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_scanf(const char * __restrict__ _Format, ...);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vscanf(const char * __restrict__ Format, va_list argp);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfscanf(FILE * __restrict__ fp, const char * __restrict__ Format, va_list argp);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnprintf(char * __restrict__ _DstBuf, size_t _MaxCount, const char * __restrict__ _Format, va_list _ArgList);

#line 181 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snprintf(char * __restrict__ s, size_t n, const char * __restrict__ format, ...);

#line 184 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_printf(const char * __restrict__, ...) __attribute__((__nothrow__));

#line 187 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vprintf(const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fprintf(FILE * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_sprintf(char * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 199 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vsprintf(char * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 202 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_asprintf(char * * __restrict__, const char * __restrict__, ...) __attribute__((__nothrow__));

#line 205 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((nonnull(1,2))) int __attribute__((__cdecl__)) __mingw_vasprintf(char * * __restrict__, const char * __restrict__, va_list) __attribute__((__nothrow__));

#line 209 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...) __asm__("sscanf");

#line 213 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsscanf(const char * __restrict__ _Str, const char * __restrict__ _Format, va_list argp) __asm__("vsscanf");

#line 217 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_scanf(const char * __restrict__ _Format, ...) __asm__("scanf");

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vscanf(const char * __restrict__ _Format, va_list argp) __asm__("vscanf");

#line 225 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...) __asm__("fscanf");

#line 229 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list argp) __asm__("vfscanf");

#line 234 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_printf(const char * __restrict__, ...) __asm__("printf") __attribute__((__nothrow__));

#line 238 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vprintf(const char * __restrict__, va_list) __asm__("vprintf") __attribute__((__nothrow__));

#line 242 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fprintf(FILE * __restrict__, const char * __restrict__, ...) __asm__("fprintf") __attribute__((__nothrow__));

#line 246 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfprintf(FILE * __restrict__, const char * __restrict__, va_list) __asm__("vfprintf") __attribute__((__nothrow__));

#line 251 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_sprintf(char * __restrict__, const char * __restrict__, ...) __asm__("sprintf") __attribute__((__nothrow__));

#line 255 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vsprintf(char * __restrict__, const char * __restrict__, va_list) __asm__("vsprintf") __attribute__((__nothrow__));

#line 259 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snprintf(char * __restrict__, size_t, const char * __restrict__, ...) __asm__("snprintf") __attribute__((__nothrow__));

#line 263 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
extern __attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnprintf(char * __restrict__, size_t, const char * __restrict__, va_list) __asm__("vsnprintf") __attribute__((__nothrow__));

#line 267 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf(unsigned long long options, char * str, size_t len, const char * format, _locale_t locale, va_list valist);

#line 268 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 269 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vsscanf(unsigned long long options, const char * input, size_t length, const char * format, _locale_t locale, va_list valist);

#line 270 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfscanf(unsigned long long options, FILE * file, const char * format, _locale_t locale, va_list valist);

#line 448 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 450 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) printf(const char * __restrict__ _Format, ...);

#line 452 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, ...);

#line 455 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vfprintf(FILE * __restrict__ _File, const char * __restrict__ _Format, va_list _ArgList);

#line 457 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) vprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 459 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,2,0))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) vsprintf(char * __restrict__ _Dest, const char * __restrict__ _Format, va_list _Args);

#line 463 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) fscanf(FILE * __restrict__ _File, const char * __restrict__ _Format, ...);

#line 466 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) scanf(const char * __restrict__ _Format, ...);

#line 469 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,3))) __attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) sscanf(const char * __restrict__ _Src, const char * __restrict__ _Format, ...);

#line 478 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vfscanf(FILE * __stream, const char * __format, __builtin_va_list __local_argv);

#line 481 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,2,0))) __attribute__((__nonnull__(2))) int vsscanf(const char * __restrict__ __source, const char * __restrict__ __format, __builtin_va_list __local_argv);

#line 483 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,1,0))) __attribute__((__nonnull__(1))) int vscanf(const char * __format, __builtin_va_list __local_argv);

#line 535 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _filbuf(FILE * _File);

#line 536 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flsbuf(int _Ch, FILE * _File);

#line 540 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _fsopen(const char * _Filename, const char * _Mode, int _ShFlag);

#line 542 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) clearerr(FILE * _File);

#line 543 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fclose(FILE * _File);

#line 544 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fcloseall(void);

#line 548 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _fdopen(int _FileHandle, const char * _Mode);

#line 550 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) feof(FILE * _File);

#line 551 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) ferror(FILE * _File);

#line 552 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fflush(FILE * _File);

#line 553 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetc(FILE * _File);

#line 554 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetchar(void);

#line 555 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetpos(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 556 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetpos64(FILE * __restrict__ _File, fpos_t * __restrict__ _Pos);

#line 557 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * fgets(char * __restrict__ _Buf, int _MaxCount, FILE * __restrict__ _File);

#line 558 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fileno(FILE * _File);

#line 566 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) char * _tempnam(const char * _DirName, const char * _FilePrefix);

#line 570 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _flushall(void);

#line 571 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode);

#line 572 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fopen64(const char * __restrict__ filename, const char * __restrict__ mode);

#line 573 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputc(int _Ch, FILE * _File);

#line 574 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputchar(int _Ch);

#line 575 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputs(const char * __restrict__ _Str, FILE * __restrict__ _File);

#line 576 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
size_t __attribute__((__cdecl__)) fread(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 577 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * freopen(const char * __restrict__ _Filename, const char * __restrict__ _Mode, FILE * __restrict__ _File);

#line 578 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fsetpos(FILE * _File, const fpos_t * _Pos);

#line 579 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fsetpos64(FILE * _File, const fpos_t * _Pos);

#line 580 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fseek(FILE * _File, long _Offset, int _Origin);

#line 581 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
long __attribute__((__cdecl__)) ftell(FILE * _File);

#line 585 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64(FILE * _File, long long _Offset, int _Origin);

#line 586 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64(FILE * _File);

#line 588 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int fseeko(FILE * _File, _off_t _Offset, int _Origin) {

#line 589 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return fseek(_File, _Offset, _Origin);
}

#line 591 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int fseeko64(FILE * _File, _off64_t _Offset, int _Origin) {

#line 592 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return _fseeki64(_File, _Offset, _Origin);
}

#line 594 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ _off_t ftello(FILE * _File) {

#line 595 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return ftell(_File);
}

#line 597 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ _off64_t ftello64(FILE * _File) {

#line 598 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return _ftelli64(_File);
}

#line 622 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
size_t __attribute__((__cdecl__)) fwrite(const void * __restrict__ _Str, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 623 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getc(FILE * _File);

#line 624 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getchar(void);

#line 625 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getmaxstdio(void);

#line 626 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * gets(char * _Buffer) __attribute__((__warning__("Using gets() is always unsafe - use fgets() instead")));

#line 628 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _getw(FILE * _File);

#line 631 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) perror(const char * _ErrMsg);

#line 634 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _pclose(FILE * _File);

#line 635 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _popen(const char * _Command, const char * _Mode);

#line 641 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putc(int _Ch, FILE * _File);

#line 642 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putchar(int _Ch);

#line 643 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) puts(const char * _Str);

#line 644 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putw(int _Word, FILE * _File);

#line 647 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) remove(const char * _Filename);

#line 648 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) rename(const char * _OldFilename, const char * _NewFilename);

#line 649 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _unlink(const char * _Filename);

#line 651 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) unlink(const char * _Filename);

#line 654 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) rewind(FILE * _File);

#line 655 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _rmtmp(void);

#line 656 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) setbuf(FILE * __restrict__ _File, char * __restrict__ _Buffer);

#line 657 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _setmaxstdio(int _Max);

#line 658 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_output_format(unsigned int _Format);

#line 659 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _get_output_format(void);

#line 660 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) setvbuf(FILE * __restrict__ _File, char * __restrict__ _Buf, int _Mode, size_t _Size);

#line 664 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,2))) __attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) _scprintf(const char * __restrict__ _Format, ...);

#line 666 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__scanf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snscanf(const char * __restrict__ _Src, size_t _MaxCount, const char * __restrict__ _Format, ...);

#line 676 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__pure__)) __attribute__((__format__(__printf__,1,0))) __attribute__((__nonnull__(1))) __attribute__((__dllimport__)) int __attribute__((__cdecl__)) _vscprintf(const char * __restrict__ _Format, va_list _ArgList);

#line 677 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * tmpfile(void);

#line 678 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * tmpnam(char * _Buffer);

#line 679 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) ungetc(int _Ch, FILE * _File);

#line 683 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _vsnprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, va_list _Args);

#line 685 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) _snprintf(char * __restrict__ _Dest, size_t _Count, const char * __restrict__ _Format, ...);

#line 741 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,0))) __attribute__((__nonnull__(3))) int vsnprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, va_list __local_argv);

#line 744 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__format__(__printf__,3,4))) __attribute__((__nonnull__(3))) int snprintf(char * __restrict__ __stream, size_t __n, const char * __restrict__ __format, ...);

#line 905 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_printf_count_output(int _Value);

#line 906 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _get_printf_count_output(void);

#line 912 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 914 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vswscanf(const wchar_t * __restrict__ _Str, const wchar_t * __restrict__ Format, va_list argp);

#line 916 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wscanf(const wchar_t * __restrict__ _Format, ...);

#line 918 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwscanf(const wchar_t * __restrict__ Format, va_list argp);

#line 920 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 922 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwscanf(FILE * __restrict__ fp, const wchar_t * __restrict__ Format, va_list argp);

#line 925 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 927 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_wprintf(const wchar_t * __restrict__ _Format, ...);

#line 929 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __mingw_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 931 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __mingw_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 933 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 935 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 937 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...);

#line 939 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __mingw_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list);

#line 942 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...) __asm__("swscanf");

#line 945 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vswscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, va_list) __asm__("vswscanf");

#line 948 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wscanf(const wchar_t * __restrict__ _Format, ...) __asm__("wscanf");

#line 951 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwscanf(const wchar_t * __restrict__ _Format, va_list) __asm__("vwscanf");

#line 954 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...) __asm__("fwscanf");

#line 957 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list) __asm__("vfwscanf");

#line 961 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 965 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_wprintf(const wchar_t * __restrict__ _Format, ...) __asm__("wprintf");

#line 968 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int __attribute__((__cdecl__)) __ms_vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vfwprintf");

#line 971 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int __attribute__((__cdecl__)) __ms_vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) __asm__("vwprintf");

#line 974 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_swprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("swprintf");

#line 977 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vswprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vswprintf");

#line 980 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_snwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, ...) __asm__("snwprintf");

#line 983 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(3))) int __attribute__((__cdecl__)) __ms_vsnwprintf(wchar_t * __restrict__, size_t, const wchar_t * __restrict__, va_list) __asm__("vsnwprintf");

#line 987 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf(unsigned long long options, wchar_t * str, size_t len, const wchar_t * format, _locale_t locale, va_list valist);

#line 988 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 989 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vswscanf(unsigned long long options, const wchar_t * input, size_t length, const wchar_t * format, _locale_t locale, va_list valist);

#line 990 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __stdio_common_vfwscanf(unsigned long long options, FILE * file, const wchar_t * format, _locale_t locale, va_list valist);

#line 1102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fwscanf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1104 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) swscanf(const wchar_t * __restrict__ _Src, const wchar_t * __restrict__ _Format, ...);

#line 1106 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) wscanf(const wchar_t * __restrict__ _Format, ...);

#line 1108 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int vfwscanf(FILE * __stream, const wchar_t * __format, va_list __local_argv);

#line 1111 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(2))) int vswscanf(const wchar_t * __restrict__ __source, const wchar_t * __restrict__ __format, va_list __local_argv);

#line 1113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__nonnull__(1))) int vwscanf(const wchar_t * __format, va_list __local_argv);

#line 1115 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, ...);

#line 1116 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) wprintf(const wchar_t * __restrict__ _Format, ...);

#line 1117 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vfwprintf(FILE * __restrict__ _File, const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1118 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList);

#line 1150 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfsopen(const wchar_t * _Filename, const wchar_t * _Mode, int _ShFlag);

#line 1153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) fgetwc(FILE * _File);

#line 1154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwchar(void);

#line 1155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) fputwc(wchar_t _Ch, FILE * _File);

#line 1156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwchar(wchar_t _Ch);

#line 1157 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) getwc(FILE * _File);

#line 1158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) getwchar(void);

#line 1159 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) putwc(wchar_t _Ch, FILE * _File);

#line 1160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) putwchar(wchar_t _Ch);

#line 1161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wint_t __attribute__((__cdecl__)) ungetwc(wint_t _Ch, FILE * _File);

#line 1162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
wchar_t * fgetws(wchar_t * __restrict__ _Dst, int _SizeInWords, FILE * __restrict__ _File);

#line 1163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputws(const wchar_t * __restrict__ _Str, FILE * __restrict__ _File);

#line 1164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _getws(wchar_t * _String);

#line 1165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putws(const wchar_t * _Str);

#line 1169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scwprintf(const wchar_t * __restrict__ _Format, ...) {

#line 1171 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_list __ap;

#line 1172 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    int __ret;

#line 1173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_start(__ap, _Format);

#line 1174 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __ret = __stdio_common_vswprintf((*__local_stdio_printf_options()) | 0x0002ULL, ((void *)0), 0, _Format, ((void *)0), __ap);

#line 1175 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_end(__ap);

#line 1176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return __ret;
}

#line 1178 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _snwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1179 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) _vsnwprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1182 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) swprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, ...);

#line 1183 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vswprintf(wchar_t * __restrict__ _Dest, size_t _Count, const wchar_t * __restrict__ _Format, va_list _Args);

#line 1185 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) snwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, ...);

#line 1186 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) vsnwprintf(wchar_t * __restrict__ s, size_t n, const wchar_t * __restrict__ format, va_list arg);

#line 1190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swprintf(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Format, ...) {

#line 1192 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_list __ap;

#line 1193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    int __ret;

#line 1194 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_start(__ap, _Format);

#line 1195 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __ret = __stdio_common_vswprintf((*__local_stdio_printf_options()), _Dest, (size_t)-1, _Format, ((void *)0), __ap);

#line 1196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    __builtin_va_end(__ap);

#line 1197 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return __ret;
}

#line 1200 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswprintf(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Format, va_list _Args) {

#line 1202 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return __stdio_common_vswprintf((*__local_stdio_printf_options()), _Dest, (size_t)-1, _Format, ((void *)0), _Args);
}

#line 1206 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscwprintf(const wchar_t * __restrict__ _Format, va_list _ArgList) {

#line 1208 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    int _Result = __stdio_common_vswprintf((*__local_stdio_printf_options()) | 0x0002ULL, ((void *)0), 0, _Format, ((void *)0), _ArgList);

#line 1209 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
    return _Result < 0 ? -1 : _Result;
}

#line 1249 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtempnam(const wchar_t * _Directory, const wchar_t * _FilePrefix);

#line 1253 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _snwscanf(const wchar_t * __restrict__ _Src, size_t _MaxCount, const wchar_t * __restrict__ _Format, ...);

#line 1254 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfdopen(int _FileHandle, const wchar_t * _Mode);

#line 1255 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode);

#line 1256 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wfreopen(const wchar_t * __restrict__ _Filename, const wchar_t * __restrict__ _Mode, FILE * __restrict__ _OldFile);

#line 1260 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wperror(const wchar_t * _ErrMsg);

#line 1262 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) FILE * _wpopen(const wchar_t * _Command, const wchar_t * _Mode);

#line 1267 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wremove(const wchar_t * _Filename);

#line 1268 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wchar_t * _wtmpnam(wchar_t * _Buffer);

#line 1270 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fgetwc_nolock(FILE * _File);

#line 1271 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _fputwc_nolock(wchar_t _Ch, FILE * _File);

#line 1272 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) wint_t __attribute__((__cdecl__)) _ungetwc_nolock(wint_t _Ch, FILE * _File);

#line 1297 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fgetc_nolock(FILE * _File);

#line 1298 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fputc_nolock(int _Char, FILE * _File);

#line 1299 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _getc_nolock(FILE * _File);

#line 1300 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putc_nolock(int _Char, FILE * _File);

#line 1312 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _lock_file(FILE * _File);

#line 1313 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _unlock_file(FILE * _File);

#line 1315 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fclose_nolock(FILE * _File);

#line 1316 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fflush_nolock(FILE * _File);

#line 1317 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock(void * __restrict__ _DstBuf, size_t _ElementSize, size_t _Count, FILE * __restrict__ _File);

#line 1318 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseek_nolock(FILE * _File, long _Offset, int _Origin);

#line 1319 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _ftell_nolock(FILE * _File);

#line 1320 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _fseeki64_nolock(FILE * _File, long long _Offset, int _Origin);

#line 1321 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _ftelli64_nolock(FILE * _File);

#line 1322 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fwrite_nolock(const void * __restrict__ _DstBuf, size_t _Size, size_t _Count, FILE * __restrict__ _File);

#line 1323 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _ungetc_nolock(int _Ch, FILE * _File);

#line 1334 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
char * tempnam(const char * _Directory, const char * _FilePrefix);

#line 1338 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fcloseall(void);

#line 1339 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
FILE * fdopen(int _FileHandle, const char * _Format);

#line 1340 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fgetchar(void);

#line 1341 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fileno(FILE * _File);

#line 1342 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) flushall(void);

#line 1343 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) fputchar(int _Ch);

#line 1344 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) getw(FILE * _File);

#line 1345 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) putw(int _Ch, FILE * _File);

#line 1346 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) rmtmp(void);

#line 1363 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __mingw_str_wide_utf8(const wchar_t * const wptr, char * * mbptr, size_t * buflen);

#line 1377 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
int __attribute__((__cdecl__)) __mingw_str_utf8_wide(const char * const mbptr, wchar_t * * wptr, size_t * buflen);

#line 1386 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
void __attribute__((__cdecl__)) __mingw_str_free(void * ptr);

#line 1393 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnl(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1394 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnle(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1395 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlp(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1396 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnlpe(int _Mode, const wchar_t * _Filename, const wchar_t * _ArgList, ...);

#line 1397 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnv(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1398 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnve(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1399 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvp(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList);

#line 1400 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _wspawnvpe(int _Mode, const wchar_t * _Filename, const wchar_t * const * _ArgList, const wchar_t * const * _Env);

#line 1417 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnv(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1418 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnve(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 1419 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvp(int _Mode, const char * _Filename, const char * const * _ArgList);

#line 1420 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdio.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _spawnvpe(int _Mode, const char * _Filename, const char * const * _ArgList, const char * const * _Env);

#line 29 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) clearerr_s(FILE * _File);

#line 31 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
size_t __attribute__((__cdecl__)) fread_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 34 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_s(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 35 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsprintf_p(unsigned long long _Options, char * _Str, size_t _Len, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnprintf_s(unsigned long long _Options, char * _Str, size_t _Len, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_s(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 38 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfprintf_p(unsigned long long _Options, FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList);

#line 40 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfscanf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 42 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfscanf(0x0001ULL, _File, _Format, _Locale, _ArgList);
}

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfscanf_s(FILE * _File, const char * _Format, va_list _ArgList) {

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfscanf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscanf_s_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 55 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vscanf_s(const char * _Format, va_list _ArgList) {

#line 57 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);
}

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fscanf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 62 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 64 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 65 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_s_l(_File, _Format, _Locale, _ArgList);

#line 66 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 67 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 70 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fscanf_s(FILE * _File, const char * _Format, ...) {

#line 72 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 73 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 74 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 76 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 77 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 80 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scanf_s_l(const char * _Format, _locale_t _Locale, ...) {

#line 82 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 83 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 84 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 85 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 86 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 87 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 90 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) scanf_s(const char * _Format, ...) {

#line 92 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 93 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 94 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 95 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);

#line 96 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 97 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 100 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfscanf_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfscanf(0, _File, _Format, _Locale, _ArgList);
}

#line 105 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscanf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 107 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfscanf_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 110 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fscanf_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 112 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 114 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 115 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_l(_File, _Format, _Locale, _ArgList);

#line 116 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 117 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 119 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scanf_l(const char * _Format, _locale_t _Locale, ...) {

#line 121 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 122 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 123 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 124 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfscanf_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 125 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 126 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 129 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsscanf_s_l(const char * _Src, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 131 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsscanf(0x0001ULL, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 133 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsscanf_s(const char * _Src, const char * _Format, va_list _ArgList) {

#line 135 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsscanf_s_l(_Src, _Format, ((void *)0), _ArgList);
}

#line 137 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sscanf_s_l(const char * _Src, const char * _Format, _locale_t _Locale, ...) {

#line 139 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 140 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 141 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 142 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsscanf_s_l(_Src, _Format, _Locale, _ArgList);

#line 143 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 144 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 146 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) sscanf_s(const char * _Src, const char * _Format, ...) {

#line 148 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 149 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 150 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 151 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsscanf_s_l(_Src, _Format, ((void *)0), _ArgList);

#line 152 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsscanf_l(const char * _Src, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsscanf(0, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sscanf_l(const char * _Src, const char * _Format, _locale_t _Locale, ...) {

#line 162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsscanf_l(_Src, _Format, _Locale, _ArgList);

#line 166 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 171 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_s_l(const char * _Src, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 174 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 175 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = __stdio_common_vsscanf(0x0001ULL, _Src, _MaxCount, _Format, _Locale, _ArgList);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 178 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 180 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_s(const char * _Src, size_t _MaxCount, const char * _Format, ...) {

#line 182 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 183 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 184 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 185 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = __stdio_common_vsscanf(0x0001ULL, _Src, _MaxCount, _Format, ((void *)0), _ArgList);

#line 186 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 187 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 191 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snscanf_l(const char * _Src, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 194 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 195 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = __stdio_common_vsscanf(0, _Src, _MaxCount, _Format, _Locale, _ArgList);

#line 197 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 198 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 202 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 204 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfprintf_s((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 206 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfprintf_s(FILE * _File, const char * _Format, va_list _ArgList) {

#line 208 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 210 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_s_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 212 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 214 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vprintf_s(const char * _Format, va_list _ArgList) {

#line 216 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 218 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_s_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 220 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 222 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 223 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_s_l(_File, _Format, _Locale, _ArgList);

#line 224 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 225 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 227 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_s_l(const char * _Format, _locale_t _Locale, ...) {

#line 229 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 230 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 231 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 232 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 233 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 234 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 236 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fprintf_s(FILE * _File, const char * _Format, ...) {

#line 238 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 239 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 240 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 241 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 242 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 243 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 245 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) printf_s(const char * _Format, ...) {

#line 247 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 248 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 249 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 250 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 251 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 252 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 255 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_c_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 257 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf((*__local_stdio_printf_options()), _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 259 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_c(char * _DstBuf, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 261 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 263 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_c_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 265 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 266 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 267 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 268 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 269 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 270 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 272 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_c(char * _DstBuf, size_t _MaxCount, const char * _Format, ...) {

#line 274 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 275 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 276 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 277 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnprintf_c_l(_DstBuf, _MaxCount, _Format, ((void *)0), _ArgList);

#line 278 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 279 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 282 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_s_l(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 284 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsnprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);
}

#line 286 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsnprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 288 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 290 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 292 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 294 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_s_l(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 296 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 297 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 298 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 299 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);

#line 300 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 301 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 303 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_s(char * _DstBuf, size_t _DstSize, size_t _MaxCount, const char * _Format, ...) {

#line 305 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 306 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 307 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 308 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);

#line 309 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 310 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 313 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_s_l(char * _DstBuf, size_t _DstSize, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 315 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _Format, _Locale, _ArgList);
}

#line 317 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vsprintf_s(char * _DstBuf, size_t _Size, const char * _Format, va_list _ArgList) {

#line 319 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsprintf_s_l(_DstBuf, _Size, _Format, ((void *)0), _ArgList);
}

#line 321 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_s_l(char * _DstBuf, size_t _DstSize, const char * _Format, _locale_t _Locale, ...) {

#line 323 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 324 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 325 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 326 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsprintf_s_l(_DstBuf, _DstSize, _Format, _Locale, _ArgList);

#line 327 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 328 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 330 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) sprintf_s(char * _DstBuf, size_t _DstSize, const char * _Format, ...) {

#line 332 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 333 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 334 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 335 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);

#line 336 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 337 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 340 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_p_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 342 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfprintf_p((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 344 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_p(FILE * _File, const char * _Format, va_list _ArgList) {

#line 346 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_p_l(_File, _Format, ((void *)0), _ArgList);
}

#line 348 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_p_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 350 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_p_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 352 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_p(const char * _Format, va_list _ArgList) {

#line 354 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_p_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 356 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_p_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 358 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 359 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 360 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 361 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = __stdio_common_vfprintf_p((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);

#line 362 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 363 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 365 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_p(FILE * _File, const char * _Format, ...) {

#line 367 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 368 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 369 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 370 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_p_l(_File, _Format, ((void *)0), _ArgList);

#line 371 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 372 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 374 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_p_l(const char * _Format, _locale_t _Locale, ...) {

#line 376 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 377 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 378 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 379 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_p_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 380 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 381 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 383 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_p(const char * _Format, ...) {

#line 385 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 386 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 387 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 388 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_p_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 389 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 390 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 393 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_p_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 395 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf_p((*__local_stdio_printf_options()), _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 397 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_p(char * _Dst, size_t _MaxCount, const char * _Format, va_list _ArgList) {

#line 399 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsprintf_p_l(_Dst, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 401 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_p_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 403 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 404 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 405 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 406 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsprintf_p_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 407 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 408 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 410 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_p(char * _Dst, size_t _MaxCount, const char * _Format, ...) {

#line 412 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 413 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 414 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 415 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsprintf_p_l(_Dst, _MaxCount, _Format, ((void *)0), _ArgList);

#line 416 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 417 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 420 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_p_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 422 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf_p(0x0002ULL, ((void *)0), 0, _Format, _Locale, _ArgList);
}

#line 424 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_p(const char * _Format, va_list _ArgList) {

#line 426 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vscprintf_p_l(_Format, ((void *)0), _ArgList);
}

#line 428 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_p_l(const char * _Format, _locale_t _Locale, ...) {

#line 430 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 431 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 432 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 433 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vscprintf_p_l(_Format, _Locale, _ArgList);

#line 434 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 435 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 437 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_p(const char * _Format, ...) {

#line 439 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 440 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 441 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 442 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vscprintf_p_l(_Format, ((void *)0), _ArgList);

#line 443 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 444 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 447 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfprintf_l(FILE * _File, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 449 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfprintf((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 451 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vprintf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 453 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfprintf_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 455 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fprintf_l(FILE * _File, const char * _Format, _locale_t _Locale, ...) {

#line 457 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 458 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 459 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 460 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_l(_File, _Format, _Locale, _ArgList);

#line 461 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 462 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 464 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _printf_l(const char * _Format, _locale_t _Locale, ...) {

#line 466 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 467 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 468 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 469 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfprintf_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 470 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 471 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 474 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnprintf_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 476 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf(0x0001ULL, _DstBuf, _MaxCount, _Format, _Locale, _ArgList);
}

#line 478 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snprintf_l(char * _DstBuf, size_t _MaxCount, const char * _Format, _locale_t _Locale, ...) {

#line 480 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 481 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 482 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 483 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnprintf_l(_DstBuf, _MaxCount, _Format, _Locale, _ArgList);

#line 484 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 485 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 487 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsprintf_l(char * _DstBuf, const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 489 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsnprintf_l(_DstBuf, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 491 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _sprintf_l(char * _DstBuf, const char * _Format, _locale_t _Locale, ...) {

#line 493 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 494 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 495 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 496 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsprintf_l(_DstBuf, _Format, _Locale, _ArgList);

#line 497 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 498 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 501 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vscprintf_l(const char * _Format, _locale_t _Locale, va_list _ArgList) {

#line 503 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsprintf(0x0002ULL, ((void *)0), 0, _Format, _Locale, _ArgList);
}

#line 505 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _scprintf_l(const char * _Format, _locale_t _Locale, ...) {

#line 507 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 508 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 509 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 510 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vscprintf_l(_Format, _Locale, _ArgList);

#line 511 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 512 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 589 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) fopen_s(FILE * * _File, const char * _Filename, const char * _Mode);

#line 590 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) freopen_s(FILE * * _File, const char * _Filename, const char * _Mode, FILE * _Stream);

#line 592 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) char * gets_s(char *, rsize_t);

#line 595 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpfile_s(FILE * * _File);

#line 597 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) tmpnam_s(char *, rsize_t);

#line 603 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) wchar_t * _getws_s(wchar_t * _Str, size_t _SizeInWords);

#line 607 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vswprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 608 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vsnwprintf_s(unsigned long long _Options, wchar_t * _Str, size_t _Len, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 609 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
int __attribute__((__cdecl__)) __stdio_common_vfwprintf_s(unsigned long long _Options, FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList);

#line 611 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfwscanf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 613 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfwscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _File, _Format, _Locale, _ArgList);
}

#line 616 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfwscanf_s(FILE * _File, const wchar_t * _Format, va_list _ArgList) {

#line 618 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwscanf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 621 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vwscanf_s_l(const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 623 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);
}

#line 626 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vwscanf_s(const wchar_t * _Format, va_list _ArgList) {

#line 628 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);
}

#line 631 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fwscanf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 633 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 634 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 635 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 636 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwscanf_s_l(_File, _Format, _Locale, _ArgList);

#line 637 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 638 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 641 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fwscanf_s(FILE * _File, const wchar_t * _Format, ...) {

#line 643 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 644 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 645 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 646 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwscanf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 647 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 648 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 651 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _wscanf_s_l(const wchar_t * _Format, _locale_t _Locale, ...) {

#line 653 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 654 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 655 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 656 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwscanf_s_l((__acrt_iob_func(0)), _Format, _Locale, _ArgList);

#line 657 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 658 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 661 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) wscanf_s(const wchar_t * _Format, ...) {

#line 663 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 664 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 665 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 666 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwscanf_s_l((__acrt_iob_func(0)), _Format, ((void *)0), _ArgList);

#line 667 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 668 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 671 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswscanf_s_l(const wchar_t * _Src, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 673 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vswscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _Src, (size_t)-1, _Format, _Locale, _ArgList);
}

#line 676 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vswscanf_s(const wchar_t * _Src, const wchar_t * _Format, va_list _ArgList) {

#line 678 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vswscanf_s_l(_Src, _Format, ((void *)0), _ArgList);
}

#line 681 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swscanf_s_l(const wchar_t * _Src, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 683 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 684 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 685 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 686 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vswscanf_s_l(_Src, _Format, _Locale, _ArgList);

#line 687 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 688 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 690 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) swscanf_s(const wchar_t * _Src, const wchar_t * _Format, ...) {

#line 692 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 693 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 694 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 695 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vswscanf_s_l(_Src, _Format, ((void *)0), _ArgList);

#line 696 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 697 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 700 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwscanf_s_l(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 702 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vswscanf((*__local_stdio_scanf_options()) | 0x0001ULL, _Src, _MaxCount, _Format, _Locale, _ArgList);
}

#line 704 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwscanf_s_l(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 706 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 707 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 708 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 709 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnwscanf_s_l(_Src, _MaxCount, _Format, _Locale, _ArgList);

#line 710 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 711 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 713 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwscanf_s(const wchar_t * _Src, size_t _MaxCount, const wchar_t * _Format, ...) {

#line 715 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 716 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 717 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 718 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnwscanf_s_l(_Src, _MaxCount, _Format, ((void *)0), _ArgList);

#line 719 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 720 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 723 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vfwprintf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 725 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vfwprintf_s((*__local_stdio_printf_options()), _File, _Format, _Locale, _ArgList);
}

#line 727 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vwprintf_s_l(const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 729 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);
}

#line 731 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vfwprintf_s(FILE * _File, const wchar_t * _Format, va_list _ArgList) {

#line 733 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwprintf_s_l(_File, _Format, ((void *)0), _ArgList);
}

#line 735 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vwprintf_s(const wchar_t * _Format, va_list _ArgList) {

#line 737 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vfwprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);
}

#line 739 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _fwprintf_s_l(FILE * _File, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 741 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 742 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 743 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 744 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwprintf_s_l(_File, _Format, _Locale, _ArgList);

#line 745 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 746 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 748 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _wprintf_s_l(const wchar_t * _Format, _locale_t _Locale, ...) {

#line 750 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 751 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 752 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 753 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwprintf_s_l((__acrt_iob_func(1)), _Format, _Locale, _ArgList);

#line 754 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 755 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 757 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) fwprintf_s(FILE * _File, const wchar_t * _Format, ...) {

#line 759 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 760 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 761 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 762 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwprintf_s_l(_File, _Format, ((void *)0), _ArgList);

#line 763 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 764 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 766 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) wprintf_s(const wchar_t * _Format, ...) {

#line 768 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 769 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 770 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 771 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vfwprintf_s_l((__acrt_iob_func(1)), _Format, ((void *)0), _ArgList);

#line 772 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 773 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 776 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vswprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 778 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vswprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _Format, _Locale, _ArgList);
}

#line 780 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) vswprintf_s(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, va_list _ArgList) {

#line 782 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vswprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);
}

#line 784 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _swprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 786 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 787 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 788 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 789 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vswprintf_s_l(_DstBuf, _DstSize, _Format, _Locale, _ArgList);

#line 790 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 791 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 793 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) swprintf_s(wchar_t * _DstBuf, size_t _DstSize, const wchar_t * _Format, ...) {

#line 795 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 796 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 797 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 798 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vswprintf_s_l(_DstBuf, _DstSize, _Format, ((void *)0), _ArgList);

#line 799 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 800 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 803 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, va_list _ArgList) {

#line 805 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return __stdio_common_vsnwprintf_s((*__local_stdio_printf_options()), _DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);
}

#line 807 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _vsnwprintf_s(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, va_list _ArgList) {

#line 809 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);
}

#line 811 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwprintf_s_l(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, _locale_t _Locale, ...) {

#line 813 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 814 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 815 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Locale);

#line 816 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, _Locale, _ArgList);

#line 817 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 818 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 820 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
static __attribute__((__unused__)) __attribute__((__cdecl__)) __inline__ int __attribute__((__cdecl__)) _snwprintf_s(wchar_t * _DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t * _Format, ...) {

#line 822 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_list _ArgList;

#line 823 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    int _Ret;

#line 824 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_start(_ArgList, _Format);

#line 825 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    _Ret = _vsnwprintf_s_l(_DstBuf, _DstSize, _MaxCount, _Format, ((void *)0), _ArgList);

#line 826 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    __builtin_va_end(_ArgList);

#line 827 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
    return _Ret;
}

#line 867 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode);

#line 868 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wfreopen_s(FILE * * _File, const wchar_t * _Filename, const wchar_t * _Mode, FILE * _OldFile);

#line 870 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wtmpnam_s(wchar_t * _DstBuf, size_t _SizeInWords);

#line 912 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdio_s.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _fread_nolock_s(void * _DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE * _File);

#line 7 "sharp-test/unit/cinterop/variadic.ce"
int sum_ints(int count, ...) {

#line 8 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 10 "sharp-test/unit/cinterop/variadic.ce"
    int total = 0;

#line 11 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 12 "sharp-test/unit/cinterop/variadic.ce"
        total = total + __builtin_va_arg(ap ,int);

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 14 "sharp-test/unit/cinterop/variadic.ce"
    return total;
}

#line 18 "sharp-test/unit/cinterop/variadic.ce"
double sum_doubles(int count, ...) {

#line 19 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 21 "sharp-test/unit/cinterop/variadic.ce"
    double total = 0.0;

#line 22 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 23 "sharp-test/unit/cinterop/variadic.ce"
        total = total + __builtin_va_arg(ap ,double);

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 25 "sharp-test/unit/cinterop/variadic.ce"
    return total;
}

#line 29 "sharp-test/unit/cinterop/variadic.ce"
int max_int(int count, ...) {

#line 30 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 32 "sharp-test/unit/cinterop/variadic.ce"
    int result = __builtin_va_arg(ap ,int);

#line 33 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 1; i < count; i = i + 1) {

#line 34 "sharp-test/unit/cinterop/variadic.ce"
        int val = __builtin_va_arg(ap ,int);

#line 35 "sharp-test/unit/cinterop/variadic.ce"
        if (val > result) 
            result = val;
    }

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 38 "sharp-test/unit/cinterop/variadic.ce"
    return result;
}

#line 44 "sharp-test/unit/cinterop/variadic.ce"
int parse_fmt(const char * fmt, ...) {

#line 45 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, fmt);

#line 47 "sharp-test/unit/cinterop/variadic.ce"
    int count = 0;

#line 48 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; fmt[i] != '\0'; i = i + 1) {

#line 49 "sharp-test/unit/cinterop/variadic.ce"
        if (fmt[i] == 'd') {

#line 50 "sharp-test/unit/cinterop/variadic.ce"
            int v = __builtin_va_arg(ap ,int);

#line 51 "sharp-test/unit/cinterop/variadic.ce"
            (void)v;

#line 52 "sharp-test/unit/cinterop/variadic.ce"
            count = count + 1;
        }
        else 

#line 53 "sharp-test/unit/cinterop/variadic.ce"
            if (fmt[i] == 'f') {

#line 54 "sharp-test/unit/cinterop/variadic.ce"
                double v = __builtin_va_arg(ap ,double);

#line 55 "sharp-test/unit/cinterop/variadic.ce"
                (void)v;

#line 56 "sharp-test/unit/cinterop/variadic.ce"
                count = count + 1;
            }
            else 

#line 57 "sharp-test/unit/cinterop/variadic.ce"
                if (fmt[i] == 's') {

#line 58 "sharp-test/unit/cinterop/variadic.ce"
                    const char * v = __builtin_va_arg(ap ,const char *);

#line 59 "sharp-test/unit/cinterop/variadic.ce"
                    (void)v;

#line 60 "sharp-test/unit/cinterop/variadic.ce"
                    count = count + 1;
                }
    }

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 64 "sharp-test/unit/cinterop/variadic.ce"
    return count;
}

#line 68 "sharp-test/unit/cinterop/variadic.ce"
int str_lengths(int count, ...) {

#line 69 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 71 "sharp-test/unit/cinterop/variadic.ce"
    int total = 0;

#line 72 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) {

#line 73 "sharp-test/unit/cinterop/variadic.ce"
        const char * s = __builtin_va_arg(ap ,const char *);

#line 74 "sharp-test/unit/cinterop/variadic.ce"
        int len = 0;

#line 75 "sharp-test/unit/cinterop/variadic.ce"
        while (s[len] != '\0') 
            len = len + 1;

#line 76 "sharp-test/unit/cinterop/variadic.ce"
        total = total + len;
    }

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 79 "sharp-test/unit/cinterop/variadic.ce"
    return total;
}

#line 83 "sharp-test/unit/cinterop/variadic.ce"
int sum_then_max(int count, ...) {

#line 84 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 87 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap2;

#line 11 "/root/code/sharp/zig/lib/include/__stdarg_va_copy.h"
    __builtin_va_copy(ap2, ap);

#line 90 "sharp-test/unit/cinterop/variadic.ce"
    int sum = 0;

#line 91 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 92 "sharp-test/unit/cinterop/variadic.ce"
        sum = sum + __builtin_va_arg(ap ,int);

#line 94 "sharp-test/unit/cinterop/variadic.ce"
    int mx = 0;

#line 95 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) {

#line 96 "sharp-test/unit/cinterop/variadic.ce"
        int val = __builtin_va_arg(ap2 ,int);

#line 97 "sharp-test/unit/cinterop/variadic.ce"
        if (i == 0 || val > mx) 
            mx = val;
    }

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);
    __builtin_va_end(ap2);

#line 102 "sharp-test/unit/cinterop/variadic.ce"
    return sum * 1000 + mx;
}

#line 106 "sharp-test/unit/cinterop/variadic.ce"
int first_arg(int n, ...) {

#line 107 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, n);

#line 109 "sharp-test/unit/cinterop/variadic.ce"
    int result = __builtin_va_arg(ap ,int);

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 111 "sharp-test/unit/cinterop/variadic.ce"
    return result;
}

#line 115 "sharp-test/unit/cinterop/variadic.ce"
long long sum_longs(int count, ...) {

#line 116 "sharp-test/unit/cinterop/variadic.ce"
    va_list ap;

#line 14 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_c23_va_start(ap, count);

#line 118 "sharp-test/unit/cinterop/variadic.ce"
    long long total = 0;

#line 119 "sharp-test/unit/cinterop/variadic.ce"
    for (int i = 0; i < count; i = i + 1) 

#line 120 "sharp-test/unit/cinterop/variadic.ce"
        total = total + __builtin_va_arg(ap ,long long);

#line 19 "/root/code/sharp/zig/lib/include/__stdarg_va_arg.h"
    __builtin_va_end(ap);

#line 122 "sharp-test/unit/cinterop/variadic.ce"
    return total;
}

#line 125 "sharp-test/unit/cinterop/variadic.ce"
int main(void) {

#line 127 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_ints(0) != 0) 
        return 1;

#line 128 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_ints(1, 42) != 42) 
        return 2;

#line 129 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_ints(3, 10, 20, 30) != 60) 
        return 3;

#line 130 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_ints(5, 1, 2, 3, 4, 5) != 15) 
        return 4;

#line 133 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_doubles(0) < -0.001 || sum_doubles(0) > 0.001) 
        return 5;

#line 134 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_doubles(2, 1.5, 2.5) < 3.99 || sum_doubles(2, 1.5, 2.5) > 4.01) 
        return 6;

#line 135 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_doubles(3, 0.1, 0.2, 0.3) < 0.59 || sum_doubles(3, 0.1, 0.2, 0.3) > 0.61) 
        return 7;

#line 138 "sharp-test/unit/cinterop/variadic.ce"
    if (max_int(1, 7) != 7) 
        return 8;

#line 139 "sharp-test/unit/cinterop/variadic.ce"
    if (max_int(3, 5, 9, 3) != 9) 
        return 9;

#line 140 "sharp-test/unit/cinterop/variadic.ce"
    if (max_int(4, -1, -5, -3, -2) != -1) 
        return 10;

#line 143 "sharp-test/unit/cinterop/variadic.ce"
    if (parse_fmt("") != 0) 
        return 11;

#line 144 "sharp-test/unit/cinterop/variadic.ce"
    if (parse_fmt("d", 10) != 1) 
        return 12;

#line 145 "sharp-test/unit/cinterop/variadic.ce"
    if (parse_fmt("ddf", 1, 2, 3.0) != 3) 
        return 13;

#line 146 "sharp-test/unit/cinterop/variadic.ce"
    if (parse_fmt("dsf", 42, "hello", 1.0) != 3) 
        return 14;

#line 147 "sharp-test/unit/cinterop/variadic.ce"
    if (parse_fmt("dds", 1, 2, "abc") != 3) 
        return 15;

#line 150 "sharp-test/unit/cinterop/variadic.ce"
    if (str_lengths(0) != 0) 
        return 16;

#line 151 "sharp-test/unit/cinterop/variadic.ce"
    if (str_lengths(1, "abc") != 3) 
        return 17;

#line 152 "sharp-test/unit/cinterop/variadic.ce"
    if (str_lengths(3, "ab", "cde", "f") != 6) 
        return 18;

#line 156 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_then_max(3, 10, 20, 30) != 60030) 
        return 19;

#line 158 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_then_max(4, 5, 1, 8, 3) != 17008) 
        return 20;

#line 161 "sharp-test/unit/cinterop/variadic.ce"
    if (first_arg(1, 99) != 99) 
        return 21;

#line 162 "sharp-test/unit/cinterop/variadic.ce"
    if (first_arg(3, 77, 88, 99) != 77) 
        return 22;

#line 165 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_longs(0) != 0) 
        return 23;

#line 166 "sharp-test/unit/cinterop/variadic.ce"
    if (sum_longs(2, 1000000000LL, 2000000000LL) != 3000000000LL) 
        return 24;

#line 169 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("%d\n", 42) < 1) 
        return 25;

#line 170 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("%s %d\n", "hello", 123) < 1) 
        return 26;

#line 171 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("%f\n", 3.14) < 1) 
        return 27;

#line 172 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("%c %x %o\n", 'A', 255, 8) < 1) 
        return 28;

#line 173 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("%ld %lld\n", (long)100, (long long)999) < 1) 
        return 29;

#line 176 "sharp-test/unit/cinterop/variadic.ce"
    if (printf("") != 0) 
        return 30;

#line 178 "sharp-test/unit/cinterop/variadic.ce"
    return 0;
}
