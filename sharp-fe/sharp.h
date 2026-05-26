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

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_H */
