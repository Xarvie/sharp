/*
 * scope.h — Sharp Frontend: Phase 4 Scope chain and symbol table.
 *
 * Builds a Scope tree from an AstNode* (AST_FILE) that lets every name
 * reference be resolved to its declaring AstNode.  No type inference here
 * — just lexical scoping.
 *
 * Scope nesting:
 *   SCOPE_GLOBAL  (built-in types: int, long, char, …)
 *     SCOPE_FILE  (top-level structs, funcs, vars, typedefs)
 *       SCOPE_STRUCT (fields + methods of one struct)
 *       SCOPE_FUNC   (function params + body)
 *         SCOPE_BLOCK  (nested { } blocks)
 *           SCOPE_BLOCK  (deeper nesting …)
 *
 * Design decisions (Phase 4):
 *   - Two-pass build: Pass 1 registers all file-level names so forward
 *     references work.  Pass 2 recurses into function bodies.
 *   - Hash table per scope with fixed 16 buckets (open chaining).
 *   - Strings are owned by Scope (strdup'd on scope_define).
 *   - scope_lookup walks the parent chain; scope_lookup_local does not.
 *   - Redefinition in the same scope emits CPP_DIAG_ERROR.
 */
#ifndef SHARP_FE_SCOPE_H
#define SHARP_FE_SCOPE_H

#include "ast.h"
#include "sharp_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * Symbol kinds
 * ---------------------------------------------------------------------- */
typedef enum {
    SYM_VAR,           /* local or global variable                        */
    SYM_FUNC,          /* function (free or associated)                   */
    SYM_TYPE,          /* struct name or typedef alias                    */
    SYM_PARAM,         /* function parameter                              */
    SYM_FIELD,         /* struct field                                    */
    SYM_METHOD,        /* struct method / associated function             */
    SYM_GENERIC_PARAM, /* <T> type parameter                              */
    SYM_BUILTIN,       /* built-in type (int, long, char, …)              */
} SymKind;

/* -------------------------------------------------------------------------
 * Symbol
 * ---------------------------------------------------------------------- */
typedef struct Symbol Symbol;
struct Symbol {
    SymKind      kind;
    char        *name;    /* owned (strdup'd)                             */
    AstNode     *decl;    /* NULL for builtins                            */
    Symbol      *next;    /* next symbol in the same hash bucket          */
};

/* -------------------------------------------------------------------------
 * Scope kinds
 * ---------------------------------------------------------------------- */
typedef enum {
    SCOPE_GLOBAL,
    SCOPE_FILE,
    SCOPE_STRUCT,
    SCOPE_FUNC,
    SCOPE_BLOCK,
} ScopeKind;

/* -------------------------------------------------------------------------
 * Scope
 * ---------------------------------------------------------------------- */
#define SCOPE_NBUCKETS 16

typedef struct Scope Scope;
struct Scope {
    ScopeKind  kind;
    Scope     *parent;              /* NULL for SCOPE_GLOBAL               */
    AstNode   *owner;               /* struct/func decl that owns this scope*/
    Symbol    *buckets[SCOPE_NBUCKETS];
    size_t     nsymbols;
    /* Phase 4: child scopes created by scope_new() with this as parent.
     * scope_free() recursively frees all children, so scope_free_chain()
     * only needs to free the parent chain (not individual children).     */
    Scope    **children;
    size_t     nchildren;
    size_t     children_cap;
};

/* -------------------------------------------------------------------------
 * Construction / destruction
 * ---------------------------------------------------------------------- */

/** Allocate a new scope of the given kind with optional parent. */
Scope *scope_new(ScopeKind kind, Scope *parent, AstNode *owner);

/** Free this scope and all symbols it owns.  Does NOT free the parent. */
void   scope_free(Scope *s);

/** Free the entire chain from s up to (and including) the root. */
void   scope_free_chain(Scope *s);

/* -------------------------------------------------------------------------
 * Symbol operations
 * ---------------------------------------------------------------------- */

/**
 * Define a symbol in scope s.
 * Returns the new Symbol on success.
 * On redefinition in the same scope, emits CPP_DIAG_ERROR to diags and
 * returns the *existing* symbol (caller can decide to continue or stop).
 */
Symbol *scope_define(Scope *s, SymKind kind, const char *name,
                     AstNode *decl, FeDiagArr *diags);

/**
 * Look up name in s and all ancestor scopes.
 * Returns the nearest Symbol, or NULL if not found.
 */
Symbol *scope_lookup(Scope *s, const char *name);

/**
 * Look up name in s only (no parent walk).
 */
Symbol *scope_lookup_local(Scope *s, const char *name);

/**
 * Type-namespace lookup: walks the bucket chain at `name` and returns
 * the first symbol whose kind is SYM_TYPE / SYM_BUILTIN /
 * SYM_GENERIC_PARAM.  Required when the same scope holds both a tag
 * symbol and an ordinary identifier with the same name (legal in C —
 * `struct sigaction { ... };` plus `int sigaction(int);`).  Plain
 * scope_lookup returns whichever was inserted last; the type-aware
 * variant is needed by ty_from_ast and cg to interpret type contexts.
 */
Symbol *scope_lookup_local_type(Scope *s, const char *name);
Symbol *scope_lookup_type(Scope *s, const char *name);

/* -------------------------------------------------------------------------
 * High-level: build scope tree from an AST_FILE
 * ---------------------------------------------------------------------- */

/**
 * Build the complete scope tree for the file.
 *
 * Two-pass strategy so forward references work:
 *   Pass 1 — register all top-level names (struct, func, typedef, var).
 *   Pass 2 — recurse into struct bodies and function bodies.
 *
 * Returns the SCOPE_FILE scope (its parent is a fresh SCOPE_GLOBAL with
 * built-in types already registered).  The caller owns the returned scope
 * and should call scope_free_chain() when done.
 *
 * Diagnostics (redefinitions, etc.) are pushed to *diags.
 */
Scope *scope_build(AstNode *file, FeDiagArr *diags);

/** Return a human-readable name for a SymKind. */
const char *sym_kind_name(SymKind k);

/** Return a human-readable name for a ScopeKind. */
const char *scope_kind_name(ScopeKind k);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_SCOPE_H */
