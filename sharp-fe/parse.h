/*
 * parse.h — Sharp Frontend: Phase 3 Recursive-Descent Parser.
 *
 * Consumes a SharpTok[] stream (from lex.h) and produces an AstNode* tree
 * rooted at AST_FILE.  No semantic analysis is performed here — the parser
 * only checks syntax and builds the tree structure.
 *
 * Key design decisions (Phase 0 DECISION probes + Phase 3 addendum):
 *
 *   D-AUTO   : 'auto x = expr;' parsed as AST_VAR_DECL with type=AST_TYPE_AUTO.
 *              Missing initialiser is a parse-phase error.
 *   D-GTGT   : '>>' in nested generics (Vec<Vec<int>>) is split into two '>'
 *              inside parse_generic_args() via a pending_close counter.
 *   D-OPERATOR: 'operator+' parsed as FUNC_DEF with is_operator=true.
 *   D-DECL-vs-EXPR: in statement position, starts with type-start tokens
 *              (C keywords / const / auto / IDENT followed by IDENT or *).
 */
#ifndef SHARP_FE_PARSE_H
#define SHARP_FE_PARSE_H

#include "ast.h"           /* AstNode, AstKind, FmtFlags */
#include "sharp_internal.h"/* FeDiagArr        */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse a complete .sp compilation unit from a lex token stream.
 *
 * tokens[0..ntokens] must contain a STOK_EOF sentinel at [ntokens].
 * The CppResult that backs SharpTok.text must outlive the returned AST.
 *
 * Returns an AST_FILE node (never NULL — error recovery keeps going).
 * All diagnostics are pushed to *diags.
 */
/** Append a unique #include string to file->u.file.user_includes. */
void file_add_include(AstNode *file, const char *inc_str);

AstNode *parse_file(const SharpTok *tokens, size_t ntokens,
                    const char *filename, FeDiagArr *diags);

/**
 * Parse with pre-populated typedef names (pass extra_typedefs=NULL if none).
 * Used internally; external callers use parse_file().
 */
AstNode *parse_file_with_typedefs(const SharpTok *tokens, size_t ntokens,
                                  const char *filename, FeDiagArr *diags,
                                  const char **extra_typedefs);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_PARSE_H */
