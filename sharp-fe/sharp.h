/*
 * sharp.h — Sharp Language Frontend Public API
 *
 * The Sharp frontend consumes the pp-token stream produced by sharp-cpp
 * and emits a C11 source file.  Together, sharp-cpp + sharp-fe form the
 * complete sharpc compiler:
 *
 *   .sp source  -->  cpp_run()  -->  CppTok[]  -->  sharp_compile_tokens()
 *                -->  SharpResult.c_text  -->  cc  -->  binary
 *
 * Usage (minimal)
 * ---------------
 *   SharpCtx *ctx = sharp_ctx_new();
 *   SharpResult res = sharp_compile_file(ctx, "input.sp");
 *   if (res.error) { fprintf(stderr, "%s\n", res.error); }
 *   else           { write res.c_text to a .c file and invoke cc }
 *   sharp_result_free(&res);
 *   sharp_ctx_free(ctx);
 *
 * Thread safety
 * -------------
 *   A SharpCtx must not be shared across threads.  The output SharpResult
 *   is immutable once returned and may be passed freely.
 *
 * Relationship with sharp-cpp
 * ---------------------------
 *   sharp.h reuses CppTok, CppLoc, CppDiag, and CppDiagLevel from cpp.h.
 *   These types are identical; no conversion is needed between the two
 *   layers.
 *
 * Phase 0 note
 * ------------
 *   This header is a skeleton.  Types and functions are added as each
 *   phase is implemented (Phase 1 adds lex types, Phase 3 adds parse
 *   entry points, etc.).  All additions must be backward-compatible.
 */
#ifndef SHARP_FE_H
#define SHARP_FE_H

#include "cpp.h"    /* CppTok, CppLoc, CppDiag, CppDiagLevel */

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Compilation result
 * ---------------------------------------------------------------------- */

typedef struct {
    /* Generated C11 source text (heap-allocated, NUL-terminated).
     * Valid only when error == NULL.                                        */
    char   *c_text;
    size_t  c_text_len;

    /* Diagnostics emitted by the front-end.
     * Uses the same CppDiag / CppLoc types as sharp-cpp.
     * Freed by sharp_result_free().                                        */
    CppDiag *diags;
    size_t   ndiags;

    /* Non-NULL on fatal error (also appears in diags).                     */
    const char *error;
} SharpResult;

/* -------------------------------------------------------------------------
 * Context — opaque
 * ---------------------------------------------------------------------- */

typedef struct SharpCtx SharpCtx;

/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

/** Allocate a new frontend context with default settings. */
SharpCtx *sharp_ctx_new(void);

/** Free a context and all memory it owns. */
void      sharp_ctx_free(SharpCtx *ctx);

/* -------------------------------------------------------------------------
 * Configuration (call before sharp_compile_*)
 * ---------------------------------------------------------------------- */

/**
 * Set the target triple used for platform-specific type sizes.
 * Recognised triples match those accepted by cpp_install_target_macros().
 * Default: "x86_64-linux-gnu".
 */
void sharp_set_target(SharpCtx *ctx, const char *triple);

/* -------------------------------------------------------------------------
 * Compilation entry points
 * ---------------------------------------------------------------------- */

/**
 * Compile a Sharp source file end-to-end (runs cpp internally, then
 * the front-end on its output).
 */
SharpResult sharp_compile_file(SharpCtx *ctx, const char *path);

/**
 * Compile from a pre-tokenised stream produced by cpp_run() / cpp_run_buf().
 * 'filename' is used for diagnostics and #line directives.
 *
 * The caller retains ownership of the tokens array; the frontend does not
 * free it.
 */
SharpResult sharp_compile_tokens(SharpCtx *ctx,
                                 const CppTok *tokens, size_t ntokens,
                                 const char *filename);

/* -------------------------------------------------------------------------
 * Result helpers
 * ---------------------------------------------------------------------- */

/** Free all heap memory inside a SharpResult (does NOT free the struct). */
void sharp_result_free(SharpResult *res);

/** Print all diagnostics to stderr in a clang-style format. */
void sharp_print_diags(const SharpResult *res);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_H */
