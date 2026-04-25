/*
 * directive.h — Directive processor API (internal).
 */
#ifndef SHARP_CPP_DIRECTIVE_H
#define SHARP_CPP_DIRECTIVE_H

#include "cpp_internal.h"

typedef struct CppState CppState;

/** Allocate and initialise a CppState from a configured CppCtx. */
CppState *cpp_state_new(CppCtx *ctx, CppDiagArr *diags);

/** Free a CppState. */
void cpp_state_free(CppState *st);

/**
 * Process a file from disk.  Built-ins and command-line macros are installed
 * before processing begins.
 */
void cpp_state_run_file(CppState *st, const char *filename, CppLang lang);

/**
 * Process an in-memory buffer.
 */
void cpp_state_run_buf(CppState *st, const char *buf, size_t len,
                        const char *filename, CppLang lang);

/** Access output text (NUL-terminated; valid until cpp_state_free). */
const char *cpp_state_text(const CppState *st);
size_t      cpp_state_text_len(const CppState *st);

/** Access output token array. */
CppTok     *cpp_state_tokens(const CppState *st);
size_t      cpp_state_ntokens(const CppState *st);

/* Helper used by expr.c's public wrapper -- parses an integer literal
 * without the full eval machinery. */
long parse_int_literal_pub(const char *sp, bool *is_unsigned);

/** Bridge: return the MacroTable owned by a CppState. */
MacroTable *macro_state_table(CppState *st);

#endif /* SHARP_CPP_DIRECTIVE_H */
