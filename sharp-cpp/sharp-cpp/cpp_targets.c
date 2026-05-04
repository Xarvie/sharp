/*
 * cpp_targets.c — Phase 7: target-triple macro injection helper.
 *
 * Maps a target-triple string (e.g. "x86_64-linux-gnu") to the set of
 * predefined preprocessor macros that a real toolchain (gcc, Apple clang,
 * MSVC) would inject for that target.  The intent is purely ergonomic:
 * instead of forcing every caller to handcraft twenty-plus cpp_define()
 * calls before #include <stdio.h> works, they can write
 *
 *     CppCtx *ctx = cpp_ctx_new();
 *     if (cpp_install_target_macros(ctx, "x86_64-linux-gnu") != 0) {
 *         fprintf(stderr, "unknown triple\n");
 *         exit(1);
 *     }
 *     cpp_add_sys_include(ctx, "/usr/include");
 *     ...
 *
 * The macro values were captured from real toolchains using
 *
 *     gcc -dM -E -x c /dev/null            (Linux/x86_64, GCC 13)
 *     clang -dM -E -x c /dev/null          (macOS/arm64, Apple clang 15)
 *     cl.exe /Zc:__cplusplus /EP /dM       (Windows/x86_64, MSVC 19.29)
 *
 * Where versions emit slightly different values across patch levels we
 * pick a representative, stable, widely-deployed value.
 *
 * Important: this helper only injects macros that real *system headers*
 * inspect to choose code paths.  It does NOT enable feature-test macros
 * (_GNU_SOURCE, _POSIX_C_SOURCE, _XOPEN_SOURCE, ...) — those remain the
 * caller's responsibility because they encode *intent*, not *fact*.
 *
 * Phase 7: target-triple macro injection helper.
 */

#include "cpp.h"

#include <string.h>

/* ====================================================================== *
 * Helper: define a numeric or string macro.
 *
 * Why a wrapper?  cpp_define()'s value parameter is a const char* that is
 * concatenated into a "D:NAME=VALUE" command-line op.  Numeric values
 * therefore have to be passed as string literals.  Wrapping makes the
 * three install_*() functions read like a table.
 * ====================================================================== */

static void def(CppCtx *ctx, const char *name, const char *value) {
    cpp_define(ctx, name, value);
}

/* ====================================================================== *
 * x86_64-linux-gnu  —  LP64, ELF, little-endian, GCC 13 representative.
 *
 *   pointer = long = 8 bytes (LP64)
 *   int                   = 4 bytes
 *   short                 = 2 bytes
 *   long long             = 8 bytes
 *   wchar_t               = 4 bytes (signed)
 *   size_t / ptrdiff_t    = unsigned long / long
 *   __SIZE_TYPE__         = "long unsigned int"
 *   byte order            = little-endian
 * ====================================================================== */

/* Macros defined identically across all targets.
 * Extracted to avoid copy-paste across install_* functions. */
static void install_common(CppCtx *ctx) {
    def(ctx, "__BYTE_ORDER__", "__ORDER_LITTLE_ENDIAN__");
    def(ctx, "__CHAR_BIT__", "8");
    def(ctx, "__INT32_TYPE__", "int");
    def(ctx, "__INT8_TYPE__", "signed char");
    def(ctx, "__INT_MAX__", "0x7fffffff");
    def(ctx, "__LONG_LONG_MAX__", "0x7fffffffffffffffLL");
    def(ctx, "__ORDER_BIG_ENDIAN__", "4321");
    def(ctx, "__ORDER_LITTLE_ENDIAN__", "1234");
    def(ctx, "__ORDER_PDP_ENDIAN__", "3412");
    def(ctx, "__SCHAR_MAX__", "0x7f");
    def(ctx, "__SHRT_MAX__", "0x7fff");
    def(ctx, "__SIZEOF_DOUBLE__", "8");
    def(ctx, "__SIZEOF_FLOAT__", "4");
    def(ctx, "__SIZEOF_INT__", "4");
    def(ctx, "__SIZEOF_LONG_LONG__", "8");
    def(ctx, "__SIZEOF_POINTER__", "8");
    def(ctx, "__SIZEOF_PTRDIFF_T__", "8");
    def(ctx, "__SIZEOF_SHORT__", "2");
    def(ctx, "__SIZEOF_SIZE_T__", "8");
    def(ctx, "__UINT32_TYPE__", "unsigned int");
    def(ctx, "__UINT8_TYPE__", "unsigned char");
}

static void install_x86_64_linux_gnu(CppCtx *ctx) {
    install_common(ctx);
    /* --- architecture identity --- */
    def(ctx, "__x86_64__",       "1");
    def(ctx, "__x86_64",         "1");
    def(ctx, "__amd64__",        "1");
    def(ctx, "__amd64",          "1");

    /* --- data model: LP64 --- */
    def(ctx, "__LP64__",         "1");
    def(ctx, "_LP64",            "1");
    def(ctx, "__WORDSIZE",       "64");

    /* --- operating system + libc --- */
    def(ctx, "__linux__",        "1");
    def(ctx, "__linux",          "1");
    def(ctx, "linux",            "1");
    def(ctx, "__unix__",         "1");
    def(ctx, "__unix",           "1");
    def(ctx, "unix",             "1");
    def(ctx, "__gnu_linux__",    "1");

    /* --- object file format --- */
    def(ctx, "__ELF__",          "1");

    /* --- compiler identity (GCC 13.3 — matches Ubuntu 24.04 system gcc) --- */
    def(ctx, "__GNUC__",         "13");
    def(ctx, "__GNUC_MINOR__",   "3");
    def(ctx, "__GNUC_PATCHLEVEL__", "0");
    def(ctx, "__STDC_HOSTED__",  "1");

    /* --- character traits --- */

    /* --- type sizes (bytes) --- */
    def(ctx, "__SIZEOF_LONG__",      "8");
    def(ctx, "__SIZEOF_WCHAR_T__",   "4");
    def(ctx, "__SIZEOF_WINT_T__",    "4");
    def(ctx, "__SIZEOF_LONG_DOUBLE__", "16");

    /* Phase R12: extended-type sizeof macros that gcc unconditionally
     * defines on x86_64 (the AMD64 SysV ABI mandates these types).
     * Linux's <linux/types.h> uses #ifdef __SIZEOF_INT128__ to gate
     * `typedef __signed__ __int128 __s128 ...` — without it, sharp-cpp
     * silently drops the type, producing diff vs gcc -E output.        */
    def(ctx, "__SIZEOF_INT128__",     "16");
    def(ctx, "__SIZEOF_FLOAT80__",    "16");
    def(ctx, "__SIZEOF_FLOAT128__",   "16");

    /* --- byte order --- */
    def(ctx, "__FLOAT_WORD_ORDER__",    "__ORDER_LITTLE_ENDIAN__");

    /* --- limits (used by limits.h, stdint.h) --- */
    /* Phase R13: match gcc's exact hex representations so that token-diff
     * against `gcc -E` is identical even when headers expand these.     */
    def(ctx, "__LONG_MAX__",      "0x7fffffffffffffffL");
    def(ctx, "__WCHAR_MAX__",     "0x7fffffff");
    def(ctx, "__WCHAR_MIN__",     "(-__WCHAR_MAX__ - 1)");
    def(ctx, "__WINT_MAX__",      "0xffffffffU");
    def(ctx, "__WINT_MIN__",      "0U");
    def(ctx, "__SIZE_MAX__",      "0xffffffffffffffffUL");
    def(ctx, "__PTRDIFF_MAX__",   "0x7fffffffffffffffL");
    def(ctx, "__INTMAX_MAX__",    "0x7fffffffffffffffL");
    def(ctx, "__UINTMAX_MAX__",   "0xffffffffffffffffUL");

    /* --- type spellings (used by stddef.h, stdint.h, sys/types.h) --- */
    def(ctx, "__SIZE_TYPE__",     "long unsigned int");
    def(ctx, "__PTRDIFF_TYPE__",  "long int");
    def(ctx, "__INTPTR_TYPE__",   "long int");
    def(ctx, "__UINTPTR_TYPE__",  "long unsigned int");
    def(ctx, "__INTMAX_TYPE__",   "long int");
    def(ctx, "__UINTMAX_TYPE__",  "long unsigned int");
    def(ctx, "__WCHAR_TYPE__",    "int");
    def(ctx, "__WINT_TYPE__",     "unsigned int");
    def(ctx, "__INT16_TYPE__",    "short int");
    def(ctx, "__INT64_TYPE__",    "long int");
    def(ctx, "__UINT16_TYPE__",   "short unsigned int");
    def(ctx, "__UINT64_TYPE__",   "long unsigned int");

    /* --- alignment --- */
    def(ctx, "__BIGGEST_ALIGNMENT__", "16");

    /* --- Phase R12: ABI / compiler-identity macros that gcc auto-defines.
     * glibc's <sys/cdefs.h> uses __USER_LABEL_PREFIX__ in __ASMNAME(name)
     * to build asm-symbol names by stringification, e.g.
     *     __asm__ ("" "__isoc99_fscanf")
     * The ELF SysV ABI prescribes an *empty* user-label prefix on Linux
     * x86_64 (Mach-O on Darwin uses "_").  Without this define, sharp-cpp
     * would stringify the macro NAME instead of getting the empty string,
     * producing __asm__("__USER_LABEL_PREFIX____isoc99_fscanf") — a junk
     * symbol that won't link.                                            */
    def(ctx, "__USER_LABEL_PREFIX__", "");
    def(ctx, "__REGISTER_PREFIX__",  "");
    /* __VERSION__ — vendor-formatted string, gcc emits with quotes.     */
    def(ctx, "__VERSION__",          "\"13.3.0\"");
    /* Used by libm/glibc to gate fast-math optimisations.                */
    def(ctx, "__FINITE_MATH_ONLY__", "0");
    /* gcc defines this to 1 unconditionally except when -fno-inline.     */
    def(ctx, "__NO_INLINE__",        "1");
    /* C++ ABI level — included even in C mode by gcc.                    */
        def(ctx, "__GXX_ABI_VERSION",    "1018");

    /* --- Phase R12: bulk-imported gcc 13 internal builtins.
     * Generated from `gcc -E -dM -x c </dev/null` minus what we
     * already define above.  Excluded: function-like _C() macros
     * (need a separate `cpp_define_func` API), per-format float
     * mantissa/min/max literal values (only __MANT_DIG__/__DIG__/
     * __HAS_*__ here — the ratio of code that uses them is low).   */
    def(ctx, "_STDC_PREDEF_H",	"1");
    def(ctx, "__ATOMIC_ACQUIRE",	"2");
    def(ctx, "__ATOMIC_ACQ_REL",	"4");
    def(ctx, "__ATOMIC_CONSUME",	"1");
    def(ctx, "__ATOMIC_HLE_ACQUIRE",	"65536");
    def(ctx, "__ATOMIC_HLE_RELEASE",	"131072");
    def(ctx, "__ATOMIC_RELAXED",	"0");
    def(ctx, "__ATOMIC_RELEASE",	"3");
    def(ctx, "__ATOMIC_SEQ_CST",	"5");
    def(ctx, "__CET__",	"3");
    def(ctx, "__CHAR16_TYPE__",	"short unsigned int");
    def(ctx, "__CHAR32_TYPE__",	"unsigned int");
    def(ctx, "__DBL_DECIMAL_DIG__",	"17");
    def(ctx, "__DBL_DIG__",	"15");
    def(ctx, "__DBL_HAS_INFINITY__",	"1");
    def(ctx, "__DBL_HAS_QUIET_NAN__",	"1");
    def(ctx, "__DBL_IS_IEC_60559__",	"1");
    def(ctx, "__DBL_MANT_DIG__",	"53");
    def(ctx, "__DBL_MAX_10_EXP__",	"308");
    def(ctx, "__DBL_MAX_EXP__",	"1024");
    def(ctx, "__DBL_MIN_10_EXP__",	"(-307)");
    def(ctx, "__DBL_MIN_EXP__",	"(-1021)");
    /* Phase R1: gcc predefines the IEEE-754 value macros below.  Real C
     * code (cJSON 1.7.18 line 4823: `maxVal * __DBL_EPSILON__`) uses them
     * without an explicit <float.h> include and expects the compiler to
     * supply them.  Values match `gcc -E -dM` on x86_64-linux-gnu. */
    def(ctx, "__DBL_DENORM_MIN__",	"((double)4.94065645841246544176568792868221372e-324L)");
    def(ctx, "__DBL_EPSILON__",	"((double)2.22044604925031308084726333618164062e-16L)");
    def(ctx, "__DBL_HAS_DENORM__",	"1");
    def(ctx, "__DBL_MAX__",	"((double)1.79769313486231570814527423731704357e+308L)");
    def(ctx, "__DBL_MIN__",	"((double)2.22507385850720138309023271733240406e-308L)");
    def(ctx, "__DBL_NORM_MAX__",	"((double)1.79769313486231570814527423731704357e+308L)");
    def(ctx, "__DECIMAL_BID_FORMAT__",	"1");
    def(ctx, "__DECIMAL_DIG__",	"21");
    def(ctx, "__DEC_EVAL_METHOD__",	"2");
    def(ctx, "__FLT128_DECIMAL_DIG__",	"36");
    def(ctx, "__FLT128_DIG__",	"33");
    def(ctx, "__FLT128_HAS_INFINITY__",	"1");
    def(ctx, "__FLT128_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT128_IS_IEC_60559__",	"1");
    def(ctx, "__FLT128_MANT_DIG__",	"113");
    def(ctx, "__FLT128_MAX_10_EXP__",	"4932");
    def(ctx, "__FLT128_MAX_EXP__",	"16384");
    def(ctx, "__FLT128_MIN_10_EXP__",	"(-4931)");
    def(ctx, "__FLT128_MIN_EXP__",	"(-16381)");
    def(ctx, "__FLT16_DECIMAL_DIG__",	"5");
    def(ctx, "__FLT16_DIG__",	"3");
    def(ctx, "__FLT16_HAS_INFINITY__",	"1");
    def(ctx, "__FLT16_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT16_IS_IEC_60559__",	"1");
    def(ctx, "__FLT16_MANT_DIG__",	"11");
    def(ctx, "__FLT16_MAX_10_EXP__",	"4");
    def(ctx, "__FLT16_MAX_EXP__",	"16");
    def(ctx, "__FLT16_MIN_10_EXP__",	"(-4)");
    def(ctx, "__FLT16_MIN_EXP__",	"(-13)");
    def(ctx, "__FLT32X_DECIMAL_DIG__",	"17");
    def(ctx, "__FLT32X_DIG__",	"15");
    def(ctx, "__FLT32X_HAS_INFINITY__",	"1");
    def(ctx, "__FLT32X_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT32X_IS_IEC_60559__",	"1");
    def(ctx, "__FLT32X_MANT_DIG__",	"53");
    def(ctx, "__FLT32X_MAX_10_EXP__",	"308");
    def(ctx, "__FLT32X_MAX_EXP__",	"1024");
    def(ctx, "__FLT32X_MIN_10_EXP__",	"(-307)");
    def(ctx, "__FLT32X_MIN_EXP__",	"(-1021)");
    def(ctx, "__FLT32_DECIMAL_DIG__",	"9");
    def(ctx, "__FLT32_DIG__",	"6");
    def(ctx, "__FLT32_HAS_INFINITY__",	"1");
    def(ctx, "__FLT32_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT32_IS_IEC_60559__",	"1");
    def(ctx, "__FLT32_MANT_DIG__",	"24");
    def(ctx, "__FLT32_MAX_10_EXP__",	"38");
    def(ctx, "__FLT32_MAX_EXP__",	"128");
    def(ctx, "__FLT32_MIN_10_EXP__",	"(-37)");
    def(ctx, "__FLT32_MIN_EXP__",	"(-125)");
    def(ctx, "__FLT64X_DECIMAL_DIG__",	"21");
    def(ctx, "__FLT64X_DIG__",	"18");
    def(ctx, "__FLT64X_HAS_INFINITY__",	"1");
    def(ctx, "__FLT64X_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT64X_IS_IEC_60559__",	"1");
    def(ctx, "__FLT64X_MANT_DIG__",	"64");
    def(ctx, "__FLT64X_MAX_10_EXP__",	"4932");
    def(ctx, "__FLT64X_MAX_EXP__",	"16384");
    def(ctx, "__FLT64X_MIN_10_EXP__",	"(-4931)");
    def(ctx, "__FLT64X_MIN_EXP__",	"(-16381)");
    def(ctx, "__FLT64_DECIMAL_DIG__",	"17");
    def(ctx, "__FLT64_DIG__",	"15");
    def(ctx, "__FLT64_HAS_INFINITY__",	"1");
    def(ctx, "__FLT64_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT64_IS_IEC_60559__",	"1");
    def(ctx, "__FLT64_MANT_DIG__",	"53");
    def(ctx, "__FLT64_MAX_10_EXP__",	"308");
    def(ctx, "__FLT64_MAX_EXP__",	"1024");
    def(ctx, "__FLT64_MIN_10_EXP__",	"(-307)");
    def(ctx, "__FLT64_MIN_EXP__",	"(-1021)");
    def(ctx, "__FLT_DECIMAL_DIG__",	"9");
    def(ctx, "__FLT_DIG__",	"6");
    def(ctx, "__FLT_EVAL_METHOD_TS_18661_3__",	"0");
    def(ctx, "__FLT_EVAL_METHOD__",	"0");
    def(ctx, "__FLT_HAS_INFINITY__",	"1");
    def(ctx, "__FLT_HAS_QUIET_NAN__",	"1");
    def(ctx, "__FLT_IS_IEC_60559__",	"1");
    def(ctx, "__FLT_MANT_DIG__",	"24");
    def(ctx, "__FLT_MAX_10_EXP__",	"38");
    def(ctx, "__FLT_MAX_EXP__",	"128");
    def(ctx, "__FLT_MIN_10_EXP__",	"(-37)");
    def(ctx, "__FLT_MIN_EXP__",	"(-125)");
    /* Phase R1: gcc-predefined IEEE-754 float-range value macros. */
    def(ctx, "__FLT_DENORM_MIN__",	"1.40129846432481707092372958328991613e-45F");
    def(ctx, "__FLT_EPSILON__",	"1.19209289550781250000000000000000000e-7F");
    def(ctx, "__FLT_HAS_DENORM__",	"1");
    def(ctx, "__FLT_MAX__",	"3.40282346638528859811704183484516925e+38F");
    def(ctx, "__FLT_MIN__",	"1.17549435082228750796873653722224568e-38F");
    def(ctx, "__FLT_NORM_MAX__",	"3.40282346638528859811704183484516925e+38F");
    def(ctx, "__FLT_RADIX__",	"2");
    def(ctx, "__FXSR__",	"1");
    def(ctx, "__GCC_ASM_FLAG_OUTPUTS__",	"1");
    def(ctx, "__GCC_ATOMIC_BOOL_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_CHAR16_T_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_CHAR32_T_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_CHAR_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_INT_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_LLONG_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_LONG_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_POINTER_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_SHORT_LOCK_FREE",	"2");
    def(ctx, "__GCC_ATOMIC_TEST_AND_SET_TRUEVAL",	"1");
    def(ctx, "__GCC_ATOMIC_WCHAR_T_LOCK_FREE",	"2");
    def(ctx, "__GCC_CONSTRUCTIVE_SIZE",	"64");
    def(ctx, "__GCC_DESTRUCTIVE_SIZE",	"64");
    def(ctx, "__GCC_HAVE_DWARF2_CFI_ASM",	"1");
    def(ctx, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1",	"1");
    def(ctx, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2",	"1");
    def(ctx, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4",	"1");
    def(ctx, "__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8",	"1");
    def(ctx, "__GCC_IEC_559",	"2");
    def(ctx, "__GCC_IEC_559_COMPLEX",	"2");
    def(ctx, "__GNUC_EXECUTION_CHARSET_NAME",	"\"UTF-8\"");
    def(ctx, "__GNUC_STDC_INLINE__",	"1");
    def(ctx, "__GNUC_WIDE_EXECUTION_CHARSET_NAME",	"\"UTF-32LE\"");
    def(ctx, "__HAVE_SPECULATION_SAFE_VALUE",	"1");
    def(ctx, "__INT16_MAX__",	"0x7fff");
    def(ctx, "__INT32_MAX__",	"0x7fffffff");
    def(ctx, "__INT64_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INT8_MAX__",	"0x7f");
    def(ctx, "__INTMAX_WIDTH__",	"64");
    def(ctx, "__INTPTR_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INTPTR_WIDTH__",	"64");
    def(ctx, "__INT_FAST16_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INT_FAST16_TYPE__",	"long int");
    def(ctx, "__INT_FAST16_WIDTH__",	"64");
    def(ctx, "__INT_FAST32_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INT_FAST32_TYPE__",	"long int");
    def(ctx, "__INT_FAST32_WIDTH__",	"64");
    def(ctx, "__INT_FAST64_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INT_FAST64_TYPE__",	"long int");
    def(ctx, "__INT_FAST64_WIDTH__",	"64");
    def(ctx, "__INT_FAST8_MAX__",	"0x7f");
    def(ctx, "__INT_FAST8_TYPE__",	"signed char");
    def(ctx, "__INT_FAST8_WIDTH__",	"8");
    def(ctx, "__INT_LEAST16_MAX__",	"0x7fff");
    def(ctx, "__INT_LEAST16_TYPE__",	"short int");
    def(ctx, "__INT_LEAST16_WIDTH__",	"16");
    def(ctx, "__INT_LEAST32_MAX__",	"0x7fffffff");
    def(ctx, "__INT_LEAST32_TYPE__",	"int");
    def(ctx, "__INT_LEAST32_WIDTH__",	"32");
    def(ctx, "__INT_LEAST64_MAX__",	"0x7fffffffffffffffL");
    def(ctx, "__INT_LEAST64_TYPE__",	"long int");
    def(ctx, "__INT_LEAST64_WIDTH__",	"64");
    def(ctx, "__INT_LEAST8_MAX__",	"0x7f");
    def(ctx, "__INT_LEAST8_TYPE__",	"signed char");
    def(ctx, "__INT_LEAST8_WIDTH__",	"8");
    def(ctx, "__INT_WIDTH__",	"32");
    def(ctx, "__LDBL_DECIMAL_DIG__",	"21");
    def(ctx, "__LDBL_DIG__",	"18");
    def(ctx, "__LDBL_HAS_INFINITY__",	"1");
    def(ctx, "__LDBL_HAS_QUIET_NAN__",	"1");
    def(ctx, "__LDBL_IS_IEC_60559__",	"1");
    def(ctx, "__LDBL_MANT_DIG__",	"64");
    def(ctx, "__LDBL_MAX_10_EXP__",	"4932");
    def(ctx, "__LDBL_MAX_EXP__",	"16384");
    def(ctx, "__LDBL_MIN_10_EXP__",	"(-4931)");
    def(ctx, "__LDBL_MIN_EXP__",	"(-16381)");
    /* Phase R1: gcc-predefined IEEE-754 long-double range value macros
     * for x86 80-bit extended precision. */
    def(ctx, "__LDBL_DENORM_MIN__",	"3.64519953188247460252840593361941982e-4951L");
    def(ctx, "__LDBL_EPSILON__",	"1.08420217248550443400745280086994171e-19L");
    def(ctx, "__LDBL_HAS_DENORM__",	"1");
    def(ctx, "__LDBL_MAX__",	"1.18973149535723176502126385303097021e+4932L");
    def(ctx, "__LDBL_MIN__",	"3.36210314311209350626267781732175260e-4932L");
    def(ctx, "__LDBL_NORM_MAX__",	"1.18973149535723176502126385303097021e+4932L");
    def(ctx, "__LONG_LONG_WIDTH__",	"64");
    def(ctx, "__LONG_WIDTH__",	"64");
    def(ctx, "__MMX_WITH_SSE__",	"1");
    def(ctx, "__MMX__",	"1");
    def(ctx, "__PIC__",	"2");
    def(ctx, "__PIE__",	"2");
    def(ctx, "__PRAGMA_REDEFINE_EXTNAME",	"1");
    def(ctx, "__PTRDIFF_WIDTH__",	"64");
    def(ctx, "__SCHAR_WIDTH__",	"8");
    def(ctx, "__SEG_FS",	"1");
    def(ctx, "__SEG_GS",	"1");
    def(ctx, "__SHRT_WIDTH__",	"16");
    def(ctx, "__SIG_ATOMIC_MAX__",	"0x7fffffff");
    def(ctx, "__SIG_ATOMIC_MIN__",	"(-__SIG_ATOMIC_MAX__ - 1)");
    def(ctx, "__SIG_ATOMIC_TYPE__",	"int");
    def(ctx, "__SIG_ATOMIC_WIDTH__",	"32");
    def(ctx, "__SIZE_WIDTH__",	"64");
    def(ctx, "__SSE2_MATH__",	"1");
    def(ctx, "__SSE2__",	"1");
    def(ctx, "__SSE_MATH__",	"1");
    def(ctx, "__SSE__",	"1");
    def(ctx, "__SSP_STRONG__",	"3");
    def(ctx, "__STDC_IEC_559_COMPLEX__",	"1");
    def(ctx, "__STDC_IEC_559__",	"1");
    def(ctx, "__STDC_IEC_60559_BFP__",	"201404L");
    def(ctx, "__STDC_IEC_60559_COMPLEX__",	"201404L");
    def(ctx, "__STDC_ISO_10646__",	"201706L");
    def(ctx, "__STDC_UTF_16__",	"1");
    def(ctx, "__STDC_UTF_32__",	"1");
    def(ctx, "__UINT16_MAX__",	"0xffff");
    def(ctx, "__UINT32_MAX__",	"0xffffffffU");
    def(ctx, "__UINT64_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT8_MAX__",	"0xff");
    def(ctx, "__UINTPTR_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT_FAST16_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT_FAST16_TYPE__",	"long unsigned int");
    def(ctx, "__UINT_FAST32_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT_FAST32_TYPE__",	"long unsigned int");
    def(ctx, "__UINT_FAST64_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT_FAST64_TYPE__",	"long unsigned int");
    def(ctx, "__UINT_FAST8_MAX__",	"0xff");
    def(ctx, "__UINT_FAST8_TYPE__",	"unsigned char");
    def(ctx, "__UINT_LEAST16_MAX__",	"0xffff");
    def(ctx, "__UINT_LEAST16_TYPE__",	"short unsigned int");
    def(ctx, "__UINT_LEAST32_MAX__",	"0xffffffffU");
    def(ctx, "__UINT_LEAST32_TYPE__",	"unsigned int");
    def(ctx, "__UINT_LEAST64_MAX__",	"0xffffffffffffffffUL");
    def(ctx, "__UINT_LEAST64_TYPE__",	"long unsigned int");
    def(ctx, "__UINT_LEAST8_MAX__",	"0xff");
    def(ctx, "__UINT_LEAST8_TYPE__",	"unsigned char");
    def(ctx, "__WCHAR_WIDTH__",	"32");
    def(ctx, "__WINT_WIDTH__",	"32");
    def(ctx, "__code_model_small__",	"1");
    def(ctx, "__k8",	"1");
    def(ctx, "__k8__",	"1");
    def(ctx, "__pic__",	"2");
    def(ctx, "__pie__",	"2");

    /* Phase R15: function-like C-suffix macros.  These are used by
     * <stdint.h> / <inttypes.h> for INTMAX_C(n), INT64_C(n) etc.
     * The token-paste (##) form must be a function-like macro; a
     * simple object-like def would not work.                         */
    cpp_define_func(ctx, "__INT8_C(c)",    "c");
    cpp_define_func(ctx, "__INT16_C(c)",   "c");
    cpp_define_func(ctx, "__INT32_C(c)",   "c");
    cpp_define_func(ctx, "__INT64_C(c)",   "c ## L");
    cpp_define_func(ctx, "__INTMAX_C(c)",  "c ## L");
    cpp_define_func(ctx, "__UINT8_C(c)",   "c");
    cpp_define_func(ctx, "__UINT16_C(c)",  "c");
    cpp_define_func(ctx, "__UINT32_C(c)",  "c ## U");
    cpp_define_func(ctx, "__UINT64_C(c)",  "c ## UL");
    cpp_define_func(ctx, "__UINTMAX_C(c)", "c ## UL");
}


/* ====================================================================== *
 * aarch64-apple-darwin  —  LP64, Mach-O, little-endian, Apple clang.
 *
 * Apple's clang fronts present themselves as both __clang__ AND __GNUC__,
 * so headers that test either gate work.  __APPLE__ + __MACH__ are the
 * canonical Darwin signatures.  Crucially: NO __ELF__, NO __linux__.
 * ====================================================================== */

static void install_aarch64_apple_darwin(CppCtx *ctx) {
    install_common(ctx);
    /* --- architecture identity --- */
    def(ctx, "__aarch64__",      "1");
    def(ctx, "__arm64__",        "1");
    def(ctx, "__arm64",          "1");
    def(ctx, "__ARM_ARCH",       "8");
    def(ctx, "__ARM_ARCH_ISA_A64", "1");
    def(ctx, "__ARM_64BIT_STATE", "1");

    /* --- data model: LP64 (Apple chose LP64 on Darwin/arm64) --- */
    def(ctx, "__LP64__",         "1");
    def(ctx, "_LP64",            "1");

    /* --- operating system identity --- */
    def(ctx, "__APPLE__",        "1");
    def(ctx, "__MACH__",         "1");
    def(ctx, "__unix__",         "1");
    def(ctx, "__unix",           "1");

    /* --- compiler identity (Apple clang masquerades as GCC 4.2.1 ABI) --- */
    def(ctx, "__GNUC__",         "4");
    def(ctx, "__GNUC_MINOR__",   "2");
    def(ctx, "__GNUC_PATCHLEVEL__", "1");
    def(ctx, "__clang__",        "1");
    def(ctx, "__clang_major__",  "15");
    def(ctx, "__clang_minor__",  "0");
    def(ctx, "__STDC_HOSTED__",  "1");

    /* --- character traits --- */

    /* On Apple/arm64, char is unsigned by default (unlike Linux/x86_64). */
    def(ctx, "__CHAR_UNSIGNED__", "1");

    /* --- type sizes (bytes) — same LP64 as Linux/x86_64 --- */
    def(ctx, "__SIZEOF_LONG__",      "8");
    def(ctx, "__SIZEOF_WCHAR_T__",   "4");
    def(ctx, "__SIZEOF_WINT_T__",    "4");
    /* On Apple/arm64, long double = double (8 bytes), not 16. */
    def(ctx, "__SIZEOF_LONG_DOUBLE__", "8");

    /* --- byte order --- */
    def(ctx, "__FLOAT_WORD_ORDER__",    "__ORDER_LITTLE_ENDIAN__");

    /* --- limits (Phase R13: hex to match gcc -E output) --- */
    def(ctx, "__LONG_MAX__",      "0x7fffffffffffffffL");
    def(ctx, "__WCHAR_MAX__",     "0x7fffffff");
    def(ctx, "__WCHAR_MIN__",     "(-__WCHAR_MAX__ - 1)");
    def(ctx, "__WINT_MAX__",      "0x7fffffff");
    def(ctx, "__WINT_MIN__",      "(-__WINT_MAX__ - 1)");
    def(ctx, "__SIZE_MAX__",      "0xffffffffffffffffUL");
    def(ctx, "__PTRDIFF_MAX__",   "0x7fffffffffffffffL");
    def(ctx, "__INTMAX_MAX__",    "0x7fffffffffffffffL");
    def(ctx, "__UINTMAX_MAX__",   "0xffffffffffffffffUL");

    /* --- type spellings — same as Linux/x86_64 LP64 --- */
    def(ctx, "__SIZE_TYPE__",     "long unsigned int");
    def(ctx, "__PTRDIFF_TYPE__",  "long int");
    def(ctx, "__INTPTR_TYPE__",   "long int");
    def(ctx, "__UINTPTR_TYPE__",  "long unsigned int");
    def(ctx, "__INTMAX_TYPE__",   "long int");
    def(ctx, "__UINTMAX_TYPE__",  "long unsigned int");
    def(ctx, "__WCHAR_TYPE__",    "int");
    def(ctx, "__WINT_TYPE__",     "int");
    def(ctx, "__INT16_TYPE__",    "short");
    def(ctx, "__INT64_TYPE__",    "long long int");
    def(ctx, "__UINT16_TYPE__",   "unsigned short");
    def(ctx, "__UINT64_TYPE__",   "long long unsigned int");

    /* --- alignment --- */
    def(ctx, "__BIGGEST_ALIGNMENT__", "8");

    /* --- Phase R12: ABI / compiler-identity macros (see x86_64-linux-gnu
     * for full rationale).  Apple's Mach-O ABI prefixes user-defined
     * symbols with a leading underscore — this is what makes asm names
     * like _printf in disassembly.                                       */
    def(ctx, "__USER_LABEL_PREFIX__", "_");
    def(ctx, "__REGISTER_PREFIX__",  "");
    def(ctx, "__VERSION__",          "\"Apple LLVM 15.0.0 (clang-1500.0.40.1)\"");
    def(ctx, "__FINITE_MATH_ONLY__", "0");
    def(ctx, "__NO_INLINE__",        "1");
    def(ctx, "__GXX_ABI_VERSION",    "1002");

    /* Phase R16: C-suffix function-like macros (LP64, same as linux) */
    cpp_define_func(ctx, "__INT8_C(c)",    "c");
    cpp_define_func(ctx, "__INT16_C(c)",   "c");
    cpp_define_func(ctx, "__INT32_C(c)",   "c");
    cpp_define_func(ctx, "__INT64_C(c)",   "c ## L");
    cpp_define_func(ctx, "__INTMAX_C(c)",  "c ## L");
    cpp_define_func(ctx, "__UINT8_C(c)",   "c");
    cpp_define_func(ctx, "__UINT16_C(c)",  "c");
    cpp_define_func(ctx, "__UINT32_C(c)",  "c ## U");
    cpp_define_func(ctx, "__UINT64_C(c)",  "c ## UL");
    cpp_define_func(ctx, "__UINTMAX_C(c)", "c ## UL");
}

/* ====================================================================== *
 * x86_64-pc-windows-msvc  —  LLP64, COFF/PE, little-endian, MSVC.
 *
 * CRITICAL: this is LLP64, not LP64.  long is 32 bits!  Code that does
 *
 *     #ifdef __LP64__
 *     // assume long is wide enough to hold a pointer
 *     #endif
 *
 * is broken on Windows-x64.  We do NOT define __LP64__ here.
 *
 * MSVC also uses a different macro vocabulary: _M_X64 / _M_AMD64 instead
 * of __x86_64__, _MSC_VER instead of __GNUC__, and so on.  Do not be
 * tempted to "alias" them — real headers use the MSVC names verbatim.
 * ====================================================================== */

static void install_x86_64_pc_windows_msvc(CppCtx *ctx) {
    install_common(ctx);
    /* --- architecture (MSVC vocabulary) --- */
    def(ctx, "_M_X64",           "100");
    def(ctx, "_M_AMD64",         "100");

    /* --- operating system --- */
    def(ctx, "_WIN32",           "1");   /* defined for both 32- and 64-bit Windows */
    def(ctx, "_WIN64",           "1");   /* additionally defined on 64-bit Windows  */

    /* --- compiler identity (MSVC 19.29 / Visual Studio 2019 16.11) --- */
    def(ctx, "_MSC_VER",         "1929");
    def(ctx, "_MSC_FULL_VER",    "192930148");
    def(ctx, "_MSC_BUILD",       "1");
    def(ctx, "_INTEGRAL_MAX_BITS", "64");

    /* MSVC always defines _CHAR_UNSIGNED on x64 only when /J is passed.
     * Default is signed char.  Don't define _CHAR_UNSIGNED. */

    /* --- character traits --- */

    /* --- type sizes (LLP64) ---
     *   short = 2, int = 4, long = 4 (NOT 8!), long long = 8, pointer = 8
     */
    def(ctx, "__SIZEOF_LONG__",      "4");   /* the LLP64 distinguishing fact */
    /* MSVC: wchar_t is 2 bytes (UTF-16). */
    def(ctx, "__SIZEOF_WCHAR_T__",   "2");
    def(ctx, "__SIZEOF_WINT_T__",    "2");
    def(ctx, "__SIZEOF_LONG_DOUBLE__", "8");

    /* --- byte order (Intel little-endian) --- */

    /* --- limits (LLP64: LONG is 32-bit, LONG_LONG is 64-bit)
     * Phase R13: use hex to match gcc-mingw -E output.              */
    def(ctx, "__LONG_MAX__",      "0x7fffffffL");           /* 32-bit, MSVC */
    /* MSVC uses 16-bit wchar_t (UTF-16). */
    def(ctx, "__WCHAR_MAX__",     "0xffff");
    def(ctx, "__WCHAR_MIN__",     "0");
    def(ctx, "__WINT_MAX__",      "0xffff");
    def(ctx, "__WINT_MIN__",      "0");
    def(ctx, "__SIZE_MAX__",      "0xffffffffffffffffULL");
    def(ctx, "__PTRDIFF_MAX__",   "0x7fffffffffffffffLL");
    def(ctx, "__INTMAX_MAX__",    "0x7fffffffffffffffLL");
    def(ctx, "__UINTMAX_MAX__",   "0xffffffffffffffffULL");

    /* --- type spellings (LLP64: size_t = unsigned long long, NOT unsigned long) --- */
    def(ctx, "__SIZE_TYPE__",     "long long unsigned int");
    def(ctx, "__PTRDIFF_TYPE__",  "long long int");
    def(ctx, "__INTPTR_TYPE__",   "long long int");
    def(ctx, "__UINTPTR_TYPE__",  "long long unsigned int");
    def(ctx, "__INTMAX_TYPE__",   "long long int");
    def(ctx, "__UINTMAX_TYPE__",  "long long unsigned int");
    /* MSVC: wchar_t is unsigned short (UTF-16 code unit). */
    def(ctx, "__WCHAR_TYPE__",    "unsigned short");
    def(ctx, "__WINT_TYPE__",     "unsigned short");
    def(ctx, "__INT16_TYPE__",    "short");
    def(ctx, "__INT64_TYPE__",    "long long");
    def(ctx, "__UINT16_TYPE__",   "unsigned short");
    def(ctx, "__UINT64_TYPE__",   "unsigned long long");

    /* --- alignment --- */
    def(ctx, "__BIGGEST_ALIGNMENT__", "8");

    /* Phase R16: C-suffix function-like macros (LLP64: 64-bit = long long) */
    cpp_define_func(ctx, "__INT8_C(c)",    "c");
    cpp_define_func(ctx, "__INT16_C(c)",   "c");
    cpp_define_func(ctx, "__INT32_C(c)",   "c");
    cpp_define_func(ctx, "__INT64_C(c)",   "c ## LL");
    cpp_define_func(ctx, "__INTMAX_C(c)",  "c ## LL");
    cpp_define_func(ctx, "__UINT8_C(c)",   "c");
    cpp_define_func(ctx, "__UINT16_C(c)",  "c");
    cpp_define_func(ctx, "__UINT32_C(c)",  "c ## U");
    cpp_define_func(ctx, "__UINT64_C(c)",  "c ## ULL");
    cpp_define_func(ctx, "__UINTMAX_C(c)", "c ## ULL");
}

/* ====================================================================== *
 * Public entry point.
 *
 * Recognised triples (initial set):
 *
 *   "x86_64-linux-gnu"          Linux x86_64, glibc, ELF, GCC ABI
 *   "aarch64-apple-darwin"      macOS arm64 (Apple Silicon), Mach-O, clang
 *   "x86_64-pc-windows-msvc"    Windows x64, COFF/PE, MSVC ABI (LLP64)
 *
 * Returns 0 on success, -1 if the triple is not recognised.  On -1 the
 * context is NOT modified (no partial state).  Callers may check the
 * return value and either fall back to a hand-rolled set, or fail the
 * build with a clear "unsupported target" message.
 *
 * The function is idempotent in the limited sense that calling it twice
 * with the same triple is harmless: the second call appends the same
 * cmdline ops, which the macro engine treats as redundant redefinitions
 * with identical bodies (no diagnostic).  Calling it with two DIFFERENT
 * triples on the same context is undefined — don't.
 * ====================================================================== */

int cpp_install_target_macros(CppCtx *ctx, const char *triple) {
    if (!ctx || !triple) return -1;

    if (strcmp(triple, "x86_64-linux-gnu") == 0) {
        install_x86_64_linux_gnu(ctx);
        return 0;
    }
    if (strcmp(triple, "aarch64-apple-darwin") == 0) {
        install_aarch64_apple_darwin(ctx);
        return 0;
    }
    if (strcmp(triple, "x86_64-pc-windows-msvc") == 0) {
        install_x86_64_pc_windows_msvc(ctx);
        return 0;
    }
    return -1;
}

/* ====================================================================== *
 * Enumerate supported triples.  NULL-terminated array for table-driven
 * callers (e.g. command-line tools that want to print "supported targets:
 * X, Y, Z" on --help).
 * ====================================================================== */

const char *const *cpp_target_known_triples(void) {
    static const char *const list[] = {
        "x86_64-linux-gnu",
        "aarch64-apple-darwin",
        "x86_64-pc-windows-msvc",
        NULL,
    };
    return list;
}
