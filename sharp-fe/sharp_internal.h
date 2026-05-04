/*
 * sharp_internal.h — Internal types and helpers shared across sharp-fe modules.
 *
 * Only front-end .c files include this header; it must not be included by
 * consumer code.  Public types live in sharp.h; cpp types are included
 * transitively through sharp.h -> cpp.h.
 *
 * Convention: all internal identifiers use the prefix 'fe_' or 'sharp_'
 * to avoid clashing with cpp_internal.h helpers that may be included by
 * the same translation unit (e.g. in test harnesses that include both).
 */
#ifndef SHARP_FE_INTERNAL_H
#define SHARP_FE_INTERNAL_H

#include "sharp.h"          /* public API + CppTok/CppDiag via cpp.h     */
#include "cpp_internal.h"   /* DA(), da_push, StrBuf, CPP_LIKELY, etc.   */

#include <assert.h>
#include <stdarg.h>

/* =========================================================================
 * Diagnostic emission helpers
 * ====================================================================== */

/* fe_diag_arr is a named type so it can appear in function signatures
 * across modules without the anonymous-struct incompatibility of DA(). */
typedef struct { CppDiag *data; size_t len, cap; } FeDiagArr;

static inline void fe_diag_push(FeDiagArr *a, CppDiag d) {
    if (a->len == a->cap) {
        size_t nc = a->cap ? a->cap * 2 : 8;
        a->data = realloc(a->data, nc * sizeof *a->data);
        if (!a->data) { perror("sharp-fe"); abort(); }
        a->cap = nc;
    }
    a->data[a->len++] = d;
}

/* Emit a front-end diagnostic.  msg is heap-allocated by this function. */
static inline void fe_emit_diag(FeDiagArr *diags, CppDiagLevel level,
                                CppLoc loc, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    char *msg = (char *)malloc((size_t)(n + 1));
    if (!msg) { perror("sharp-fe"); abort(); }
    va_start(ap, fmt);
    vsnprintf(msg, (size_t)(n + 1), fmt, ap);
    va_end(ap);

    CppDiag d = { level, loc, msg };
    fe_diag_push(diags, d);
}

/* Convenience wrappers. */
#define FE_ERROR(diags, loc, ...) \
    fe_emit_diag(diags, CPP_DIAG_ERROR, loc, __VA_ARGS__)
#define FE_WARNING(diags, loc, ...) \
    fe_emit_diag(diags, CPP_DIAG_WARNING, loc, __VA_ARGS__)
#define FE_NOTE(diags, loc, ...) \
    fe_emit_diag(diags, CPP_DIAG_NOTE, loc, __VA_ARGS__)
#define FE_FATAL(diags, loc, ...) \
    fe_emit_diag(diags, CPP_DIAG_FATAL, loc, __VA_ARGS__)

/* =========================================================================
 * SharpCtx internal layout (Phase 0 skeleton)
 *
 * Fields are added as phases are implemented.  Never expose this struct
 * in sharp.h; callers use the opaque SharpCtx* pointer.
 * ====================================================================== */

struct SharpCtx {
    char *target;     /* heap-allocated target triple, default x86_64-linux-gnu */
    /* Phase 1+: lex state, intern table, etc. */
};

/* =========================================================================
 * Null-location sentinel (for internal errors without a source position)
 * ====================================================================== */
static inline CppLoc fe_noloc(void) {
    CppLoc l = { "<internal>", 0, 0 };
    return l;
}

#endif /* SHARP_FE_INTERNAL_H */
