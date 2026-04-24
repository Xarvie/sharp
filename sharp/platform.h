/*
 * sharp/platform.h — Sharp 标准库内部跨平台常量
 *
 * 此文件仅供 Sharp 标准库（如 stdio.sp）内部使用。
 * 用户代码不应直接 #include 此文件。
 *
 * 所有宏以 _SH_ 为前缀，避免与 C 标准库及用户代码冲突。
 * 值来源于各平台 C 标准库头文件，编译时由 C 预处理器展开。
 */

#ifndef _SH_PLATFORM_H
#define _SH_PLATFORM_H

/* ======================== Windows ======================== */
#if defined(_WIN32) || defined(_WIN64)

  /* stdio.h 宏常量 */
  #define _SH_EOF             (-1)
  #define _SH_SEEK_SET        0
  #define _SH_SEEK_CUR        1
  #define _SH_SEEK_END        2
  #define _SH_BUFSIZ          512
  #define _SH_FILENAME_MAX    260
  #define _SH_FOPEN_MAX       20
  #define _SH_TMP_MAX         32767

  /* setvbuf 模式 */
  #define _SH_IOFBF           0x0000
  #define _SH_IOLBF           0x0040
  #define _SH_IONBF           0x0004

/* ======================== Linux ======================== */
#elif defined(__linux__)

  /* stdio.h 宏常量 */
  #define _SH_EOF             (-1)
  #define _SH_SEEK_SET        0
  #define _SH_SEEK_CUR        1
  #define _SH_SEEK_END        2
  #define _SH_BUFSIZ          8192
  #define _SH_FILENAME_MAX    4096
  #define _SH_FOPEN_MAX       16
  #define _SH_TMP_MAX         2147483647

  /* setvbuf 模式 */
  #define _SH_IOFBF           0
  #define _SH_IOLBF           1
  #define _SH_IONBF           2

/* ======================== macOS ======================== */
#elif defined(__APPLE__) && defined(__MACH__)

  /* stdio.h 宏常量 */
  #define _SH_EOF             (-1)
  #define _SH_SEEK_SET        0
  #define _SH_SEEK_CUR        1
  #define _SH_SEEK_END        2
  #define _SH_BUFSIZ          1024
  #define _SH_FILENAME_MAX    1024
  #define _SH_FOPEN_MAX       20
  #define _SH_TMP_MAX         9223372036854775807

  /* setvbuf 模式 */
  #define _SH_IOFBF           0
  #define _SH_IOLBF           1
  #define _SH_IONBF           2

/* ======================== Fallback ======================== */
#else

  /* 保守默认值（POSIX-like） */
  #define _SH_EOF             (-1)
  #define _SH_SEEK_SET        0
  #define _SH_SEEK_CUR        1
  #define _SH_SEEK_END        2
  #define _SH_BUFSIZ          1024
  #define _SH_FILENAME_MAX    4096
  #define _SH_FOPEN_MAX       16
  #define _SH_TMP_MAX         2147483647

  #define _SH_IOFBF           0
  #define _SH_IOLBF           1
  #define _SH_IONBF           2

  #warning "sharp/platform.h: unsupported platform, using POSIX defaults"

#endif /* platform detection */

#endif /* _SH_PLATFORM_H */
