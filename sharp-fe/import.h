/*
 * import.h — Sharp Frontend: import resolution.
 *
 * Sharp's `import "path.sp"` is processed AFTER parsing (at the AST level),
 * NOT by the CPP.  When the parser encounters `import "foo.sp"`, it emits an
 * AST_IMPORT node.  This module walks a parsed file's AST, finds all
 * AST_IMPORT nodes, reads those files, parses them recursively, and merges
 * their top-level declarations into the caller's AST.
 *
 * Design decisions (Probe §import):
 *   - Same file imported multiple times is a no-op after the first.
 *   - Import cycles are detected and reported as errors.
 *   - Macros defined in the CPP (#define) do NOT leak through import — each
 *     imported file starts with a fresh CppCtx (inheriting only system
 *     include paths, not cmdline defines).
 *   - The imported file's top-level declarations are prepended to the
 *     importer's decls list in order (allowing forward references).
 *   - Import paths are resolved relative to the importing file's directory,
 *     then relative to each entry in the search path.
 *
 * API:
 *   ImportCtx *import_ctx_new(void);
 *   void       import_ctx_free(ImportCtx *ctx);
 *   void       import_ctx_add_search_path(ImportCtx *ctx, const char *dir);
 *
 *   // Resolve all AST_IMPORT nodes in `file`, merging imported decls in-place.
 *   // `file_path` is the path of the file that `file` was parsed from
 *   //   (used for relative-path resolution and cycle detection).
 *   // Errors are pushed to `diags`.
 *   void import_resolve(ImportCtx *ctx, AstNode *file,
 *                       const char *file_path, FeDiagArr *diags);
 */
#ifndef SHARP_FE_IMPORT_H
#define SHARP_FE_IMPORT_H

#include "parse.h"   /* AstNode, FeDiagArr */
#include "sharp_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ImportCtx ImportCtx;

ImportCtx *import_ctx_new(void);
void       import_ctx_free(ImportCtx *ctx);
void       import_ctx_add_search_path(ImportCtx *ctx, const char *dir);

/**
 * Resolve all import "..." declarations in the AST in-place.
 *
 * For each AST_IMPORT node found:
 *   1. Resolve the path (relative to file_path's directory, then search paths).
 *   2. Parse the target .sp file (recursively resolving its imports).
 *   3. Prepend its top-level decls into the current file's decls list.
 *   4. Replace the AST_IMPORT node with the merged decls.
 *
 * The function is idempotent: importing the same (canonical) path twice is
 * a no-op; the second import is silently removed.
 */
void import_resolve(ImportCtx *ctx, AstNode *file,
                    const char *file_path, FeDiagArr *diags);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_IMPORT_H */
