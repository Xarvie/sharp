/*
 * lex.h — Sharp Frontend: Phase 1 Second-Stage Lexer.
 *
 * Consumes the pp-token stream produced by sharp-cpp (CppTok[]) and emits
 * a SharpTok[] array with:
 *   - C11 + Sharp keyword promotion (CPPT_IDENT → STOK_KW_*)
 *   - Punctuator subdivision (CPPT_PUNCT → STOK_LBRACE, STOK_PLUS, …)
 *   - Literal value parsing (CPPT_PP_NUMBER → STOK_INT_LIT / STOK_FLOAT_LIT)
 *   - Character constant value extraction (CPPT_CHAR_CONST → STOK_CHAR_LIT)
 *   - String literal pass-through (CPPT_STRING_LIT → STOK_STRING_LIT)
 *   - AT_INTRINSIC pass-through (CPPT_AT_INTRINSIC → STOK_AT_INTRINSIC)
 *
 * Whitespace, newlines, and comments are skipped (cpp emits none by default;
 * if keep_whitespace was enabled by the caller, lex drops them here).
 *
 * Ownership
 * ---------
 *   SharpTok.text borrows a pointer into the CppResult's raw text buffer.
 *   The CppResult must outlive the SharpTok array.
 *   Free the array with lex_free(); do NOT free SharpTok.text fields.
 *
 * Phase 0 note (ISSUE P1.8)
 * --------------------------
 *   The '>>' right-shift token in nested generics (Vec<Vec<int>>) is handled
 *   by parse.c (Phase 3), not here.  lex emits STOK_GTGT for '>>' verbatim.
 */
#ifndef SHARP_FE_LEX_H
#define SHARP_FE_LEX_H

#include "sharp.h"          /* CppTok, CppLoc via cpp.h          */
#include "sharp_internal.h" /* FeDiagArr                         */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Token kinds
 * ---------------------------------------------------------------------- */

typedef enum {
    STOK_EOF = 0,

    /* ── C11 keywords ──────────────────────────────────────────────────── */
    STOK_AUTO, STOK_BREAK, STOK_CASE, STOK_CHAR, STOK_CONST,
    STOK_CONTINUE, STOK_DEFAULT, STOK_DO, STOK_DOUBLE, STOK_ELSE,
    STOK_ENUM, STOK_EXTERN, STOK_FLOAT, STOK_FOR, STOK_GOTO,
    STOK_IF, STOK_INLINE, STOK_INT, STOK_LONG, STOK_REGISTER,
    STOK_RESTRICT, STOK_RETURN, STOK_SHORT, STOK_SIGNED, STOK_SIZEOF,
    STOK_STATIC, STOK_STRUCT, STOK_SWITCH, STOK_TYPEDEF, STOK_UNION,
    STOK_UNSIGNED, STOK_VOID, STOK_VOLATILE, STOK_WHILE,
    /* C11 underscore keywords */
    STOK__ALIGNAS, STOK__ALIGNOF, STOK__ATOMIC, STOK__BOOL,
    STOK__COMPLEX, STOK__GENERIC, STOK__IMAGINARY, STOK__NORETURN,
    STOK__STATIC_ASSERT, STOK__THREAD_LOCAL,

    /* ── Sharp keywords ─────────────────────────────────────────────────── */
    STOK_DEFER,      /* 'defer'    — Sharp scope-exit statement            */
    STOK_OPERATOR,   /* 'operator' — Sharp operator-overload introducer    */
    STOK_NULL,       /* 'null'     — Sharp null pointer constant           */
    STOK_THIS,       /* 'this'     — Sharp implicit receiver               */
    STOK_IMPORT,     /* 'import'   — Sharp module import (spec §模块系统)
                      * 'import "path.sp";' is handled at the parse layer;
                      * lex promotes the identifier and leaves the string  */
    STOK_EXTERN_STRUCT, /* parse-layer sentinel, not produced by lex       */

    /* ── GCC extension keywords ────────────────────────────────────────── */
    STOK_ATTRIBUTE,  /* '__attribute__' or '__attribute' — GCC attribute
                      * specifier introducer.  Always followed by `((…))`.
                      * Sharp parses and silently discards these — they're
                      * decorative on every syntax position they appear at
                      * (visibility, alignment, packing, deprecation, …);
                      * the C compiler revisits them after sharp emits its
                      * cleaned C output.  Lex promotes; eat_attributes()
                      * in parse.c consumes the token plus its `((…))`
                      * payload at every grammar position they may show
                      * up in.                                              */
    STOK_ASM,        /* '__asm__' / '__asm' / 'asm' — GCC asm-name override
                      * (declarator suffix), e.g.
                      *   extern int foo(int) __asm__("real_name");
                      * Sharp parses and silently discards: the symbol-rename
                      * has no surface effect on Sharp semantics; the C
                      * compiler reprocesses it on its second pass.        */

    /* ── Identifiers ─────────────────────────────────────────────────────── */
    STOK_IDENT,

    /* ── Literals ────────────────────────────────────────────────────────── */
    STOK_INT_LIT,    /* integer constant; value in .u.ival / .u.uval       */
    STOK_FLOAT_LIT,  /* floating-point constant; value in .u.fval          */
    STOK_CHAR_LIT,   /* character constant; code-point in .u.ival          */
    STOK_STRING_LIT, /* string literal; text/len are the full token spell  */

    /* ── Punctuators ─────────────────────────────────────────────────────── */
    STOK_LBRACE,     /* {   */   STOK_RBRACE,     /* }   */
    STOK_LBRACKET,   /* [   */   STOK_RBRACKET,   /* ]   */
    STOK_LPAREN,     /* (   */   STOK_RPAREN,     /* )   */
    STOK_SEMI,       /* ;   */   STOK_COLON,      /* :   */
    STOK_COMMA,      /* ,   */   STOK_QUESTION,   /* ?   */
    STOK_TILDE,      /* ~   */   STOK_HASH,       /* #   */
    STOK_HASHHASH,   /* ## */   STOK_ELLIPSIS,   /* ... */
    STOK_DOT,        /* .   */   STOK_ARROW,      /* ->  */

    STOK_PLUS,       /* +   */   STOK_PLUSPLUS,   /* ++  */   STOK_PLUSEQ,   /* += */
    STOK_MINUS,      /* -   */   STOK_MINUSMINUS, /* --  */   STOK_MINUSEQ,  /* -= */
    STOK_STAR,       /* *   */                                STOK_STAREQ,   /* *= */
    STOK_SLASH,      /* /   */                                STOK_SLASHEQ,  /* /= */
    STOK_PERCENT,    /* %   */                                STOK_PERCENTEQ,/* %= */
    STOK_AMP,        /* &   */   STOK_AMPAMP,     /* && */   STOK_AMPEQ,    /* &= */
    STOK_PIPE,       /* |   */   STOK_PIPEPIPE,   /* || */   STOK_PIPEEQ,   /* |= */
    STOK_CARET,      /* ^   */                                STOK_CARETEQ,  /* ^= */
    STOK_BANG,       /* !   */   STOK_BANGEQ,     /* != */
    STOK_EQ,         /* =   */   STOK_EQEQ,       /* == */
    STOK_LT,         /* <   */   STOK_LTLT,       /* << */   STOK_LTEQ,     /* <= */   STOK_LTLTEQ, /* <<= */
    STOK_GT,         /* >   */   STOK_GTGT,       /* >> */   STOK_GTEQ,     /* >= */   STOK_GTGTEQ, /* >>= */

    /* ── Sharp-specific ─────────────────────────────────────────────────── */
    STOK_AT_INTRINSIC,  /* @ident — Sharp compile-time intrinsic          */

    /* ── Other / unknown ─────────────────────────────────────────────────── */
    STOK_OTHER,

    STOK_COUNT
} SharpTokKind;

/* -------------------------------------------------------------------------
 * Token
 * ---------------------------------------------------------------------- */

typedef struct {
    SharpTokKind kind;

    /* Spelling: borrowed from CppResult._raw_text.  Not NUL-terminated.
     * Use text[0..len-1].                                                  */
    const char *text;
    size_t      len;

    /* Source location (inherited from CppTok.loc).                         */
    CppLoc loc;

    /* Parsed literal value — valid only for INT_LIT, FLOAT_LIT, CHAR_LIT. */
    union {
        int64_t  ival;  /* signed integer / signed char code point          */
        uint64_t uval;  /* unsigned integer / unsigned char code point       */
        double   fval;  /* floating-point value                             */
    } u;

    /* For INT_LIT / CHAR_LIT: true if the value is unsigned (U suffix,
     * or high-bit forced promotion per C11 §6.4.4.1/5).                   */
    bool is_unsigned;
    bool is_long;      /* L suffix on integer literal */
    bool is_longlong;  /* LL suffix on integer literal */

    /* Inherited from CppTok.from_macro.                                     */
    bool from_macro;
} SharpTok;

/* -------------------------------------------------------------------------
 * Entry point
 * ---------------------------------------------------------------------- */

/**
 * Run the second-stage lexer over a CppTok stream.
 *
 * tokens[0..ntokens-1] must remain valid (and the CppResult that owns them
 * must remain alive) until lex_free() is called on the returned array.
 *
 * Returns a heap-allocated SharpTok array terminated by a STOK_EOF sentinel.
 * Writes the count of non-EOF tokens to *out_count.
 *
 * Phase R1: the `sharp_keywords` flag controls whether Sharp-specific
 * keywords (`defer`, `import`, `null`, `operator`, `this`) are promoted
 * to their `STOK_*` token kinds.  Pass `true` for `.sp` source and
 * `false` for any C input — cJSON's `cJSON *null = ...` is the
 * canonical example of why C code cannot reserve `null`.  C11 keywords
 * are always promoted regardless of the flag.
 *
 * Diagnostics (malformed literals, etc.) are pushed to *diags.
 *
 * Never returns NULL; aborts on OOM.
 */
SharpTok *lex_run(const CppTok *tokens, size_t ntokens,
                  const char *filename, FeDiagArr *diags,
                  size_t *out_count, bool sharp_keywords);

/**
 * Free a SharpTok array returned by lex_run().
 * Does NOT free SharpTok.text (that is borrowed from CppResult).
 */
void lex_free(SharpTok *toks);

/** Return a human-readable name for a SharpTokKind. */
const char *lex_tok_kind_name(SharpTokKind k);

/** Return true if kind is a C or Sharp keyword (STOK_AUTO .. STOK_IMPORT). */
bool lex_is_keyword(SharpTokKind k);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_LEX_H */
