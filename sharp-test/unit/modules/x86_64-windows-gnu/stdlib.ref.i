
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

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) void * _memccpy(void * _Dst, const void * _Src, int _Val, size_t _MaxCount);

#line 46 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memchr(const void * _Buf, int _Val, size_t _MaxCount);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 48 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp_l(const void * _Buf1, const void * _Buf2, size_t _Size, _locale_t _Locale);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) memcmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memcpy(void * __restrict__ _Dst, const void * __restrict__ _Src, size_t _Size);

#line 51 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memcpy_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * mempcpy(void * _Dst, const void * _Src, size_t _Size);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memset(void * _Dst, int _Val, size_t _Size);

#line 55 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memccpy(void * _Dst, const void * _Src, int _Val, size_t _Size);

#line 56 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 59 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * _strset(char * _Str, int _Val);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * _strset_l(char * _Str, int _Val, _locale_t _Locale);

#line 61 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strcpy(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 62 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strcat(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcmp(const char * _Str1, const char * _Str2);

#line 64 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strlen(const char * _Str);

#line 65 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strnlen(const char * _Str, size_t _MaxCount);

#line 66 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memmove(void * _Dst, const void * _Src, size_t _Size);

#line 71 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strdup(const char * _Src);

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strchr(const char * _Str, int _Val);

#line 76 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp(const char * _Str1, const char * _Str2);

#line 77 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcmpi(const char * _Str1, const char * _Str2);

#line 78 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 79 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcoll(const char * _Str1, const char * _Str2);

#line 80 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 81 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll(const char * _Str1, const char * _Str2);

#line 82 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 83 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 84 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 85 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 86 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 87 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strcspn(const char * _Str, const char * _Control);

#line 88 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strerror(const char * _ErrMsg);

#line 89 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strerror(int);

#line 90 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strlwr(char * _String);

#line 91 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strlwr_l(char * _String, _locale_t _Locale);

#line 92 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strncat(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 93 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strncmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 94 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 95 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 96 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strncpy(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 97 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strnset(char * _Str, int _Val, size_t _MaxCount);

#line 98 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strnset_l(char * str, int c, size_t count, _locale_t _Locale);

#line 99 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strpbrk(const char * _Str, const char * _Control);

#line 100 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strrchr(const char * _Str, int _Ch);

#line 101 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strrev(char * _Str);

#line 102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strspn(const char * _Str, const char * _Control);

#line 103 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strstr(const char * _Str, const char * _SubStr);

#line 104 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strtok(char * __restrict__ _Str, const char * __restrict__ _Delim);

#line 107 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strtok_r(char * __restrict__ _Str, const char * __restrict__ _Delim, char * * __restrict__ __last);

#line 109 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strupr(char * _String);

#line 110 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strupr_l(char * _String, _locale_t _Locale);

#line 111 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strxfrm(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount);

#line 112 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _strxfrm_l(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 119 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strdup(const char * _Src);

#line 123 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcmpi(const char * _Str1, const char * _Str2);

#line 124 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) stricmp(const char * _Str1, const char * _Str2);

#line 125 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strlwr(char * _Str);

#line 126 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strnicmp(const char * _Str1, const char * _Str, size_t _MaxCount);

#line 127 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strncasecmp(const char *, const char *, size_t);

#line 128 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcasecmp(const char *, const char *);

#line 136 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strnset(char * _Str, int _Val, size_t _MaxCount);

#line 137 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strrev(char * _Str);

#line 138 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strset(char * _Str, int _Val);

#line 139 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strupr(char * _Str);

#line 149 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsdup(const wchar_t * _Str);

#line 153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcscat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcschr(const wchar_t * _Str, wchar_t _Ch);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcscmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcscpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 157 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcscspn(const wchar_t * _Str, const wchar_t * _Control);

#line 158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcslen(const wchar_t * _Str);

#line 159 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsnlen(const wchar_t * _Src, size_t _MaxCount);

#line 160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsncat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsncmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsncpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * _wcsncpy_l(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count, _locale_t _Locale);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcspbrk(const wchar_t * _Str, const wchar_t * _Control);

#line 165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsrchr(const wchar_t * _Str, wchar_t _Ch);

#line 166 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsspn(const wchar_t * _Str, const wchar_t * _Control);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsstr(const wchar_t * _Str, const wchar_t * _SubStr);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim, wchar_t * * __restrict__ _Ptr);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * _wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim);

#line 176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcserror(int _ErrNum);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * __wcserror(const wchar_t * _Str);

#line 178 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 179 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 180 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 181 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 182 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 183 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsrev(wchar_t * _Str);

#line 184 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsset(wchar_t * _Str, wchar_t _Val);

#line 185 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr(wchar_t * _String);

#line 186 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr_l(wchar_t * _String, _locale_t _Locale);

#line 187 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr(wchar_t * _String);

#line 188 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr_l(wchar_t * _String, _locale_t _Locale);

#line 189 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsxfrm(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount);

#line 190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcsxfrm_l(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 191 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcscoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 192 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcscoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 194 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 195 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 197 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 198 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 205 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsdup(const wchar_t * _Str);

#line 210 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 211 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 212 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 213 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsrev(wchar_t * _Str);

#line 214 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsset(wchar_t * _Str, wchar_t _Val);

#line 215 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcslwr(wchar_t * _Str);

#line 216 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsupr(wchar_t * _Str);

#line 217 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strset_s(char * _Dst, size_t _DstSize, int _Value);

#line 26 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strerror_s(char * _Buf, size_t _SizeInBytes, const char * _ErrMsg);

#line 28 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) strerror_s(char * _Buf, size_t _SizeInBytes, int _ErrNum);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s(char * _Str, size_t _Size);

#line 32 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 34 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strnset_s(char * _Str, size_t _Size, int _Val, size_t _MaxCount);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s(char * _Str, size_t _Size);

#line 38 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 41 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncat_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 43 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncat_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcpy_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncpy_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncpy_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 51 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) char * strtok_s(char * _Str, const char * _Delim, char * * _Context);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) char * _strtok_s_l(char * _Str, const char * _Delim, char * * _Context, _locale_t _Locale);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcat_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memmove_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) wchar_t * wcstok_s(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context);

#line 64 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcserror_s(wchar_t * _Buf, size_t _SizeInWords, int _ErrNum);

#line 66 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) __wcserror_s(wchar_t * _Buffer, size_t _SizeInWords, const wchar_t * _ErrMsg);

#line 67 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s(wchar_t * _Dst, size_t _DstSizeInWords, wchar_t _Val, size_t _MaxCount);

#line 68 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s(wchar_t * _Str, size_t _SizeInWords, wchar_t _Val);

#line 69 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s(wchar_t * _Str, size_t _SizeInWords);

#line 71 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s_l(wchar_t * _Str, size_t _SizeInWords, _locale_t _Locale);

#line 73 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s(wchar_t * _Str, size_t _Size);

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s_l(wchar_t * _Str, size_t _Size, _locale_t _Locale);

#line 78 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscpy_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 80 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscat_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 83 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncat_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 85 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncat_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 87 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncpy_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 89 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncpy_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 91 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) wchar_t * _wcstok_s_l(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context, _locale_t _Locale);

#line 92 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, _locale_t _Locale);

#line 94 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, size_t _Count, _locale_t _Locale);

#line 19 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wdupenv_s(wchar_t * * _Buffer, size_t * _BufferSizeInWords, const wchar_t * _VarName);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itow_s(int _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 27 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltow_s(long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultow_s(unsigned long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 33 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wgetenv_s(size_t * _ReturnSize, wchar_t * _DstBuf, size_t _DstSizeInWords, const wchar_t * _VarName);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64tow_s(long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64tow_s(unsigned long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 39 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wmakepath_s(wchar_t * _PathResult, size_t _SizeInWords, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 42 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wputenv_s(const wchar_t * _Name, const wchar_t * _Value);

#line 44 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsearchenv_s(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath, size_t _SizeInWords);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsplitpath_s(const wchar_t * _FullPath, wchar_t * _Drive, size_t _DriveSizeInWords, wchar_t * _Dir, size_t _DirSizeInWords, wchar_t * _Filename, size_t _FilenameSizeInWords, wchar_t * _Ext, size_t _ExtSizeInWords);

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef int (*_onexit_t)(void);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct _div_t {
    int quot;
    int rem;
} div_t;

#line 65 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 68 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct _ldiv_t {
    long quot;
    long rem;
} ldiv_t;

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 77 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    unsigned char ld[10];
} _LDOUBLE;

#line 82 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 84 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    double x;
} _CRT_DOUBLE;

#line 86 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 88 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    float f;
} _CRT_FLOAT;

#line 93 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 95 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    long double x;
} _LONGDOUBLE;

#line 100 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"

#line 102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    unsigned char ld12[12];
} _LDBL12;

#line 113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) ___mb_cur_max_func(void);

#line 135 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef void (*_purecall_handler)(void);

#line 137 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_set_purecall_handler(_purecall_handler _Handler))(void);

#line 138 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_get_purecall_handler(void))(void);

#line 140 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef void (*_invalid_parameter_handler)(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t);

#line 141 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_set_invalid_parameter_handler(_invalid_parameter_handler _Handler))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 142 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_get_invalid_parameter_handler(void))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 146 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) extern int * _errno(void);

#line 148 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _set_errno(int _Value);

#line 149 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_errno(int * _Value);

#line 151 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long * __doserrno(void);

#line 153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _set_doserrno(unsigned long _Value);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_doserrno(unsigned long * _Value);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * __sys_errlist(void);

#line 156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __sys_nerr(void);

#line 160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * * __p___argv(void);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __p__fmode(void);

#line 162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __p___argc(void);

#line 163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p___wargv(void);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * __p__pgmptr(void);

#line 165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * __p__wpgmptr(void);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_pgmptr(char * * _Value);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_wpgmptr(wchar_t * * _Value);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_fmode(int _Mode);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_fmode(int * _PMode);

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * * __p__environ(void);

#line 222 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p__wenviron(void);

#line 234 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osplatform(void);

#line 235 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osver(void);

#line 236 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winver(void);

#line 237 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winmajor(void);

#line 238 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winminor(void);

#line 256 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osplatform(unsigned int * _Value);

#line 257 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osver(unsigned int * _Value);

#line 258 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winver(unsigned int * _Value);

#line 259 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winmajor(unsigned int * _Value);

#line 260 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winminor(unsigned int * _Value);

#line 274 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) exit(int _Code) __attribute__((__noreturn__));

#line 275 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) _exit(int _Code) __attribute__((__noreturn__));

#line 277 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) quick_exit(int _Code) __attribute__((__noreturn__));

#line 282 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) _Exit(int) __attribute__((__noreturn__));

#line 291 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) abort(void);

#line 296 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_abort_behavior(unsigned int _Flags, unsigned int _Mask);

#line 300 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) abs(int _X);

#line 301 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) labs(long _X);

#line 304 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) _abs64(long long);

#line 311 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) atexit(void (*)(void));

#line 313 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) at_quick_exit(void (*)(void));

#line 317 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) atof(const char * _String);

#line 318 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) _atof_l(const char * _String, _locale_t _Locale);

#line 320 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) atoi(const char * _Str);

#line 321 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoi_l(const char * _Str, _locale_t _Locale);

#line 322 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) atol(const char * _Str);

#line 323 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _atol_l(const char * _Str, _locale_t _Locale);

#line 326 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * bsearch(const void * _Key, const void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 327 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) qsort(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 329 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned short __attribute__((__cdecl__)) _byteswap_ushort(unsigned short _Short);

#line 330 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _byteswap_ulong(unsigned long _Long);

#line 331 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _byteswap_uint64(unsigned long long _Int64);

#line 332 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
div_t __attribute__((__cdecl__)) div(int _Numerator, int _Denominator);

#line 333 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * getenv(const char * _VarName);

#line 334 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _itoa(int _Value, char * _Dest, int _Radix);

#line 335 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _i64toa(long long _Val, char * _DstBuf, int _Radix);

#line 336 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ui64toa(unsigned long long _Val, char * _DstBuf, int _Radix);

#line 337 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64(const char * _String);

#line 338 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64_l(const char * _String, _locale_t _Locale);

#line 339 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64(const char * _String, char * * _EndPtr, int _Radix);

#line 340 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 341 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64(const char * _String, char * * _EndPtr, int _Radix);

#line 342 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 343 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
ldiv_t __attribute__((__cdecl__)) ldiv(long _Numerator, long _Denominator);

#line 344 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ltoa(long _Value, char * _Dest, int _Radix);

#line 345 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mblen(const char * _Ch, size_t _MaxCount);

#line 346 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mblen_l(const char * _Ch, size_t _MaxCount, _locale_t _Locale);

#line 347 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen(const char * _Str);

#line 348 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen_l(const char * _Str, _locale_t _Locale);

#line 349 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen(const char * _Str, size_t _MaxCount);

#line 350 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen_l(const char * _Str, size_t _MaxCount, _locale_t _Locale);

#line 351 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mbtowc(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes);

#line 352 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mbtowc_l(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes, _locale_t _Locale);

#line 353 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
size_t __attribute__((__cdecl__)) mbstowcs(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount);

#line 354 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstowcs_l(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 355 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mkstemp(char * template_name);

#line 356 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) rand(void);

#line 357 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_error_mode(int _Mode);

#line 358 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) srand(unsigned int _Seed);

#line 385 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtod(const char * __restrict__ _Str, char * * __restrict__ _EndPtr);

#line 386 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtof(const char * __restrict__ nptr, char * * __restrict__ endptr);

#line 388 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtold(const char * __restrict__, char * * __restrict__);

#line 392 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
extern double __attribute__((__cdecl__)) __attribute__((__nothrow__)) __strtod(const char * __restrict__, char * * __restrict__);

#line 400 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __mingw_strtof(const char * __restrict__, char * * __restrict__);

#line 401 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __mingw_strtod(const char * __restrict__, char * * __restrict__);

#line 402 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __mingw_strtold(const char * __restrict__, char * * __restrict__);

#line 404 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _strtof_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 405 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _strtod_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 406 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) strtol(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 407 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _strtol_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 408 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) strtoul(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 409 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _strtoul_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 412 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) system(const char * _Command);

#line 414 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ultoa(unsigned long _Value, char * _Dest, int _Radix);

#line 415 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) wctomb(char * _MbCh, wchar_t _WCh);

#line 416 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wctomb_l(char * _MbCh, wchar_t _WCh, _locale_t _Locale);

#line 417 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
size_t __attribute__((__cdecl__)) wcstombs(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount);

#line 418 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcstombs_l(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 452 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * calloc(size_t _NumOfElements, size_t _SizeOfElements);

#line 453 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) free(void * _Memory);

#line 454 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * malloc(size_t _Size);

#line 455 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * realloc(void * _Memory, size_t _NewSize);

#line 456 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _aligned_free(void * _Memory);

#line 457 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_malloc(size_t _Size, size_t _Alignment);

#line 458 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset);

#line 459 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_realloc(void * _Memory, size_t _Size, size_t _Alignment);

#line 460 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 461 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _recalloc(void * _Memory, size_t _Count, size_t _Size);

#line 462 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment);

#line 463 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment, size_t _Offset);

#line 464 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _aligned_msize(void * _Memory, size_t _Alignment, size_t _Offset);

#line 487 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _itow(int _Value, wchar_t * _Dest, int _Radix);

#line 488 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ltow(long _Value, wchar_t * _Dest, int _Radix);

#line 489 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ultow(unsigned long _Value, wchar_t * _Dest, int _Radix);

#line 491 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __mingw_wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 492 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __mingw_wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 493 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __mingw_wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 506 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 507 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 510 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 512 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wcstod_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 513 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _wcstof_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 514 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) wcstol(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 515 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wcstol_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 516 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) wcstoul(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 517 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _wcstoul_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 518 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wgetenv(const wchar_t * _VarName);

#line 521 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wsystem(const wchar_t * _Command);

#line 523 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof(const wchar_t * _Str);

#line 524 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof_l(const wchar_t * _Str, _locale_t _Locale);

#line 525 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi(const wchar_t * _Str);

#line 526 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi_l(const wchar_t * _Str, _locale_t _Locale);

#line 527 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol(const wchar_t * _Str);

#line 528 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol_l(const wchar_t * _Str, _locale_t _Locale);

#line 530 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _i64tow(long long _Val, wchar_t * _DstBuf, int _Radix);

#line 531 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ui64tow(unsigned long long _Val, wchar_t * _DstBuf, int _Radix);

#line 532 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64(const wchar_t * _Str);

#line 533 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64_l(const wchar_t * _Str, _locale_t _Locale);

#line 534 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 535 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 536 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 537 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 540 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putenv(const char * _EnvString);

#line 541 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wputenv(const wchar_t * _EnvString);

#line 550 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _fullpath(char * _FullPath, const char * _Path, size_t _SizeInBytes);

#line 555 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 556 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 557 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 558 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl(_CRT_DOUBLE * _Result, char * _Str);

#line 559 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl(_LDOUBLE * _Result, char * _Str);

#line 560 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt(_CRT_FLOAT * _Result, char * _Str);

#line 561 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl_l(_CRT_DOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 562 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl_l(_LDOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 563 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt_l(_CRT_FLOAT * _Result, char * _Str, _locale_t _Locale);

#line 579 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotl(unsigned long, int);

#line 580 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotr(unsigned long, int);

#line 586 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _makepath(char * _Path, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 587 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int (*_onexit(_onexit_t _Func))(void);

#line 597 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotl64(unsigned long long _Val, int _Shift);

#line 598 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long Value, int Shift);

#line 605 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotr(unsigned int _Val, int _Shift);

#line 606 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotl(unsigned int _Val, int _Shift);

#line 609 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long _Val, int _Shift);

#line 610 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _searchenv(const char * _Filename, const char * _EnvVar, char * _ResultPath);

#line 611 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _splitpath(const char * _FullPath, char * _Drive, char * _Dir, char * _Filename, char * _Ext);

#line 612 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 620 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wfullpath(wchar_t * _FullPath, const wchar_t * _Path, size_t _SizeInWords);

#line 624 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wmakepath(wchar_t * _ResultPath, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 629 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsearchenv(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath);

#line 630 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsplitpath(const wchar_t * _FullPath, wchar_t * _Drive, wchar_t * _Dir, wchar_t * _Filename, wchar_t * _Ext);

#line 633 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _beep(unsigned _Frequency, unsigned _Duration) __attribute__((__deprecated__));

#line 635 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _seterrormode(int _Mode) __attribute__((__deprecated__));

#line 636 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _sleep(unsigned long _Duration) __attribute__((__deprecated__));

#line 657 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 658 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 659 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 660 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * itoa(int _Val, char * _DstBuf, int _Radix);

#line 661 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ltoa(long _Val, char * _DstBuf, int _Radix);

#line 662 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) putenv(const char * _EnvString);

#line 666 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 669 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ultoa(unsigned long _Val, char * _Dstbuf, int _Radix);

#line 670 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int (*onexit(_onexit_t _Func))(void);

#line 676 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
typedef struct {
    long long quot, rem;
} lldiv_t;

#line 678 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
lldiv_t __attribute__((__cdecl__)) lldiv(long long, long long);

#line 680 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) llabs(long long);

#line 685 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) strtoll(const char * __restrict__, char * * __restrict, int);

#line 686 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) strtoull(const char * __restrict__, char * * __restrict__, int);

#line 689 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) atoll(const char *);

#line 19 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _dupenv_s(char * * _PBuffer, size_t * _PBufferSizeInBytes, const char * _VarName);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) void * bsearch_s(const void * _Key, const void * _Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 25 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) getenv_s(size_t * _ReturnSize, char * _DstBuf, rsize_t _DstSize, const char * _VarName);

#line 27 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix);

#line 29 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64toa_s(long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64toa_s(unsigned long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 31 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltoa_s(long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 33 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) mbstowcs_s(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount);

#line 35 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _mbstowcs_s_l(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount, _locale_t _Locale);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultoa_s(unsigned long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 39 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wctomb_s(int * _SizeConverted, char * _MbCh, rsize_t _SizeInBytes, wchar_t _WCh);

#line 40 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wctomb_s_l(int * _SizeConverted, char * _MbCh, size_t _SizeInBytes, wchar_t _WCh, _locale_t _Locale);

#line 41 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcstombs_s(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes);

#line 43 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcstombs_s_l(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes, _locale_t _Locale);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ecvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDights, int * _PtDec, int * _PtSign);

#line 48 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _fcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _gcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDigits);

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _makepath_s(char * _PathResult, size_t _Size, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _putenv_s(const char * _Name, const char * _Value);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _searchenv_s(const char * _Filename, const char * _EnvVar, char * _ResultPath, size_t _SizeInBytes);

#line 55 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _splitpath_s(const char * _FullPath, char * _Drive, size_t _DriveSize, char * _Dir, size_t _DirSize, char * _Filename, size_t _FilenameSize, char * _Ext, size_t _ExtSize);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) qsort_s(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"

#line 56 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
typedef struct _heapinfo {
    int * _pentry;
    size_t _size;
    int _useflag;
} _HEAPINFO;

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) unsigned int * __p__amblksiz(void);

#line 129 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_malloc(size_t _Size, size_t _Alignment);

#line 130 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void __mingw_aligned_free(void * _Memory);

#line 131 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 132 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_offset_malloc(size_t, size_t, size_t);

#line 133 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_realloc(void * _Memory, size_t _Size, size_t _Offset);

#line 134 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
size_t __mingw_aligned_msize(void * memblock, size_t alignment, size_t offset);

#line 144 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _resetstkoflw(void);

#line 146 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _set_malloc_crt_max_wait(unsigned long _NewValue);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) void * _expand(void * _Memory, size_t _NewSize);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _msize(void * _Memory);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _get_sbh_threshold(void);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_sbh_threshold(size_t _NewValue);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_amblksiz(size_t _Value);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_amblksiz(size_t * _Value);

#line 171 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapadd(void * _Memory, size_t _Size);

#line 172 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapchk(void);

#line 173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapmin(void);

#line 174 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapset(unsigned int _Fill);

#line 175 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapwalk(_HEAPINFO * _EntryInfo);

#line 176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _heapused(size_t * _Used, size_t * _Commit);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _get_heap_handle(void);

#line 9 "sharp-test/unit/modules/stdlib.ce"
typedef long isize;

#line 12 "sharp-test/unit/modules/stdlib.ce"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 16 "sharp-test/unit/modules/stdlib.ce"
isize Str__size(Str * this);

#line 17 "sharp-test/unit/modules/stdlib.ce"
bool Str__eq(Str * this, Str other);

#line 31 "sharp-test/unit/modules/stdlib.ce"
typedef struct String String;
struct String {
    char * ptr;
    isize len;
    isize cap;
};

#line 36 "sharp-test/unit/modules/stdlib.ce"
void String__push_byte(String * this, char c);

#line 46 "sharp-test/unit/modules/stdlib.ce"
isize String__size(String * this);

#line 47 "sharp-test/unit/modules/stdlib.ce"
void String__destroy(String * this);

#line 58 "sharp-test/unit/modules/stdlib.ce"
typedef struct Vec_int Vec_int;
struct Vec_int {
    int * data;
    isize len;
    isize cap;
};

#line 63 "sharp-test/unit/modules/stdlib.ce"
void Vec_int__push(Vec_int * this, int val);

#line 73 "sharp-test/unit/modules/stdlib.ce"
int Vec_int__pop(Vec_int * this);

#line 78 "sharp-test/unit/modules/stdlib.ce"
int Vec_int__get(Vec_int * this, isize i);

#line 79 "sharp-test/unit/modules/stdlib.ce"
isize Vec_int__size(Vec_int * this);

#line 80 "sharp-test/unit/modules/stdlib.ce"
void Vec_int__destroy(Vec_int * this);

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

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) void * _memccpy(void * _Dst, const void * _Src, int _Val, size_t _MaxCount);

#line 46 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memchr(const void * _Buf, int _Val, size_t _MaxCount);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 48 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _memicmp_l(const void * _Buf1, const void * _Buf2, size_t _Size, _locale_t _Locale);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) memcmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memcpy(void * __restrict__ _Dst, const void * __restrict__ _Src, size_t _Size);

#line 51 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memcpy_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * mempcpy(void * _Dst, const void * _Src, size_t _Size);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memset(void * _Dst, int _Val, size_t _Size);

#line 55 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memccpy(void * _Dst, const void * _Src, int _Val, size_t _Size);

#line 56 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) memicmp(const void * _Buf1, const void * _Buf2, size_t _Size);

#line 59 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * _strset(char * _Str, int _Val);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * _strset_l(char * _Str, int _Val, _locale_t _Locale);

#line 61 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strcpy(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 62 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strcat(char * __restrict__ _Dest, const char * __restrict__ _Source);

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcmp(const char * _Str1, const char * _Str2);

#line 64 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strlen(const char * _Str);

#line 65 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strnlen(const char * _Str, size_t _MaxCount);

#line 66 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
void * memmove(void * _Dst, const void * _Src, size_t _Size);

#line 71 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strdup(const char * _Src);

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strchr(const char * _Str, int _Val);

#line 76 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp(const char * _Str1, const char * _Str2);

#line 77 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcmpi(const char * _Str1, const char * _Str2);

#line 78 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricmp_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 79 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcoll(const char * _Str1, const char * _Str2);

#line 80 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strcoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 81 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll(const char * _Str1, const char * _Str2);

#line 82 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _stricoll_l(const char * _Str1, const char * _Str2, _locale_t _Locale);

#line 83 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 84 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strncoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 85 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 86 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicoll_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 87 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strcspn(const char * _Str, const char * _Control);

#line 88 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strerror(const char * _ErrMsg);

#line 89 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strerror(int);

#line 90 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strlwr(char * _String);

#line 91 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strlwr_l(char * _String, _locale_t _Locale);

#line 92 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strncat(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 93 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strncmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 94 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp(const char * _Str1, const char * _Str2, size_t _MaxCount);

#line 95 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _strnicmp_l(const char * _Str1, const char * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 96 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strncpy(char * __restrict__ _Dest, const char * __restrict__ _Source, size_t _Count);

#line 97 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strnset(char * _Str, int _Val, size_t _MaxCount);

#line 98 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strnset_l(char * str, int c, size_t count, _locale_t _Locale);

#line 99 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strpbrk(const char * _Str, const char * _Control);

#line 100 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strrchr(const char * _Str, int _Ch);

#line 101 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strrev(char * _Str);

#line 102 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strspn(const char * _Str, const char * _Control);

#line 103 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strstr(const char * _Str, const char * _SubStr);

#line 104 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strtok(char * __restrict__ _Str, const char * __restrict__ _Delim);

#line 107 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strtok_r(char * __restrict__ _Str, const char * __restrict__ _Delim, char * * __restrict__ __last);

#line 109 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strupr(char * _String);

#line 110 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) char * _strupr_l(char * _String, _locale_t _Locale);

#line 111 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) strxfrm(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount);

#line 112 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _strxfrm_l(char * __restrict__ _Dst, const char * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 119 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strdup(const char * _Src);

#line 123 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcmpi(const char * _Str1, const char * _Str2);

#line 124 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) stricmp(const char * _Str1, const char * _Str2);

#line 125 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strlwr(char * _Str);

#line 126 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strnicmp(const char * _Str1, const char * _Str, size_t _MaxCount);

#line 127 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strncasecmp(const char *, const char *, size_t);

#line 128 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) strcasecmp(const char *, const char *);

#line 136 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strnset(char * _Str, int _Val, size_t _MaxCount);

#line 137 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strrev(char * _Str);

#line 138 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strset(char * _Str, int _Val);

#line 139 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
char * strupr(char * _Str);

#line 149 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsdup(const wchar_t * _Str);

#line 153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcscat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcschr(const wchar_t * _Str, wchar_t _Ch);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcscmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcscpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source);

#line 157 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcscspn(const wchar_t * _Str, const wchar_t * _Control);

#line 158 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcslen(const wchar_t * _Str);

#line 159 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsnlen(const wchar_t * _Src, size_t _MaxCount);

#line 160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsncat(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsncmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsncpy(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count);

#line 163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * _wcsncpy_l(wchar_t * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _Count, _locale_t _Locale);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcspbrk(const wchar_t * _Str, const wchar_t * _Control);

#line 165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsrchr(const wchar_t * _Str, wchar_t _Ch);

#line 166 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsspn(const wchar_t * _Str, const wchar_t * _Control);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsstr(const wchar_t * _Str, const wchar_t * _SubStr);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim, wchar_t * * __restrict__ _Ptr);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * _wcstok(wchar_t * __restrict__ _Str, const wchar_t * __restrict__ _Delim);

#line 176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcserror(int _ErrNum);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * __wcserror(const wchar_t * _Str);

#line 178 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 179 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 180 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 181 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicmp_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 182 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 183 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsrev(wchar_t * _Str);

#line 184 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsset(wchar_t * _Str, wchar_t _Val);

#line 185 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr(wchar_t * _String);

#line 186 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcslwr_l(wchar_t * _String, _locale_t _Locale);

#line 187 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr(wchar_t * _String);

#line 188 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) wchar_t * _wcsupr_l(wchar_t * _String, _locale_t _Locale);

#line 189 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
size_t __attribute__((__cdecl__)) wcsxfrm(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount);

#line 190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcsxfrm_l(wchar_t * __restrict__ _Dst, const wchar_t * __restrict__ _Src, size_t _MaxCount, _locale_t _Locale);

#line 191 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcscoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 192 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcscoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 194 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, _locale_t _Locale);

#line 195 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 196 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsncoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 197 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 198 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wcsnicoll_l(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount, _locale_t _Locale);

#line 205 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsdup(const wchar_t * _Str);

#line 210 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsicmp(const wchar_t * _Str1, const wchar_t * _Str2);

#line 211 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsnicmp(const wchar_t * _Str1, const wchar_t * _Str2, size_t _MaxCount);

#line 212 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsnset(wchar_t * _Str, wchar_t _Val, size_t _MaxCount);

#line 213 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsrev(wchar_t * _Str);

#line 214 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsset(wchar_t * _Str, wchar_t _Val);

#line 215 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcslwr(wchar_t * _Str);

#line 216 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
wchar_t * wcsupr(wchar_t * _Str);

#line 217 "/root/code/sharp/zig/lib/libc/include/any-windows-any/string.h"
int __attribute__((__cdecl__)) wcsicoll(const wchar_t * _Str1, const wchar_t * _Str2);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strset_s(char * _Dst, size_t _DstSize, int _Value);

#line 26 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strerror_s(char * _Buf, size_t _SizeInBytes, const char * _ErrMsg);

#line 28 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) strerror_s(char * _Buf, size_t _SizeInBytes, int _ErrNum);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s(char * _Str, size_t _Size);

#line 32 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strlwr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 34 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strnset_s(char * _Str, size_t _Size, int _Val, size_t _MaxCount);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s(char * _Str, size_t _Size);

#line 38 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strupr_s_l(char * _Str, size_t _Size, _locale_t _Locale);

#line 41 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncat_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 43 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncat_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 45 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcpy_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strncpy_s(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _strncpy_s_l(char * _Dst, size_t _DstSizeInChars, const char * _Src, size_t _MaxCount, _locale_t _Locale);

#line 51 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) char * strtok_s(char * _Str, const char * _Delim, char * * _Context);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) char * _strtok_s_l(char * _Str, const char * _Delim, char * * _Context, _locale_t _Locale);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) strcat_s(char * _Dst, rsize_t _SizeInBytes, const char * _Src);

#line 56 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) size_t __attribute__((__cdecl__)) strnlen_s(const char * _src, size_t _count) {

#line 57 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
    return _src ? strnlen(_src, _count) : 0;
}

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((dllimport)) errno_t __attribute__((__cdecl__)) memmove_s(void * _dest, size_t _numberOfElements, const void * _src, size_t _count);

#line 63 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) wchar_t * wcstok_s(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context);

#line 64 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcserror_s(wchar_t * _Buf, size_t _SizeInWords, int _ErrNum);

#line 66 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) __wcserror_s(wchar_t * _Buffer, size_t _SizeInWords, const wchar_t * _ErrMsg);

#line 67 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s(wchar_t * _Dst, size_t _DstSizeInWords, wchar_t _Val, size_t _MaxCount);

#line 68 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s(wchar_t * _Str, size_t _SizeInWords, wchar_t _Val);

#line 69 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s(wchar_t * _Str, size_t _SizeInWords);

#line 71 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcslwr_s_l(wchar_t * _Str, size_t _SizeInWords, _locale_t _Locale);

#line 73 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s(wchar_t * _Str, size_t _Size);

#line 75 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsupr_s_l(wchar_t * _Str, size_t _Size, _locale_t _Locale);

#line 78 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscpy_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 80 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcscat_s(wchar_t * _Dst, rsize_t _SizeInWords, const wchar_t * _Src);

#line 83 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncat_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 85 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncat_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 87 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcsncpy_s(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount);

#line 89 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsncpy_s_l(wchar_t * _Dst, size_t _DstSizeInChars, const wchar_t * _Src, size_t _MaxCount, _locale_t _Locale);

#line 91 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) wchar_t * _wcstok_s_l(wchar_t * _Str, const wchar_t * _Delim, wchar_t * * _Context, _locale_t _Locale);

#line 92 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, _locale_t _Locale);

#line 94 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcsnset_s_l(wchar_t * _Str, size_t _SizeInChars, wchar_t _Val, size_t _Count, _locale_t _Locale);

#line 97 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) size_t __attribute__((__cdecl__)) wcsnlen_s(const wchar_t * _src, size_t _count) {

#line 98 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/string_s.h"
    return _src ? wcsnlen(_src, _count) : 0;
}

#line 19 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wdupenv_s(wchar_t * * _Buffer, size_t * _BufferSizeInWords, const wchar_t * _VarName);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itow_s(int _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 27 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltow_s(long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultow_s(unsigned long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 33 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wgetenv_s(size_t * _ReturnSize, wchar_t * _DstBuf, size_t _DstSizeInWords, const wchar_t * _VarName);

#line 36 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64tow_s(long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64tow_s(unsigned long long _Val, wchar_t * _DstBuf, size_t _SizeInWords, int _Radix);

#line 39 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wmakepath_s(wchar_t * _PathResult, size_t _SizeInWords, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 42 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wputenv_s(const wchar_t * _Name, const wchar_t * _Value);

#line 44 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsearchenv_s(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath, size_t _SizeInWords);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/corecrt_wstdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wsplitpath_s(const wchar_t * _FullPath, wchar_t * _Drive, size_t _DriveSizeInWords, wchar_t * _Dir, size_t _DirSizeInWords, wchar_t * _Filename, size_t _FilenameSizeInWords, wchar_t * _Ext, size_t _ExtSizeInWords);

#line 113 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) ___mb_cur_max_func(void);

#line 137 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_set_purecall_handler(_purecall_handler _Handler))(void);

#line 138 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_get_purecall_handler(void))(void);

#line 141 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_set_invalid_parameter_handler(_invalid_parameter_handler _Handler))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 142 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void (*_get_invalid_parameter_handler(void))(const unsigned short *, const unsigned short *, const unsigned short *, unsigned int, unsigned long long);

#line 146 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) extern int * _errno(void);

#line 148 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _set_errno(int _Value);

#line 149 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_errno(int * _Value);

#line 151 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long * __doserrno(void);

#line 153 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _set_doserrno(unsigned long _Value);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_doserrno(unsigned long * _Value);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * __sys_errlist(void);

#line 156 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __sys_nerr(void);

#line 160 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * * __p___argv(void);

#line 161 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __p__fmode(void);

#line 162 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int * __p___argc(void);

#line 163 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p___wargv(void);

#line 164 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * __p__pgmptr(void);

#line 165 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * __p__wpgmptr(void);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_pgmptr(char * * _Value);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_wpgmptr(wchar_t * * _Value);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_fmode(int _Mode);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_fmode(int * _PMode);

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * * * __p__environ(void);

#line 222 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * * * __p__wenviron(void);

#line 234 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osplatform(void);

#line 235 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__osver(void);

#line 236 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winver(void);

#line 237 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winmajor(void);

#line 238 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int * __p__winminor(void);

#line 256 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osplatform(unsigned int * _Value);

#line 257 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_osver(unsigned int * _Value);

#line 258 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winver(unsigned int * _Value);

#line 259 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winmajor(unsigned int * _Value);

#line 260 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
errno_t __attribute__((__cdecl__)) _get_winminor(unsigned int * _Value);

#line 274 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) exit(int _Code) __attribute__((__noreturn__));

#line 275 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) _exit(int _Code) __attribute__((__noreturn__));

#line 277 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__nothrow__)) quick_exit(int _Code) __attribute__((__noreturn__));

#line 282 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) _Exit(int) __attribute__((__noreturn__));

#line 291 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) __attribute__((__noreturn__)) abort(void);

#line 296 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned int __attribute__((__cdecl__)) _set_abort_behavior(unsigned int _Flags, unsigned int _Mask);

#line 300 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) abs(int _X);

#line 301 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) labs(long _X);

#line 304 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) _abs64(long long);

#line 306 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
extern __inline__ __attribute__((__always_inline__,__gnu_inline__)) long long __attribute__((__cdecl__)) _abs64(long long x) {

#line 307 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
    return __builtin_llabs(x);
}

#line 311 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) atexit(void (*)(void));

#line 313 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) at_quick_exit(void (*)(void));

#line 317 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) atof(const char * _String);

#line 318 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) _atof_l(const char * _String, _locale_t _Locale);

#line 320 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) atoi(const char * _Str);

#line 321 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoi_l(const char * _Str, _locale_t _Locale);

#line 322 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) atol(const char * _Str);

#line 323 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _atol_l(const char * _Str, _locale_t _Locale);

#line 326 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * bsearch(const void * _Key, const void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 327 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) qsort(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(const void *, const void *));

#line 329 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned short __attribute__((__cdecl__)) _byteswap_ushort(unsigned short _Short);

#line 330 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _byteswap_ulong(unsigned long _Long);

#line 331 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _byteswap_uint64(unsigned long long _Int64);

#line 332 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
div_t __attribute__((__cdecl__)) div(int _Numerator, int _Denominator);

#line 333 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * getenv(const char * _VarName);

#line 334 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _itoa(int _Value, char * _Dest, int _Radix);

#line 335 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _i64toa(long long _Val, char * _DstBuf, int _Radix);

#line 336 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ui64toa(unsigned long long _Val, char * _DstBuf, int _Radix);

#line 337 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64(const char * _String);

#line 338 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _atoi64_l(const char * _String, _locale_t _Locale);

#line 339 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64(const char * _String, char * * _EndPtr, int _Radix);

#line 340 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _strtoi64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 341 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64(const char * _String, char * * _EndPtr, int _Radix);

#line 342 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _strtoui64_l(const char * _String, char * * _EndPtr, int _Radix, _locale_t _Locale);

#line 343 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
ldiv_t __attribute__((__cdecl__)) ldiv(long _Numerator, long _Denominator);

#line 344 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ltoa(long _Value, char * _Dest, int _Radix);

#line 345 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mblen(const char * _Ch, size_t _MaxCount);

#line 346 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mblen_l(const char * _Ch, size_t _MaxCount, _locale_t _Locale);

#line 347 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen(const char * _Str);

#line 348 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrlen_l(const char * _Str, _locale_t _Locale);

#line 349 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen(const char * _Str, size_t _MaxCount);

#line 350 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstrnlen_l(const char * _Str, size_t _MaxCount, _locale_t _Locale);

#line 351 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mbtowc(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes);

#line 352 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _mbtowc_l(wchar_t * __restrict__ _DstCh, const char * __restrict__ _SrcCh, size_t _SrcSizeInBytes, _locale_t _Locale);

#line 353 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
size_t __attribute__((__cdecl__)) mbstowcs(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount);

#line 354 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _mbstowcs_l(wchar_t * __restrict__ _Dest, const char * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 355 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) mkstemp(char * template_name);

#line 356 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) rand(void);

#line 357 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_error_mode(int _Mode);

#line 358 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) srand(unsigned int _Seed);

#line 385 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtod(const char * __restrict__ _Str, char * * __restrict__ _EndPtr);

#line 386 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtof(const char * __restrict__ nptr, char * * __restrict__ endptr);

#line 388 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __attribute__((__nothrow__)) strtold(const char * __restrict__, char * * __restrict__);

#line 392 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
extern double __attribute__((__cdecl__)) __attribute__((__nothrow__)) __strtod(const char * __restrict__, char * * __restrict__);

#line 400 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __mingw_strtof(const char * __restrict__, char * * __restrict__);

#line 401 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __mingw_strtod(const char * __restrict__, char * * __restrict__);

#line 402 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __mingw_strtold(const char * __restrict__, char * * __restrict__);

#line 404 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _strtof_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 405 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _strtod_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, _locale_t _Locale);

#line 406 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) strtol(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 407 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _strtol_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 408 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) strtoul(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix);

#line 409 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _strtoul_l(const char * __restrict__ _Str, char * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 412 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) system(const char * _Command);

#line 414 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ultoa(unsigned long _Value, char * _Dest, int _Radix);

#line 415 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) wctomb(char * _MbCh, wchar_t _WCh);

#line 416 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wctomb_l(char * _MbCh, wchar_t _WCh, _locale_t _Locale);

#line 417 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
size_t __attribute__((__cdecl__)) wcstombs(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount);

#line 418 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _wcstombs_l(char * __restrict__ _Dest, const wchar_t * __restrict__ _Source, size_t _MaxCount, _locale_t _Locale);

#line 452 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * calloc(size_t _NumOfElements, size_t _SizeOfElements);

#line 453 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) free(void * _Memory);

#line 454 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * malloc(size_t _Size);

#line 455 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void * realloc(void * _Memory, size_t _NewSize);

#line 456 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _aligned_free(void * _Memory);

#line 457 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_malloc(size_t _Size, size_t _Alignment);

#line 458 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset);

#line 459 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_realloc(void * _Memory, size_t _Size, size_t _Alignment);

#line 460 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 461 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _recalloc(void * _Memory, size_t _Count, size_t _Size);

#line 462 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment);

#line 463 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void * _aligned_offset_recalloc(void * _Memory, size_t _Count, size_t _Size, size_t _Alignment, size_t _Offset);

#line 464 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _aligned_msize(void * _Memory, size_t _Alignment, size_t _Offset);

#line 487 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _itow(int _Value, wchar_t * _Dest, int _Radix);

#line 488 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ltow(long _Value, wchar_t * _Dest, int _Radix);

#line 489 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ultow(unsigned long _Value, wchar_t * _Dest, int _Radix);

#line 491 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) __mingw_wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 492 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) __mingw_wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 493 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) __mingw_wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 506 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
double __attribute__((__cdecl__)) wcstod(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr);

#line 507 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
float __attribute__((__cdecl__)) wcstof(const wchar_t * __restrict__ nptr, wchar_t * * __restrict__ endptr);

#line 510 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long double __attribute__((__cdecl__)) wcstold(const wchar_t * __restrict__, wchar_t * * __restrict__);

#line 512 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wcstod_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 513 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) float __attribute__((__cdecl__)) _wcstof_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, _locale_t _Locale);

#line 514 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long __attribute__((__cdecl__)) wcstol(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 515 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wcstol_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 516 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) wcstoul(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix);

#line 517 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _wcstoul_l(const wchar_t * __restrict__ _Str, wchar_t * * __restrict__ _EndPtr, int _Radix, _locale_t _Locale);

#line 518 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wgetenv(const wchar_t * _VarName);

#line 521 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wsystem(const wchar_t * _Command);

#line 523 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof(const wchar_t * _Str);

#line 524 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) double __attribute__((__cdecl__)) _wtof_l(const wchar_t * _Str, _locale_t _Locale);

#line 525 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi(const wchar_t * _Str);

#line 526 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wtoi_l(const wchar_t * _Str, _locale_t _Locale);

#line 527 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol(const wchar_t * _Str);

#line 528 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long __attribute__((__cdecl__)) _wtol_l(const wchar_t * _Str, _locale_t _Locale);

#line 530 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _i64tow(long long _Val, wchar_t * _DstBuf, int _Radix);

#line 531 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _ui64tow(unsigned long long _Val, wchar_t * _DstBuf, int _Radix);

#line 532 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64(const wchar_t * _Str);

#line 533 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wtoi64_l(const wchar_t * _Str, _locale_t _Locale);

#line 534 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 535 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) long long __attribute__((__cdecl__)) _wcstoi64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 536 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix);

#line 537 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) unsigned long long __attribute__((__cdecl__)) _wcstoui64_l(const wchar_t * _Str, wchar_t * * _EndPtr, int _Radix, _locale_t _Locale);

#line 540 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _putenv(const char * _EnvString);

#line 541 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _wputenv(const wchar_t * _EnvString);

#line 550 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _fullpath(char * _FullPath, const char * _Path, size_t _SizeInBytes);

#line 555 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 556 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 557 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) char * _gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 558 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl(_CRT_DOUBLE * _Result, char * _Str);

#line 559 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl(_LDOUBLE * _Result, char * _Str);

#line 560 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt(_CRT_FLOAT * _Result, char * _Str);

#line 561 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atodbl_l(_CRT_DOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 562 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoldbl_l(_LDOUBLE * _Result, char * _Str, _locale_t _Locale);

#line 563 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _atoflt_l(_CRT_FLOAT * _Result, char * _Str, _locale_t _Locale);

#line 579 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotl(unsigned long, int);

#line 580 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long __attribute__((__cdecl__)) _lrotr(unsigned long, int);

#line 586 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _makepath(char * _Path, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 587 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int (*_onexit(_onexit_t _Func))(void);

#line 597 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotl64(unsigned long long _Val, int _Shift);

#line 598 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long Value, int Shift);

#line 605 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotr(unsigned int _Val, int _Shift);

#line 606 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned int __attribute__((__cdecl__)) _rotl(unsigned int _Val, int _Shift);

#line 609 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) _rotr64(unsigned long long _Val, int _Shift);

#line 610 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _searchenv(const char * _Filename, const char * _EnvVar, char * _ResultPath);

#line 611 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _splitpath(const char * _FullPath, char * _Drive, char * _Dir, char * _Filename, char * _Ext);

#line 612 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 620 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) wchar_t * _wfullpath(wchar_t * _FullPath, const wchar_t * _Path, size_t _SizeInWords);

#line 624 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wmakepath(wchar_t * _ResultPath, const wchar_t * _Drive, const wchar_t * _Dir, const wchar_t * _Filename, const wchar_t * _Ext);

#line 629 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsearchenv(const wchar_t * _Filename, const wchar_t * _EnvVar, wchar_t * _ResultPath);

#line 630 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _wsplitpath(const wchar_t * _FullPath, wchar_t * _Drive, wchar_t * _Dir, wchar_t * _Filename, wchar_t * _Ext);

#line 633 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _beep(unsigned _Frequency, unsigned _Duration) __attribute__((__deprecated__));

#line 635 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _seterrormode(int _Mode) __attribute__((__deprecated__));

#line 636 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) _sleep(unsigned long _Duration) __attribute__((__deprecated__));

#line 657 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ecvt(double _Val, int _NumOfDigits, int * _PtDec, int * _PtSign);

#line 658 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * fcvt(double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 659 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * gcvt(double _Val, int _NumOfDigits, char * _DstBuf);

#line 660 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * itoa(int _Val, char * _DstBuf, int _Radix);

#line 661 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ltoa(long _Val, char * _DstBuf, int _Radix);

#line 662 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int __attribute__((__cdecl__)) putenv(const char * _EnvString);

#line 666 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
void __attribute__((__cdecl__)) swab(char * _Buf1, char * _Buf2, int _SizeInBytes);

#line 669 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
char * ultoa(unsigned long _Val, char * _Dstbuf, int _Radix);

#line 670 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
int (*onexit(_onexit_t _Func))(void);

#line 678 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
lldiv_t __attribute__((__cdecl__)) lldiv(long long, long long);

#line 680 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) llabs(long long);

#line 685 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) strtoll(const char * __restrict__, char * * __restrict, int);

#line 686 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
unsigned long long __attribute__((__cdecl__)) strtoull(const char * __restrict__, char * * __restrict__, int);

#line 689 "/root/code/sharp/zig/lib/libc/include/any-windows-any/stdlib.h"
long long __attribute__((__cdecl__)) atoll(const char *);

#line 19 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _dupenv_s(char * * _PBuffer, size_t * _PBufferSizeInBytes, const char * _VarName);

#line 24 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) void * bsearch_s(const void * _Key, const void * _Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 25 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) getenv_s(size_t * _ReturnSize, char * _DstBuf, rsize_t _DstSize, const char * _VarName);

#line 27 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _itoa_s(int _Value, char * _DstBuf, size_t _Size, int _Radix);

#line 29 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _i64toa_s(long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 30 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ui64toa_s(unsigned long long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 31 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ltoa_s(long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 33 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) mbstowcs_s(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount);

#line 35 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _mbstowcs_s_l(size_t * _PtNumOfCharConverted, wchar_t * _DstBuf, size_t _SizeInWords, const char * _SrcBuf, size_t _MaxCount, _locale_t _Locale);

#line 37 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ultoa_s(unsigned long _Val, char * _DstBuf, size_t _Size, int _Radix);

#line 39 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wctomb_s(int * _SizeConverted, char * _MbCh, rsize_t _SizeInBytes, wchar_t _WCh);

#line 40 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wctomb_s_l(int * _SizeConverted, char * _MbCh, size_t _SizeInBytes, wchar_t _WCh, _locale_t _Locale);

#line 41 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) wcstombs_s(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes);

#line 43 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _wcstombs_s_l(size_t * _PtNumOfCharConverted, char * _Dst, size_t _DstSizeInBytes, const wchar_t * _Src, size_t _MaxCountInBytes, _locale_t _Locale);

#line 47 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _ecvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDights, int * _PtDec, int * _PtSign);

#line 48 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _fcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDec, int * _PtDec, int * _PtSign);

#line 49 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _gcvt_s(char * _DstBuf, size_t _Size, double _Val, int _NumOfDigits);

#line 50 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _makepath_s(char * _PathResult, size_t _Size, const char * _Drive, const char * _Dir, const char * _Filename, const char * _Ext);

#line 52 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _putenv_s(const char * _Name, const char * _Value);

#line 53 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _searchenv_s(const char * _Filename, const char * _EnvVar, char * _ResultPath, size_t _SizeInBytes);

#line 55 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _splitpath_s(const char * _FullPath, char * _Drive, size_t _DriveSize, char * _Dir, size_t _DirSize, char * _Filename, size_t _FilenameSize, char * _Ext, size_t _ExtSize);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/sec_api/stdlib_s.h"
__attribute__((__dllimport__)) void __attribute__((__cdecl__)) qsort_s(void * _Base, size_t _NumOfElements, size_t _SizeOfElements, int (*_PtFuncCompare)(void *, const void *, const void *), void * _Context);

#line 60 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) unsigned int * __p__amblksiz(void);

#line 129 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_malloc(size_t _Size, size_t _Alignment);

#line 130 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void __mingw_aligned_free(void * _Memory);

#line 131 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_offset_realloc(void * _Memory, size_t _Size, size_t _Alignment, size_t _Offset);

#line 132 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_offset_malloc(size_t, size_t, size_t);

#line 133 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
void * __mingw_aligned_realloc(void * _Memory, size_t _Size, size_t _Offset);

#line 134 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
size_t __mingw_aligned_msize(void * memblock, size_t alignment, size_t offset);

#line 30 "/root/code/sharp/zig/lib/include/mm_malloc.h"
static __inline__ void * _mm_malloc(size_t __size, size_t __align) {

#line 34 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    if (__align == 1) {

#line 35 "/root/code/sharp/zig/lib/include/mm_malloc.h"
        return malloc(__size);
    }

#line 38 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    if (!(__align & (__align - 1)) && __align < sizeof(void *)) 

#line 39 "/root/code/sharp/zig/lib/include/mm_malloc.h"
        __align = sizeof(void *);

#line 41 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    void * __mallocedMemory;

#line 43 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    __mallocedMemory = __mingw_aligned_malloc(__size, __align);

#line 51 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    return __mallocedMemory;
}

#line 55 "/root/code/sharp/zig/lib/include/mm_malloc.h"
static __inline__ void __attribute__((__always_inline__,__nodebug__)) _mm_free(void * __p) {

#line 58 "/root/code/sharp/zig/lib/include/mm_malloc.h"
    __mingw_aligned_free(__p);
}

#line 144 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _resetstkoflw(void);

#line 146 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) unsigned long __attribute__((__cdecl__)) _set_malloc_crt_max_wait(unsigned long _NewValue);

#line 154 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) void * _expand(void * _Memory, size_t _NewSize);

#line 155 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _msize(void * _Memory);

#line 167 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _get_sbh_threshold(void);

#line 168 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _set_sbh_threshold(size_t _NewValue);

#line 169 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _set_amblksiz(size_t _Value);

#line 170 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) errno_t __attribute__((__cdecl__)) _get_amblksiz(size_t * _Value);

#line 171 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapadd(void * _Memory, size_t _Size);

#line 172 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapchk(void);

#line 173 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapmin(void);

#line 174 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapset(unsigned int _Fill);

#line 175 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) int __attribute__((__cdecl__)) _heapwalk(_HEAPINFO * _EntryInfo);

#line 176 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) size_t __attribute__((__cdecl__)) _heapused(size_t * _Used, size_t * _Commit);

#line 177 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
__attribute__((__dllimport__)) intptr_t __attribute__((__cdecl__)) _get_heap_handle(void);

#line 190 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
static __inline void * _MarkAllocaS(void * _Ptr, unsigned int _Marker) {

#line 191 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
    if (_Ptr) {

#line 192 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
        *((unsigned int *)_Ptr) = _Marker;

#line 193 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
        _Ptr = (char *)_Ptr + 16;
    }

#line 195 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
    return _Ptr;
}

#line 218 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
static __inline void __attribute__((__cdecl__)) _freea(void * _Memory) {

#line 219 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
    unsigned int _Marker;

#line 220 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
    if (_Memory) {

#line 221 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
        _Memory = (char *)_Memory - 16;

#line 222 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
        _Marker = *(unsigned int *)_Memory;

#line 223 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
        if (_Marker == 0xDDDD) {

#line 224 "/root/code/sharp/zig/lib/libc/include/any-windows-any/malloc.h"
            free(_Memory);
        }
    }
}

#line 16 "sharp-test/unit/modules/stdlib.ce"
isize Str__size(Str * this) {
    return this->len;
}

#line 17 "sharp-test/unit/modules/stdlib.ce"
bool Str__eq(Str * this, Str other) {

#line 18 "sharp-test/unit/modules/stdlib.ce"
    if (this->len != other.len) 
        return false;

#line 19 "sharp-test/unit/modules/stdlib.ce"
    for (long i = 0; i < this->len; i = i + 1) 

#line 20 "sharp-test/unit/modules/stdlib.ce"
        if (this->ptr[i] != other.ptr[i]) 
            return false;

#line 21 "sharp-test/unit/modules/stdlib.ce"
    return true;
}

#line 23 "sharp-test/unit/modules/stdlib.ce"
Str str_from_lit(const char * lit) {

#line 24 "sharp-test/unit/modules/stdlib.ce"
    Str s;
    s.ptr = lit;
    s.len = 0;

#line 25 "sharp-test/unit/modules/stdlib.ce"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 26 "sharp-test/unit/modules/stdlib.ce"
    return s;
}

#line 28 "sharp-test/unit/modules/stdlib.ce"
bool str_eq(Str a, Str b) {
    return Str__eq(&a, b);
}

#line 36 "sharp-test/unit/modules/stdlib.ce"
void String__push_byte(String * this, char c) {

#line 37 "sharp-test/unit/modules/stdlib.ce"
    if (this->len >= this->cap) {

#line 38 "sharp-test/unit/modules/stdlib.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 39 "sharp-test/unit/modules/stdlib.ce"
        this->ptr = (char *)realloc(this->ptr, new_cap);

#line 40 "sharp-test/unit/modules/stdlib.ce"
        if (!this->ptr) 
            __builtin_trap();

#line 41 "sharp-test/unit/modules/stdlib.ce"
        this->cap = new_cap;
    }

#line 43 "sharp-test/unit/modules/stdlib.ce"
    this->ptr[this->len] = c;

#line 44 "sharp-test/unit/modules/stdlib.ce"
    this->len = this->len + 1;
}

#line 46 "sharp-test/unit/modules/stdlib.ce"
isize String__size(String * this) {
    return this->len;
}

#line 47 "sharp-test/unit/modules/stdlib.ce"
void String__destroy(String * this) {
    free(this->ptr);
    this->ptr = (char *)0;
    this->len = 0;
    this->cap = 0;
}

#line 48 "sharp-test/unit/modules/stdlib.ce"
String string_from_str(Str s) {

#line 49 "sharp-test/unit/modules/stdlib.ce"
    String r;

#line 50 "sharp-test/unit/modules/stdlib.ce"
    r.len = s.len;
    r.cap = s.len;

#line 51 "sharp-test/unit/modules/stdlib.ce"
    r.ptr = (char *)malloc(r.cap);

#line 52 "sharp-test/unit/modules/stdlib.ce"
    if (!r.ptr && r.cap > 0) 
        __builtin_trap();

#line 53 "sharp-test/unit/modules/stdlib.ce"
    for (long i = 0; i < s.len; i = i + 1) 
        r.ptr[i] = s.ptr[i];

#line 54 "sharp-test/unit/modules/stdlib.ce"
    return r;
}

#line 63 "sharp-test/unit/modules/stdlib.ce"
void Vec_int__push(Vec_int * this, int val) {

#line 64 "sharp-test/unit/modules/stdlib.ce"
    if (this->len >= this->cap) {

#line 65 "sharp-test/unit/modules/stdlib.ce"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 66 "sharp-test/unit/modules/stdlib.ce"
        this->data = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 67 "sharp-test/unit/modules/stdlib.ce"
        if (!this->data) 
            __builtin_trap();

#line 68 "sharp-test/unit/modules/stdlib.ce"
        this->cap = new_cap;
    }

#line 70 "sharp-test/unit/modules/stdlib.ce"
    this->data[this->len] = val;

#line 71 "sharp-test/unit/modules/stdlib.ce"
    this->len = this->len + 1;
}

#line 73 "sharp-test/unit/modules/stdlib.ce"
int Vec_int__pop(Vec_int * this) {

#line 74 "sharp-test/unit/modules/stdlib.ce"
    if (this->len == 0) 
        __builtin_trap();

#line 75 "sharp-test/unit/modules/stdlib.ce"
    this->len = this->len - 1;

#line 76 "sharp-test/unit/modules/stdlib.ce"
    return this->data[this->len];
}

#line 78 "sharp-test/unit/modules/stdlib.ce"
int Vec_int__get(Vec_int * this, isize i) {
    return this->data[i];
}

#line 79 "sharp-test/unit/modules/stdlib.ce"
isize Vec_int__size(Vec_int * this) {
    return this->len;
}

#line 80 "sharp-test/unit/modules/stdlib.ce"
void Vec_int__destroy(Vec_int * this) {
    free(this->data);
    this->data = (int *)0;
    this->len = 0;
    this->cap = 0;
}

#line 82 "sharp-test/unit/modules/stdlib.ce"
int main(void) {

#line 84 "sharp-test/unit/modules/stdlib.ce"
    Str hello = str_from_lit("hello");

#line 85 "sharp-test/unit/modules/stdlib.ce"
    Str world = str_from_lit("world");

#line 86 "sharp-test/unit/modules/stdlib.ce"
    Str hello2 = str_from_lit("hello");

#line 88 "sharp-test/unit/modules/stdlib.ce"
    isize hello_len = Str__size(&hello);

#line 89 "sharp-test/unit/modules/stdlib.ce"
    if (hello_len != 5) 
        return 1;

#line 90 "sharp-test/unit/modules/stdlib.ce"
    if (!Str__eq(&hello, hello2)) 
        return 2;

#line 91 "sharp-test/unit/modules/stdlib.ce"
    if (str_eq(hello, world)) 
        return 3;

#line 94 "sharp-test/unit/modules/stdlib.ce"
    String s = string_from_str(hello);

#line 95 "sharp-test/unit/modules/stdlib.ce"
    isize s_size = String__size(&s);

#line 96 "sharp-test/unit/modules/stdlib.ce"
    String__push_byte(&s, '!');

#line 97 "sharp-test/unit/modules/stdlib.ce"
    isize s_size2 = String__size(&s);

#line 98 "sharp-test/unit/modules/stdlib.ce"
    if (s_size != 5) 
        return 4;

#line 99 "sharp-test/unit/modules/stdlib.ce"
    if (s_size2 != 6) 
        return 5;

#line 100 "sharp-test/unit/modules/stdlib.ce"
    String__destroy(&s);

#line 103 "sharp-test/unit/modules/stdlib.ce"
    Vec_int v;

#line 104 "sharp-test/unit/modules/stdlib.ce"
    v.data = ((void *)0);

#line 105 "sharp-test/unit/modules/stdlib.ce"
    v.len = 0;

#line 106 "sharp-test/unit/modules/stdlib.ce"
    v.cap = 0;

#line 107 "sharp-test/unit/modules/stdlib.ce"
    Vec_int__push(&v, 10);

#line 108 "sharp-test/unit/modules/stdlib.ce"
    Vec_int__push(&v, 20);

#line 109 "sharp-test/unit/modules/stdlib.ce"
    Vec_int__push(&v, 30);

#line 110 "sharp-test/unit/modules/stdlib.ce"
    isize v_size = Vec_int__size(&v);

#line 111 "sharp-test/unit/modules/stdlib.ce"
    int v_last = Vec_int__pop(&v);

#line 112 "sharp-test/unit/modules/stdlib.ce"
    int v_first = Vec_int__get(&v, 0);

#line 113 "sharp-test/unit/modules/stdlib.ce"
    if (v_size != 3) 
        return 6;

#line 114 "sharp-test/unit/modules/stdlib.ce"
    if (v_last != 30) 
        return 7;

#line 115 "sharp-test/unit/modules/stdlib.ce"
    if (v_first != 10) 
        return 8;

#line 116 "sharp-test/unit/modules/stdlib.ce"
    Vec_int__destroy(&v);

#line 119 "sharp-test/unit/modules/stdlib.ce"
    Vec_int v2;

#line 120 "sharp-test/unit/modules/stdlib.ce"
    v2.data = ((void *)0);

#line 121 "sharp-test/unit/modules/stdlib.ce"
    v2.len = 0;

#line 122 "sharp-test/unit/modules/stdlib.ce"
    v2.cap = 0;

#line 123 "sharp-test/unit/modules/stdlib.ce"
    for (int i = 0; i < 10; i = i + 1) 
        Vec_int__push(&v2, i * 10);

#line 124 "sharp-test/unit/modules/stdlib.ce"
    if (Vec_int__size(&v2) != 10) 
        return 9;

#line 125 "sharp-test/unit/modules/stdlib.ce"
    int pop_sum = 0;

#line 126 "sharp-test/unit/modules/stdlib.ce"
    while (Vec_int__size(&v2) > 0) 
        pop_sum = pop_sum + Vec_int__pop(&v2);

#line 127 "sharp-test/unit/modules/stdlib.ce"
    if (pop_sum != 450) 
        return 10;

#line 128 "sharp-test/unit/modules/stdlib.ce"
    Vec_int__destroy(&v2);

#line 131 "sharp-test/unit/modules/stdlib.ce"
    String s2;

#line 132 "sharp-test/unit/modules/stdlib.ce"
    s2.ptr = (char *)0;
    s2.len = 0;
    s2.cap = 0;

#line 133 "sharp-test/unit/modules/stdlib.ce"
    String__push_byte(&s2, 'A');

#line 134 "sharp-test/unit/modules/stdlib.ce"
    String__push_byte(&s2, 'B');

#line 135 "sharp-test/unit/modules/stdlib.ce"
    String__push_byte(&s2, 'C');

#line 136 "sharp-test/unit/modules/stdlib.ce"
    if (String__size(&s2) != 3) 
        return 11;

#line 137 "sharp-test/unit/modules/stdlib.ce"
    if (s2.ptr[0] != 'A' || s2.ptr[1] != 'B' || s2.ptr[2] != 'C') 
        return 12;

#line 138 "sharp-test/unit/modules/stdlib.ce"
    String__destroy(&s2);

#line 140 "sharp-test/unit/modules/stdlib.ce"
    return 0;
}
