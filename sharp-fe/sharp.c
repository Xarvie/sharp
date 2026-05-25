/*
 * sharp.c — Sharp Frontend Public API implementation.
 *
 * NOTE: The standalone compiler driver (sharpc.c) directly orchestrates
 * cpp→lex→parse→scope→sema→cg.  The SharpCtx / SharpResult public API
 * below is a library facade for external consumers; the full compilation
 * pipeline lives in sharpc.c, not here.
 */

#include "sharp_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Lifecycle
 * ---------------------------------------------------------------------- */

SharpCtx *sharp_ctx_new(void) {
    SharpCtx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) { perror("sharp-fe"); abort(); }
    /* Default target triple. */
    {
        const char *s = "x86_64-linux-gnu";
        size_t len = strlen(s) + 1;
        ctx->target = malloc(len);
        if (ctx->target) memcpy(ctx->target, s, len);
    }
    return ctx;
}

void sharp_ctx_free(SharpCtx *ctx) {
    if (!ctx) return;
    free(ctx->target);
    free(ctx);
}

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */

void sharp_set_target(SharpCtx *ctx, const char *triple) {
    if (!ctx || !triple) return;
    free(ctx->target);
    {
        size_t len = strlen(triple) + 1;
        ctx->target = malloc(len);
        if (ctx->target) memcpy(ctx->target, triple, len);
    }
}

/* -------------------------------------------------------------------------
 * Result helpers
 * ---------------------------------------------------------------------- */

void sharp_result_free(SharpResult *res) {
    if (!res) return;
    free(res->c_text);
    for (size_t i = 0; i < res->ndiags; i++) free(res->diags[i].msg);
    free(res->diags);
    *res = (SharpResult){0};
}

void sharp_print_diags(const SharpResult *res) {
    if (!res) return;
    for (size_t i = 0; i < res->ndiags; i++) {
        CppDiag d = res->diags[i];
        const char *level =
            d.level == CPP_DIAG_FATAL   ? "fatal"   :
            d.level == CPP_DIAG_ERROR   ? "error"   :
            d.level == CPP_DIAG_WARNING ? "warning" : "note";
        fprintf(stderr, "%s:%d:%d: %s: %s\n",
                d.loc.file ? d.loc.file : "?",
                d.loc.line, d.loc.col, level, d.msg);
    }
}

/* -------------------------------------------------------------------------
 * Library entry points (stubs)
 *
 * The full pipeline is implemented in sharpc.c (standalone driver).
 * These library functions are retained for API compatibility but return
 * an error — use the sharpc binary instead.
 * ---------------------------------------------------------------------- */

SharpResult sharp_compile_tokens(SharpCtx *ctx,
                                 const CppTok *tokens, size_t ntokens,
                                 const char *filename) {
    (void)ctx; (void)tokens; (void)ntokens; (void)filename;
    /* Library API unavailable — use the sharpc binary instead. */
    SharpResult res = {0};
    res.error = "sharp-fe: library API unavailable — use the sharpc binary instead";
    return res;
}

SharpResult sharp_compile_file(SharpCtx *ctx, const char *path) {
    (void)ctx; (void)path;
    SharpResult res = {0};
    res.error = "sharp-fe: library API unavailable — use the sharpc binary instead";
    return res;
}
