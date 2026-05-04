/*
 * cg.c — Phase 8 Code Generation: AST → C11 source.
 */
#include "cg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* =========================================================================
 * String Builder
 * ====================================================================== */
typedef struct { char *buf; size_t len, cap; } CgSB;

static void cgb_push(CgSB *sb, char c) {
    if (sb->len + 1 >= sb->cap) {
        sb->cap = sb->cap ? sb->cap * 2 : 256;
        sb->buf = realloc(sb->buf, sb->cap);
        if (!sb->buf) abort();
    }
    sb->buf[sb->len++] = c;
    sb->buf[sb->len]   = '\0';
}

static void cgb_puts(CgSB *sb, const char *s) {
    for (; *s; s++) cgb_push(sb, *s);
}

static void cgb_printf(CgSB *sb, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n <= 0) return;
    while (sb->len + (size_t)n + 1 >= sb->cap) {
        sb->cap = sb->cap ? sb->cap * 2 : 256;
        sb->buf = realloc(sb->buf, sb->cap);
        if (!sb->buf) abort();
    }
    va_start(ap, fmt);
    vsnprintf(sb->buf + sb->len, (size_t)(n + 1), fmt, ap);
    va_end(ap);
    sb->len += (size_t)n;
}

static char *cgb_take(CgSB *sb) {
    char *s = sb->buf ? sb->buf : cpp_xstrdup("");
    sb->buf = NULL; sb->len = sb->cap = 0;
    return s;
}

/* =========================================================================
 * CgCtx
 * ====================================================================== */
struct CgCtx {
    TyStore    *ts;
    Scope      *file_scope;
    CgSB        out;
    int         indent;
    const char *cur_struct;    /* name of struct being emitted (for mangling) */
    /* Phase 11 */
    const AstNode *file_ast;  /* root AST for finding generic struct defs */
    char      **inst_names;   /* mangled names of emitted specializations   */
    size_t      ninsts;
    size_t      insts_cap;
    /* Generic function specialization context (active during cg_emit_spec_func) */
    const char **gp_names;   /* param name T, U, ... */
    Type       **gp_vals;    /* concrete type for each param */
    size_t       ngp;
    Scope       *spec_scope; /* function scope active during specialization */
    /* Generic function instance dedup */
    char      **gfn_names;
    size_t      ngfn;
    size_t      gfn_cap;
};

/* Phase 11 forward declarations */
static char *cg_mangle_inst(const char *sname, Type **args, size_t nargs);
static void  cgb_mangle_type(CgSB *sb, Type *t);
static const char *cg_gfunc_mangle_for_call(CgCtx *ctx, const char *fname,
                                              const AstNode *call);

/* Resolve scope for type lookups: use spec_scope during specialization. */
static Scope *cg_type_scope(const CgCtx *ctx) {
    return ctx->spec_scope ? ctx->spec_scope : ctx->file_scope;
}

CgCtx *cg_ctx_new(TyStore *ts, Scope *file_scope) {
    CgCtx *ctx = calloc(1, sizeof *ctx);
    if (!ctx) abort();
    ctx->ts         = ts;
    ctx->file_scope = file_scope;
    return ctx;
}

void cg_ctx_free(CgCtx *ctx) {
    if (!ctx) return;
    free(ctx->out.buf);
    for (size_t i = 0; i < ctx->ninsts; i++) free(ctx->inst_names[i]);
    free(ctx->inst_names);
    for (size_t i = 0; i < ctx->ngfn; i++) free(ctx->gfn_names[i]);
    free(ctx->gfn_names);
    free(ctx);
}

/* =========================================================================
 * Indentation helpers
 * ====================================================================== */
static void cg_indent(CgCtx *ctx) {
    for (int i = 0; i < ctx->indent; i++) cgb_puts(&ctx->out, "    ");
}
static void cg_nl(CgCtx *ctx)        { cgb_push(&ctx->out, '\n'); }
static void cg_puts(CgCtx *ctx, const char *s) { cgb_puts(&ctx->out, s); }
static void cg_printf(CgCtx *ctx, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap); va_end(ap);
    if (n <= 0) return;
    while (ctx->out.len + (size_t)n + 1 >= ctx->out.cap) {
        ctx->out.cap = ctx->out.cap ? ctx->out.cap * 2 : 256;
        ctx->out.buf = realloc(ctx->out.buf, ctx->out.cap);
        if (!ctx->out.buf) abort();
    }
    va_start(ap, fmt);
    vsnprintf(ctx->out.buf + ctx->out.len, (size_t)(n+1), fmt, ap);
    va_end(ap);
    ctx->out.len += (size_t)n;
}

/* =========================================================================
 * S1: detect outer-layer `volatile` on an AST type tree.
 *
 * The Sharp Type* model interns const through TY_CONST but deliberately
 * does not track volatile (it has no semantic effect on type
 * compatibility — the C compiler enforces access ordering).  To preserve
 * the volatile keyword in the generated C, we walk the AST type tree
 * looking for AST_TYPE_VOLATILE at the outer layer (skipping any
 * intervening AST_TYPE_CONST wrappers).  Returns true iff found.  We do
 * not handle volatile-on-pointee (`int * volatile p`) — rare in
 * practice and harmless to drop because the qualifier affects
 * code-generation rules cc applies regardless of how we present the
 * type.
 * ====================================================================== */
static bool ast_type_outer_is_volatile(const AstNode *ty) {
    while (ty) {
        if (ty->kind == AST_TYPE_VOLATILE) return true;
        if (ty->kind == AST_TYPE_CONST)    { ty = ty->u.type_const.base; continue; }
        return false;
    }
    return false;
}

/* =========================================================================
 * Type generation
 * (Outputs the C declaration prefix; name is added separately)
 * ====================================================================== */
static void cg_type(CgCtx *ctx, Type *t) {
    if (!t) { cg_puts(ctx, "/* ?type */ void"); return; }
    switch (t->kind) {
    case TY_ERROR:    cg_puts(ctx, "/* error */ int"); break;
    case TY_VOID:     cg_puts(ctx, "void");            break;
    case TY_BOOL:     cg_puts(ctx, "_Bool");           break;
    case TY_CHAR:     cg_puts(ctx, "char");            break;
    case TY_SHORT:    cg_puts(ctx, "short");           break;
    case TY_INT:      cg_puts(ctx, "int");             break;
    case TY_LONG:     cg_puts(ctx, "long");            break;
    case TY_LONGLONG: cg_puts(ctx, "long long");       break;
    case TY_UCHAR:    cg_puts(ctx, "unsigned char");   break;
    case TY_USHORT:   cg_puts(ctx, "unsigned short");  break;
    case TY_UINT:     cg_puts(ctx, "unsigned int");    break;
    case TY_ULONG:    cg_puts(ctx, "unsigned long");   break;
    case TY_ULONGLONG:cg_puts(ctx, "unsigned long long"); break;
    case TY_FLOAT:    cg_puts(ctx, "float");           break;
    case TY_DOUBLE:   cg_puts(ctx, "double");          break;
    case TY_CONST: {
        /* Phase R1: C const placement is positional, not just a leading
         * keyword.  The previous "const " + recurse always put `const`
         * on the LEFT, which is wrong when the type is a const-qualified
         * pointer:
         *
         *   const T   (TY_CONST(scalar))      → "const T"          ✓
         *   const T * (TY_PTR(TY_CONST(T)))   → "const T *"        ✓ (TY_PTR arm)
         *   T * const (TY_CONST(TY_PTR(T)))   → must be "T * const"
         *
         * cJSON 1.7.18 has `cJSON * const item` everywhere; under the
         * old emission it became `const cJSON * item`, and cc rejected
         * the function body's `item->field = …` writes against the
         * wrong-sided const.  Emit `* const` on the right when const
         * wraps a pointer; otherwise keep the leading-keyword form. */
        Type *inner = t->u.const_.base;
        if (inner && inner->kind == TY_PTR) {
            cg_type(ctx, inner);     /* "T *" */
            cg_puts(ctx, " const");  /* → "T * const" */
        } else {
            cg_puts(ctx, "const ");
            cg_type(ctx, inner);
        }
        break;
    }
    case TY_PTR: {
        /* Phase R3: pointer-to-function — `ret (*)(params)` form.
         * cg_decl already has this for named declarators (line 299+);
         * cg_type's plain-TY_PTR arm would emit `func *` which isn't
         * a syntactically valid C type expression.  Detect 1+ PTR
         * layers ending in TY_FUNC and emit the abstract form
         * matching what parse_type now accepts.  sqlite3.c casts
         * through `(int (*)(int, uid_t, gid_t)) ptr` to invoke
         * variadic syscall pointers. */
        int nstars = 0;
        const Type *c = t;
        while (c && c->kind == TY_PTR) { nstars++; c = c->u.ptr.base; }
        if (c && c->kind == TY_FUNC) {
            cg_type(ctx, c->u.func.ret);
            cg_puts(ctx, " (");
            for (int i = 0; i < nstars; i++) cg_puts(ctx, "*");
            cg_puts(ctx, ")(");
            if (c->u.func.nparams == 0) {
                cg_puts(ctx, "void");
            } else {
                for (size_t i = 0; i < c->u.func.nparams; i++) {
                    if (i) cg_puts(ctx, ", ");
                    cg_type(ctx, c->u.func.params[i]);
                }
            }
            cg_puts(ctx, ")");
            break;
        }
        cg_type(ctx, t->u.ptr.base);
        cg_puts(ctx, " *");
        break;
    }
    case TY_ARRAY:
        /* Fallback: in expression contexts (sizeof, cast, function param),
         * arrays decay to pointers.  cg_decl() handles the variable-decl
         * form `T name[N]` separately. */
        cg_type(ctx, t->u.array.base);
        cg_puts(ctx, " *");
        break;
    case TY_STRUCT:
        if (t->u.struct_.nargs > 0) {
            char *mn = cg_mangle_inst(t->u.struct_.name,
                                       t->u.struct_.args, t->u.struct_.nargs);
            cg_puts(ctx, mn);
            free(mn);
        } else {
            /* C tag namespace allows `struct X` and `int X(...)` to
             * coexist (see scope.c S5 fall-through registering both).
             * When the struct name is also bound to an ordinary
             * identifier in this scope, the auto-emitted
             * `typedef struct X X;` (cg pass 1) collides with that
             * function/variable.  Detect the collision via the
             * symbol chain and prefix with `struct ` to disambiguate
             * — this requires the source-level `typedef X` to be
             * skipped in the same case (handled in pass-1 below). */
            const char *nm = t->u.struct_.name;
            Scope *fs = cg_type_scope(ctx);
            bool has_value_symbol = false;
            if (fs) {
                Symbol *sym = scope_lookup_local(fs, nm);
                for (; sym; sym = sym->next) {
                    if (strcmp(sym->name, nm) != 0) continue;
                    if (sym->kind == SYM_FUNC || sym->kind == SYM_VAR) {
                        has_value_symbol = true;
                        break;
                    }
                }
            }
            if (has_value_symbol) cg_puts(ctx, "struct ");
            cg_puts(ctx, nm);
        }
        break;
    case TY_PARAM:
        /* During generic function specialization, substitute concrete type. */
        for (size_t _i = 0; _i < ctx->ngp; _i++) {
            if (strcmp(t->u.param.name, ctx->gp_names[_i]) == 0) {
                cg_type(ctx, ctx->gp_vals[_i]);
                return;
            }
        }
        /* Fallback: emit the param name (best-effort for unresolved cases). */
        cg_puts(ctx, t->u.param.name);
        break;
    default:
        cg_puts(ctx, "void /*?*/");
        break;
    }
}

/* Write a full declaration "Type name", handling:
 *   - pointer: "char *name"
 *   - 1-D array: "int name[3]"
 *   - N-D array: "int name[3][4]"  (innermost element type first, then name,
 *                                    then suffixes outer→inner)
 *
 * For arrays, C's grammar is `T base[outer][inner]`.  Our TY_ARRAY tree is
 * outermost-first (a[3][4] → array(3, array(4, T))), so we emit the
 * innermost element type, then the name, then the array suffixes by
 * walking the chain outermost→innermost.
 */
static void cg_decl(CgCtx *ctx, Type *t, const char *name) {
    /* Detect array type (possibly nested) */
    if (t && t->kind == TY_ARRAY) {
        /* Find innermost element type */
        const Type *cursor = t;
        while (cursor && cursor->kind == TY_ARRAY) {
            cursor = cursor->u.array.base;
        }
        /* Phase R3: array of function-pointer needs the nested
         * declarator form
         *
         *     ret (* name[N])(args)
         *
         * Plain `cg_type` would emit `ret (*)(args)` and the caller
         * would tack `name[N]` after — C rejects that.  Detect when
         * the innermost element peels through PTR layers down to
         * TY_FUNC and switch to the nested shape.  Lua hits this
         * with `static const lua_CFunction searchers[] = {...};`.
         *
         * We additionally peel a single layer of TY_CONST/TY_VOLATILE
         * around the pointer so that `const cb_fn arr[]` produces
         * `ret (* const name[])(args)` (the `const` decorates the
         * pointer itself; the canonical place is between `*` and
         * the name). */
        bool elem_const = false;
        const Type *elem = cursor;
        if (elem && elem->kind == TY_CONST) {
            elem_const = true;
            elem = elem->u.const_.base;
        }
        int elem_nstars = 0;
        const Type *peel = elem;
        while (peel && peel->kind == TY_PTR) { elem_nstars++; peel = peel->u.ptr.base; }
        if (peel && peel->kind == TY_FUNC) {
            cg_type(ctx, peel->u.func.ret);
            cg_puts(ctx, " (");
            for (int i = 0; i < elem_nstars; i++) cg_puts(ctx, "*");
            if (elem_const) cg_puts(ctx, " const");
            if (name && *name) {
                if (elem_const) cg_puts(ctx, " ");
                cg_puts(ctx, name);
            }
            /* Array suffixes go INSIDE the parens, between name and
             * the closing `)`. */
            const Type *suff = t;
            while (suff && suff->kind == TY_ARRAY) {
                long sz = suff->u.array.size;
                if (sz < 0) cg_puts(ctx, "[]");
                else        cg_printf(ctx, "[%ld]", sz);
                suff = suff->u.array.base;
            }
            cg_puts(ctx, ")(");
            if (peel->u.func.nparams == 0) {
                cg_puts(ctx, "void");
            } else {
                for (size_t i = 0; i < peel->u.func.nparams; i++) {
                    if (i) cg_puts(ctx, ", ");
                    cg_type(ctx, peel->u.func.params[i]);
                }
            }
            cg_puts(ctx, ")");
            return;
        }
        cg_type(ctx, (Type *)cursor);
        if (name && *name) cg_printf(ctx, " %s", name);
        else cg_puts(ctx, " ");
        /* Emit suffixes outermost→innermost (= source order) */
        cursor = t;
        while (cursor && cursor->kind == TY_ARRAY) {
            long sz = cursor->u.array.size;
            if (sz < 0) cg_puts(ctx, "[]");
            else        cg_printf(ctx, "[%ld]", sz);
            cursor = cursor->u.array.base;
        }
        return;
    }

    /* S1: function-pointer declaration.  C requires the declarator-style
     *     ret-type ( * name )( param-types )
     * because plain `cg_type` emission would lose the parameter list (TY_FUNC
     * has no compact prefix form).  We handle the common "1+ pointers to
     * function" pattern by counting leading PTR layers down to a TY_FUNC.
     * Returns with the entire declaration emitted; the caller appends `;`.
     *
     * Phase R5: also handle TY_CONST(TY_PTR(TY_FUNC(...))) — a const
     * function pointer, e.g. `compressFunc_t const compress`.  ISO C
     * requires the const to sit between the `*` and the name:
     *     int (* const compress)(void *, ...)
     * not:
     *     int (*)(void *, ...) const compress   ← rejected by cc */
    if (t && t->kind == TY_CONST) {
        const Type *inner = t->u.const_.base;
        if (inner) {
            int nstars = 0;
            const Type *c = inner;
            while (c && c->kind == TY_PTR) { nstars++; c = c->u.ptr.base; }
            if (nstars > 0 && c && c->kind == TY_FUNC) {
                /* const function-pointer: ret (* const name)(params) */
                cg_type(ctx, c->u.func.ret);
                cg_puts(ctx, " (");
                for (int i = 0; i < nstars; i++) cg_puts(ctx, "*");
                cg_puts(ctx, " const");
                if (name && *name) cg_printf(ctx, " %s", name);
                cg_puts(ctx, ")(");
                if (c->u.func.nparams == 0) {
                    cg_puts(ctx, "void");
                } else {
                    for (size_t i = 0; i < c->u.func.nparams; i++) {
                        if (i) cg_puts(ctx, ", ");
                        cg_type(ctx, c->u.func.params[i]);
                    }
                }
                cg_puts(ctx, ")");
                return;
            }
        }
    }
    if (t && t->kind == TY_PTR) {
        int nstars = 0;
        const Type *c = t;
        while (c && c->kind == TY_PTR) { nstars++; c = c->u.ptr.base; }
        if (c && c->kind == TY_FUNC) {
            /* Return type. */
            cg_type(ctx, c->u.func.ret);
            cg_puts(ctx, " (");
            for (int i = 0; i < nstars; i++) cg_puts(ctx, "*");
            if (name && *name) cg_puts(ctx, name);
            cg_puts(ctx, ")(");
            if (c->u.func.nparams == 0) {
                cg_puts(ctx, "void");
            } else {
                for (size_t i = 0; i < c->u.func.nparams; i++) {
                    if (i) cg_puts(ctx, ", ");
                    cg_type(ctx, c->u.func.params[i]);
                }
            }
            cg_puts(ctx, ")");
            return;
        }
    }

    /* For pointer types, the * sits between type and name. */
    cg_type(ctx, t);
    if (name && *name) cg_printf(ctx, " %s", name);
}

/* =========================================================================
 * Operator mangling helper
 * ====================================================================== */
static const char *op_suffix(const char *sym) {
    if (!sym) return "op";
    if (!strcmp(sym, "+"))  return "add";
    if (!strcmp(sym, "-"))  return "sub";
    if (!strcmp(sym, "*"))  return "mul";
    if (!strcmp(sym, "/"))  return "div";
    if (!strcmp(sym, "%"))  return "mod";
    if (!strcmp(sym, "==")) return "eq";
    if (!strcmp(sym, "!=")) return "ne";
    if (!strcmp(sym, "<"))  return "lt";
    if (!strcmp(sym, ">"))  return "gt";
    if (!strcmp(sym, "<=")) return "le";
    if (!strcmp(sym, ">=")) return "ge";
    if (!strcmp(sym, "[]")) return "idx";
    if (!strcmp(sym, "&"))  return "band";
    if (!strcmp(sym, "|"))  return "bor";
    if (!strcmp(sym, "^"))  return "bxor";
    if (!strcmp(sym, "<<")) return "shl";
    if (!strcmp(sym, ">>")) return "shr";
    return "op";
}

/* Emit the mangled C name for a struct method/operator. */
static void cg_method_name(CgCtx *ctx, const char *sname, const char *mname) {
    if (strncmp(mname, "operator", 8) == 0) {
        const char *sym = mname + 8;  /* e.g. "+" from "operator+" */
        cg_printf(ctx, "%s__op_%s", sname, op_suffix(sym));
    } else {
        cg_printf(ctx, "%s__%s", sname, mname);
    }
}

/* =========================================================================
 * Forward declarations (expression codegen)
 * ====================================================================== */
static void cg_expr(CgCtx *ctx, const AstNode *expr);
static const char *cg_gfunc_mangle_for_call(CgCtx *ctx, const char *fname,
                                              const AstNode *call_expr);
static void cg_block(CgCtx *ctx, const AstNode *block);
static void cg_stmt(CgCtx *ctx, const AstNode *stmt,
                    const AstNode **defers, size_t ndefers);
static void cg_struct(CgCtx *ctx, const AstNode *sd);
static const char *binop_str(SharpTokKind op);
static void cg_func(CgCtx *ctx, const AstNode *fn, const char *sname);

/* Emit only the declaration (prototype) of a function, without the body.
 * Used by Pass 1c so that global variable initialisers that reference
 * function pointers can see the function names as declared. */
static void cg_func_decl(CgCtx *ctx, const AstNode *fn, const char *sname) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;
    if (fn->u.func_def.generic_params.len > 0) return;
    /* If already bodyless (extern decl), cg_func emits `;` anyway. */
    if (!fn->u.func_def.body) {
        cg_func(ctx, fn, sname);
        return;
    }
    /* For functions with a body we need to emit just the prototype.
     * Temporarily synthesise a fake node with body=NULL by shallow copy. */
    AstNode tmp = *fn;
    tmp.u.func_def.body = NULL;
    cg_func(ctx, &tmp, sname);
}

/* Emit a single struct/union field declaration from the AST field node.
 * Unlike cg_decl (which emits from the Type*), this path preserves the
 * original array-size expression so enum-valued sizes like `TM_N` or
 * `LUAI_NUMTAGS` are emitted correctly instead of as `[]`. */
/* Emit a constant expression (array size, enum value, bit-field width)
 * directly from the AST without requiring sema type annotations.
 * Handles the subset of expressions that appear as static constant sizes
 * in C declarations: integer literals, identifiers (enum names, macros),
 * casts, sizeof, and binary arithmetic.  Falls back to a numeric 0 for
 * unrecognised nodes — the C compiler will report a real error if the
 * expression is actually invalid; we preserve the shape so it can. */
static void cg_const_expr(CgCtx *ctx, const AstNode *e) {
    if (!e) { cg_puts(ctx, "0"); return; }
    switch (e->kind) {
    case AST_INT_LIT:
        if (e->u.int_lit.is_longlong)
            cg_printf(ctx, e->u.int_lit.is_unsigned ? "%lluULL" : "%lldLL",
                      (unsigned long long)e->u.int_lit.val);
        else if (e->u.int_lit.is_long)
            cg_printf(ctx, e->u.int_lit.is_unsigned ? "%luUL" : "%ldL",
                      (unsigned long)e->u.int_lit.val);
        else if (e->u.int_lit.is_unsigned)
            /* Phase R6: plain `U` suffix (no L/LL) must emit `unsigned int`,
             * NOT `unsigned long long`.  Hash functions (e.g. lz4's
             * `sequence * 2654435761U`) rely on 32-bit wrapping truncation:
             * `uint32_t * uint32_t → uint32_t`.  Emitting `%lluU` widens to
             * `uint32_t * uint64_t → uint64_t`, destroying overflow semantics
             * and producing hash indices that overflow the table.
             * If the value overflows uint32_t (val > UINT_MAX), fall back to
             * ULL to avoid silent truncation. */
            if ((uint64_t)e->u.int_lit.val <= 0xFFFFFFFFULL)
                cg_printf(ctx, "%uU", (unsigned int)e->u.int_lit.val);
            else
                cg_printf(ctx, "%lluULL", (unsigned long long)e->u.int_lit.val);
        else
            cg_printf(ctx, "%lld", (long long)e->u.int_lit.val);
        break;
    case AST_IDENT:
        cg_puts(ctx, e->u.ident.name ? e->u.ident.name : "0");
        break;
    case AST_CAST: {
        /* (type)expr — emit with the declared cast type, not the sema type. */
        cg_puts(ctx, "(");
        AstNode *ct = e->u.cast.type;
        if (ct) {
            /* Try to get the cast type from the AST directly. */
            Type *cty = ty_from_ast(ctx->ts, ct, cg_type_scope(ctx), NULL);
            if (cty && !ty_is_error(cty)) {
                cg_type(ctx, cty);
            } else {
                cg_puts(ctx, "int");  /* fallback */
            }
        } else {
            cg_puts(ctx, "int");
        }
        cg_puts(ctx, ")(");
        cg_const_expr(ctx, e->u.cast.operand);
        cg_puts(ctx, ")");
        break;
    }
    case AST_SIZEOF:
        cg_puts(ctx, "sizeof(");
        if (e->u.sizeof_.is_type && e->u.sizeof_.operand) {
            Type *t = ty_from_ast(ctx->ts, e->u.sizeof_.operand,
                                  cg_type_scope(ctx), NULL);
            if (t && !ty_is_error(t)) cg_type(ctx, t);
            else cg_puts(ctx, "int");
        } else {
            cg_const_expr(ctx, e->u.sizeof_.operand);
        }
        cg_puts(ctx, ")");
        break;
    case AST_BINOP:
        cg_puts(ctx, "(");
        cg_const_expr(ctx, e->u.binop.lhs);
        cg_printf(ctx, " %s ", binop_str(e->u.binop.op));
        cg_const_expr(ctx, e->u.binop.rhs);
        cg_puts(ctx, ")");
        break;
    case AST_UNARY:
        if (!e->u.unary.postfix) {
            static const char *uops[] = {
                [STOK_MINUS]="-",[STOK_BANG]="!",[STOK_TILDE]="~",[STOK_PLUS]="+"
            };
            SharpTokKind op = e->u.unary.op;
            if (op < STOK_COUNT && uops[op])
                cg_puts(ctx, uops[op]);
        }
        cg_const_expr(ctx, e->u.unary.operand);
        break;
    default:
        cg_puts(ctx, "0");
        break;
    }
}

static void cg_field_decl_from_ast(CgCtx *ctx, const AstNode *fd) {
    if (!fd || fd->kind != AST_FIELD_DECL) return;
    /* Resolve through the type system for the base element type;
     * the array suffix size is taken from the AST where possible. */
    Type *ft = ty_from_ast(ctx->ts, fd->u.field_decl.type,
                           cg_type_scope(ctx), NULL);
    /* Check whether the field's AST type has an array suffix with a
     * non-literal size expression that ty_from_ast could not evaluate
     * (returns sz == -1).  If so, emit the declaration directly from
     * the AST to preserve the original expression. */
    bool array_with_unknown_sz = false;
    if (ft && ft->kind == TY_ARRAY && ft->u.array.size < 0) {
        /* Walk the AST type to find the innermost non-array type and
         * collect all array size expressions in order. */
        const AstNode *ast_ty = fd->u.field_decl.type;
        /* Strip const/volatile wrappers. */
        while (ast_ty && (ast_ty->kind == AST_TYPE_CONST ||
                          ast_ty->kind == AST_TYPE_VOLATILE))
            ast_ty = ast_ty->u.type_const.base;
        if (ast_ty && ast_ty->kind == AST_TYPE_ARRAY &&
            ast_ty->u.type_array.size &&
            ast_ty->u.type_array.size->kind != AST_INT_LIT) {
            array_with_unknown_sz = true;
            /* Find base element type. */
            const AstNode *base_ast = ast_ty;
            while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                base_ast = base_ast->u.type_array.base;
            Type *base_t = base_ast
                ? ty_from_ast(ctx->ts, base_ast, cg_type_scope(ctx), NULL)
                : ft->u.array.base;
            cg_type(ctx, base_t ? base_t : ft);
            cg_printf(ctx, " %s", fd->u.field_decl.name);
            /* Emit array suffixes from AST (outermost first). */
            const AstNode *cur = ast_ty;
            while (cur && cur->kind == AST_TYPE_ARRAY) {
                cg_puts(ctx, "[");
                if (cur->u.type_array.size)
                    cg_const_expr(ctx, cur->u.type_array.size);
                cg_puts(ctx, "]");
                cur = cur->u.type_array.base;
            }
        }
    }
    if (!array_with_unknown_sz) {
        cg_decl(ctx, ft, fd->u.field_decl.name);
    }
    if (fd->u.field_decl.bit_width) {
        cg_puts(ctx, " : ");
        cg_expr(ctx, fd->u.field_decl.bit_width);
    }
    cg_puts(ctx, ";\n");
}

/* Emit deferred statements in LIFO order. */
static void cg_emit_defers(CgCtx *ctx, const AstNode **defers, size_t n) {
    for (size_t i = n; i > 0; i--) {
        const AstNode *d = defers[i-1];
        cg_indent(ctx);
        cg_stmt(ctx, d->u.defer_.body, NULL, 0);
    }
}

/* =========================================================================
 * Expression codegen
 * ====================================================================== */

/* Determine the C output for a binary operator token. */
static const char *binop_str(SharpTokKind op) {
    switch (op) {
    case STOK_PLUS: return "+";    case STOK_MINUS:    return "-";
    case STOK_STAR: return "*";    case STOK_SLASH:    return "/";
    case STOK_PERCENT: return "%"; case STOK_AMP:      return "&";
    case STOK_PIPE:  return "|";   case STOK_CARET:    return "^";
    case STOK_LTLT:  return "<<";  case STOK_GTGT:     return ">>";
    case STOK_EQEQ:  return "==";  case STOK_BANGEQ:   return "!=";
    case STOK_LT:    return "<";   case STOK_GT:       return ">";
    case STOK_LTEQ:  return "<=";  case STOK_GTEQ:     return ">=";
    case STOK_AMPAMP:return "&&";  case STOK_PIPEPIPE: return "||";
    case STOK_EQ:    return "=";   case STOK_PLUSEQ:   return "+=";
    case STOK_MINUSEQ: return "-="; case STOK_STAREQ:  return "*=";
    case STOK_SLASHEQ:return "/="; case STOK_PERCENTEQ:return "%=";
    case STOK_AMPEQ:  return "&="; case STOK_PIPEEQ:   return "|=";
    case STOK_CARETEQ:return "^="; case STOK_LTLTEQ:   return "<<=";
    case STOK_GTGTEQ: return ">>=";case STOK_COMMA:    return ",";
    default: return "/*?op*/";
    }
}

/* Get the struct name from a type (TY_STRUCT), stripping const/ptr. */
static const char *struct_name_of(Type *t) {
    if (!t) return NULL;
    if (t->kind == TY_CONST) t = t->u.const_.base;
    if (t->kind == TY_PTR)   t = t->u.ptr.base;
    if (t && t->kind == TY_CONST) t = t->u.const_.base;
    if (t && t->kind == TY_STRUCT) return t->u.struct_.name;
    return NULL;
}

static void cg_expr(CgCtx *ctx, const AstNode *expr) {
    if (!expr) { cg_puts(ctx, "/*null-expr*/"); return; }
    switch (expr->kind) {

    case AST_INT_LIT:
        /* Phase R6: emit integer literals with their original type suffix so
         * that arithmetic uses the correct width.  Critically, a plain `U`
         * literal (e.g. `2654435761U`) must remain `unsigned int` — not be
         * widened to `unsigned long long` — so that `uint32_t * uint32_t`
         * multiplication truncates to 32 bits as the author intended (lz4
         * hash functions depend on this property). */
        if (expr->u.int_lit.is_longlong)
            cg_printf(ctx, expr->u.int_lit.is_unsigned ? "%lluULL" : "%lldLL",
                      (unsigned long long)expr->u.int_lit.val);
        else if (expr->u.int_lit.is_long)
            cg_printf(ctx, expr->u.int_lit.is_unsigned ? "%luUL" : "%ldL",
                      (unsigned long)expr->u.int_lit.val);
        else if (expr->u.int_lit.is_unsigned)
            /* val fits in uint32_t → `U` suffix (unsigned int);
             * otherwise fall back to `ULL` to avoid silent truncation. */
            if ((uint64_t)expr->u.int_lit.val <= 0xFFFFFFFFULL)
                cg_printf(ctx, "%uU", (unsigned int)expr->u.int_lit.val);
            else
                cg_printf(ctx, "%lluULL", (unsigned long long)expr->u.int_lit.val);
        else
            cg_printf(ctx, "%lld", (long long)expr->u.int_lit.val);
        break;
    case AST_FLOAT_LIT: {
        /* Phase S5: use %.17g for round-trip precision.  The default
         * %g prints 6 significant digits, which silently truncates
         * constants like Lua's `(3.141592653589793238462643383279502884)`
         * down to `3.14159` and fails math.tan(pi/4) by ~1.3e-6.
         * 17 is DBL_DECIMAL_DIG — guaranteed enough to uniquely identify
         * any IEEE 754 double.
         *
         * Also: %g emits an integer-shaped literal (no `.` or `e`) for
         * values like 1.0 or 100.0.  In C, `1` is `int`; `1.0` is
         * `double`.  If we don't enforce the decimal point we can flip
         * `1.0 / 2` (== 0.5) into `1 / 2` (== 0).  Append `.0` when the
         * output has neither `.` nor `e`/`E`. */
        char buf[40];
        snprintf(buf, sizeof buf, "%.17g", expr->u.float_lit.val);
        cg_puts(ctx, buf);
        bool has_dot = false;
        for (const char *p = buf; *p; p++) {
            if (*p == '.' || *p == 'e' || *p == 'E') { has_dot = true; break; }
        }
        if (!has_dot) cg_puts(ctx, ".0");
        break;
    }
    case AST_CHAR_LIT:
        /* Emit as integer for simplicity */
        cg_printf(ctx, "%lld", (long long)expr->u.char_lit.val);
        break;
    case AST_STRING_LIT:
        cg_puts(ctx, "\"");
        /* The token text includes surrounding quotes — strip them */
        if (expr->u.string_lit.len >= 2) {
            for (size_t i = 1; i + 1 < expr->u.string_lit.len; i++)
                cgb_push(&ctx->out, expr->u.string_lit.text[i]);
        }
        cg_puts(ctx, "\"");
        break;
    case AST_NULL_LIT:
        cg_puts(ctx, "NULL");
        break;
    case AST_IDENT:
        cg_puts(ctx, expr->u.ident.name);
        break;

    /* Phase S5: GCC labels-as-values — emit `&&label` verbatim.  cc
     * (gcc/clang) accept it as the address-of-label extension. */
    case AST_ADDR_OF_LABEL:
        cg_puts(ctx, "&&");
        cg_puts(ctx, expr->u.addr_of_label.label);
        break;

    case AST_BINOP: {
        /* Check for operator overload (struct type).  Only direct
         * struct values participate — `struct S *p == NULL` is plain
         * pointer comparison and must NOT mangle through
         * `S__op_eq`.  struct_name_of strips one level of pointer
         * for the field-access / method-call paths but here we want
         * to disambiguate value-vs-pointer at the top, mirroring the
         * sema-side guard. */
        Type *lt = sema_type_of(expr->u.binop.lhs);
        Type *lt_unconst = lt;
        if (lt_unconst && lt_unconst->kind == TY_CONST)
            lt_unconst = lt_unconst->u.const_.base;
        bool lhs_is_struct_value =
            lt_unconst && lt_unconst->kind == TY_STRUCT;
        const char *sn = lhs_is_struct_value ? struct_name_of(lt) : NULL;
        /* Only emit a mangled operator call when the operator symbol
         * actually exists in the struct's scope.  Without this check, a
         * plain `value_ = other_value_` on a C union type (which has no
         * operator= defined) would emit `Value__op_eq(...)` causing an
         * undefined-reference link error. */
        if (sn) {
            char opname[64];
            snprintf(opname, sizeof opname, "operator%s",
                     binop_str(expr->u.binop.op));
            /* Look up the operator symbol in the struct's own scope. */
            Scope *ss_s = NULL;
            if (ctx->file_scope) {
                Symbol *sym = scope_lookup_type(ctx->file_scope, sn);
                if (sym && sym->decl && sym->decl->type_ref)
                    ss_s = (Scope *)sym->decl->type_ref;
            }
            Symbol *osym = ss_s ? scope_lookup_local(ss_s, opname) : NULL;
            if (!osym) sn = NULL;   /* operator not defined — fall through */
        }
        if (sn) {
            /* Build "operator<sym>" then mangle */
            char opname[64];
            snprintf(opname, sizeof opname, "operator%s",
                     binop_str(expr->u.binop.op));
            cg_method_name(ctx, sn, opname);
            cg_puts(ctx, "(");
            /* lhs as first arg: take address if value type */
            if (lt->kind == TY_STRUCT) cg_puts(ctx, "&");
            cg_expr(ctx, expr->u.binop.lhs);
            cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.binop.rhs);
            cg_puts(ctx, ")");
        } else {
            cg_puts(ctx, "(");
            cg_expr(ctx, expr->u.binop.lhs);
            cg_printf(ctx, " %s ", binop_str(expr->u.binop.op));
            cg_expr(ctx, expr->u.binop.rhs);
            cg_puts(ctx, ")");
        }
        break;
    }

    case AST_UNARY: {
        static const char *uops[] = {
            [STOK_MINUS]="-", [STOK_BANG]="!", [STOK_TILDE]="~",
            [STOK_PLUS]="+",  [STOK_AMP]="&",  [STOK_STAR]="*",
            [STOK_PLUSPLUS]="++", [STOK_MINUSMINUS]="--"
        };
        SharpTokKind op = expr->u.unary.op;
        const char *s = (op < STOK_COUNT && uops[op]) ? uops[op] : "/*?*/";
        if (expr->u.unary.postfix) {
            cg_puts(ctx, "("); cg_expr(ctx, expr->u.unary.operand);
            cg_puts(ctx, s);   cg_puts(ctx, ")");
        } else {
            cg_puts(ctx, "("); cg_puts(ctx, s);
            cg_expr(ctx, expr->u.unary.operand);
            cg_puts(ctx, ")");
        }
        break;
    }

    case AST_TERNARY:
        cg_puts(ctx, "(");
        cg_expr(ctx, expr->u.ternary.cond);
        cg_puts(ctx, " ? ");
        cg_expr(ctx, expr->u.ternary.then_);
        cg_puts(ctx, " : ");
        cg_expr(ctx, expr->u.ternary.else_);
        cg_puts(ctx, ")");
        break;

    case AST_COMMA:
        cg_puts(ctx, "(");
        cg_expr(ctx, expr->u.comma.lhs);
        cg_puts(ctx, ", ");
        cg_expr(ctx, expr->u.comma.rhs);
        cg_puts(ctx, ")");
        break;

    case AST_CALL: {
        /* For generic function calls, emit the mangled specialized name. */
        const AstNode *callee = expr->u.call.callee;
        bool emitted_name = false;
        if (callee && callee->kind == AST_IDENT) {
            const char *fname = callee->u.ident.name;
            Symbol *sym = ctx->file_scope
                ? scope_lookup(ctx->file_scope, fname) : NULL;
            if (sym && sym->kind == SYM_FUNC && sym->decl &&
                sym->decl->kind == AST_FUNC_DEF &&
                sym->decl->u.func_def.generic_params.len > 0) {
                /* Generic function — emit mangled name. */
                const char *mn = cg_gfunc_mangle_for_call(ctx, fname, expr);
                cg_puts(ctx, mn);
                emitted_name = true;
            }
        }
        if (!emitted_name) cg_expr(ctx, expr->u.call.callee);
        cg_puts(ctx, "(");
        for (size_t i = 0; i < expr->u.call.args.len; i++) {
            if (i) cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.call.args.data[i]);
        }
        cg_puts(ctx, ")");
        break;
    }

    case AST_METHOD_CALL: {
        Type *recv_t = sema_type_of(expr->u.method_call.recv);
        const char *sn = struct_name_of(recv_t);
        bool arrow = expr->u.method_call.arrow;
        /* Sema may have flagged this method-call as actually a
         * function-pointer-field call (see ast.h is_field_call doc).
         * When set, emit the source-level form `recv.field(args)`
         * with no method-mangling. */
        if (expr->u.method_call.is_field_call) {
            cg_expr(ctx, expr->u.method_call.recv);
            cg_puts(ctx, arrow ? "->" : ".");
            cg_puts(ctx, expr->u.method_call.method);
            cg_puts(ctx, "(");
            for (size_t i = 0; i < expr->u.method_call.args.len; i++) {
                if (i) cg_puts(ctx, ", ");
                cg_expr(ctx, expr->u.method_call.args.data[i]);
            }
            cg_puts(ctx, ")");
            break;
        }
        if (sn) {
            /* For generic struct instances, mangle the struct name. */
            Type *base_recv = recv_t;
            if (base_recv && base_recv->kind == TY_CONST) base_recv = base_recv->u.const_.base;
            if (base_recv && base_recv->kind == TY_PTR)   base_recv = base_recv->u.ptr.base;
            if (base_recv && base_recv->kind == TY_CONST) base_recv = base_recv->u.const_.base;
            char *mangled_sn = (base_recv && base_recv->kind == TY_STRUCT &&
                                base_recv->u.struct_.nargs > 0)
                ? cg_mangle_inst(sn, base_recv->u.struct_.args, base_recv->u.struct_.nargs)
                : NULL;
            cg_method_name(ctx, mangled_sn ? mangled_sn : sn,
                           expr->u.method_call.method);
            cg_puts(ctx, "(");
            /* this pointer: pass receiver address or value */
            if (!arrow && recv_t && recv_t->kind == TY_STRUCT)
                cg_puts(ctx, "&");
            cg_expr(ctx, expr->u.method_call.recv);
            for (size_t i = 0; i < expr->u.method_call.args.len; i++) {
                cg_puts(ctx, ", ");
                cg_expr(ctx, expr->u.method_call.args.data[i]);
            }
            cg_puts(ctx, ")");
            free(mangled_sn);
        } else {
            /* Fallback: emit as expr.method(args) */
            cg_expr(ctx, expr->u.method_call.recv);
            cg_puts(ctx, arrow ? "->" : ".");
            cg_puts(ctx, expr->u.method_call.method);
            cg_puts(ctx, "(");
            for (size_t i = 0; i < expr->u.method_call.args.len; i++) {
                if (i) cg_puts(ctx, ", ");
                cg_expr(ctx, expr->u.method_call.args.data[i]);
            }
            cg_puts(ctx, ")");
        }
        break;
    }

    case AST_FIELD_ACCESS:
        cg_expr(ctx, expr->u.field_access.recv);
        cg_puts(ctx, expr->u.field_access.arrow ? "->" : ".");
        cg_puts(ctx, expr->u.field_access.field);
        break;

    case AST_INDEX:
        cg_expr(ctx, expr->u.index_.base);
        cg_puts(ctx, "[");
        cg_expr(ctx, expr->u.index_.index);
        cg_puts(ctx, "]");
        break;

    case AST_CAST: {
        /* Prefer the sema-computed type (which is the cast's declared result
         * type, resolved through the type system).  Fall back to directly
         * emitting the AST cast-type when sema did not annotate the node
         * (e.g., a cast inside a macro that sema never visited) or when
         * the type resolved to an error/void sentinel. */
        Type *cast_t = (Type*)sema_type_of(expr);
        cg_puts(ctx, "((");
        if (cast_t && !ty_is_error(cast_t) && cast_t->kind != TY_VOID &&
            cast_t->kind != TY_ERROR) {
            cg_type(ctx, cast_t);
        } else if (expr->u.cast.type) {
            /* Emit the type directly from the AST using the const-expr type
             * helper so we avoid needing sema annotations. */
            Type *ast_t = ty_from_ast(ctx->ts, expr->u.cast.type,
                                      cg_type_scope(ctx), NULL);
            if (ast_t && !ty_is_error(ast_t)) {
                cg_type(ctx, ast_t);
            } else {
                /* Last resort: pretty-print the AST type node. */
                cg_puts(ctx, "int");
            }
        } else {
            cg_puts(ctx, "int");
        }
        cg_puts(ctx, ")");
        cg_expr(ctx, expr->u.cast.operand);
        cg_puts(ctx, ")");
        break;
    }

    case AST_SIZEOF:
        cg_puts(ctx, "sizeof(");
        if (expr->u.sizeof_.is_type) {
            /* Prefer the sema-annotated type; fall back to ty_from_ast
             * for types (like struct typedefs) whose sizeof operand node
             * was not annotated by sema — without this, sizeof(LG) where
             * LG is a struct typedef would emit sizeof(int).
             *
             * Phase R5: if both sema and ty_from_ast fail (returns
             * ty_error), the type name is probably a LOCAL typedef that is
             * only visible in the function-body scope, not the file scope
             * that cg_type_scope() returns (e.g. `typedef struct {...} t_a`
             * defined inside a function body).  In that case emit the type
             * name verbatim from the AST — the local C scope will resolve
             * it correctly when the generated file is compiled. */
            Type *t = sema_type_of(expr->u.sizeof_.operand);
            if (!t || ty_is_error(t))
                t = ty_from_ast(ctx->ts, expr->u.sizeof_.operand,
                                cg_type_scope(ctx), NULL);
            if (t && !ty_is_error(t)) {
                cg_type(ctx, t);
            } else if (expr->u.sizeof_.operand &&
                       expr->u.sizeof_.operand->kind == AST_TYPE_NAME &&
                       expr->u.sizeof_.operand->u.type_name.name) {
                /* Local type name: emit verbatim for cc to resolve. */
                cg_puts(ctx, expr->u.sizeof_.operand->u.type_name.name);
            } else {
                cg_puts(ctx, "int");
            }
        } else {
            cg_expr(ctx, expr->u.sizeof_.operand);
        }
        cg_puts(ctx, ")");
        break;

    case AST_STRUCT_LIT: {
        /* Emit as C compound literal: (Type){ .field = val, ... } */
        Type *t = sema_type_of(expr);
        /* Fallback: if sema didn't run, resolve type from the AST node. */
        if (!t || ty_is_error(t))
            t = ty_from_ast(ctx->ts, expr->u.struct_lit.type, cg_type_scope(ctx), NULL);
        cg_puts(ctx, "(");
        if (t && !ty_is_error(t)) cg_type(ctx, t);
        else cg_puts(ctx, "/* ?struct_type */");
        cg_puts(ctx, "){ ");
        for (size_t i = 0; i < expr->u.struct_lit.field_names.len; i++) {
            if (i) cg_puts(ctx, ", ");
            AstNode *fn = expr->u.struct_lit.field_names.data[i];
            cg_printf(ctx, ".%s = ", fn->u.ident.name);
            if (i < expr->u.struct_lit.field_vals.len)
                cg_expr(ctx, expr->u.struct_lit.field_vals.data[i]);
        }
        cg_puts(ctx, " }");
        break;
    }

    /* S4: braced initializer.  Used both as a top-level initializer
     * (`int v[3] = {1,2,3};`) and as a value inside a compound literal
     * (`(struct P){...}`).  Either way, the C output is the same:
     * `{ item, item, ... }` with each item rendered by recursion. */
    case AST_INIT_LIST: {
        cg_puts(ctx, "{ ");
        for (size_t i = 0; i < expr->u.init_list.items.len; i++) {
            if (i) cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.init_list.items.data[i]);
        }
        cg_puts(ctx, " }");
        break;
    }

    case AST_DESIGNATED_INIT: {
        if (expr->u.designated_init.designator_kind == 0) {
            cg_printf(ctx, ".%s = ", expr->u.designated_init.field_name);
        } else {
            cg_puts(ctx, "[");
            cg_expr(ctx, expr->u.designated_init.index_expr);
            cg_puts(ctx, "] = ");
        }
        cg_expr(ctx, expr->u.designated_init.value);
        break;
    }

    case AST_COMPOUND_LIT: {
        /* `(Type){ ... }` — the type-name is rendered through the AST
         * (we go via ty_from_ast then cg_type so const/struct/etc work
         * uniformly).  The init list renders itself with surrounding
         * braces. */
        Type *t = ty_from_ast(ctx->ts, expr->u.compound_lit.type,
                              cg_type_scope(ctx), NULL);
        cg_puts(ctx, "(");
        if (t && !ty_is_error(t)) cg_type(ctx, t);
        else cg_puts(ctx, "/* ?compound_lit_type */");
        cg_puts(ctx, ")");
        cg_expr(ctx, expr->u.compound_lit.init);
        break;
    }

    case AST_AT_INTRINSIC:
        /* @has_operator / @static_assert: emit as 1 for now */
        cg_printf(ctx, "/* @%s */ 1", expr->u.at_intrinsic.name);
        break;

    case AST_STMT_EXPR:
        /* Phase R4: GCC statement-expression `({ stmts; expr; })`.
         * We emit the GNU extension verbatim: `({` then the block's
         * statements (using cg_block's indented emission), then `})`.
         * GCC and clang both accept this syntax; the C compiler validates
         * the semantics.  The surrounding parentheses are required by the
         * GNU extension grammar. */
        cg_puts(ctx, "({");
        if (expr->u.stmt_expr.block) {
            /* cg_block emits the statements indented, without outer braces.
             * We wrap manually so the result is `({ stmt; stmt; expr; })`. */
            const AstNode *block = expr->u.stmt_expr.block;
            const AstNode *saved_defers = NULL;
            (void)saved_defers;
            for (size_t i = 0; i < block->u.block.stmts.len; i++) {
                cg_puts(ctx, " ");
                /* Emit each stmt without a surrounding block; the `({`
                 * and `})` are the delimiters.  Pass NULL defers because
                 * defer semantics across stmt-expr boundaries are not
                 * supported (cc enforces the semantic). */
                cg_stmt(ctx, block->u.block.stmts.data[i], NULL, 0);
                /* strip the trailing newline that cg_stmt appends so the
                 * whole expr fits on one logical "line" in the output */
            }
        }
        cg_puts(ctx, " })");
        break;

    default:
        cg_puts(ctx, "/*?expr*/");
        break;
    }
}

/* =========================================================================
 * Statement codegen
 * ====================================================================== */
static void cg_stmt(CgCtx *ctx, const AstNode *stmt,
                    const AstNode **defers, size_t ndefers) {
    if (!stmt) return;
    switch (stmt->kind) {

    case AST_EXPR_STMT:
        cg_indent(ctx);
        cg_expr(ctx, stmt->u.expr_stmt.expr);
        cg_puts(ctx, ";");
        cg_nl(ctx);
        break;

    case AST_STRUCT_DEF:
        /* Local struct/union definition inside a function body.
         * Emit it as a block-scope type definition so that variable
         * declarations of that type (e.g. `cD s;`) in the same block
         * can use the struct name.  The generated C places the full
         * definition inline before any variables that reference it. */
        cg_indent(ctx);
        cg_struct(ctx, stmt);
        break;

    case AST_TYPEDEF_DECL: {
        /* Phase R5: local typedef inside a function body, e.g.
         *   typedef struct { char c; LZ4_stream_t t; } t_a;
         * The struct/union body was already emitted by the preceding
         * AST_STRUCT_DEF statement (from the parse_block R5 ordering fix).
         * cg_struct emits `typedef struct X X; struct X { ... };` so the
         * struct tag name is usable as a plain identifier.  We just need to
         * emit the alias: `typedef TargetName Alias;`.
         *
         * Note: ty_from_ast uses the file scope and CANNOT find local types.
         * We therefore emit the target name directly from the AST node,
         * which is safe because the preceding struct body emission put the
         * tag into the local C scope. */
        const char *alias = stmt->u.typedef_decl.alias;
        const AstNode *target = stmt->u.typedef_decl.target;
        if (!alias || !target) break;
        cg_indent(ctx);
        if (target->kind == AST_TYPE_NAME && target->u.type_name.name) {
            cg_printf(ctx, "typedef %s %s;\n", target->u.type_name.name, alias);
        } else {
            /* Non-trivial target (pointer, array, etc.): try ty_from_ast. */
            Type *t = ty_from_ast(ctx->ts, target, cg_type_scope(ctx), NULL);
            cg_puts(ctx, "typedef ");
            cg_decl(ctx, t ? t : ty_int(ctx->ts), alias);
            cg_puts(ctx, ";\n");
        }
        break;
    }

    case AST_DECL_STMT: {
        AstNode *vd = stmt->u.decl_stmt.decl;
        if (!vd) break;
        /* Phase R5: block-scope typedef (`typedef struct {...} t_a;`) is
         * wrapped as AST_DECL_STMT { decl: AST_TYPEDEF_DECL } by the
         * parser's stmt_wrap path.  Previously the `!= AST_VAR_DECL`
         * guard dropped it silently.  Delegate to the AST_TYPEDEF_DECL
         * case above, which emits `typedef TargetName Alias;` from the
         * AST node directly (ty_from_ast uses file scope and can't find
         * locally-defined struct/union types). */
        if (vd->kind == AST_TYPEDEF_DECL) {
            cg_stmt(ctx, vd, defers, ndefers);
            break;
        }
        if (vd->kind != AST_VAR_DECL) break;
        cg_indent(ctx);
        switch (vd->u.var_decl.storage) {
        case SC_STATIC:   cg_puts(ctx, "static ");   break;
        case SC_EXTERN:   cg_puts(ctx, "extern ");   break;
        case SC_REGISTER: cg_puts(ctx, "register "); break;
        default: break;
        }
        /* Phase R2: emit C11 _Thread_local after storage-class. */
        if (vd->u.var_decl.is_thread_local) cg_puts(ctx, "_Thread_local ");
        if (ast_type_outer_is_volatile(vd->u.var_decl.type))
            cg_puts(ctx, "volatile ");
        Type *t = (Type*)vd->type_ref;
        if (!t || ty_is_error(t))
            t = vd->u.var_decl.type
              ? ty_from_ast(ctx->ts, vd->u.var_decl.type, cg_type_scope(ctx), NULL)
              : ty_int(ctx->ts);
        /* When the type is an array with an unknown (unevaluated) size
         * — e.g. `unsigned char buff[CHAR_BIT * sizeof(size_t)]` where
         * `CHAR_BIT` is an identifier constant — ty_from_ast stores
         * size=-1 and cg_decl would emit `buff[]`.  Fall back to the
         * original AST to emit the size expression verbatim so the C
         * compiler can evaluate it. */
        bool array_fallback = (t && t->kind == TY_ARRAY && t->u.array.size < 0);
        if (array_fallback && vd->u.var_decl.type) {
            /* Walk down the AST type to find the innermost base type
             * and collect array size expressions in order. */
            const AstNode *ast_ty = vd->u.var_decl.type;
            while (ast_ty && (ast_ty->kind == AST_TYPE_CONST ||
                              ast_ty->kind == AST_TYPE_VOLATILE))
                ast_ty = ast_ty->u.type_const.base;
            if (ast_ty && ast_ty->kind == AST_TYPE_ARRAY &&
                ast_ty->u.type_array.size) {
                const AstNode *base_ast = ast_ty;
                while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                    base_ast = base_ast->u.type_array.base;
                Type *base_t = base_ast
                    ? ty_from_ast(ctx->ts, base_ast, cg_type_scope(ctx), NULL)
                    : t->u.array.base;
                cg_type(ctx, base_t ? base_t : t);
                cg_printf(ctx, " %s", vd->u.var_decl.name);
                const AstNode *cur = ast_ty;
                while (cur && cur->kind == AST_TYPE_ARRAY) {
                    cg_puts(ctx, "[");
                    if (cur->u.type_array.size)
                        cg_const_expr(ctx, cur->u.type_array.size);
                    cg_puts(ctx, "]");
                    cur = cur->u.type_array.base;
                }
            } else {
                cg_decl(ctx, t, vd->u.var_decl.name);
            }
        } else {
            cg_decl(ctx, t, vd->u.var_decl.name);
        }
        /* Phase R6: GCC attributes after the declarator, before `=` or `;`.
         * Covers `int x __attribute__((aligned(16)));` and similar forms. */
        if (vd->u.var_decl.gcc_attrs) {
            cg_puts(ctx, " ");
            cg_puts(ctx, vd->u.var_decl.gcc_attrs);
        }
        if (vd->u.var_decl.init) {
            cg_puts(ctx, " = ");
            cg_expr(ctx, vd->u.var_decl.init);
        }
        cg_puts(ctx, ";");
        cg_nl(ctx);
        break;
    }

    case AST_RETURN:
        cg_emit_defers(ctx, defers, ndefers);
        cg_indent(ctx);
        cg_puts(ctx, "return");
        if (stmt->u.return_.value) {
            cg_puts(ctx, " ");
            cg_expr(ctx, stmt->u.return_.value);
        }
        cg_puts(ctx, ";");
        cg_nl(ctx);
        break;

    case AST_BREAK:
        cg_emit_defers(ctx, defers, ndefers);
        cg_indent(ctx); cg_puts(ctx, "break;"); cg_nl(ctx);
        break;

    case AST_CONTINUE:
        cg_emit_defers(ctx, defers, ndefers);
        cg_indent(ctx); cg_puts(ctx, "continue;"); cg_nl(ctx);
        break;

    case AST_GOTO:
        cg_indent(ctx);
        cg_printf(ctx, "goto %s;", stmt->u.goto_.label);
        cg_nl(ctx);
        break;

    /* Phase S5: GCC computed goto — `goto *expr;`.  We run any pending
     * defers first because, like a plain goto, control transfers
     * unstructurally out of the current scope.  The expression is
     * wrapped in parens to keep precedence safe under any cg_expr
     * output. */
    case AST_COMPUTED_GOTO:
        cg_emit_defers(ctx, defers, ndefers);
        cg_indent(ctx);
        cg_puts(ctx, "goto *(");
        cg_expr(ctx, stmt->u.computed_goto.target);
        cg_puts(ctx, ");");
        cg_nl(ctx);
        break;

    case AST_LABEL:
        cg_printf(ctx, "%s:", stmt->u.label_.label);
        cg_nl(ctx);
        break;

    case AST_BLOCK:
        cg_block(ctx, stmt);
        break;

    case AST_IF: {
        /* Compile-time @intrinsic condition (e.g. `if (@has_operator(T,+))`):
         * sema annotated cond->type_ref with ty_int (live=then) or ty_void
         * (live=else).  Emit only the live branch — no `if` wrapper, no
         * dead-branch code. */
        AstNode *cond_if = stmt->u.if_.cond;
        if (cond_if && cond_if->kind == AST_AT_INTRINSIC && cond_if->type_ref) {
            Type *annot = (Type*)cond_if->type_ref;
            bool live_is_then = (annot->kind != TY_VOID);
            if (live_is_then) {
                if (stmt->u.if_.then_)
                    cg_stmt(ctx, stmt->u.if_.then_, defers, ndefers);
            } else {
                if (stmt->u.if_.else_)
                    cg_stmt(ctx, stmt->u.if_.else_, defers, ndefers);
            }
            break;
        }
        cg_indent(ctx); cg_puts(ctx, "if (");
        cg_expr(ctx, cond_if);
        cg_puts(ctx, ") ");
        if (stmt->u.if_.then_ && stmt->u.if_.then_->kind == AST_BLOCK) {
            cg_block(ctx, stmt->u.if_.then_);
        } else {
            cg_puts(ctx, "{\n"); ctx->indent++;
            if (stmt->u.if_.then_)
                cg_stmt(ctx, stmt->u.if_.then_, defers, ndefers);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        }
        if (stmt->u.if_.else_) {
            cg_indent(ctx); cg_puts(ctx, "else ");
            if (stmt->u.if_.else_->kind == AST_BLOCK) {
                cg_block(ctx, stmt->u.if_.else_);
            } else {
                cg_puts(ctx, "{\n"); ctx->indent++;
                cg_stmt(ctx, stmt->u.if_.else_, defers, ndefers);
                ctx->indent--;
                cg_indent(ctx); cg_puts(ctx, "}\n");
            }
        }
        break;
    }

    case AST_WHILE:
        cg_indent(ctx); cg_puts(ctx, "while (");
        cg_expr(ctx, stmt->u.while_.cond);
        cg_puts(ctx, ") ");
        if (stmt->u.while_.body && stmt->u.while_.body->kind == AST_BLOCK)
            cg_block(ctx, stmt->u.while_.body);
        else {
            cg_puts(ctx, "{\n"); ctx->indent++;
            if (stmt->u.while_.body) cg_stmt(ctx, stmt->u.while_.body, NULL, 0);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        }
        break;

    case AST_DO_WHILE:
        cg_indent(ctx); cg_puts(ctx, "do ");
        /* Phase R2: body may be a BLOCK or a single statement.  Before
         * the fix, this branch unconditionally called cg_block, which
         * dereferences `body->u.block.stmts` — when body is actually
         * an AST_EXPR_STMT (the bare `do x++; while (...)` form), that
         * read landed in the wrong union arm and emitted an empty
         * `do { } while (...)`.  stb_image's zlib decoder
         *   do *zout++ = v; while (--len);
         * tripped this, producing `do { } while ((--len));` and a
         * "bad dist" decode failure at runtime.  Mirror the
         * if/while/for/switch dispatch: BLOCK → cg_block, anything
         * else → wrap in `{ }` + cg_stmt. */
        if (stmt->u.do_while.body &&
            stmt->u.do_while.body->kind == AST_BLOCK) {
            cg_block(ctx, stmt->u.do_while.body);
        } else {
            cg_puts(ctx, "{\n"); ctx->indent++;
            if (stmt->u.do_while.body)
                cg_stmt(ctx, stmt->u.do_while.body, NULL, 0);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        }
        cg_indent(ctx); cg_puts(ctx, "while (");
        cg_expr(ctx, stmt->u.do_while.cond);
        cg_puts(ctx, ");");
        cg_nl(ctx);
        break;

    /* S2: switch / case / default.  Body is normally a BLOCK whose
     * statements interleave AST_CASE / AST_DEFAULT pseudo-labels with
     * regular statements; cg_block renders each statement at the
     * current indentation.  We emit the `case`/`default` label without
     * indenting it (matching the typical Linux/K&R style for switch
     * statements). */
    case AST_SWITCH:
        cg_indent(ctx); cg_puts(ctx, "switch (");
        cg_expr(ctx, stmt->u.switch_.cond);
        cg_puts(ctx, ") ");
        if (stmt->u.switch_.body && stmt->u.switch_.body->kind == AST_BLOCK)
            cg_block(ctx, stmt->u.switch_.body);
        else {
            cg_puts(ctx, "{\n"); ctx->indent++;
            if (stmt->u.switch_.body) cg_stmt(ctx, stmt->u.switch_.body, NULL, 0);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        }
        break;

    case AST_CASE:
        cg_indent(ctx);
        cg_puts(ctx, "case ");
        cg_expr(ctx, stmt->u.case_.value);
        cg_puts(ctx, ":");
        cg_nl(ctx);
        break;

    case AST_DEFAULT:
        cg_indent(ctx);
        cg_puts(ctx, "default:");
        cg_nl(ctx);
        break;

    case AST_FOR:
        cg_indent(ctx); cg_puts(ctx, "for (");
        if (stmt->u.for_.init) {
            AstNode *init = stmt->u.for_.init;
            if (init->kind == AST_DECL_STMT) {
                AstNode *vd = init->u.decl_stmt.decl;
                Type *t = (Type*)vd->type_ref;
                if (!t || ty_is_error(t))
                    t = vd->u.var_decl.type
                      ? ty_from_ast(ctx->ts, vd->u.var_decl.type, cg_type_scope(ctx), NULL)
                      : ty_int(ctx->ts);
                cg_decl(ctx, t, vd->u.var_decl.name);
                if (vd->u.var_decl.init) {
                    cg_puts(ctx, " = ");
                    cg_expr(ctx, vd->u.var_decl.init);
                }
            } else if (init->kind == AST_EXPR_STMT) {
                cg_expr(ctx, init->u.expr_stmt.expr);
            }
        }
        cg_puts(ctx, "; ");
        if (stmt->u.for_.cond) cg_expr(ctx, stmt->u.for_.cond);
        cg_puts(ctx, "; ");
        if (stmt->u.for_.post) cg_expr(ctx, stmt->u.for_.post);
        cg_puts(ctx, ") ");
        /* Body may be a BLOCK or a single statement — handle both. */
        if (stmt->u.for_.body && stmt->u.for_.body->kind == AST_BLOCK)
            cg_block(ctx, stmt->u.for_.body);
        else {
            cg_puts(ctx, "{\n"); ctx->indent++;
            if (stmt->u.for_.body) cg_stmt(ctx, stmt->u.for_.body, NULL, 0);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        }
        break;

    case AST_DEFER:
        /* defer is handled by cg_block's defer collection — skip here */
        break;

    /* Phase R2: block-scope enum definition.  C allows
     *   void f(void) { enum { W = 16, H = 16 }; int x = W + H; }
     * The enumerators must be emitted as a real C enum so that the
     * regenerated code's references to `W`/`H` resolve.  Mirror the
     * top-level emission shape (cg.c around line 2091). */
    case AST_ENUM_DEF: {
        cg_indent(ctx);
        cg_puts(ctx, "enum");
        if (stmt->u.enum_def.name) cg_printf(ctx, " %s", stmt->u.enum_def.name);
        cg_puts(ctx, " {\n");
        ctx->indent++;
        for (size_t j = 0; j < stmt->u.enum_def.items.len; j++) {
            AstNode *en = stmt->u.enum_def.items.data[j];
            if (!en || en->kind != AST_ENUMERATOR) continue;
            cg_indent(ctx);
            cg_puts(ctx, en->u.enumerator.name);
            if (en->u.enumerator.value) {
                cg_puts(ctx, " = ");
                cg_expr(ctx, en->u.enumerator.value);
            }
            cg_puts(ctx, j + 1 < stmt->u.enum_def.items.len ? ",\n" : "\n");
        }
        ctx->indent--;
        cg_indent(ctx); cg_puts(ctx, "};"); cg_nl(ctx);
        break;
    }

    default:
        cg_indent(ctx);
        cg_printf(ctx, "/* unsupported stmt kind %d */", stmt->kind);
        cg_nl(ctx);
        break;
    }
}

static void cg_block(CgCtx *ctx, const AstNode *block) {
    if (!block) { cg_puts(ctx, "{ }"); cg_nl(ctx); return; }

    /* Collect defers in this block. */
    const AstNode **defers = NULL;
    size_t ndefers = 0, dcap = 0;

    cg_puts(ctx, "{"); cg_nl(ctx);
    ctx->indent++;

    for (size_t i = 0; i < block->u.block.stmts.len; i++) {
        const AstNode *s = block->u.block.stmts.data[i];
        if (!s) continue;
        if (s->kind == AST_DEFER) {
            /* Register defer */
            if (ndefers == dcap) {
                dcap = dcap ? dcap * 2 : 4;
                defers = realloc(defers, dcap * sizeof *defers);
                if (!defers) abort();
            }
            defers[ndefers++] = s;
        } else {
            cg_stmt(ctx, s, defers, ndefers);
        }
    }

    /* End of block: emit remaining defers LIFO. */
    cg_emit_defers(ctx, defers, ndefers);
    free(defers);

    ctx->indent--;
    cg_indent(ctx); cg_puts(ctx, "}"); cg_nl(ctx);
}

/* =========================================================================
 * Function (free or method) codegen
 * ====================================================================== */
static void cg_func(CgCtx *ctx, const AstNode *fn, const char *sname) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;

    /* Generic functions: skip — specialized versions emitted by Phase 11 pass. */
    if (fn->u.func_def.generic_params.len > 0) return;

    /* S1: storage-class & function-specifier prefix.  We honour either
     * the new `storage` field or the legacy `is_static` flag (back-
     * compat with import.c, which still sets is_static directly). */
    StorageClass sc = fn->u.func_def.storage;
    if (sc == SC_NONE && fn->u.func_def.is_static) sc = SC_STATIC;
    /* C99 `inline` without an explicit storage class declares an inline
     * definition that is NOT a stand-alone external — the linker still
     * expects an external one elsewhere.  The Sharp transpiler has only
     * one TU per source, so the safe and idiomatic emission is
     * `static inline` (the same pattern most modern C code uses for
     * header-defined helpers).  Promote SC_NONE+inline to SC_STATIC. */
    if (sc == SC_NONE && fn->u.func_def.is_inline) sc = SC_STATIC;
    switch (sc) {
    case SC_STATIC:   cg_puts(ctx, "static ");   break;
    case SC_EXTERN:   cg_puts(ctx, "extern ");   break;
    case SC_REGISTER: cg_puts(ctx, "register "); break;  /* unusual on funcs */
    case SC_TYPEDEF:  break;                              /* not legal here */
    case SC_AUTO_C:   break;                              /* not legal here */
    case SC_NONE:     break;
    }
    if (fn->u.func_def.is_inline) cg_puts(ctx, "inline ");
    /* Phase R2: C11 _Thread_local on function declarations is unusual
     * but ISO permits it; emit verbatim to preserve source intent. */
    if (fn->u.func_def.is_thread_local) cg_puts(ctx, "_Thread_local ");
    /* Phase R6: GCC attributes in leading position — after storage-class
     * specifiers, before the return type.  This is the canonical position
     * accepted by all versions of GCC and Clang for both declarations and
     * definitions.  GCC 8+ warns about trailing attributes on function
     * definitions; leading position avoids the warning entirely.
     * Example: `static inline __attribute__((always_inline)) int f(…)`. */
    if (fn->u.func_def.gcc_attrs) {
        cg_puts(ctx, fn->u.func_def.gcc_attrs);
        cg_puts(ctx, " ");
    }

    /* Return type */
    AstNode *ret_node = fn->u.func_def.ret_type;
    Type *ret_t = ret_node ? ty_from_ast(ctx->ts, ret_node, cg_type_scope(ctx), NULL)
                           : ty_void(ctx->ts);
    /* Phase R3: a function whose return type is itself a pointer to a
     * function must be emitted as a *nested* declarator
     *
     *     inner_ret (* name(outer_args))(inner_args)
     *
     * (ISO C 6.7.6.3 — the declarator "inhabits" the place where the
     * pointed-to function would be).  Lua's lua.h declares
     *
     *     int (*lua_atpanic(lua_State *L, int (*panicf)(lua_State *)))
     *                      (lua_State *);
     *
     * and several siblings.  Detection: peel TY_PTR layers; if we land
     * on TY_FUNC, switch to the nested emission shape.  Otherwise the
     * normal `ret_t  name(args)` form applies. */
    int  ret_nstars  = 0;
    Type *ret_inner  = NULL;
    {
        Type *peel = ret_t;
        while (peel && peel->kind == TY_PTR) { ret_nstars++; peel = peel->u.ptr.base; }
        if (peel && peel->kind == TY_FUNC) ret_inner = peel;
    }
    if (ret_inner) {
        /* outer-ret + `(* …` opener; the matching `)( inner-args )`
         * closer is emitted after the outer parameter list below. */
        cg_type(ctx, ret_inner->u.func.ret);
        cg_puts(ctx, " (");
        for (int i = 0; i < ret_nstars; i++) cg_puts(ctx, "*");
    } else {
        cg_type(ctx, ret_t);
        cg_puts(ctx, " ");
    }

    /* Function name */
    if (sname) {
        cg_method_name(ctx, sname, fn->u.func_def.name);
    } else if (strncmp(fn->u.func_def.name, "operator", 8) == 0) {
        /* Free operator function: mangle to `operator_<sym>__<param0>__<param1>...`
         * so the C identifier is legal and matches the call-site mangling.
         * e.g. `operator+(Vec3, Vec3)` → `operator_add__Vec3__Vec3` */
        const char *sym = fn->u.func_def.name + 8;
        cg_printf(ctx, "operator_%s", op_suffix(sym));
        for (size_t pi = 0; pi < fn->u.func_def.params.len; pi++) {
            AstNode *p = fn->u.func_def.params.data[pi];
            if (!p) continue;
            if (p->u.param_decl.is_vararg) continue;
            Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type,
                                   cg_type_scope(ctx), NULL);
            if (pt && pt->kind == TY_STRUCT && pt->u.struct_.name) {
                cg_printf(ctx, "__%s", pt->u.struct_.name);
            } else if (pt) {
                const char *kn = ty_kind_name(pt->kind);
                cg_printf(ctx, "__%s", kn);
            }
        }
    } else {
        cg_puts(ctx, fn->u.func_def.name);
    }

    /* Parameters */
    cg_puts(ctx, "(");
    bool first = true;

    /* For methods: add 'this' as first param (always pointer). */
    if (sname) {
        if (fn->u.func_def.is_const_method)
            cg_printf(ctx, "const %s *this", sname);
        else
            cg_printf(ctx, "%s *this", sname);
        first = false;
    }

    bool has_vararg = false;
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (!p) continue;
        if (p->u.param_decl.is_vararg) { has_vararg = true; continue; }
        if (!first) cg_puts(ctx, ", ");
        first = false;
        Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type, cg_type_scope(ctx), NULL);
        cg_decl(ctx, pt, p->u.param_decl.name);
    }
    if (has_vararg) {
        if (!first) cg_puts(ctx, ", ");
        cg_puts(ctx, "...");
        first = false;
    }
    if (first) cg_puts(ctx, "void");
    cg_puts(ctx, ")");

    /* Phase R3: close the nested declarator for "function returning
     * function pointer".  Form: `outer_ret (* name(outer_args))
     *                                       (inner_args)`. */
    if (ret_inner) {
        cg_puts(ctx, ")(");
        if (ret_inner->u.func.nparams == 0) {
            cg_puts(ctx, "void");
        } else {
            for (size_t i = 0; i < ret_inner->u.func.nparams; i++) {
                if (i) cg_puts(ctx, ", ");
                cg_type(ctx, ret_inner->u.func.params[i]);
            }
        }
        cg_puts(ctx, ")");
    }

    if (!fn->u.func_def.body) {
        cg_puts(ctx, ";"); cg_nl(ctx);
        return;
    }

    cg_puts(ctx, " ");
    cg_block(ctx, fn->u.func_def.body);
}

/* =========================================================================
 * Struct codegen
 * ====================================================================== */
static void cg_struct(CgCtx *ctx, const AstNode *sd) {
    if (!sd || sd->kind != AST_STRUCT_DEF) return;

    /* Generic structs: Phase 11 */
    if (sd->u.struct_def.generic_params.len > 0) {
        cg_printf(ctx, "/* TODO Phase11: generic struct %s */\n",
                  sd->u.struct_def.name);
        return;
    }

    const char *sname = sd->u.struct_def.name;
    /* S2: same emitter handles structs and unions; the keyword is the
     * only thing that differs.  No `typedef union` forward decl is
     * emitted because unions can be self-referencing only via pointer
     * (same restriction as struct), so the symbol-level `union T` form
     * already covers forward-references inside the body. */
    const char *kw = sd->u.struct_def.is_union ? "union" : "struct";

    /* Skip the local `typedef ... X;` if the same name is also bound
     * to a function or variable in this scope (see cg_type TY_STRUCT
     * branch — collision case emits `struct X` instead of plain `X`).
     * The pass-1 forward decl emitted `struct X;` rather than a
     * typedef in this case, so just emit the body. */
    bool collision = false;
    if (ctx->file_scope) {
        Symbol *sym = scope_lookup_local(ctx->file_scope, sname);
        for (; sym; sym = sym->next) {
            if (strcmp(sym->name, sname) != 0) continue;
            if (sym->kind == SYM_FUNC || sym->kind == SYM_VAR) {
                collision = true;
                break;
            }
        }
    }

    /* If this AST_STRUCT_DEF has no body (a `struct X;` forward decl
     * in the source), skip the entire body emission — the pass-1
     * forward decl already emitted either `typedef struct X X;` or
     * `struct X;` depending on the collision check.  Emitting an
     * empty `struct X { };` here would cause a redefinition cascade
     * if a later AST_STRUCT_DEF in file.decls completes the type.
     * scope_define merges the two SYM_TYPE entries (see scope.c S5
     * struct fwd+body case) but the AST nodes remain independent in
     * file.decls. */
    bool has_body = sd->u.struct_def.fields.len > 0 ||
                    sd->u.struct_def.methods.len > 0;
    if (!has_body) {
        return;
    }

    /* Struct body */
    if (!collision) {
        cg_printf(ctx, "typedef %s %s %s;\n", kw, sname, sname);
    }
    cg_printf(ctx, "%s %s {\n", kw, sname);
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        AstNode *fd = sd->u.struct_def.fields.data[i];
        cg_puts(ctx, "    ");
        cg_field_decl_from_ast(ctx, fd);
    }
    cg_printf(ctx, "};\n\n");

    /* Method definitions */
    ctx->cur_struct = sname;
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        cg_func(ctx, sd->u.struct_def.methods.data[i], sname);
        cg_nl(ctx);
    }
    ctx->cur_struct = NULL;
}

/* =========================================================================
 * Phase 11: Generic monomorphization
 * ====================================================================== */

/* Append the mangled suffix for a Type to a CgSB. */
static void cgb_mangle_type(CgSB *sb, Type *t) {
    if (!t) { cgb_puts(sb, "void"); return; }
    switch (t->kind) {
    case TY_VOID:      cgb_puts(sb, "void");   break;
    case TY_BOOL:      cgb_puts(sb, "bool");   break;
    case TY_CHAR:      cgb_puts(sb, "char");   break;
    case TY_SHORT:     cgb_puts(sb, "short");  break;
    case TY_INT:       cgb_puts(sb, "int");    break;
    case TY_LONG:      cgb_puts(sb, "long");   break;
    case TY_LONGLONG:  cgb_puts(sb, "llong");  break;
    case TY_UCHAR:     cgb_puts(sb, "uchar");  break;
    case TY_USHORT:    cgb_puts(sb, "ushort"); break;
    case TY_UINT:      cgb_puts(sb, "uint");   break;
    case TY_ULONG:     cgb_puts(sb, "ulong");  break;
    case TY_ULONGLONG: cgb_puts(sb, "ullong"); break;
    case TY_FLOAT:     cgb_puts(sb, "float");  break;
    case TY_DOUBLE:    cgb_puts(sb, "double"); break;
    case TY_PTR:   cgb_mangle_type(sb, t->u.ptr.base); cgb_puts(sb, "p"); break;
    case TY_CONST: cgb_puts(sb, "c"); cgb_mangle_type(sb, t->u.const_.base); break;
    case TY_STRUCT:
        cgb_puts(sb, t->u.struct_.name);
        for (size_t i = 0; i < t->u.struct_.nargs; i++) {
            cgb_puts(sb, "__");
            cgb_mangle_type(sb, t->u.struct_.args[i]);
        }
        break;
    case TY_PARAM: cgb_puts(sb, t->u.param.name); break;
    default:       cgb_puts(sb, "unk");            break;
    }
}

static char *cg_mangle_inst(const char *sname, Type **args, size_t nargs) {
    CgSB sb = {0};
    cgb_puts(&sb, sname);
    for (size_t i = 0; i < nargs; i++) {
        cgb_puts(&sb, "__");
        cgb_mangle_type(&sb, args[i]);
    }
    return cgb_take(&sb);
}

/* Substitute generic params in a Type*. */
static Type *subst_type(TyStore *ts, Type *t,
                         const char **pnames, Type **pvals, size_t np) {
    if (!t || np == 0) return t;
    switch (t->kind) {
    case TY_PARAM:
        for (size_t i = 0; i < np; i++)
            if (strcmp(t->u.param.name, pnames[i]) == 0) return pvals[i];
        return t;
    case TY_PTR:
        return ty_ptr(ts, subst_type(ts, t->u.ptr.base, pnames, pvals, np));
    case TY_CONST:
        return ty_const(ts, subst_type(ts, t->u.const_.base, pnames, pvals, np));
    case TY_ARRAY:
        return ty_array(ts, subst_type(ts, t->u.array.base, pnames, pvals, np), t->u.array.size);
    case TY_STRUCT:
        if (t->u.struct_.nargs > 0) {
            Type **na = malloc(t->u.struct_.nargs * sizeof *na);
            if (!na) abort();
            for (size_t i = 0; i < t->u.struct_.nargs; i++)
                na[i] = subst_type(ts, t->u.struct_.args[i], pnames, pvals, np);
            Type *r = ty_struct_type(ts, t->u.struct_.name, na,
                                      t->u.struct_.nargs, t->u.struct_.decl);
            free(na);
            return r;
        }
        return t;
    default: return t;
    }
}

static const AstNode *find_generic_struct(const AstNode *file, const char *name) {
    if (!file) return NULL;
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        const AstNode *d = file->u.file.decls.data[i];
        if (d && d->kind == AST_STRUCT_DEF &&
            strcmp(d->u.struct_def.name, name) == 0 &&
            d->u.struct_def.generic_params.len > 0)
            return d;
    }
    return NULL;
}

static bool cg_inst_seen(CgCtx *ctx, const char *mn) {
    for (size_t i = 0; i < ctx->ninsts; i++)
        if (strcmp(ctx->inst_names[i], mn) == 0) return true;
    if (ctx->ninsts == ctx->insts_cap) {
        ctx->insts_cap = ctx->insts_cap ? ctx->insts_cap * 2 : 8;
        ctx->inst_names = realloc(ctx->inst_names,
                                   ctx->insts_cap * sizeof *ctx->inst_names);
        if (!ctx->inst_names) abort();
    }
    ctx->inst_names[ctx->ninsts++] = cpp_xstrdup(mn);
    return false;
}

static void cg_emit_spec_method(CgCtx *ctx, const AstNode *fn,
                                  const char *msname,
                                  const char **pnames, Type **pvals, size_t np) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;
    Type *ret_t = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, cg_type_scope(ctx), NULL);
    ret_t = subst_type(ctx->ts, ret_t, pnames, pvals, np);
    cg_type(ctx, ret_t); cg_puts(ctx, " ");
    cg_method_name(ctx, msname, fn->u.func_def.name);
    cg_puts(ctx, "(");
    bool first = true;
    if (fn->u.func_def.is_const_method)
        cg_printf(ctx, "const %s *this", msname);
    else
        cg_printf(ctx, "%s *this", msname);
    first = false;
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (!p || p->u.param_decl.is_vararg) continue;
        if (!first) cg_puts(ctx, ", ");
        first = false;
        Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type, cg_type_scope(ctx), NULL);
        pt = subst_type(ctx->ts, pt, pnames, pvals, np);
        cg_decl(ctx, pt, p->u.param_decl.name);
    }
    if (first) cg_puts(ctx, "void");
    cg_puts(ctx, ")");
    if (!fn->u.func_def.body) { cg_puts(ctx, ";"); cg_nl(ctx); return; }
    cg_puts(ctx, " ");
    cg_block(ctx, fn->u.func_def.body);
}

static void cg_emit_specialization(CgCtx *ctx, Type *t);
static void cg_collect_type(CgCtx *ctx, Type *t);
static void cg_collect_block(CgCtx *ctx, const AstNode *block);
static void cg_collect_stmt(CgCtx *ctx, const AstNode *stmt);
static void cg_collect_expr(CgCtx *ctx, const AstNode *expr);
static void cg_collect_gfunc_call(CgCtx *ctx, const AstNode *call_expr);
static const char *cg_gfunc_mangle_for_call(CgCtx *ctx, const char *fname,
                                              const AstNode *call_expr);

static void cg_emit_specialization(CgCtx *ctx, Type *t) {
    if (!t || t->kind != TY_STRUCT || t->u.struct_.nargs == 0) return;
    const char *sname = t->u.struct_.name;
    char *mn = cg_mangle_inst(sname, t->u.struct_.args, t->u.struct_.nargs);
    if (cg_inst_seen(ctx, mn)) { free(mn); return; }

    const AstNode *sd = find_generic_struct(ctx->file_ast, sname);
    if (!sd) {
        cg_printf(ctx, "typedef struct %s %s;\n", mn, mn);
        free(mn); return;
    }
    size_t np = sd->u.struct_def.generic_params.len;
    if (np != t->u.struct_.nargs) { free(mn); return; }

    const char **pnames = malloc(np * sizeof *pnames);
    if (!pnames) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = sd->u.struct_def.generic_params.data[i]->u.type_param.name;

    cg_printf(ctx, "typedef struct %s %s;\n", mn, mn);
    cg_printf(ctx, "struct %s {\n", mn);
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        const AstNode *fd = sd->u.struct_def.fields.data[i];
        Type *ft = ty_from_ast(ctx->ts, fd->u.field_decl.type, cg_type_scope(ctx), NULL);
        ft = subst_type(ctx->ts, ft, pnames, t->u.struct_.args, np);
        cg_puts(ctx, "    ");
        cg_decl(ctx, ft, fd->u.field_decl.name);
        cg_puts(ctx, ";\n");
    }
    cg_printf(ctx, "};\n\n");
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        cg_emit_spec_method(ctx, sd->u.struct_def.methods.data[i],
                             mn, pnames, t->u.struct_.args, np);
        cg_nl(ctx);
    }
    free(pnames); free(mn);
}

static void cg_collect_type(CgCtx *ctx, Type *t) {
    if (!t) return;
    switch (t->kind) {
    case TY_STRUCT:
        if (t->u.struct_.nargs > 0) {
            for (size_t i = 0; i < t->u.struct_.nargs; i++)
                cg_collect_type(ctx, t->u.struct_.args[i]);
            cg_emit_specialization(ctx, t);
        }
        break;
    case TY_PTR:   cg_collect_type(ctx, t->u.ptr.base);    break;
    case TY_CONST: cg_collect_type(ctx, t->u.const_.base); break;
    default: break;
    }
}

static void cg_collect_expr(CgCtx *ctx, const AstNode *expr) {
    if (!expr) return;
    if (expr->type_ref) cg_collect_type(ctx, (Type*)expr->type_ref);
    switch (expr->kind) {
    case AST_BINOP:
        cg_collect_expr(ctx, expr->u.binop.lhs);
        cg_collect_expr(ctx, expr->u.binop.rhs);
        break;
    case AST_CALL:
        cg_collect_gfunc_call(ctx, expr);  /* Phase 11: collect generic fn calls */
        cg_collect_expr(ctx, expr->u.call.callee);
        for (size_t i = 0; i < expr->u.call.args.len; i++)
            cg_collect_expr(ctx, expr->u.call.args.data[i]);
        break;
    default: break;
    }
}

static void cg_collect_stmt(CgCtx *ctx, const AstNode *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case AST_DECL_STMT: {
        AstNode *vd = stmt->u.decl_stmt.decl;
        if (vd && vd->kind == AST_VAR_DECL) {
            if (vd->type_ref) cg_collect_type(ctx, (Type*)vd->type_ref);
            cg_collect_expr(ctx, vd->u.var_decl.init);
        }
        break;
    }
    case AST_EXPR_STMT: cg_collect_expr(ctx, stmt->u.expr_stmt.expr); break;
    case AST_RETURN:    cg_collect_expr(ctx, stmt->u.return_.value);  break;
    case AST_BLOCK:     cg_collect_block(ctx, stmt); break;
    case AST_IF:
        cg_collect_expr(ctx, stmt->u.if_.cond);
        cg_collect_stmt(ctx, stmt->u.if_.then_);
        cg_collect_stmt(ctx, stmt->u.if_.else_);
        break;
    case AST_WHILE:
        cg_collect_expr(ctx, stmt->u.while_.cond);
        cg_collect_stmt(ctx, stmt->u.while_.body);
        break;
    case AST_FOR:
        cg_collect_stmt(ctx, stmt->u.for_.init);
        cg_collect_expr(ctx, stmt->u.for_.cond);
        cg_collect_stmt(ctx, stmt->u.for_.body);
        break;
    default: break;
    }
}

static void cg_collect_block(CgCtx *ctx, const AstNode *block) {
    if (!block) return;
    for (size_t i = 0; i < block->u.block.stmts.len; i++)
        cg_collect_stmt(ctx, block->u.block.stmts.data[i]);
}


/* =========================================================================
 * Phase 11: Generic FREE-STANDING function monomorphization
 * ======================================================================== */

/* One specialization record: (function def, mangled name, concrete type args). */
typedef struct {
    const AstNode *fn;
    char          *mangle_name;
    Type         **targs;       /* concrete types [ngp] */
    size_t         ntargs;
} GFuncInst;

/* Check / record a generic function instance by mangled name. */
static bool cg_gfunc_seen(CgCtx *ctx, const char *mn) {
    for (size_t i = 0; i < ctx->ngfn; i++)
        if (strcmp(ctx->gfn_names[i], mn) == 0) return true;
    if (ctx->ngfn == ctx->gfn_cap) {
        ctx->gfn_cap = ctx->gfn_cap ? ctx->gfn_cap * 2 : 8;
        ctx->gfn_names = realloc(ctx->gfn_names,
            ctx->gfn_cap * sizeof *ctx->gfn_names);
        if (!ctx->gfn_names) abort();
    }
    ctx->gfn_names[ctx->ngfn++] = cpp_xstrdup(mn);
    return false;
}

/* Infer TY_PARAM bindings by matching formal parameter types to actual types.
 * pvals[i] is filled (or left NULL) for each param named pnames[i]. */
static void infer_gfunc_args(const AstNode *fn,
                              const char **pnames, Type **pvals, size_t np,
                              const AstNode **actual_args, size_t nactual,
                              TyStore *ts, Scope *fscope) {
    size_t nparams = fn->u.func_def.params.len;
    for (size_t pi = 0; pi < nparams && pi < nactual; pi++) {
        AstNode *par = fn->u.func_def.params.data[pi];
        if (!par) continue;
        Type *formal = ty_from_ast(ts, par->u.param_decl.type, fscope, NULL);
        const AstNode *act = actual_args[pi];
        Type *actual = act ? (Type*)act->type_ref : NULL;
        if (!formal || !actual) continue;
        /* Simple case: formal is TY_PARAM "T", actual is concrete → T=actual */
        if (formal->kind == TY_PARAM) {
            for (size_t k = 0; k < np; k++) {
                if (strcmp(formal->u.param.name, pnames[k]) == 0 &&
                    !pvals[k]) {
                    pvals[k] = actual;
                }
            }
        }
        /* Pointer case: formal is *T, actual is *concrete → T=concrete */
        if (formal->kind == TY_PTR && actual->kind == TY_PTR &&
            formal->u.ptr.base && formal->u.ptr.base->kind == TY_PARAM) {
            for (size_t k = 0; k < np; k++) {
                if (strcmp(formal->u.ptr.base->u.param.name, pnames[k]) == 0 &&
                    !pvals[k]) {
                    pvals[k] = actual->u.ptr.base;
                }
            }
        }
    }
}

/* Emit a specialized free-standing generic function. */
static void cg_emit_spec_func(CgCtx *ctx, const AstNode *fn,
                               const char *mname,
                               const char **pnames, Type **pvals, size_t np) {
    if (!fn || fn->kind != AST_FUNC_DEF || !fn->u.func_def.body) return;
    Scope *fscope = fn->type_ref ? (Scope*)fn->type_ref : ctx->file_scope;

    /* Install substitution context. */
    ctx->gp_names  = pnames;
    ctx->gp_vals   = pvals;
    ctx->ngp       = np;
    ctx->spec_scope = fscope;  /* cg_type_scope() will use this for T resolution */

    /* Return type */
    Type *ret_t = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, fscope, NULL);
    ret_t = subst_type(ctx->ts, ret_t, pnames, pvals, np);
    cg_type(ctx, ret_t);
    cg_printf(ctx, " %s(", mname);

    /* Parameters */
    bool first = true;
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (!p || p->u.param_decl.is_vararg) continue;
        if (!first) cg_puts(ctx, ", ");
        first = false;
        Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type, fscope, NULL);
        pt = subst_type(ctx->ts, pt, pnames, pvals, np);
        cg_decl(ctx, pt, p->u.param_decl.name);
    }
    if (first) cg_puts(ctx, "void");
    cg_puts(ctx, ") ");

    /* Body: cg_type inside will use gp_names/gp_vals for TY_PARAM. */
    cg_block(ctx, fn->u.func_def.body);
    cg_nl(ctx);

    /* Clear substitution context. */
    ctx->gp_names  = NULL;
    ctx->gp_vals   = NULL;
    ctx->ngp       = 0;
    ctx->spec_scope = NULL;
}

/* GFuncInst array stored on ctx (ctx fields: gfn_names dedup, plus separate list). */
static GFuncInst *g_gfinsts = NULL;
static size_t     g_ngfinsts = 0;
static size_t     g_gfinsts_cap = 0;

static void gfinst_push(const AstNode *fn, const char *mn,
                         Type **targs, size_t ntargs) {
    if (g_ngfinsts == g_gfinsts_cap) {
        g_gfinsts_cap = g_gfinsts_cap ? g_gfinsts_cap * 2 : 8;
        g_gfinsts = realloc(g_gfinsts, g_gfinsts_cap * sizeof *g_gfinsts);
        if (!g_gfinsts) abort();
    }
    Type **ta = malloc(ntargs * sizeof *ta);
    if (!ta && ntargs) abort();
    for (size_t i = 0; i < ntargs; i++) ta[i] = targs[i];
    g_gfinsts[g_ngfinsts++] = (GFuncInst){fn, cpp_xstrdup(mn), ta, ntargs};
}

static void gfinst_free_all(void) {
    for (size_t i = 0; i < g_ngfinsts; i++) {
        free(g_gfinsts[i].mangle_name);
        free(g_gfinsts[i].targs);
    }
    free(g_gfinsts);
    g_gfinsts = NULL; g_ngfinsts = 0; g_gfinsts_cap = 0;
}

/* Called from cg_collect_expr for AST_CALL nodes — detect generic fn calls. */
static void cg_collect_gfunc_call(CgCtx *ctx, const AstNode *call) {
    if (!call || call->kind != AST_CALL) return;
    const AstNode *callee = call->u.call.callee;
    if (!callee || callee->kind != AST_IDENT) return;

    Symbol *sym = scope_lookup(ctx->file_scope, callee->u.ident.name);
    if (!sym || sym->kind != SYM_FUNC || !sym->decl ||
        sym->decl->kind != AST_FUNC_DEF) return;
    const AstNode *fn = sym->decl;
    size_t np = fn->u.func_def.generic_params.len;
    if (np == 0) return;  /* not generic */

    Scope *fscope = fn->type_ref ? (Scope*)fn->type_ref : ctx->file_scope;

    /* Collect param names and infer concrete types from actual args. */
    const char **pnames = malloc(np * sizeof *pnames);
    Type       **pvals  = calloc(np, sizeof *pvals);
    if (!pnames || !pvals) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = fn->u.func_def.generic_params.data[i]->u.generic_param.name;

    /* Build actual_args array from AstNode pointers. */
    size_t nact = call->u.call.args.len;
    const AstNode **aargs = malloc(nact * sizeof *aargs);
    if (!aargs && nact) abort();
    for (size_t i = 0; i < nact; i++)
        aargs[i] = call->u.call.args.data[i];

    infer_gfunc_args(fn, pnames, pvals, np, aargs, nact, ctx->ts, fscope);
    free(aargs);

    /* Check all params were inferred; skip if any are NULL. */
    for (size_t i = 0; i < np; i++) {
        if (!pvals[i]) {
            free(pnames); free(pvals);
            return;
        }
    }

    /* Build mangled name: funcname__T1__T2 */
    CgSB sb = {0};
    cgb_puts(&sb, fn->u.func_def.name);
    for (size_t i = 0; i < np; i++) {
        cgb_puts(&sb, "__");
        cgb_mangle_type(&sb, pvals[i]);
    }
    char *mn = cgb_take(&sb);

    if (!cg_gfunc_seen(ctx, mn)) {
        gfinst_push(fn, mn, pvals, np);
    }

    free(mn);
    free(pnames);
    free(pvals);
}

/* Emit all collected generic function specializations. */
static void cg_emit_gfunc_specs(CgCtx *ctx) {
    for (size_t i = 0; i < g_ngfinsts; i++) {
        GFuncInst *gi = &g_gfinsts[i];
        const AstNode *fn = gi->fn;
        size_t np = fn->u.func_def.generic_params.len;
        const char **pnames = malloc(np * sizeof *pnames);
        if (!pnames) abort();
        for (size_t k = 0; k < np; k++)
            pnames[k] = fn->u.func_def.generic_params.data[k]->u.generic_param.name;
        cg_emit_spec_func(ctx, fn, gi->mangle_name, pnames, gi->targs, np);
        free(pnames);
    }
}

/* Rename: specializations reference mangled name at call sites. */
static const char *cg_gfunc_mangle_for_call(CgCtx *ctx, const char *fname,
                                              const AstNode *call) {
    if (!call) return fname;
    Symbol *sym = scope_lookup(ctx->file_scope, fname);
    if (!sym || sym->kind != SYM_FUNC || !sym->decl ||
        sym->decl->kind != AST_FUNC_DEF) return fname;
    const AstNode *fn = sym->decl;
    if (fn->u.func_def.generic_params.len == 0) return fname;

    size_t np = fn->u.func_def.generic_params.len;
    Scope *fscope = fn->type_ref ? (Scope*)fn->type_ref : ctx->file_scope;
    const char **pnames = malloc(np * sizeof *pnames);
    Type       **pvals  = calloc(np, sizeof *pvals);
    if (!pnames || !pvals) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = fn->u.func_def.generic_params.data[i]->u.generic_param.name;

    size_t nact = call->u.call.args.len;
    const AstNode **aargs = malloc(nact * sizeof *aargs);
    if (!aargs && nact) abort();
    for (size_t i = 0; i < nact; i++) aargs[i] = call->u.call.args.data[i];
    infer_gfunc_args(fn, pnames, pvals, np, aargs, nact, ctx->ts, fscope);
    free(aargs);

    CgSB sb = {0};
    cgb_puts(&sb, fname);
    for (size_t i = 0; i < np; i++) {
        cgb_puts(&sb, "__");
        if (pvals[i]) cgb_mangle_type(&sb, pvals[i]);
        else cgb_puts(&sb, "unk");
    }
    char *mn = cgb_take(&sb);
    /* Store in ctx to avoid leaking — we push it to gfn_names for lifetime */
    if (ctx->ngfn < ctx->gfn_cap || (ctx->gfn_cap = ctx->gfn_cap?ctx->gfn_cap*2:8,
        (ctx->gfn_names = realloc(ctx->gfn_names,
            ctx->gfn_cap * sizeof *ctx->gfn_names)) != NULL)) {
        ctx->gfn_names[ctx->ngfn++] = mn;
    }
    free(pnames); free(pvals);
    return mn;
}

static void cg_collect_all(CgCtx *ctx, const AstNode *file) {
    if (!file) return;
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        const AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        switch (d->kind) {
        case AST_FUNC_DEF: {
            Type *rt = ty_from_ast(ctx->ts, d->u.func_def.ret_type, cg_type_scope(ctx), NULL);
            cg_collect_type(ctx, rt);
            for (size_t p = 0; p < d->u.func_def.params.len; p++) {
                AstNode *pd = d->u.func_def.params.data[p];
                if (!pd) continue;
                Type *pt = ty_from_ast(ctx->ts, pd->u.param_decl.type, cg_type_scope(ctx), NULL);
                cg_collect_type(ctx, pt);
            }
            cg_collect_block(ctx, d->u.func_def.body);
            break;
        }
        case AST_VAR_DECL:
            if (d->type_ref) cg_collect_type(ctx, (Type*)d->type_ref);
            cg_collect_expr(ctx, d->u.var_decl.init);
            break;
        default: break;
        }
    }
}

/* =========================================================================
 * File-level code generation
 * ====================================================================== */
static void cg_file(CgCtx *ctx, const AstNode *file) {
    /* Preamble */
    cg_puts(ctx,
        "/* Generated by Sharp compiler (Phase 8) */\n"
        "#include <stddef.h>\n"
        "#include <stdbool.h>\n"
        "#ifndef NULL\n#define NULL ((void*)0)\n#endif\n\n");

    if (!file) return;

    /* Pass 0: scan the entire file.decls for every AST_STRUCT_DEF and
     * every self-referential typedef (`typedef struct X X;`) and emit a
     * forward typedef for each.  This must be done BEFORE Pass 1 so that
     * any struct whose definition appears later than its first use as a
     * pointer field type (e.g. `lua_State` forward-declared in lua.h and
     * used as a function-pointer parameter in lauxlib.h structs) is
     * available as an incomplete type for pointer contexts. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        const char *nm0 = NULL;
        const char *kw0 = "struct";
        if (d->kind == AST_STRUCT_DEF) {
            if (d->u.struct_def.generic_params.len > 0) continue;
            nm0 = d->u.struct_def.name;
            kw0 = d->u.struct_def.is_union ? "union" : "struct";
        } else if (d->kind == AST_TYPEDEF_DECL) {
            /* Self-referential typedef: `typedef struct X X;` or
             * `typedef union X X;` — detect by target being TYPE_NAME
             * matching the alias.  These declare the struct tag without
             * a body; we emit a forward typedef for them here. */
            const AstNode *tgt = d->u.typedef_decl.target;
            const char *alias = d->u.typedef_decl.alias;
            if (tgt && alias &&
                (tgt->kind == AST_TYPE_NAME) &&
                tgt->u.type_name.name &&
                strcmp(tgt->u.type_name.name, alias) == 0) {
                nm0 = alias;
                /* Determine struct vs union by looking up the registered
                 * symbol — scope_define may have promoted this to the
                 * real AST_STRUCT_DEF body by now. */
                kw0 = "struct";
                if (ctx->file_scope) {
                    Symbol *sym0 = scope_lookup_type(ctx->file_scope, alias);
                    if (sym0 && sym0->decl &&
                        sym0->decl->kind == AST_STRUCT_DEF &&
                        sym0->decl->u.struct_def.is_union)
                        kw0 = "union";
                }
            }
        }
        if (!nm0) continue;
        /* Check for func/var collision. */
        bool col0 = false;
        if (ctx->file_scope) {
            Symbol *s0 = scope_lookup_local(ctx->file_scope, nm0);
            for (; s0; s0 = s0->next) {
                if (strcmp(s0->name, nm0) != 0) continue;
                if (s0->kind == SYM_FUNC || s0->kind == SYM_VAR)
                    { col0 = true; break; }
            }
        }
        if (!col0)
            cg_printf(ctx, "typedef %s %s %s;\n", kw0, nm0, nm0);
    }
    cg_nl(ctx);

    /* Pass 1: forward-declare all struct/union types and emit enum
     * definitions.  Enums are emitted in full here because they have no
     * forward-vs-body distinction in C — the body is the whole
     * declaration, and enumerator names need to be in scope before any
     * code that uses them. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_STRUCT_DEF) {
            if (d->u.struct_def.generic_params.len > 0) continue;
            const char *kw = d->u.struct_def.is_union ? "union" : "struct";
            const char *snm = d->u.struct_def.name;
            /* Skip the auto `typedef struct X X;` when X is also bound
             * to an ordinary identifier in the same scope (function or
             * variable).  In that case cg_type emits `struct X` for
             * struct references (see TY_STRUCT branch), so the typedef
             * is unnecessary and would collide on the C side. */
            bool collision = false;
            if (ctx->file_scope) {
                Symbol *sym = scope_lookup_local(ctx->file_scope, snm);
                for (; sym; sym = sym->next) {
                    if (strcmp(sym->name, snm) != 0) continue;
                    if (sym->kind == SYM_FUNC || sym->kind == SYM_VAR) {
                        collision = true;
                        break;
                    }
                }
            }
            if (collision) {
                /* Plain forward decl, no typedef. */
                cg_printf(ctx, "%s %s;\n", kw, snm);
            } else {
                cg_printf(ctx, "typedef %s %s %s;\n", kw, snm, snm);
            }
        } else if (d->kind == AST_ENUM_DEF) {
            cg_puts(ctx, "enum");
            if (d->u.enum_def.name) cg_printf(ctx, " %s", d->u.enum_def.name);
            cg_puts(ctx, " {\n");
            for (size_t j = 0; j < d->u.enum_def.items.len; j++) {
                AstNode *en = d->u.enum_def.items.data[j];
                if (!en || en->kind != AST_ENUMERATOR) continue;
                cg_printf(ctx, "    %s", en->u.enumerator.name);
                if (en->u.enumerator.value) {
                    cg_puts(ctx, " = ");
                    cg_expr(ctx, en->u.enumerator.value);
                }
                cg_puts(ctx, j + 1 < d->u.enum_def.items.len ? ",\n" : "\n");
            }
            cg_puts(ctx, "};\n");
        }
    }
    cg_nl(ctx);

    /* Pass 1.5: emit full struct/union body definitions for every struct
     * that has a body, in file.decls order.  This must come BEFORE
     * Pass 1b (global variables) so that any variable declaration whose
     * type is a struct or contains a struct element (e.g. `const luaL_Reg
     * arr[]`) sees a complete type.  The forward typedef emitted in Pass 1
     * ensures there is no redefinition error here.
     *
     * We emit only the struct BODY (`struct X { ... };`), NOT the
     * `typedef struct X X;` line — that was already done in Pass 1. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d || d->kind != AST_STRUCT_DEF) continue;
        if (d->u.struct_def.generic_params.len > 0) continue;
        bool has_body = d->u.struct_def.fields.len > 0 ||
                        d->u.struct_def.methods.len > 0;
        if (!has_body) continue;
        /* Only emit the body: `struct/union Name { fields... };` */
        const char *kw15 = d->u.struct_def.is_union ? "union" : "struct";
        const char *nm15 = d->u.struct_def.name;
        cg_printf(ctx, "%s %s {\n", kw15, nm15);
        for (size_t fi = 0; fi < d->u.struct_def.fields.len; fi++) {
            AstNode *fd = d->u.struct_def.fields.data[fi];
            cg_puts(ctx, "    ");
            cg_field_decl_from_ast(ctx, fd);
        }
        cg_puts(ctx, "};\n");
    }
    cg_nl(ctx);

    /* Pass 1c: forward-declare all free functions and struct methods.
     * This must come BEFORE Pass 1b (global variables) so that global
     * array initialisers that reference function pointers (e.g. Lua's
     * `static const luaL_Reg boxmt[] = { {"__gc", boxgc}, ... };`) see
     * the function names as declared. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_FUNC_DEF && d->u.func_def.generic_params.len == 0) {
            /* Emit a forward declaration: rettype name(params); */
            cg_func_decl(ctx, d, NULL);
        } else if (d->kind == AST_STRUCT_DEF &&
                   d->u.struct_def.generic_params.len == 0) {
            for (size_t m = 0; m < d->u.struct_def.methods.len; m++) {
                AstNode *fn = d->u.struct_def.methods.data[m];
                if (fn && fn->kind == AST_FUNC_DEF &&
                    fn->u.func_def.generic_params.len == 0)
                    cg_func_decl(ctx, fn, d->u.struct_def.name);
            }
        }
    }
    cg_nl(ctx);

    /* Pass 1b: emit global variable definitions before struct methods.
     * This ensures methods that reference globals can see them. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d || d->kind != AST_VAR_DECL) continue;
        Type *t = (Type*)d->type_ref;
        if (!t) t = ty_from_ast(ctx->ts, d->u.var_decl.type, cg_type_scope(ctx), NULL);
        switch (d->u.var_decl.storage) {
        case SC_STATIC:   cg_puts(ctx, "static ");   break;
        case SC_EXTERN:   cg_puts(ctx, "extern ");   break;
        case SC_REGISTER: cg_puts(ctx, "register "); break;
        default: break;
        }
        /* Phase R2: emit C11 _Thread_local after storage-class. */
        if (d->u.var_decl.is_thread_local) cg_puts(ctx, "_Thread_local ");
        if (ast_type_outer_is_volatile(d->u.var_decl.type))
            cg_puts(ctx, "volatile ");
        /* Same array-size fallback as in cg_stmt DECL_STMT: when the
         * size is unknown from the type system, emit from the AST. */
        bool gv_array_fallback = (t && t->kind == TY_ARRAY && t->u.array.size < 0);
        if (gv_array_fallback && d->u.var_decl.type) {
            const AstNode *ast_ty = d->u.var_decl.type;
            while (ast_ty && (ast_ty->kind == AST_TYPE_CONST ||
                              ast_ty->kind == AST_TYPE_VOLATILE))
                ast_ty = ast_ty->u.type_const.base;
            if (ast_ty && ast_ty->kind == AST_TYPE_ARRAY &&
                ast_ty->u.type_array.size) {
                const AstNode *base_ast = ast_ty;
                while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                    base_ast = base_ast->u.type_array.base;
                Type *base_t = base_ast
                    ? ty_from_ast(ctx->ts, base_ast, cg_type_scope(ctx), NULL)
                    : t->u.array.base;
                cg_type(ctx, base_t ? base_t : t);
                cg_printf(ctx, " %s", d->u.var_decl.name);
                const AstNode *cur = ast_ty;
                while (cur && cur->kind == AST_TYPE_ARRAY) {
                    cg_puts(ctx, "[");
                    if (cur->u.type_array.size)
                        cg_const_expr(ctx, cur->u.type_array.size);
                    cg_puts(ctx, "]");
                    cur = cur->u.type_array.base;
                }
            } else {
                cg_decl(ctx, t, d->u.var_decl.name);
            }
        } else {
            cg_decl(ctx, t, d->u.var_decl.name);
        }
        /* Phase R6: GCC attributes after the declarator, before `=` or `;`. */
        if (d->u.var_decl.gcc_attrs) {
            cg_puts(ctx, " ");
            cg_puts(ctx, d->u.var_decl.gcc_attrs);
        }
        if (d->u.var_decl.init) { cg_puts(ctx, " = "); cg_expr(ctx, d->u.var_decl.init); }
        cg_puts(ctx, ";\n");
    }
    cg_nl(ctx);

    /* Phase 11 pass: collect and emit all generic specializations. */
    ctx->file_ast = file;
    gfinst_free_all();  /* reset global generic func instance list */
    cg_collect_all(ctx, file);
    cg_nl(ctx);
    /* Emit generic function specializations. */
    cg_emit_gfunc_specs(ctx);

    /* Pass 2: emit struct methods (Sharp extension — free functions attached
     * to a struct scope).  Struct bodies were already fully emitted in
     * Pass 1.5 so they are not re-emitted here.  cg_struct skips body-less
     * entries on its own; for entries with bodies it re-emits them which
     * would be a C redefinition error — so we explicitly skip those. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_STRUCT_DEF) {
            if (d->u.struct_def.generic_params.len > 0) continue;
            /* Only emit methods; the body was already emitted in Pass 1.5. */
            for (size_t m = 0; m < d->u.struct_def.methods.len; m++) {
                AstNode *fn = d->u.struct_def.methods.data[m];
                if (!fn || fn->kind != AST_FUNC_DEF) continue;
                cg_func(ctx, fn, d->u.struct_def.name);
                cg_nl(ctx);
            }
        }
    }

    /* Pass 3: emit free functions and global vars. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (d->kind == AST_FUNC_DEF) {
            cg_func(ctx, d, NULL);
            cg_nl(ctx);
        } else if (d->kind == AST_VAR_DECL) {
            /* Global vars already emitted in Pass 1b (before struct methods). */
            (void)d;
        } else if (d->kind == AST_TYPEDEF_DECL) {
            const AstNode *target = d->u.typedef_decl.target;
            const char    *alias  = d->u.typedef_decl.alias;
            /* Phase R5: skip re-emitting typedefs whose alias names are
             * already defined by the generated preamble headers
             * (<stddef.h>, <stdbool.h>).  These come from system headers
             * that were expanded in the preprocessed input; re-emitting
             * them causes "conflicting types" errors because the anonymous
             * struct body in the preprocessed source gets a different
             * internal name than the one in the preamble's stddef.h.
             *
             * The canonical example: lz4.h includes <stddef.h> which
             * defines `max_align_t` as an anonymous struct.  After cpp
             * preprocessing, that anonymous struct appears in the token
             * stream and cg assigns it a name (__anon_struct_N).  The
             * generated file's preamble then includes <stddef.h> again,
             * which defines `max_align_t` with a different anonymous
             * struct — causing the conflict.
             *
             * This set covers names guaranteed to be defined by
             * <stddef.h> on any GCC/clang target; it does NOT need to
             * cover names that use the same underlying type on all
             * platforms (e.g. `size_t` = `unsigned long` everywhere)
             * because those don't produce struct-vs-struct conflicts. */
            static const char * const PREAMBLE_TYPEDEF_BLOCKLIST[] = {
                "max_align_t",   /* stddef.h: struct with alignment members */
                NULL
            };
            if (alias) {
                bool skip = false;
                for (int bi = 0; PREAMBLE_TYPEDEF_BLOCKLIST[bi]; bi++) {
                    if (strcmp(alias, PREAMBLE_TYPEDEF_BLOCKLIST[bi]) == 0) {
                        skip = true; break;
                    }
                }
                if (skip) continue;
            }
            /* Self-referential `typedef struct Tag Tag;` (or the union
             * variant) — emit it verbatim with the original `struct`
             * or `union` keyword.  Without that keyword, `typedef Tag
             * Tag;` is a forward reference to itself and the C compiler
             * cannot resolve `Tag` as a type.
             *
             * The kind is determined by looking up the symbol associated
             * with `alias`: if scope_define merged it with an
             * AST_STRUCT_DEF (see scope.c S5 typedef↔struct merge), the
             * symbol now points at that struct/union body; the
             * is_union flag tells us which keyword to emit.  When the
             * forward typedef stands alone (no body anywhere), default
             * to `struct` — it's the dominant case in C and the C
             * compiler accepts struct-tag-only declarations of unions
             * as long as they remain incomplete. */
            if (target && target->kind == AST_TYPE_NAME &&
                target->u.type_name.name && alias &&
                strcmp(target->u.type_name.name, alias) == 0) {
                const char *kw = "struct";
                Symbol *sym = scope_lookup(cg_type_scope(ctx), alias);
                if (sym && sym->decl && sym->decl->kind == AST_STRUCT_DEF &&
                    sym->decl->u.struct_def.is_union) {
                    kw = "union";
                }
                cg_printf(ctx, "typedef %s %s %s;\n", kw, alias, alias);
                continue;
            }
            /* Function-typed typedefs and pointer-to-function typedefs
             * need declarator-style emission so the parameter list and
             * the alias name end up in the right places.  cg_decl knows
             * how to render `RetType (*name)(Params)` and friends, while
             * cg_type collapses a TY_FUNC to a fallback because there
             * is no compact prefix form. */
            Type *t = ty_from_ast(ctx->ts, target, cg_type_scope(ctx), NULL);
            cg_puts(ctx, "typedef ");
            cg_decl(ctx, t, alias);
            cg_puts(ctx, ";\n");
        }
    }
}

/* =========================================================================
 * Public API
 * ====================================================================== */
char *cg_generate(CgCtx *ctx, const AstNode *file) {
    cg_file(ctx, file);
    return cgb_take(&ctx->out);
}
