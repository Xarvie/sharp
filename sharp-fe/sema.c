/*
 * sema.c — Phase 6 Semantic Analysis (base).
 */
#include "sema.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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
} SS;

/* =========================================================================
 * Phase 11 sema helper: substitute generic params in a Type*.
 * This mirrors cg.c's subst_type but lives in sema so field-type
 * resolution for generic struct instances works correctly.
 * ====================================================================== */
static Type *sema_subst(TyStore *ts, Type *t,
                         const char **pnames, Type **pvals, size_t np) {
    if (!t || np == 0) return t;
    switch (t->kind) {
    case TY_PARAM:
        for (size_t i = 0; i < np; i++)
            if (strcmp(t->u.param.name, pnames[i]) == 0) return pvals[i];
        return t;
    case TY_PTR:
        return ty_ptr(ts, sema_subst(ts, t->u.ptr.base, pnames, pvals, np));
    case TY_CONST:
        return ty_const(ts, sema_subst(ts, t->u.const_.base, pnames, pvals, np));
    case TY_ARRAY:
        return ty_array(ts, sema_subst(ts, t->u.array.base, pnames, pvals, np),
                        t->u.array.size);
    case TY_STRUCT:
        if (t->u.struct_.nargs > 0) {
            Type **na = malloc(t->u.struct_.nargs * sizeof *na);
            if (!na) abort();
            for (size_t i = 0; i < t->u.struct_.nargs; i++)
                na[i] = sema_subst(ts, t->u.struct_.args[i], pnames, pvals, np);
            Type *r = ty_struct_type(ts, t->u.struct_.name,
                                     na, t->u.struct_.nargs, t->u.struct_.decl);
            free(na);
            return r;
        }
        return t;
    default: return t;
    }
}

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
    Type *r = sema_subst(ts, t, pnames, recv_base->u.struct_.args, np);
    free(pnames);
    return r;
}

/* Phase 7 forward declarations */
/* Phase 9+10 forward declarations */
static void sema_defer_check(const AstNode *body, FeDiagArr *diags);
static int  eval_intrinsic(SS *ss, const AstNode *expr);
static int  eval_has_operator(SS *ss, const AstNode *expr);
static bool eval_const_int(const AstNode *expr, int64_t *val);

static Scope *struct_scope_of(TyStore *ts, Type *t, Scope *file_scope);
static const char *op_overload_name(SharpTokKind k);
static bool recv_object_is_const(Type *t);
static Type *sema_field_access_expr(SS *ss, AstNode *expr);
static Type *sema_method_call_expr(SS *ss, AstNode *expr);

/* =========================================================================
 * Diagnostic helpers
 * ====================================================================== */
static void sema_err(SS *ss, CppLoc loc, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    char *msg = malloc((size_t)(n + 1));
    if (!msg) abort();
    va_start(ap, fmt);
    vsnprintf(msg, (size_t)(n + 1), fmt, ap);
    va_end(ap);
    CppDiag d = { CPP_DIAG_ERROR, loc, msg };
    fe_diag_push(ss->ctx->diags, d);
}

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
     * AST-level qualifier stripped by ty_from_ast); only TY_CONST needs
     * peeling here. */
    while (a && a->kind == TY_CONST) a = a->u.const_.base;
    while (b && b->kind == TY_CONST) b = b->u.const_.base;
    if (!a || !b) return ts ? ty_int(ts) : a;
    /* double wins */
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
        /* Adding const to pointee is fine: int* → const int*. */
        if (ty_is_const(lb) && !ty_is_const(rb) &&
            ty_eq(lb->u.const_.base, rb)) return true;
        /* Dropping const from pointee is not fine: const int* → int*. */
        if (!ty_is_const(lb) && ty_is_const(rb)) return false;
        if (ty_eq(ty_unconst(ts, lb), ty_unconst(ts, rb))) return true;
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
                sema_err(ss, expr->loc,
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
        Type *lt_unconst = lt;
        if (lt_unconst && lt_unconst->kind == TY_CONST)
            lt_unconst = lt_unconst->u.const_.base;
        bool lhs_is_struct_value =
            lt_unconst && lt_unconst->kind == TY_STRUCT;
        const char *op_nm = lhs_is_struct_value ? op_overload_name(op) : NULL;
        if (op_nm) {
            Scope *ss_s = struct_scope_of(ts, lt, ss->scope);
            if (ss_s) {
                Symbol *osym = scope_lookup_local(ss_s, op_nm);
                if (osym && osym->decl && osym->decl->kind == AST_FUNC_DEF) {
                    Scope *mscope = osym->decl->type_ref
                                  ? (Scope*)osym->decl->type_ref : ss->scope;
                    Type *ret = ty_from_ast(ts, osym->decl->u.func_def.ret_type,
                                            mscope, NULL);
                    return sema_subst_for_struct(ts, lt, ret);
                }
                /* Struct found but operator not defined. */
                if (!ty_is_error(lt))
                    sema_err(ss, expr->loc,
                        "operator '%s' not defined for struct type",
                        op_nm + 8); /* skip "operator" prefix */
                return ty_error(ts);
            }
        }
    }

    /* Comparison → int */
    if (op == STOK_EQEQ || op == STOK_BANGEQ ||
        op == STOK_LT   || op == STOK_GT      ||
        op == STOK_LTEQ || op == STOK_GTEQ) {
        /* Phase R7: suppress cascade errors — when either operand has an
         * error type (e.g. result of calling a function-pointer variable
         * whose return type could not be resolved by sema), the comparison
         * is still valid C; let cc verify the actual types. */
        if (!ty_is_error(lt) && !ty_is_error(rt) &&
            (!ty_is_scalar(lt) || !ty_is_scalar(rt)))
            sema_err(ss, expr->loc, "comparison of non-scalar types");
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

    /* Bitwise on integers */
    if ((op == STOK_AMP || op == STOK_PIPE || op == STOK_CARET ||
         op == STOK_LTLT || op == STOK_GTGT) &&
        ty_is_integer(lu) && ty_is_integer(ru))
        return arith_conv(ts, lu, ru);

    /* Comma */
    if (op == STOK_COMMA) return rt;

    if (!ty_is_error(lt) && !ty_is_error(rt))
        sema_err(ss, expr->loc, "invalid operand types for binary operator");
    return ty_error(ts);
}

static Type *sema_unary(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    Type *ot = sema_expr(ss, expr->u.unary.operand);
    SharpTokKind op = expr->u.unary.op;

    if (op == STOK_BANG)  return ty_int(ts);
    if (op == STOK_TILDE) return ty_is_integer(ot) ? ot : ty_int(ts);
    if (op == STOK_MINUS || op == STOK_PLUS) {
        if (!ty_is_arithmetic(ot))
            sema_err(ss, expr->loc, "unary arithmetic on non-arithmetic type");
        return ty_is_arithmetic(ot) ? ot : ty_int(ts);
    }
    if (op == STOK_AMP) {
        /* C 6.3.2.1 ¶4: applying & to a function designator yields a
         * pointer to the function — identical to the implicit decay.
         * In Sharp's type model, SYM_FUNC idents already return
         * PTR(FUNC), so `&io_fclose` must NOT add another PTR layer.
         * For regular lvalues (variables, fields) the normal PTR wrap
         * is correct. */
        if (ot && ot->kind == TY_PTR && ot->u.ptr.base &&
            ot->u.ptr.base->kind == TY_FUNC) {
            return ot;   /* already PTR(FUNC) — address-of function is a no-op */
        }
        return ty_ptr(ts, ot);   /* & lvalue → T* */
    }
    if (op == STOK_STAR) {
        if (!ty_is_pointer(ot)) {
            sema_err(ss, expr->loc, "dereference of non-pointer");
            return ty_error(ts);
        }
        return ty_deref(ot);
    }
    if (op == STOK_PLUSPLUS || op == STOK_MINUSMINUS) {
        if (!ty_is_scalar(ot))
            sema_err(ss, expr->loc, "++ / -- on non-scalar");
        return ot;
    }
    return ot;
}

static Type *sema_call(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    /* Evaluate callee to determine function type. */
    Type *callee_t = sema_expr(ss, expr->u.call.callee);
    /* Evaluate all arguments (type-check them even if we can't verify). */
    for (size_t i = 0; i < expr->u.call.args.len; i++)
        sema_expr(ss, expr->u.call.args.data[i]);

    /* GCC builtins (`__builtin_expect`, etc.).  These are not declared
     * in source; sema_expr's AST_IDENT path returns int for the callee
     * but the call must also resolve to int (or whatever the builtin's
     * signature says) — we collapse them all to int, matching the
     * common cases used by Lua's `luai_likely`/`luai_unlikely` and
     * by glibc's `__builtin_offsetof`, etc.  Real signatures are
     * resolved by the C compiler when it processes the generated C. */
    if (expr->u.call.callee->kind == AST_IDENT &&
        strncmp(expr->u.call.callee->u.ident.name, "__builtin_", 10) == 0) {
        return ty_int(ts);
    }

    /* If callee is a FUNC type, return its return type. */
    if (callee_t && callee_t->kind == TY_FUNC) return callee_t->u.func.ret;
    /* Phase R7: calling through a function pointer: TY_PTR(TY_FUNC(ret,...)).
     * Strip one layer of TY_PTR (and optional TY_CONST) to reach TY_FUNC. */
    {
        Type *inner = callee_t;
        if (inner && inner->kind == TY_CONST)  inner = inner->u.const_.base;
        if (inner && inner->kind == TY_PTR)    inner = inner->u.ptr.base;
        if (inner && inner->kind == TY_CONST)  inner = inner->u.const_.base;
        if (inner && inner->kind == TY_FUNC)   return inner->u.func.ret;
    }
    /* For identifiers resolved to a SYM_FUNC decl, we can look up the ret type. */
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
                Scope *fscope = fn->type_ref ? (Scope*)fn->type_ref : ss->scope;
                Type *ret_raw = ty_from_ast(ts, fn->u.func_def.ret_type,
                                            fscope, NULL);
                if (ret_raw && ret_raw->kind == TY_PARAM) {
                    /* Find the param that matches the return type name. */
                    for (size_t pi = 0;
                         pi < fn->u.func_def.params.len && pi < expr->u.call.args.len;
                         pi++) {
                        AstNode *par = fn->u.func_def.params.data[pi];
                        if (!par) continue;
                        Scope *ps = fn->type_ref ? (Scope*)fn->type_ref : ss->scope;
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
    case AST_NULL_LIT:
        t = ty_ptr(ts, ty_void(ts)); break;

    /* Phase S5: GCC labels-as-values.  `&&label` has type `void *`.
     * The label name is not resolved here (it refers to a goto-target,
     * not a value-binding); cc validates that the label exists in the
     * enclosing function. */
    case AST_ADDR_OF_LABEL:
        t = ty_ptr(ts, ty_void(ts)); break;

    case AST_IDENT: {
        const char *name = expr->u.ident.name;
        /* 'this' is a special param — let scope lookup find it. */
        Symbol *sym = scope_lookup(ss->scope, name);
        if (!sym) {
            /* GCC builtins (`__builtin_expect`, `__builtin_offsetof`,
             * `__builtin_memcpy`, etc.) and pseudo-builtins from the
             * preprocessor are never declared explicitly in C source.
             * Treat any name starting with `__builtin_` as a callable
             * external — its type collapses to int (the C compiler
             * will resolve the real signature when compiling the
             * generated C output).  Lua's luaconf.h uses
             * `__builtin_expect` in luai_likely / luai_unlikely.
             * Phase R4: also accept GCC atomic builtins (__atomic_load_n,
             * __atomic_store_n, __atomic_compare_exchange_n, etc.) and
             * legacy sync builtins (__sync_fetch_and_add, etc.).  They are
             * never declared explicitly; cc resolves the real semantics. */
            if (strncmp(name, "__builtin_", 10) == 0 ||
                strncmp(name, "__atomic_",   9) == 0 ||
                strncmp(name, "__sync_",     7) == 0) {
                t = ty_int(ts);
                break;
            }
            /* Phase R5: GCC magic string variables — __PRETTY_FUNCTION__
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
            sema_err(ss, expr->loc, "undefined name '%s'", name);
            t = ty_error(ts);
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
                /* type_ref was set when we processed the VAR_DECL */
                t = decl ? (Type*)decl->type_ref : ty_error(ts);
                if (!t) t = decl ? ty_from_ast(ts, decl->u.var_decl.type,
                                                ss->scope, ss->ctx->diags)
                                 : ty_error(ts);
                break;
            case SYM_PARAM:
                if (decl && decl->kind == AST_PARAM_DECL) {
                    t = ty_from_ast(ts, decl->u.param_decl.type,
                                    ss->scope, ss->ctx->diags);
                } else if (strcmp(expr->u.ident.name, "this") == 0) {
                    /* 'this' in a struct method: ptr to enclosing struct.
                     * Phase 4 stores the FUNC_DEF as decl for 'this', so
                     * we derive its type from the enclosing struct scope. */
                    Scope *ss_s = ss->scope;
                    while (ss_s && ss_s->kind != SCOPE_STRUCT) ss_s = ss_s->parent;
                    if (ss_s && ss_s->owner && ss_s->owner->kind == AST_STRUCT_DEF) {
                        const char *sn = ss_s->owner->u.struct_def.name;
                        Type *st = ty_struct_type(ts, sn, NULL, 0, ss_s->owner);
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
                sema_err(ss, expr->loc, "'%s' is a type, not a value", name);
                t = ty_error(ts);
                break;
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
        if (!ty_is_scalar(cond_t))
            sema_err(ss, expr->loc, "ternary condition must be scalar");
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
        } else {
            /* Phase 7: check for operator[] overload. */
            Scope *ss_s = struct_scope_of(ts, base_t, ss->ctx->file_scope);
            Symbol *osym = ss_s ? scope_lookup_local(ss_s, "operator[]") : NULL;
            if (osym && osym->decl && osym->decl->kind == AST_FUNC_DEF) {
                t = ty_from_ast(ts, osym->decl->u.func_def.ret_type,
                                ss->scope, ss->ctx->diags);
            } else {
                if (!ty_is_error(base_t))
                    sema_err(ss, expr->loc, "subscript of non-pointer / non-indexable type");
                t = ty_error(ts);
            }
        }
        break;
    }

    case AST_CAST:
        sema_expr(ss, expr->u.cast.operand);
        t = ty_from_ast(ts, expr->u.cast.type, ss->scope, ss->ctx->diags);
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
                sema_err(ss, expr->loc, "@static_assert: %s", msg);
            }
            /* If result == -1, condition is non-constant → no compile-time error. */
            t = ty_void(ts);
        } else if (strcmp(iname, "has_operator") == 0) {
            /* @has_operator(T, op): compile-time bool query, no side effects. */
            t = ty_bool(ts);
        } else {
            sema_err(ss, expr->loc, "unknown @intrinsic '%s'", iname);
            t = ty_error(ts);
        }
        break;
    }

    case AST_STMT_EXPR: {
        /* Phase R4: GCC statement-expression ({ stmts; expr; }).
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
         * node (stored in type_ref, freed by scope_free_chain when the
         * parent scope is freed).
         *
         * Type of the whole expression = type of the last
         * expression-statement, or void if the block is empty / ends
         * with a non-expression statement. */
        AstNode *block = expr->u.stmt_expr.block;
        t = ty_void(ts);
        if (!block) break;

        /* Create and attach a block-level scope if not already set. */
        Scope *blk_scope = block->type_ref
                         ? (Scope *)block->type_ref
                         : scope_new(SCOPE_BLOCK, ss->scope, block);
        if (!block->type_ref) {
            block->type_ref = blk_scope;
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

    default:
        t = ty_error(ts);
        break;
    }

    if (!t) t = ty_error(ts);
    expr->type_ref = (void *)t;
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
        if (vd->u.var_decl.type && vd->u.var_decl.type->kind != AST_TYPE_AUTO) {
            decl_type = ty_from_ast(ts, vd->u.var_decl.type,
                                    ss->scope, ss->ctx->diags);
        }
        /* Infer init type first (important for auto). */
        Type *init_t = vd->u.var_decl.init
                     ? sema_expr(ss, vd->u.var_decl.init)
                     : NULL;
        /* Resolve the declared type. */
        if (!decl_type && init_t && !ty_is_error(init_t)) {
            decl_type = init_t; /* auto deduction */
        } else if (!decl_type) {
            decl_type = ty_error(ts);
        }
        /* Type-check init vs declared type. */
        if (init_t && !ty_is_error(decl_type) && !ty_is_error(init_t)) {
            if (!assign_compat(ts, decl_type, init_t))
                sema_err(ss, vd->loc,
                    "cannot initialize '%s' with value of type '%s'",
                    ty_kind_name(decl_type->kind), ty_kind_name(init_t->kind));
        }
        vd->type_ref = (void *)decl_type;
        break;
    }

    case AST_RETURN: {
        Type *ret = ss->ret_type;
        if (!ret) break; /* outside a function? */
        if (!stmt->u.return_.value) {
            if (!ty_is_void(ret))
                sema_err(ss, stmt->loc,
                    "non-void function must return a value");
        } else {
            Type *vt = sema_expr(ss, stmt->u.return_.value);
            if (ty_is_void(ret))
                sema_err(ss, stmt->loc,
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
                    sema_err(ss, stmt->loc,
                        "return type mismatch: expected %s, got %s",
                        ty_kind_name(ret->kind), ty_kind_name(vt->kind));
            }
        }
        break;
    }

    case AST_BLOCK: {
        Scope *bs = stmt->type_ref ? (Scope*)stmt->type_ref : ss->scope;
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
            cond->type_ref = ct_val
                ? (void*)ty_int(ts)    /* live = then_ */
                : (void*)ty_void(ts);  /* live = else_ */
            if (ct_val)
                sema_stmt(ss, stmt->u.if_.then_);   /* true  branch checked */
            else
                sema_stmt(ss, stmt->u.if_.else_);   /* false branch checked */
        } else {
            /* Runtime condition — check both branches as usual. */
            Type *ct = sema_expr(ss, cond);
            if (!ty_is_scalar(ct))
                sema_err(ss, stmt->loc, "if condition must be scalar");
            sema_stmt(ss, stmt->u.if_.then_);
            sema_stmt(ss, stmt->u.if_.else_);
        }
        break;
    }

    case AST_WHILE: {
        Type *ct = sema_expr(ss, stmt->u.while_.cond);
        if (!ty_is_scalar(ct))
            sema_err(ss, stmt->loc, "while condition must be scalar");
        sema_stmt(ss, stmt->u.while_.body);
        break;
    }

    case AST_FOR: {
        Scope *for_scope = stmt->type_ref ? (Scope*)stmt->type_ref : ss->scope;
        SS inner = *ss; inner.scope = for_scope;
        sema_stmt(&inner, stmt->u.for_.init);
        if (stmt->u.for_.cond) {
            Type *ct = sema_expr(&inner, stmt->u.for_.cond);
            if (!ty_is_scalar(ct))
                sema_err(ss, stmt->loc, "for condition must be scalar");
        }
        if (stmt->u.for_.post) sema_expr(&inner, stmt->u.for_.post);
        sema_stmt(&inner, stmt->u.for_.body);
        break;
    }

    case AST_DO_WHILE: {
        sema_stmt(ss, stmt->u.do_while.body);
        Type *ct = sema_expr(ss, stmt->u.do_while.cond);
        if (!ty_is_scalar(ct))
            sema_err(ss, stmt->loc, "do-while condition must be scalar");
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

    Scope *fs = fn->type_ref ? (Scope*)fn->type_ref : parent_scope;
    Type *ret  = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, fs, ctx->diags);
    SS ss = { ctx, fs, ret };
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
    if (!t) return NULL;
    /* strip top-level const */
    if (t->kind == TY_CONST) t = t->u.const_.base;
    /* strip pointer (for arrow access or pointer receiver) */
    if (t->kind == TY_PTR) {
        t = t->u.ptr.base;
        if (t && t->kind == TY_CONST) t = t->u.const_.base;
    }
    /* Phase R1: TY_ARRAY decays to pointer-to-element when used as
     * the operand of `.` or `->`.  Without this strip, `pair p[1];
     * p->x;` reports "no member 'x' in struct" — cJSON 1.7.18's
     * `printbuffer buffer[1]; buffer->buffer = …` hits this exactly.
     * Strip element-then-const to mirror the TY_PTR branch above. */
    if (t && t->kind == TY_ARRAY) {
        t = t->u.array.base;
        if (t && t->kind == TY_CONST) t = t->u.const_.base;
    }
    if (!t || t->kind != TY_STRUCT) return NULL;
    /* prefer decl-attached scope from Phase 4 */
    if (t->u.struct_.decl && t->u.struct_.decl->type_ref)
        return (Scope *)t->u.struct_.decl->type_ref;
    /* fallback: look up by name in file scope */
    if (file_scope) {
        Symbol *sym = scope_lookup_type(file_scope, t->u.struct_.name);
        if (sym && sym->decl && sym->decl->type_ref)
            return (Scope *)sym->decl->type_ref;
    }
    return NULL;
}

/* Map a binary operator token to its overload name, e.g. STOK_PLUS → "operator+". */
static const char *op_overload_name(SharpTokKind k) {
    switch (k) {
    case STOK_PLUS:     return "operator+";
    case STOK_MINUS:    return "operator-";
    case STOK_STAR:     return "operator*";
    case STOK_SLASH:    return "operator/";
    case STOK_PERCENT:  return "operator%";
    case STOK_EQEQ:     return "operator==";
    case STOK_BANGEQ:   return "operator!=";
    case STOK_LT:       return "operator<";
    case STOK_GT:       return "operator>";
    case STOK_LTEQ:     return "operator<=";
    case STOK_GTEQ:     return "operator>=";
    case STOK_AMP:      return "operator&";
    case STOK_PIPE:     return "operator|";
    case STOK_CARET:    return "operator^";
    case STOK_LTLT:     return "operator<<";
    case STOK_GTGT:     return "operator>>";
    default:            return NULL;
    }
}

/* Lookup a method/field symbol in the struct scope of recv_type. */
static Symbol *struct_member(TyStore *ts, Type *recv_type, const char *name,
                              Scope *file_scope) {
    Scope *ss = struct_scope_of(ts, recv_type, file_scope);
    if (!ss) return NULL;
    return scope_lookup_local(ss, name);
}

/* Check if recv is const (const T or const T* pointee). */
static bool recv_is_const(Type *t) {
    if (!t) return false;
    if (t->kind == TY_CONST) return true;                /* const T (value) */
    if (t->kind == TY_PTR && t->u.ptr.base &&
        t->u.ptr.base->kind == TY_CONST) return true;   /* T const* — wait, that's different */
    return false;
}

/* Check if the pointee (or value) of a receiver is const.
 * "const B*" or "const B" — the object referred to is const. */
static bool recv_object_is_const(Type *t) {
    if (!t) return false;
    if (t->kind == TY_CONST) return true;  /* const B (value) */
    /* B* const: pointer is const but object is not */
    /* const B*: pointee is const */
    if (t->kind == TY_PTR) {
        Type *base = t->u.ptr.base;
        if (base && base->kind == TY_CONST) return true; /* const B* */
    }
    return false;
}

/* Sema for field access: recv.field or recv->field */
static Type *sema_field_access_expr(SS *ss, AstNode *expr) {
    TyStore *ts = ss->ctx->ts;
    Type *recv_t = sema_expr(ss, expr->u.field_access.recv);
    bool arrow   = expr->u.field_access.arrow;
    const char *field = expr->u.field_access.field;

    /* Validate arrow: recv must be a pointer. */
    if (arrow) {
        Type *base = recv_t;
        if (base->kind == TY_CONST) base = base->u.const_.base;
        if (!base || !ty_is_pointer(base)) {
            sema_err(ss, expr->loc, "'->' requires pointer operand");
            return ty_error(ts);
        }
    }

    /* Check if recv is a type name (associated function call context). */
    if (expr->u.field_access.recv->kind == AST_IDENT) {
        const char *recv_name = expr->u.field_access.recv->u.ident.name;
        Symbol *rsym = scope_lookup(ss->scope, recv_name);
        if (rsym && rsym->kind == SYM_TYPE) {
            /* Type.field — look up in struct scope as a member. */
            Scope *ss_s = rsym->decl ? (Scope*)rsym->decl->type_ref : NULL;
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
            sema_err(ss, expr->loc,
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
        Type *base_t2 = recv_t;
        if (base_t2 && base_t2->kind == TY_CONST)  base_t2 = base_t2->u.const_.base;
        if (base_t2 && base_t2->kind == TY_PTR)    base_t2 = base_t2->u.ptr.base;
        /* Phase R1: array decays to ptr-to-element for `.`/`->`. */
        if (base_t2 && base_t2->kind == TY_ARRAY)  base_t2 = base_t2->u.array.base;
        if (base_t2 && base_t2->kind == TY_CONST)  base_t2 = base_t2->u.const_.base;
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
    Type *recv_t = sema_expr(ss, expr->u.method_call.recv);
    bool arrow   = expr->u.method_call.arrow;
    const char *method = expr->u.method_call.method;

    /* Evaluate arguments regardless. */
    for (size_t i = 0; i < expr->u.method_call.args.len; i++)
        sema_expr(ss, expr->u.method_call.args.data[i]);

    /* Validate arrow: recv must be a pointer. */
    if (arrow) {
        Type *base = recv_t;
        if (base && base->kind == TY_CONST) base = base->u.const_.base;
        if (!base || !ty_is_pointer(base)) {
            sema_err(ss, expr->loc, "'->' requires pointer operand");
            return ty_error(ts);
        }
    }

    /* Check if recv is a type name (associated function). */
    if (expr->u.method_call.recv->kind == AST_IDENT) {
        const char *recv_name = expr->u.method_call.recv->u.ident.name;
        Symbol *rsym = scope_lookup(ss->scope, recv_name);
        if (rsym && rsym->kind == SYM_TYPE) {
            Scope *ss_s = rsym->decl ? (Scope*)rsym->decl->type_ref : NULL;
            if (ss_s) {
                Symbol *msym = scope_lookup_local(ss_s, method);
                if (msym && msym->decl && msym->decl->kind == AST_FUNC_DEF) {
                    AstNode *fn = msym->decl;
                    return ty_from_ast(ts, fn->u.func_def.ret_type,
                                       ss->scope, ss->ctx->diags);
                }
            }
            sema_err(ss, expr->loc,
                "no associated function '%s' in type", method);
            return ty_error(ts);
        }
    }

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
            sema_err(ss, expr->loc,
                "no method '%s' in struct", method);
        return ty_error(ts);
    }

    AstNode *fn = msym->decl;
    if (!fn || fn->kind != AST_FUNC_DEF) return ty_error(ts);

    /* const receiver check. */
    bool is_const_recv = recv_object_is_const(recv_t);
    if (is_const_recv && !fn->u.func_def.is_const_method) {
        sema_err(ss, expr->loc,
            "cannot call non-const method '%s' on const receiver", method);
    }

    /* Use the method's own scope for return type resolution so that generic
     * type params (T) are found.  Suppress diagnostics here — unknown 'T'
     * is expected for uninstantiated generics; substitution handles it. */
    Scope *mscope = fn->type_ref ? (Scope*)fn->type_ref : ss->scope;
    Type *ret_t = ty_from_ast(ts, fn->u.func_def.ret_type, mscope, NULL);
    /* Substitute generic params if receiver is a concrete instantiation. */
    Type *base_t3 = recv_t;
    if (base_t3 && base_t3->kind == TY_CONST) base_t3 = base_t3->u.const_.base;
    if (base_t3 && base_t3->kind == TY_PTR)   base_t3 = base_t3->u.ptr.base;
    if (base_t3 && base_t3->kind == TY_CONST) base_t3 = base_t3->u.const_.base;
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

/* Try to evaluate an expression as a compile-time integer constant.
 * Returns true and sets *val on success, false if not a constant. */
static bool eval_const_int(const AstNode *expr, int64_t *val) {
    if (!expr) return false;
    switch (expr->kind) {
    case AST_INT_LIT:
        *val = expr->u.int_lit.val;
        return true;
    case AST_UNARY:
        if (expr->u.unary.op == STOK_MINUS) {
            int64_t v;
            if (eval_const_int(expr->u.unary.operand, &v)) {
                *val = -v; return true;
            }
        }
        return false;
    default:
        return false;
    }
}

/* Map a SharpTokKind (binary operator) to its symbol string for "operator<sym>". */
static const char *op_tok_to_sym(SharpTokKind k) {
    switch (k) {
    case STOK_PLUS:   return "+";  case STOK_MINUS:  return "-";
    case STOK_STAR:   return "*";  case STOK_SLASH:  return "/";
    case STOK_PERCENT:return "%";  case STOK_EQEQ:   return "==";
    case STOK_BANGEQ: return "!="; case STOK_LT:     return "<";
    case STOK_GT:     return ">";  case STOK_LTEQ:   return "<=";
    case STOK_GTEQ:   return ">="; case STOK_AMPAMP: return "&&";
    case STOK_PIPEPIPE:return "||";case STOK_AMP:    return "&";
    case STOK_PIPE:   return "|";  case STOK_CARET:  return "^";
    case STOK_LTLT:   return "<<"; case STOK_GTGT:   return ">>";
    case STOK_LBRACKET:return "[]";
    default: return NULL;
    }
}

/* Evaluate @has_operator(TypeName, op_symbol) in current scope.
 * Returns 1 if the named struct has the operator, 0 otherwise.
 *
 * The op_symbol argument is parsed as an expression — the parser sees:
 *   +   → UNARY(STOK_PLUS, dummy)
 *   ==  → BINOP(dummy, STOK_EQEQ, dummy)   (dummy = IDENT("?") or INT_LIT(0))
 * We extract the operator from whatever AST node the parser produced.
 */
static int eval_has_operator(SS *ss, const AstNode *expr) {
    if (!expr || expr->u.at_intrinsic.args.len < 2) return 0;
    AstNode *type_arg = expr->u.at_intrinsic.args.data[0];
    AstNode *op_arg   = expr->u.at_intrinsic.args.data[1];

    /* Resolve type name → struct scope */
    const char *type_name = NULL;
    if (type_arg->kind == AST_IDENT)
        type_name = type_arg->u.ident.name;
    if (!type_name) return 0;

    Symbol *sym = scope_lookup(ss->scope, type_name);
    if (!sym || sym->kind != SYM_TYPE || !sym->decl) return 0;

    Scope *ss_s = (Scope*)sym->decl->type_ref;
    if (!ss_s) return 0;

    /* Extract the operator symbol string from the parsed op_arg node. */
    char opname[64] = {0};
    const char *sym_str = NULL;

    if (op_arg->kind == AST_UNARY) {
        /* "+" parsed as UNARY(PLUS, dummy) */
        sym_str = op_tok_to_sym(op_arg->u.unary.op);
    } else if (op_arg->kind == AST_BINOP) {
        /* "==" parsed as BINOP(dummy, EQEQ, dummy) */
        sym_str = op_tok_to_sym(op_arg->u.binop.op);
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

    return scope_lookup_local(ss_s, opname) != NULL;
}

/* Evaluate a @static_assert or @has_operator expression.
 * Returns the result as a compile-time boolean (0 or 1),
 * or -1 if the expression is not a compile-time constant. */
static int eval_intrinsic(SS *ss, const AstNode *expr) {
    if (!expr || expr->kind != AST_AT_INTRINSIC) return -1;
    const char *name = expr->u.at_intrinsic.name;

    if (strcmp(name, "has_operator") == 0)
        return eval_has_operator(ss, expr);

    if (strcmp(name, "static_assert") == 0) {
        /* @static_assert evaluates to void/1; the check is side-effectful. */
        if (expr->u.at_intrinsic.args.len < 1) return -1;
        AstNode *cond = expr->u.at_intrinsic.args.data[0];
        /* Condition may itself be @has_operator */
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
            /* Phase 8: run sema on all struct methods so type_ref is set. */
            AstNode *sd = d;
            Scope *ss_s = sd->type_ref ? (Scope*)sd->type_ref : ctx->file_scope;
            for (size_t mj = 0; mj < sd->u.struct_def.methods.len; mj++)
                sema_func(ctx, ss_s, sd->u.struct_def.methods.data[mj]);
            break;
        }
        case AST_VAR_DECL: {
            /* File-level variable declaration. */
            SS ss = { ctx, ctx->file_scope, NULL };
            AstNode *vd = d;
            Type *dt = ty_from_ast(ctx->ts, vd->u.var_decl.type,
                                   ctx->file_scope, ctx->diags);
            Type *it = vd->u.var_decl.init
                     ? sema_expr(&ss, vd->u.var_decl.init) : NULL;
            if (!dt || dt->kind == TY_ERROR) dt = it ? it : ty_error(ctx->ts);
            vd->type_ref = dt;
            break;
        }
        default:
            break;
        }
    }

    return ctx->diags->len == err_before;
}
