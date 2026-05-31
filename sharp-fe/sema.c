/*
 * sema.c — Phase 6 Semantic Analysis (base).
 */
#include "sema.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static bool is_comparison_op(SharpTokKind op) {
    return op == STOK_EQEQ || op == STOK_BANGEQ ||
           op == STOK_LT   || op == STOK_GT ||
           op == STOK_LTEQ || op == STOK_GTEQ;
}

static bool is_gcc_builtin_name(const char *name) {
    return strncmp(name, "__builtin_", 10) == 0 ||
           strncmp(name, "__atomic_",   9) == 0 ||
           strncmp(name, "__sync_",     7) == 0;
}

/* =========================================================================
 * Internal state
 * ====================================================================== */

struct SemaCtx {
    TyStore   *ts;
    Scope     *file_scope;
    FeDiagArr *diags;
};

typedef struct {
    SemaCtx *ctx;
    Scope   *scope;      /* current lexical scope */
    Type    *ret_type;   /* return type of the enclosing function */
    AstNode *cur_constexpr_fn; /* current constexpr function being evaluated */
    /* v0.13: optional generic substitution context.  Set by the cg-side
     * re-eval path to allow @-primitives to resolve TY_PARAM args against
     * concrete instantiation types. */
    const char **subst_pnames;
    Type       **subst_pvals;
    size_t       subst_np;
} SS;

static void sema_require_scalar(SS *ss, const AstNode *node, Type *t, const char *what) {
    if (!ty_is_scalar(t))
        FE_ERROR(ss->ctx->diags, node->loc, "%s condition must be scalar", what);
}

/* =========================================================================
 * Phase 11 sema helper: substitute generic params in a Type*.
 * This mirrors cg.c's subst_type but lives in sema so field-type
 * resolution for generic struct instances works correctly.
 * ====================================================================== */
/* sema_subst moved to type.c as ty_subst */

/* Given a concrete generic TY_STRUCT and a raw field/method Type*,
 * substitute the generic params to produce the concrete type.
 * Returns t unchanged if the struct is not generic or has no decl. */
static Type *sema_subst_for_struct(TyStore *ts, Type *recv_base, Type *t) {
    if (!recv_base || recv_base->kind != TY_STRUCT) return t;
    if (recv_base->u.struct_.nargs == 0) return t;
    AstNode *sd = recv_base->u.struct_.decl;
    if (!sd || sd->kind != AST_STRUCT_DEF) return t;
    size_t np = sd->u.struct_def.generic_params.len;
    if (np != recv_base->u.struct_.nargs) return t;
    const char **pnames = malloc(np * sizeof *pnames);
    if (!pnames) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = sd->u.struct_def.generic_params.data[i]->u.type_param.name;
    Type *r = ty_subst(ts, t, pnames, recv_base->u.struct_.args, np);
    free(pnames);
    return r;
}

static Type *sema_subst_generic_receiver(SS *ss, const AstNode *recv_node, Type *ret_t) {
    if (!recv_node || recv_node->kind != AST_CAST ||
        recv_node->u.cast.operand != NULL ||
        !recv_node->u.cast.type ||
        recv_node->u.cast.type->kind != AST_TYPE_GENERIC)
        return ret_t;
    const AstNode *gt = recv_node->u.cast.type;
    const char *gname = gt->u.type_generic.name;
    size_t ngt = gt->u.type_generic.args.len;
    Type **gtargs = ngt ? malloc(ngt * sizeof *gtargs) : NULL;
    if (gtargs || ngt == 0) {
        TyStore *ts = ss->ctx->ts;
        for (size_t gi = 0; gi < ngt; gi++)
            gtargs[gi] = ty_from_ast(ts, gt->u.type_generic.args.data[gi], ss->scope, NULL);
        Symbol *gsym = scope_lookup_type(ss->scope, gname);
        AstNode *gdecl = (gsym && gsym->decl && gsym->decl->kind == AST_STRUCT_DEF)
                         ? gsym->decl : NULL;
        Type *concrete = ty_struct_type(ts, gname, gtargs, ngt, gdecl);
        if (concrete && !ty_is_error(concrete))
            ret_t = sema_subst_for_struct(ts, concrete, ret_t);
        free(gtargs);
    }
    return ret_t;
}

/* Phase 7 forward declarations */
/* Phase 9+10 forward declarations */
static void sema_defer_check(const AstNode *body, FeDiagArr *diags);
static int  eval_intrinsic(SS *ss, const AstNode *expr);
static int  eval_has_operator(SS *ss, const AstNode *expr);
/* v0.13 — new @-primitives */
static int  eval_has_method(SS *ss, const AstNode *expr);
static int  eval_has_field(SS *ss, const AstNode *expr);
static int  eval_is_arithmetic(SS *ss, const AstNode *expr);
static int  eval_is_pointer(SS *ss, const AstNode *expr);
static char *compute_type_name(SS *ss, const AstNode *expr); /* @type_name */
static Type *resolve_intrinsic_type_arg(SS *ss, const AstNode *arg);
static bool  ty_contains_param(const Type *t);
static bool eval_const_int(const AstNode *expr, int64_t *val);

static Scope *struct_scope_of(TyStore *ts, Type *t, Scope *file_scope);
static const char *op_overload_name(SharpTokKind k);
static bool recv_object_is_const(Type *t);
static Type *sema_field_access_expr(SS *ss, AstNode *expr);
static Type *sema_method_call_expr(SS *ss, AstNode *expr);

/* =========================================================================
 * Type helpers
 * ====================================================================== */

/* Usual arithmetic conversion (simplified): return the "wider" of two
 * arithmetic types.  Priority: double > float > unsigned > signed > bool. */
static Type *arith_conv(TyStore *ts, Type *a, Type *b) {
    if (!ty_is_arithmetic(a) || !ty_is_arithmetic(b)) return a;
    /* Strip type qualifiers: in arithmetic context, `const T` behaves as `T`.
     * Without stripping, a TY_CONST node would slip past the early-return
     * guards below and cause an out-of-bounds read in rank[] (TY_CONST is
     * beyond TY_ULONGLONG which is the last index in the array).
     * Note: TY_VOLATILE does not exist in the type system (volatile is an
     * AST-level qualifier stripped by ty_from_ast); only TY_CONST and TY_ATOMIC need
     * peeling here. */
    a = ty_strip_cvq(ts, a);
    b = ty_strip_cvq(ts, b);
    if (!a || !b) return ts ? ty_int(ts) : a;
    /* double wins */
    if (a->kind == TY_LONGDOUBLE || b->kind == TY_LONGDOUBLE) return ty_longdouble(ts, "long double");
    if (a->kind == TY_DOUBLE || b->kind == TY_DOUBLE) return ty_double(ts);
    if (a->kind == TY_FLOAT  || b->kind == TY_FLOAT)  return ty_float(ts);
    /* unsigned long long > long long > unsigned int > int > ... */
    static const int rank[] = {
        [TY_BOOL]=0, [TY_CHAR]=1, [TY_SHORT]=2, [TY_INT]=3, [TY_LONG]=4,
        [TY_LONGLONG]=5,
        [TY_UCHAR]=1,[TY_USHORT]=2,[TY_UINT]=3,[TY_ULONG]=4,[TY_ULONGLONG]=5
    };
    /* Bounds-safe: only index if kind is within the rank array. */
    int max_rank_idx = (int)(sizeof rank / sizeof rank[0]) - 1;
    int ra = (a->kind <= (TyKind)max_rank_idx) ? rank[a->kind] : 0;
    int rb = (b->kind <= (TyKind)max_rank_idx) ? rank[b->kind] : 0;
    if (ty_is_unsigned(a) || ty_is_unsigned(b)) {
        /* If either operand is unsigned, result is unsigned of higher rank. */
        int r = ra > rb ? ra : rb;
        switch (r) {
        case 5: return ty_ulonglong(ts);
        case 4: return ty_ulong(ts);
        case 3: return ty_uint(ts);
        default: return ty_uint(ts);
        }
    }
    return (ra >= rb) ? a : b;
}

/* Check if rhs type is assignment-compatible with lhs type.
 * Returns true if the assignment is valid.  Returns false if it would
 * discard qualifiers or is otherwise incompatible. */
static bool assign_compat(TyStore *ts, Type *lhs, Type *rhs) {
    if (!lhs || !rhs) return false;
    if (ty_is_error(lhs) || ty_is_error(rhs)) return true; /* suppress cascade */
    if (ty_eq(lhs, rhs)) return true;
    /* __typeof__(...) verbatim opaque types — we cannot check
     * compatibility without evaluating the typeof expression.  Accept any
     * assignment to/from these types so that stdatomic.h macros like
     * `__typeof__((void)0, *ptr) tmp = val` compile without errors. */
    if (lhs->kind == TY_STRUCT && lhs->u.struct_.name &&
        strncmp(lhs->u.struct_.name, "__typeof__(", 11) == 0) return true;
    if (rhs->kind == TY_STRUCT && rhs->u.struct_.name &&
        strncmp(rhs->u.struct_.name, "__typeof__(", 11) == 0) return true;

    /* GCC vector extension: vector types of the same size are assignable.
     * Also allow vector ↔ scalar for broadcast/extract patterns. */
    if (ty_is_vector(lhs) && ty_is_vector(rhs)) {
        if (ty_eq(ty_unconst(ts, lhs), ty_unconst(ts, rhs))) return true;
        if (lhs->u.vector.count == rhs->u.vector.count &&
            ty_eq(ty_unconst(ts, lhs->u.vector.elem),
                  ty_unconst(ts, rhs->u.vector.elem))) return true;
        return false;
    }
    if (ty_is_vector(lhs) && ty_is_arithmetic(rhs)) return true;
    if (ty_is_arithmetic(lhs) && ty_is_vector(rhs)) return true;

    /* S4: const is a *storage* qualifier, not a property of a read value.
     * For VALUE types (anything except pointers), `const T` and `T` are
     * mutually convertible: writing requires the LHS not to be const,
     * but the value-level rule "rvalue of qualified type has unqualified
     * type" (C 6.3.2.1) means initialising or returning a `const T` from
     * an unqualified `T` (and vice-versa) is fine.  We therefore unconst
     * both sides and accept on equality, EXCEPT for pointer types where
     * the pointee qualifier carries semantic weight (handled below). */
    if (!ty_is_pointer(lhs) && !ty_is_pointer(rhs) &&
        ty_eq(ty_unconst(ts, lhs), ty_unconst(ts, rhs))) {
        return true;
    }

    /* T → const T (adding const is always fine). */
    if (ty_is_const(lhs) && ty_eq(lhs->u.const_.base, rhs)) return true;

    /* (Removed: an old over-broad rule that rejected any
     * non-const-pointer lhs vs const rhs.  C distinguishes
     * top-level const on the pointer vs const on the pointee — the
     * former is dropped for rvalues (see C 6.3.2.1), the latter
     * is checked by the pointer-compat path below.  The general
     * "rvalue strips top-level const" handling lives below the
     * pointer-compat block.) */

    /* S4: string literal → char array.  In C, a string-literal
     * initializer for a `char[N]` (or `char[]`) variable copies the
     * bytes into the array.  The literal's expression type is `char *`
     * but the initialization context lifts the array-from-string rule.
     * We accept any TY_ARRAY whose element is char/uchar from a TY_PTR
     * to char/uchar (the literal). */
    if (lhs->kind == TY_ARRAY && rhs->kind == TY_PTR) {
        Type *eb = ty_unconst(ts, lhs->u.array.base);
        Type *pb = ty_unconst(ts, rhs->u.ptr.base);
        if ((eb->kind == TY_CHAR || eb->kind == TY_UCHAR) &&
            (pb->kind == TY_CHAR || pb->kind == TY_UCHAR)) {
            return true;
        }
    }

    /* Arithmetic ↔ arithmetic: always accept (narrowing is a warn, not error
     * in Phase 6; strict narrowing left for Phase 7). */
    if (ty_is_arithmetic(lhs) && ty_is_arithmetic(rhs)) return true;

    /* Pointer compat */
    if (ty_is_pointer(lhs) && ty_is_pointer(rhs)) {
        Type *lb = ty_deref(lhs), *rb = ty_deref(rhs);
        /* C 6.3.2.3: a pointer to void may be converted to or from a
         * pointer to any object type and back without information loss.
         * Accept either side being void* (after stripping any const on
         * the pointee — `void *` vs `const void *` is the typical
         * spelling pair).  This handles libc's `memcpy` returning
         * `void*` assigned to `char *` and Lua's `b = (char *)b + m;`
         * back to a `void *b` parameter. */
        Type *lb_uc = ty_unconst(ts, lb);
        Type *rb_uc = ty_unconst(ts, rb);
        if ((lb_uc && lb_uc->kind == TY_VOID) ||
            (rb_uc && rb_uc->kind == TY_VOID))
            return true;
        /* va_arg(ap, T*) returns int* as a sema placeholder (the
         * real type is known only at C-compile time).  Accept any pointer
         * assignment from int* so that `pArgList = va_arg(ap, StructType*)`
         * does not produce a spurious "incompatible ptr" error. */
        if (rb_uc && rb_uc->kind == TY_INT) return true;
        /* Adding const to pointee is fine: int* → const int*. */
        if (ty_is_const(lb) && !ty_is_const(rb) &&
            ty_eq(lb->u.const_.base, rb)) return true;
        /* C allows dropping const from pointee (const T* → T*)
         * with a warning, not an error.  Previously this was `return false`
         * which caused spurious errors on sqlite3's `p->z = "0"` and all
         * string-literal assignments to char* fields.  Accept here and let
         * the downstream cc emit -Wdiscarded-qualifiers if desired. */
        if (!ty_is_const(lb) && ty_is_const(rb)) return true;
        if (ty_eq(ty_unconst(ts, lb), ty_unconst(ts, rb))) return true;
        /* pointer-to-array compatibility.
         * `unsigned char (*)[N]` ↔ `unsigned const char (*)[N]`.
         * When both pointees are TY_ARRAY, compare unconst element types. */
        if (lb && lb->kind == TY_ARRAY && rb && rb->kind == TY_ARRAY) {
            Type *le = ty_unconst(ts, lb->u.array.base);
            Type *re = ty_unconst(ts, rb->u.array.base);
            if (le && re && ty_eq(le, re)) return true;
        }
    }

    /* Function-pointer compat: any pointer-to-function is assignable
     * to any other pointer-to-function in C (through an explicit
     * cast, but in practice via implicit conversion in function
     * pointers matching callbacks).  Sharp does not check parameter
     * signatures — we rely on the C compiler to catch mismatches.
     * This covers `Hook h = hookf;` where h is a typedef'd func ptr
     * and hookf is a plain function (decayed to PTR(FUNC)). */
    if (ty_is_pointer(lhs) && ty_is_pointer(rhs)) {
        Type *lb = ty_deref(lhs);
        Type *rb = ty_deref(rhs);
        if (lb && lb->kind == TY_FUNC && rb && rb->kind == TY_FUNC)
            return true;
    }

    /* C 6.3.2.1: an rvalue has the unqualified version of its
     * declared type.  When assigning an rvalue (rhs) into a place
     * whose type is unqualified at the top level (lhs), strip the
     * top-level const from rhs and retry equality.  Without this,
     * const char *s = arr[0] where arr[0] is const char *const
     * fails because the value carries CONST(PTR(CONST(char))) and
     * we expect PTR(CONST(char)) - the inner const-on-pointee is
     * preserved, only the outer const-on-pointer is dropped. */
    if (rhs && rhs->kind == TY_CONST && lhs && lhs->kind != TY_CONST) {
        if (assign_compat(ts, lhs, rhs->u.const_.base)) return true;
    }

    /* null (void*) → any pointer */
    if (ty_is_pointer(lhs) && rhs == ty_ptr(ts, ty_void(ts))) return true;

    /* C null pointer constant: an integer constant expression with value
     * 0 (or such an expression cast to void*) is convertible to any
     * pointer type.  We don't track constness of the value through sema,
     * so we accept any integer→pointer assignment in initialisation
     * contexts where the user is almost certainly writing `T *p = 0;`
     * or `T *p = NULL;`.  This also covers `FILE *f = 0;` patterns
     * common in libc consumers. */
    if (ty_is_pointer(lhs) && ty_is_integer(rhs)) return true;

    return false;
}

/* =========================================================================
 * Forward declarations
 * ====================================================================== */
static Type *sema_expr(SS *ss, AstNode *expr);
static void  sema_stmt(SS *ss, AstNode *stmt);
static void  sema_block(SS *ss, AstNode *block, Scope *block_scope);

/* =========================================================================
 * Expression sema
 * ====================================================================== */

static bool ty_is_struct_value(TyStore *ts, Type *t) {
    Type *u = ty_unconst(ts, t);
    return u && u->kind == TY_STRUCT;
}

static Type *sema_binop(SS *ss, AstNode *expr) {
    SharpTokKind op = expr->u.binop.op;
    Type *lt = sema_expr(ss, expr->u.binop.lhs);
    Type *rt = sema_expr(ss, expr->u.binop.rhs);
    TyStore *ts = ss->ctx->ts;

    /* Assignment family */
    static const SharpTokKind assigns[] = {
        STOK_EQ, STOK_PLUSEQ, STOK_MINUSEQ, STOK_STAREQ,
        STOK_SLASHEQ, STOK_PERCENTEQ, STOK_AMPEQ, STOK_PIPEEQ,
        STOK_CARETEQ, STOK_LTLTEQ, STOK_GTGTEQ
    };
    for (size_t i = 0; i < sizeof assigns / sizeof assigns[0]; i++) {
        if (op == assigns[i]) {
            if (!assign_compat(ts, lt, rt))
                FE_ERROR(ss->ctx->diags, expr->loc,
                    "incompatible types in assignment: cannot assign %s to %s",
                    ty_kind_name(rt->kind), ty_kind_name(lt->kind));
            return lt;
        }
    }

    /* Phase 7: operator overload — if lhs is a struct, check for operator.
     * This must come BEFORE the generic scalar checks so that struct
     * operator== is not mistaken for "comparison of non-scalar types".
     *
     * Important: only direct struct receivers participate in overload
     * dispatch.  `struct S *p` is NOT a struct receiver — `p == NULL`
     * is plain pointer comparison and must fall through to the scalar
     * path.  Without this guard, `p != 0` on a typed pointer would
     * find the struct scope (via struct_scope_of which strips one
     * level of pointer) and report "operator '!=' not defined for
     * struct type" even though the operands are pointers. */
    {
        Type *lt_unconst = ty_unconst(ts, lt);
        bool lhs_is_struct_value = ty_is_struct_value(ts, lt);
        const char *op_nm = lhs_is_struct_value ? op_overload_name(op) : NULL;
        if (op_nm) {
            Scope *ss_s = struct_scope_of(ts, lt, ss->scope);
            if (ss_s) {
                Symbol *osym = scope_lookup_local(ss_s, op_nm);
                if (osym && osym->decl && osym->decl->kind == AST_FUNC_DEF) {
                    Scope *mscope = osym->decl->sem_scope
                                  ? osym->decl->sem_scope : ss->scope;
                    Type *ret = ty_from_ast(ts, osym->decl->u.func_def.ret_type,
                                            mscope, NULL);
                    return sema_subst_for_struct(ts, lt, ret);
                }
                /* Struct method not found — fall through to free-function
                 * lookup below rather than erroring immediately. */
            }
            /* Free-function operator: search file scope for
             *   RetType operator+(LhsType, RhsType) { ... }
             *
             * Overload resolution: when multiple free functions share
             * the same operator name (e.g. operator+(VecC, VecS) and
             * operator+(VecS, VecC)), we must pick the one whose first
             * parameter type matches the LHS struct AND whose second
             * parameter type matches the RHS type.  We iterate through
             * all chained overloads via scope_lookup_next_local() until
             * we find an exact match or exhaust the chain.
             *
             * Only applies when the struct scope exists — opaque types
             * like __typeof__(...) have no struct scope. */
            if (ss_s && ss->ctx && ss->ctx->file_scope) {
                /* Obtain the RHS type for overload disambiguation. */
                Type *rt_unc = ty_unconst(ts, rt);

                for (Symbol *fsym = scope_lookup_local(
                             ss->ctx->file_scope, op_nm);
                     fsym;
                     fsym = scope_lookup_next_local(
                             ss->ctx->file_scope, fsym, op_nm)) {
                    if (!fsym->decl ||
                        fsym->decl->kind != AST_FUNC_DEF ||
                        !fsym->decl->u.func_def.is_operator ||
                        fsym->decl->u.func_def.params.len < 1)
                        continue;

                    /* p0 must match LHS struct type. */
                    AstNode *p0 = fsym->decl->u.func_def.params.data[0];
                    if (!p0 || !p0->u.param_decl.type) continue;
                    Type *p0t = ty_from_ast(ts, p0->u.param_decl.type,
                                            ss->scope, NULL);
                    Type *p0t_unc = ty_unconst(ts, p0t);
                    if (!p0t_unc || p0t_unc->kind != TY_STRUCT) continue;
                    if (!lt_unconst || lt_unconst->kind != TY_STRUCT) continue;
                    if (strcmp(p0t_unc->u.struct_.name,
                               lt_unconst->u.struct_.name) != 0) continue;

                    /* When the operator has a second parameter and the
                     * RHS is also a struct, p1 must match the RHS struct
                     * type to select the correct overload.  When the RHS
                     * is a non-struct scalar (e.g. int, float) the p1
                     * check is skipped — the LHS match is sufficient. */
                    if (fsym->decl->u.func_def.params.len >= 2 &&
                        rt_unc && rt_unc->kind == TY_STRUCT) {
                        AstNode *p1 = fsym->decl->u.func_def.params.data[1];
                        if (p1 && p1->u.param_decl.type) {
                            Type *p1t = ty_from_ast(ts,
                                p1->u.param_decl.type, ss->scope, NULL);
                            Type *p1t_unc = ty_unconst(ts, p1t);
                            if (p1t_unc && p1t_unc->kind == TY_STRUCT &&
                                strcmp(p1t_unc->u.struct_.name,
                                       rt_unc->u.struct_.name) != 0)
                                continue; /* RHS type mismatch */
                        }
                    }

                    /* Matching overload found — return its return type. */
                    Type *ret = ty_from_ast(ts,
                        fsym->decl->u.func_def.ret_type, ss->scope, NULL);
                    return ret ? ret : ty_int(ts);
                }

                /* Extension method fallback: search file scope for
                 * extension operators (struct_name != NULL). */
                if (lt_unconst && lt_unconst->kind == TY_STRUCT &&
                    lt_unconst->u.struct_.name) {
                    const char *sname = lt_unconst->u.struct_.name;
                    Symbol *es = find_extension_method(ss->ctx->file_scope, sname, op_nm);
                    if (es && es->decl && es->decl->kind == AST_FUNC_DEF &&
                        es->decl->u.func_def.is_operator &&
                        es->decl->u.func_def.params.len >= 1) {
                        AstNode *p0 = es->decl->u.func_def.params.data[0];
                        if (p0 && p0->u.param_decl.type) {
                            Scope *mscope = es->decl->sem_scope
                                          ? es->decl->sem_scope : ss->scope;
                            Type *p0t = ty_from_ast(ts, p0->u.param_decl.type,
                                                    mscope, NULL);
                            Type *p0u = ty_peel_to_struct(p0t);
                            if (p0u && strcmp(p0u->u.struct_.name, sname) == 0) {
                                /* RHS match check for binary operators */
                                bool rhs_ok = true;
                                if (es->decl->u.func_def.params.len >= 2 &&
                                    rt_unc && rt_unc->kind == TY_STRUCT) {
                                    AstNode *p1 = es->decl->u.func_def.params.data[1];
                                    if (p1 && p1->u.param_decl.type) {
                                        Type *p1t = ty_from_ast(ts,
                                            p1->u.param_decl.type, mscope, NULL);
                                        Type *p1u = ty_unconst(ts, p1t);
                                        if (p1u && p1u->kind == TY_STRUCT &&
                                            strcmp(p1u->u.struct_.name,
                                                   rt_unc->u.struct_.name) != 0)
                                            rhs_ok = false;
                                    }
                                }
                                if (rhs_ok) {
                                    Type *ret = ty_from_ast(ts,
                                        es->decl->u.func_def.ret_type, mscope, NULL);
                                    if (ret && !ty_is_error(ret) && lt_unconst)
                                        ret = sema_subst_for_struct(ts, lt_unconst, ret);
                                    return ret ? ret : ty_int(ts);
                                }
                            }
                        }
                    }
                }
            }

            /* Struct found but no method or free-function operator. */
            if (ss_s && !ty_is_error(lt))
                FE_ERROR(ss->ctx->diags, expr->loc,
                    "operator '%s' not defined for struct type",
                    op_nm + 8); /* skip "operator" prefix */
            if (ss_s) return ty_error(ts);
        }
    }

    /* Generic type parameter (T in struct Vec<T> methods).
     * Allow all operations on generic params and let C catch errors
     * at instantiation time — same semantics as C++ templates.
     * This must come BEFORE the scalar comparison check so that
     * `T a; T b; a > b;` where T is TY_PARAM does not error. */
    {
        Type *lu_raw = ty_unconst(ts, lt);
        Type *ru_raw = ty_unconst(ts, rt);
        if (lu_raw && lu_raw->kind == TY_PARAM &&
            ru_raw && ru_raw->kind == TY_PARAM &&
            strcmp(lu_raw->u.param.name, ru_raw->u.param.name) == 0) {
            /* Both operands are the same generic param T — allow any op. */
            if (is_comparison_op(op))
                return ty_int(ts);
            if (op == STOK_PLUS || op == STOK_MINUS ||
                op == STOK_STAR || op == STOK_SLASH ||
                op == STOK_PERCENT)
                return lt;
        }
    }

    /* Comparison → int */
    if (is_comparison_op(op)) {
        /* suppress cascade errors — when either operand has an
         * error type (e.g. result of calling a function-pointer variable
         * whose return type could not be resolved by sema), the comparison
         * is still valid C; let cc verify the actual types. */
        if (!ty_is_error(lt) && !ty_is_error(rt) &&
            (!ty_is_scalar(lt) || !ty_is_scalar(rt)))
            FE_ERROR(ss->ctx->diags, expr->loc, "comparison of non-scalar types");
        return ty_int(ts);
    }

    /* Logical → int */
    if (op == STOK_AMPAMP || op == STOK_PIPEPIPE) return ty_int(ts);

    /* Pointer ± integer */
    if ((op == STOK_PLUS || op == STOK_MINUS) &&
        ty_is_pointer(lt) && ty_is_integer(rt)) return lt;
    if (op == STOK_PLUS && ty_is_integer(lt) && ty_is_pointer(rt)) return rt;

    /* Pointer difference */
    if (op == STOK_MINUS && ty_is_pointer(lt) && ty_is_pointer(rt))
        return ty_long(ts);

    /* S1: const-qualified arithmetic types undergo the same usual
     * arithmetic conversions as their unqualified counterparts.  Strip
     * the qualifier before the arithmetic / bitwise checks so that
     * `const int x; x - 5` does not erroneously become a type error. */
    Type *lu = ty_unconst(ts, lt);
    Type *ru = ty_unconst(ts, rt);

    /* Arithmetic */
    if (ty_is_arithmetic(lu) && ty_is_arithmetic(ru))
        return arith_conv(ts, lu, ru);

    /* GCC vector extension: vector + vector, vector + scalar, etc. */
    if (ty_is_vector(lu) && ty_is_vector(ru))
        return lu;
    if (ty_is_vector(lu) && ty_is_arithmetic(ru))
        return lu;
    if (ty_is_arithmetic(lu) && ty_is_vector(ru))
        return ru;

    /* Bitwise on integers */
    if ((op == STOK_AMP || op == STOK_PIPE || op == STOK_CARET ||
         op == STOK_LTLT || op == STOK_GTGT) &&
        ty_is_integer(lu) && ty_is_integer(ru))
        return arith_conv(ts, lu, ru);

    /* Comma */
    if (op == STOK_COMMA) return rt;

    if (!ty_is_error(lt) && !ty_is_error(rt))
        FE_ERROR(ss->ctx->diags, expr->loc, "invalid operand types for binary operator");
    return ty_error(ts);
}

static Type *sema_unary(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    Type *ot = sema_expr(ss, expr->u.unary.operand);
    SharpTokKind op = expr->u.unary.op;

    if (op == STOK_BANG)  return ty_int(ts);
    if (op == STOK_TILDE) return (ty_is_integer(ot) || ty_is_vector(ot)) ? ot : ty_int(ts);
    if (op == STOK_MINUS || op == STOK_PLUS) {
        if (ty_is_vector(ot)) return ot;
        /* Check for unary operator overload on struct type (spec §运算符重载).
         * Lookup order: (1) struct method operator-() (2) free operator-(T). */
        if (ty_is_struct_value(ts, ot)) {
            Type *ot_unc = ty_unconst(ts, ot);
            const char *op_nm = (op == STOK_MINUS) ? "operator-" : "operator+";
            /* (1) struct scope */
            Scope *ss_s = struct_scope_of(ts, ot, ss->scope);
            if (ss_s) {
                Symbol *osym = scope_lookup_local(ss_s, op_nm);
                if (osym && osym->decl && osym->decl->kind == AST_FUNC_DEF) {
                    Scope *mscope = osym->decl->sem_scope
                                  ? osym->decl->sem_scope : ss->scope;
                    Type *ret = ty_from_ast(ts, osym->decl->u.func_def.ret_type,
                                            mscope, NULL);
                    return sema_subst_for_struct(ts, ot_unc, ret);
                }
            }
            /* (2) free function in file scope */
            if (ss->ctx && ss->ctx->file_scope) {
                Symbol *fsym = scope_lookup(ss->ctx->file_scope, op_nm);
                if (fsym && fsym->decl &&
                    fsym->decl->kind == AST_FUNC_DEF &&
                    fsym->decl->u.func_def.is_operator &&
                    fsym->decl->u.func_def.params.len == 1) {
                    AstNode *p0 = fsym->decl->u.func_def.params.data[0];
                    if (p0 && p0->u.param_decl.type) {
                        Type *p0t = ty_from_ast(ts, p0->u.param_decl.type,
                                                ss->scope, NULL);
                        Type *p0t_unc = ty_unconst(ts, p0t);
                        if (p0t_unc && p0t_unc->kind == TY_STRUCT &&
                            ot_unc->kind == TY_STRUCT &&
                            strcmp(p0t_unc->u.struct_.name,
                                   ot_unc->u.struct_.name) == 0) {
                            Type *ret = ty_from_ast(ts,
                                fsym->decl->u.func_def.ret_type,
                                ss->scope, NULL);
                            return ret ? ret : ty_int(ts);
                        }
                    }
                }
                /* (3) extension method operator in file scope
                 *     (e.g. `Vec2 Vec2.operator-(this) { ... }`). */
                const char *sname = ot_unc->u.struct_.name;
                Symbol *es = find_extension_method(ss->ctx->file_scope, sname, op_nm);
                if (es && es->decl && es->decl->kind == AST_FUNC_DEF &&
                    es->decl->u.func_def.is_operator) {
                    Scope *mscope = es->decl->sem_scope
                                  ? es->decl->sem_scope : ss->scope;
                    Type *ret = ty_from_ast(ts,
                        es->decl->u.func_def.ret_type, mscope, NULL);
                    return sema_subst_for_struct(ts, ot_unc, ret);
                }
            }
        }
        if (!ty_is_arithmetic(ot))
            FE_ERROR(ss->ctx->diags, expr->loc, "unary arithmetic on non-arithmetic type");
        return ty_is_arithmetic(ot) ? ot : ty_int(ts);
    }
    if (op == STOK_AMP) {
        /* C 6.3.2.1 ¶4: applying & to a function designator yields a
         * pointer to the function — identical to the implicit decay.
         * In Sharp's type model, SYM_FUNC idents already return
         * PTR(FUNC), so `&io_fclose` must NOT add another PTR layer.
         * For regular lvalues (variables, fields) the normal PTR wrap
         * is correct. */
        if (ty_is_func_ptr(ot)) {
            return ot;   /* already PTR(FUNC) — address-of function is a no-op */
        }
        return ty_ptr(ts, ot);   /* & lvalue → T* */
    }
    if (op == STOK_STAR) {
        if (!ty_is_pointer(ot)) {
            FE_ERROR(ss->ctx->diags, expr->loc, "dereference of non-pointer");
            return ty_error(ts);
        }
        return ty_deref(ot);
    }
    if (op == STOK_PLUSPLUS || op == STOK_MINUSMINUS) {
        if (!ty_is_scalar(ot))
            FE_ERROR(ss->ctx->diags, expr->loc, "++ / -- on non-scalar");
        return ot;
    }
    return ot;
}

static void sema_expr_vec(SS *ss, AstVec *vec) {
    for (size_t i = 0; i < vec->len; i++)
        sema_expr(ss, vec->data[i]);
}

static Type *sema_call(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    /* Evaluate callee to determine function type. */
    Type *callee_t = sema_expr(ss, expr->u.call.callee);
    /* Evaluate all arguments (type-check them even if we can't verify). */
    sema_expr_vec(ss, &expr->u.call.args);

    /* GCC builtins (`__builtin_expect`, etc.).  These are not declared
     * in source; sema_expr's AST_IDENT path returns int for the callee
     * but the call must also resolve to int (or whatever the builtin's
     * signature says) — we collapse them all to int, matching the
     * common cases used by Lua's `luai_likely`/`luai_unlikely` and
     * by glibc's `__builtin_offsetof`, etc.  Real signatures are
     * resolved by the C compiler when it processes the generated C.
     * extend to __atomic_* and __sync_* (GCC atomic intrinsics
     * used by sqlite3's AtomicLoad/AtomicStore macros).  Without this,
     * sema_call returns ty_error for these undeclared builtins, which
     * causes spurious "if condition must be scalar" errors. */
    if (expr->u.call.callee->kind == AST_IDENT) {
        const char *bname = expr->u.call.callee->u.ident.name;
        if (is_gcc_builtin_name(bname)) {
            return ty_int(ts);
        }
        /* POSIX/GNU extension functions not declared without
         * _GNU_SOURCE.  Rather than injecting headers, accept these as
         * returning void* (pointer to memory).  The C compiler resolves
         * the real signature when it compiles the generated C. */
        if (strcmp(bname, "memrchr") == 0)
            return ty_ptr(ts, ty_void(ts));
    }

    /* If callee is a FUNC type, return its return type.
     * But first do arg-count check if we have the symbol's AST decl. */
    if (expr->u.call.callee->kind == AST_IDENT) {
        Symbol *sym2 = scope_lookup(ss->scope, expr->u.call.callee->u.ident.name);
        if (sym2 && sym2->kind == SYM_FUNC && sym2->decl &&
            sym2->decl->kind == AST_FUNC_DEF) {
            AstNode *fn2 = sym2->decl;
            if (fn2->u.func_def.generic_params.len == 0) {
                bool va = false;
                size_t np = fn2->u.func_def.params.len;
                for (size_t pi = 0; pi < np; pi++) {
                    AstNode *par = fn2->u.func_def.params.data[pi];
                    if (par && par->u.param_decl.is_vararg) { va = true; break; }
                }
                size_t na = expr->u.call.args.len;
                if (!va && np > 0 && na != np)
                    fe_emit_diag(ss->ctx->diags, CPP_DIAG_ERROR, expr->loc,
                        "function '%s' expects %zu argument(s), got %zu",
                        expr->u.call.callee->u.ident.name, np, na);
            }
        }
    }
    if (callee_t && callee_t->kind == TY_FUNC) return callee_t->u.func.ret;
    /* calling through a function pointer: TY_PTR(TY_FUNC(ret,...)).
     * Strip one layer of TY_PTR (and optional TY_CONST) to reach TY_FUNC. */
    {
        Type *inner = callee_t;
        inner = ty_unconst(ts, inner);
        if (inner && inner->kind == TY_PTR)    inner = inner->u.ptr.base;
        inner = ty_unconst(ts, inner);
        if (inner && inner->kind == TY_FUNC)   return inner->u.func.ret;
    }
    /* For identifiers resolved to a SYM_FUNC decl, we can look up the ret type
     * and check argument count. */
    if (expr->u.call.callee->kind == AST_IDENT) {
        Symbol *sym = scope_lookup(ss->scope,
                                   expr->u.call.callee->u.ident.name);
        if (sym && sym->kind == SYM_FUNC && sym->decl &&
            sym->decl->kind == AST_FUNC_DEF) {
            AstNode *fn = sym->decl;
            size_t ngp = fn->u.func_def.generic_params.len;
            if (ngp > 0) {
                /* Generic function: return type may contain TY_PARAMs.
                 * Attempt simple inference from first argument types. */
                Scope *fscope = fn->sem_scope ? fn->sem_scope : ss->scope;
                Type *ret_raw = ty_from_ast(ts, fn->u.func_def.ret_type,
                                            fscope, NULL);
                if (ret_raw && ret_raw->kind == TY_PARAM) {
                    /* Find the param that matches the return type name. */
                    for (size_t pi = 0;
                         pi < fn->u.func_def.params.len && pi < expr->u.call.args.len;
                         pi++) {
                        AstNode *par = fn->u.func_def.params.data[pi];
                        if (!par) continue;
                        Scope *ps = fn->sem_scope ? fn->sem_scope : ss->scope;
                        Type *par_t = ty_from_ast(ts, par->u.param_decl.type, ps, NULL);
                        if (par_t && par_t->kind == TY_PARAM &&
                            strcmp(par_t->u.param.name,
                                   ret_raw->u.param.name) == 0) {
                            /* Param matches return type param: use actual arg type. */
                            return sema_expr(ss, expr->u.call.args.data[pi]);
                        }
                    }
                }
                /* Fallback: just evaluate return type as-is */
                return ret_raw;
            }
            return ty_from_ast(ts, fn->u.func_def.ret_type, ss->scope, ss->ctx->diags);
        }
    }
    /* when callee type is void* (sema_warn path for undefined
     * names), treat the call as returning int — the real type is resolved
     * by cc from the generated C.  This prevents spurious "if condition
     * must be scalar" errors when calling functions declared only in
     * headers that sema didn't see (e.g. EVUTIL_ISXDIGIT_ in libevent). */
    if (callee_t && callee_t->kind == TY_PTR &&
        callee_t->u.ptr.base && callee_t->u.ptr.base->kind == TY_VOID) {
        return ty_int(ts);
    }
    return ty_error(ts);  /* return type unknown — suppress cascade */
}

static Type *sema_expr(SS *ss, AstNode *expr) {
    if (!expr) return ty_error(ss->ctx->ts);
    TyStore *ts = ss->ctx->ts;
    Type *t = NULL;

    switch (expr->kind) {
    case AST_INT_LIT: {
        /* Determine type from suffix first, then value range.
         * C rules: LL → long long, L → long, else pick smallest. */
        if (expr->u.int_lit.is_longlong) {
            t = expr->u.int_lit.is_unsigned ? ty_ulonglong(ts) : ty_longlong(ts);
        } else if (expr->u.int_lit.is_long) {
            t = expr->u.int_lit.is_unsigned ? ty_ulong(ts) : ty_long(ts);
        } else if (expr->u.int_lit.is_unsigned) {
            t = ty_uint(ts);
        } else {
            int64_t v = expr->u.int_lit.val;
            if (v <= 2147483647LL) t = ty_int(ts);
            else if (v <= 9223372036854775807LL) t = ty_long(ts);
            else t = ty_longlong(ts);
        }
        break;
    }
    case AST_FLOAT_LIT:  t = ty_double(ts); break;
    case AST_CHAR_LIT:   t = ty_int(ts);    break;  /* char literal is int */
    case AST_STRING_LIT:
        t = ty_ptr(ts, ty_const(ts, ty_char(ts))); break;
    /* Phase S5: GCC labels-as-values.  `&&label` has type `void *`.
     * The label name is not resolved here (it refers to a goto-target,
     * not a value-binding); cc validates that the label exists in the
     * enclosing function. */
    case AST_ADDR_OF_LABEL:
        t = ty_ptr(ts, ty_void(ts)); break;

    case AST_IDENT: {
        const char *name = expr->u.ident.name;
        /* 'this' is a special param — let scope lookup find it. */
        Symbol *sym = scope_lookup_value(ss->scope, name);
        if (!sym) {
            /* GCC builtins (`__builtin_expect`, `__builtin_offsetof`,
             * `__builtin_memcpy`, etc.) and pseudo-builtins from the
             * preprocessor are never declared explicitly in C source.
             * Treat any name starting with `__builtin_` as a callable
             * external — its type collapses to int (the C compiler
             * will resolve the real signature when compiling the
             * generated C output).  Lua's luaconf.h uses
             * `__builtin_expect` in luai_likely / luai_unlikely.
             * also accept GCC atomic builtins (__atomic_load_n,
             * __atomic_store_n, __atomic_compare_exchange_n, etc.) and
             * legacy sync builtins (__sync_fetch_and_add, etc.).  They are
             * never declared explicitly; cc resolves the real semantics. */
            /* __builtin_va_arg(ap, TYPE) is stored as a
             * verbatim AST_IDENT by parse.c.  Return the type arg so
             * that `*va_arg(ap, int*)` does not produce "dereference of
             * non-pointer".  If the type string ends with `*` we return
             * a pointer type (int* as a safe stand-in); otherwise int. */
            if (strncmp(name, "__builtin_va_arg(", 17) == 0) {
                const char *comma = strrchr(name, ',');
                bool is_ptr = false;
                if (comma) {
                    const char *p2 = name + strlen(name) - 1;
                    while (p2 > comma && (*p2 == ')' || *p2 == ' ')) p2--;
                    if (*p2 == '*') is_ptr = true;
                }
                if (is_ptr) { t = ty_ptr(ts, ty_int(ts)); break; }
                /* For struct/union args return proper struct type so that
                 * `struct T x = va_arg(ap, struct T)` passes type checks. */
                if (comma) {
                    const char *ts2 = comma + 1;
                    while (*ts2 == ' ') ts2++;
                    const char *te = name + strlen(name) - 1;
                    while (te > ts2 && *te == ')') te--;
                    while (te > ts2 && *te == ' ') te--;
                    size_t tlen = (size_t)(te - ts2 + 1);
                    if (tlen > 7 &&
                        (strncmp(ts2, "struct ", 7) == 0 ||
                         strncmp(ts2, "union ",  6) == 0)) {
                        size_t kw = (ts2[0] == 'u') ? 6 : 7;
                        const char *tag = ts2 + kw;
                        size_t tag_len = tlen - kw;
                        while (tag_len && *tag == ' ') { tag++; tag_len--; }
                        if (tag_len > 0) {
                            char tbuf[128];
                            if (tag_len >= sizeof tbuf) tag_len = sizeof tbuf - 1;
                            memcpy(tbuf, tag, tag_len); tbuf[tag_len] = '\0';
                            t = ty_struct_type(ts, tbuf, NULL, 0, NULL);
                            break;
                        }
                    }
                }
                t = ty_int(ts);
                break;
            }
            if (is_gcc_builtin_name(name)) {
                /* __builtin_convertvector returns a vector type.
                 * The captured text is "__builtin_convertvector(expr, TypeName)".
                 * Try to resolve the TypeName to a TY_VECTOR for proper type
                 * checking in subsequent expressions. */
                if (strncmp(name, "__builtin_convertvector(", 24) == 0) {
                    const char *comma = strrchr(name, ',');
                    if (comma) {
                        const char *tn = comma + 1;
                        while (*tn == ' ') tn++;
                        const char *te = name + strlen(name) - 1;
                        while (te > tn && *te == ')') te--;
                        while (te > tn && *te == ' ') te--;
                        size_t tlen = (size_t)(te - tn + 1);
                        if (tlen > 0) {
                            char tbuf[128];
                            if (tlen >= sizeof tbuf) tlen = sizeof tbuf - 1;
                            memcpy(tbuf, tn, tlen); tbuf[tlen] = '\0';
                            Symbol *vsym = scope_lookup_type(ss->scope, tbuf);
                            if (vsym && vsym->decl &&
                                vsym->decl->kind == AST_TYPEDEF_DECL) {
                                t = ty_from_ast(ts, vsym->decl->u.typedef_decl.target,
                                                ss->scope, NULL);
                                if (!ty_is_vector(t))
                                    t = ty_vector_type(ts, t, 4, tbuf);
                                break;
                            }
                            t = ty_vector_type(ts, ty_float(ts), 4, tbuf);
                            break;
                        }
                    }
                }
                t = ty_int(ts);
                break;
            }
            /* GCC magic string variables — __PRETTY_FUNCTION__
             * (function name + signature, GCC extension) and __FUNCTION__
             * (alias for __func__, MSVC/GCC).  Both evaluate to a
             * NUL-terminated string literal with type `const char *`.
             * They are predefined per-function identifiers, not macros,
             * so cpp does not expand them; sema must accept them. */
            if (strcmp(name, "__PRETTY_FUNCTION__") == 0 ||
                strcmp(name, "__FUNCTION__") == 0) {
                t = ty_ptr(ts, ty_const(ts, ty_char(ts)));
                break;
            }
            /* POSIX/GNU extension functions not declared without
             * _GNU_SOURCE (e.g. memrchr).  Treat as returning void* so cg
             * emits the call verbatim and cc resolves the real signature. */
            if (strcmp(name, "memrchr") == 0) {
                t = ty_ptr(ts, ty_void(ts));
                break;
            }
            /* undefined name — emit a warning (not an error) and
             * return void* so code generation can still proceed.  When a
             * function-like macro renames a C library function (e.g.
             * #define strdup(p) Curl_cstrdup(p)), the original name used as
             * an rvalue isn't found in scope.  void* lets cast expressions
             * like (FnType)strdup type-check; cc resolves the actual name. */
            FE_WARNING(ss->ctx->diags, expr->loc, "undefined name '%s'", name);
            t = ty_ptr(ts, ty_void(ts));
        } else {
            AstNode *decl = sym->decl;
            switch (sym->kind) {
            case SYM_VAR:
                /* S2: SYM_VAR can resolve to either a real AST_VAR_DECL
                 * (regular variable) or AST_ENUMERATOR (an enum constant
                 * registered as SYM_VAR for lookup convenience).  An
                 * enumerator has type int. */
                if (decl && decl->kind == AST_ENUMERATOR) {
                    t = ty_int(ts);
                    break;
                }
                /* sem_type was set when we processed the VAR_DECL */
                t = decl ? decl->sem_type : ty_error(ts);
                if (!t) t = decl ? ty_from_ast(ts, decl->u.var_decl.type,
                                                ss->scope, ss->ctx->diags)
                                 : ty_error(ts);
                /* C 6.3.2.1: a function designator is converted to a
                 * pointer to the function in expression contexts.  When
                 * a variable's type resolves to TY_FUNC (e.g. via a
                 * typedef like `typedef int F(int); F fn;`), it decays
                 * to TY_PTR(TY_FUNC(...)) when used in an expression. */
                if (t && t->kind == TY_FUNC)
                    t = ty_ptr(ts, t);
                break;
            case SYM_PARAM:
                if (decl && decl->kind == AST_PARAM_DECL) {
                    t = ty_from_ast(ts, decl->u.param_decl.type,
                                    ss->scope, ss->ctx->diags);
                    /* C 6.7.6.3p8: a parameter declared with function type
                     * is adjusted to pointer-to-function.  E.g.
                     * `typedef int F(int); void g(F ntf)` makes ntf's
                     * type `int (*)(int)`, not `int(int)`. */
                    if (t && t->kind == TY_FUNC)
                        t = ty_ptr(ts, t);
                } else if (strcmp(expr->u.ident.name, "this") == 0) {
                    /* 'this' in a struct method: ptr to enclosing struct.
                     * Phase 4 stores the FUNC_DEF as decl for 'this', so
                     * we derive its type from the enclosing struct scope.
                     *
                     * v0.13: for generic struct methods (e.g. HashMap<K,V>),
                     * attach TY_PARAM args to `this`'s struct type so that
                     * cg-time substitution can produce the correct mangled
                     * struct name (HashMap__Point__int rather than just
                     * HashMap). */
                    Scope *ss_s = ss->scope;
                    while (ss_s && ss_s->kind != SCOPE_STRUCT) ss_s = ss_s->parent;
                    if (ss_s && ss_s->owner && ss_s->owner->kind == AST_STRUCT_DEF) {
                        const char *sn = ss_s->owner->u.struct_def.name;
                        size_t np = ss_s->owner->u.struct_def.generic_params.len;
                        Type **args = NULL;
                        if (np > 0) {
                            args = malloc(np * sizeof *args);
                            if (!args) abort();
                            for (size_t i = 0; i < np; i++) {
                                AstNode *gp =
                                    ss_s->owner->u.struct_def.generic_params.data[i];
                                args[i] = ty_param(ts, gp->u.generic_param.name);
                            }
                        }
                        Type *st = ty_struct_type(ts, sn, args, np, ss_s->owner);
                        free(args);
                        /* Is this a const method? Look up the enclosing FUNC scope. */
                        Scope *fsc = ss->scope;
                        while (fsc && fsc->kind != SCOPE_FUNC) fsc = fsc->parent;
                        bool is_cm = fsc && fsc->owner &&
                                     fsc->owner->kind == AST_FUNC_DEF &&
                                     fsc->owner->u.func_def.is_const_method;
                        t = is_cm ? ty_ptr(ts, ty_const(ts, st))
                                  : ty_ptr(ts, st);
                    } else {
                        t = ty_error(ts);
                    }
                } else {
                    t = ty_error(ts);
                }
                break;
            case SYM_FUNC: {
                /* C 6.3.2.1: a function designator is converted to a
                 * pointer to the function whenever it appears in an
                 * expression context that is not the operand of & or
                 * a call.  Return the func's TYPE wrapped in PTR so
                 * comparisons (`hook != hookf`) and assignments
                 * (`lua_Hook h = hookf`) work correctly.  For pure
                 * call sites the type is ignored; for comparison /
                 * assignment sema needs a pointer-to-function. */
                AstNode *fn_decl = sym->decl;
                if (fn_decl && fn_decl->kind == AST_FUNC_DEF) {
                    /* Build PTR(FUNC(ret, params...)) from the decl.
                     * Simplify to PTR(void) when ret type is unavailable
                     * — the C compiler will validate the real signature. */
                    Type *ret = fn_decl->u.func_def.ret_type
                        ? ty_from_ast(ts, fn_decl->u.func_def.ret_type,
                                      ss->scope, NULL)
                        : ty_void(ts);
                    Type *fn = ty_func(ts, ret, NULL, 0);
                    t = ty_ptr(ts, fn);
                } else {
                    t = ty_ptr(ts, ty_void(ts));  /* generic fn ptr */
                }
                break;
            }
            case SYM_BUILTIN:
            case SYM_TYPE:
                /* Type names used as expressions are errors. */
                FE_ERROR(ss->ctx->diags, expr->loc, "'%s' is a type, not a value", name);
                t = ty_error(ts);
                break;
            case SYM_FIELD: {
                /* Plain field name used inside a struct method body --
                 * implicit `this->field` access.  Resolve the field type. */
                AstNode *fd = sym->decl;
                if (fd && fd->kind == AST_FIELD_DECL && fd->u.field_decl.type) {
                    t = ty_from_ast(ts, fd->u.field_decl.type,
                                    ss->scope, ss->ctx->diags);
                }
                if (!t) t = ty_int(ts);
                break;
            }
            default:
                t = ty_error(ts);
                break;
            }
        }
        break;
    }

    case AST_BINOP:   t = sema_binop(ss, expr); break;
    case AST_UNARY:   t = sema_unary(ss, expr); break;

    case AST_TERNARY: {
        Type *cond_t = sema_expr(ss, expr->u.ternary.cond);
        sema_require_scalar(ss, expr, cond_t, "ternary");
        Type *a = sema_expr(ss, expr->u.ternary.then_);
        Type *b = sema_expr(ss, expr->u.ternary.else_);
        if (ty_is_arithmetic(a) && ty_is_arithmetic(b))
            t = arith_conv(ts, a, b);
        else t = a;
        break;
    }

    case AST_COMMA: {
        sema_expr(ss, expr->u.comma.lhs);
        t = sema_expr(ss, expr->u.comma.rhs);
        break;
    }

    case AST_CALL:  t = sema_call(ss, expr); break;

    /* Phase G: explicit generic function call  func<int>(args).
     * Walk value arguments so their sem_type/sem_scope fields are set (needed for
     * type inference in cg_collect_gfunc_call).  Resolve the return type
     * by looking up the function symbol, getting its declared ret_type, and
     * substituting TY_PARAM occurrences with the explicit type args. */
    case AST_GENERIC_CALL: {
        sema_expr_vec(ss, &expr->u.generic_call.call_args);

        const char *fname = expr->u.generic_call.name;
        Symbol *gsym = fname ? scope_lookup(ss->scope, fname) : NULL;
        if (gsym && gsym->kind == SYM_FUNC && gsym->decl &&
            gsym->decl->kind == AST_FUNC_DEF) {
            const AstNode *fn  = gsym->decl;
            size_t ngp         = fn->u.func_def.generic_params.len;
            Scope *fscope      = fn->sem_scope
                                 ? fn->sem_scope : ss->scope;
            Type *ret_raw = ty_from_ast(ts, fn->u.func_def.ret_type,
                                        fscope, NULL);
            /* Substitute explicit type args (e.g. <int>) into TY_PARAMs.
             * Only perform substitution when ALL generic params have explicit
             * type args.  Partial substitution leaves unbound TY_PARAMs which
             * cause downstream errors (e.g. "cannot initialize 'ptr' with value
             * of type 'param'").  When args are partial, fall through to the
             * int placeholder — cg will infer the missing types from actual
             * call arguments. */
            if (ret_raw && ngp > 0) {
                size_t nta  = expr->u.generic_call.type_args.len;
                if (nta == ngp) {
                    const char **pnames = malloc(nta * sizeof *pnames);
                    Type       **pvals  = malloc(nta * sizeof *pvals);
                    if (pnames && pvals) {
                        for (size_t _k = 0; _k < nta; _k++) {
                            pnames[_k] = fn->u.func_def.generic_params
                                           .data[_k]->u.generic_param.name;
                            pvals[_k]  = ty_from_ast(ts,
                                expr->u.generic_call.type_args.data[_k],
                                fscope, NULL);
                        }
                        ret_raw = ty_subst(ts, ret_raw, pnames, pvals, nta);
                    }
                    free(pnames); free(pvals);
                    if (ret_raw) { t = ret_raw; break; }
                }
                /* Partial type args — ret_raw still contains unbound TY_PARAMs.
                 * Don't use it; fall through to int placeholder so cg can infer. */
                ret_raw = NULL;
            }
            if (ret_raw) { t = ret_raw; break; }
        }
        /* Fallback: int placeholder (sema is lenient for generics) */
        t = ty_int(ts);
        break;
    }

    case AST_METHOD_CALL:
        t = sema_method_call_expr(ss, expr);
        break;

    case AST_FIELD_ACCESS:
        t = sema_field_access_expr(ss, expr);
        break;

    case AST_INDEX: {
        Type *base_t = sema_expr(ss, expr->u.index_.base);
        sema_expr(ss, expr->u.index_.index);
        if (ty_is_pointer(base_t)) {
            t = ty_deref(base_t);
        } else if (ty_is_vector(base_t)) {
            t = base_t->u.vector.elem;
        } else {
            /* Phase 7: check for operator[] overload. */
            Scope *ss_s = struct_scope_of(ts, base_t, ss->ctx->file_scope);
            Symbol *osym = ss_s ? scope_lookup_local(ss_s, "operator[]") : NULL;
            if (osym && osym->decl && osym->decl->kind == AST_FUNC_DEF) {
                /* Resolve return type in struct scope (for generic param T).
                 * Pass NULL diags to suppress "unknown 'T'" for generic params,
                 * same as sema_field_access_expr does. */
                t = ty_from_ast(ts, osym->decl->u.func_def.ret_type,
                                ss_s, NULL);
                /* Substitute generic params if receiver is concrete instantiation. */
                if (!ty_is_error(t) && !ty_is_error(base_t))
                    t = sema_subst_for_struct(ts, base_t, t);
            } else {
                /* Extension method fallback: search file scope for
                 * `RetType StructName.operator[](this, ...) { }`. */
                Symbol *eosym = NULL;
                {
                    Type *peel = ty_peel_to_struct(base_t);
                    if (peel && ss->ctx->file_scope) {
                        Symbol *es = find_extension_method(ss->ctx->file_scope,
                            peel->u.struct_.name, "operator[]");
                        if (es && es->decl && es->decl->kind == AST_FUNC_DEF &&
                            es->decl->u.func_def.is_operator)
                            eosym = es;
                    }
                }
                if (eosym && eosym->decl && eosym->decl->kind == AST_FUNC_DEF) {
                    Scope *mscope = eosym->decl->sem_scope ? eosym->decl->sem_scope : ss->scope;
                    t = ty_from_ast(ts, eosym->decl->u.func_def.ret_type, mscope, NULL);
                    if (!ty_is_error(t) && !ty_is_error(base_t))
                        t = sema_subst_for_struct(ts, base_t, t);
                } else {
                    if (!ty_is_error(base_t))
                        FE_ERROR(ss->ctx->diags, expr->loc, "subscript of non-pointer / non-indexable type");
                    t = ty_error(ts);
                }
            }
        }
        break;
    }

    case AST_CAST:
        /* A CAST node with NULL operand is the parser-generated carrier for
         * a generic TYPE EXPRESSION used as a method receiver, e.g.:
         *   Vec<int>.new()  →  METHOD_CALL{ recv: CAST{TYPE_GENERIC,NULL}, … }
         * (Phase G: func<int>(args) now produces AST_GENERIC_CALL instead.)
         * Skip type resolution on the NULL operand to avoid a crash; the
         * real result type comes from the enclosing METHOD_CALL sema path. */
        if (expr->u.cast.operand == NULL &&
            expr->u.cast.type &&
            expr->u.cast.type->kind == AST_TYPE_GENERIC) {
            t = ty_int(ts);  /* placeholder; METHOD_CALL path handles type */
            break;
        }
        sema_expr(ss, expr->u.cast.operand);
        t = ty_from_ast(ts, expr->u.cast.type, ss->scope, ss->ctx->diags);
        break;

    case AST_COMPOUND_LIT:
        t = ty_from_ast(ts, expr->u.compound_lit.type, ss->scope, ss->ctx->diags);
        if (expr->u.compound_lit.init) {
            AstNode *init_root = expr->u.compound_lit.init;
            if (init_root->kind == AST_INIT_LIST) {
                for (size_t i = 0; i < init_root->u.init_list.items.len; i++) {
                    AstNode *item = init_root->u.init_list.items.data[i];
                    if (!item) continue;
                    if (item->kind == AST_DESIGNATED_INIT)
                        sema_expr(ss, item->u.designated_init.value);
                    else
                        sema_expr(ss, item);
                }
            }
        }
        break;

    case AST_PAREN:   /* C8: transparent — same type as inner */
        sema_expr(ss, expr->u.paren.inner);
        t = (Type*)sema_type_of(expr->u.paren.inner);
        break;

    case AST_SIZEOF:
        if (expr->u.sizeof_.is_type)
            ty_from_ast(ts, expr->u.sizeof_.operand, ss->scope, ss->ctx->diags);
        else
            sema_expr(ss, expr->u.sizeof_.operand);
        t = ty_long(ts);
        break;

    case AST_STRUCT_LIT:
        t = ty_from_ast(ts, expr->u.struct_lit.type, ss->scope, ss->ctx->diags);
        for (size_t i = 0; i < expr->u.struct_lit.field_vals.len; i++)
            sema_expr(ss, expr->u.struct_lit.field_vals.data[i]);
        break;

    case AST_AT_INTRINSIC: {
        /* Phase 10: evaluate @intrinsic expressions.
         * NOTE: @intrinsic arguments have special semantics (type names,
         * operator symbols) — we do NOT call sema_expr on them generically.
         * Instead each intrinsic handles its own argument evaluation. */
        const char *iname = expr->u.at_intrinsic.name;

        if (strcmp(iname, "static_assert") == 0) {
            /* @static_assert(cond, "msg")
             * Evaluate condition via compile-time folder. */
            int result = eval_intrinsic(ss, expr);
            if (result == 0) {
                /* Compile-time false → error. */
                const char *msg = "condition is false";
                if (expr->u.at_intrinsic.args.len >= 2) {
                    AstNode *msga = expr->u.at_intrinsic.args.data[1];
                    if (msga->kind == AST_STRING_LIT && msga->u.string_lit.len > 2) {
                        /* strip quotes: text+1, len-2 */
                        static char msgbuf[256];
                        size_t copylen = msga->u.string_lit.len - 2;
                        if (copylen >= sizeof msgbuf) copylen = sizeof msgbuf - 1;
                        memcpy(msgbuf, msga->u.string_lit.text + 1, copylen);
                        msgbuf[copylen] = '\0';
                        msg = msgbuf;
                    }
                }
                FE_ERROR(ss->ctx->diags, expr->loc, "@static_assert: %s", msg);
            }
            /* If result == -1, condition is non-constant → no compile-time error. */
            t = ty_void(ts);
        } else if (strcmp(iname, "has_operator")  == 0 ||
                   strcmp(iname, "has_method")    == 0 ||
                   strcmp(iname, "has_field")     == 0 ||
                   strcmp(iname, "is_arithmetic") == 0 ||
                   strcmp(iname, "is_pointer")    == 0) {
            /* All bool query intrinsics: type is bool; we also evaluate
             * eagerly here so cg.c can emit the literal 0 or 1.
             * If eval returns -1 (e.g. inside a generic template body
             * where the type param is unresolved) we leave resolved_set
             * = false; cg will emit a placeholder 0. */
            t = ty_bool(ts);
            int v = eval_intrinsic(ss, expr);
            if (v == 0 || v == 1) {
                expr->u.at_intrinsic.resolved_val = v;
                expr->u.at_intrinsic.resolved_set = true;
            }
        } else if (strcmp(iname, "type_name") == 0) {
            /* @type_name(T): returns const char*.  The string is a heap-
             * allocated mangled name; cg emits it as a literal. */
            if (expr->u.at_intrinsic.args.len != 1) {
                FE_ERROR(ss->ctx->diags, expr->loc,
                    "@type_name requires exactly 1 argument, got %zu",
                    expr->u.at_intrinsic.args.len);
                t = ty_error(ts);
            } else {
                char *nm = compute_type_name(ss, expr);
                if (nm) {
                    expr->u.at_intrinsic.resolved_str = nm;
                    expr->u.at_intrinsic.resolved_set = true;
                }
                t = ty_ptr(ts, ty_const(ts, ty_char(ts)));
            }
        } else if (strcmp(iname, "to_cstr") == 0) {
            /* @to_cstr(expr): convert sp_str_t slice to NUL-terminated C string.
             * Requires exactly 1 argument. Returns const char*. */
            if (expr->u.at_intrinsic.args.len != 1) {
                FE_ERROR(ss->ctx->diags, expr->loc,
                    "@to_cstr requires exactly 1 argument, got %zu",
                    expr->u.at_intrinsic.args.len);
                t = ty_error(ts);
            } else {
                sema_expr(ss, expr->u.at_intrinsic.args.data[0]);
                t = ty_ptr(ts, ty_const(ts, ty_char(ts)));
            }
        } else {
            FE_ERROR(ss->ctx->diags, expr->loc, "unknown @intrinsic '%s'", iname);
            t = ty_error(ts);
        }
        break;
    }

    case AST_STMT_EXPR: {
        /* GCC statement-expression ({ stmts; expr; }).
         *
         * The block inside the stmt_expr was not visited by scope_build
         * (scope_build only walks statement-level trees, not expressions).
         * We therefore create a fresh SCOPE_BLOCK here, register any
         * DECL_STMT declarations into it, then walk the block with sema
         * using that scope.
         *
         * This is the "lazy scope creation" pattern: scope_build is the
         * normal path, but for expression-level blocks we build the scope
         * on demand inside sema.  The created scope is owned by the block
         * node (stored in sem_scope, freed by scope_free_chain when the
         * parent scope is freed).
         *
         * Type of the whole expression = type of the last
         * expression-statement, or void if the block is empty / ends
         * with a non-expression statement. */
        AstNode *block = expr->u.stmt_expr.block;
        t = ty_void(ts);
        if (!block) break;

        /* Create and attach a block-level scope if not already set. */
        Scope *blk_scope = block->sem_scope
                         ? block->sem_scope
                         : scope_new(SCOPE_BLOCK, ss->scope, block);
        if (!block->sem_scope) {
            block->sem_scope = blk_scope;
            /* Register every DECL_STMT variable declaration into the new
             * scope so that later sema_expr calls can find them. */
            for (size_t i = 0; i < block->u.block.stmts.len; i++) {
                AstNode *s = block->u.block.stmts.data[i];
                if (!s || s->kind != AST_DECL_STMT) continue;
                AstNode *vd = s->u.decl_stmt.decl;
                if (!vd || vd->kind != AST_VAR_DECL) continue;
                if (vd->u.var_decl.name)
                    scope_define(blk_scope, SYM_VAR,
                                 vd->u.var_decl.name, vd,
                                 ss->ctx->diags);
            }
        }

        SS inner = *ss;
        inner.scope = blk_scope;
        size_t n_stmts = block->u.block.stmts.len;
        for (size_t i = 0; i < n_stmts; i++) {
            AstNode *s = block->u.block.stmts.data[i];
            if (!s) continue;
            if (i + 1 == n_stmts && s->kind == AST_EXPR_STMT) {
                /* Last statement is an expression-statement: its type is
                 * the value of the whole statement-expression. */
                t = sema_expr(&inner, s->u.expr_stmt.expr);
            } else {
                sema_stmt(&inner, s);
            }
        }
        break;
    }

    case AST_GENERIC_EXPR: {
        sema_expr(ss, expr->u.generic_expr.controlling);
        for (size_t i = 0; i < expr->u.generic_expr.associations.len; i++) {
            AstNode *a = expr->u.generic_expr.associations.data[i];
            sema_expr(ss, a->u.generic_assoc.value);
        }
        t = ty_int(ts);
        break;
    }

    case AST_GENERIC_ASSOC:
        t = ty_int(ts);
        break;

    case AST_INIT_LIST:
        for (size_t i = 0; i < expr->u.init_list.items.len; i++)
            sema_expr(ss, expr->u.init_list.items.data[i]);
        t = ty_error(ts);
        break;
    case AST_DESIGNATED_INIT:
        sema_expr(ss, expr->u.designated_init.value);
        t = ty_error(ts);
        break;

    default:
        t = ty_error(ts);
        break;
    }

    if (!t) t = ty_error(ts);
    expr->sem_type = t;
    return t;
}

/* =========================================================================
 * Statement sema
 * ====================================================================== */

static void sema_stmt(SS *ss, AstNode *stmt) {
    if (!stmt) return;
    TyStore *ts = ss->ctx->ts;

    switch (stmt->kind) {
    case AST_EXPR_STMT:
        sema_expr(ss, stmt->u.expr_stmt.expr);
        break;

    case AST_DECL_STMT: {
        AstNode *vd = stmt->u.decl_stmt.decl;
        if (!vd || vd->kind != AST_VAR_DECL) break;
        Type *decl_type = NULL;
        /* BUG-002 fix: detect `const auto x = expr` — the type AST is
         * AST_TYPE_CONST wrapping AST_TYPE_AUTO.  We must NOT call
         * ty_from_ast on it (there's no concrete type yet); instead we
         * deduce from the init expr and then wrap in const. */
        bool has_const_auto =
            (vd->u.var_decl.type &&
             vd->u.var_decl.type->kind == AST_TYPE_CONST &&
             vd->u.var_decl.type->u.type_const.base &&
             vd->u.var_decl.type->u.type_const.base->kind == AST_TYPE_AUTO);
        if (vd->u.var_decl.type &&
            vd->u.var_decl.type->kind != AST_TYPE_AUTO &&
            !has_const_auto) {
            decl_type = ty_from_ast(ts, vd->u.var_decl.type,
                                    ss->scope, ss->ctx->diags);
        }
        /* Infer init type first (important for auto). */
        Type *init_t = vd->u.var_decl.init
                     ? sema_expr(ss, vd->u.var_decl.init)
                     : NULL;
        /* Resolve the declared type. */
        if (!decl_type && init_t && !ty_is_error(init_t)) {
            /* auto or const auto deduction */
            decl_type = has_const_auto ? ty_const(ts, init_t) : init_t;
        } else if (!decl_type) {
            decl_type = ty_error(ts);
        }
        /* Type-check init vs declared type. */
        if (init_t && !ty_is_error(decl_type) && !ty_is_error(init_t)) {
            if (!assign_compat(ts, decl_type, init_t))
                FE_ERROR(ss->ctx->diags, vd->loc,
                    "cannot initialize '%s' with value of type '%s'",
                    ty_kind_name(decl_type->kind), ty_kind_name(init_t->kind));
        }
        vd->sem_type = decl_type;
        break;
    }

    case AST_RETURN: {
        Type *ret = ss->ret_type;
        if (!ret) break; /* outside a function? */
        if (!stmt->u.return_.value) {
            if (!ty_is_void(ret))
                FE_ERROR(ss->ctx->diags, stmt->loc,
                    "non-void function must return a value");
        } else {
            Type *vt = sema_expr(ss, stmt->u.return_.value);
            if (ty_is_void(ret))
                FE_ERROR(ss->ctx->diags, stmt->loc,
                    "void function cannot return a value");
            else {
                /* C 6.8.6.4: the value is converted as if by assignment
                 * to an object having the unqualified version of the
                 * function's return type.  Strip top-level const on
                 * both sides before comparison. */
                Type *ret_uc = ty_unconst(ts, ret);
                Type *vt_uc  = ty_unconst(ts, vt);
                if (!assign_compat(ts, ret_uc, vt_uc) &&
                    !ty_is_error(ret_uc) && !ty_is_error(vt_uc))
                    FE_ERROR(ss->ctx->diags, stmt->loc,
                        "return type mismatch: expected %s, got %s",
                        ty_kind_name(ret->kind), ty_kind_name(vt->kind));
            }
        }
        break;
    }

    case AST_BLOCK: {
        Scope *bs = stmt->sem_scope ? stmt->sem_scope : ss->scope;
        sema_block(ss, stmt, bs);
        break;
    }

    case AST_IF: {
        /* Compile-time conditional: `if (@has_operator(T, op)) { ... }`
         * When the condition is a compile-time @intrinsic that evaluates
         * to a known bool, skip type-checking the dead branch entirely
         * (same semantics as C++ `if constexpr`).  Both branches must
         * be syntactically valid but the dead one need not type-check.
         * This is the canonical Sharp way to provide two implementations
         * based on type capability; `@static_assert` is for hard errors. */
        AstNode *cond = stmt->u.if_.cond;
        int ct_val = -1;
        if (cond && cond->kind == AST_AT_INTRINSIC)
            ct_val = eval_intrinsic(ss, cond);

        if (ct_val >= 0) {
            /* Compile-time branch: annotate cond with int (true) or void
             * (false) so cg can detect which branch is live without
             * re-evaluating the intrinsic. */
            cond->sem_type = ct_val ? ty_int(ts) : ty_void(ts);  /* live branch */
                /* live = then_ if ct_val, else_ otherwise */
            if (ct_val)
                sema_stmt(ss, stmt->u.if_.then_);   /* true  branch checked */
            else
                sema_stmt(ss, stmt->u.if_.else_);   /* false branch checked */
        } else {
            /* Runtime condition — check both branches as usual. */
            Type *ct = sema_expr(ss, cond);
            sema_require_scalar(ss, stmt, ct, "if");
            sema_stmt(ss, stmt->u.if_.then_);
            sema_stmt(ss, stmt->u.if_.else_);
        }
        break;
    }

    case AST_WHILE: {
        Type *ct = sema_expr(ss, stmt->u.while_.cond);
        sema_require_scalar(ss, stmt, ct, "while");
        sema_stmt(ss, stmt->u.while_.body);
        break;
    }

    case AST_FOR: {
        Scope *for_scope = stmt->sem_scope ? stmt->sem_scope : ss->scope;
        SS inner = *ss; inner.scope = for_scope;
        sema_stmt(&inner, stmt->u.for_.init);
        if (stmt->u.for_.cond) {
            Type *ct = sema_expr(&inner, stmt->u.for_.cond);
            sema_require_scalar(ss, stmt, ct, "for");
        }
        if (stmt->u.for_.post) sema_expr(&inner, stmt->u.for_.post);
        sema_stmt(&inner, stmt->u.for_.body);
        break;
    }

    case AST_DO_WHILE: {
        sema_stmt(ss, stmt->u.do_while.body);
        Type *ct = sema_expr(ss, stmt->u.do_while.cond);
        sema_require_scalar(ss, stmt, ct, "do-while");
        break;
    }

    case AST_DEFER:
        sema_stmt(ss, stmt->u.defer_.body);
        break;

    case AST_BREAK: case AST_CONTINUE: case AST_LABEL: case AST_GOTO:
        break;  /* no type-checking needed */

    /* Phase S5: GCC computed goto.  Sema-check the target expression
     * so it gets a type and any inner identifiers resolve in scope; the
     * value's type itself is not constrained here — cc rejects targets
     * that are not pointer-typed. */
    case AST_COMPUTED_GOTO:
        sema_expr(ss, stmt->u.computed_goto.target);
        break;
    case AST_CASE:
        if (stmt->u.case_.value) sema_expr(ss, stmt->u.case_.value);
        break;
    case AST_SWITCH: {
        sema_expr(ss, stmt->u.switch_.cond);
        sema_stmt(ss, stmt->u.switch_.body);
        break;
    }
    default:
        break;
    }
}

static void sema_block(SS *ss, AstNode *block, Scope *block_scope) {
    if (!block) return;
    SS inner = *ss;
    inner.scope = block_scope ? block_scope : ss->scope;
    for (size_t i = 0; i < block->u.block.stmts.len; i++)
        sema_stmt(&inner, block->u.block.stmts.data[i]);
}

/* =========================================================================
 * Function and file sema
 * ====================================================================== */

static void sema_func(SemaCtx *ctx, Scope *parent_scope, AstNode *fn) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;
    if (!fn->u.func_def.body) return;  /* forward decl */
    /* Generic functions: body is checked at specialization time (CG phase).
     * Checking now would produce spurious TY_PARAM cascade errors. */
    if (fn->u.func_def.generic_params.len > 0) return;

    Scope *fs = fn->sem_scope ? fn->sem_scope : parent_scope;
    Type *ret  = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, fs, ctx->diags);
    SS ss = { ctx, fs, ret, NULL, NULL, NULL, 0 };
    sema_block(&ss, fn->u.func_def.body, fs);
    /* Phase 9: check for goto crossing defer. */
    sema_defer_check(fn->u.func_def.body, ctx->diags);
}

/* =========================================================================
 * Phase 7 — struct scope helpers
 * ====================================================================== */

/* Given a Type*, strip const/ptr to reach the TY_STRUCT base, then return
 * its Scope*.  arrow=true means we expect a pointer receiver. */
static Scope *struct_scope_of(TyStore *ts, Type *t, Scope *file_scope) {
    (void)ts;
    Type *st = ty_peel_to_struct(t);
    if (!st) return NULL;
    /* prefer decl-attached scope from Phase 4 */
    if (st->u.struct_.decl && st->u.struct_.decl->sem_scope)
        return (Scope *)st->u.struct_.decl->sem_scope;
    /* fallback: look up by name in file scope */
    if (file_scope) {
        Symbol *sym = scope_lookup_type(file_scope, st->u.struct_.name);
        if (sym && sym->decl && sym->decl->sem_scope)
            return (Scope *)sym->decl->sem_scope;
    }
    return NULL;
}

/* Map a binary operator token to its overload name, e.g. STOK_PLUS → "operator+". */
static const char *op_overload_name(SharpTokKind k) {
    const char *sym = fe_op_sym(k);
    if (!sym) return NULL;
    static char buf[64];
    snprintf(buf, sizeof buf, "operator%s", sym);
    return buf;
}

/* Lookup a method/field symbol in the struct scope of recv_type. */
static Symbol *struct_member(TyStore *ts, Type *recv_type, const char *name,
                              Scope *file_scope) {
    Scope *ss = struct_scope_of(ts, recv_type, file_scope);
    Symbol *msym = ss ? scope_lookup_local(ss, name) : NULL;
    if (msym) return msym;

    /* Extension method fallback: search file scope for SYM_FUNC with
     * matching struct_name (recv_type is pointer to struct/class). */
    if (file_scope) {
        Type *peel = ty_peel_to_struct(recv_type);
        if (peel) {
            const char *sname = peel->u.struct_.name;
            Symbol *esym = find_extension_method(file_scope, sname, name);
            if (esym) return esym;
        }
    }
    return NULL;
}

/* Check if the pointee (or value) of a receiver is const.
 * "const B*" or "const B" — the object referred to is const. */
static bool recv_object_is_const(Type *t) {
    if (!t) return false;
    if (t->kind == TY_CONST) return true;
    if (t->kind == TY_ATOMIC && t->u.atomic.base &&
        t->u.atomic.base->kind == TY_CONST) return true;
    if (t->kind == TY_PTR) {
        Type *base = t->u.ptr.base;
        while (base && base->kind == TY_ATOMIC) base = base->u.atomic.base;
        if (base && base->kind == TY_CONST) return true;
    }
    return false;
}

static bool sema_require_arrow(SS *ss, const AstNode *expr, Type *recv_t) {
    Type *base = ty_unconst(ss->ctx->ts, recv_t);
    if (!base || !ty_is_pointer(base)) {
        FE_ERROR(ss->ctx->diags, expr->loc, "'->' requires pointer operand");
        return false;
    }
    return true;
}

/* Sema for field access: recv.field or recv->field */
static Type *sema_field_access_expr(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    Type *recv_t = sema_expr(ss, expr->u.field_access.recv);
    bool arrow   = expr->u.field_access.arrow;
    const char *field = expr->u.field_access.field;

    /* Validate arrow: recv must be a pointer. */
    if (arrow && !sema_require_arrow(ss, expr, recv_t)) return ty_error(ts);

    /* Check if recv is a type name (associated function call context). */
    if (expr->u.field_access.recv->kind == AST_IDENT) {
        const char *recv_name = expr->u.field_access.recv->u.ident.name;
        Symbol *rsym = scope_lookup(ss->scope, recv_name);
        if (rsym && rsym->kind == SYM_TYPE) {
            /* Type.field — look up in struct scope as a member. */
            Scope *ss_s = rsym->decl ? (Scope*)rsym->decl->sem_scope : NULL;
            if (ss_s) {
                Symbol *msym = scope_lookup_local(ss_s, field);
                if (msym && msym->decl && msym->decl->kind == AST_FUNC_DEF) {
                    AstNode *fn = msym->decl;
                    return ty_from_ast(ts, fn->u.func_def.ret_type,
                                       ss->scope, ss->ctx->diags);
                }
            }
        }
    }

    /* Locate the struct scope. */
    Symbol *fsym = struct_member(ts, recv_t, field, ss->ctx->file_scope);
    if (!fsym) {
        if (!ty_is_error(recv_t))
            FE_ERROR(ss->ctx->diags, expr->loc,
                "no member '%s' in struct", field);
        return ty_error(ts);
    }

    if (fsym->kind == SYM_FIELD && fsym->decl) {
        /* Resolve field type using the struct's own scope (for generic param lookup). */
        Scope *field_scope = struct_scope_of(ts, recv_t, ss->ctx->file_scope);
        Type *ft = ty_from_ast(ts, fsym->decl->u.field_decl.type,
                               field_scope ? field_scope : ss->scope,
                               NULL /* suppress "unknown 'T'" for generic params */);
        /* Substitute generic params if the receiver is a concrete instantiation. */
        Type *base_t2 = ty_peel_to_struct(recv_t);
        ft = sema_subst_for_struct(ts, base_t2, ft);
        return ft;
    }
    if (fsym->kind == SYM_METHOD && fsym->decl) {
        /* field access on a method — return func type sentinel */
        return ty_error(ts);
    }
    return ty_error(ts);
}

/* Sema for method call: recv.method(args) or recv->method(args) */
static Type *sema_method_call_expr(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    bool arrow   = expr->u.method_call.arrow;
    const char *method = expr->u.method_call.method;

    /* ── Check for associated function call BEFORE sema_expr(recv) ──────
     * Associated function: `Type.method(args)` where recv is a type name.
     * Must be checked first to avoid "is a type, not a value" from sema_expr.
     *
     * Case 1: concrete struct — recv is AST_IDENT naming a SYM_TYPE.
     * Case 2: generic struct  — recv is AST_CAST(type=TYPE_GENERIC, op=NULL)
     *                           (parser wraps generic type exprs in cast nodes). */
    AstNode *recv_node = expr->u.method_call.recv;
    {
        const char *assoc_struct_name = NULL;

        if (recv_node && recv_node->kind == AST_IDENT) {
            const char *rname = recv_node->u.ident.name;
            Symbol *rsym = scope_lookup(ss->scope, rname);
            if (rsym && rsym->kind == SYM_TYPE)
                assoc_struct_name = rname;
        } else if (recv_node && recv_node->kind == AST_CAST &&
                   recv_node->u.cast.operand == NULL &&
                   recv_node->u.cast.type &&
                   recv_node->u.cast.type->kind == AST_TYPE_GENERIC) {
            assoc_struct_name = recv_node->u.cast.type->u.type_generic.name;
        }

        if (assoc_struct_name) {
            /* Evaluate arguments */
            sema_expr_vec(ss, &expr->u.method_call.args);
            /* Look up the method in the struct's scope */
            Symbol *tsym = scope_lookup_type(ss->scope, assoc_struct_name);
            Scope *struct_scope = NULL;
            if (tsym && tsym->decl && tsym->decl->sem_scope)
                struct_scope = (Scope*)tsym->decl->sem_scope;
            if (struct_scope) {
                Symbol *msym = scope_lookup_local(struct_scope, method);
                if (msym && msym->decl && msym->decl->kind == AST_FUNC_DEF) {
                    AstNode *fn = msym->decl;
                    /* Return type: use method's own scope to resolve generics,
                     * then substitute concrete type args if receiver is generic
                     * (e.g. Stack<int>.new() → return type Stack<T> → Stack__int).
                     * We look up the concrete monomorphized struct by mangled name
                     * (e.g. Stack__int) to avoid infinite recursion in ty_from_ast
                     * for self-referential generic types. */
                    Scope *mscope = fn->sem_scope ? fn->sem_scope : ss->scope;
                    Type *ret_t = ty_from_ast(ts, fn->u.func_def.ret_type, mscope, NULL);
                    ret_t = sema_subst_generic_receiver(ss, recv_node, ret_t);
                    return ret_t;
                }
            }
            /* Extension method fallback: look up SYM_FUNC at file scope
             * whose decl has matching struct_name. */
            if (ss->ctx && ss->ctx->file_scope) {
                Symbol *esym = find_extension_method(ss->ctx->file_scope,
                    assoc_struct_name, method);
                if (esym && esym->decl && esym->decl->kind == AST_FUNC_DEF) {
                    AstNode *efn = esym->decl;
                    Scope *mscope = efn->sem_scope ? efn->sem_scope : ss->scope;
                    Type *ret_t = ty_from_ast(ts, efn->u.func_def.ret_type, mscope, NULL);
                    ret_t = sema_subst_generic_receiver(ss, recv_node, ret_t);
                    return ret_t;
                }
            }
            FE_ERROR(ss->ctx->diags, expr->loc,
                "no associated function '%s' in type '%s'", method, assoc_struct_name);
            return ty_error(ts);
        }
    }

    /* Regular method call: evaluate receiver as expression. */
    Type *recv_t = sema_expr(ss, recv_node);

    /* Evaluate arguments regardless. */
    sema_expr_vec(ss, &expr->u.method_call.args);

    /* v0.13: defer method dispatch on TY_PARAM receivers — the method
     * cannot be resolved until specialization installs the concrete
     * type.  Return a placeholder type so type-of-this-expression
     * queries don't cascade-error.  cg-time AST_METHOD_CALL handles
     * the real dispatch via cg_resolve_type. */
    {
        Type *peel = recv_t;
        peel = ty_unconst(ts, peel);
        if (peel && peel->kind == TY_PTR)   peel = peel->u.ptr.base;
        peel = ty_unconst(ts, peel);
        if (peel && peel->kind == TY_PARAM)
            return ty_int(ts);  /* lenient placeholder; cg resolves for real */
    }

    /* Validate arrow: recv must be a pointer. */
    if (arrow && !sema_require_arrow(ss, expr, recv_t)) return ty_error(ts);

    /* Look up method in struct scope. */
    Symbol *msym = struct_member(ts, recv_t, method, ss->ctx->file_scope);
    if (!msym || (msym->kind != SYM_METHOD && msym->kind != SYM_FUNC)) {
        /* Not a method — but might be a function-pointer field, which
         * the parser cannot disambiguate from a method call at parse
         * time (both look like `expr.name(args)`).  Look up `name` as
         * a field; if its type is a pointer-to-function (or a function
         * type), treat the call as `(recv.field)(args)` and check
         * arguments against the field's signature.  This is the C
         * idiom used by glibc's <bits/types/sigevent_t.h> sigevent_t
         * thread struct, GTK vtables, etc. */
        if (msym && msym->kind == SYM_FIELD && msym->decl &&
            msym->decl->kind == AST_FIELD_DECL) {
            Type *ft = ty_from_ast(ts, msym->decl->u.field_decl.type,
                                   ss->ctx->file_scope, NULL);
            Type *base = ft;
            if (base && base->kind == TY_PTR) base = base->u.ptr.base;
            if (base && base->kind == TY_FUNC) {
                /* Mark this AST_METHOD_CALL node for cg as a
                 * function-pointer-field call.  Args were already
                 * sema-checked above, so we don't redo them here.
                 * Return the function-pointer's return type. */
                expr->u.method_call.is_field_call = true;
                return base->u.func.ret;
            }
        }
        if (!ty_is_error(recv_t))
            FE_ERROR(ss->ctx->diags, expr->loc,
                "no method '%s' in struct", method);
        return ty_error(ts);
    }

    AstNode *fn = msym->decl;
    if (!fn || fn->kind != AST_FUNC_DEF) return ty_error(ts);

    /* const receiver check. */
    bool is_const_recv = recv_object_is_const(recv_t);
    if (is_const_recv && !fn->u.func_def.is_const_method) {
        FE_ERROR(ss->ctx->diags, expr->loc,
            "cannot call non-const method '%s' on const receiver", method);
    }

    /* Use the method's own scope for return type resolution so that generic
     * type params (T) are found.  Suppress diagnostics here — unknown 'T'
     * is expected for uninstantiated generics; substitution handles it. */
    Scope *mscope = fn->sem_scope ? fn->sem_scope : ss->scope;
    Type *ret_t = ty_from_ast(ts, fn->u.func_def.ret_type, mscope, NULL);
    /* Substitute generic params if receiver is a concrete instantiation. */
    Type *base_t3 = ty_peel_to_struct(recv_t);
    return sema_subst_for_struct(ts, base_t3, ret_t);
}

/* =========================================================================
 * Phase 9: defer–goto safety check
 *
 * Conservative rule (probe P10.5 decision):
 * Any goto in a function that also contains any defer → error on the goto.
 * break/continue are explicitly safe per spec §defer-stmt.
 * ====================================================================== */

typedef struct {
    bool       has_defer;   /* true once we encounter any defer in the fn */
    FeDiagArr *diags;
} DcState;

static void dc_stmt(DcState *ds, const AstNode *stmt);

static void dc_stmts(DcState *ds, const AstVec *stmts) {
    for (size_t i = 0; i < stmts->len; i++)
        dc_stmt(ds, stmts->data[i]);
}

static void dc_stmt(DcState *ds, const AstNode *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case AST_DEFER:
        ds->has_defer = true;
        /* Recurse into defer body to catch nested gotos. */
        dc_stmt(ds, stmt->u.defer_.body);
        break;
    case AST_GOTO:
        if (ds->has_defer && ds->diags)
            FE_ERROR(ds->diags, stmt->loc,
                "goto '%s' crosses a defer statement",
                stmt->u.goto_.label ? stmt->u.goto_.label : "?");
        break;
    /* Phase S5: a computed goto is just as unstructured as a plain goto
     * — defer crossing is the same defect.  We can't name the target
     * label since it's a runtime value, so the diag is generic. */
    case AST_COMPUTED_GOTO:
        if (ds->has_defer && ds->diags)
            FE_ERROR(ds->diags, stmt->loc,
                "computed goto crosses a defer statement");
        break;
    case AST_BLOCK:
        dc_stmts(ds, &stmt->u.block.stmts);
        break;
    case AST_IF:
        dc_stmt(ds, stmt->u.if_.then_);
        dc_stmt(ds, stmt->u.if_.else_);
        break;
    case AST_WHILE:
        dc_stmt(ds, stmt->u.while_.body);
        break;
    case AST_FOR:
        dc_stmt(ds, stmt->u.for_.init);
        dc_stmt(ds, stmt->u.for_.body);
        break;
    case AST_DO_WHILE:
        dc_stmt(ds, stmt->u.do_while.body);
        break;
    case AST_SWITCH:
        dc_stmt(ds, stmt->u.switch_.body);
        break;
    case AST_DECL_STMT: case AST_EXPR_STMT: case AST_RETURN:
    case AST_BREAK:     case AST_CONTINUE:  case AST_LABEL:
        break; /* no recursion needed / safe */
    default:
        break;
    }
}

/* Two-pass: first scan for any defer, then scan for any goto.
 * By scanning left-to-right and tracking has_defer, we catch gotos that
 * come after a defer.  For gotos before a defer, we need a second pass
 * (conservative: if function has both, every goto is an error). */
static void sema_defer_check(const AstNode *body, FeDiagArr *diags) {
    if (!body) return;
    /* Pass 1: does the body contain any defer? */
    DcState probe = { false, NULL };
    dc_stmts(&probe, &body->u.block.stmts);
    if (!probe.has_defer) return;  /* no defer → nothing to check */

    /* Pass 2: mark has_defer = true from the start, flag any goto.
     * Only run if diags is non-NULL (safety guard). */
    if (!diags) return;
    DcState check = { true, diags };
    dc_stmts(&check, &body->u.block.stmts);
}

/* =========================================================================
 * Phase 10: @intrinsic semantic evaluation
 *
 * @static_assert(cond, "msg") — if cond is a compile-time 0, emit error.
 * @has_operator(Type, op)     — returns 1 if Type has operator op, else 0.
 *   When nested inside @static_assert, the combined check fires.
 * ====================================================================== */

/* C23 constexpr: compile-time expression evaluator.
 *
 * EvalResult holds a constant value and its type.  Currently only integer
 * results are fully supported; float and pointer results are future work.
 */
typedef struct {
    bool   ok;
    int64_t ival;      /* integer result */
    double   fval;     /* float result */
    bool     is_float; /* true if fval is valid */
} EvalResult;

static EvalResult constexpr_eval(SS *ss, const AstNode *expr);

/* Try to evaluate an expression as a compile-time integer constant.
 * Returns true and sets *val on success, false if not a constant. */
static bool eval_const_int(const AstNode *expr, int64_t *val) {
    EvalResult r = constexpr_eval(NULL, expr);
    if (r.ok && !r.is_float) { *val = r.ival; return true; }
    return false;
}

/* Core constexpr evaluator — handles binary/unary ops, ternary, sizeof,
 * function calls to constexpr functions, and constexpr variable refs. */
static EvalResult constexpr_eval(SS *ss, const AstNode *expr) {
    EvalResult fail = { false, 0, 0.0, false };
    if (!expr) return fail;

    switch (expr->kind) {
    case AST_INT_LIT:
        return (EvalResult){ true, (int64_t)expr->u.int_lit.val, 0.0, false };

    case AST_FLOAT_LIT: {
        double v = expr->u.float_lit.val;
        return (EvalResult){ true, (int64_t)v, v, true };
    }

    case AST_IDENT: {
        /* Resolve constexpr variable reference */
        if (!ss) return fail;
        Symbol *sym = scope_lookup(ss->scope, expr->u.ident.name);
        if (!sym || sym->kind != SYM_VAR || !sym->decl) return fail;
        AstNode *vd = sym->decl;
        if (vd->kind != AST_VAR_DECL || !vd->u.var_decl.is_constexpr) return fail;
        if (!vd->u.var_decl.init) return fail;
        return constexpr_eval(ss, vd->u.var_decl.init);
    }

    case AST_PAREN:
        return constexpr_eval(ss, expr->u.paren.inner);

    case AST_UNARY: {
        EvalResult op = constexpr_eval(ss, expr->u.unary.operand);
        if (!op.ok) return fail;
        switch (expr->u.unary.op) {
        case STOK_MINUS:
            return op.is_float
                ? (EvalResult){ true, -(int64_t)op.fval, -op.fval, true }
                : (EvalResult){ true, -op.ival, (double)(-op.ival), op.is_float };
        case STOK_PLUS:
            return op;
        case STOK_TILDE:
            return (EvalResult){ true, ~op.ival, (double)(~op.ival), op.is_float };
        case STOK_BANG:
            return (EvalResult){ true, !op.ival ? 1 : 0, (double)(!op.ival ? 1 : 0), op.is_float };
        default:
            return fail;
        }
    }

    case AST_BINOP: {
        EvalResult lhs = constexpr_eval(ss, expr->u.binop.lhs);
        EvalResult rhs = constexpr_eval(ss, expr->u.binop.rhs);
        if (!lhs.ok || !rhs.ok) return fail;

        bool lf = lhs.is_float, rf = rhs.is_float;
        bool use_float = lf || rf;
        int64_t li = lhs.ival, ri = rhs.ival;
        double fd = use_float ? (lf ? lhs.fval : lhs.ival) : 0;
        double rd = use_float ? (rf ? rhs.fval : rhs.ival) : 0;

        switch (expr->u.binop.op) {
        case STOK_PLUS:
            return use_float
                ? (EvalResult){ true, (int64_t)(fd + rd), fd + rd, true }
                : (EvalResult){ true, li + ri, (double)(li + ri), false };
        case STOK_MINUS:
            return use_float
                ? (EvalResult){ true, (int64_t)(fd - rd), fd - rd, true }
                : (EvalResult){ true, li - ri, (double)(li - ri), false };
        case STOK_STAR:
            return use_float
                ? (EvalResult){ true, (int64_t)(fd * rd), fd * rd, true }
                : (EvalResult){ true, li * ri, (double)(li * ri), false };
        case STOK_SLASH:
            if (use_float) {
                if (rd == 0.0) return fail;
                return (EvalResult){ true, (int64_t)(fd / rd), fd / rd, true };
            }
            if (ri == 0) return fail;
            return (EvalResult){ true, li / ri, (double)(li / ri), false };
        case STOK_PERCENT:
            if (ri == 0) return fail;
            return (EvalResult){ true, li % ri, (double)(li % ri), false };
        case STOK_EQEQ:
            return use_float
                ? (EvalResult){ true, fd == rd ? 1 : 0, (double)(fd == rd ? 1 : 0), false }
                : (EvalResult){ true, li == ri ? 1 : 0, 0.0, false };
        case STOK_BANGEQ:
            return use_float
                ? (EvalResult){ true, fd != rd ? 1 : 0, (double)(fd != rd ? 1 : 0), false }
                : (EvalResult){ true, li != ri ? 1 : 0, 0.0, false };
        case STOK_LT:
            return use_float
                ? (EvalResult){ true, fd < rd ? 1 : 0, (double)(fd < rd ? 1 : 0), false }
                : (EvalResult){ true, li < ri ? 1 : 0, 0.0, false };
        case STOK_GT:
            return use_float
                ? (EvalResult){ true, fd > rd ? 1 : 0, (double)(fd > rd ? 1 : 0), false }
                : (EvalResult){ true, li > ri ? 1 : 0, 0.0, false };
        case STOK_LTEQ:
            return use_float
                ? (EvalResult){ true, fd <= rd ? 1 : 0, (double)(fd <= rd ? 1 : 0), false }
                : (EvalResult){ true, li <= ri ? 1 : 0, 0.0, false };
        case STOK_GTEQ:
            return use_float
                ? (EvalResult){ true, fd >= rd ? 1 : 0, (double)(fd >= rd ? 1 : 0), false }
                : (EvalResult){ true, li >= ri ? 1 : 0, 0.0, false };
        case STOK_AMPAMP:
            return (EvalResult){ true, (li && ri) ? 1 : 0, 0.0, false };
        case STOK_PIPEPIPE:
            return (EvalResult){ true, (li || ri) ? 1 : 0, 0.0, false };
        case STOK_AMP:
            return (EvalResult){ true, li & ri, (double)(li & ri), false };
        case STOK_PIPE:
            return (EvalResult){ true, li | ri, (double)(li | ri), false };
        case STOK_CARET:
            return (EvalResult){ true, li ^ ri, (double)(li ^ ri), false };
        case STOK_LTLT:
            return (EvalResult){ true, ri >= 0 && (uint64_t)ri < 64 ? li << ri : 0,
                                 (double)(ri >= 0 && (uint64_t)ri < 64 ? (int64_t)((uint64_t)li << ri) : 0), false };
        case STOK_GTGT:
            return (EvalResult){ true, ri >= 0 && (uint64_t)ri < 64 ? li >> ri : 0,
                                 (double)(ri >= 0 && (uint64_t)ri < 64 ? li >> ri : 0), false };
        default:
            return fail;
        }
    }

    case AST_TERNARY: {
        EvalResult cond = constexpr_eval(ss, expr->u.ternary.cond);
        if (!cond.ok) return fail;
        return cond.ival
            ? constexpr_eval(ss, expr->u.ternary.then_)
            : constexpr_eval(ss, expr->u.ternary.else_);
    }

    case AST_SIZEOF: {
        /* Simplified: only handles basic types with known sizes */
        if (!ss || !expr->u.sizeof_.is_type) return fail;
        AstNode *ty_node = expr->u.sizeof_.operand;
        if (ty_node->kind == AST_TYPE_NAME) {
            const char *tn = ty_node->u.type_name.name;
            if (strcmp(tn, "int") == 0 || strcmp(tn, "long") == 0 ||
                strcmp(tn, "isize") == 0 || strcmp(tn, "usize") == 0)
                return (EvalResult){ true, 4, 4.0, false };
            if (strcmp(tn, "char") == 0 || strcmp(tn, "bool") == 0)
                return (EvalResult){ true, 1, 1.0, false };
            if (strcmp(tn, "short") == 0)
                return (EvalResult){ true, 2, 2.0, false };
            if (strcmp(tn, "long long") == 0)
                return (EvalResult){ true, 8, 8.0, false };
            if (strcmp(tn, "float") == 0)
                return (EvalResult){ true, 4, 4.0, false };
            if (strcmp(tn, "double") == 0)
                return (EvalResult){ true, 8, 8.0, false };
        }
        return fail;
    }

    case AST_CALL: {
        if (!ss) return fail;
        AstNode *callee = expr->u.call.callee;
        if (callee->kind != AST_IDENT) return fail;
        Symbol *sym = scope_lookup(ss->scope, callee->u.ident.name);
        if (!sym || sym->kind != SYM_FUNC || !sym->decl) return fail;
        AstNode *fn = sym->decl;
        if (fn->kind != AST_FUNC_DEF || !fn->u.func_def.is_constexpr) return fail;

        /* N3205: constexpr function calls are allowed, but recursive calls
         * to the same function are prohibited. */
        if (ss->cur_constexpr_fn == fn) return fail;

        /* Evaluate arguments */
        size_t nargs = expr->u.call.args.len;
        AstNode **args = expr->u.call.args.data;
        AstVec *params = &fn->u.func_def.params;

        EvalResult *arg_vals = malloc(nargs * sizeof *arg_vals);
        if (!arg_vals) abort();
        for (size_t i = 0; i < nargs; i++) {
            arg_vals[i] = constexpr_eval(ss, args[i]);
            if (!arg_vals[i].ok) { free(arg_vals); return fail; }
        }

        /* Substituted body evaluation: create a new SS with a child scope
         * that maps param names to constexpr variable nodes carrying values. */
        Scope *tmp_scope = scope_new(SCOPE_BLOCK, ss->scope, NULL);
        for (size_t pi = 0; pi < params->len && pi < nargs; pi++) {
            AstNode *pd = params->data[pi];
            if (pd->kind != AST_PARAM_DECL) continue;
            /* Create a fake constexpr var node that holds the arg value */
            AstNode *fake_vd = ast_node_new(AST_VAR_DECL, pd->loc);
            fake_vd->u.var_decl.name = pd->u.param_decl.name;
            fake_vd->u.var_decl.is_constexpr = true;
            /* Build an int literal node as the init */
            AstNode *lit = ast_node_new(AST_INT_LIT, pd->loc);
            lit->u.int_lit.val = arg_vals[pi].ival;
            fake_vd->u.var_decl.init = lit;
            scope_define(tmp_scope, SYM_VAR, pd->u.param_decl.name, fake_vd, NULL);
        }

        SS tmp_ss = *ss;
        tmp_ss.scope = tmp_scope;

        EvalResult body_result = fail;
        if (fn->u.func_def.body) {
            AstVec *stmts = &fn->u.func_def.body->u.block.stmts;
            for (size_t si = 0; si < stmts->len; si++) {
                AstNode *s = stmts->data[si];
                if (s && s->kind == AST_RETURN && s->u.return_.value) {
                    body_result = constexpr_eval(&tmp_ss, s->u.return_.value);
                    break;
                }
            }
        }

        free(arg_vals);
        scope_free(tmp_scope);
        return body_result;
    }

    default:
        return fail;
    }
}

typedef struct { Scope *scope; Type *type; int status; } StructArgResult;

static StructArgResult resolve_struct_type_arg(SS *ss, const AstNode *ty_arg) {
    StructArgResult r = {NULL, NULL, 0};
    Type *t = resolve_intrinsic_type_arg(ss, ty_arg);
    if (!t || ty_is_error(t)) return r;
    if (ty_contains_param(t)) { r.status = -1; return r; }
    t = ty_strip_cvq(ss->ctx->ts, t);
    if (!t || t->kind != TY_STRUCT) return r;
    AstNode *sd = t->u.struct_.decl;
    if (!sd || !sd->sem_scope) return r;
    r.scope = (Scope*)sd->sem_scope;
    r.type = t;
    r.status = 1;
    return r;
}

/* Evaluate @has_operator(TypeName, op_symbol) in current scope.
 * Returns 1 if the named struct has the operator, 0 otherwise.
 *
 * The op_symbol argument is parsed as an expression — the parser sees:
 *   +   → UNARY(STOK_PLUS, dummy)
 *   ==  → BINOP(dummy, STOK_EQEQ, dummy)   (dummy = IDENT("?") or INT_LIT(0))
 * We extract the operator from whatever AST node the parser produced.
 */
static Scope *scope_find_file_scope(Scope *s) {
    while (s && s->kind != SCOPE_FILE) s = s->parent;
    return s;
}

static int eval_has_operator(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 2) return 0;
    AstNode *type_arg = expr->u.at_intrinsic.args.data[0];
    AstNode *op_arg   = expr->u.at_intrinsic.args.data[1];

    /* v0.13: use the unified type resolver so substitution applies.
     * This lets @has_operator(T, +) in a generic template body re-eval
     * to the concrete struct at instantiation time. */
    StructArgResult sr = resolve_struct_type_arg(ss, type_arg);
    if (sr.status == -1) return -1;
    if (!sr.scope) return 0;
    Scope *ss_s = sr.scope;

    /* Extract the operator symbol string from the parsed op_arg node. */
    char opname[64] = {0};
    const char *sym_str = NULL;

    if (op_arg->kind == AST_UNARY) {
        /* "+" parsed as UNARY(PLUS, dummy) */
        sym_str = fe_op_sym(op_arg->u.unary.op);
    } else if (op_arg->kind == AST_BINOP) {
        /* "==" parsed as BINOP(dummy, EQEQ, dummy) */
        sym_str = fe_op_sym(op_arg->u.binop.op);
    } else if (op_arg->kind == AST_IDENT) {
        /* Plain identifier — maybe "[]" or a named operator */
        sym_str = op_arg->u.ident.name;
    }

    if (!sym_str) return 0;

    /* Build "operator<sym>" if not already prefixed. */
    if (strncmp(sym_str, "operator", 8) == 0) {
        snprintf(opname, sizeof opname, "%s", sym_str);
    } else {
        snprintf(opname, sizeof opname, "operator%s", sym_str);
    }

    if (scope_lookup_local(ss_s, opname) != NULL)
        return 1;

    /* Extension method fallback: search file scope for operator defined
     * as top-level `RetType StructName.operator+(params) { ... }`. */
    if (ss->ctx && ss->ctx->file_scope && sr.type->u.struct_.name) {
        /* Walk up to SCOPE_FILE — during specialization re-eval,
         * ctx->file_scope may be a function-local scope. */
        Scope *fs = scope_find_file_scope(ss->ctx->file_scope);
        if (fs) {
            Symbol *es = find_extension_method(fs, sr.type->u.struct_.name, opname);
            if (es && es->decl && es->decl->kind == AST_FUNC_DEF &&
                es->decl->u.func_def.is_operator)
                return 1;
        }
    }
    return 0;
}

/* =========================================================================
 * v0.13 — @-primitive evaluation helpers
 *
 * The 5 new primitives all share a "first arg is a TYPE" shape; the parser
 * stores it as an AST_TYPE_* node (not an AST_IDENT) for built-in types,
 * and as either AST_IDENT or AST_TYPE_NAME for struct names.  We use
 * ty_from_ast to resolve it to a Type* in the current scope.
 * ====================================================================== */

/* Resolve the first arg of an @intrinsic to a Type*.  Handles both:
 *   - AST_IDENT (struct name from generic-friendly parsing path)
 *   - AST_TYPE_* (parse_type result for built-in keywords etc.)
 * Returns ty_error on failure.
 *
 * v0.13: if ss carries a generic substitution context (subst_np > 0),
 * applies ty_subst so that TY_PARAM args (e.g. K inside HashMap<K,V>)
 * resolve to their concrete instantiation type. */
static Type *resolve_intrinsic_type_arg(SS *ss, const AstNode *arg) {
    if (!arg) return ty_error(ss->ctx->ts);
    Type *t;
    if (arg->kind == AST_IDENT) {
        /* parse_type wraps idents in AST_TYPE_NAME, but the has_operator
         * path uses parse_expr which yields AST_IDENT directly.  Build a
         * synthetic AST_TYPE_NAME so ty_from_ast can resolve it. */
        AstNode tmp = {0};
        tmp.kind = AST_TYPE_NAME;
        tmp.loc  = arg->loc;
        tmp.u.type_name.name = (char*)arg->u.ident.name;
        t = ty_from_ast(ss->ctx->ts, &tmp, ss->scope, ss->ctx->diags);
    } else {
        t = ty_from_ast(ss->ctx->ts, arg, ss->scope, ss->ctx->diags);
    }
    if (ss->subst_np > 0 && t && !ty_is_error(t)) {
        t = ty_subst(ss->ctx->ts, t,
                     ss->subst_pnames, ss->subst_pvals, ss->subst_np);
    }
    return t;
}

/* Returns true if t still contains any unresolved TY_PARAM.  Used to defer
 * @-primitive evaluation from template-definition time (where T is
 * unbound) to instantiation time (where T is concrete).  v0.13. */
static bool ty_contains_param(const Type *t) {
    if (!t) return false;
    switch (t->kind) {
    case TY_PARAM:  return true;
    case TY_PTR:    return ty_contains_param(t->u.ptr.base);
    case TY_CONST:  return ty_contains_param(t->u.const_.base);
    case TY_ATOMIC: return ty_contains_param(t->u.atomic.base);
    case TY_ARRAY:  return ty_contains_param(t->u.array.base);
    case TY_FUNC: {
        if (ty_contains_param(t->u.func.ret)) return true;
        for (size_t i = 0; i < t->u.func.nparams; i++)
            if (ty_contains_param(t->u.func.params[i])) return true;
        return false;
    }
    case TY_STRUCT:
        for (size_t i = 0; i < t->u.struct_.nargs; i++)
            if (ty_contains_param(t->u.struct_.args[i])) return true;
        return false;
    case TY_ENUM:   return false;
    case TY_VECTOR: return ty_contains_param(t->u.vector.elem);
    default:        return false;
    }
}

/* @has_method(T, name): T is struct AND has a SYM_METHOD named `name`.
 * Operators (registered as "operator+" etc.) are NOT considered methods. */
static int eval_has_method(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 2) return -1;
    AstNode *ty_arg = expr->u.at_intrinsic.args.data[0];
    AstNode *nm_arg = expr->u.at_intrinsic.args.data[1];
    if (nm_arg->kind != AST_IDENT) return -1;

    StructArgResult sr = resolve_struct_type_arg(ss, ty_arg);
    if (sr.status == -1) return -1;
    if (!sr.scope) return 0;
    Scope *st_scope = sr.scope;

    const char *name = nm_arg->u.ident.name;
    if (!name) return 0;
    /* Exclude operator-form names: operators belong to @has_operator. */
    if (strncmp(name, "operator", 8) == 0) return 0;

    Symbol *sym = scope_lookup_local(st_scope, name);
    if (sym && sym->kind == SYM_METHOD) return 1;

    /* Extension method fallback: search file scope for SYM_FUNC with
     * matching struct_name (non-operator, non-static methods). */
    if (!sym && ss->ctx && ss->ctx->file_scope && sr.type->u.struct_.name) {
        /* Walk up to the SCOPE_FILE scope — during generic
         * specialization, ss->ctx->file_scope may be a function-local
         * scope rather than the actual file scope. */
        Scope *fs = scope_find_file_scope(ss->ctx->file_scope);
        if (fs) {
            Symbol *es = find_extension_method(fs, sr.type->u.struct_.name, name);
            if (es && es->decl && es->decl->kind == AST_FUNC_DEF)
                return es->decl->u.func_def.has_receiver ? 1 : 0;
        }
    }
    return 0;
}

/* @has_field(T, name): T is struct AND has a SYM_FIELD named `name`. */
static int eval_has_field(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 2) return -1;
    AstNode *ty_arg = expr->u.at_intrinsic.args.data[0];
    AstNode *nm_arg = expr->u.at_intrinsic.args.data[1];
    if (nm_arg->kind != AST_IDENT) return -1;

    StructArgResult sr = resolve_struct_type_arg(ss, ty_arg);
    if (sr.status == -1) return -1;
    if (!sr.scope) return 0;
    Scope *st_scope = sr.scope;

    const char *name = nm_arg->u.ident.name;
    if (!name) return 0;

    Symbol *sym = scope_lookup_local(st_scope, name);
    if (!sym) return 0;
    return sym->kind == SYM_FIELD ? 1 : 0;
}

/* @is_arithmetic(T): T is any integer / float / char / bool. */
static int eval_is_arithmetic(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 1) return -1;
    Type *t = resolve_intrinsic_type_arg(ss, expr->u.at_intrinsic.args.data[0]);
    if (!t || ty_is_error(t)) return 0;
    if (ty_contains_param(t)) return -1;
    return ty_is_arithmetic(t) ? 1 : 0;
}

/* @is_pointer(T): T is any pointer form (T*, const T*, T* const, ...). */
static int eval_is_pointer(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 1) return -1;
    Type *t = resolve_intrinsic_type_arg(ss, expr->u.at_intrinsic.args.data[0]);
    if (!t || ty_is_error(t)) return 0;
    if (ty_contains_param(t)) return -1;
    return ty_is_pointer(t) ? 1 : 0;
}

/* @type_name(T): produce a heap-allocated C string holding T's mangled name.
 * The caller stores this on at_intrinsic.resolved_str; cg emits it as a
 * string literal.  Mangling rules mirror cg.c's cgb_mangle_type (spec §
 * 名字改编). */

static char *compute_type_name(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 1) return NULL;
    Type *t = resolve_intrinsic_type_arg(ss, expr->u.at_intrinsic.args.data[0]);
    if (!t || ty_is_error(t)) {
        const char *e = "<error>";
        size_t n = strlen(e) + 1;
        char *r = malloc(n);
        if (r) memcpy(r, e, n);
        return r;
    }
    StrBuf sb = {0};
    ty_mangle(&sb, t);
    if (sb.buf) return sb.buf;
    char *r = malloc(1);
    if (r) r[0] = '\0';
    return r;
}

/* Evaluate a @static_assert or @has_* expression.
 * Returns the result as a compile-time boolean (0 or 1),
 * or -1 if the expression is not a compile-time constant. */
static int eval_intrinsic(SS *ss, const AstNode *expr) {
    if (!expr || expr->kind != AST_AT_INTRINSIC) return -1;
    const char *name = expr->u.at_intrinsic.name;

    if (strcmp(name, "has_operator")  == 0) return eval_has_operator(ss, expr);
    if (strcmp(name, "has_method")    == 0) return eval_has_method(ss, expr);
    if (strcmp(name, "has_field")     == 0) return eval_has_field(ss, expr);
    if (strcmp(name, "is_arithmetic") == 0) return eval_is_arithmetic(ss, expr);
    if (strcmp(name, "is_pointer")    == 0) return eval_is_pointer(ss, expr);

    if (strcmp(name, "static_assert") == 0) {
        /* @static_assert evaluates to void/1; the check is side-effectful. */
        if (expr->u.at_intrinsic.args.len < 1) return -1;
        AstNode *cond = expr->u.at_intrinsic.args.data[0];
        /* Condition may itself be any @ query primitive */
        if (cond->kind == AST_AT_INTRINSIC) {
            int r = eval_intrinsic(ss, cond);
            return (r >= 0) ? r : -1;
        }
        int64_t v;
        if (eval_const_int(cond, &v)) return (int)(v != 0);
        return -1;  /* non-constant */
    }
    return -1;
}

/* =========================================================================
 * Public API
 * ====================================================================== */

/* v0.13: Re-evaluate an @-intrinsic with an optional generic substitution
 * context.  Used by cg to specialise compile-time queries inside generic
 * function/method bodies. */
int sema_eval_intrinsic_specialized(TyStore *ts, Scope *scope, FeDiagArr *diags,
                                    AstNode *intr,
                                    const char **pnames, Type **pvals, size_t np) {
    if (!ts || !intr) return -1;
    SemaCtx tmp = { .ts = ts, .file_scope = scope, .diags = diags };
    SS ss = {0};
    ss.ctx = &tmp;
    ss.scope = scope ? scope : NULL;
    ss.subst_pnames = pnames;
    ss.subst_pvals  = pvals;
    ss.subst_np     = np;
    int r = eval_intrinsic(&ss, intr);
    /* For @type_name, also recompute and cache the resolved string with
     * substitution applied (overwriting any template-time placeholder). */
    if (intr->kind == AST_AT_INTRINSIC &&
        intr->u.at_intrinsic.name &&
        strcmp(intr->u.at_intrinsic.name, "type_name") == 0) {
        char *fresh = compute_type_name(&ss, intr);
        if (fresh) {
            free(intr->u.at_intrinsic.resolved_str);
            intr->u.at_intrinsic.resolved_str = fresh;
            intr->u.at_intrinsic.resolved_set = true;
            r = 1;
        }
    } else if (r == 0 || r == 1) {
        /* Cache the bool result on the node so cg's expression-position
         * emit picks it up too. */
        intr->u.at_intrinsic.resolved_val = r;
        intr->u.at_intrinsic.resolved_set = true;
    }
    return r;
}

/* v0.13: walk a generic function-template body to populate sem_type on
 * every expression node, regardless of whether the function is generic.
 * Called by cg's specialization path so that cg_resolve_type has
 * TY_PARAM-typed annotations to substitute against the concrete instance.
 * Suppresses diagnostics by routing through a private FeDiagArr scratch
 * unless `diags` is supplied. */
void sema_func_template_body(TyStore *ts, Scope *parent_scope,
                             FeDiagArr *diags, AstNode *fn) {
    if (!fn || fn->kind != AST_FUNC_DEF || !fn->u.func_def.body) return;
    /* If caller passed NULL diags, allocate a local one we'll discard.
     * sema_err relies on a non-NULL FeDiagArr (fe_diag_push dereferences),
     * so we can't just pass NULL through. */
    FeDiagArr scratch = {0};
    bool own_diags = false;
    if (!diags) { diags = &scratch; own_diags = true; }
    SemaCtx tmp = { .ts = ts, .file_scope = parent_scope, .diags = diags };
    Scope *fs = fn->sem_scope ? fn->sem_scope : parent_scope;
    Type *ret = ty_from_ast(ts, fn->u.func_def.ret_type, fs, diags);
    SS ss = {0};
    ss.ctx = &tmp;
    ss.scope = fs;
    ss.ret_type = ret;
    sema_block(&ss, fn->u.func_def.body, fs);
    if (own_diags) {
        /* Discard any diagnostics produced — the call is just for type
         * annotations, not validation.  Free the underlying buffer. */
        free(scratch.data);
    }
}

SemaCtx *sema_ctx_new(TyStore *ts, Scope *file_scope, FeDiagArr *diags) {
    SemaCtx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) abort();
    ctx->ts         = ts;
    ctx->file_scope = file_scope;
    ctx->diags      = diags;
    return ctx;
}

void sema_ctx_free(SemaCtx *ctx) { free(ctx); }

bool sema_check_file(SemaCtx *ctx, AstNode *file) {
    if (!file) return true;
    size_t err_before = ctx->diags->len;

    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        switch (d->kind) {
        case AST_FUNC_DEF:
            sema_func(ctx, ctx->file_scope, d);
            break;
        case AST_STRUCT_DEF: {
            AstNode *sd = d;
            Scope *ss_s = sd->sem_scope ? (Scope*)sd->sem_scope : ctx->file_scope;
            for (size_t mj = 0; mj < sd->u.struct_def.methods.len; mj++)
                sema_func(ctx, ss_s, sd->u.struct_def.methods.data[mj]);
            break;
        }
        case AST_VAR_DECL: {
            SS ss = { ctx, ctx->file_scope, NULL, NULL, NULL, NULL, 0 };
            AstNode *vd = d;
            Type *dt = ty_from_ast(ctx->ts, vd->u.var_decl.type,
                                   ctx->file_scope, ctx->diags);
            Type *it = vd->u.var_decl.init
                     ? sema_expr(&ss, vd->u.var_decl.init) : NULL;
            if (!dt || dt->kind == TY_ERROR) dt = it ? it : ty_error(ctx->ts);
            vd->sem_type = dt;
            break;
        }
        default:
            break;
        }
    }

    return ctx->diags->len == err_before;
}
