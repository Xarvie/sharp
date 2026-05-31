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
    char        *name;           /* owned (strdup'd)                          */
    AstNode     *decl;           /* NULL for builtins and opaque syms         */
    Symbol      *next;           /* next symbol in the same hash bucket       */
    bool         was_typedef;    /* typedef struct Foo Foo; self-typedef       */
    bool         is_enum_const;  /* V3: true for injected enum constant symbols */
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
/* Initial bucket count for a new scope's symbol table.
 * Must be a power of two.  All scope kinds start at 8; the file scope
 * grows on demand (via scope_insert) up to whatever is needed.
 * Keeping this small avoids wasting memory for short-lived block scopes. */
#define SCOPE_INIT_BUCKETS 8

typedef struct Scope Scope;
struct Scope {
    ScopeKind  kind;
    Scope     *parent;              /* NULL for SCOPE_GLOBAL               */
    AstNode   *owner;               /* struct/func decl that owns this scope*/

    /* Open-addressing (linear-probing) symbol hash table.
     * buckets[i] is NULL for empty slots, (Symbol*)1 for tombstones.
     * Invariant: nbuckets is always a power of two.
     * Load factor kept below 0.75; resized 2x when threshold is hit.     */
    Symbol   **buckets;
    size_t     nbuckets;            /* current capacity (power of two)     */
    size_t     nsymbols;            /* live entries (tombstones not counted)*/
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


/* ── Symbol lookup ──────────────────────────────────────────────────────
 *
 * Two orthogonal axes:
 *   • scope chain:  local (current scope only) vs chain (walk parents)
 *   • kind filter:  any | type-namespace only | struct/union tag only
 *
 * The primary interface is two functions:
 *   scope_lookup_local(s, name)      — current scope, any kind
 *   scope_lookup_ex(s, name, flags)  — optionally chain, with kind filter
 *
 * Flags:
 *   SLOOK_ANY        — match any SymKind  (default, value 0)
 *   SLOOK_TYPE_ONLY  — match SYM_TYPE / SYM_BUILTIN / SYM_GENERIC_PARAM
 *   SLOOK_STRUCT_TAG — match SYM_TYPE whose decl is AST_STRUCT_DEF
 */
typedef enum {
    SLOOK_ANY        = 0,
    SLOOK_TYPE_ONLY  = 1,
    SLOOK_STRUCT_TAG = 2,
} ScopeLookupFlags;

Symbol *scope_lookup_local(Scope *s, const char *name);
Symbol *scope_lookup_ex(Scope *s, const char *name, ScopeLookupFlags flags);

/* scope_lookup_value — like scope_lookup but in expression context,
 * prefers SYM_VAR/SYM_FUNC/SYM_PARAM/SYM_FIELD over SYM_TYPE when both
 * share the same name. Mirrors C's separate tag/ordinary identifier namespaces. */
Symbol *scope_lookup_value(Scope *s, const char *name);

/* Legacy wrappers — zero overhead; kept so existing call-sites compile
 * without mechanical mass-renaming. */
static inline Symbol *scope_lookup(Scope *s, const char *name) {
    return scope_lookup_ex(s, name, SLOOK_ANY);
}
static inline Symbol *scope_lookup_type(Scope *s, const char *name) {
    return scope_lookup_ex(s, name, SLOOK_TYPE_ONLY);
}
static inline Symbol *scope_lookup_struct_tag(Scope *s, const char *name) {
    return scope_lookup_ex(s, name, SLOOK_STRUCT_TAG);
}
/* Local type lookup — current scope only, type-namespace filter. */
Symbol *scope_lookup_local_type(Scope *s, const char *name);

/* scope_find_typedef — find a typedef symbol by name in the scope chain.
 * Returns the first SYM_TYPE symbol whose decl is AST_TYPEDEF_DECL, or NULL.
 * Used by cg.c to detect typedef aliases for function-pointer and
 * ptr-to-array return types where the flat typedef form must be preserved. */
Symbol *scope_find_typedef(Scope *scope, const char *name);

/* scope_lookup_next_local — advance past a previously found symbol to the
 * next symbol with the same name in the same hash bucket chain.  Used to
 * iterate operator overloads: after scope_lookup_local() returns the first
 * "operator+" symbol, repeated calls to this function yield any additional
 * overloads registered under the same name.  Returns NULL at end of chain.
 * Only searches within the bucket chain (i.e., the same scope level). */
Symbol *scope_lookup_next_local(Scope *s, Symbol *prev, const char *name);


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
/* B2: variant that accepts a pre-built prelude scope (pass NULL for default) */
Scope *scope_build_with_prelude(AstNode *file, FeDiagArr *diags,
                                 Scope *prelude);

/** Return a human-readable name for a SymKind. */
const char *sym_kind_name(SymKind k);

/** Return a human-readable name for a ScopeKind. */
const char *scope_kind_name(ScopeKind k);

/** Find an extension method in a file scope by struct name and method name.
 * Scans all buckets for a SYM_FUNC whose decl is an AST_FUNC_DEF with
 * the given struct_name and method_name.  Returns the first match, or NULL. */
Symbol *find_extension_method(Scope *file_scope, const char *struct_name,
                              const char *method_name);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_SCOPE_H */
