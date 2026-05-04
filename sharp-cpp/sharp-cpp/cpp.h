/*
 * cpp.h — Sharp/C Preprocessor Public API
 *
 * A self-contained C11 preprocessor that handles both plain C and the Sharp
 * language extension set.  It implements all six of the ISO C11 translation
 * phases that precede compilation proper (phases 1–6), producing a flat
 * token stream annotated with source locations.
 *
 * Usage
 * -----
 *   CppCtx *ctx = cpp_ctx_new();
 *   cpp_add_include_path(ctx, "/usr/include");
 *   cpp_define(ctx, "NDEBUG", "1");
 *   CppResult res = cpp_run(ctx, "input.sp", CPP_LANG_SHARP);
 *   if (res.error) { fprintf(stderr, "%s\n", res.error); }
 *   else           { use res.tokens and res.text }
 *   cpp_result_free(&res);
 *   cpp_ctx_free(ctx);
 *
 * Thread safety
 * -------------
 *   A CppCtx must not be shared across threads.  Each thread should own its
 *   own context.  The output CppResult is immutable and can be passed freely.
 */
#ifndef SHARP_CPP_H
#define SHARP_CPP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Language mode
 * ---------------------------------------------------------------------- */

typedef enum {
    CPP_LANG_C     = 0,   /* Pure C11 — no Sharp extensions              */
    CPP_LANG_SHARP = 1,   /* Sharp source (.sp) — superset of C mode     */
} CppLang;

/* -------------------------------------------------------------------------
 * Source location
 * ---------------------------------------------------------------------- */

typedef struct {
    const char *file;   /* interned filename string (owned by CppCtx)    */
    int         line;   /* 1-based line number                            */
    int         col;    /* 1-based column number                          */
} CppLoc;

/* -------------------------------------------------------------------------
 * Preprocessing token kinds
 * ---------------------------------------------------------------------- */

typedef enum {
    /* --- End-of-file sentinel --- */
    CPPT_EOF = 0,

    /* --- Whitespace / newline (kept so consumers can reconstruct text) --- */
    CPPT_SPACE,          /* horizontal whitespace run                     */
    CPPT_NEWLINE,        /* logical newline (after line-splicing)          */

    /* --- Comments (stripped by default, emitted in -C mode) --- */
    CPPT_COMMENT,

    /* --- Core pp-token categories (ISO §6.4) --- */
    CPPT_HEADER_NAME,    /* <foo.h> or "foo.h" in #include context        */
    CPPT_IDENT,          /* identifier or keyword (not yet classified)     */
    CPPT_PP_NUMBER,      /* preprocessing number (superset of int/float)  */
    CPPT_CHAR_CONST,     /* 'x', L'x', u'x', U'x', u8'x'                 */
    CPPT_STRING_LIT,     /* "s", L"s", u"s", U"s", u8"s"                 */
    CPPT_PUNCT,          /* any punctuator / operator                      */
    CPPT_OTHER,          /* any other single character (§6.4.9)           */

    /* --- Synthetic tokens produced during macro expansion --- */
    CPPT_PLACEMARKER,    /* empty argument placeholder (§6.10.3.4)        */

    /* --- Sharp-language extension --- */
    CPPT_AT_INTRINSIC,   /* @ident  — Sharp compile-time intrinsic token.
                          * Produced by the tokenizer in Sharp mode when '@'
                          * is followed by an identifier start character.
                          * Spell contains the full "@ident" string, e.g.
                          * "@has_destructor".  Never macro-expanded;
                          * passed through to the Sharp frontend verbatim. */

    CPPT_COUNT
} CppTokKind;

/* -------------------------------------------------------------------------
 * Preprocessing token
 * ---------------------------------------------------------------------- */

typedef struct {
    CppTokKind  kind;

    /* Spelling: points into the output text buffer (not NUL-terminated). */
    const char *text;
    size_t      len;

    /* Source origin of this token before expansion.                       */
    CppLoc      loc;

    /* For CPPT_IDENT: true if this token was produced by macro expansion.  */
    bool        from_macro;

    /* For string literals: the concatenation has already been performed.  */
    bool        concat_done;
} CppTok;

/* -------------------------------------------------------------------------
 * Diagnostic severity
 * ---------------------------------------------------------------------- */

typedef enum {
    CPP_DIAG_NOTE,
    CPP_DIAG_WARNING,
    CPP_DIAG_ERROR,
    CPP_DIAG_FATAL,
} CppDiagLevel;

typedef struct {
    CppDiagLevel  level;
    CppLoc        loc;
    char         *msg;   /* heap-allocated, freed by cpp_result_free()    */
} CppDiag;

/* -------------------------------------------------------------------------
 * Preprocessor result
 * ---------------------------------------------------------------------- */

typedef struct {
    /* Flat token array (heap-allocated).  Count does NOT include trailing
     * CPPT_EOF sentinel.  token.text pointers are valid until cpp_result_free(). */
    CppTok  *tokens;
    size_t   ntokens;

    /* Full preprocessed text (heap-allocated, NUL-terminated).
     * Each logical line is annotated with # <line> <"file"> markers
     * when linemarkers != 0.                                              */
    char    *text;
    size_t   text_len;

    /* Diagnostics (heap-allocated array).                                 */
    CppDiag *diags;
    size_t   ndiags;

    /* Non-NULL on fatal error: brief description (also appears in diags). */
    const char *error;

    /* Raw pre-phase-6 text buffer.  CppTok.text pointers point into here.
     * Kept alive until cpp_result_free() so that token text is accessible. */
    char    *_raw_text;
} CppResult;

/* -------------------------------------------------------------------------
 * Context — opaque
 * ---------------------------------------------------------------------- */

typedef struct CppCtx CppCtx;

/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

/** Allocate a new preprocessor context with default settings. */
CppCtx *cpp_ctx_new(void);

/** Free a context and all memory it owns. */
void    cpp_ctx_free(CppCtx *ctx);

/* -------------------------------------------------------------------------
 * Configuration (call before cpp_run)
 * ---------------------------------------------------------------------- */

/**
 * Add a directory to the system #include <...> search path.
 * Paths are searched in the order they are added.
 */
void cpp_add_sys_include(CppCtx *ctx, const char *dir);

/**
 * Add a directory to the user #include "..." search path.
 * User paths are searched before system paths for "..." includes.
 */
void cpp_add_user_include(CppCtx *ctx, const char *dir);

/**
 * Pre-define a macro as if `-DNAME=value` were given on the command line.
 * Pass value=NULL to define NAME as the empty string (equivalent to -DNAME).
 */
void cpp_define(CppCtx *ctx, const char *name, const char *value);
/* Phase R15: define a function-like macro.  name_with_params includes
 * the parameter list, e.g. cpp_define_func(ctx, "__INT64_C(c)", "c##L") */
void cpp_define_func(CppCtx *ctx, const char *name_with_params, const char *body);

/**
 * Pre-undefine a macro as if `-UNAME` were given.
 * Useful to suppress compiler-default built-in macros.
 */
void cpp_undefine(CppCtx *ctx, const char *name);

/* Phase 7: target-triple macro injection helper.
 *
 * Predefine the set of macros that a real toolchain would inject for the
 * given target triple, so that system headers (limits.h, stdint.h,
 * sys/types.h, ...) take the right code path.  This is purely an
 * ergonomic shortcut for what would otherwise be twenty-plus
 * cpp_define() calls; it does not change any preprocessor behaviour.
 *
 * Recognised triples:
 *
 *   "x86_64-linux-gnu"          Linux x86_64, glibc, ELF, GCC ABI
 *   "aarch64-apple-darwin"      macOS arm64 (Apple Silicon), Mach-O, clang
 *   "x86_64-pc-windows-msvc"    Windows x64, COFF/PE, MSVC ABI (LLP64)
 *
 * Returns 0 on success.  Returns -1 (without modifying the context) if
 * the triple is not recognised, or if either argument is NULL.  Callers
 * may then fall back to handcrafted cpp_define() calls or fail the build
 * with a clear "unsupported target" message.
 *
 * This helper does NOT enable feature-test macros like _GNU_SOURCE,
 * _POSIX_C_SOURCE or _XOPEN_SOURCE.  Those encode caller intent and
 * remain the caller's responsibility.
 */
int cpp_install_target_macros(CppCtx *ctx, const char *triple);

/* Phase 7: enumerate the triples that cpp_install_target_macros accepts.
 *
 * Returns a NULL-terminated array of C strings.  The array is owned by
 * the library; do not free its elements.  Useful for command-line tools
 * that want to print a list of supported targets.
 */
const char *const *cpp_target_known_triples(void);

/**
 * Control whether whitespace tokens are emitted in the output token array.
 * Default: false (whitespace is suppressed).
 */
void cpp_keep_whitespace(CppCtx *ctx, bool keep);

/**
 * Control whether line-marker directives (# N "file") are emitted in text.
 * Default: true.
 */
void cpp_emit_linemarkers(CppCtx *ctx, bool emit);
void cpp_set_lang_std(CppCtx *ctx, long std);

/**
 * Control whether C99 §5.1.1.2 phase 6 (adjacent string literal
 * concatenation) is applied to the preprocessed text.  Default: false —
 * matches `gcc -E` and `clang -E` output, where adjacent string literals
 * stay as separate tokens for the compiler frontend.  Set to true if
 * sharp-cpp is being used as the *only* translation step before code
 * generation (e.g. an embedded scripting language eating preprocessed C).
 */
void cpp_set_phase6_concat(CppCtx *ctx, bool on);

/**
 * Control whether C99 §5.1.1.2 phase 1 trigraph translation is
 * performed.  Default: false — matches `gcc -E` (which only warns about
 * trigraphs unless `-trigraphs` is given).  Set to true for strict ISO C
 * conformance or to preprocess legacy code that genuinely uses trigraphs.
 */
void cpp_set_trigraphs(CppCtx *ctx, bool on);

/**
 * Control whether comments are preserved (pass-through to output text).
 * Default: false.
 */
void cpp_keep_comments(CppCtx *ctx, bool keep);

/**
 * Set maximum #include nesting depth (default: 200).
 */
void cpp_set_max_include_depth(CppCtx *ctx, int depth);

/**
 * Register a callback invoked for each #pragma line that the preprocessor
 * does not handle internally.  Return true to suppress emission, false to
 * pass the pragma through to the token stream.
 */
typedef bool (*CppPragmaHandler)(CppCtx *ctx, const char *pragma_text,
                                 CppLoc loc, void *userdata);
void cpp_set_pragma_handler(CppCtx *ctx, CppPragmaHandler fn, void *userdata);

/* -------------------------------------------------------------------------
 * Execution
 * ---------------------------------------------------------------------- */

/**
 * Run the preprocessor on `filename`.  The file is read from disk.
 * `lang` selects language-specific built-in macros.
 *
 * Returns a CppResult that must be freed with cpp_result_free().
 */
CppResult cpp_run(CppCtx *ctx, const char *filename, CppLang lang);

/**
 * Run the preprocessor on an in-memory buffer of `len` bytes.
 * `filename` is used for diagnostics and __FILE__.
 */
CppResult cpp_run_buf(CppCtx *ctx, const char *buf, size_t len,
                      const char *filename, CppLang lang);

/* -------------------------------------------------------------------------
 * Result helpers
 * ---------------------------------------------------------------------- */

/** Free all heap memory inside a CppResult (does NOT free the struct itself). */
void cpp_result_free(CppResult *res);

/** Return a human-readable name for a token kind. */
const char *cpp_tok_kind_name(CppTokKind kind);

/** Print all diagnostics to stderr. */
void cpp_print_diags(const CppResult *res);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_CPP_H */
