/*
 * macro.h — Macro table and expansion API.
 */
#ifndef SHARP_CPP_MACRO_H
#define SHARP_CPP_MACRO_H

#include "cpp_internal.h"

/* Opaque macro definition (also used by directive.c for redefinition checks) */
typedef struct MacroDef MacroDef;

/** Create an empty macro table. */
MacroTable *macro_table_new(InternTable *interns);

/** Free the macro table and all definitions it contains. */
void macro_table_free(MacroTable *t);

/** Look up a macro by name.  Returns NULL if undefined. */
MacroDef *macro_lookup(MacroTable *t, const char *name);

/** Return true if the named macro exists and is function-like. */
bool macro_lookup_is_func(MacroTable *t, const char *name);

/** Install or replace a macro definition. */
void macro_define(MacroTable *t, MacroDef *def, CppDiagArr *diags, CppLoc loc);

/** Remove a macro definition (no-op if not defined). */
void macro_undef(MacroTable *t, const char *name);

/**
 * Parse a macro definition from the token list following "#define".
 * Returns a heap-allocated MacroDef, or NULL on parse error.
 */
MacroDef *macro_parse_define(const TokList *line_toks,
                              InternTable *interns,
                              CppDiagArr *diags);

/**
 * Expand a token list, performing macro substitution.
 *
 * `input`  — flat token list to expand (consumed but not freed).
 * `output` — receives the expanded tokens (appended).
 */
void macro_expand(TokList *input, MacroTable *mt,
                  InternTable *interns, CppDiagArr *diags,
                  TokList *output);

/**
 * Define an object-like macro from a raw body string.
 * Convenience wrapper used by the directive layer for built-in macros.
 * `body_str` may be NULL (empty body) or any token sequence.
 */
void macro_define_object(MacroTable *t, InternTable *interns,
                         const char *name, const char *body_str,
                         CppLoc loc);

/**
 * Return true if macro expansion limits have been breached.
 * When true, callers should skip all further macro expansion.
 */
bool macro_limits_breached(void);

#endif /* SHARP_CPP_MACRO_H */
