/*
 * type.h — Sharp Frontend: Phase 5 Type System.
 *
 * Provides an interned type representation.  Two Type* values are equal
 * (ty_eq) iff they point to the same interned node, which is guaranteed
 * by the construction functions below.
 *
 * All types are owned by a TyStore.  The store must outlive every Type*
 * it produced.  Free with ty_store_free().
 *
 * Primitive types (void, bool, int, …) are singletons inside TyStore.
 * Compound types (ptr, const, array, struct, param) are heap-allocated
 * and deduplicated via a linear-scan intern table.
 */
#ifndef SHARP_FE_TYPE_H
#define SHARP_FE_TYPE_H

#include "scope.h"   /* Scope, FeDiagArr — also pulls in ast.h */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------
 * TyKind
 * ---------------------------------------------------------------------- */
typedef enum {
    TY_ERROR,      /* error sentinel — prevents cascading errors           */
    TY_VOID,
    TY_BOOL,
    /* Signed integers */
    TY_CHAR, TY_SHORT, TY_INT, TY_LONG, TY_LONGLONG,
    /* Unsigned integers */
    TY_UCHAR, TY_USHORT, TY_UINT, TY_ULONG, TY_ULONGLONG,
    /* Floating-point */
    TY_FLOAT, TY_DOUBLE,
    /* Compound */
    TY_PTR,        /* T*                                                   */
    TY_ARRAY,      /* T[N]  (size==-1 means incomplete T[])               */
    TY_CONST,      /* const T                                              */
    TY_FUNC,       /* ret(params…)  — not interned in Phase 5             */
    TY_STRUCT,     /* named struct, may carry generic args                 */
    TY_PARAM,      /* unbound generic parameter T                         */
    TY_COUNT
} TyKind;

/* -------------------------------------------------------------------------
 * Type
 * ---------------------------------------------------------------------- */
typedef struct Type Type;
struct Type {
    TyKind kind;
    union {
        /* TY_PTR */
        struct { Type *base; } ptr;
        /* TY_ARRAY — size == -1 for incomplete [] */
        struct { Type *base; int64_t size; } array;
        /* TY_CONST */
        struct { Type *base; } const_;
        /* TY_FUNC */
        struct { Type *ret; Type **params; size_t nparams; } func;
        /* TY_STRUCT */
        struct {
            const char  *name;   /* interned string                       */
            Type       **args;   /* generic args (interned), or NULL      */
            size_t       nargs;
            AstNode     *decl;   /* originating AstNode (may be NULL)     */
        } struct_;
        /* TY_PARAM */
        struct { const char *name; } param;
    } u;
};

/* -------------------------------------------------------------------------
 * TyStore — owns all Type objects
 * ---------------------------------------------------------------------- */
typedef struct TyStore TyStore;

TyStore *ty_store_new(void);
void     ty_store_free(TyStore *ts);

/* -------------------------------------------------------------------------
 * Primitive type accessors (always return the same pointer for a given ts)
 * ---------------------------------------------------------------------- */
Type *ty_error(TyStore *ts);
Type *ty_void(TyStore *ts);
Type *ty_bool(TyStore *ts);
Type *ty_char(TyStore *ts);
Type *ty_short(TyStore *ts);
Type *ty_int(TyStore *ts);
Type *ty_long(TyStore *ts);
Type *ty_longlong(TyStore *ts);
Type *ty_uchar(TyStore *ts);
Type *ty_ushort(TyStore *ts);
Type *ty_uint(TyStore *ts);
Type *ty_ulong(TyStore *ts);
Type *ty_ulonglong(TyStore *ts);
Type *ty_float(TyStore *ts);
Type *ty_double(TyStore *ts);

/* -------------------------------------------------------------------------
 * Compound type constructors (interned — equal inputs → same pointer)
 * ---------------------------------------------------------------------- */
Type *ty_ptr(TyStore *ts, Type *base);
Type *ty_array(TyStore *ts, Type *base, int64_t size);
Type *ty_const(TyStore *ts, Type *base);
Type *ty_func(TyStore *ts, Type *ret, Type **params, size_t nparams);
Type *ty_struct_type(TyStore *ts, const char *name,
                     Type **args, size_t nargs, AstNode *decl);
Type *ty_param(TyStore *ts, const char *name);

/* -------------------------------------------------------------------------
 * Equality — pointer equality after interning
 * ---------------------------------------------------------------------- */
static inline bool ty_eq(const Type *a, const Type *b) { return a == b; }

/* -------------------------------------------------------------------------
 * Queries
 * ---------------------------------------------------------------------- */
bool ty_is_const(const Type *t);      /* true iff top-level const         */
bool ty_is_unsigned(const Type *t);   /* true for UCHAR…ULONGLONG         */
bool ty_is_signed_int(const Type *t); /* CHAR…LONGLONG                    */
bool ty_is_integer(const Type *t);    /* any integral type                */
bool ty_is_float(const Type *t);      /* FLOAT or DOUBLE                  */
bool ty_is_arithmetic(const Type *t); /* integer or float                 */
bool ty_is_pointer(const Type *t);    /* TY_PTR (with optional const)     */
bool ty_is_void(const Type *t);       /* TY_VOID                          */
bool ty_is_error(const Type *t);      /* TY_ERROR                         */
bool ty_is_scalar(const Type *t);     /* arithmetic or pointer            */

/* -------------------------------------------------------------------------
 * Modifiers
 * ---------------------------------------------------------------------- */
/** Remove one layer of top-level const.  Returns base if const, else t. */
Type *ty_unconst(TyStore *ts, Type *t);

/** Return the pointee type, or NULL if not a pointer. */
Type *ty_deref(const Type *t);

/* -------------------------------------------------------------------------
 * Lookup: built-in type name → Type* (NULL if not a built-in name)
 * ---------------------------------------------------------------------- */
Type *ty_from_name(TyStore *ts, const char *name);

/* -------------------------------------------------------------------------
 * Resolve: AstNode type expression → Type*
 *
 * Uses scope to look up user-defined type names.
 * AST_TYPE_AUTO returns ty_error() — caller (sema) must infer the type.
 * On error, pushes a diagnostic and returns ty_error().
 * ---------------------------------------------------------------------- */
Type *ty_from_ast(TyStore *ts, const AstNode *node,
                  Scope *scope, FeDiagArr *diags);

/* -------------------------------------------------------------------------
 * Debug
 * ---------------------------------------------------------------------- */
const char *ty_kind_name(TyKind k);
void        ty_print(const Type *t, FILE *fp);

#ifdef __cplusplus
}
#endif
#endif /* SHARP_FE_TYPE_H */
