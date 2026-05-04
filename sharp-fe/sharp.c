/*
 * sharp.c — Sharp Frontend Public API implementation (Phase 0 skeleton).
 *
 * This file contains the lifecycle functions and configuration for SharpCtx.
 * Compilation entry points (sharp_compile_file, sharp_compile_tokens) are
 * stubs that return an error until the lex/parse/sema/cg pipeline is wired
 * together in Phase 12.
 *
 * Each subsequent phase adds to this file or delegates to a dedicated
 * module (lex.c, parse.c, sema.c, mono.c, cg.c).  Every addition carries
 * a /* Phase N: ... */ comment so the audit trail is maintained.
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
    /* Phase 0: default target. */
    ctx->target = strdup("x86_64-linux-gnu");
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
    ctx->target = strdup(triple);
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
 * Compilation entry points (stubs — filled in Phase 1-12)
 * ---------------------------------------------------------------------- */

SharpResult sharp_compile_tokens(SharpCtx *ctx,
                                 const CppTok *tokens, size_t ntokens,
                                 const char *filename) {
    (void)ctx; (void)tokens; (void)ntokens; (void)filename;
    /* Phase 0: stub — pipeline not yet implemented. */
    SharpResult res = {0};
    res.error = "sharp-fe: front-end pipeline not yet implemented (Phase 0)";
    return res;
}

SharpResult sharp_compile_file(SharpCtx *ctx, const char *path) {
    (void)ctx; (void)path;
    SharpResult res = {0};
    res.error = "sharp-fe: front-end pipeline not yet implemented (Phase 0)";
    return res;
}
