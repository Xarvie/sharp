/*
 * type.c — Phase 5 Type System implementation.
 */
#include "type.h"
#include "lex.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * TyStore layout
 * ====================================================================== */

struct TyStore {
    /* Primitive singletons — stable addresses for the store's lifetime. */
    Type prims[TY_COUNT];

    /* Interned compound types (heap-allocated, deduplicated). */
    Type  **compound;
    size_t  compound_len;
    size_t  compound_cap;

    /* Interned strings (names for TY_PARAM and TY_STRUCT). */
    char  **strings;
    size_t  strings_len;
    size_t  strings_cap;
};

/* =========================================================================
 * String intern
 * ====================================================================== */

static const char *ts_intern_str(TyStore *ts, const char *s) {
    for (size_t i = 0; i < ts->strings_len; i++)
        if (strcmp(ts->strings[i], s) == 0) return ts->strings[i];
    if (ts->strings_len == ts->strings_cap) {
        size_t nc = ts->strings_cap ? ts->strings_cap * 2 : 16;
        ts->strings = realloc(ts->strings, nc * sizeof *ts->strings);
        if (!ts->strings) { perror("sharp-fe type"); abort(); }
        ts->strings_cap = nc;
    }
    char *copy = cpp_xstrdup(s);
    ts->strings[ts->strings_len++] = copy;
    return copy;
}

/* =========================================================================
 * Compound type intern
 * ====================================================================== */

/* Structural equality for two compound types (base pointers already interned). */
static bool ty_compound_eq(const Type *a, const Type *b) {
    if (a->kind != b->kind) return false;
    switch (a->kind) {
    case TY_PTR:    return a->u.ptr.base    == b->u.ptr.base;
    case TY_CONST:  return a->u.const_.base == b->u.const_.base;
    case TY_ARRAY:  return a->u.array.base  == b->u.array.base &&
                           a->u.array.size  == b->u.array.size;
    case TY_PARAM:  return a->u.param.name  == b->u.param.name; /* interned */
    case TY_STRUCT:
        if (a->u.struct_.name  != b->u.struct_.name)  return false;
        if (a->u.struct_.nargs != b->u.struct_.nargs)  return false;
        for (size_t i = 0; i < a->u.struct_.nargs; i++)
            if (a->u.struct_.args[i] != b->u.struct_.args[i]) return false;
        return true;
    case TY_FUNC:
        if (a->u.func.ret    != b->u.func.ret)    return false;
        if (a->u.func.nparams != b->u.func.nparams) return false;
        for (size_t i = 0; i < a->u.func.nparams; i++)
            if (a->u.func.params[i] != b->u.func.params[i]) return false;
        return true;
    default: return false;
    }
}

static Type *ts_intern(TyStore *ts, Type candidate) {
    for (size_t i = 0; i < ts->compound_len; i++)
        if (ty_compound_eq(ts->compound[i], &candidate)) return ts->compound[i];

    Type *t = malloc(sizeof *t);
    if (!t) { perror("sharp-fe type"); abort(); }
    *t = candidate;

    /* For TY_STRUCT: copy the args array. */
    if (t->kind == TY_STRUCT && t->u.struct_.nargs > 0) {
        size_t sz = t->u.struct_.nargs * sizeof(Type*);
        Type **copy = malloc(sz);
        if (!copy) { perror("sharp-fe type"); abort(); }
        memcpy(copy, t->u.struct_.args, sz);
        t->u.struct_.args = copy;
    }
    /* For TY_FUNC: copy the params array. */
    if (t->kind == TY_FUNC && t->u.func.nparams > 0) {
        size_t sz = t->u.func.nparams * sizeof(Type*);
        Type **copy = malloc(sz);
        if (!copy) { perror("sharp-fe type"); abort(); }
        memcpy(copy, t->u.func.params, sz);
        t->u.func.params = copy;
    }

    if (ts->compound_len == ts->compound_cap) {
        size_t nc = ts->compound_cap ? ts->compound_cap * 2 : 16;
        ts->compound = realloc(ts->compound, nc * sizeof *ts->compound);
        if (!ts->compound) { perror("sharp-fe type"); abort(); }
        ts->compound_cap = nc;
    }
    ts->compound[ts->compound_len++] = t;
    return t;
}

/* =========================================================================
 * Lifecycle
 * ====================================================================== */

TyStore *ty_store_new(void) {
    TyStore *ts = calloc(1, sizeof *ts);
    if (!ts) { perror("sharp-fe type"); abort(); }

    /* Phase 5: initialise primitive singleton kinds. */
    for (int k = 0; k < TY_COUNT; k++)
        ts->prims[k].kind = (TyKind)k;

    return ts;
}

void ty_store_free(TyStore *ts) {
    if (!ts) return;
    for (size_t i = 0; i < ts->compound_len; i++) {
        Type *t = ts->compound[i];
        if (t->kind == TY_STRUCT && t->u.struct_.nargs > 0) free(t->u.struct_.args);
        if (t->kind == TY_FUNC   && t->u.func.nparams > 0)  free(t->u.func.params);
        free(t);
    }
    free(ts->compound);
    for (size_t i = 0; i < ts->strings_len; i++) free(ts->strings[i]);
    free(ts->strings);
    free(ts);
}

/* =========================================================================
 * Primitive accessors
 * ====================================================================== */
Type *ty_error(TyStore *ts)    { return &ts->prims[TY_ERROR]; }
Type *ty_void(TyStore *ts)     { return &ts->prims[TY_VOID]; }
Type *ty_bool(TyStore *ts)     { return &ts->prims[TY_BOOL]; }
Type *ty_char(TyStore *ts)     { return &ts->prims[TY_CHAR]; }
Type *ty_short(TyStore *ts)    { return &ts->prims[TY_SHORT]; }
Type *ty_int(TyStore *ts)      { return &ts->prims[TY_INT]; }
Type *ty_long(TyStore *ts)     { return &ts->prims[TY_LONG]; }
Type *ty_longlong(TyStore *ts) { return &ts->prims[TY_LONGLONG]; }
Type *ty_uchar(TyStore *ts)    { return &ts->prims[TY_UCHAR]; }
Type *ty_ushort(TyStore *ts)   { return &ts->prims[TY_USHORT]; }
Type *ty_uint(TyStore *ts)     { return &ts->prims[TY_UINT]; }
Type *ty_ulong(TyStore *ts)    { return &ts->prims[TY_ULONG]; }
Type *ty_ulonglong(TyStore *ts){ return &ts->prims[TY_ULONGLONG]; }
Type *ty_float(TyStore *ts)    { return &ts->prims[TY_FLOAT]; }
Type *ty_double(TyStore *ts)   { return &ts->prims[TY_DOUBLE]; }

/* =========================================================================
 * Compound constructors
 * ====================================================================== */

Type *ty_ptr(TyStore *ts, Type *base) {
    return ts_intern(ts, (Type){ .kind = TY_PTR, .u.ptr = { base } });
}
Type *ty_array(TyStore *ts, Type *base, int64_t size) {
    return ts_intern(ts, (Type){ .kind = TY_ARRAY, .u.array = { base, size } });
}
Type *ty_const(TyStore *ts, Type *base) {
    if (base->kind == TY_CONST) return base; /* const const T == const T */
    return ts_intern(ts, (Type){ .kind = TY_CONST, .u.const_ = { base } });
}
Type *ty_func(TyStore *ts, Type *ret, Type **params, size_t nparams) {
    return ts_intern(ts, (Type){ .kind = TY_FUNC,
        .u.func = { ret, params, nparams } });
}
Type *ty_struct_type(TyStore *ts, const char *name,
                     Type **args, size_t nargs, AstNode *decl) {
    const char *iname = ts_intern_str(ts, name);
    /* Search for an existing interned TY_STRUCT with the same name and
     * generic args.  If found, update its decl pointer when the new
     * call has a richer decl (a full struct body vs. a forward decl or
     * a typedef sentinel).  This ensures that all code paths that call
     * ty_struct_type for the same struct tag get the same Type* node
     * AND that node's decl reflects the most complete AST seen so far.
     * Without this update, `scope_define`'s promotion of a typedef
     * symbol to the full struct body is invisible to the compound type
     * intern table, causing field lookups to use the stale forward-decl
     * AST and different ty_from_ast invocations to produce the same
     * interned pointer even though they passed different decl values. */
    Type candidate = { .kind = TY_STRUCT,
                       .u.struct_ = { iname, args, nargs, decl } };
    for (size_t i = 0; i < ts->compound_len; i++) {
        if (ty_compound_eq(ts->compound[i], &candidate)) {
            /* Found existing node.  Promote decl when the new one has
             * a struct body and the existing one does not. */
            AstNode *od = ts->compound[i]->u.struct_.decl;
            bool od_has_body = od && od->kind == AST_STRUCT_DEF &&
                               (od->u.struct_def.fields.len > 0 ||
                                od->u.struct_def.methods.len > 0);
            bool nd_has_body = decl && decl->kind == AST_STRUCT_DEF &&
                               (decl->u.struct_def.fields.len > 0 ||
                                decl->u.struct_def.methods.len > 0);
            if (nd_has_body && !od_has_body) {
                ts->compound[i]->u.struct_.decl = decl;
            }
            return ts->compound[i];
        }
    }
    /* Not found — allocate and intern a new node. */
    Type *t = malloc(sizeof *t);
    if (!t) { perror("sharp-fe type"); abort(); }
    *t = candidate;
    /* Copy the generic-args array so the node is independent of the
     * caller's stack / temporary allocation.  The original ts_intern
     * had the same copy step; we must replicate it here since we no
     * longer route through ts_intern. */
    if (t->u.struct_.nargs > 0 && t->u.struct_.args) {
        size_t sz = t->u.struct_.nargs * sizeof(Type*);
        Type **copy = malloc(sz);
        if (!copy) { perror("sharp-fe type"); abort(); }
        memcpy(copy, t->u.struct_.args, sz);
        t->u.struct_.args = copy;
    }
    if (ts->compound_len == ts->compound_cap) {
        size_t nc = ts->compound_cap ? ts->compound_cap * 2 : 32;
        ts->compound = realloc(ts->compound, nc * sizeof *ts->compound);
        if (!ts->compound) { perror("sharp-fe type"); abort(); }
        ts->compound_cap = nc;
    }
    ts->compound[ts->compound_len++] = t;
    return t;
}
Type *ty_param(TyStore *ts, const char *name) {
    const char *iname = ts_intern_str(ts, name);
    return ts_intern(ts, (Type){ .kind = TY_PARAM, .u.param = { iname } });
}

/* =========================================================================
 * Queries
 * ====================================================================== */

bool ty_is_const(const Type *t) {
    return t && t->kind == TY_CONST;
}
bool ty_is_unsigned(const Type *t) {
    if (!t) return false;
    switch (t->kind) {
    case TY_UCHAR: case TY_USHORT: case TY_UINT:
    case TY_ULONG: case TY_ULONGLONG: return true;
    default: return false;
    }
}
bool ty_is_signed_int(const Type *t) {
    if (!t) return false;
    switch (t->kind) {
    case TY_CHAR: case TY_SHORT: case TY_INT:
    case TY_LONG: case TY_LONGLONG: return true;
    default: return false;
    }
}
bool ty_is_integer(const Type *t) {
    if (!t) return false;
    /* const T counts as integer iff T does — same as how ty_is_pointer
     * looks through const for `const T*`.  Without this, `const char x;
     * x != 'a'` and `const char *s; s[0] != 'a'` (whose deref yields
     * `const char`) both fail ty_is_scalar in comparison contexts. */
    if (t->kind == TY_CONST) return ty_is_integer(t->u.const_.base);
    return ty_is_signed_int(t) || ty_is_unsigned(t) || t->kind == TY_BOOL;
}
bool ty_is_float(const Type *t) {
    if (!t) return false;
    if (t->kind == TY_CONST) return ty_is_float(t->u.const_.base);
    return t->kind == TY_FLOAT || t->kind == TY_DOUBLE;
}
bool ty_is_arithmetic(const Type *t) {
    return ty_is_integer(t) || ty_is_float(t);
}
bool ty_is_pointer(const Type *t) {
    if (!t) return false;
    if (t->kind == TY_PTR) return true;
    /* In Sharp (like C), arrays decay to pointers in expression contexts —
     * subscripting and pointer arithmetic both apply to T[N]. */
    if (t->kind == TY_ARRAY) return true;
    /* const T* counts as a pointer */
    if (t->kind == TY_CONST) return ty_is_pointer(t->u.const_.base);
    return false;
}
bool ty_is_void(const Type *t)  { return t && t->kind == TY_VOID; }
bool ty_is_error(const Type *t) { return t && t->kind == TY_ERROR; }
bool ty_is_scalar(const Type *t) {
    return ty_is_arithmetic(t) || ty_is_pointer(t);
}

/* =========================================================================
 * Modifiers
 * ====================================================================== */

Type *ty_unconst(TyStore *ts, Type *t) {
    (void)ts;
    if (t && t->kind == TY_CONST) return t->u.const_.base;
    return t;
}

Type *ty_deref(const Type *t) {
    if (!t) return NULL;
    if (t->kind == TY_PTR) return t->u.ptr.base;
    /* Array decay: T[N] -> T when subscripted / dereferenced. */
    if (t->kind == TY_ARRAY) return t->u.array.base;
    if (t->kind == TY_CONST && t->u.const_.base->kind == TY_PTR)
        return t->u.const_.base->u.ptr.base;
    if (t->kind == TY_CONST && t->u.const_.base->kind == TY_ARRAY)
        return t->u.const_.base->u.array.base;
    return NULL;
}

/* =========================================================================
 * Name → primitive type
 * ====================================================================== */

Type *ty_from_name(TyStore *ts, const char *name) {
    if (!name) return NULL;
    if (!strcmp(name,"void"))                        return ty_void(ts);
    if (!strcmp(name,"bool")||!strcmp(name,"_Bool")) return ty_bool(ts);
    if (!strcmp(name,"char"))                        return ty_char(ts);
    if (!strcmp(name,"short"))                       return ty_short(ts);
    if (!strcmp(name,"int"))                         return ty_int(ts);
    if (!strcmp(name,"long"))                        return ty_long(ts);
    if (!strcmp(name,"float"))                       return ty_float(ts);
    if (!strcmp(name,"double"))                      return ty_double(ts);
    if (!strcmp(name,"signed"))                      return ty_int(ts);
    if (!strcmp(name,"unsigned"))                    return ty_uint(ts);
    if (!strcmp(name,"__int128"))                    return ty_longlong(ts);
    /* GCC builtin types.  __builtin_va_list is the underlying type
     * for C's va_list.  We model it as a TY_STRUCT named
     * "__builtin_va_list" so that cg_type emits the name verbatim
     * and __builtin_va_start / __builtin_va_end / __builtin_va_arg
     * receive the correct type from the C compiler (which defines
     * __builtin_va_list itself).  Previously this returned char* but
     * that caused __builtin_va_start to reject the argument type,
     * crashing at runtime. */
    if (!strcmp(name,"__builtin_va_list"))
        return ty_struct_type(ts, "__builtin_va_list", NULL, 0, NULL);
    /* S1: composed C type names produced by parse_decl_specifiers().
     * The order matters only for short-circuit speed; correctness is
     * by exact-string match. */
    if (!strcmp(name,"signed char"))                 return ty_char(ts);
    if (!strcmp(name,"unsigned char"))               return ty_uchar(ts);
    if (!strcmp(name,"signed short")||
        !strcmp(name,"short int")||
        !strcmp(name,"signed short int"))            return ty_short(ts);
    if (!strcmp(name,"unsigned short")||
        !strcmp(name,"unsigned short int"))          return ty_ushort(ts);
    if (!strcmp(name,"signed int"))                  return ty_int(ts);
    if (!strcmp(name,"unsigned int"))                return ty_uint(ts);
    if (!strcmp(name,"long int")||
        !strcmp(name,"signed long")||
        !strcmp(name,"signed long int"))             return ty_long(ts);
    if (!strcmp(name,"unsigned long")||
        !strcmp(name,"unsigned long int"))           return ty_ulong(ts);
    if (!strcmp(name,"long long")||
        !strcmp(name,"long long int")||
        !strcmp(name,"signed long long")||
        !strcmp(name,"signed long long int"))        return ty_longlong(ts);
    if (!strcmp(name,"unsigned long long")||
        !strcmp(name,"unsigned long long int"))      return ty_ulonglong(ts);
    if (!strcmp(name,"long double"))                 return ty_double(ts);
    return NULL;
}

/* =========================================================================
 * ty_from_ast — resolve AstNode type expression to Type*
 * ====================================================================== */

/* Evaluate a constant expression used as an array size.  Handles the
 * forms commonly produced by CPP macro expansion in system headers:
 *
 *   integer literal              → value
 *   (expr)                       → recurse on inner
 *   sizeof(type-keyword)         → platform size of that primitive
 *   binary: expr * expr          → multiply
 *   binary: expr / expr          → divide (truncated)
 *   binary: expr + expr          → add
 *   binary: expr - expr          → subtract
 *
 * Returns the evaluated value, or -1 if the expression is not a
 * recognizable constant (the array will be emitted as incomplete []).
 * This is pure type-layer arithmetic; no scope/diag are needed because
 * sizeof on primitive types is always known. */
static int64_t eval_array_size(TyStore *ts, const AstNode *expr,
                                Scope *scope) {
    if (!expr) return -1;
    switch (expr->kind) {
    case AST_INT_LIT:
        return expr->u.int_lit.val;
    case AST_UNARY:
        /* (expr) is represented as a UNARY with no real operator in some
         * parens-expression parse outputs; fall through if unrecognised. */
        if (expr->u.unary.op == 0)
            return eval_array_size(ts, expr->u.unary.operand, scope);
        return -1;
    case AST_CAST:
        /* (type)expr — just evaluate the inner. */
        return eval_array_size(ts, expr->u.cast.operand, scope);
    case AST_SIZEOF:
        /* sizeof(primitive) — return the platform size. */
        if (!expr->u.sizeof_.is_type || !expr->u.sizeof_.operand)
            return -1;
        if (expr->u.sizeof_.operand->kind == AST_TYPE_NAME) {
            Type *t = ty_from_name(ts,
                expr->u.sizeof_.operand->u.type_name.name);
            if (!t) return -1;
            switch (t->kind) {
            case TY_CHAR:  case TY_UCHAR:  return 1;
            case TY_SHORT: case TY_USHORT: return sizeof(short);
            case TY_INT:   case TY_UINT:   return sizeof(int);
            case TY_LONG:  case TY_ULONG:  return sizeof(long);
            case TY_LONGLONG: case TY_ULONGLONG: return sizeof(long long);
            case TY_FLOAT:  return sizeof(float);
            case TY_DOUBLE: return sizeof(double);
            default: return -1;
            }
        }
        return -1;
    case AST_BINOP: {
        int64_t l = eval_array_size(ts, expr->u.binop.lhs, scope);
        int64_t r = eval_array_size(ts, expr->u.binop.rhs, scope);
        if (l < 0 || r < 0) return -1;
        switch (expr->u.binop.op) {
        case STOK_PLUS:    return l + r;
        case STOK_MINUS:   return l - r;
        case STOK_STAR:    return l * r;
        case STOK_SLASH:   return r != 0 ? l / r : -1;
        case STOK_PERCENT: return r != 0 ? l % r : -1;
        case STOK_LTLT:    return l << r;
        case STOK_GTGT:    return l >> r;
        default:           return -1;
        }
    }
    default:
        return -1;
    }
}

Type *ty_from_ast(TyStore *ts, const AstNode *node,
                  Scope *scope, FeDiagArr *diags) {
    if (!node) return ty_error(ts);

    switch (node->kind) {
    case AST_TYPE_VOID:  return ty_void(ts);
    case AST_TYPE_AUTO:  return ty_error(ts);  /* sema must infer */
    // AST_TYPE_AUTO already covers error path

    case AST_TYPE_CONST: {
        Type *base = ty_from_ast(ts, node->u.type_const.base, scope, diags);
        return ty_const(ts, base);
    }
    case AST_TYPE_VOLATILE: {
        /* S1: volatile is a C type qualifier.  In Sharp's interned Type*
         * world we don't track it (it has no semantic effect on type
         * compatibility — only on access ordering, which the C compiler
         * enforces).  We pass the qualifier through to the generated C
         * via cg.c; here we return the unqualified base type. */
        return ty_from_ast(ts, node->u.type_volatile.base, scope, diags);
    }
    case AST_TYPE_PTR: {
        Type *base = ty_from_ast(ts, node->u.type_ptr.base, scope, diags);
        return ty_ptr(ts, base);
    }
    case AST_TYPE_ARRAY: {
        Type *base = ty_from_ast(ts, node->u.type_array.base, scope, diags);
        int64_t sz = -1;
        if (node->u.type_array.size)
            sz = eval_array_size(ts, node->u.type_array.size, scope);
        return ty_array(ts, base, sz);
    }

    case AST_TYPE_NAME: {
        const char *name = node->u.type_name.name;
        if (!name) {
            /* Defensive: a malformed AST_TYPE_NAME node with no name
             * shouldn't reach here (parse.c always sets one), but if
             * it does we treat as ty_error rather than null-deref in
             * scope_lookup. */
            return ty_error(ts);
        }
        /* Built-in? */
        Type *prim = ty_from_name(ts, name);
        if (prim) return prim;
        /* Look up in scope. */
        Symbol *sym = scope ? scope_lookup_type(scope, name) : NULL;
        if (!sym) {
            if (diags)
                FE_ERROR(diags, node->loc, "unknown type '%s'", name);
            return ty_error(ts);
        }
        if (sym->kind == SYM_BUILTIN)       return ty_from_name(ts, name);
        if (sym->kind == SYM_GENERIC_PARAM) return ty_param(ts, name);
        if (sym->kind == SYM_TYPE) {
            /* SYM_TYPE covers struct/union tags, typedef aliases, AND
             * enum tags (S2).  We distinguish by the declaration kind:
             *   AST_TYPEDEF_DECL — recurse to target type
             *   AST_ENUM_DEF     — collapses to int (Sharp does not give
             *                      enums a distinct Type*; the C compiler
             *                      sees the original `enum Tag` keyword
             *                      via the round-tripped definition)
             *   AST_STRUCT_DEF   — TY_STRUCT (also covers union via
             *                      is_union flag on the underlying decl)
             */
            if (sym->decl && sym->decl->kind == AST_TYPEDEF_DECL) {
                /* Detect the C idiom `typedef struct Tag Tag;` where the
                 * typedef target resolves back to the same name.  Without
                 * this guard `ty_from_ast` recurses forever, since the
                 * scope lookup on the target name `Tag` finds the same
                 * typedef again.  Resolution: when the target is an
                 * AST_TYPE_NAME naming this typedef itself, materialise
                 * the type as the underlying struct/union tag.  This is
                 * the semantics C gives the idiom — `Tag x;` declares a
                 * struct, not an infinite chain. */
                const AstNode *target = sym->decl->u.typedef_decl.target;
                if (target && target->kind == AST_TYPE_NAME &&
                    target->u.type_name.name &&
                    strcmp(target->u.type_name.name, name) == 0) {
                    return ty_struct_type(ts, name, NULL, 0, sym->decl);
                }
                return ty_from_ast(ts, sym->decl->u.typedef_decl.target,
                                   scope, diags);
            }
            if (sym->decl && sym->decl->kind == AST_ENUM_DEF) {
                return ty_int(ts);
            }
            return ty_struct_type(ts, name, NULL, 0, sym->decl);
        }
        if (diags)
            FE_ERROR(diags, node->loc, "'%s' is not a type", name);
        return ty_error(ts);
    }

    case AST_TYPE_PARAM:
        return ty_param(ts, node->u.type_param.name);

    case AST_TYPE_GENERIC: {
        size_t nargs = node->u.type_generic.args.len;
        Type **args = NULL;
        if (nargs > 0) {
            args = malloc(nargs * sizeof *args);
            if (!args) { perror("sharp-fe type"); abort(); }
            for (size_t i = 0; i < nargs; i++)
                args[i] = ty_from_ast(ts,
                    node->u.type_generic.args.data[i], scope, diags);
        }
        Symbol *sym = scope ? scope_lookup_type(scope, node->u.type_generic.name) : NULL;
        Type *t = ty_struct_type(ts, node->u.type_generic.name,
                                 args, nargs,
                                 sym ? sym->decl : NULL);
        free(args);
        return t;
    }

    case AST_TYPE_FUNC: {
        Type *ret = ty_from_ast(ts, node->u.type_func.ret, scope, diags);
        size_t np = node->u.type_func.params.len;
        Type **params = np ? malloc(np * sizeof *params) : NULL;
        for (size_t i = 0; i < np; i++) {
            /* S1: params hold AST_PARAM_DECL nodes; extract the type. */
            const AstNode *pi = node->u.type_func.params.data[i];
            const AstNode *ty_node = pi;
            if (pi && pi->kind == AST_PARAM_DECL) {
                ty_node = pi->u.param_decl.type;
            }
            params[i] = ty_from_ast(ts, ty_node, scope, diags);
        }
        Type *t = ty_func(ts, ret, params, np);
        free(params);
        return t;
    }

    default:
        if (diags)
            FE_ERROR(diags, node->loc,
                "cannot resolve AST node kind %s to a type",
                ast_kind_name(node->kind));
        return ty_error(ts);
    }
}

/* =========================================================================
 * Debug
 * ====================================================================== */

const char *ty_kind_name(TyKind k) {
    static const char *names[] = {
        "error","void","bool",
        "char","short","int","long","long long",
        "unsigned char","unsigned short","unsigned int","unsigned long","unsigned long long",
        "float","double",
        "ptr","array","const","func","struct","param"
    };
    if ((unsigned)k < TY_COUNT) return names[k];
    return "?";
}

void ty_print(const Type *t, FILE *fp) {
    if (!t) { fprintf(fp, "(null)"); return; }
    switch (t->kind) {
    case TY_PTR:
        ty_print(t->u.ptr.base, fp); fprintf(fp, "*");
        break;
    case TY_CONST:
        fprintf(fp, "const "); ty_print(t->u.const_.base, fp);
        break;
    case TY_ARRAY:
        ty_print(t->u.array.base, fp);
        if (t->u.array.size >= 0) fprintf(fp, "[%lld]", (long long)t->u.array.size);
        else fprintf(fp, "[]");
        break;
    case TY_STRUCT:
        fprintf(fp, "%s", t->u.struct_.name);
        if (t->u.struct_.nargs > 0) {
            fprintf(fp, "<");
            for (size_t i = 0; i < t->u.struct_.nargs; i++) {
                if (i) fprintf(fp, ", ");
                ty_print(t->u.struct_.args[i], fp);
            }
            fprintf(fp, ">");
        }
        break;
    case TY_PARAM:
        fprintf(fp, "%s", t->u.param.name);
        break;
    case TY_FUNC:
        ty_print(t->u.func.ret, fp);
        fprintf(fp, "(");
        for (size_t i = 0; i < t->u.func.nparams; i++) {
            if (i) fprintf(fp, ", ");
            ty_print(t->u.func.params[i], fp);
        }
        fprintf(fp, ")");
        break;
    default:
        fprintf(fp, "%s", ty_kind_name(t->kind));
        break;
    }
}
