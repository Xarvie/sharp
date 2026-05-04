/*
 * cg.h — Sharp Frontend: Phase 8 Code Generation.
 *
 * Translates a sema-checked AST_FILE into a C11 source string.
 * The output is suitable for passing to gcc/clang.
 *
 * Translation rules (spec §代码生成):
 *   - struct methods → StructName__methodName(StructName* this, params)
 *   - const methods  → StructName__methodName(const StructName* this, ...)
 *   - operators      → StructName__op_<suffix>(StructName* this, rhs)
 *     where suffix: add sub mul div mod eq ne lt gt le ge shl shr band bor bxor idx
 *   - struct literals → compound literals  (Type){ .field = val, … }
 *   - null           → NULL
 *   - defer          → inline cleanup blocks (emitted at every block exit, LIFO)
 *   - Generic structs/funcs → TODO comment (Phase 11 handles monomorphization)
 *
 * Phase 8 does NOT generate:
 *   - Generic instantiations (Phase 11)
 *   - import resolution output (handled before cg)
 *   - @intrinsic evaluation (Phase 10)
 */
#ifndef SHARP_FE_CG_H
#define SHARP_FE_CG_H

#include "sema.h"   /* pulls in type.h, scope.h, ast.h */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CgCtx CgCtx;

/**
 * Create a code-generation context.
 * ts and file_scope must outlive the context.
 */
CgCtx *cg_ctx_new(TyStore *ts, Scope *file_scope);
void   cg_ctx_free(CgCtx *ctx);

/**
 * Generate C11 source code from a sema-checked AST_FILE node.
 *
 * Returns a heap-allocated NUL-terminated C string.
 * The caller must free() it.
 * Never returns NULL (returns at minimum an empty string).
 */
char *cg_generate(CgCtx *ctx, const AstNode *file);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_CG_H */
