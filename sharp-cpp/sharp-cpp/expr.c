/*
 * expr.c — Constant-expression evaluator for #if / #elif
 *
 * Implements the full C11 preprocessing constant-expression grammar:
 *
 *   expr      ::= ternary
 *   ternary   ::= logor ('?' expr ':' ternary)?
 *   logor     ::= logand ('||' logand)*
 *   logand    ::= bitor  ('&&' bitor)*
 *   bitor     ::= bitxor ('|'  bitxor)*
 *   bitxor    ::= bitand ('^'  bitand)*
 *   bitand    ::= equal  ('&'  equal)*
 *   equal     ::= relat  (('=='|'!=') relat)*
 *   relat     ::= shift  (('<'|'>'|'<='|'>=') shift)*
 *   shift     ::= add    (('<<'|'>>') add)*
 *   add       ::= mul    (('+'|'-') mul)*
 *   mul       ::= unary  (('*'|'/'|'%') unary)*
 *   unary     ::= ('+'|'-'|'~'|'!') unary | primary
 *   primary   ::= INT | CHAR | '(' expr ')' | 'defined' '(' NAME ')' | 'defined' NAME
 *
 * All arithmetic is performed in intmax_t / uintmax_t (§6.10.1 p4).
 * No floating-point is permitted in preprocessing constant expressions.
 */

#include "cpp_internal.h"
#include "expr.h"
#include "macro.h"

#include <inttypes.h>

typedef intmax_t  ival;
typedef uintmax_t uval;

/* =========================================================================
 * Value — a wide integer carrying its signed/unsigned-ness as part of the
 * type, per ISO C11 §6.10.1p4 and §6.3.1.8 (usual arithmetic conversions).
 *
 * Phase 3: previously the evaluator passed plain `intmax_t` around and
 * dropped the unsigned bit on the floor right after parsing the literal.
 * That made `(-1U) > 0` evaluate to false (the signed comparison saw -1 > 0).
 *
 * Now every intermediate result is a Value, and the binary operators
 * apply the standard "if either operand is unsigned, both promote to
 * unsigned" rule before computing.  Comparisons and logical operators
 * return a signed 0/1.
 *
 * The bit pattern is always stored in the .v field; the .is_unsigned bit
 * tells you how to interpret it.  This makes round-tripping through the
 * intmax_t-typed public API trivial.                                    */
typedef struct {
    intmax_t v;
    bool     is_unsigned;
} Value;

static Value vsigned(intmax_t x)   { return (Value){ x,           false }; }
static Value vunsigned(uintmax_t x){ return (Value){ (intmax_t)x, true  }; }
static Value vbool(bool b)         { return vsigned(b ? 1 : 0); }

/* Apply the C11 usual arithmetic conversions: if either operand is unsigned,
 * both become unsigned.  The bit patterns don't change — only the
 * interpretation flag — so this never loses information for intmax-sized
 * values.  Mutates both operands in place.                                */
static void usual_conv(Value *a, Value *b) {
    if (a->is_unsigned || b->is_unsigned) {
        a->is_unsigned = true;
        b->is_unsigned = true;
    }
}

/* =========================================================================
 * Token cursor over a TokList (skips whitespace automatically)
 * ====================================================================== */

typedef struct {
    TokNode      *cur;
    MacroTable   *mt;
    InternTable  *interns;
    CppDiagArr *diags;
    bool          had_error;
    /* Phase R4: when set, eval_* functions evaluate normally but do NOT
     * push diagnostics.  Used by the conditional operator to type-check
     * the unselected arm without polluting the diagnostic stream with
     * errors from code that won't be picked.  See n_13_7.c:
     *   #if  MACRO_0 ? 10 / MACRO_0 : 0
     * Without this, evaluating the then-arm to learn its type would
     * trigger a "division by zero" diagnostic even though the arm
     * isn't selected. */
    bool          suppress_diag;
} EvalCtx;

static const PPTok *ec_peek(EvalCtx *ec) {
    TokNode *n = ec->cur;
    while (n && (n->tok.kind == CPPT_SPACE || n->tok.kind == CPPT_NEWLINE ||
                 n->tok.kind == CPPT_COMMENT))
        n = n->next;
    return n ? &n->tok : NULL;
}

static void ec_skip_ws(EvalCtx *ec) {
    while (ec->cur && (ec->cur->tok.kind == CPPT_SPACE ||
                       ec->cur->tok.kind == CPPT_NEWLINE ||
                       ec->cur->tok.kind == CPPT_COMMENT))
        ec->cur = ec->cur->next;
}

static const PPTok *ec_get(EvalCtx *ec) {
    ec_skip_ws(ec);
    if (!ec->cur) return NULL;
    const PPTok *t = &ec->cur->tok;
    ec->cur = ec->cur->next;
    return t;
}

static bool ec_accept(EvalCtx *ec, const char *sp) {
    ec_skip_ws(ec);
    if (!ec->cur) return false;
    if (strcmp(pptok_spell(&ec->cur->tok), sp) == 0) {
        ec->cur = ec->cur->next;
        return true;
    }
    return false;
}

static void ec_error(EvalCtx *ec, CppLoc loc, const char *msg) {
    if (ec->suppress_diag) { ec->had_error = true; return; }
    CppDiag d = { CPP_DIAG_ERROR, loc, cpp_xstrdup(msg) };
    da_push(ec->diags, d);
    ec->had_error = true;
}

/* =========================================================================
 * Parse an integer constant (pp-number token)
 *
 * Returns the value with .is_unsigned set per the suffix:
 *   - bare integer:                    signed unless its magnitude exceeds
 *                                       INTMAX_MAX, in which case promoted
 *                                       to unsigned (per C11 §6.4.4.1p5)
 *   - U / u suffix:                     unsigned
 *   - L / LL / l / ll suffix:           signed (intmax)
 *   - UL / ULL / LU / LLU combinations: unsigned
 *
 * Note: the integer-promotion rule for hex/octal literals (where a value
 * that doesn't fit in `int` is promoted up the rank) collapses here
 * because we always operate at intmax width.  Effectively: if the literal
 * has a U suffix → unsigned; otherwise signed unless it overflows intmax.
 * ====================================================================== */

/* Phase R4: parse_int_literal_ex returns whether the literal overflowed.
 * The plain wrapper preserves the old API for code paths that don't have
 * an EvalCtx handy (e.g. apply_cmdline_macros).                         */
static Value parse_int_literal_ex(const char *sp, bool *overflow) {
    const char *p = sp;
    int base = 10;
    uval val = 0;
    bool is_unsigned = false;
    bool ovf = false;

    if (*p == '0') {
        p++;
        if (*p == 'x' || *p == 'X') { base = 16; p++; }
        else if (*p == 'b' || *p == 'B') { base = 2; p++; }
        else base = 8;
    }

    for (; *p; p++) {
        char c = (char)tolower((unsigned char)*p);
        int digit;
        if      (c >= '0' && c <= '9') digit = c - '0';
        else if (c >= 'a' && c <= 'f') digit = 10 + (c - 'a');
        else break; /* suffix */
        if (digit >= base) break;
        /* Check overflow before val*base + digit */
        uval limit = (uval)-1 / (uval)base;
        if (val > limit) ovf = true;
        uval new_val = val * (uval)base + (uval)digit;
        if (new_val < val) ovf = true;
        val = new_val;
    }
    /* Suffixes (any order, any combination of u/U/l/L allowed for tolerance). */
    while (*p == 'u' || *p == 'U' || *p == 'l' || *p == 'L') {
        if (*p == 'u' || *p == 'U') is_unsigned = true;
        p++;
    }

    /* C11 §6.4.4.1p5: a hex/octal/binary literal that doesn't fit in
     * intmax_t but does fit in uintmax_t is promoted to unsigned.  For
     * non-decimal literals a value with the high bit set on intmax_t is
     * already negative if interpreted signed, so promote to unsigned to
     * match the standard's intent (and what GCC/Clang actually do).    */
    if (!is_unsigned && base != 10 && (val >> 63) != 0) {
        is_unsigned = true;
    }

    if (overflow) *overflow = ovf;
    return (Value){ (intmax_t)val, is_unsigned };
}

static Value parse_int_literal(const char *sp) {
    return parse_int_literal_ex(sp, NULL);
}

/* Public wrapper used by directive.c — preserves the legacy return shape
 * (long + bool*) so callers that only care about the parsed value don't
 * need to refactor.  This is the variant declared in directive.h.       */
long parse_int_literal_pub(const char *sp, bool *is_unsigned) {
    Value v = parse_int_literal(sp);
    if (is_unsigned) *is_unsigned = v.is_unsigned;
    return (long)v.v;
}

/* =========================================================================
 * Parse a character constant (returns its numeric value as a Value).
 *
 * Phase 3 hardening: the prefix (`L`, `u`, `U`, `u8`) determines whether
 * the value is signed or unsigned per C11 §6.4.4.4:
 *   - 'a'      → int           (signed)
 *   - L'a'     → wchar_t       (implementation-defined; we use signed)
 *   - u'a'     → char16_t      (unsigned 16-bit)
 *   - U'a'     → char32_t      (unsigned 32-bit)
 *   - u8'a'    → char (C23)    (unsigned 8-bit)
 * Multi-character constants like 'AB' are implementation-defined; we
 * follow GCC and pack big-endian into the value.
 * ====================================================================== */

static Value parse_char_const_ex(const char *sp, int *char_count_out,
                                  bool *escape_overflow_out) {
    /* Determine prefix → signedness + width */
    bool is_unsigned = false;
    bool is_wide = false;
    if (sp[0] == 'L') { is_wide = true; }
    else if (sp[0] == 'u' && sp[1] == '8') is_unsigned = true;
    else if (sp[0] == 'u') { is_unsigned = true; is_wide = true; }
    else if (sp[0] == 'U') { is_unsigned = true; is_wide = true; }

    /* Skip prefix and opening quote */
    while (*sp && *sp != '\'') sp++;
    if (*sp == '\'') sp++;

    /* Read possibly-multiple character contents until the closing quote.
     * For multi-char constants pack into the value (big-endian, GCC-style). */
    uval val = 0;
    int char_count = 0;
    bool escape_overflow = false;
    while (*sp && *sp != '\'') {
        uval ch;
        if (*sp == '\\') {
            sp++;
            switch (*sp) {
            case 'n':  ch = '\n'; sp++; break;
            case 't':  ch = '\t'; sp++; break;
            case 'r':  ch = '\r'; sp++; break;
            case '\\': ch = '\\'; sp++; break;
            case '\'': ch = '\''; sp++; break;
            case '"':  ch = '"';  sp++; break;
            case 'a':  ch = '\a'; sp++; break;
            case 'b':  ch = '\b'; sp++; break;
            case 'f':  ch = '\f'; sp++; break;
            case 'v':  ch = '\v'; sp++; break;
            case '?':  ch = '?'; sp++; break;
            case 'x': {
                /* Hex escape — any number of hex digits */
                ch = 0; sp++;
                int hx = 0;
                while (isxdigit((unsigned char)*sp)) {
                    ch = ch*16 + (uval)(isdigit((unsigned char)*sp)
                                         ? *sp-'0'
                                         : tolower((unsigned char)*sp)-'a'+10);
                    sp++; hx++;
                }
                /* Phase R4 (u_2): per C99 §6.4.4.4/3 a hex escape
                 * requires at least one hex digit.  `\x` followed by
                 * a non-hex char is invalid.                          */
                if (hx == 0) escape_overflow = true;
                /* Phase R4: per C99 §6.4.4.4/9 a hex escape's value
                 * must fit in the destination char type.
                 *   narrow char: UCHAR_MAX = 0xFF
                 *   wide char  : on most targets wchar_t is 32-bit;
                 *               we require value <= 0xFFFFFFFFu.
                 * mcpp's e_32_5 (narrow) and e_33_2 (wide) exercise
                 * these.                                              */
                if (!is_wide && ch > 0xFFu) escape_overflow = true;
                if (is_wide && ch > 0xFFFFFFFFu) escape_overflow = true;
                break;
            }
            /* Phase R4: Universal Character Names (C99 §6.4.3).
             * \uHHHH    — 4 hex digits  (BMP code point)
             * \UHHHHHHHH — 8 hex digits (full Unicode code point)
             * Each is a SINGLE char (not multi-char), so char_count
             * increments by 1.  Prior to this, sharp-cpp fell through
             * to `default` and treated each digit as a separate char,
             * so `'\U00006F22'` was counted as 10 chars and incorrectly
             * triggered "character constant too long".  mcpp's
             * n_ucn1.c exercises this.                                */
            case 'u': case 'U': {
                int hex_digits = (*sp == 'u') ? 4 : 8;
                sp++;
                ch = 0;
                int got = 0;
                while (got < hex_digits && isxdigit((unsigned char)*sp)) {
                    ch = ch*16 + (uval)(isdigit((unsigned char)*sp)
                                         ? *sp-'0'
                                         : tolower((unsigned char)*sp)-'a'+10);
                    sp++; got++;
                }
                break;
            }
            /* Phase R4: octal escape covers \0..\7 prefix.
             * Per C99 §6.4.4.4, an octal-escape-sequence is `\` followed
             * by 1-3 octal digits.  The leading digit may be 0, so `\0`
             * (NUL), `\033` (ESC), and `\123` are all valid octal forms.
             * Previously `case '0'` was bundled with `case '?'` and set
             * `ch = '?'`, which silently corrupted any character constant
             * starting with `\0NN`.  mcpp's n_32.c exposed this through
             * `'\033' != '\x1b'` (both should be 27).                   */
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7': {
                /* Octal escape — up to 3 digits */
                ch = 0;
                int count = 0;
                while (count < 3 && *sp >= '0' && *sp <= '7') {
                    ch = ch*8 + (uval)(*sp - '0');
                    sp++; count++;
                }
                break;
            }
            default:
                ch = (unsigned char)*sp;
                if (*sp) sp++;
                break;
            }
        } else {
            ch = (unsigned char)*sp;
            sp++;
        }
        char_count++;
        /* Pack into accumulator (big-endian within the wide value) */
        val = (val << 8) | (ch & 0xFFu);
    }

    if (char_count_out) *char_count_out = char_count;
    if (escape_overflow_out) *escape_overflow_out = escape_overflow;
    return (Value){ (intmax_t)val, is_unsigned };
}

static Value parse_char_const(const char *sp) {
    return parse_char_const_ex(sp, NULL, NULL);
}

/* =========================================================================
 * Recursive descent evaluator
 *
 * Phase 3: every eval_* function returns a Value, and binary operators
 * apply usual_conv() before computing.  Comparisons and the logical
 * operators (&&, ||, !) return signed 0/1.  Shifts take the type of
 * their LEFT operand (per C11 §6.5.7p3).
 * ====================================================================== */

static Value eval_expr(EvalCtx *ec);

/* =========================================================================
 * GCC builtin / attribute membership tables.
 * These are file-scope so they appear in the binary only once.
 * ======================================================================= */

/* GCC 13: 303 builtins confirmed via #if __has_builtin(...) probe. */
static const char *const kGccBuiltins[] = {

                    "__builtin_FILE", "__builtin_FUNCTION", "__builtin_LINE",
                    "__builtin__exit", "__builtin_abort", "__builtin_abs",
                    "__builtin_add_overflow", "__builtin_add_overflow_p",
                    "__builtin_alloca", "__builtin_alloca_with_align",
                    "__builtin_apply", "__builtin_apply_args",
                    "__builtin_assume_aligned",
                    "__builtin_atan2", "__builtin_atan2f", "__builtin_atan2l",
                    "__builtin_bswap128", "__builtin_bswap16",
                    "__builtin_bswap32", "__builtin_bswap64",
                    "__builtin_cabs", "__builtin_cabsf", "__builtin_cabsl",
                    "__builtin_calloc",
                    "__builtin_carg", "__builtin_cargf", "__builtin_cargl",
                    "__builtin_cbrt", "__builtin_cbrtf", "__builtin_cbrtl",
                    "__builtin_ceil", "__builtin_ceilf", "__builtin_ceill",
                    "__builtin_cexp", "__builtin_cexpf", "__builtin_cexpl",
                    "__builtin_choose_expr",
                    "__builtin_cimag", "__builtin_cimagf", "__builtin_cimagl",
                    "__builtin_classify_type",
                    "__builtin_clog", "__builtin_clogf", "__builtin_clogl",
                    "__builtin_clrsb", "__builtin_clrsbl", "__builtin_clrsbll",
                    "__builtin_clz", "__builtin_clzl", "__builtin_clzll",
                    "__builtin_conj", "__builtin_conjf", "__builtin_conjl",
                    "__builtin_constant_p",
                    "__builtin_copysign", "__builtin_copysignf", "__builtin_copysignl",
                    "__builtin_cos", "__builtin_cosf", "__builtin_cosl",
                    "__builtin_cproj", "__builtin_cprojf", "__builtin_cprojl",
                    "__builtin_cpu_init", "__builtin_cpu_is", "__builtin_cpu_supports",
                    "__builtin_creal", "__builtin_crealf", "__builtin_creall",
                    "__builtin_csqrt", "__builtin_csqrtf", "__builtin_csqrtl",
                    "__builtin_ctz", "__builtin_ctzl", "__builtin_ctzll",
                    "__builtin_dwarf_cfa", "__builtin_dwarf_sp_column",
                    "__builtin_dynamic_object_size",
                    "__builtin_eh_return", "__builtin_eh_return_data_regno",
                    "__builtin_exit",
                    "__builtin_exp", "__builtin_exp2", "__builtin_exp2f",
                    "__builtin_exp2l", "__builtin_expect",
                    "__builtin_expect_with_probability",
                    "__builtin_expf", "__builtin_expl",
                    "__builtin_expm1", "__builtin_expm1f", "__builtin_expm1l",
                    "__builtin_extract_return_addr",
                    "__builtin_fabs", "__builtin_fabsf", "__builtin_fabsl",
                    "__builtin_ffs", "__builtin_ffsl", "__builtin_ffsll",
                    "__builtin_floor", "__builtin_floorf", "__builtin_floorl",
                    "__builtin_fma", "__builtin_fmaf", "__builtin_fmal",
                    "__builtin_fmax", "__builtin_fmaxf", "__builtin_fmaxl",
                    "__builtin_fmin", "__builtin_fminf", "__builtin_fminl",
                    "__builtin_fmod", "__builtin_fmodf", "__builtin_fmodl",
                    "__builtin_fpclassify",
                    "__builtin_fprintf", "__builtin_fputc", "__builtin_fputs",
                    "__builtin_frame_address", "__builtin_free",
                    "__builtin_frexp", "__builtin_frexpf", "__builtin_frexpl",
                    "__builtin_frob_return_addr",
                    "__builtin_fscanf",
                    "__builtin_huge_val", "__builtin_huge_valf", "__builtin_huge_vall",
                    "__builtin_hypot", "__builtin_hypotf", "__builtin_hypotl",
                    "__builtin_ia32_pause", "__builtin_ia32_rdtsc", "__builtin_ia32_rdtscp",
                    "__builtin_ilogb", "__builtin_ilogbf", "__builtin_ilogbl",
                    "__builtin_inf", "__builtin_inff", "__builtin_infl",
                    "__builtin_init_dwarf_reg_size_table",
                    "__builtin_isalnum", "__builtin_isalpha", "__builtin_isblank",
                    "__builtin_iscntrl", "__builtin_isdigit",
                    "__builtin_isfinite", "__builtin_isgraph",
                    "__builtin_isgreater", "__builtin_isgreaterequal",
                    "__builtin_isinf", "__builtin_isinf_sign",
                    "__builtin_isless", "__builtin_islessequal",
                    "__builtin_islessgreater", "__builtin_islower",
                    "__builtin_isnan", "__builtin_isnormal",
                    "__builtin_isprint", "__builtin_ispunct",
                    "__builtin_isspace", "__builtin_isunordered", "__builtin_isupper",
                    "__builtin_ldexp", "__builtin_ldexpf", "__builtin_ldexpl",
                    "__builtin_llrint", "__builtin_llrintf", "__builtin_llrintl",
                    "__builtin_llround", "__builtin_llroundf", "__builtin_llroundl",
                    "__builtin_log", "__builtin_log10", "__builtin_log10f",
                    "__builtin_log10l", "__builtin_log1p", "__builtin_log1pf",
                    "__builtin_log1pl", "__builtin_log2", "__builtin_log2f",
                    "__builtin_log2l", "__builtin_logb", "__builtin_logbf",
                    "__builtin_logbl", "__builtin_logf", "__builtin_logl",
                    "__builtin_longjmp",
                    "__builtin_lrint", "__builtin_lrintf", "__builtin_lrintl",
                    "__builtin_lround", "__builtin_lroundf", "__builtin_lroundl",
                    "__builtin_malloc",
                    "__builtin_memchr", "__builtin_memcmp", "__builtin_memcpy",
                    "__builtin_memmove", "__builtin_mempcpy", "__builtin_memset",
                    "__builtin_modf", "__builtin_modff", "__builtin_modfl",
                    "__builtin_mul_overflow", "__builtin_mul_overflow_p",
                    "__builtin_nan", "__builtin_nanf", "__builtin_nanl",
                    "__builtin_nans", "__builtin_nansf", "__builtin_nansl",
                    "__builtin_nearbyint", "__builtin_nearbyintf", "__builtin_nearbyintl",
                    "__builtin_object_size", "__builtin_offsetof",
                    "__builtin_parity", "__builtin_parityl", "__builtin_parityll",
                    "__builtin_popcount", "__builtin_popcountl", "__builtin_popcountll",
                    "__builtin_pow", "__builtin_powf",
                    "__builtin_powi", "__builtin_powif", "__builtin_powil",
                    "__builtin_powl", "__builtin_prefetch",
                    "__builtin_printf", "__builtin_puts",
                    "__builtin_realloc",
                    "__builtin_remainder", "__builtin_remainderf", "__builtin_remainderl",
                    "__builtin_remquo", "__builtin_remquof", "__builtin_remquol",
                    "__builtin_return_address",
                    "__builtin_rint", "__builtin_rintf", "__builtin_rintl",
                    "__builtin_round", "__builtin_roundf", "__builtin_roundl",
                    "__builtin_sadd_overflow",
                    "__builtin_scalbn", "__builtin_scalbnf", "__builtin_scalbnl",
                    "__builtin_scanf",
                    "__builtin_setjmp",
                    "__builtin_signbit", "__builtin_signbitf", "__builtin_signbitl",
                    "__builtin_sin", "__builtin_sinf", "__builtin_sinl",
                    "__builtin_smul_overflow",
                    "__builtin_snprintf", "__builtin_sprintf",
                    "__builtin_sqrt", "__builtin_sqrtf", "__builtin_sqrtl",
                    "__builtin_sscanf", "__builtin_ssub_overflow",
                    "__builtin_stack_restore", "__builtin_stack_save",
                    "__builtin_stpcpy", "__builtin_stpncpy",
                    "__builtin_strcat", "__builtin_strchr", "__builtin_strcmp",
                    "__builtin_strcpy", "__builtin_strlen", "__builtin_strncmp",
                    "__builtin_strncpy", "__builtin_strpbrk", "__builtin_strrchr",
                    "__builtin_strstr",
                    "__builtin_sub_overflow", "__builtin_sub_overflow_p",
                    "__builtin_tolower", "__builtin_toupper",
                    "__builtin_trap", "__builtin_trunc", "__builtin_truncf",
                    "__builtin_truncl",
                    "__builtin_types_compatible_p",
                    "__builtin_uadd_overflow", "__builtin_umul_overflow",
                    "__builtin_unreachable", "__builtin_unwind_init",
                    "__builtin_update_setjmp_buf", "__builtin_usub_overflow",
                    "__builtin_va_copy",
                    "__builtin_vfprintf", "__builtin_vfscanf",
                    "__builtin_vprintf", "__builtin_vscanf",
                    "__builtin_vsnprintf", "__builtin_vsprintf", "__builtin_vsscanf",
                    /* strtol family */
                    "__builtin_strtol", "__builtin_strtoul",
                    "__builtin_strtoll", "__builtin_strtoull",
                    "__builtin_strtof", "__builtin_strtod", "__builtin_strtold",
                    NULL
};

/* GCC 13: ~81 attributes confirmed via gcc manual §6.33. */
static const char *const kGccAttrs[] = {

                /* function attributes */
                "alias", "aligned", "alloc_align", "alloc_size",
                "always_inline", "artificial", "assume_aligned",
                "cold", "const", "constructor", "destructor",
                "deprecated", "error", "externally_visible",
                "fallthrough", "flatten", "format", "format_arg",
                "gnu_inline", "hot", "ifunc", "interrupt",
                "leaf", "malloc", "may_alias", "mode",
                "no_caller_saved_registers", "no_icf",
                "no_instrument_function", "no_profile_instrument_function",
                "no_reorder", "no_sanitize", "no_sanitize_address",
                "no_sanitize_undefined", "no_split_stack",
                "no_stack_limit", "noclone", "noinline",
                "noipa", "nonnull", "noplt", "noreturn",
                "nothrow", "optimize", "patchable_function_entry",
                "pure", "returns_nonnull", "returns_twice",
                "section", "sentinel", "simd", "stack_protect",
                "symver", "target", "target_clones",
                "tls_model", "transaction_callable",
                "transaction_pure", "transaction_unsafe",
                "unavailable", "unused", "used", "visibility",
                "warn_unused_result", "warning", "weakref",
                "weak", "retain", "access", "fd_arg", "fd_arg_read",
                "fd_arg_write", "null_terminated_string_arg",
                /* variable / type attributes */
                "cleanup", "common", "copy", "designated_init",
                "nonstring", "objc_root_class", "packed",
                "scalar_storage_order", "transparent_union",
                "vector_size",
                /* Phase R17: indirect_return (GCC 13, CET/shadow-stack) */
                "indirect_return",
                NULL
};

static bool table_contains(const char *const *table, const char *name) {
    for (; *table; table++)
        if (strcmp(*table, name) == 0) return true;
    return false;
}

static Value eval_primary(EvalCtx *ec) {
    ec_skip_ws(ec);
    const PPTok *t = ec_get(ec);
    if (!t) { ec->had_error = true; return vsigned(0); }

    /* Parenthesised expression */
    if (t->kind == CPPT_PUNCT && strcmp(pptok_spell(t), "(") == 0) {
        Value v = eval_expr(ec);
        if (!ec_accept(ec, ")")) {
            ec_error(ec, t->loc, "expected ')' in #if expression");
        }
        return v;
    }

    /* defined(NAME) or defined NAME — always returns signed 0 or 1 */
    if (t->kind == CPPT_IDENT && strcmp(pptok_spell(t), "defined") == 0) {
        bool paren = ec_accept(ec, "(");
        ec_skip_ws(ec);
        const PPTok *name_tok = ec_get(ec);
        if (!name_tok || name_tok->kind != CPPT_IDENT) {
            ec_error(ec, t->loc, "expected macro name after 'defined'");
            if (paren) ec_accept(ec, ")");
            return vsigned(0);
        }
        const char *name = pptok_spell(name_tok);
        if (paren && !ec_accept(ec, ")"))
            ec_error(ec, t->loc, "expected ')' after defined(name)");
        /* Phase R5: __has_builtin / __has_feature / __has_attribute /
         * __has_include / __has_include_next family is implicitly
         * "defined" — see install_builtins comment in directive.c.    */
        if (is_has_family(name)) return vbool(true);
        return vbool(macro_lookup(ec->mt, name) != NULL);
    }

    /* Phase R4 / R13: GCC/Clang's __has_builtin / __has_attribute /
     * __has_feature / __has_extension / __has_c_attribute family.
     *
     * Phase R13: __has_attribute now uses a real attribute table so that
     * `__has_attribute(__nothrow__)` etc. returns 1 for the ~80 attributes
     * GCC 13 supports, matching `gcc -E` output on glibc/musl/linux headers.
     * All others return 0.  __has_builtin/__has_feature/__has_extension
     * still conservatively return 0 (sufficient for all current test corpus).
     */
    if (t->kind == CPPT_IDENT) {
        const char *idname = pptok_spell(t);
        bool is_has_attr    = (strcmp(idname, "__has_attribute")    == 0);
        bool is_has_builtin = (strcmp(idname, "__has_builtin")      == 0);
        bool is_has_other   = (!is_has_attr && !is_has_builtin &&
                               (strcmp(idname, "__has_feature")      == 0 ||
                                strcmp(idname, "__has_extension")    == 0 ||
                                strcmp(idname, "__has_c_attribute")  == 0 ||
                                strcmp(idname, "__has_cpp_attribute")== 0));
        if (is_has_attr || is_has_builtin || is_has_other) {
            /* Collect the single identifier argument (strip leading __ / __). */
            char argbuf[128] = "";
            ec_skip_ws(ec);
            if (ec_accept(ec, "(")) {
                ec_skip_ws(ec);
                const PPTok *arg = ec_get(ec);
                if (arg && arg->kind == CPPT_IDENT) {
                    const char *sp = pptok_spell(arg);
                    /* Strip leading and trailing __ e.g. __nothrow__ → nothrow */
                    size_t slen = strlen(sp);
                    const char *inner = sp;
                    if (slen > 4 && sp[0]=='_' && sp[1]=='_' &&
                        sp[slen-1]=='_' && sp[slen-2]=='_') {
                        inner = sp + 2;
                        slen  = slen - 4;
                    }
                    if (slen < sizeof argbuf) {
                        memcpy(argbuf, inner, slen);
                        argbuf[slen] = '\0';
                    }
                }
                /* Consume rest until matching ')' */
                int depth = 1;
                while (ec->cur && depth > 0) {
                    const PPTok *p = ec_get(ec);
                    if (!p) break;
                    if (p->kind == CPPT_PUNCT) {
                        const char *psp = pptok_spell(p);
                        if (strcmp(psp, "(") == 0) depth++;
                        else if (strcmp(psp, ")") == 0) depth--;
                    }
                }
            }

            if (is_has_other)   return vsigned(0);
            if (is_has_builtin) return vbool(table_contains(kGccBuiltins, argbuf));


            return vbool(table_contains(kGccAttrs, argbuf));
        }
    }

    /* Integer constant */
    if (t->kind == CPPT_PP_NUMBER) {
        /* Phase R4: detect overflow in the integer parse so that a
         * literal too large for uintmax_t raises a diagnostic instead
         * of silently truncating.  mcpp's e_12_8.c exercises this with
         * `123456789012345678901` which is wider than uintmax_t.       */
        bool ovf = false;
        Value v = parse_int_literal_ex(pptok_spell(t), &ovf);
        if (ovf) {
            char msg[160];
            snprintf(msg, sizeof msg,
                     "integer constant '%s' is out of range for its type",
                     pptok_spell(t));
            ec_error(ec, t->loc, msg);
        }
        return v;
    }

    /* Character constant */
    if (t->kind == CPPT_CHAR_CONST) {
        /* Phase R4: per C99 §6.4.4.4/2 a char constant must contain at
         * least one c-char (or escape sequence).  '' is a constraint
         * violation.  mcpp's e_4_3.c exercises this.                  */
        const char *sp_chk = pptok_spell(t);
        const char *qp = strchr(sp_chk, '\'');
        if (qp && qp[1] == '\'') {
            ec_error(ec, t->loc, "empty character constant");
            return vsigned(0);
        }
        /* Phase R4: detect multi-char constants longer than the value
         * type can hold.  Per C99 §6.4.4.4 multi-char constants are
         * implementation-defined, but more than sizeof(uintmax_t)
         * (8 bytes / 8 chars) characters can never fit even if we
         * pack one byte per char.  mcpp's e_35_2.c does
         *     #if 'abcdefghi'        (9 chars: triggers diagnostic)
         */
        int char_count = 0;
        bool escape_overflow = false;
        Value v = parse_char_const_ex(sp_chk, &char_count, &escape_overflow);
        if (char_count > 8) {
            char msg[160];
            snprintf(msg, sizeof msg,
                "character constant too long for its type "
                "(%d characters; max 8)", char_count);
            ec_error(ec, t->loc, msg);
        }
        if (escape_overflow) {
            ec_error(ec, t->loc,
                "invalid escape sequence in character constant "
                "(no following hex digits, or value out of range for char)");
        }
        return v;
    }

    /* Unexpanded identifier — evaluates to 0 (§6.10.1 p4) */
    if (t->kind == CPPT_IDENT) {
        return vsigned(0);
    }

    char msg[128];
    snprintf(msg, sizeof msg, "unexpected token '%s' in #if expression",
             pptok_spell(t));
    ec_error(ec, t->loc, msg);
    return vsigned(0);
}

static Value eval_unary(EvalCtx *ec) {
    ec_skip_ws(ec);
    const PPTok *t = ec_peek(ec);
    if (!t) return eval_primary(ec);

    const char *sp = pptok_spell(t);
    if (t->kind == CPPT_PUNCT) {
        if (strcmp(sp, "+") == 0) {
            ec_get(ec);
            return eval_unary(ec);
        }
        if (strcmp(sp, "-") == 0) {
            ec_get(ec);
            Value x = eval_unary(ec);
            /* Unary minus on unsigned wraps modulo 2^N (intmax width); on
             * signed it's arithmetic negation.  Both cases are covered by
             * negating the bit pattern as uintmax (two's complement).   */
            if (x.is_unsigned)
                return vunsigned(0u - (uintmax_t)x.v);
            else
                return vsigned(-x.v);
        }
        if (strcmp(sp, "~") == 0) {
            ec_get(ec);
            Value x = eval_unary(ec);
            x.v = ~x.v;
            return x; /* preserves is_unsigned */
        }
        if (strcmp(sp, "!") == 0) {
            ec_get(ec);
            Value x = eval_unary(ec);
            return vbool(x.v == 0);
        }
    }
    return eval_primary(ec);
}

static Value eval_mul(EvalCtx *ec) {
    Value v = eval_unary(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if (strcmp(sp, "*") == 0) {
            ec_get(ec);
            Value r = eval_unary(ec);
            usual_conv(&v, &r);
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v * (uintmax_t)r.v);
            else
                v.v = v.v * r.v;
        }
        else if (strcmp(sp, "/") == 0) {
            ec_get(ec);
            Value r = eval_unary(ec);
            usual_conv(&v, &r);
            if (r.v == 0) {
                ec_error(ec, t->loc, "division by zero in #if");
                /* Make division yield a deterministic 0 in the
                 * suppressed-diag case so the (unselected) ternary arm
                 * still has a usable type/value to feed usual_conv.   */
                v.v = 0;
                break;
            }
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v / (uintmax_t)r.v);
            else
                v.v = v.v / r.v;
        }
        else if (strcmp(sp, "%") == 0) {
            ec_get(ec);
            Value r = eval_unary(ec);
            usual_conv(&v, &r);
            if (r.v == 0) {
                ec_error(ec, t->loc, "modulo by zero in #if");
                v.v = 0;
                break;
            }
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v % (uintmax_t)r.v);
            else
                v.v = v.v % r.v;
        }
        else break;
    }
    return v;
}

static Value eval_add(EvalCtx *ec) {
    Value v = eval_mul(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if (strcmp(sp, "+") == 0) {
            ec_get(ec);
            Value r = eval_mul(ec);
            usual_conv(&v, &r);
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v + (uintmax_t)r.v);
            else {
                /* Phase R4 (e_intmax): per C99 §6.5/5 signed integer
                 * overflow is undefined behavior; in #if expressions
                 * mcpp wants this reported as an error.  Detect by
                 * checking whether the result's sign disagrees with
                 * what arithmetic on signs would predict.            */
                intmax_t a = v.v, b = r.v;
                intmax_t s = (intmax_t)((uintmax_t)a + (uintmax_t)b);
                if (((a ^ s) & (b ^ s)) < 0)
                    ec_error(ec, t->loc,
                        "integer overflow in preprocessor expression");
                v.v = s;
            }
        }
        else if (strcmp(sp, "-") == 0) {
            ec_get(ec);
            Value r = eval_mul(ec);
            usual_conv(&v, &r);
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v - (uintmax_t)r.v);
            else {
                intmax_t a = v.v, b = r.v;
                intmax_t s = (intmax_t)((uintmax_t)a - (uintmax_t)b);
                if (((a ^ b) & (a ^ s)) < 0)
                    ec_error(ec, t->loc,
                        "integer overflow in preprocessor expression");
                v.v = s;
            }
        }
        else break;
    }
    return v;
}

static Value eval_shift(EvalCtx *ec) {
    /* C11 §6.5.7p3: the type of the shift result is that of the LEFT
     * operand after promotion.  We do NOT apply usual_conv here. */
    Value v = eval_add(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        if (strcmp(sp, "<<") == 0) {
            ec_get(ec);
            Value r = eval_add(ec);
            uintmax_t amt = (uintmax_t)r.v & 63u; /* mask to width-1 */
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v << amt);
            else
                v.v = v.v << amt;
        }
        else if (strcmp(sp, ">>") == 0) {
            ec_get(ec);
            Value r = eval_add(ec);
            uintmax_t amt = (uintmax_t)r.v & 63u;
            if (v.is_unsigned)
                v.v = (intmax_t)((uintmax_t)v.v >> amt);
            else
                v.v = v.v >> amt;  /* implementation-defined, GCC keeps sign */
        }
        else break;
    }
    return v;
}

static Value eval_relat(EvalCtx *ec) {
    Value v = eval_shift(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        bool is_lt = (strcmp(sp, "<")  == 0);
        bool is_gt = (strcmp(sp, ">")  == 0);
        bool is_le = (strcmp(sp, "<=") == 0);
        bool is_ge = (strcmp(sp, ">=") == 0);
        if (!(is_lt || is_gt || is_le || is_ge)) break;
        ec_get(ec);
        Value r = eval_shift(ec);
        usual_conv(&v, &r);
        bool result;
        if (v.is_unsigned) {
            uintmax_t a = (uintmax_t)v.v, b = (uintmax_t)r.v;
            result = is_lt ? (a <  b)
                  :  is_gt ? (a >  b)
                  :  is_le ? (a <= b)
                  :          (a >= b);
        } else {
            intmax_t a = v.v, b = r.v;
            result = is_lt ? (a <  b)
                  :  is_gt ? (a >  b)
                  :  is_le ? (a <= b)
                  :          (a >= b);
        }
        v = vbool(result);
    }
    return v;
}

static Value eval_equal(EvalCtx *ec) {
    Value v = eval_relat(ec);
    for (;;) {
        ec_skip_ws(ec);
        const PPTok *t = ec_peek(ec);
        if (!t || t->kind != CPPT_PUNCT) break;
        const char *sp = pptok_spell(t);
        bool is_eq = (strcmp(sp, "==") == 0);
        bool is_ne = (strcmp(sp, "!=") == 0);
        if (!(is_eq || is_ne)) break;
        ec_get(ec);
        Value r = eval_relat(ec);
        usual_conv(&v, &r);
        /* Equality is bit-pattern identical for signed/unsigned at this
         * width — since usual_conv only flips interpretation, never bits. */
        bool result = (v.v == r.v);
        v = vbool(is_eq ? result : !result);
    }
    return v;
}

static Value eval_bitand(EvalCtx *ec) {
    Value v = eval_equal(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "&") == 0) {
        ec_get(ec);
        Value r = eval_equal(ec);
        usual_conv(&v, &r);
        v.v &= r.v;
    }
    return v;
}

static Value eval_bitxor(EvalCtx *ec) {
    Value v = eval_bitand(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "^") == 0) {
        ec_get(ec);
        Value r = eval_bitand(ec);
        usual_conv(&v, &r);
        v.v ^= r.v;
    }
    return v;
}

static Value eval_bitor(EvalCtx *ec) {
    Value v = eval_bitxor(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "|") == 0) {
        ec_get(ec);
        Value r = eval_bitxor(ec);
        usual_conv(&v, &r);
        v.v |= r.v;
    }
    return v;
}

/* Skip an expression without evaluating (for short-circuit).
 * Skips balanced parentheses, unary operators, identifiers, literals, etc.
 * Stops at comma, ')', or end of tokens. */
static void ec_skip_expr(EvalCtx *ec) {
    int depth = 0;
    int tern_depth = 0;
    while (ec->cur) {
        ec_skip_ws(ec);
        if (!ec->cur) break;
        const char *sp = pptok_spell(ec_peek(ec));
        if (strcmp(sp, "(") == 0) { depth++; ec_get(ec); }
        else if (strcmp(sp, ")") == 0) {
            if (depth == 0) break;
            depth--; ec_get(ec);
        }
        else if (strcmp(sp, ",") == 0 && depth == 0) break;
        else if (strcmp(sp, "?") == 0 && depth == 0) {
            tern_depth++; ec_get(ec);
        }
        else if (strcmp(sp, ":") == 0 && depth == 0) {
            if (tern_depth == 0) break;
            tern_depth--; ec_get(ec);
        }
        else if (strcmp(sp, "||") == 0 && depth == 0 && tern_depth == 0) break;
        else if (strcmp(sp, "&&") == 0 && depth == 0 && tern_depth == 0) break;
        else ec_get(ec);
    }
}

/* Phase R14: skip a full logand-level expression for the short-circuit
 * path of eval_logor.  ec_skip_expr stops at && (bitor level), which is
 * correct for eval_logand's short-circuit, but in eval_logor we must
 * consume the ENTIRE right-hand logand operand which may itself contain
 * && chains.  Without this, `1 || 0 && 0` would leave `&& 0` in the
 * stream, causing eval_logor to return early and the caller to see a
 * stale && that corrupts the surrounding expression.
 *
 * This is the bug that caused `AV_GCC_VERSION_AT_LEAST(2,6)` to evaluate
 * as 0 instead of 1: `__GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 6`
 * was read as `((__GNUC__ > 2) || (__GNUC__ == 2)) && ...` = `1 && 0` = 0.
 */
static void ec_skip_logand(EvalCtx *ec) {
    /* skip one bitor operand then consume any trailing && chains */
    while (true) {
        ec_skip_expr(ec);      /* stops at && or || (bitor level) */
        ec_skip_ws(ec);
        if (!ec->cur) break;
        const char *sp = pptok_spell(ec_peek(ec));
        if (strcmp(sp, "&&") == 0) {
            ec_get(ec);        /* consume && and loop to skip next operand */
        } else {
            break;             /* stopped at ||, ,, ), EOF, etc. */
        }
    }
}

static Value eval_logand(EvalCtx *ec) {
    Value v = eval_bitor(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "&&") == 0) {
        ec_get(ec);
        if (v.v != 0) {
            Value r = eval_bitor(ec);
            v = vbool(r.v != 0);
        } else {
            ec_skip_expr(ec);
            v = vbool(false);
        }
    }
    return v;
}

static Value eval_logor(EvalCtx *ec) {
    Value v = eval_logand(ec);
    while (ec_skip_ws(ec), ec_peek(ec) && ec_peek(ec)->kind == CPPT_PUNCT &&
           strcmp(pptok_spell(ec_peek(ec)), "||") == 0) {
        ec_get(ec);
        if (v.v == 0) {
            Value r = eval_logand(ec);
            v = vbool(r.v != 0);
        } else {
            ec_skip_logand(ec);  /* Phase R14: skip full && chain, not just one bitor */
            v = vbool(true);
        }
    }
    return v;
}

static Value eval_expr(EvalCtx *ec) {
    /* Handle leading comma: treat as (0, rest_of_expression) */
    ec_skip_ws(ec);
    if (ec->cur && ec->cur->tok.kind == CPPT_PUNCT &&
        pptok_spell(&ec->cur->tok) && strcmp(pptok_spell(&ec->cur->tok), ",") == 0) {
        ec_get(ec);  /* consume comma */
        ec_skip_ws(ec);
        Value cond = eval_logor(ec);
        ec_skip_ws(ec);
        while (ec->cur && ec->cur->tok.kind == CPPT_PUNCT &&
               pptok_spell(&ec->cur->tok) && strcmp(pptok_spell(&ec->cur->tok), ",") == 0) {
            ec_get(ec);
            ec_skip_ws(ec);
            if (ec->cur && ec->cur->tok.kind != CPPT_NEWLINE && ec->cur->tok.kind != CPPT_EOF) {
                cond = eval_logor(ec);
                ec_skip_ws(ec);
            } else {
                break;
            }
        }
        return cond;
    }

    Value cond = eval_logor(ec);
    ec_skip_ws(ec);
    const PPTok *t = ec_peek(ec);
    if (t && t->kind == CPPT_PUNCT && strcmp(pptok_spell(t), "?") == 0) {
        ec_get(ec);
        /* Phase R4: per C99 §6.5.15/5, the result type is determined
         * by usual_conv on the two arms.  But §6.5.15/4 requires
         * short-circuit: only the selected arm is "evaluated" — its
         * side effects must not happen if not selected.  In a #if
         * context there are no side effects, but mcpp's n_13_7.c does
         *   #if  MACRO_0 ? 10 / MACRO_0 : 0
         * which would emit a spurious "division by zero" if we let
         * the unselected arm push diagnostics.
         *
         * Solution: evaluate both arms — selected arm normally,
         * unselected arm with `suppress_diag` so its errors don't
         * surface but its type info is still computed for usual_conv.
         * This preserves both n_13_5 (type promotion through ternary)
         * and n_13_7 (short-circuit hides division by zero).         */
        bool prev_supp = ec->suppress_diag;
        bool select_then = (cond.v != 0);

        Value then_val, else_val;
        if (select_then) {
            then_val = eval_expr(ec);
            ec_accept(ec, ":");
            ec->suppress_diag = true;
            else_val = eval_expr(ec);
            ec->suppress_diag = prev_supp;
        } else {
            ec->suppress_diag = true;
            then_val = eval_expr(ec);
            ec->suppress_diag = prev_supp;
            ec_accept(ec, ":");
            else_val = eval_expr(ec);
        }

        Value a = then_val, b = else_val;
        usual_conv(&a, &b);
        Value selected = select_then ? then_val : else_val;
        selected.is_unsigned = a.is_unsigned;  /* both equal after usual_conv */
        return selected;
    }
    /* Trailing comma operator(s) */
    while (ec->cur && ec->cur->tok.kind == CPPT_PUNCT &&
           pptok_spell(&ec->cur->tok) && strcmp(pptok_spell(&ec->cur->tok), ",") == 0) {
        ec_get(ec);
        ec_skip_ws(ec);
        if (ec->cur && ec->cur->tok.kind != CPPT_NEWLINE && ec->cur->tok.kind != CPPT_EOF) {
            cond = eval_logor(ec);
            ec_skip_ws(ec);
        } else {
            break;
        }
    }
    return cond;
}

/* =========================================================================
 * Public entry point
 *
 * Returns intmax_t for backward compatibility — the unsigned-ness flag is
 * relevant only inside the evaluator, where it dictates which arithmetic
 * conversions apply.  Comparisons and logical operators always reduce to
 * 0/1, so the caller observing a non-zero return for a `#if` directive
 * works the same regardless of internal type.                          */

intmax_t cpp_eval_if_expr(TokList *toks, MacroTable *mt,
                          InternTable *interns, CppDiagArr *diags,
                          bool *error_out) {
    EvalCtx ec = {
        .cur     = toks->head,
        .mt      = mt,
        .interns = interns,
        .diags   = diags,
        .had_error = false,
        .suppress_diag = false,
    };
    if (!ec.cur || ec.cur->tok.kind == CPPT_NEWLINE) {
        ec_error(&ec, (CppLoc){0}, "#if with no expression");
        if (error_out) *error_out = true;
        return 0;
    }
    Value v = eval_expr(&ec);
    /* Phase R4: trailing token check.  After eval_expr returns the
     * cursor should point at end-of-line or EOF.  Anything else means
     * the input contained tokens that aren't valid in a #if expression
     * (e.g. `A = B`, `A++`, `A.B` — assignment, postfix, member access
     * are not allowed per C99 §6.10.1).  mcpp's e_14_2 exercises this. */
    ec_skip_ws(&ec);
    if (ec.cur && ec.cur->tok.kind != CPPT_NEWLINE && ec.cur->tok.kind != CPPT_EOF) {
        char msg[160];
        snprintf(msg, sizeof msg,
                 "operator '%s' is not valid in #if expression",
                 pptok_spell(&ec.cur->tok));
        ec_error(&ec, ec.cur->tok.loc, msg);
    }
    if (error_out) *error_out = ec.had_error;
    return v.v;
}
