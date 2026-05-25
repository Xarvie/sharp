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
 * Semantic annotations set by sema (Phase 6+):
 *   - AstNode.sem_scope (Scope*): set on FUNC_DEF, BLOCK, IF, FOR, WHILE
 *   - AstNode.sem_type  (Type*):  set on all expression nodes and VAR_DECL
 *   (These fields replace the old single `void *type_ref` which held either
 *   type depending on node kind.)
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
 *   - Every reachable expression AstNode has sem_type set to a Type*.
 *   - Every AST_VAR_DECL (including auto) has sem_type = resolved Type*.
 *   - Errors are pushed to the diags passed to sema_ctx_new().
 *
 * Returns true if no error diagnostics were emitted.
 */
bool sema_check_file(SemaCtx *ctx, AstNode *file);

/** Get the Type* inferred for an expression node (NULL if not yet run). */
static inline Type *sema_type_of(const AstNode *n) {
    return n ? n->sem_type : NULL;
}

/**
 * v0.13: Re-evaluate an @-intrinsic with an optional generic substitution
 * context.  Called by cg from inside generic-function / generic-method
 * specializations so that primitives like @has_method(K, hash) resolve
 * against the concrete instantiation type K, not against the TY_PARAM
 * placeholder used at template-definition time.
 *
 * Returns 0 / 1 for boolean intrinsics, -1 if non-evaluable.  Pass np=0
 * (and NULL arrays) for non-template contexts.
 *
 * For @type_name, returns 1 and stores the resolved string on the AST
 * node's at_intrinsic.resolved_str (overwriting any previous value).
 */
int sema_eval_intrinsic_specialized(TyStore *ts, Scope *scope, FeDiagArr *diags,
                                    AstNode *intr,
                                    const char **pnames, Type **pvals, size_t np);

/**
 * v0.13: Walk a generic function-template body and populate sem_type on
 * every expression node, ignoring the usual "skip generic" gate in
 * sema_func.  Called by cg's specialization path so the substituted-type
 * resolution machinery (cg_resolve_type) has annotations to substitute.
 *
 * Idempotent.  Use NULL diags to suppress diagnostic output; the call is
 * meant to populate types, not to validate them at this stage.
 */
void sema_func_template_body(TyStore *ts, Scope *parent_scope,
                             FeDiagArr *diags, AstNode *fn);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_SEMA_H */
