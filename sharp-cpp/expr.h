/*
 * expr.h — #if / #elif constant-expression evaluator.
 */
#ifndef SHARP_CPP_EXPR_H
#define SHARP_CPP_EXPR_H

#include "cpp_internal.h"
#include "macro.h"
#include <inttypes.h>

/**
 * Evaluate a preprocessing constant expression from a token list.
 *
 * The `toks` list should already have had macro expansion applied EXCEPT
 * for `defined` which is handled internally here.
 *
 * Returns the intmax_t value of the expression.
 * Sets *error_out to true if a parse/evaluation error occurred.
 */
intmax_t cpp_eval_if_expr(TokList *toks, MacroTable *mt,
                           InternTable *interns, CppDiagArr *diags,
                           bool *error_out);

#endif /* SHARP_CPP_EXPR_H */
