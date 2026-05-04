/*
 * sema.h — Sharp Frontend: Phase 6 Semantic Analysis (base).
 *
 * Phase 6 scope:
 *   - Variable declaration type resolution (incl. auto deduction)
 *   - Function return-type checking
 *   - Expression type inference: literals, arithmetic, comparison,
 *     unary, ternary, assign, cast, sizeof, subscript, call (free func)
 *   - Control-flow condition checking (if/while/for cond must be scalar)
 *   - Undefined variable / type-mismatch errors
 *
 * Phase 6 explicitly defers:
 *   - Method calls / field access / operator overload  → Phase 7
 *   - defer, @intrinsic                               → Phase 9-10
 *   - Generic monomorphisation                        → Phase 11
 *
 * AstNode.type_ref usage after Phase 6:
 *   - Declaration nodes (FUNC_DEF, STRUCT_DEF, BLOCK): Scope* (from Phase 4)
 *   - Expression nodes: Type* set by sema_check_file()
 *   - VAR_DECL: Type* (the resolved / inferred type)
 */
#ifndef SHARP_FE_SEMA_H
#define SHARP_FE_SEMA_H

#include "type.h"
#include "scope.h"
#include "ast.h"
#include "sharp_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SemaCtx SemaCtx;

/** Create a sema context bound to the given type store, file scope, and
 *  diagnostic array.  All three must outlive the context. */
SemaCtx *sema_ctx_new(TyStore *ts, Scope *file_scope, FeDiagArr *diags);
void     sema_ctx_free(SemaCtx *ctx);

/**
 * Run the Phase 6 semantic pass over a parsed file AST.
 *
 * After this call:
 *   - Every reachable expression AstNode has its type_ref set to a Type*.
 *   - Every AST_VAR_DECL (including auto) has type_ref = resolved Type*.
 *   - Errors are pushed to the diags passed to sema_ctx_new().
 *
 * Returns true if no error diagnostics were emitted.
 */
bool sema_check_file(SemaCtx *ctx, AstNode *file);

/** Get the Type* inferred for an expression node (NULL if not yet run). */
static inline Type *sema_type_of(const AstNode *n) {
    return n ? (Type *)n->type_ref : NULL;
}

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_SEMA_H */
