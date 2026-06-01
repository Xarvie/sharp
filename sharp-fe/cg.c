/*
 * cg.c -- Phase 8 Code Generation: AST → C11 source.
 */
#include "cg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* =========================================================================
 * CgCtx
 * ====================================================================== */
/* ── CgCtx — code-generation context ───────────────────────────────────────
 *
 * Fields are grouped by responsibility.  When the number of fields in any
 * group exceeds ~10, extract it into a named sub-struct embedded here.
 *
 * Group A — Core: type store, scope chain, output buffer
 * Group B — Source navigation: root file, location tracking, AST back-refs
 * Group C — Generic specialization: active params, dedup lists, instance queue
 * Group D — Emission control: mode flags, defer context, #include accumulation
 * ======================================================================== */
struct CgCtx {
    /* ── Group A: Core ──────────────────────────────────────────────── */
    TyStore       *ts;           /* type store (owns all Type* nodes)       */
    Scope         *file_scope;   /* top-level scope for symbol resolution   */
    StrBuf         out;          /* output string buffer                    */
    int            indent;       /* current indentation level               */

    /* ── Group B: Source navigation ────────────────────────────────── */
    const char    *root_file;    /* absolute path of the root source file;
                                  * decls whose loc.file != root_file come
                                  * from included headers → skipped in CG  */
    const char    *last_loc_file;/* last emitted #line file — avoids dups  */
    int            last_loc_line;/* last emitted #line number               */
    const AstNode *file_ast;     /* root AST node (AST_FILE)                */
    const AstVec  *local_block_stmts;/* block stmts for local struct lookup */
    const char    *cur_struct;   /* struct name being emitted (mangling)    */
    const AstNode *cur_struct_def;/* struct AST node for field lookup       */

    /* ── Group C: Generic specialization ───────────────────────────── */
    /* Active substitution context (set while emitting one specialization) */
    const char  **gp_names;     /* generic param names: T, U, …            */
    Type        **gp_vals;      /* concrete type for each param             */
    size_t        ngp;           /* number of active generic params          */
    Scope        *spec_scope;   /* function scope for the active spec body  */

    /* Struct specialization dedup (inst_names[i] = mangled name) */
    char        **inst_names;
    size_t        ninsts;
    size_t        insts_cap;

    /* Free-function specialization dedup */
    char        **gfn_names;
    size_t        ngfn;
    size_t        gfn_cap;
    /* ── Group D: Emission control ─────────────────────────────────── */
    bool           proto_only;    /* true → emit prototype only (no body)     */
    bool           uses_to_cstr;  /* true → @to_cstr() intrinsic is used      */

    /* Forward typedef dedup (Sharp mode Phase 1.5) */
    const char   **fwd_typedef_names; /* struct names that got early typedef   */
    size_t         n_fwd_typedef_names;

    /* Function body emission context */
    Type          *cur_fn_ret;      /* return type of function being emitted   */
    int            chain_tmp_seq;   /* unique-name counter for method chaining */

    /* Phase G: generic free-standing function instantiation list.
     * Moved from file-global to CgCtx so multiple compilation units
     * processed in the same process do not share state.
     * gfinsts / ngfinsts / gfinsts_cap replace the old g_gfinsts globals. */
    struct GFuncInst_ {
        const struct AstNode *fn;
        char                 *mangle_name;
        Type                **targs;      /* concrete type arguments */
        size_t                 ntargs;
        bool                   walked;     /* transitive body walk completed */
    }    *gfinsts;
    size_t ngfinsts;
    size_t gfinsts_cap;
};

/* =========================================================================
 * Context lifecycle and utilities
 * ====================================================================== */

static bool decl_is_user(const CgCtx *ctx, const AstNode *d) {
    /* Currently always returns true: the code generator must emit all
     * declarations (including those from system headers) because the
     * generated C code may reference types that are not fully covered
     * by the emitted #include directives alone.  The root_file check
     * is kept for potential future use when include emission is
     * complete enough to allow skipping system-header decls. */
    if (!ctx->root_file || !d) return true;
    const char *f = d->loc.file;
    if (!f || f[0] == '\0') return false;
    if (strcmp(f, ctx->root_file) == 0) return true;
    return true;
}

/* Forward declarations for output helpers.
 * Definitions follow in the Indentation helpers section. */
static void cg_puts(CgCtx *ctx, const char *s);
static void cg_nl(CgCtx *ctx);
static void cg_printf(CgCtx *ctx, const char *fmt, ...)
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((format(printf, 2, 3)))
#endif
;
static void cg_const_expr(CgCtx *ctx, const AstNode *e); /* forward */


/* Phase 11 forward declarations */
static char *cg_mangle_inst(const char *sname, Type **args, size_t nargs);
static char *cg_mangle_type_str(Type *t);
static void cg_type_from_ast(CgCtx *ctx, const AstNode *n);
static void cg_emit_func_params_ast(CgCtx *ctx, const AstNode *fn_ast);
static void cg_const_expr(CgCtx *ctx, const AstNode *n);

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
    /* Phase G: free per-ctx generic function instance list */
    for (size_t i = 0; i < ctx->ngfinsts; i++) {
        free(ctx->gfinsts[i].mangle_name);
        free(ctx->gfinsts[i].targs);
    }
    free(ctx->gfinsts);
    free(ctx->fwd_typedef_names);
    free(ctx);
}

/* =========================================================================
 * Indentation helpers
 * ====================================================================== */
static void cg_indent(CgCtx *ctx) {
    for (int i = 0; i < ctx->indent; i++) sb_push_cstr(&ctx->out, "    ");
}
static void cg_nl(CgCtx *ctx)        { sb_push_ch(&ctx->out, '\n'); }
static void cg_puts(CgCtx *ctx, const char *s) { sb_push_cstr(&ctx->out, s); }

/* =========================================================================
 * #line directive support (commercial-grade source mapping)
 *
 * GCC-style linemarker format:
 *   #line <line> "<file>"
 *
 * Rules:
 *   1. Only emit for user source files (skip system headers)
 *   2. Skip redundant markers (same file+line as last emission)
 *   3. Handle path normalization (use basename for cleaner output)
 *   4. Escape special characters in filename (backslash, double-quote)
 * ====================================================================== */

static bool cg_loc_is_user_file(CgCtx *ctx, const char *file) {
    if (!file || file[0] == '\0' || file[0] == '<') return false;
    /* If root_file is set, only track files matching it or its directory */
    if (ctx->root_file) {
        const char *rb = strrchr(ctx->root_file, '/');
        rb = rb ? rb + 1 : ctx->root_file;
        const char *fb = strrchr(file, '/');
        fb = fb ? fb + 1 : file;
        if (strcmp(fb, rb) == 0) return true;
        size_t dlen = rb - ctx->root_file;
        if (strncmp(file, ctx->root_file, dlen) == 0 &&
            (file[dlen] == '/' || file[dlen] == '\0')) return true;
        return true;
    }
    return true;
}

/**
 * Resolve a source file path to an absolute path for #line directives.
 * This ensures GDB/VS Code can always find the source file regardless
 * of the working directory.
 */
static const char *cg_resolve_path(const char *file) {
    if (!file || file[0] == '\0') return file;
#ifdef _WIN32
    static char abs_buf[1024];
    if (_fullpath(abs_buf, file, sizeof abs_buf)) return abs_buf;
#endif
    return file;
}

static void cg_escape_filename(CgCtx *ctx, const char *file) {
    for (; *file; file++) {
        if (*file == '\\' || *file == '"') {
            sb_push_ch(&ctx->out, '\\');
            sb_push_ch(&ctx->out, *file);
        } else {
            sb_push_ch(&ctx->out, *file);
        }
    }
}

/**
 * Emit a #line directive if the source location has changed.
 * Returns true if a marker was emitted.
 */
static bool cg_emit_linemarker(CgCtx *ctx, CppLoc loc) {
    if (!loc.file || loc.file[0] == '\0') return false;
    if (!cg_loc_is_user_file(ctx, loc.file)) return false;

    const char *abs_file = cg_resolve_path(loc.file);
    if (ctx->last_loc_file && ctx->last_loc_line == loc.line &&
        strcmp(ctx->last_loc_file, abs_file) == 0) return false;

    sb_push_ch(&ctx->out, '\n');
    sb_push_cstr(&ctx->out, "#line ");
    sb_printf(&ctx->out, "%d", loc.line);
    sb_push_cstr(&ctx->out, " \"");
    cg_escape_filename(ctx, abs_file);
    sb_push_cstr(&ctx->out, "\"\n");

    ctx->last_loc_file = abs_file;
    ctx->last_loc_line = loc.line;
    return true;
}

/* -------------------------------------------------------------------------
 * Method-chaining temporary helpers
 * When a struct-returning call is used as a receiver (method chaining),
 * we materialise it into a named local variable so we can take its address.
 * chain_pre accumulates declarations emitted before the enclosing statement.
 * ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
static bool is_lvalue(const AstNode *expr);

/* Check if an expression produces an rvalue struct (i.e. cannot take &).
 * Uses is_lvalue to cover all rvalue cases: calls, binops, ternaries etc. */
static bool cg_is_rvalue_struct(const AstNode *expr) {
    if (!expr) return false;
    Type *t = expr->sem_type;
    if (!t) return false;
    t = ty_unconst(NULL, t);
    if (!t || t->kind != TY_STRUCT) return false;
    /* If it's not an lvalue, C forbids & on it. */
    return !is_lvalue(expr);
}

static const AstNode *cg_find_struct_def(const CgCtx *ctx, const char *name) {
    if (!ctx->file_ast || !name) return NULL;
    for (size_t k = 0; k < ctx->file_ast->u.file.decls.len; k++) {
        const AstNode *d = ctx->file_ast->u.file.decls.data[k];
        if (d && d->kind == AST_STRUCT_DEF && d->u.struct_def.name &&
            strcmp(d->u.struct_def.name, name) == 0)
            return d;
    }
    return NULL;
}

static const AstNode *find_inline_struct_in_vec(const AstVec *vec, const char *name) {
    if (!vec) return NULL;
    for (size_t k = 0; k < vec->len; k++) {
        const AstNode *d = vec->data[k];
        if (d && d->kind == AST_STRUCT_DEF &&
            (d->u.struct_def.from_inline_var ||
             (d->u.struct_def.name && strncmp(d->u.struct_def.name, "__anon_", 7) == 0)) &&
            d->u.struct_def.name && strcmp(d->u.struct_def.name, name) == 0)
            return d;
    }
    return NULL;
}

static const AstNode *cg_find_inline_struct_def(const CgCtx *ctx, const char *name) {
    if (!name) return NULL;
    const AstNode *r = find_inline_struct_in_vec(ctx->local_block_stmts, name);
    if (r) return r;
    if (!ctx->file_ast) return NULL;
    return find_inline_struct_in_vec(&ctx->file_ast->u.file.decls, name);
}

static void cg_emit_typedef_close(CgCtx *ctx, const char *cname,
                                    const char *tail_attrs,
                                    const char *gcc_attrs) {
    if (gcc_attrs && tail_attrs)
        cg_printf(ctx, "} %s %s %s;\n", tail_attrs, cname, gcc_attrs);
    else if (tail_attrs)
        cg_printf(ctx, "} %s %s;\n", tail_attrs, cname);
    else if (gcc_attrs)
        cg_printf(ctx, "} %s %s;\n", cname, gcc_attrs);
    else
        cg_printf(ctx, "} %s;\n", cname);
}

static void cg_emit_typedef_semi(CgCtx *ctx, const char *gcc_attrs) {
    if (gcc_attrs)
        cg_printf(ctx, " %s;\n", gcc_attrs);
    else
        cg_puts(ctx, ";\n");
}

static inline bool field_is_comma_cont(const AstNode *n) {
    return n && n->kind == AST_FIELD_DECL && n->u.field_decl.is_comma_cont;
}

static void cg_printf(CgCtx *ctx, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    sb_vprintf(&ctx->out, fmt, ap);
    va_end(ap);
}

static void cg_emit_int_lit(CgCtx *ctx, const AstNode *lit) {
    if (lit->u.int_lit.is_longlong) {
        if (lit->u.int_lit.is_unsigned)
            cg_printf(ctx, "%lluULL", (unsigned long long)lit->u.int_lit.val);
        else
            cg_printf(ctx, "%lldLL", (long long)lit->u.int_lit.val);
    } else if (lit->u.int_lit.is_long) {
        if (lit->u.int_lit.is_unsigned)
            cg_printf(ctx, "%luUL", (unsigned long)lit->u.int_lit.val);
        else
            cg_printf(ctx, "%ldL", (long)lit->u.int_lit.val);
    } else if (lit->u.int_lit.is_unsigned) {
        if ((uint64_t)lit->u.int_lit.val <= 0xFFFFFFFFULL)
            cg_printf(ctx, "%uU", (unsigned)lit->u.int_lit.val);
        else
            cg_printf(ctx, "%lluULL", (unsigned long long)lit->u.int_lit.val);
    } else
        cg_printf(ctx, "%lld", (long long)lit->u.int_lit.val);
}

static void cg_emit_stars(CgCtx *ctx, int n) {
    for (int i = 0; i < n; i++) cg_puts(ctx, "*");
}

static void cg_emit_storage_class(CgCtx *ctx, StorageClass sc) {
    switch (sc) {
    case SC_STATIC:   cg_puts(ctx, "static ");   break;
    case SC_EXTERN:   cg_puts(ctx, "extern ");   break;
    case SC_REGISTER: cg_puts(ctx, "register "); break;
    default: break;
    }
}

typedef struct { bool is_const; bool is_volatile; int nptr; const AstNode *base; } AstCvpaPeel;
static AstCvpaPeel ast_type_peel_cvpa(const AstNode *n) {
    AstCvpaPeel r = {false, false, 0, n};
    while (r.base && (r.base->kind == AST_TYPE_ARRAY ||
                      r.base->kind == AST_TYPE_CONST ||
                      r.base->kind == AST_TYPE_VOLATILE ||
                      r.base->kind == AST_TYPE_ATOMIC ||
                      r.base->kind == AST_TYPE_PTR)) {
        if (r.base->kind == AST_TYPE_ARRAY)
            r.base = r.base->u.type_array.base;
        else if (r.base->kind == AST_TYPE_CONST)
            { r.is_const = true; r.base = r.base->u.type_const.base; }
        else if (r.base->kind == AST_TYPE_VOLATILE)
            { r.is_volatile = true; r.base = r.base->u.type_volatile.base; }
        else if (r.base->kind == AST_TYPE_ATOMIC)
            r.base = r.base->u.type_atomic.base;
        else
            { r.nptr++; r.base = r.base->u.type_ptr.base; }
    }
    return r;
}

typedef struct { int nstars; const AstNode *base; } AstPtrPeel;
static AstPtrPeel ast_peel_ptr(const AstNode *n) {
    AstPtrPeel r = {0, n};
    while (r.base && r.base->kind == AST_TYPE_PTR) {
        r.nstars++;
        r.base = r.base->u.type_ptr.base;
    }
    return r;
}

static const AstNode *ast_type_strip_cv(const AstNode *n) {
    while (n && (n->kind == AST_TYPE_CONST ||
                 n->kind == AST_TYPE_VOLATILE ||
                 n->kind == AST_TYPE_ATOMIC))
        n = n->kind == AST_TYPE_CONST ? n->u.type_const.base
          : n->kind == AST_TYPE_VOLATILE ? n->u.type_volatile.base
          : n->u.type_atomic.base;
    return n;
}

static const char *struct_kw(const AstNode *sd) {
    return sd->u.struct_def.is_union ? "union" : "struct";
}

static void cg_emit_array_suffixes(CgCtx *ctx, const AstNode *cur) {
    while (cur && cur->kind == AST_TYPE_ARRAY) {
        cg_puts(ctx, "[");
        if (cur->u.type_array.size)
            cg_const_expr(ctx, cur->u.type_array.size);
        cg_puts(ctx, "]");
        cur = cur->u.type_array.base;
    }
}

/* =========================================================================
 * Type generation
 * (Outputs the C declaration prefix; name is added separately)
 * ====================================================================== */

/* Forward declaration for mutual recursion. */
static void cg_type(CgCtx *ctx, Type *t);

/* ── C5: AST-based declarator emitter ────────────────────────────────────
 * In C mode, emit a full declaration (type + name) directly from the AST,
 * preserving typedef aliases.  Works for:
 *   myint x             (simple typedef alias)
 *   const myint x       (qualified alias)
 *   myint *p            (pointer to alias)
 *   myint **pp          (multiple pointers)
 *   const myint *p      (pointer to const alias)
 *   myint arr[N]        (array of alias)
 * Returns true on success; false means fall back to type-machinery path. */
/* C8: Check whether the type AST contains a true AST_TYPE_VOLATILE node
 * or an _Atomic node at any depth.  Used to choose the AST-based emission
 * path that preserves volatile/atomic in types. */
static bool ast_type_has_volatile(const AstNode *ty) {
    while (ty) {
        if (ty->kind == AST_TYPE_VOLATILE && ty->loc.line != 0) return true;
        if (ty->kind == AST_TYPE_ATOMIC) return true;
        if (ty->kind == AST_TYPE_CONST)
            { ty = ty->u.type_const.base; continue; }
        if (ty->kind == AST_TYPE_PTR)
            { ty = ty->u.type_ptr.base; continue; }
        if (ty->kind == AST_TYPE_ARRAY)
            { ty = ty->u.type_array.base; continue; }
        return false;
    }
    return false;
}

/* ast_type_is_flat_emittable -- true when the AST type node can be emitted
 * as a flat "type_text name" string using cg_type_from_ast.
 * Arrays need a suffix `name[N]` and func-ptrs need `ret (*name)(params)`,
 * so they are NOT flat-emittable and must go through cg_decl. */
static bool ast_type_is_flat_emittable(const AstNode *ty) {
    if (!ty) return false;
    switch (ty->kind) {
    case AST_TYPE_NAME:    return true;
    case AST_TYPEOF_TYPE:  return true;
    case AST_TYPE_VOID:    return true;
    case AST_TYPE_GENERIC: return true;  /* handled by cg_type_from_ast */
    case AST_TYPE_PARAM:   return true;  /* handled by cg_type_from_ast */
    case AST_TYPE_CONST:   return ast_type_is_flat_emittable(ty->u.type_const.base);
    case AST_TYPE_VOLATILE:return ast_type_is_flat_emittable(ty->u.type_volatile.base);
    case AST_TYPE_ATOMIC:  return ast_type_is_flat_emittable(ty->u.type_atomic.base);
    case AST_TYPE_PTR:     return ast_type_is_flat_emittable(ty->u.type_ptr.base);
    default:               return false;
    }
}

static void cg_decl(CgCtx *ctx, Type *t, const char *name); /* fwd */
/* cg_decl_ast -- emit "type name" using original AST nodes.
 * Handles: flat types (via cg_type_from_ast), arrays of flat types,
 * and falls back to cg_decl(Type*, name) for everything else.
 * Returns true if successfully emitted. */
static bool cg_decl_ast(CgCtx *ctx, const AstNode *ty_ast, const char *name) {
    if (!ty_ast) return false;
    /* Simple flat types: `const T * name` etc. */
    if (ast_type_is_flat_emittable(ty_ast)) {
        cg_type_from_ast(ctx, ty_ast);
        if (name) cg_printf(ctx, " %s", name);
        return true;
    }

    /* Array types: emit using AST to preserve typedef names and qualifiers.
     * Handles both 1D and multi-dimensional arrays (T name[A][B][C]).
     * Walk to the innermost non-array base type via the AST. */
    if (ty_ast->kind == AST_TYPE_ARRAY) {
        /* Find the innermost non-array base */
        const AstNode *innermost = ty_ast;
        while (innermost && innermost->kind == AST_TYPE_ARRAY)
            innermost = innermost->u.type_array.base;
        /* Array of (const) pointer-to-function type:
         * `int (*const arr[])(sqlite3*)` has innermost = PTR(FUNC) with
         * ptr_const=true.  The flat-emittable check rejects it, and the
         * semantic path loses the `const` (ty_from_ast ignores ptr_const).
         * Emit directly from AST: ret (* const name[...])(params). */
        bool inner_fnptr = false;
        bool inner_fnptr_const = false;
        const AstNode *fn_inner = innermost;
        if (fn_inner && fn_inner->kind == AST_TYPE_PTR && fn_inner->u.type_ptr.ptr_const &&
            fn_inner->u.type_ptr.base && fn_inner->u.type_ptr.base->kind == AST_TYPE_FUNC) {
            inner_fnptr = true; inner_fnptr_const = true;
        } else if (fn_inner && fn_inner->kind == AST_TYPE_PTR && !fn_inner->u.type_ptr.ptr_const &&
                   fn_inner->u.type_ptr.base && fn_inner->u.type_ptr.base->kind == AST_TYPE_FUNC) {
            inner_fnptr = true;
        }
        if (inner_fnptr && name) {
            const AstNode *fn_ast2 = fn_inner->u.type_ptr.base;
            const AstNode *ret_ast2 = fn_ast2->u.type_func.ret;
            if (ret_ast2) cg_type_from_ast(ctx, ret_ast2);
            else          cg_puts(ctx, "void");
            cg_puts(ctx, " (*");
            if (inner_fnptr_const) cg_puts(ctx, " const");
            cg_printf(ctx, " %s", name);
            /* Array dimensions */
            const AstNode *cur2 = ty_ast;
            cg_emit_array_suffixes(ctx, cur2);
            cg_puts(ctx, ")(");
            cg_emit_func_params_ast(ctx, fn_ast2);
            cg_puts(ctx, ")");
            return true;
        }
        /* Only proceed if the innermost base is flat-emittable */
        if (innermost && ast_type_is_flat_emittable(innermost)) {
            cg_type_from_ast(ctx, innermost);
            if (name) cg_printf(ctx, " %s", name);
            /* Emit array dimensions outermost-first.
             * ISO C requires: [static N] / [restrict N] -- qualifier BEFORE size. */
            const AstNode *cur = ty_ast;
            while (cur && cur->kind == AST_TYPE_ARRAY) {
                cg_puts(ctx, "[");
                if (cur->u.type_array.has_vla_star) {
                    cg_puts(ctx, "*");
                } else {
                    /* qualifiers precede the size expression */
                    if (cur->u.type_array.has_static)   cg_puts(ctx, "static ");
                    if (cur->u.type_array.has_restrict) cg_puts(ctx, "restrict ");
                    if (cur->u.type_array.size)
                        cg_const_expr(ctx, cur->u.type_array.size);
                }
                cg_puts(ctx, "]");
                cur = cur->u.type_array.base;
            }
            return true;
        }
    }

    /* Function pointer: `ret (*name)(params)` -- emit via AST to preserve
     * typedef names in return type and parameter types. */
    if (ty_ast->kind == AST_TYPE_PTR && ty_ast->u.type_ptr.base &&
        ty_ast->u.type_ptr.base->kind == AST_TYPE_FUNC) {
        const AstNode *fn_ast = ty_ast->u.type_ptr.base;
        const AstNode *ret_ast = fn_ast->u.type_func.ret;
        /* Check if return type is also a function pointer */
        bool ret_is_fnptr = (ret_ast && ret_ast->kind == AST_TYPE_PTR &&
                             ret_ast->u.type_ptr.base &&
                             ret_ast->u.type_ptr.base->kind == AST_TYPE_FUNC);
        if (ret_is_fnptr) {
            /* ret_ret (*(*name)(params))(ret_params) */
            const AstNode *rr_fn = ret_ast->u.type_ptr.base;
            cg_type_from_ast(ctx, rr_fn->u.type_func.ret);
            cg_printf(ctx, " (*(*%s)(", name ? name : "");
            cg_emit_func_params_ast(ctx, fn_ast);
            cg_puts(ctx, "))(");
            cg_emit_func_params_ast(ctx, rr_fn);
            cg_puts(ctx, ")");
        } else if (ret_ast && ret_ast->kind == AST_TYPE_PTR &&
                   ret_ast->u.type_ptr.base &&
                   ret_ast->u.type_ptr.base->kind == AST_TYPE_ARRAY) {
            /* `int (*(*fp)(void))[4]` -- fnptr returning ptr-to-array */
            const AstNode *ret_arr = ret_ast->u.type_ptr.base;
            cg_type_from_ast(ctx, ret_arr->u.type_array.base);
            cg_printf(ctx, " (*(*%s)(", name ? name : "");
            cg_emit_func_params_ast(ctx, fn_ast);
            cg_puts(ctx, "))[");
            if (ret_arr->u.type_array.size)
                cg_const_expr(ctx, ret_arr->u.type_array.size);
            cg_puts(ctx, "]");
        } else {
            /* ret (*name)(params) or ret (* const/volatile name)(params) */
            if (ret_ast) cg_type_from_ast(ctx, ret_ast);
            else         cg_puts(ctx, "void");
            if (ty_ast->u.type_ptr.ptr_const && ty_ast->u.type_ptr.ptr_volatile)
                cg_printf(ctx, " (* const volatile %s)(", name ? name : "");
            else if (ty_ast->u.type_ptr.ptr_const)
                cg_printf(ctx, " (* const %s)(", name ? name : "");
            else if (ty_ast->u.type_ptr.ptr_volatile)
                cg_printf(ctx, " (* volatile %s)(", name ? name : "");
            else
                cg_printf(ctx, " (*%s)(", name ? name : "");
            cg_emit_func_params_ast(ctx, fn_ast);
            cg_puts(ctx, ")");
        }
        return true;
    }

    return false;  /* caller should use cg_decl(Type*, name) */
}

/* Emit function parameter list for a TY_FUNC type.  Handles the nparams==0
 * case, regular params, and the is_vararg trailing `...`.
 * centralises vararg emission so cg_type / cg_decl are consistent.
 * emit empty () for nparams==0 instead of (void).  Old-style C
 * function-pointer types (e.g. `int (*)(ANYARGS)` where ANYARGS is empty)
 * must stay as `int (*)()` to remain compatible with definitions that have
 * typed parameters.  Using `(void)` converts them to an explicitly-no-arg
 * prototype which conflicts with typed definitions (C11 §6.7.6.3). */
static void cg_func_params(CgCtx *ctx, const Type *fn) {
    if (fn->u.func.nparams == 0 && !fn->u.func.is_vararg) {
        /* C8: if the AST recorded an explicit (void), preserve it.
         * Otherwise emit nothing -- `()` stays compatible with old-style
         * C function-pointer types that have typed parameters. */
        if (!fn->u.func.params_unspecified)
            cg_puts(ctx, "void");
    } else {
        for (size_t i = 0; i < fn->u.func.nparams; i++) {
            if (i) cg_puts(ctx, ", ");
            cg_type(ctx, fn->u.func.params[i]);
        }
        if (fn->u.func.is_vararg) {
            if (fn->u.func.nparams > 0) cg_puts(ctx, ", ");
            cg_puts(ctx, "...");
        }
    }
}

/* cg_type_from_ast -- emit a type using the original AST node, preserving
 * typedef names verbatim.  This avoids the array-typedef decay bug:
 *
 *   typedef struct __jmp_buf_tag jmp_buf[1];   // jmp_buf is an array typedef
 *   jmp_buf *foo(...);                          // AST: PTR(NAME("jmp_buf"))
 *
 * If we resolve NAME("jmp_buf") → TY_ARRAY and then call cg_type on
 * TY_PTR(TY_ARRAY), cg_type's fallback emits `T *` for the array part →
 * `T **` overall (wrong!).  Emitting the AST node directly gives `jmp_buf *`.
 *
 * used for function return types and typedef'd pointer types.
 */
static void cg_type_from_ast(CgCtx *ctx, const AstNode *n) {
    if (!n) { cg_puts(ctx, "void"); return; }

    switch (n->kind) {
    case AST_TYPE_NAME: {
        const char *ast_nm = n->u.type_name.name;
        if (n->u.type_name.display_name)
            ast_nm = n->u.type_name.display_name;

        /* Phase G fix: enum bare-tag reference (`enum E *p`).
         * is_enum_tag is set by the parser when the source used the `enum`
         * keyword before the tag name (no body).  Emit `enum Name` directly
         * without any scope lookup -- the tag may be forward-declared and
         * absent from scope, but the output must still say `enum E`. */
        if (n->u.type_name.is_enum_tag && n->u.type_name.name) {
            cg_printf(ctx, "enum %s", n->u.type_name.name);
            return;
        }
        /* Generic param substitution: if name matches a gp context param,
         * emit the concrete type directly. */
        if (ast_nm && ctx->ngp > 0) {
            for (size_t _gi = 0; _gi < ctx->ngp; _gi++) {
                if (ctx->gp_names[_gi] && strcmp(ast_nm, ctx->gp_names[_gi]) == 0) {
                    cg_type(ctx, ctx->gp_vals[_gi]);
                    return;
                }
            }
        }
        Type *resolved = ty_from_ast(ctx->ts, (AstNode *)n,
                                     cg_type_scope(ctx), NULL);
        /* +: if the AST node has a non-anon typedef name but
         * the resolved type involves an __anon_ struct, preserve the
         * typedef name.  This keeps "gz_statep" as "gz_statep" instead
         * of resolving to "struct __anon_struct_20 *". */
        bool preserve_typedef = false;
        if (ast_nm && strncmp(ast_nm, "__anon_", 7) != 0 && resolved) {
            /* Check if resolved type contains an anonymous struct anywhere */
            Type *check = resolved;
            while (check) {
                if (check->kind == TY_STRUCT && check->u.struct_.name &&
                    strncmp(check->u.struct_.name, "__anon_", 7) == 0) {
                    preserve_typedef = true; break;
                }
                if (check->kind == TY_PTR)    { check = check->u.ptr.base;    continue; }
                if (check->kind == TY_CONST)  { check = check->u.const_.base; continue; }
                if (check->kind == TY_ATOMIC) { check = check->u.atomic.base; continue; }
                break;
            }
        }
        if (preserve_typedef) {
            cg_puts(ctx, ast_nm);
        } else if (resolved && resolved->kind == TY_ARRAY && ast_nm) {
            cg_puts(ctx, ast_nm);
        } else if (ast_nm && n->u.type_name.is_struct_tag &&
                   ctx->file_scope) {
            /* p93: enum tag used with `enum` keyword */
            Scope *fs = cg_type_scope(ctx);
            bool is_enum_tag = false;
            bool is_union_tag = false;
            if (fs) {
                /* scope_lookup_type walks the full scope chain for SYM_TYPE */
                Symbol *sym = scope_lookup_type(fs, ast_nm);
                if (sym && sym->decl && sym->decl->kind == AST_ENUM_DEF)
                    is_enum_tag = true;
                if (sym && sym->decl && sym->decl->kind == AST_STRUCT_DEF &&
                    sym->decl->u.struct_def.is_union)
                    is_union_tag = true;
                /* Also check struct-tag namespace for union vs struct */
                if (!is_enum_tag && !is_union_tag) {
                    Symbol *tag = scope_lookup_struct_tag(fs, ast_nm);
                    if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
                        tag->decl->u.struct_def.is_union)
                        is_union_tag = true;
                }
            }
            /* When the AST used `struct X` / `union X` syntax
             * (is_struct_tag=true), always emit the struct/union keyword -- do NOT
             * fall through to cg_type() which may suppress the keyword for types
             * that also have a typedef with the same name (the was_typedef path).
             * Example: `const struct sqlite3_io_methods *pMethods` must be emitted
             * as-is; cg_type() would now output `sqlite3_io_methods *` (no struct).
             * For enums keep `enum X` (original code unchanged). */
            if (is_enum_tag)
                cg_printf(ctx, "enum %s", ast_nm);
            else
                cg_printf(ctx, "%s %s", is_union_tag ? "union" : "struct", ast_nm);
        } else if (!resolved || ty_is_error(resolved)) {
            /* local struct/typedef not in file scope -- emit verbatim */
            cg_puts(ctx, ast_nm ? ast_nm : "int");
        } else if (ast_nm && !n->u.type_name.is_struct_tag &&
                   strncmp(ast_nm, "__anon_", 7) != 0 && (
            /* GCC builtin type names starting with __ (not synthetic __anon_ tags) */
            (ast_nm[0] == '_' && ast_nm[1] == '_') ||
            /* C8: signed/unsigned __int128 variants */
            (strstr(ast_nm, "__int128") != NULL) ||
            /* A4: preserve the original source spelling for any primitive type
             * specifier: both multi-word ("unsigned short", "long long") and
             * single-word ("unsigned", "int", "short", "long", "char", "float",
             * "double", "signed", "_Bool", "void").  This avoids cg_type adding
             * a spurious `int` (e.g. `unsigned` → `unsigned int`).
             * Typedef aliases for TY_INT (like size_t) are handled below via
             * the file_scope + is_typedef_alias check; they
             * are NOT single-word C keywords so they never reach this branch. */
            (strchr(ast_nm, ' ') != NULL) ||
            (strcmp(ast_nm, "int") == 0 || strcmp(ast_nm, "unsigned") == 0 ||
             strcmp(ast_nm, "signed") == 0 || strcmp(ast_nm, "short") == 0 ||
             strcmp(ast_nm, "long") == 0 || strcmp(ast_nm, "char") == 0 ||
             strcmp(ast_nm, "float") == 0 || strcmp(ast_nm, "double") == 0 ||
             strcmp(ast_nm, "_Bool") == 0))) {
            /* Emit verbatim to preserve the spelling. */
            cg_puts(ctx, ast_nm);
        } else if (ast_nm && !n->u.type_name.is_struct_tag &&
                   ctx->file_scope) {
            /* C8/C5: in C mode, check if this name is a registered typedef
             * and emit it directly to preserve the alias (e.g. my_int_t,
             * uid_t, gid_t, size_t) instead of resolving to a primitive.
             * Also check for enum tags (TY_INT from enum → `enum X`). */
            Symbol *sym = scope_lookup(ctx->file_scope, ast_nm);
            bool is_typedef_alias = false, is_enum_tag = false;
            for (Symbol *s = sym; s; s = s->next) {
                if (strcmp(s->name, ast_nm) != 0) continue;
                if (s->kind == SYM_TYPE && s->decl) {
                    if (s->decl->kind == AST_TYPEDEF_DECL) is_typedef_alias = true;
                    if (s->decl->kind == AST_ENUM_DEF)    is_enum_tag       = true;
                    /* class: auto-typedef injected — treat as typedef alias */
                    if (s->decl->kind == AST_STRUCT_DEF &&
                        s->decl->u.struct_def.is_class)   is_typedef_alias = true;
                    if (s->was_typedef)                    is_typedef_alias = true;
                }
            }
            /* Also check struct-tag namespace for class */
            if (!is_typedef_alias) {
                Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, ast_nm);
                if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
                    tag->decl->u.struct_def.is_class) is_typedef_alias = true;
            }
            /* C8: self-referential typedef merged with struct/enum def in scope --
             * treat AST_STRUCT_DEF as typedef alias when used without struct keyword.
             * For enums, use file_ast scan to distinguish plain `enum color` (no
             * self-referential typedef → emit `enum Name`) from `typedef enum BinOpr
             * {...} BinOpr` (has typedef with same alias → emit bare `Name`). */
            /* Enum: only suppress `enum` keyword if there's an actual self-referential
             * typedef in the file (like `typedef enum BinOpr {...} BinOpr`). */
            if (is_enum_tag && !is_typedef_alias && ctx->file_ast) {
                for (size_t _fi = 0; _fi < ctx->file_ast->u.file.decls.len; _fi++) {
                    const AstNode *_fd = ctx->file_ast->u.file.decls.data[_fi];
                    if (!_fd || _fd->kind != AST_TYPEDEF_DECL) continue;
                    if (_fd->u.typedef_decl.alias &&
                        strcmp(_fd->u.typedef_decl.alias, ast_nm) == 0)
                        { is_typedef_alias = true; is_enum_tag = false; break; }
                }
            }
            if (is_typedef_alias) {
                /* If we're in a specialization context and the resolved type
                 * is a generic struct with type args, substitute unresolved
                 * TY_PARAMs and emit the mangled name (e.g. Node → Node__int
                 * instead of raw Node).  This handles self-referential types
                 * inside generic struct specializations. */
                if (ctx->ngp > 0 && resolved && resolved->kind == TY_STRUCT &&
                    resolved->u.struct_.nargs > 0) {
                    Type *subst = ty_subst(ctx->ts, resolved, ctx->gp_names,
                                           ctx->gp_vals, ctx->ngp);
                    cg_type(ctx, subst);
                } else {
                    cg_puts(ctx, ast_nm);
                }
            } else if (is_enum_tag)
                cg_printf(ctx, "enum %s", ast_nm);
            else if (!sym) {
                /* No symbol: emit verbatim (fallback for unrecognized primitives) */
                cg_puts(ctx, ast_nm);
            } else
                cg_type(ctx, resolved);
        } else {
            cg_type(ctx, resolved);
        }
        break;
    }
    case AST_STRUCT_DEF:
        /* inline struct used as var-decl base: emit `struct Tag` */
        cg_puts(ctx, n->u.struct_def.is_union ? "union " : "struct ");
        if (n->u.struct_def.name) cg_puts(ctx, n->u.struct_def.name);
        break;
    case AST_TYPE_ARRAY:
        /* Recurse to base; caller handles array suffix separately */
        cg_type_from_ast(ctx, n->u.type_array.base);
        break;
    case AST_TYPE_PTR: {
        const AstNode *base = n->u.type_ptr.base;
        /* Count consecutive PTR layers before reaching FUNC:
         * `int (**)(sqlite3_vtab*)` is PTR(PTR(FUNC(...))).  The old code
         * only handled PTR(FUNC) directly; for multi-star (PTR(PTR(FUNC)))
         * it fell to the else branch: `int (*)(params) *` -- wrong syntax.
         * Now count all leading PTR stars then emit  ret (**...)(params). */
        int nstars = 1;
        const AstNode *inner = base;
        AstPtrPeel aip = ast_peel_ptr(inner);
        nstars += aip.nstars;
        inner = aip.base;
        /* C8: if chain ends at FUNC, emit as fn-ptr: ret (**...)(params) */
        if (inner && inner->kind == AST_TYPE_FUNC) {
            const AstNode *fn_ast = inner;
            const AstNode *ret_ast = fn_ast->u.type_func.ret;
            cg_type_from_ast(ctx, ret_ast);
            cg_puts(ctx, " (");
            cg_emit_stars(ctx, nstars);
            cg_puts(ctx, ")(");
            bool fp = true;
            for (size_t i = 0; i < fn_ast->u.type_func.params.len; i++) {
                AstNode *p = fn_ast->u.type_func.params.data[i];
                if (!p) continue;
                const AstNode *pty = (p->kind == AST_PARAM_DECL)
                                     ? p->u.param_decl.type : p;
                bool is_va = (p->kind == AST_PARAM_DECL && p->u.param_decl.is_vararg);
                if (is_va) { if (!fp) cg_puts(ctx, ", "); cg_puts(ctx, "..."); fp = false; continue; }
                if (!fp) { cg_puts(ctx, ", "); } fp = false;
                cg_type_from_ast(ctx, pty);
                /* abstract declarator -- no name */
            }
            if (fp && !fn_ast->u.type_func.params_unspecified)
                cg_puts(ctx, "void");
            cg_puts(ctx, ")");
        } else if (inner && inner->kind == AST_TYPE_ARRAY) {
            /* pointer-to-array -- e.g. `char (*pa)[256]`.
             * The AST is PTR(ARRAY(256, TYPE_NAME("char"))).
             * The old else branch produced `char [256] *` (wrong).
             * Now produce `char (*)[256]` (abstract) or `char (*name)[256]`. */
            cg_type_from_ast(ctx, inner->u.type_array.base);
            cg_puts(ctx, " (");
            cg_emit_stars(ctx, nstars);
            cg_puts(ctx, ")[");
            /* ISO C: qualifiers precede size -- [static N] not [Nstatic] */
            if (inner->u.type_array.has_static)   cg_puts(ctx, "static ");
            if (inner->u.type_array.has_restrict) cg_puts(ctx, "restrict ");
            if (inner->u.type_array.has_vla_star) {
                cg_puts(ctx, "*");
            } else if (inner->u.type_array.size) {
                cg_const_expr(ctx, inner->u.type_array.size);
            }
            cg_puts(ctx, "]");
        } else {
            cg_type_from_ast(ctx, base);
            cg_puts(ctx, " *");
            if (n->u.type_ptr.nullability) {
                cg_puts(ctx, " "); cg_puts(ctx, n->u.type_ptr.nullability);
            }
            if (n->u.type_ptr.ptr_const)    cg_puts(ctx, " const");
            if (n->u.type_ptr.ptr_volatile) cg_puts(ctx, " volatile");
            if (n->u.type_ptr.ptr_restrict) {
                cg_puts(ctx, " ");
                cg_puts(ctx, (n->u.type_ptr.restrict_kw)
                             ? n->u.type_ptr.restrict_kw : "restrict");
            }
        }
        break;
    }
    case AST_TYPE_CONST: {
        const char *kw = n->u.type_const.kw
                         ? n->u.type_const.kw : "const";
        if (n->u.type_const.is_postfix) {
            cg_type_from_ast(ctx, n->u.type_const.base);
            cg_puts(ctx, " "); cg_puts(ctx, kw);
        } else {
            cg_puts(ctx, kw); cg_puts(ctx, " ");
            cg_type_from_ast(ctx, n->u.type_const.base);
        }
        break;
    }
    case AST_TYPE_VOLATILE: {
        const char *kw = n->u.type_volatile.kw
                         ? n->u.type_volatile.kw : "volatile";
        /* Post-type __attribute__ OR volatile declared after base type
         * (e.g. `void volatile **`): emit base first then qualifier. */
        if ((kw && strncmp(kw, "__attribute__", 13) == 0) || n->u.type_volatile.is_postfix) {
            cg_type_from_ast(ctx, n->u.type_volatile.base);
            cg_puts(ctx, " "); cg_puts(ctx, kw);
        } else {
            cg_puts(ctx, kw); cg_puts(ctx, " ");
            cg_type_from_ast(ctx, n->u.type_volatile.base);
        }
        break;
    }
    case AST_TYPE_ATOMIC: {
        if (n->u.type_atomic.is_paren) {
            /* _Atomic(T) parenthesised form — C11 6.7.2.4p4 */
            cg_puts(ctx, "_Atomic(");
            cg_type_from_ast(ctx, n->u.type_atomic.base);
            cg_puts(ctx, ")");
        } else {
            /* _Atomic T postfix/qualifier form — C11 6.7.2.4p5 */
            cg_puts(ctx, "_Atomic ");
            cg_type_from_ast(ctx, n->u.type_atomic.base);
        }
        break;
    }
    case AST_TYPE_PARAM: {
        /* Generic type parameter: look up in gp_names/gp_vals context first. */
        const char *pname = n->u.type_param.name;
        for (size_t _i = 0; _i < ctx->ngp; _i++) {
            if (ctx->gp_names[_i] && strcmp(pname, ctx->gp_names[_i]) == 0) {
                cg_type(ctx, ctx->gp_vals[_i]);
                return;
            }
        }
        /* Unresolved param -- emit name as-is (will be caught by cc). */
        cg_puts(ctx, pname);
        break;
    }
    case AST_TYPEOF_TYPE:
        cg_puts(ctx, n->u.typeof_type.verbatim);
        break;
    case AST_TYPE_GENERIC: {
        /* Generic instantiation like Vec<int> or Node<T> in a specialization:
         * resolve to mangled struct name with concrete args. */
        const char *gname = n->u.type_generic.name;
        size_t nargs = n->u.type_generic.args.len;
        Type **args = nargs ? malloc(nargs * sizeof *args) : NULL;
        if (nargs && !args) abort();
        for (size_t _i = 0; _i < nargs; _i++) {
            AstNode *arg_ast = n->u.type_generic.args.data[_i];
            args[_i] = NULL;
            /* Resolve type-param names (T, U, K, V …) through gp context. */
            const char *aname = NULL;
            if (arg_ast && arg_ast->kind == AST_TYPE_PARAM)
                aname = arg_ast->u.type_param.name;
            else if (arg_ast && arg_ast->kind == AST_TYPE_NAME)
                aname = arg_ast->u.type_name.name;
            if (aname) {
                for (size_t _k = 0; _k < ctx->ngp; _k++) {
                    if (ctx->gp_names[_k] && strcmp(aname, ctx->gp_names[_k]) == 0) {
                        args[_i] = ctx->gp_vals[_k]; break;
                    }
                }
            }
            if (!args[_i] && arg_ast)
                args[_i] = ty_from_ast(ctx->ts, arg_ast, cg_type_scope(ctx), NULL);
        }
        char *mn = cg_mangle_inst(gname, args, nargs);
        cg_puts(ctx, mn);
        free(mn); free(args);
        break;
    }
    default: {
        Type *t = ty_from_ast(ctx->ts, (AstNode *)n, cg_type_scope(ctx), NULL);
        cg_type(ctx, t);
        break;
    }
    }
}

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
    case TY_LONGDOUBLE:
        if (t->u.longdouble.name)
            cg_puts(ctx, t->u.longdouble.name);
        else
            cg_puts(ctx, "long double");
        break;
    case TY_CONST: {
        /* C const placement is positional, not just a leading
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
    case TY_ATOMIC: {
        Type *inner = t->u.atomic.base;
        if (inner && inner->kind == TY_PTR) {
            cg_type(ctx, inner);
            cg_puts(ctx, " _Atomic");
        } else {
            cg_puts(ctx, "_Atomic ");
            cg_type(ctx, inner);
        }
        break;
    }
    case TY_PTR: {
        /* pointer-to-function -- `ret (*)(params)` form.
         * cg_decl already has this for named declarators (line 299+);
         * cg_type's plain-TY_PTR arm would emit `func *` which isn't
         * a syntactically valid C type expression.  Detect 1+ PTR
         * layers ending in TY_FUNC and emit the abstract form
         * matching what parse_type now accepts.  sqlite3.c casts
         * through `(int (*)(int, uid_t, gid_t)) ptr` to invoke
         * variadic syscall pointers.
         *
         * handle doubly-nested function pointer:
         *   TY_PTR(TY_FUNC(ret=TY_PTR(TY_FUNC(ret=R,params=Q)), params=P))
         * which must be emitted as the abstract declarator:
         *   R (*(*)(P))(Q)   [pointer to func(P) returning pointer to func(Q)->R]
         * The naive emission `R(*)(Q) (*)(P)` is not valid C syntax. */
        TyPtrPeel pp = ty_peel_ptr((Type *)t);
        int nstars = pp.nstars;
        const Type *c = pp.base;
        if (c && c->kind == TY_FUNC) {
            /* Peel the return type to check for a nested function pointer. */
            const Type *ret = c->u.func.ret;
            TyPtrPeel ipp = ty_peel_ptr((Type *)ret);
            int inner_nstars = ipp.nstars;
            const Type *inner_c = ipp.base;
            if (inner_c && inner_c->kind == TY_FUNC) {
                /* Doubly-nested: R (*(*)(P))(Q)
                 * inner_c->ret = R, inner_c->params = Q (inner func params)
                 * c->params = P (outer func params) */
                cg_type(ctx, inner_c->u.func.ret);   /* R */
                cg_puts(ctx, " (");
                cg_emit_stars(ctx, nstars);
                cg_puts(ctx, "(");
                cg_emit_stars(ctx, inner_nstars);
                cg_puts(ctx, ")(");
                /* outer function params P */
                cg_func_params(ctx, c);
                cg_puts(ctx, "))(");
                /* inner function params Q */
                cg_func_params(ctx, inner_c);
                cg_puts(ctx, ")");
                break;
            }
            /* Single-level pointer-to-function: ret (*)(params) */
            cg_type(ctx, c->u.func.ret);
            cg_puts(ctx, " (");
            cg_emit_stars(ctx, nstars);
            cg_puts(ctx, ")(");
            cg_func_params(ctx, c);
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
             * emit the appropriate keyword prefix (`struct` or
             * `union`) for named types to avoid local-variable shadowing.
             * e.g. a local `altsvcinfo *altsvc` shadows typedef `altsvc`,
             * so `altsvc *as` is ambiguous; `struct altsvc *as` is not.
             * Look up the symbol to determine struct vs union. */
            const char *nm = t->u.struct_.name;
            /* Skip verbatim typeof(...) strings and other non-tag names */
            bool is_verbatim = (nm && (strncmp(nm, "__typeof__(", 11) == 0 ||
                                       strcmp(nm, "__builtin_va_list") == 0 ||
                                       strchr(nm, ' ') || strchr(nm, '(')));
            if (!is_verbatim) {
                /* Determine whether to emit struct/union prefix.
                 *
                 * Correct rules (typedef takes priority):
                 *  1. If scope has a typedef for this name → no prefix.
                 *     (covers `typedef struct X X`, `typedef enum{..} X`,
                 *      system-header opaque types, etc.)
                 *  2. Else if scope has a struct/union def → emit struct/union.
                 *  3. Else if struct tag found (forward decl) → emit struct.
                 *  4. Else (unknown) → emit struct (safe fallback).
                 *
                 * We use scope_lookup (walks parent chain) so that typedefs
                 * declared in an outer scope are visible from nested contexts
                 * (e.g. a function-pointer parameter inside a struct field). */
                bool needs_prefix = true; /* default: emit struct */
                bool is_union = false;
                Scope *fs = cg_type_scope(ctx);
                if (fs) {
                    /* Step 1: check for typedef -- takes priority over struct def.
                     *
                     * Two cases where a typedef exists for 'nm':
                     *  a) The symbol's decl is still AST_TYPEDEF_DECL (typedef
                     *     declared, struct def not yet seen or not self-ref).
                     *  b) scope.c S5 rule: `typedef struct X X` followed by
                     *     `struct X { ... }` promotes the symbol's decl to
                     *     AST_STRUCT_DEF and sets was_typedef=true.  We must
                     *     check was_typedef, not just decl->kind. */
                    Symbol *sym = scope_lookup(fs, nm);
                    bool found_typedef = false;
                    for (Symbol *s = sym; s; s = s->next) {
                        if (strcmp(s->name, nm) != 0) continue;
                        if (s->kind == SYM_TYPE && s->decl) {
                            if (s->was_typedef) {
                                found_typedef = true;
                                is_union = (s->decl->kind == AST_STRUCT_DEF) &&
                                            s->decl->u.struct_def.is_union;
                                break;
                            }
                            if (s->decl->kind == AST_TYPEDEF_DECL) {
                                found_typedef = true;
                                break;
                            }
                            /* class: auto-typedef injected — treat as typedef alias */
                            if (s->decl->kind == AST_STRUCT_DEF &&
                                s->decl->u.struct_def.is_class) {
                                found_typedef = true;
                                is_union = s->decl->u.struct_def.is_union;
                                break;
                            }
                        }
                    }
                    /* Also check struct-tag namespace for class */
                    if (!found_typedef) {
                        Symbol *tag = scope_lookup_struct_tag(fs, nm);
                        if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
                            tag->decl->u.struct_def.is_class) {
                            found_typedef = true;
                            is_union = tag->decl->u.struct_def.is_union;
                        }
                    }
                    if (found_typedef) {
                        needs_prefix = false;  /* explicit typedef: use alias name */
                    } else {
                        /* No explicit typedef: use struct/union prefix for valid C11 */
                        for (Symbol *s = sym; s; s = s->next) {
                            if (strcmp(s->name, nm) != 0) continue;
                            if (s->kind == SYM_TYPE && s->decl &&
                                s->decl->kind == AST_STRUCT_DEF) {
                                is_union = s->decl->u.struct_def.is_union;
                                needs_prefix = true;
                                goto emit_struct_prefix;
                            }
                        }
                        /* Check struct-tag namespace */
                        {
                            Symbol *tag = scope_lookup_struct_tag(fs, nm);
                            if (tag && tag->decl) {
                                if (tag->decl->kind == AST_STRUCT_DEF)
                                    is_union = tag->decl->u.struct_def.is_union;
                                needs_prefix = true;
                            }
                        }
                    }
                }
                emit_struct_prefix:
                if (needs_prefix) cg_puts(ctx, is_union ? "union " : "struct ");
            }
            cg_puts(ctx, nm);
        }
        break;
    case TY_ENUM:
        /* Enum type: emit `enum Tag` to preserve enum identity in C output.
         * This ensures function pointer typedefs like `typedef void (*fp)(MyEnum*)`
         * produce `void (*)(enum MyEnum *)` instead of `void (*)(int *)`,
         * which would be incompatible with `void func(MyEnum* p)`.
         * For anonymous enums with typedef alias, emit bare name (the typedef alias). */
        if (t->u.enum_.name) {
            /* Check if the enum has a typedef alias — prefer that over `enum Tag`. */
            if (ctx->file_scope) {
                Symbol *sym = scope_lookup(ctx->file_scope, t->u.enum_.name);
                if (sym && sym->decl && sym->decl->kind == AST_TYPEDEF_DECL) {
                    /* Found typedef alias: use the typedef name directly. */
                    cg_puts(ctx, t->u.enum_.name);
                    break;
                }
            }
            /* Named enum without typedef alias: emit `enum Tag`. */
            cg_puts(ctx, "enum ");
            cg_puts(ctx, t->u.enum_.name);
        } else {
            cg_puts(ctx, "int");
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
    case TY_VECTOR:
        if (t->u.vector.name)
            cg_puts(ctx, t->u.vector.name);
        else {
            cg_type(ctx, t->u.vector.elem);
            cg_printf(ctx, " __attribute__((vector_size(%d)))",
                      t->u.vector.count * 4);
        }
        break;
    case TY_FUNC:
        /* Bare function type in abstract context (e.g. as a parameter type).
         * ISO C §6.7.6.3: "a declaration of a parameter as 'function returning
         * type' shall be adjusted to be 'pointer to function returning type'".
         * Emit as abstract function-pointer form: rettype (*)(params). */
        cg_type(ctx, t->u.func.ret);
        cg_puts(ctx, " (*)(");
        cg_func_params(ctx, t);
        cg_puts(ctx, ")");
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
        /* array of function-pointer needs the nested
         * declarator form
         *
         *     ret (* name[N])(args)
         *
         * Plain `cg_type` would emit `ret (*)(args)` and the caller
         * would tack `name[N]` after -- C rejects that.  Detect when
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
        TyPtrPeel epp = ty_peel_ptr((Type *)elem);
        int elem_nstars = epp.nstars;
        const Type *peel = epp.base;
        if (peel && peel->kind == TY_FUNC) {
            cg_type(ctx, peel->u.func.ret);
            cg_puts(ctx, " (");
            cg_emit_stars(ctx, elem_nstars);
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
            cg_func_params(ctx, peel);
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
     * also handle TY_CONST(TY_PTR(TY_FUNC(...))) -- a const
     * function pointer, e.g. `compressFunc_t const compress`.  ISO C
     * requires the const to sit between the `*` and the name:
     *     int (* const compress)(void *, ...)
     * not:
     *     int (*)(void *, ...) const compress   ← rejected by cc */
    if (t && t->kind == TY_CONST) {
        const Type *inner = t->u.const_.base;
        if (inner) {
            TyPtrPeel pp = ty_peel_ptr((Type *)inner);
            int nstars = pp.nstars;
            const Type *c = pp.base;
            if (nstars > 0 && c && c->kind == TY_FUNC) {
                /* const function-pointer: ret (* const name)(params) */
                cg_type(ctx, c->u.func.ret);
                cg_puts(ctx, " (");
                cg_emit_stars(ctx, nstars);
                cg_puts(ctx, " const");
                if (name && *name) cg_printf(ctx, " %s", name);
                cg_puts(ctx, ")(");
                cg_func_params(ctx, c);
                cg_puts(ctx, ")");
                return;
            }
        }
    }
    if (t && t->kind == TY_PTR) {
        TyPtrPeel pp = ty_peel_ptr((Type *)t);
        int nstars = pp.nstars;
        const Type *c = pp.base;
        if (c && c->kind == TY_FUNC) {
            /* detect doubly-nested func-ptr return type.
             * TY_PTR(TY_FUNC(ret=TY_PTR(TY_FUNC(ret=R,params=Q)), params=P))
             * must emit: R (*(*name)(P))(Q)  -- named nested declarator.
             * The naive `cg_type(ret)` path gives `R(*)(Q) (*name)(P)` which
             * is invalid C. */
            const Type *ret = c->u.func.ret;
            TyPtrPeel ipp = ty_peel_ptr((Type *)ret);
            int inner_ns = ipp.nstars;
            const Type *inner_c = ipp.base;
            if (inner_c && inner_c->kind == TY_FUNC) {
                /* R (*(*name)(P))(Q) */
                cg_type(ctx, inner_c->u.func.ret);          /* R */
                cg_puts(ctx, " (");
                cg_emit_stars(ctx, nstars);
                cg_puts(ctx, "(");
                cg_emit_stars(ctx, inner_ns);
                if (name && *name) cg_puts(ctx, name);
                cg_puts(ctx, ")(");                          /* outer params P */
                cg_func_params(ctx, c);
                cg_puts(ctx, "))(");                         /* inner params Q */
                cg_func_params(ctx, inner_c);
                cg_puts(ctx, ")");
                return;
            }
            /* Single-level: Return type. */
            cg_type(ctx, c->u.func.ret);
            cg_puts(ctx, " (");
            cg_emit_stars(ctx, nstars);
            if (name && *name) cg_puts(ctx, name);
            cg_puts(ctx, ")(");
            cg_func_params(ctx, c);
            cg_puts(ctx, ")");
            return;
        }
    }

    /* For pointer types, the * sits between type and name. */
    /* TY_PTR(TY_ARRAY(T,N)) → nested declarator T (*name)[N].
     * Plain cg_type(TY_PTR(TY_ARRAY)) emits T ** (wrong pointer depth).
     * ISO C requires T (*name)[N] for pointer-to-array declarations. */
    if (ty_is_ptr_to_array(t)) {
        Type *arr = t->u.ptr.base;
        cg_type(ctx, arr->u.array.base);
        cg_puts(ctx, " (*");
        if (name && *name) cg_puts(ctx, name);
        cg_puts(ctx, ")");
        cg_puts(ctx, "[");
        if (arr->u.array.size >= 0) {
            char _b[32]; snprintf(_b, sizeof _b, "%lld", (long long)arr->u.array.size);
            cg_puts(ctx, _b);
        }
        cg_puts(ctx, "]");
        return;
    }
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

static void cg_emit_mangled_free_op_name(CgCtx *ctx, const char *sym,
                                          AstNode *fn) {
    cg_printf(ctx, "operator_%s", op_suffix(sym));
    for (size_t pi = 0; pi < fn->u.func_def.params.len; pi++) {
        AstNode *p = fn->u.func_def.params.data[pi];
        if (!p || p->u.param_decl.is_vararg) continue;
        Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type,
                               cg_type_scope(ctx), NULL);
        if (pt && pt->kind == TY_STRUCT && pt->u.struct_.name)
            cg_printf(ctx, "__%s", pt->u.struct_.name);
        else if (pt) {
            char *ms = cg_mangle_type_str(pt);
            cg_printf(ctx, "__%s", ms);
            free(ms);
        }
    }
}

static int cg_resolve_generic_param_count(CgCtx *ctx, int np, int type_nargs,
                                           const char *struct_name) {
    if (np == 0 && type_nargs > 0 && ctx->file_scope) {
        Symbol *ss = scope_lookup_type(ctx->file_scope, struct_name);
        if (ss && ss->kind == SYM_TYPE && ss->decl &&
            ss->decl->kind == AST_STRUCT_DEF)
            return (int)ss->decl->u.struct_def.generic_params.len;
    }
    return np;
}

static void cg_fill_pnames_from_struct(CgCtx *ctx, const char *struct_name,
                                        const char **pnames, int np) {
    Symbol *ss = scope_lookup_type(ctx->file_scope, struct_name);
    if (ss && ss->kind == SYM_TYPE && ss->decl &&
        ss->decl->kind == AST_STRUCT_DEF) {
        for (size_t i = 0;
             i < (size_t)np && i < ss->decl->u.struct_def.generic_params.len;
             i++)
            pnames[i] = ss->decl->u.struct_def.generic_params.data[i]
                              ->u.generic_param.name;
    }
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

/* Resolve a type through the specialization substitution context.
 * When inside a generic struct method specialization, TY_PARAM("T")
 * becomes the concrete type (e.g., TY_STRUCT for Vec<Vec<int>>). */
/* When inside a generic struct method specialization, TY_PARAM("T")
 * becomes the concrete type (e.g., TY_STRUCT for Vec<Vec<int>>).
 * v0.13: also substitute through TY_PTR / TY_ARRAY / TY_STRUCT args so
 * `this : HashMap<K,V>*` resolves to `HashMap<Point,int>*` rather than
 * staying as the template type. */
static Type *cg_resolve_type(CgCtx *ctx, Type *t) {
    if (!t || !ctx->gp_names) return t;
    if (t->kind == TY_CONST) {
        Type *base = cg_resolve_type(ctx, t->u.const_.base);
        return base == t->u.const_.base ? t : ty_const(ctx->ts, base);
    }
    if (t->kind == TY_ATOMIC) {
        Type *base = cg_resolve_type(ctx, t->u.atomic.base);
        return base == t->u.atomic.base ? t : ty_atomic(ctx->ts, base);
    }
    if (t->kind == TY_PARAM) {
        for (size_t i = 0; i < ctx->ngp; i++) {
            if (strcmp(t->u.param.name, ctx->gp_names[i]) == 0)
                return ctx->gp_vals[i];
        }
        return t;
    }
    /* Delegate deeper substitution to the type-store's ty_subst, which
     * already walks PTR/ARRAY/FUNC/STRUCT structurally. */
    return ty_subst(ctx->ts, t, ctx->gp_names, ctx->gp_vals, ctx->ngp);
}

/* =========================================================================
 * Forward declarations (expression codegen)
 * ====================================================================== */
static void cg_expr(CgCtx *ctx, const AstNode *expr);
static void cg_block(CgCtx *ctx, const AstNode *block);
static void cg_stmt(CgCtx *ctx, const AstNode *stmt,
                    const AstNode **defers, size_t ndefers);

static void cg_emit_rvalue_struct_addr(CgCtx *ctx, Type *type,
                                         const AstNode *expr) {
    char tmp[32];
    snprintf(tmp, sizeof tmp, "__sharp_rv%d", ctx->chain_tmp_seq++);
    cg_puts(ctx, "(__extension__(({ ");
    cg_type(ctx, type);
    cg_printf(ctx, " %s = (", tmp);
    cg_expr(ctx, expr);
    cg_printf(ctx, "); &%s; })))", tmp);
}

static Scope *cg_struct_scope(CgCtx *ctx, const char *name) {
    Symbol *sym = scope_lookup_type(ctx->file_scope, name);
    return (sym && sym->decl && sym->decl->sem_scope)
           ? (Scope *)sym->decl->sem_scope : NULL;
}

static void cg_emit_struct_receiver_arg(CgCtx *ctx, Type *type,
                                          AstNode *expr) {
    if (type && type->kind == TY_STRUCT) {
        if (is_lvalue(expr)) {
            cg_puts(ctx, "&");
            cg_expr(ctx, expr);
        } else {
            cg_emit_rvalue_struct_addr(ctx, type, expr);
        }
    } else {
        cg_expr(ctx, expr);
    }
}

/* fwd: comma-continuation declarator emitter (defined near cg_var_c) */
static void cg_emit_comma_cont_declarator(CgCtx *ctx,
                                          const AstNode *ty,
                                          const char    *name);
/* fwd: generic function mangler (single definition, used before its location) */
static const char *cg_gfunc_mangle_for_call(CgCtx *ctx, const char *fname,
                                              const AstNode *call_expr);
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
    /* v0.13: emit prototype with full original AST so cg_fn_uses_this()
     * sees the real body (needed to decide whether `this` is a parameter
     * for struct methods).  Use ctx->proto_only to suppress body output. */
    bool saved = ctx->proto_only;
    ctx->proto_only = true;
    cg_func(ctx, fn, sname);
    ctx->proto_only = saved;
}

/* =========================================================================
 * Struct/union and function emission
 * ====================================================================== */

/* Emit a single struct/union field declaration from the AST field node.
 * Unlike cg_decl (which emits from the Type*), this path preserves the
 * original array-size expression so enum-valued sizes like `TM_N` or
 * `LUAI_NUMTAGS` are emitted correctly instead of as `[]`. */
/* Emit a constant expression (array size, enum value, bit-field width)
 * directly from the AST without requiring sema type annotations.
 * Handles the subset of expressions that appear as static constant sizes
 * in C declarations: integer literals, identifiers (enum names, macros),
 * casts, sizeof, and binary arithmetic.  Falls back to a numeric 0 for
 * unrecognised nodes -- the C compiler will report a real error if the
 * expression is actually invalid; we preserve the shape so it can. */
static void cg_const_expr(CgCtx *ctx, const AstNode *e) {
    if (!e) { cg_puts(ctx, "0"); return; }
    switch (e->kind) {
    case AST_INT_LIT:
        if (e->u.int_lit.orig_text) {
            cg_puts(ctx, e->u.int_lit.orig_text);
        } else {
            cg_emit_int_lit(ctx, e);
        }
        break;
    case AST_IDENT:
        cg_puts(ctx, e->u.ident.name ? e->u.ident.name : "0");
        break;
    case AST_PAREN:   /* C8 */
        cg_puts(ctx, "(");
        cg_const_expr(ctx, e->u.paren.inner);
        cg_puts(ctx, ")");
        break;
    case AST_CAST: {
        /* (type)expr -- emit with the declared cast type, not the sema type. */
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
        cg_puts(ctx, ")");
        cg_const_expr(ctx, e->u.cast.operand);
        break;
    }
    case AST_SIZEOF:
        /* sizeof(type) and sizeof(expr) in constant contexts.
         * When the operand is a type, emit via ty_from_ast.
         * When the operand is an expression, emit the expression via
         * cg_expr (not cg_const_expr) so that array subscripts, function
         * calls, and other non-const forms are emitted correctly.
         * cc evaluates sizeof(expr) as a compile-time constant. */
        cg_puts(ctx, e->u.sizeof_.is_alignof ? "_Alignof(" : "sizeof(");
        if (e->u.sizeof_.is_type && e->u.sizeof_.operand) {
            /* Use AST-direct emission to preserve typedef names (e.g.
             * sizeof(lua_Number) stays as lua_Number, not double). */
            cg_type_from_ast(ctx, e->u.sizeof_.operand);
        } else {
            /* Expression operand: delegate to cg_expr for full coverage. */
            cg_expr(ctx, e->u.sizeof_.operand);
        }
        cg_puts(ctx, ")");
        break;
    case AST_BINOP:
        /* C8: don't add outer parens -- the AST_PAREN node from user-written
         * parens already handles grouping. Adding parens here causes
         * `((1 == 1))` when source had `(1 == 1)`. In array-size constant
         * expressions, standard C operator precedence applies without help. */
        cg_const_expr(ctx, e->u.binop.lhs);
        cg_printf(ctx, " %s ", binop_str(e->u.binop.op));
        cg_const_expr(ctx, e->u.binop.rhs);
        break;
    case AST_UNARY:
        if (!e->u.unary.postfix) {
            static const char *uops[] = {
                [STOK_MINUS]="-",[STOK_BANG]="!",[STOK_TILDE]="~",[STOK_PLUS]="+"
            };
            SharpTokKind op = e->u.unary.op;
            if (op < STOK_COUNT && uops[op])
                cg_puts(ctx, uops[op]);
            /* Same spacing fix as cg_expr: prevent -- / ++ / !! / ~~ merge */
            if (op == STOK_MINUS || op == STOK_PLUS ||
                op == STOK_BANG || op == STOK_TILDE) {
                const AstNode *inner = e->u.unary.operand;
                while (inner && inner->kind == AST_PAREN)
                    inner = inner->u.paren.inner;
                if (inner && inner->kind == AST_UNARY && !inner->u.unary.postfix) {
                    SharpTokKind iop = inner->u.unary.op;
                    if (iop == STOK_MINUS || iop == STOK_PLUS ||
                        iop == STOK_BANG || iop == STOK_TILDE)
                        cg_puts(ctx, " ");
                }
            }
        }
        cg_const_expr(ctx, e->u.unary.operand);
        break;
    case AST_TERNARY:
        /* ternary `cond ? then : else` in array-size expressions.
         * `MAX(a,b)` expands to `((a)>(b) ? (a) : (b))` -- without this case,
         * `cg_const_expr` would fall to `default:` and emit `0`, causing
         * struct fields like `workspace[ZSTD_BUILD_FSE_TABLE_WKSP_SIZE_U32]`
         * to be emitted with the wrong size.
         * C8: no outer parens here -- the ternary itself is at top level
         * inside [...] so extra wrapping makes `(((1==1)) ? 1 : -1)`. */
        cg_const_expr(ctx, e->u.ternary.cond);
        cg_puts(ctx, " ? ");
        cg_const_expr(ctx, e->u.ternary.then_);
        cg_puts(ctx, " : ");
        cg_const_expr(ctx, e->u.ternary.else_);
        break;
    default:
        cg_puts(ctx, "0");
        break;
    }
}

static void cg_emit_field_group(CgCtx *ctx, const AstVec *fields, size_t start); /* fwd */
static void cg_emit_struct_fields(CgCtx *ctx, const AstNode *sd,
                                   const char *indent,
                                   bool try_inline); /* fwd */

static void cg_field_decl_from_ast(CgCtx *ctx, const AstNode *fd) {
    if (!fd || fd->kind != AST_FIELD_DECL) return;

    /* anonymous struct/union field injection.
     * Fields synthesised with name `__anon_field_N` (from parse.c's
     * anonymous-aggregate handling) must be re-emitted as an INLINE BODY
     * without a tag name and without a declarator name.
     *
     * C §6.7.2.1: anonymous union/struct requires no tag AND no declarator.
     * Emitting `union __anon_union_23;` (tagged, no declarator) is NOT an
     * anonymous union and cc does not inject its members.
     * Correct form: `union { fields... };` (no tag, no declarator).
     *
     * Implementation: find the __anon_struct_N def in file_ast, emit its
     * body inline using the `union { ... };` / `struct { ... };` syntax. */
    const char *fname = fd->u.field_decl.name;
    /* Preserve parenthesized field names like `(requests)` from the source. */
    char _fname_paren_buf[256];
    if (fd->u.field_decl.name_paren && fname) {
        snprintf(_fname_paren_buf, sizeof _fname_paren_buf, "(%s)", fname);
        fname = _fname_paren_buf;
    }
    bool is_anon = fname && strncmp(fd->u.field_decl.name ? fd->u.field_decl.name : "", "__anon_", 7) == 0;

    if (is_anon && fd->u.field_decl.type &&
        fd->u.field_decl.type->kind == AST_TYPE_NAME &&
        ctx->file_ast) {
        const char *inner_name = fd->u.field_decl.type->u.type_name.name;
        const AstNode *inner_sd = cg_find_struct_def(ctx, inner_name);
        if (inner_sd && inner_sd->u.struct_def.fields.len > 0) {
            /* Emit inline body: `union {\n  fields...\n};` */
            const char *kw = struct_kw(inner_sd);
            cg_printf(ctx, "%s {\n", kw);
            for (size_t j = 0; j < inner_sd->u.struct_def.fields.len; j++) {
                const AstNode *sub = inner_sd->u.struct_def.fields.data[j];
                cg_puts(ctx, "    ");  /* indent inside inner body */
                cg_field_decl_from_ast(ctx, sub);
            }
            cg_puts(ctx, "    };\n");
            return;  /* body emitted; skip the normal path */
        }
        /* If we can't find or inline the body, fall through to
         * the normal path which emits the tag reference. */
    }

    /* C8: NAMED field with anonymous struct/union TYPE (direct or array).
     * Source: `struct { int x; } pt;` or `struct { int x; } arr[N];` */
    {
        const AstNode *fty_inner = fd->u.field_decl.type;
        while (fty_inner && fty_inner->kind == AST_TYPE_ARRAY)
            fty_inner = fty_inner->u.type_array.base;
        if (!is_anon && fname && fty_inner && fty_inner->kind == AST_TYPE_NAME &&
            fty_inner->u.type_name.name &&
            strncmp(fty_inner->u.type_name.name, "__anon_", 7) == 0 && ctx->file_ast) {
            const char *inner_name = fty_inner->u.type_name.name;
            const AstNode *inner_sd = cg_find_struct_def(ctx, inner_name);
            if (!inner_sd && ctx->local_block_stmts) {
                for (size_t k = 0; k < ctx->local_block_stmts->len; k++) {
                    const AstNode *d2 = ctx->local_block_stmts->data[k];
                    if (d2 && d2->kind == AST_STRUCT_DEF && d2->u.struct_def.name &&
                        strcmp(d2->u.struct_def.name, inner_name) == 0) { inner_sd = d2; break; }
                }
            }
            if (inner_sd && inner_sd->u.struct_def.fields.len > 0) {
                const char *kw = struct_kw(inner_sd);
                cg_printf(ctx, "%s {\n", kw);
                cg_emit_struct_fields(ctx, inner_sd, "    ", false);
                if (inner_sd->u.struct_def.tail_attrs)
                    cg_printf(ctx, "} %s %s", inner_sd->u.struct_def.tail_attrs, fname);
                else
                    cg_printf(ctx, "} %s", fname);
                /* Emit array suffixes if the field type is an array */
                const AstNode *arr = fd->u.field_decl.type;
                cg_emit_array_suffixes(ctx, arr);
                cg_puts(ctx, ";\n");
                return;
            }
        }
    }

    /* C-mode: named field whose base type is a nested struct defined inside
     * this struct body (is_nested_in_struct flag).  Emit inline:
     *   struct sColMap { int iFrom; char *zCol; } aCol[1];
     *   struct InLoop { ... } *aInLoop;  */
    if (fname && ctx->file_ast) {
        AstCvpaPeel fp = ast_type_peel_cvpa(fd->u.field_decl.type);
        if (fp.base && fp.base->kind == AST_TYPE_NAME &&
            fp.base->u.type_name.name) {
            const char *inner_name = fp.base->u.type_name.name;
            /* Find matching nested struct def */
            const AstNode *inner_sd = NULL;
            for (size_t k = 0; k < ctx->file_ast->u.file.decls.len; k++) {
                const AstNode *d2 = ctx->file_ast->u.file.decls.data[k];
                if (d2 && d2->kind == AST_STRUCT_DEF &&
                    d2->u.struct_def.is_nested_in_struct &&
                    d2->u.struct_def.name &&
                    strcmp(d2->u.struct_def.name, inner_name) == 0) {
                    inner_sd = d2; break;
                }
            }
            if (inner_sd && inner_sd->u.struct_def.fields.len > 0) {
                if (inner_sd->u.struct_def.name &&
                    strncmp(inner_sd->u.struct_def.name, "__anon_", 7) != 0 &&
                    !inner_sd->u.struct_def.from_inline_typedef) {
                    /* Named nested struct is emitted standalone by
                     * cg_emit_decl_sharp — fall through so the field uses
                     * a tag-name reference instead of an inline body.
                     * Exception: when from_inline_typedef is true,
                     * cg_emit_decl_sharp skips it, so we must emit the
                     * body inline here (e.g. union __WIDL_... inside a
                     * typedef struct). */
                } else {
                    const char *kw = struct_kw(inner_sd);
                    cg_printf(ctx, "%s %s {\n", kw, inner_name);
                    cg_emit_struct_fields(ctx, inner_sd, "    ", false);
                    if (inner_sd->u.struct_def.tail_attrs)
                        cg_printf(ctx, "} %s", inner_sd->u.struct_def.tail_attrs);
                    else
                        cg_puts(ctx, "}");
                    /* Emit pointer stars between struct body and name */
                    cg_emit_stars(ctx, fp.nptr);
                    cg_printf(ctx, " %s", fname);
                    /* Emit array suffix(es) */
                    const AstNode *arr = fd->u.field_decl.type;
                    cg_emit_array_suffixes(ctx, arr);
                    cg_puts(ctx, ";\n");
                    return;
                }
            }
        }
    }
    Type *ft = ty_from_ast(ctx->ts, fd->u.field_decl.type,
                           cg_type_scope(ctx), NULL);
    /* Check whether the field's AST type has an array suffix with a
     * non-literal size expression that ty_from_ast could not evaluate
     * (returns sz == -1).  If so, emit the declaration directly from
     * the AST to preserve the original expression. */
    bool array_with_unknown_sz = false;
    if (ft && ft->kind == TY_ARRAY && ft->u.array.size < 0) {
        const AstNode *ast_ty = ast_type_strip_cv(fd->u.field_decl.type);
        if (ast_ty && ast_ty->kind == AST_TYPE_ARRAY &&
            ast_ty->u.type_array.size &&
            ast_ty->u.type_array.size->kind != AST_INT_LIT) {
            array_with_unknown_sz = true;
            const AstNode *base_ast = ast_ty;
            while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                base_ast = base_ast->u.type_array.base;
            /* Use AST-direct emission to preserve typedef names (e.g.
             * TString not "struct TString", StkId not "union StackValue *"). */
            if (base_ast)
                cg_type_from_ast(ctx, base_ast);
            else {
                Type *base_t = base_ast
                    ? ty_from_ast(ctx->ts, base_ast, cg_type_scope(ctx), NULL)
                    : ft->u.array.base;
                cg_type(ctx, base_t ? base_t : ft);
            }
            cg_printf(ctx, " %s", fname);
            const AstNode *cur = ast_ty;
            cg_emit_array_suffixes(ctx, cur);
        }
    }
    if (!array_with_unknown_sz) {
        /* C8: use cg_type_from_ast when field type has volatile/_Atomic */
        const AstNode *fty = fd->u.field_decl.type;
        bool fhas_qual = fty &&
            (ast_type_has_volatile(fty) ||
             fty->kind == AST_TYPE_ATOMIC);
        /* C8/C5: for function-pointer fields, use cg_type_from_ast for the
         * return type to preserve typedef aliases (e.g. `my_int_t (*fn)(int)`
         * instead of `int (*fn)(int)` when my_int_t is a typedef for int). */
        bool is_fnptr = ty_is_func_ptr(ft);
        bool use_ast_ret = is_fnptr && fty &&
                           fty->kind == AST_TYPE_PTR &&
                           fty->u.type_ptr.base &&
                           fty->u.type_ptr.base->kind == AST_TYPE_FUNC;
        if (fhas_qual && fname) {
            cg_type_from_ast(ctx, fty);
            cg_printf(ctx, " %s", fname);
        } else if (use_ast_ret && fname) {
            /* Emit function-pointer field. Two cases:
             * (A) Normal:   ret (*name)(params)
             * (B) Returns fn-ptr: ret_ret (*(*name)(params))(ret_params)
             *     e.g. `void (*(*xDlSym)(vfs*,void*,char*))(void)` */
            const AstNode *fn_ast = fty->u.type_ptr.base; /* the FUNC node */
            const AstNode *ret_ast = fn_ast->u.type_func.ret;
            /* Check if return type is also a function pointer */
            bool ret_is_fnptr = (ret_ast && ret_ast->kind == AST_TYPE_PTR &&
                                 ret_ast->u.type_ptr.base &&
                                 ret_ast->u.type_ptr.base->kind == AST_TYPE_FUNC);
            if (ret_is_fnptr) {
                /* Case B: `ret_ret (*(*name)(params))(ret_params)` */
                const AstNode *ret_fn = ret_ast->u.type_ptr.base; /* ret's FUNC */
                cg_type_from_ast(ctx, ret_fn->u.type_func.ret); /* deepest ret */
                cg_printf(ctx, " (*(*%s)(", fname);
                cg_emit_func_params_ast(ctx, fn_ast);
                cg_puts(ctx, "))(");
                cg_emit_func_params_ast(ctx, ret_fn);
                cg_puts(ctx, ")");
            } else {
                /* Case A: normal `ret (*name)(params)` */
                cg_type_from_ast(ctx, ret_ast);
                cg_printf(ctx, " (*%s)(", fname);
                cg_emit_func_params_ast(ctx, fn_ast);
                cg_puts(ctx, ")");
            }
        } else {
            /* C5: emit from AST to preserve qualifiers, typedef aliases, and array forms. */
            Type *ft2 = ft;
            bool emitted = fty && cg_decl_ast(ctx, fty, fname);
            if (!emitted) cg_decl(ctx, ft2, fname);
        }
    }
    if (fd->u.field_decl.bit_width) {
        cg_puts(ctx, " : ");
        cg_expr(ctx, fd->u.field_decl.bit_width);
    }
    /* C7: emit GCC field attributes after the declarator */
    if (fd->u.field_decl.gcc_attrs) {
        cg_puts(ctx, " ");
        cg_puts(ctx, fd->u.field_decl.gcc_attrs);
    }
    cg_puts(ctx, ";\n");
}

/* A8: Emit a group of struct fields, respecting comma-continuation.
 * `fields[start]` is the first field; subsequent fields with is_comma_cont
 * are emitted as ", name : width" on the same line. */
static void cg_emit_field_group(CgCtx *ctx, const AstVec *fields,
                                 size_t start) {
    const AstNode *fd = fields->data[start];
    if (!fd || fd->kind != AST_FIELD_DECL) return;

    /* Is this a continuation itself? Emit normally (fallback). */
    if (fd->u.field_decl.is_comma_cont) {
        cg_field_decl_from_ast(ctx, fd);
        return;
    }

    /* Find how many fields follow as comma continuations. */
    size_t end = start + 1;
    while (end < fields->len) {
        const AstNode *nxt = fields->data[end];
        if (!nxt || nxt->kind != AST_FIELD_DECL ||
            !nxt->u.field_decl.is_comma_cont) break;
        end++;
    }
    /* Single field -- emit normally. */
    if (end == start + 1) {
        cg_field_decl_from_ast(ctx, fd);
        return;
    }

    /* Multi-field comma group: emit "type name[:w], name[:w], ...;\n" */
    /* Emit up to (but not including) the trailing ";\n" of the first field.
     * We replicate the relevant parts of cg_field_decl_from_ast here. */
    const AstNode *fty = fd->u.field_decl.type;
    const char *fname  = fd->u.field_decl.name;
    if (!fty || !fname) { cg_field_decl_from_ast(ctx, fd); return; }

    /* type + first declarator */
    bool emitted = cg_decl_ast(ctx, fty, fname);
    if (!emitted) {
        Type *ft = ty_from_ast(ctx->ts, (AstNode *)fty,
                               cg_type_scope(ctx), NULL);
        cg_decl(ctx, ft, fname);
    }
    if (fd->u.field_decl.bit_width) {
        cg_puts(ctx, " : ");
        cg_expr(ctx, fd->u.field_decl.bit_width);
    }

    /* continuations: emit declarator suffix (pointer stars etc.) + name */
    for (size_t k = start + 1; k < end; k++) {
        const AstNode *cf = fields->data[k];
        cg_puts(ctx, ", ");
        /* Emit the declarator part of the continuation.  The base type was
         * already emitted above; we need only the ptr/const/array wrappers
         * and the name.  Walk the type tree and collect wrappers. */
        if (cf->u.field_decl.type && cf->u.field_decl.type != fty) {
            /* Collect wrapper nodes (outermost first) */
            const AstNode *wrappers[32];
            size_t nw = 0;
            const AstNode *cur = cf->u.field_decl.type;
            while (cur && nw < 32) {
                if (cur->kind == AST_TYPE_PTR) {
                    wrappers[nw++] = cur;
                    cur = cur->u.type_ptr.base;
                } else {
                    break; /* stop at base type qualifiers -- shared by all declarators */
                }
            }
            /* Emit pointer wrappers innermost-first */
            for (size_t _w = nw; _w > 0; _w--) {
                const AstNode *wr = wrappers[_w - 1];
                cg_puts(ctx, "*");
                if (wr->u.type_ptr.nullability) {
                    cg_puts(ctx, " ");
                    cg_puts(ctx, wr->u.type_ptr.nullability);
                }
                if (wr->u.type_ptr.ptr_const)    cg_puts(ctx, " const");
                if (wr->u.type_ptr.ptr_volatile) cg_puts(ctx, " volatile");
                if (wr->u.type_ptr.ptr_restrict)
                    cg_puts(ctx, wr->u.type_ptr.restrict_kw ? wr->u.type_ptr.restrict_kw : " restrict");
                cg_puts(ctx, " ");
            }
        }
        cg_puts(ctx, cf->u.field_decl.name ? cf->u.field_decl.name : "?");
        /* Emit array suffixes for the continuation field.
         * The base type was already emitted above; continuation fields
         * only need pointer wrappers (emitted above) + array suffixes. */
        {
            const AstNode *arrs[32];
            size_t na = 0;
            const AstNode *acur = cf->u.field_decl.type;
            while (acur && acur->kind == AST_TYPE_ARRAY && na < 32) {
                arrs[na++] = acur;
                acur = acur->u.type_array.base;
            }
            for (size_t _ai = na; _ai > 0; _ai--) {
                const AstNode *ar = arrs[_ai - 1];
                cg_puts(ctx, "[");
                if (ar->u.type_array.size) cg_const_expr(ctx, ar->u.type_array.size);
                cg_puts(ctx, "]");
            }
        }
        if (cf->u.field_decl.bit_width) {
            cg_puts(ctx, " : ");
            cg_expr(ctx, cf->u.field_decl.bit_width);
        }
    }
    if (fd->u.field_decl.gcc_attrs) {
        cg_puts(ctx, " ");
        cg_puts(ctx, fd->u.field_decl.gcc_attrs);
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
    const char *s = fe_op_sym(op);
    return s ? s : "/*?op*/";
}

/* Get the struct name from a type (TY_STRUCT), stripping const/ptr. */
static const char *struct_name_of(Type *t) {
    Type *s = ty_peel_to_struct(t);
    return s ? s->u.struct_.name : NULL;
}

/* Return the C operator precedence for a binary/ternary/assign op.
 * Higher = tighter binding.  Used to decide when parens are necessary. */
static int binop_prec(SharpTokKind op) {
    switch (op) {
    case STOK_COMMA:                              return 1;
    case STOK_EQ: case STOK_PLUSEQ: case STOK_MINUSEQ:
    case STOK_STAREQ: case STOK_SLASHEQ: case STOK_PERCENTEQ:
    case STOK_AMPEQ: case STOK_PIPEEQ: case STOK_CARETEQ:
    case STOK_LTLTEQ: case STOK_GTGTEQ:      return 2;
    case STOK_PIPEPIPE:                           return 4;
    case STOK_AMPAMP:                             return 5;
    case STOK_PIPE:                               return 6;
    case STOK_CARET:                              return 7;
    case STOK_AMP:                                return 8;
    case STOK_EQEQ: case STOK_BANGEQ:            return 9;
    case STOK_LT: case STOK_GT:
    case STOK_LTEQ: case STOK_GTEQ:              return 10;
    case STOK_LTLT: case STOK_GTGT:           return 11;
    case STOK_PLUS: case STOK_MINUS:              return 12;
    case STOK_STAR: case STOK_SLASH: case STOK_PERCENT: return 13;
    default:                                       return 14;
    }
}

/* Check if an expression node is an addressable lvalue in C.
 * Used by operator-overload codegen to decide whether &lhs is valid. */
static bool is_lvalue(const AstNode *expr) {
    if (!expr) return false;
    switch (expr->kind) {
    case AST_IDENT:         return true;  /* variable */
    case AST_FIELD_ACCESS:  return true;  /* recv.field / recv->field */
    case AST_INDEX:         return true;  /* base[index] */
    case AST_COMPOUND_LIT:  return true;  /* (Type){...} — C compound literal */
    case AST_PAREN:         return is_lvalue(expr->u.paren.inner);
    case AST_UNARY:
        return expr->u.unary.op == STOK_STAR;  /* *ptr deref */
    default:
        return false;  /* calls, binops, literals, casts, ternaries etc. */
    }
}

/* Emit expr, wrapping in parens if its precedence is strictly less than
 * min_prec (i.e. it would bind more loosely than the surrounding context). */
static void cg_expr_p(CgCtx *ctx, const AstNode *expr, int min_prec);

/* Emit expr at statement-top-level: strip redundant outer parens that
 * cg_expr(BINOP) would add.  Safe only when there is no surrounding
 * expression context (return value, bare expression statement). */
static void cg_expr_toplevel(CgCtx *ctx, const AstNode *expr) {
    if (!expr) { return; }  /* null statement: caller emits ";" */
    /* BINOP: emit as "lhs op rhs" without wrapping parens. */
    if (expr->kind == AST_BINOP) {
        /* Check for operator overload (Sharp) -- fall through to cg_expr. */
        if (expr->u.binop.lhs) {
            Type *lt = sema_type_of(expr->u.binop.lhs);
            Type *ltu = ty_unconst(ctx->ts, lt);
            if (ltu && ltu->kind == TY_STRUCT) {
                /* Might be overloaded -- let cg_expr handle. */
                cg_expr(ctx, expr);
                return;
            }
        }
        cg_expr(ctx, expr->u.binop.lhs);
        cg_printf(ctx, " %s ", binop_str(expr->u.binop.op));
        cg_expr(ctx, expr->u.binop.rhs);
        return;
    }
    /* TERNARY: emit as "cond ? then : else" without outer parens. */
    if (expr->kind == AST_TERNARY) {
        cg_expr_toplevel(ctx, expr->u.ternary.cond);
        cg_puts(ctx, " ? ");
        cg_expr_toplevel(ctx, expr->u.ternary.then_);
        cg_puts(ctx, " : ");
        cg_expr_toplevel(ctx, expr->u.ternary.else_);
        return;
    }
    cg_expr(ctx, expr);
}

static void cg_expr(CgCtx *ctx, const AstNode *expr) {
    if (!expr) { cg_puts(ctx, "/*null-expr*/"); return; }
    
    /* C23: if this is a compile-time constant expression and we're in a constant context,
     * evaluate it at compile time and emit the result directly. */
    /* Note: we need to pass the sema context for proper symbol resolution */
    /* For now, let's add a simplified version that evaluates if possible */
    
    switch (expr->kind) {

    case AST_INT_LIT:
        if (expr->u.int_lit.orig_text) {
            cg_puts(ctx, expr->u.int_lit.orig_text);
            break;
        }
        cg_emit_int_lit(ctx, expr);
        break;
    case AST_FLOAT_LIT: {
        /* C3: in C mode use the original token text to preserve float
         * suffixes (f, F, L) and exact representation (e.g. 1.5f, 2.0L). */
        if (expr->u.float_lit.orig_text) {
            cg_puts(ctx, expr->u.float_lit.orig_text);
            break;
        }
        /* Phase S5: use %.17g for round-trip precision.  The default
         * %g prints 6 significant digits, which silently truncates
         * constants like Lua's `(3.141592653589793238462643383279502884)`
         * down to `3.14159` and fails math.tan(pi/4) by ~1.3e-6.
         * 17 is DBL_DECIMAL_DIG -- guaranteed enough to uniquely identify
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
        /* C3: in C mode use the original token text to preserve 'c' form. */
        if (expr->u.char_lit.orig_text) {
            cg_puts(ctx, expr->u.char_lit.orig_text);
            break;
        }
        /* Sharp mode: emit as integer for simplicity */
        cg_printf(ctx, "%lld", (long long)expr->u.char_lit.val);
        break;
    case AST_STRING_LIT: {
        const char *t = expr->u.string_lit.text;
        size_t len = expr->u.string_lit.len;
        /* parse.c already concatenates adjacent string
         * literals with spaces, e.g. "" "hello" → text = "\"\" \"hello\"".
         * We must emit this verbatim -- NOT strip all quotes and re-add a
         * single pair (which would collapse "" "hello" into " hello").
         * The parser's concatenation preserves the exact token structure
         * that gcc -E would produce. */
        for (size_t i = 0; i < len; i++)
            sb_push_ch(&ctx->out, t[i]);
        break;
    }
    case AST_IDENT:
        cg_puts(ctx, expr->u.ident.name);
        break;

    /* Phase S5: GCC labels-as-values -- emit `&&label` verbatim.  cc
     * (gcc/clang) accept it as the address-of-label extension. */
    case AST_ADDR_OF_LABEL:
        cg_puts(ctx, "&&");
        cg_puts(ctx, expr->u.addr_of_label.label);
        break;

    case AST_BINOP: {
        /* Check for operator overload (struct type).  Only direct
         * struct values participate -- `struct S *p == NULL` is plain
         * pointer comparison and must NOT mangle through
         * `S__op_eq`.  struct_name_of strips one level of pointer
         * for the field-access / method-call paths but here we want
         * to disambiguate value-vs-pointer at the top, mirroring the
         * sema-side guard. */
        Type *lt = sema_type_of(expr->u.binop.lhs);
        /* During specialization, resolve TY_PARAM types to concrete types. */
        lt = cg_resolve_type(ctx, lt);
        Type *lt_unconst = ty_unconst(ctx->ts, lt);
        bool lhs_is_struct_value =
            lt_unconst && lt_unconst->kind == TY_STRUCT;
        /* Get the struct name -- mangle if generic instantiation. */
        char *mangled_sn = NULL;
        const char *sn = lhs_is_struct_value ? lt_unconst->u.struct_.name : NULL;
        const char *scope_sn = sn;  /* original name for scope lookup */
        const char *effective_sn = sn;
        
        if (sn && lt_unconst->u.struct_.nargs > 0) {
            /* Generic instantiation: produce mangled name like Vec__int */
            mangled_sn = cg_mangle_inst(sn, lt_unconst->u.struct_.args,
                                        lt_unconst->u.struct_.nargs);
            effective_sn = mangled_sn;
        }
        /* Only emit a mangled operator call when the operator symbol
         * actually exists in the struct's scope.  Without this check, a
         * plain `value_ = other_value_` on a C union type (which has no
         * operator= defined) would emit `Value__op_eq(...)` causing an
         * undefined-reference link error. */
        if (sn) {
            char opname[64];
            snprintf(opname, sizeof opname, "operator%s",
                     binop_str(expr->u.binop.op));
            /* Look up the operator symbol in the struct's own scope.
             * Use the original (unmangled) name for scope lookup, since
             * mangled names are only used for C code generation. */
            Scope *ss_s = (ctx->file_scope && scope_sn)
                          ? cg_struct_scope(ctx, scope_sn) : NULL;
            Symbol *osym = ss_s ? scope_lookup_local(ss_s, opname) : NULL;
            if (!osym) effective_sn = NULL;   /* operator not defined -- try free-function */
        }
        /* Free-function operator fallback: look up operator+(lhs, rhs) in
         * file scope when the struct has no matching method.
         * Iterate all overloads and pick the one matching LHS/RHS types. */
        const AstNode *free_op_fn = NULL;
        if (!effective_sn && lhs_is_struct_value && ctx->file_scope) {
            char opname2[64];
            snprintf(opname2, sizeof opname2, "operator%s",
                     binop_str(expr->u.binop.op));
            /* Get RHS type for overload disambiguation. */
            Type *rt = sema_type_of(expr->u.binop.rhs);
            rt = cg_resolve_type(ctx, rt);
            Type *rt_unc = ty_unconst(ctx->ts, rt);

            for (Symbol *fsym = scope_lookup_local(ctx->file_scope, opname2);
                 fsym;
                 fsym = scope_lookup_next_local(ctx->file_scope, fsym, opname2)) {
                if (!fsym->decl ||
                    fsym->decl->kind != AST_FUNC_DEF ||
                    !fsym->decl->u.func_def.is_operator ||
                    fsym->decl->u.func_def.params.len < 1)
                    continue;

                /* p0 must match LHS struct type. */
                AstNode *p0 = fsym->decl->u.func_def.params.data[0];
                if (!p0 || !p0->u.param_decl.type) continue;
                Type *p0t = ty_from_ast(ctx->ts, p0->u.param_decl.type,
                                        cg_type_scope(ctx), NULL);
                Type *p0t_unc = ty_unconst(ctx->ts, p0t);
                if (!p0t_unc || p0t_unc->kind != TY_STRUCT) continue;
                if (!lt_unconst || lt_unconst->kind != TY_STRUCT) continue;
                if (strcmp(p0t_unc->u.struct_.name,
                           lt_unconst->u.struct_.name) != 0) continue;

                /* p1 must match RHS type when RHS is also a struct. */
                if (fsym->decl->u.func_def.params.len >= 2 &&
                    rt_unc && rt_unc->kind == TY_STRUCT) {
                    AstNode *p1 = fsym->decl->u.func_def.params.data[1];
                    if (p1 && p1->u.param_decl.type) {
                        Type *p1t = ty_from_ast(ctx->ts,
                            p1->u.param_decl.type, cg_type_scope(ctx), NULL);
                        Type *p1t_unc = ty_unconst(ctx->ts, p1t);
                        if (p1t_unc && p1t_unc->kind == TY_STRUCT &&
                            strcmp(p1t_unc->u.struct_.name,
                                   rt_unc->u.struct_.name) != 0)
                            continue; /* RHS type mismatch */
                    }
                }

                /* Matching overload found. */
                free_op_fn = fsym->decl;
                break;
            }
            /* Extension-method operator fallback: the symbol is registered
             * with a mangled name `Struct__operatorX` in file scope. */
            if (!free_op_fn && scope_sn) {
                char emname[128];
                snprintf(emname, sizeof emname, "%s__operator%s",
                         scope_sn, binop_str(expr->u.binop.op));
                Symbol *emsym = scope_lookup_local(ctx->file_scope, emname);
                if (emsym && emsym->decl &&
                    emsym->decl->kind == AST_FUNC_DEF &&
                    emsym->decl->u.func_def.is_operator) {
                    free_op_fn = emsym->decl;
                    effective_sn = effective_sn ? effective_sn : mangled_sn ? mangled_sn : sn;
                }
            }
        }
        if (effective_sn) {
            /* Build "operator<sym>" then mangle */
            char opname[64];
            snprintf(opname, sizeof opname, "operator%s",
                     binop_str(expr->u.binop.op));
            cg_method_name(ctx, effective_sn, opname);
            cg_puts(ctx, "(");
            /* lhs as first arg: take address if value type.
             * If lhs is an rvalue (e.g. a function call), materialise a
             * temporary via GNU statement expression as done for method
             * chaining.  C forbids taking the address of an rvalue. */
            cg_emit_struct_receiver_arg(ctx, lt_unconst, expr->u.binop.lhs);
            cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.binop.rhs);
            cg_puts(ctx, ")");
        } else if (free_op_fn) {
            /* Free-function operator: mangle name from param types, emit
             * (lhs, rhs) -- no address-taking, both args are values. */
            const char *op_sym = free_op_fn->u.func_def.name + 8; /* after "operator" */
            cg_emit_mangled_free_op_name(ctx, op_sym, free_op_fn);
            cg_puts(ctx, "(");
            cg_expr(ctx, expr->u.binop.lhs);
            cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.binop.rhs);
            cg_puts(ctx, ")");
        } else {
            /* Emit without wrapping parens; cg_expr_p handles precedence. */
            int prec = binop_prec(expr->u.binop.op);
            bool right_assoc = (prec == 2); /* assignment operators */
            if (right_assoc) {
                cg_expr_p(ctx, expr->u.binop.lhs, prec + 1);
                cg_printf(ctx, " %s ", binop_str(expr->u.binop.op));
                cg_expr_p(ctx, expr->u.binop.rhs, prec);
            } else {
                cg_expr_p(ctx, expr->u.binop.lhs, prec);
                cg_printf(ctx, " %s ", binop_str(expr->u.binop.op));
                cg_expr_p(ctx, expr->u.binop.rhs, prec + 1);
            }
        }
        free(mangled_sn);
        break;
    }

    case AST_UNARY: {
        static const char *uops[] = {
            [STOK_MINUS]="-", [STOK_BANG]="!", [STOK_TILDE]="~",
            [STOK_PLUS]="+",  [STOK_AMP]="&",  [STOK_STAR]="*",
            [STOK_PLUSPLUS]="++", [STOK_MINUSMINUS]="--"
        };
        SharpTokKind op = expr->u.unary.op;
        /* Check for unary operator overload on struct type.
         * Lookup: (1) struct method (2) free function operator-(T). */
        if (!expr->u.unary.postfix &&
            (op == STOK_MINUS || op == STOK_BANG || op == STOK_TILDE)) {
            Type *ut = sema_type_of(expr->u.unary.operand);
            Type *ut_unc = ty_unconst(ctx->ts, ut);
            if (ut_unc && ut_unc->kind == TY_STRUCT) {
                const char *sn = ut_unc->u.struct_.name;
                const char *op_sym = (op == STOK_MINUS) ? "operator-"
                                   : (op == STOK_BANG)  ? "operator!"
                                                        : "operator~";
                const char *eff_sn = sn;
                /* For generic struct instances, mangle the struct name. */
                char *mangled_eff = NULL;
                if (ut_unc->u.struct_.nargs > 0) {
                    mangled_eff = cg_mangle_inst(eff_sn, ut_unc->u.struct_.args,
                                                 ut_unc->u.struct_.nargs);
                    eff_sn = mangled_eff;
                }
                /* (1) struct method */
                bool found_method = false;
                if (ctx->file_scope) {
                    Scope *ss_s = ctx->file_scope
                                  ? cg_struct_scope(ctx, sn) : NULL;
                    Symbol *osym = ss_s ? scope_lookup_local(ss_s, op_sym) : NULL;
                    if (osym) {
                        cg_method_name(ctx, eff_sn, op_sym);
                        cg_puts(ctx, "(");
                        cg_emit_struct_receiver_arg(ctx, ut, expr->u.unary.operand);
                        cg_puts(ctx, ")");
                        found_method = true;
                    }
                }
                free(mangled_eff);
                if (!found_method && ctx->file_scope) {
                    /* (2a) Extension-method operator via mangled name */
                    {
                        char emname[128];
                        snprintf(emname, sizeof emname, "%s__%s", sn, op_sym);
                        Symbol *emsym = scope_lookup_local(ctx->file_scope, emname);
                        if (emsym && emsym->decl &&
                            emsym->decl->kind == AST_FUNC_DEF &&
                            emsym->decl->u.func_def.is_operator) {
                            cg_method_name(ctx, eff_sn, op_sym);
                            cg_puts(ctx, "(");
                            cg_emit_struct_receiver_arg(ctx, ut, expr->u.unary.operand);
                            cg_puts(ctx, ")");
                            found_method = true;
                        }
                    }
                }
                if (!found_method && ctx->file_scope) {
                    /* (2b) free function */
                    Symbol *fsym = scope_lookup(ctx->file_scope, op_sym);
                    if (fsym && fsym->decl &&
                        fsym->decl->kind == AST_FUNC_DEF &&
                        fsym->decl->u.func_def.is_operator) {
                        const char *sym_sfx = fsym->decl->u.func_def.name + 8;
                        cg_emit_mangled_free_op_name(ctx, sym_sfx, fsym->decl);
                        cg_puts(ctx, "(");
                        cg_expr(ctx, expr->u.unary.operand);
                        cg_puts(ctx, ")");
                        found_method = true;
                    }
                }
                if (found_method) break;
            }
        }
        const char *s = (op < STOK_COUNT && uops[op]) ? uops[op] : "/*?*/";
        if (expr->u.unary.postfix) {
            cg_expr_p(ctx, expr->u.unary.operand, 14);
            cg_puts(ctx, s);
        } else {
            cg_puts(ctx, s);
            /* Insert space when operand starts with a character that would
             * otherwise merge with the unary operator into a different token:
             *   --0x6200  (should be -(-0x6200))
             *   ++x       (should be +(+x))
             *   !!flag    (should be !(!flag))
             *   ~^mask    (should be ~(~mask))
             * Without this, `--` becomes the decrement operator and `++` the
             * increment operator, both requiring lvalues. */
            if (op == STOK_MINUS || op == STOK_PLUS ||
                op == STOK_BANG || op == STOK_TILDE) {
                const AstNode *inner = expr->u.unary.operand;
                while (inner && inner->kind == AST_PAREN)
                    inner = inner->u.paren.inner;
                bool needs_space = false;
                if (inner && inner->kind == AST_UNARY && !inner->u.unary.postfix) {
                    SharpTokKind iop = inner->u.unary.op;
                    if (iop == STOK_MINUS || iop == STOK_PLUS ||
                        iop == STOK_BANG || iop == STOK_TILDE)
                        needs_space = true;
                }
                if (needs_space)
                    cg_puts(ctx, " ");
            }
            cg_expr_p(ctx, expr->u.unary.operand, 14);
        }
        break;
    }

    case AST_TERNARY:
        cg_expr_p(ctx, expr->u.ternary.cond, 4);
        cg_puts(ctx, " ? ");
        cg_expr(ctx, expr->u.ternary.then_);
        cg_puts(ctx, " : ");
        cg_expr(ctx, expr->u.ternary.else_);
        break;

    case AST_COMMA:
        cg_expr(ctx, expr->u.comma.lhs);
        cg_puts(ctx, ", ");
        cg_expr(ctx, expr->u.comma.rhs);
        break;


    case AST_CALL: {
        /* For generic function calls, emit the mangled specialized name. */
        const AstNode *callee = expr->u.call.callee;
        bool emitted_name = false;

        if (!emitted_name && callee && callee->kind == AST_IDENT) {
            const char *fname = callee->u.ident.name;
            /* __builtin_types_compatible_p(T1, T2) takes type
             * arguments that our parser substitutes as 0 via __typeof__.
             * Emit 0 (not compatible) so IS_ARRAY_NOT_POINTER macro works:
             *   IS_ARRAY_NOT_POINTER(arr) = !__builtin_types_compatible_p(0,0)
             *   = !0 = 1 (is array) → STATIC_ASSERT_EXPR(1) = valid bitfield. */
            if (strcmp(fname, "__builtin_types_compatible_p") == 0) {
                cg_puts(ctx, "0");
                break;
            }
            Symbol *sym = ctx->file_scope
                ? scope_lookup(ctx->file_scope, fname) : NULL;
            if (sym && sym->kind == SYM_FUNC && sym->decl &&
                sym->decl->kind == AST_FUNC_DEF &&
                sym->decl->u.func_def.generic_params.len > 0) {
                /* Generic function -- emit mangled name. */
                const char *mn = cg_gfunc_mangle_for_call(ctx, fname, expr);
                cg_puts(ctx, mn);
                emitted_name = true;
            }
        }
        if (!emitted_name) {
            /* C4: when callee is a cast expression in C mode, wrap it in
             * parens: ((type)fp)(args) -- without parens (type)fp(args) is
             * parsed as (type)(fp(args)) by C's higher-prec call operator. */
            bool callee_is_cast = (callee && callee->kind == AST_CAST);
            if (callee_is_cast) cg_puts(ctx, "(");
            cg_expr(ctx, expr->u.call.callee);
            if (callee_is_cast) cg_puts(ctx, ")");
        }
        cg_puts(ctx, "(");
        for (size_t i = 0; i < expr->u.call.args.len; i++) {
            if (i) cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.call.args.data[i]);
        }
        cg_puts(ctx, ")");
        break;
    }

    /* ── Phase G: AST_GENERIC_CALL ─────────────────────────────────────
     * func<int>(a, b)  -- explicit type-argument generic function call.
     *
     * Emit the mangled name (func__int) then the value arguments.
     * The mangled name was already recorded by cg_collect_gfunc_call
     * during the collect pass; here we just look it up via the same
     * cg_gfunc_mangle_for_call helper. */
    case AST_GENERIC_CALL: {
        const char *mn = cg_gfunc_mangle_for_call(ctx,
                            expr->u.generic_call.name, expr);
        cg_puts(ctx, mn);
        cg_puts(ctx, "(");
        for (size_t i = 0; i < expr->u.generic_call.call_args.len; i++) {
            if (i) cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.generic_call.call_args.data[i]);
        }
        cg_puts(ctx, ")");
        break;
    }

    case AST_METHOD_CALL: {
        Type *recv_t = sema_type_of(expr->u.method_call.recv);
        /* v0.13: during specialization, resolve TY_PARAM to the concrete
         * instantiation type so method dispatch goes through the mangled
         * receiver name (Point__hash) rather than falling through to a
         * verbatim `val.hash()` emission. */
        recv_t = cg_resolve_type(ctx, recv_t);
        const char *sn = struct_name_of(recv_t);
        bool arrow = expr->u.method_call.arrow;

        /* ── Associated function call: Type.method(args) ────────────────
         * Detected when receiver is a type name (AST_IDENT → SYM_TYPE)
         * or a generic type expression (AST_CAST with NULL operand).
         * Emit as: StructName__method(args)  -- no `this` argument. */
        {
            AstNode *recv_node = expr->u.method_call.recv;
            const char *assoc_sname = NULL;
            /* Case 1: concrete struct -- Counter.make() */
            if (recv_node && recv_node->kind == AST_IDENT) {
                const char *rname = recv_node->u.ident.name;
                Symbol *rsym = scope_lookup(ctx->file_scope, rname);
                if (rsym && rsym->kind == SYM_TYPE)
                    assoc_sname = rname;
            }
            /* Case 2: generic struct -- Vec<int>.new() */
            if (!assoc_sname && recv_node && recv_node->kind == AST_CAST &&
                recv_node->u.cast.operand == NULL && recv_node->u.cast.type &&
                recv_node->u.cast.type->kind == AST_TYPE_GENERIC) {
                /* Build mangled name Vec__int from the type args */
                const char *gname = recv_node->u.cast.type->u.type_generic.name;
                size_t nargs = recv_node->u.cast.type->u.type_generic.args.len;
                Type **targs = nargs ? malloc(nargs * sizeof *targs) : NULL;
                if (nargs && !targs) abort();
                for (size_t _i = 0; _i < nargs; _i++) {
                    AstNode *arg = recv_node->u.cast.type->u.type_generic.args.data[_i];
                    targs[_i] = arg ? ty_from_ast(ctx->ts, arg, ctx->file_scope, NULL) : NULL;
                }
                char *mn = cg_mangle_inst(gname, targs, nargs);
                free(targs);
                /* Emit: Vec__int__new(args) */
                cg_method_name(ctx, mn, expr->u.method_call.method);
                cg_puts(ctx, "(");
                for (size_t i = 0; i < expr->u.method_call.args.len; i++) {
                    if (i) cg_puts(ctx, ", ");
                    cg_expr(ctx, expr->u.method_call.args.data[i]);
                }
                cg_puts(ctx, ")");
                free(mn);
                break;
            }
            if (assoc_sname) {
                cg_method_name(ctx, assoc_sname, expr->u.method_call.method);
                cg_puts(ctx, "(");
                for (size_t i = 0; i < expr->u.method_call.args.len; i++) {
                    if (i) cg_puts(ctx, ", ");
                    cg_expr(ctx, expr->u.method_call.args.data[i]);
                }
                cg_puts(ctx, ")");
                break;
            }
        }

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
            const char *effective_sn = sn;
            /* For generic struct instances, mangle the struct name. */
            Type *base_recv = ty_peel_to_struct(recv_t);
            char *mangled_sn = (base_recv && base_recv->u.struct_.nargs > 0)
                ? cg_mangle_inst(effective_sn, base_recv->u.struct_.args, base_recv->u.struct_.nargs)
                : NULL;
            cg_method_name(ctx, mangled_sn ? mangled_sn : effective_sn,
                           expr->u.method_call.method);
            cg_puts(ctx, "(");
            /* this pointer: pass receiver address or value.
             * If the receiver is itself a struct-returning call (rvalue),
             * C forbids taking its address directly.  Materialise it into
             * a named temporary and pass &tmp instead. */
            if (!arrow && recv_t && recv_t->kind == TY_STRUCT) {
                if (cg_is_rvalue_struct(expr->u.method_call.recv)) {
                    cg_emit_rvalue_struct_addr(ctx, recv_t, expr->u.method_call.recv);
                } else {
                    cg_puts(ctx, "&");
                    cg_expr(ctx, expr->u.method_call.recv);
                }
            } else {
                cg_expr(ctx, expr->u.method_call.recv);
            }
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

    case AST_INDEX: {
        /* Check for operator[] overload on struct type. */
        Type *idx_base_t = sema_type_of(expr->u.index_.base);
        /* Resolve TY_PARAM through specialization context. */
        idx_base_t = cg_resolve_type(ctx, idx_base_t);
        Type *idx_base_unc = ty_unconst(ctx->ts, idx_base_t);
        const char *idx_sn_raw = (idx_base_unc && idx_base_unc->kind == TY_STRUCT)
                             ? idx_base_unc->u.struct_.name : NULL;
        char *idx_mangled = NULL;
        const char *idx_sn = idx_sn_raw;
        if (idx_sn_raw && idx_base_unc->u.struct_.nargs > 0) {
            idx_mangled = cg_mangle_inst(idx_sn_raw,
                          idx_base_unc->u.struct_.args,
                          idx_base_unc->u.struct_.nargs);
            idx_sn = idx_mangled;
        }
        if (idx_sn_raw && ctx->file_scope) {
            /* Look up operator[] in the struct's scope using ORIGINAL name. */
            Scope *ss_s = (idx_sn_raw && ctx->file_scope)
                          ? cg_struct_scope(ctx, idx_sn_raw) : NULL;
            Symbol *osym = ss_s ? scope_lookup_local(ss_s, "operator[]") : NULL;
            if (!osym) idx_sn = NULL;   /* no operator[] in struct scope */
        } else {
            idx_sn = NULL;
        }
        /* Extension method fallback: search file scope for
         * `RetType StructName.operator[](this, ...) { }`.
         * For generic structs like Vec__int, the struct name in the
         * expression is the mangled form but the extension method uses
         * the original unmangled name — check prefix match. */
        if (!idx_sn && idx_sn_raw && ctx->file_scope) {
            size_t snr_len = strlen(idx_sn_raw);
            for (size_t b = 0; b < ctx->file_scope->nbuckets; b++) {
                for (Symbol *es = ctx->file_scope->buckets[b];
                     es; es = es->next) {
                if (es->kind != SYM_FUNC || !es->decl) continue;
                if (es->decl->kind != AST_FUNC_DEF) continue;
                if (!es->decl->u.func_def.is_operator) continue;
                if (!es->decl->u.func_def.struct_name) continue;
                if (strcmp(es->decl->u.func_def.name, "operator[]") != 0) continue;
                const char *sn = es->decl->u.func_def.struct_name;
                size_t snl = strlen(sn);
                /* Exact match OR generic prefix: "Vec" matches "Vec__int" */
                if (strcmp(sn, idx_sn_raw) == 0 ||
                    (snl < snr_len && strncmp(sn, idx_sn_raw, snl) == 0 &&
                     idx_sn_raw[snl] == '_' && idx_sn_raw[snl+1] == '_'))
                {
                    idx_sn = idx_mangled ? idx_mangled : idx_sn_raw;
                    break;
                }
                }
            }
        }
        if (idx_sn) {
            cg_method_name(ctx, idx_sn, "operator[]");
            cg_puts(ctx, "(");
            /* receiver: take address of value type */
            if (idx_base_unc && idx_base_unc->kind == TY_STRUCT)
                cg_puts(ctx, "&");
            cg_expr(ctx, expr->u.index_.base);
            cg_puts(ctx, ", ");
            cg_expr(ctx, expr->u.index_.index);
            cg_puts(ctx, ")");
        } else {
            cg_expr(ctx, expr->u.index_.base);
            cg_puts(ctx, "[");
            cg_expr(ctx, expr->u.index_.index);
            cg_puts(ctx, "]");
        }
        break;
    }

    case AST_PAREN:   /* C8: preserve user-written parens */
        cg_puts(ctx, "(");
        cg_expr(ctx, expr->u.paren.inner);
        cg_puts(ctx, ")");
        break;

    case AST_CAST: {
        /* In C mode: emit "(type)operand" with cast prec=14 for operand.
         * In Sharp mode: emit "((type)operand)" -- extra parens avoid
         * ambiguity in lvalue and complex expression contexts. */
        Type *cast_t = (Type*)sema_type_of(expr);
        cg_puts(ctx, "(");
        /* C8/C5: in C mode, prefer cg_type_from_ast when we have the AST
         * type -- it preserves volatile qualifiers AND typedef aliases.
         * Exceptions: pointer-to-array and abstract fn-ptr casts still need
         * the special handling in the Type* path below. */
        if (expr->u.cast.type) {
            const AstNode *ct = expr->u.cast.type;
            /* Check for the special array-ptr and fn-ptr cases that need
             * cg_type (complex nested declarators). Fall back to Type* path
             * for those; use AST path for simple typedef/volatile cases. */
            bool use_ast = true;
            if (cast_t) {
                Type *pb = ty_unconst(ctx->ts, cast_t);
                /* For PTR(FUNC(ret=PTR(FUNC))) doubly-nested function pointer
                 * casts, use ty_from_ast (not sema's cast_t which may be wrong)
                 * so cg_type gets the correct nested structure. */
            if (ty_is_func_ptr(pb)) {
                const Type *ret = pb->u.ptr.base->u.func.ret;
                ret = ty_peel_ptr((Type *)ret).base;
                if (ret && ret->kind == TY_FUNC) {
                    /* Use ty_from_ast for accuracy; fall to else branch below */
                    Type *ast_t2 = ty_from_ast(ctx->ts, expr->u.cast.type,
                                               cg_type_scope(ctx), NULL);
                    if (ast_t2 && !ty_is_error(ast_t2)) {
                        cg_type(ctx, ast_t2);
                        goto cast_type_done;
                    }
                    use_ast = false;
                }
            }
            /* Disable ast path for PTR(ARRAY) UNLESS the AST cast type
                 * is PTR(TYPE_NAME(typedef_alias)) -- in that case preserve. */
                if (ty_is_ptr_to_array(pb)) {
                    /* Check if AST is PTR(TYPE_NAME(typedef)) -- keep use_ast=true */
                    const AstNode *cast_ast = expr->u.cast.type;
                    bool is_typedef_ptr = cast_ast &&
                        cast_ast->kind == AST_TYPE_PTR &&
                        cast_ast->u.type_ptr.base &&
                        cast_ast->u.type_ptr.base->kind == AST_TYPE_NAME &&
                        !cast_ast->u.type_ptr.base->u.type_name.is_struct_tag;
                    if (is_typedef_ptr && ctx->file_scope) {
                        const char *bnm = cast_ast->u.type_ptr.base->u.type_name.name;
                        Symbol *sym2 = scope_find_typedef(ctx->file_scope, bnm);
                        if (!sym2) use_ast = false;
                    } else {
                        use_ast = false;
                    }
                }
            }
            if (use_ast) {
                cg_type_from_ast(ctx, ct);
                goto cast_type_done;
            }
        }
        if (cast_t && !ty_is_error(cast_t) && cast_t->kind != TY_VOID &&
            cast_t->kind != TY_ERROR) {
            if (ty_is_ptr_to_array(cast_t)) {
                Type *arr = cast_t->u.ptr.base;
                cg_type(ctx, arr->u.array.base);
                cg_puts(ctx, " (*)[");
                if (arr->u.array.size >= 0) {
                    char _cb[32];
                    snprintf(_cb, sizeof _cb, "%lld", (long long)arr->u.array.size);
                    cg_puts(ctx, _cb);
                }
                cg_puts(ctx, "]");
            } else {
                cg_type(ctx, cast_t);
            }
        } else if (expr->u.cast.type) {
            Type *ast_t = ty_from_ast(ctx->ts, expr->u.cast.type,
                                      cg_type_scope(ctx), NULL);
            if (ast_t && !ty_is_error(ast_t)) {
                if (ty_is_ptr_to_array(ast_t)) {
                    Type *arr = ast_t->u.ptr.base;
                    cg_type(ctx, arr->u.array.base);
                    cg_puts(ctx, " (*)[");
                    if (arr->u.array.size >= 0) {
                        char _cb[32];
                        snprintf(_cb, sizeof _cb, "%lld", (long long)arr->u.array.size);
                        cg_puts(ctx, _cb);
                    }
                    cg_puts(ctx, "]");
                } else {
                    cg_type(ctx, ast_t);
                }
            } else {
                cg_puts(ctx, "int");
            }
        } else {
            cg_puts(ctx, "int");
        }
        cast_type_done:
        cg_puts(ctx, ")");
        /* cast prec=14; operand gets parens if lower prec */
        cg_expr_p(ctx, expr->u.cast.operand, 14);
        break;
    }
    case AST_SIZEOF:
        /* `_Alignof`/`__alignof` nodes share the sizeof AST node
         * but emit `_Alignof(T)` instead of `sizeof(T)`. */
        /* C8: in C mode, emit `sizeof expr` without parens when the source
         * had no parens (sizeof x); use parens only when source had them
         * (sizeof(x)) or when this is a type operand or _Alignof. */
        bool sz_needs_parens = expr->u.sizeof_.is_type ||
                               expr->u.sizeof_.is_alignof ||
                               expr->u.sizeof_.expr_has_parens;
        if (sz_needs_parens) {
            if (expr->u.sizeof_.is_alignof) {
                const char *sp = expr->u.sizeof_.alignof_spelling;
                cg_puts(ctx, sp ? sp : "_Alignof");
                cg_puts(ctx, "(");
            } else {
                cg_puts(ctx, "sizeof(");
            }
        } else {
            cg_puts(ctx, "sizeof ");
        }
        if (expr->u.sizeof_.is_type) {
            /* Prefer the sema-annotated type; fall back to ty_from_ast
             * for types (like struct typedefs) whose sizeof operand node
             * was not annotated by sema -- without this, sizeof(LG) where
             * LG is a struct typedef would emit sizeof(int).
             *
             * if both sema and ty_from_ast fail (returns
             * ty_error), the type name is probably a LOCAL typedef that is
             * only visible in the function-body scope, not the file scope
             * that cg_type_scope() returns (e.g. `typedef struct {...} t_a`
             * defined inside a function body).  In that case emit the type
             * name verbatim from the AST -- the local C scope will resolve
             * it correctly when the generated file is compiled.
             *
             * array-type operands (from `sizeof(char[N])`) go
             * through cg_decl-style emission: `char[N]` must be written as
             * `char[N]` not as `char *` or `char`.  Use the AST directly
             * when the operand is an array type node. */
            AstNode *op = expr->u.sizeof_.operand;
            if (op && op->kind == AST_TYPE_ARRAY) {
                /* Emit array type verbatim from AST: base[size] */
                /* Find innermost base type */
                const AstNode *cursor = op;
                while (cursor && cursor->kind == AST_TYPE_ARRAY)
                    cursor = cursor->u.type_array.base;
                if (cursor) {
                    Type *bt = ty_from_ast(ctx->ts, (AstNode*)cursor,
                                          cg_type_scope(ctx), NULL);
                    cg_type(ctx, bt ? bt : ty_int(ctx->ts));
                }
                /* Emit array suffixes */
                cursor = op;
                cg_emit_array_suffixes(ctx, cursor);
            } else {
                /* C8/C5: in C mode with an AST operand, prefer cg_type_from_ast
                 * which preserves typedef names (e.g. S → S, not struct __anon_struct_0). */
                if (op) {
                    cg_type_from_ast(ctx, op);
                } else {
                    Type *t = sema_type_of(op);
                    if (!t || ty_is_error(t))
                        t = ty_from_ast(ctx->ts, op, cg_type_scope(ctx), NULL);
                    if (t && !ty_is_error(t) && t->kind == TY_ARRAY &&
                        op && op->kind == AST_TYPE_NAME && op->u.type_name.name) {
                        cg_puts(ctx, op->u.type_name.name);
                    } else if (t && !ty_is_error(t)) {
                        cg_type(ctx, t);
                    } else if (op && op->kind == AST_TYPE_NAME &&
                               op->u.type_name.name) {
                        cg_puts(ctx, op->u.type_name.name);
                    } else {
                        cg_puts(ctx, "int");
                    }
                }
            }
        } else {
            /* Generic param in sizeof(T) may parse as AST_IDENT when T is not
             * registered as a type in expr context. Check gp_names first. */
            const AstNode *sop = expr->u.sizeof_.operand;
            if (sop && sop->kind == AST_IDENT && ctx->ngp > 0) {
                const char *sname = sop->u.ident.name;
                bool emitted = false;
                for (size_t _gi = 0; _gi < ctx->ngp && !emitted; _gi++) {
                    if (ctx->gp_names[_gi] && strcmp(sname, ctx->gp_names[_gi]) == 0) {
                        cg_type(ctx, ctx->gp_vals[_gi]);
                        emitted = true;
                    }
                }
                if (!emitted) cg_expr(ctx, sop);
            } else {
                cg_expr(ctx, expr->u.sizeof_.operand);
            }
        }
        if (sz_needs_parens) cg_puts(ctx, ")");
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
        if (expr->u.init_list.trailing_comma)
            cg_puts(ctx, ",");
        cg_puts(ctx, " }");
        break;
    }

    case AST_DESIGNATED_INIT: {
        if (expr->u.designated_init.designator_kind == 0) {
            cg_printf(ctx, ".%s = ", expr->u.designated_init.field_name);
        } else {
            cg_puts(ctx, "[");
            cg_expr(ctx, expr->u.designated_init.index_expr);
            if (expr->u.designated_init.index_end) {
                cg_puts(ctx, " ... ");
                cg_expr(ctx, expr->u.designated_init.index_end);
            }
            cg_puts(ctx, "] = ");
        }
        cg_expr(ctx, expr->u.designated_init.value);
        break;
    }

    case AST_COMPOUND_LIT: {
        /* `(Type){ ... }` -- the type-name is rendered through the AST
         * (we go via ty_from_ast then cg_type so const/struct/etc work
         * uniformly).  The init list renders itself with surrounding
         * braces. */
        cg_puts(ctx, "(");
        /* Use cg_type_from_ast in C mode so array types
         * like (char[32]){0} are emitted faithfully, not decayed to pointers. */
        if (expr->u.compound_lit.type) {
            /* For array types, emit base type then [N] suffix(es). */
            const AstNode *ty_ast = expr->u.compound_lit.type;
            cg_type_from_ast(ctx, ty_ast);
            /* Emit array suffixes outermost to innermost (they're stored innermost-first). */
            {
                /* Collect array dimensions */
                const AstNode *a = ty_ast;
                const AstNode *dims[16]; int ndims = 0;
                while (a && a->kind == AST_TYPE_ARRAY && ndims < 16) {
                    dims[ndims++] = a;
                    a = a->u.type_array.base;
                }
                /* Emit in reverse (outermost first) */
                for (int _di = ndims - 1; _di >= 0; _di--) {
                    cg_puts(ctx, "[");
                    if (dims[_di]->u.type_array.size)
                        cg_const_expr(ctx, dims[_di]->u.type_array.size);
                    cg_puts(ctx, "]");
                }
            }
        } else {
            Type *t = ty_from_ast(ctx->ts, expr->u.compound_lit.type,
                                  cg_type_scope(ctx), NULL);
            if (t && !ty_is_error(t)) cg_type(ctx, t);
            else cg_puts(ctx, "/* ?compound_lit_type */");
        }
        cg_puts(ctx, ")");
        cg_expr(ctx, expr->u.compound_lit.init);
        break;
    }

    case AST_AT_INTRINSIC: {
        const char *iname = expr->u.at_intrinsic.name;
        if (strcmp(iname, "to_cstr") == 0) {
            /* @to_cstr(expr) → __SP_MAGIC_TO_C_STR(expr)
             * The macro uses GCC VLA statement-expression to create a
             * NUL-terminated C string from an sp_str_t slice on the stack. */
            ctx->uses_to_cstr = true;
            cg_puts(ctx, "__SP_MAGIC_TO_C_STR(");
            if (expr->u.at_intrinsic.args.len > 0)
                cg_expr(ctx, expr->u.at_intrinsic.args.data[0]);
            cg_puts(ctx, ")");
        } else if (strcmp(iname, "type_name") == 0) {
            /* v0.13: emit the mangled name as a C string literal.
             * Inside a generic specialization, re-evaluate so we get the
             * concrete instantiation type name (e.g. "Point" rather than
             * the template-time "T"). */
            if (ctx->ngp > 0) {
                Scope *eval_scope = ctx->spec_scope ? ctx->spec_scope : ctx->file_scope;
                sema_eval_intrinsic_specialized(
                    ctx->ts, eval_scope, NULL, (AstNode*)expr,
                    ctx->gp_names, ctx->gp_vals, ctx->ngp);
            }
            const char *s = expr->u.at_intrinsic.resolved_str;
            if (!s) s = "<unresolved>";
            cg_puts(ctx, "\"");
            for (const char *p = s; *p; p++) {
                if (*p == '"' || *p == '\\') cg_printf(ctx, "\\%c", *p);
                else                          cg_printf(ctx, "%c", *p);
            }
            cg_puts(ctx, "\"");
        } else if (strcmp(iname, "has_operator")  == 0 ||
                   strcmp(iname, "has_method")    == 0 ||
                   strcmp(iname, "has_field")     == 0 ||
                   strcmp(iname, "is_arithmetic") == 0 ||
                   strcmp(iname, "is_pointer")    == 0) {
            /* v0.13: emit precomputed 0/1.  Inside a generic
             * specialization, re-evaluate so the result reflects the
             * concrete instantiation, not the TY_PARAM placeholder. */
            int v;
            if (ctx->ngp > 0) {
                Scope *eval_scope = ctx->spec_scope ? ctx->spec_scope : ctx->file_scope;
                int rv = sema_eval_intrinsic_specialized(
                    ctx->ts, eval_scope, NULL, (AstNode*)expr,
                    ctx->gp_names, ctx->gp_vals, ctx->ngp);
                v = (rv == 0 || rv == 1) ? rv : 0;
            } else {
                v = expr->u.at_intrinsic.resolved_set
                    ? expr->u.at_intrinsic.resolved_val : 0;
            }
            cg_printf(ctx, "/* @%s */ %d", iname, v);
        } else if (strcmp(iname, "static_assert") == 0) {
            /* v0.13: inside a generic specialization, re-evaluate the
             * condition with the substitution context.  If it resolves
             * to false, emit a block-scope `_Static_assert(0, msg)` so
             * the failure is reported at C compile time with the user's
             * message -- this is how a HashMap<NoHash, V> instantiation
             * is rejected when K lacks `hash()`. */
            int rv = 1;
            if (ctx->ngp > 0 && expr->u.at_intrinsic.args.len >= 1) {
                AstNode *cond = expr->u.at_intrinsic.args.data[0];
                if (cond && cond->kind == AST_AT_INTRINSIC) {
                    Scope *es = ctx->spec_scope ? ctx->spec_scope : ctx->file_scope;
                    int crv = sema_eval_intrinsic_specialized(
                        ctx->ts, es, NULL, cond,
                        ctx->gp_names, ctx->gp_vals, ctx->ngp);
                    if (crv == 0) rv = 0;
                }
            }
            if (rv == 0) {
                cg_puts(ctx, "_Static_assert(0, ");
                if (expr->u.at_intrinsic.args.len >= 2) {
                    AstNode *msga = expr->u.at_intrinsic.args.data[1];
                    if (msga && msga->kind == AST_STRING_LIT) {
                        cg_puts(ctx, msga->u.string_lit.text);
                    } else {
                        cg_puts(ctx, "\"@static_assert failed\"");
                    }
                } else {
                    cg_puts(ctx, "\"@static_assert failed\"");
                }
                cg_puts(ctx, ")");
            } else {
                cg_printf(ctx, "/* @%s */ 1", iname);
            }
        } else {
            /* Unknown -- keep old fallback. */
            cg_printf(ctx, "/* @%s */ 1", iname);
        }
        break;
    }

    case AST_STMT_EXPR:
        /* GCC statement-expression `({ stmts; expr; })`.
         * We emit the GNU extension verbatim: `({` then the block's
         * statements (using cg_block's indented emission), then `})`.
         * GCC and clang both accept this syntax; the C compiler validates
         * the semantics.  The surrounding parentheses are required by the
         * GNU extension grammar. */
        cg_puts(ctx, "({");
        if (expr->u.stmt_expr.block) {
            const AstNode *block = expr->u.stmt_expr.block;
            for (size_t i = 0; i < block->u.block.stmts.len; i++) {
                const AstNode *s = block->u.block.stmts.data[i];
                /* A8/p71: group comma-continuation var decls inline */
                if (s && s->kind == AST_DECL_STMT &&
                    s->u.decl_stmt.decl &&
                    s->u.decl_stmt.decl->kind == AST_VAR_DECL &&
                    !s->u.decl_stmt.decl->u.var_decl.is_comma_cont) {
                    size_t j = i + 1;
                    while (j < block->u.block.stmts.len) {
                        const AstNode *ns = block->u.block.stmts.data[j];
                        if (!ns || ns->kind != AST_DECL_STMT ||
                            !ns->u.decl_stmt.decl ||
                            ns->u.decl_stmt.decl->kind != AST_VAR_DECL ||
                            !ns->u.decl_stmt.decl->u.var_decl.is_comma_cont) break;
                        j++;
                    }
                    if (j > i + 1) {
                        /* Emit comma group: `type first = init, b = init, ...;` */
                        cg_puts(ctx, " ");
                        const AstNode *vd0 = s->u.decl_stmt.decl;
                        {
                            Type *t0 = vd0->sem_type;
                            if (!t0 || ty_is_error(t0))
                                t0 = vd0->u.var_decl.type
                                   ? ty_from_ast(ctx->ts, vd0->u.var_decl.type,
                                                 cg_type_scope(ctx), NULL)
                                   : ty_int(ctx->ts);
                            bool emitted = vd0->u.var_decl.type &&
                                cg_decl_ast(ctx, vd0->u.var_decl.type, vd0->u.var_decl.name);
                            if (!emitted) cg_decl(ctx, t0, vd0->u.var_decl.name);
                        }
                        if (vd0->u.var_decl.init) {
                            cg_puts(ctx, " = ");
                            cg_expr(ctx, vd0->u.var_decl.init);
                        }
                        for (size_t k = i + 1; k < j; k++) {
                            const AstNode *vdk =
                                block->u.block.stmts.data[k]->u.decl_stmt.decl;
                            cg_puts(ctx, ", ");
                            cg_emit_comma_cont_declarator(ctx,
                                vdk->u.var_decl.type, vdk->u.var_decl.name);
                            if (vdk->u.var_decl.init) {
                                cg_puts(ctx, " = ");
                                cg_expr(ctx, vdk->u.var_decl.init);
                            }
                        }
                        cg_puts(ctx, ";");
                        i = j - 1;
                        continue;
                    }
                }
                cg_puts(ctx, " ");
                cg_stmt(ctx, s, NULL, 0);
            }
        }
        cg_puts(ctx, " })");
        break;

    case AST_GENERIC_EXPR:
        cg_puts(ctx, "_Generic(");
        cg_expr(ctx, expr->u.generic_expr.controlling);
        for (size_t i = 0; i < expr->u.generic_expr.associations.len; i++) {
            cg_puts(ctx, ", ");
            const AstNode *a = expr->u.generic_expr.associations.data[i];
            if (a->u.generic_assoc.is_default) {
                cg_puts(ctx, "default");
            } else {
                cg_type_from_ast(ctx, a->u.generic_assoc.type_name);
            }
            cg_puts(ctx, ": ");
            cg_expr(ctx, a->u.generic_assoc.value);
        }
        cg_puts(ctx, ")");
        break;

    case AST_GENERIC_ASSOC:
        cg_puts(ctx, "/* generic-assoc */");
        break;

    default:
        cg_printf(ctx, "#error \"unhandled AST kind %d (%s) in cg_expr\"\n",
                  expr->kind, ast_kind_name(expr->kind));
        break;
    }
}

/* =========================================================================
 * Statement codegen
 * ====================================================================== */
static void cg_stmt(CgCtx *ctx, const AstNode *stmt,
                    const AstNode **defers, size_t ndefers) {
    if (!stmt) return;
    cg_emit_linemarker(ctx, stmt->loc);
    switch (stmt->kind) {

    case AST_GCC_VERBATIM:
        /* C7: GCC extension preserved verbatim.
         * In C mode: emit the text as a statement.
         * In Sharp mode: silently ignore (Sharp doesn't use these). */
        if (stmt->u.gcc_verbatim.text) {
            cg_indent(ctx);
            cg_puts(ctx, stmt->u.gcc_verbatim.text);
            cg_puts(ctx, ";");
            cg_nl(ctx);
        }
        break;

    case AST_EXPR_STMT:
        cg_indent(ctx);
        cg_expr_toplevel(ctx, stmt->u.expr_stmt.expr);
        cg_puts(ctx, ";");
        cg_nl(ctx);
        break;

    case AST_STRUCT_DEF:
        /* Local struct/union definition inside a function body.
         * Emit it as a block-scope type definition so that variable
         * declarations of that type (e.g. `cD s;`) in the same block
         * can use the struct name.  The generated C places the full
         * definition inline before any variables that reference it.
         * In C mode: skip structs that are emitted inline by their
         * enclosing field or variable declaration. */
        if (stmt->u.struct_def.is_nested_in_struct ||
            stmt->u.struct_def.from_inline_var) break;
        cg_indent(ctx);
        cg_struct(ctx, stmt);
        break;

    case AST_TYPEDEF_DECL: {
        const char *alias = stmt->u.typedef_decl.alias;
        const AstNode *target = stmt->u.typedef_decl.target;
        const char *td_attrs = stmt->u.typedef_decl.gcc_attrs;
        if (!alias || !target) break;
        cg_indent(ctx);
        if (target->kind == AST_TYPE_NAME && target->u.type_name.name) {
            if (td_attrs)
                cg_printf(ctx, "typedef %s %s %s;\n", target->u.type_name.name, alias, td_attrs);
            else
                cg_printf(ctx, "typedef %s %s;\n", target->u.type_name.name, alias);
        } else {
            Type *t = ty_from_ast(ctx->ts, target, cg_type_scope(ctx), NULL);
            cg_puts(ctx, "typedef ");
            cg_decl(ctx, t ? t : ty_int(ctx->ts), alias);
            if (td_attrs) { cg_puts(ctx, " "); cg_puts(ctx, td_attrs); }
            cg_puts(ctx, ";\n");
        }
        break;
    }

    case AST_DECL_STMT: {
        AstNode *vd = stmt->u.decl_stmt.decl;
        if (!vd) break;
        if (vd->kind == AST_TYPEDEF_DECL) {
            cg_stmt(ctx, vd, defers, ndefers);
            break;
        }
        /* BUG-005: local function prototype -- FUNC_DEF with no body
         * wrapped in a DECL_STMT.  Emit as a forward declaration.
         * Pass NULL as sname: sname is only for struct methods; passing
         * the function name would trigger method-name mangling. */
        if (vd->kind == AST_FUNC_DEF && !vd->u.func_def.body) {
            cg_indent(ctx);
            cg_func_decl(ctx, vd, NULL);
            break;
        }
        if (vd->kind != AST_VAR_DECL) break;
        /* A8/p71: In C mode, comma-continuation vars (int a=1,b=2) are
         * emitted as part of the first decl's line -- skip here. */
        if (vd->u.var_decl.is_comma_cont) break;
        cg_indent(ctx);
        cg_emit_storage_class(ctx, vd->u.var_decl.storage);
        /* emit C11 _Thread_local after storage-class. */
        if (vd->u.var_decl.is_thread_local) cg_puts(ctx, "_Thread_local ");
        /* emit C11 _Alignas after storage-class.
         * gcc -E emits spaces around parens: `_Alignas ( 16 )` */
        if (vd->u.var_decl.alignas_text) {
            cg_printf(ctx, "_Alignas ( %s ) ", vd->u.var_decl.alignas_text);
        }
        /* C-mode: inline named-struct-in-var (from_inline_var) or
         * anonymous struct body for local variable declarations. */
        if (ctx->local_block_stmts && vd->u.var_decl.type) {
            AstCvpaPeel vp = ast_type_peel_cvpa(vd->u.var_decl.type);
            if (vp.base && vp.base->kind == AST_TYPE_NAME && vp.base->u.type_name.name) {
                const char *vnm = vp.base->u.type_name.name;
                const AstNode *vsd = cg_find_inline_struct_def(ctx, vnm);
                if (vsd && vsd->u.struct_def.fields.len > 0) {
                    const char *vkw = struct_kw(vsd);
                    bool is_named = strncmp(vnm, "__anon_", 7) != 0;
                    if (vp.is_const) cg_puts(ctx, "const ");
                    if (vp.is_volatile) cg_puts(ctx, "volatile ");
                    if (is_named) cg_printf(ctx, "%s %s {\n", vkw, vnm);
                    else          cg_printf(ctx, "%s {\n", vkw);
                    cg_emit_struct_fields(ctx, vsd, "  ", false);
                    cg_puts(ctx, "}");
                    cg_emit_stars(ctx, vp.nptr);
                    cg_printf(ctx, " %s", vd->u.var_decl.name ? vd->u.var_decl.name : "");
                    /* Array suffixes */
                    const AstNode *arr = vd->u.var_decl.type;
                    cg_emit_array_suffixes(ctx, arr);
                    if (vd->u.var_decl.init) { cg_puts(ctx, " = "); cg_expr(ctx, vd->u.var_decl.init); }
                    cg_puts(ctx, ";"); cg_nl(ctx);
                    /* Clear from_inline_var so that
                     * subsequent variable declarations of the same struct type
                     * (e.g. `const struct EncName *pEnc` after the array
                     * `static const struct EncName { } encnames[]`) do NOT
                     * re-emit the struct body inline.  They will fall through
                     * to cg_type_from_ast which emits `struct EncName` using
                     * the is_struct_tag=true AST node. */
                    ((AstNode *)vsd)->u.struct_def.from_inline_var = false;
                    break;
                }
            }
        }
        /* C8: volatile handled below via cg_type_from_ast path. */
        Type *t = vd->sem_type;
        if (!t || ty_is_error(t))
            t = vd->u.var_decl.type
              ? ty_from_ast(ctx->ts, vd->u.var_decl.type, cg_type_scope(ctx), NULL)
              : ty_int(ctx->ts);
        /* auto type deduction: emit the sema-inferred concrete type. */
        bool is_plain_auto = (vd->u.var_decl.type &&
            vd->u.var_decl.type->kind == AST_TYPE_AUTO);
        /* BUG-002 fix: also handle `const auto x = expr` */
        bool is_const_auto = (vd->u.var_decl.type &&
            vd->u.var_decl.type->kind == AST_TYPE_CONST &&
            vd->u.var_decl.type->u.type_const.base &&
            vd->u.var_decl.type->u.type_const.base->kind == AST_TYPE_AUTO);
        if ((is_plain_auto || is_const_auto) && vd->u.var_decl.init) {
            if (t && !ty_is_error(t)) {
                /* Emit the concrete inferred type, not __auto_type. */
                if (t->kind == TY_STRUCT && t->u.struct_.nargs == 0) {
                    cg_printf(ctx, "struct %s %s", t->u.struct_.name,
                              vd->u.var_decl.name);
                } else {
                    cg_decl(ctx, t, vd->u.var_decl.name);
                }
            } else {
                cg_puts(ctx, "__auto_type ");
                if (vd->u.var_decl.name) cg_puts(ctx, vd->u.var_decl.name);
            }
            cg_puts(ctx, " = ");
            cg_expr(ctx, vd->u.var_decl.init);
            cg_puts(ctx, ";");
            cg_nl(ctx);
            break;
        }
        /* local struct types (e.g. `static const struct Foo {...} arr[]`)
         * may not be in cg_type_scope (file scope) because they were defined
         * inside a function body.  When resolution fails, emit the type using
         * the original AST node so we get `struct Foo` / `Foo` rather than
         * `int / *error* /`. */
        bool type_from_ast = false;
        if ((!t || ty_is_error(t)) && vd->u.var_decl.type) {
            type_from_ast = true;
        } else if (t && vd->u.var_decl.type) {
            /* Also use AST fallback when the sema type has an error anywhere
             * in its chain (e.g. TY_PTR(TY_CONST(TY_ERROR)) for `EncName *pEnc`
             * where EncName is a local struct invisible to file scope). */
            Type *file_t = ty_from_ast(ctx->ts, vd->u.var_decl.type,
                                       cg_type_scope(ctx), NULL);
            if (!file_t || ty_has_error(file_t))
                type_from_ast = true;

        }
        /* When the type is an array with an unknown (unevaluated) size
         * -- e.g. `unsigned char buff[CHAR_BIT * sizeof(size_t)]` where
         * `CHAR_BIT` is an identifier constant -- ty_from_ast stores
         * size=-1 and cg_decl would emit `buff[]`.  Fall back to the
         * original AST to emit the size expression verbatim so the C
         * compiler can evaluate it. */
        bool array_fallback = (t && t->kind == TY_ARRAY && t->u.array.size < 0);
        if (array_fallback && vd->u.var_decl.type) {
            /* Walk down the AST type to find the innermost base type
             * and collect array size expressions in order. */
            const AstNode *ast_ty = ast_type_strip_cv(vd->u.var_decl.type);
            if (ast_ty && ast_ty->kind == AST_TYPE_ARRAY &&
                ast_ty->u.type_array.size) {
                const AstNode *base_ast = ast_ty;
                while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                    base_ast = base_ast->u.type_array.base;
                Type *base_t = base_ast
                    ? ty_from_ast(ctx->ts, base_ast, cg_type_scope(ctx), NULL)
                    : t->u.array.base;
                /* when the element type is a (const) function pointer,
                 * use cg_decl with the FULL array type so it emits the correct
                 * `ret (* const name[N])(params)` form.  cg_type alone would emit
                 * `ret (*)(params) const name[N]` which is a syntax error. */
                {
                    const Type *elem = base_t;
                    bool elem_is_cfnptr = false;
                    if (elem && elem->kind == TY_CONST)  elem = elem->u.const_.base;
                    if (elem && elem->kind == TY_PTR)    elem = elem->u.ptr.base;
                    if (elem && elem->kind == TY_FUNC)   elem_is_cfnptr = true;
                    if (elem_is_cfnptr && t->kind == TY_ARRAY) {
                        cg_decl(ctx, t, vd->u.var_decl.name);
                        goto array_fallback_done;
                    }
                }
                /* C mode: always use cg_type_from_ast to preserve ptr_const,
                 * volatile, and other qualifiers that Type* doesn't track. */
                if (base_ast)
                    cg_type_from_ast(ctx, (AstNode *)base_ast);
                else if (!base_t || ty_is_error(base_t))
                    cg_puts(ctx, "void");
                else
                    cg_type(ctx, base_t);
                cg_printf(ctx, " %s", vd->u.var_decl.name);
                const AstNode *cur = ast_ty;
                cg_emit_array_suffixes(ctx, cur);
                array_fallback_done:;
            } else {
                /* No explicit array size in AST.  When the base type resolved
                 * to error (e.g. local struct EncName not in file scope),
                 * fall back to AST-based type emission. */
                Type *arr_base = t->u.array.base;
                /* Peel const layers to check for error base */
                const Type *peek = arr_base;
                while (peek && (peek->kind == TY_CONST || peek->kind == TY_ATOMIC))
                    peek = ty_unconst(NULL, peek);
                if (peek && ty_is_error(peek) && vd->u.var_decl.type) {
                    const AstNode *ast_base = vd->u.var_decl.type;
                    while (ast_base && ast_base->kind == AST_TYPE_ARRAY)
                        ast_base = ast_base->u.type_array.base;
                    cg_type_from_ast(ctx, (AstNode *)ast_base);
                    cg_printf(ctx, " %s[]", vd->u.var_decl.name);
                } else if (vd->u.var_decl.type) {
                    /* C mode: use AST to preserve ptr_const, volatile, etc.
                     * For arrays of const function pointers
                     * (e.g. `int (*const arr[])(params)`) the AST path emits
                     * `const int (*)(params) arr[]` which is wrong syntax.
                     * Use cg_decl with the semantic type instead, which
                     * correctly emits `int (* const arr[])(params)`. */
                    const AstNode *ast_base = vd->u.var_decl.type;
                    while (ast_base && ast_base->kind == AST_TYPE_ARRAY)
                        ast_base = ast_base->u.type_array.base;
                    /* Check if element type is (const) pointer-to-function */
                    bool elem_is_cfnptr = false;
                    {
                        const AstNode *eb = ast_base;
                        if (eb && (eb->kind == AST_TYPE_CONST || eb->kind == AST_TYPE_VOLATILE))
                            eb = (eb->kind == AST_TYPE_CONST) ? eb->u.type_const.base : eb->u.type_volatile.base;
                        /* Must be pointer whose base is a function type */
                        if (eb && eb->kind == AST_TYPE_PTR && eb->u.type_ptr.base &&
                            eb->u.type_ptr.base->kind == AST_TYPE_FUNC)
                            elem_is_cfnptr = true;
                        /* ptr_const on a pointer-to-function: `int (*const arr[])(p)` */
                        if (eb && eb->kind == AST_TYPE_PTR && eb->u.type_ptr.ptr_const &&
                            eb->u.type_ptr.base && eb->u.type_ptr.base->kind == AST_TYPE_FUNC)
                            elem_is_cfnptr = true;
                    }
                    if (elem_is_cfnptr) {
                        /* Use semantic cg_decl for correct `ret (* const name[])(params)` form */
                        cg_decl(ctx, t, vd->u.var_decl.name);
                    } else {
                        cg_type_from_ast(ctx, (AstNode *)ast_base);
                        cg_printf(ctx, " %s[]", vd->u.var_decl.name);
                    }
                } else {
                    cg_decl(ctx, t, vd->u.var_decl.name);
                }
            }
        } else if (type_from_ast && vd->u.var_decl.name) {
            /* Local struct type: emit directly from AST node.
             * Walk to find base type, emit it, then the name and array suffixes. */
            const AstNode *ast_ty = ast_type_strip_cv(vd->u.var_decl.type);
            /* Collect array dimensions */
            const AstNode *cur = ast_ty;
            while (cur && cur->kind == AST_TYPE_ARRAY)
                cur = cur->u.type_array.base;
            /* Emit base type */
            cg_type_from_ast(ctx, vd->u.var_decl.type);  /* handles const/array */
            cg_printf(ctx, " %s", vd->u.var_decl.name);
            /* Emit array suffixes */
            cur = ast_ty;
            cg_emit_array_suffixes(ctx, cur);
        } else {
            /* C8: if the type has volatile/_Atomic anywhere, emit from AST
             * directly to preserve the qualifier -- the Type* path strips both. */
            bool has_vol = vd->u.var_decl.type &&
                           (ast_type_has_volatile(vd->u.var_decl.type) ||
                            vd->u.var_decl.type->kind == AST_TYPE_ATOMIC);
            if (has_vol && vd->u.var_decl.type) {
                cg_type_from_ast(ctx, vd->u.var_decl.type);
                if (vd->u.var_decl.name)
                    cg_printf(ctx, " %s", vd->u.var_decl.name);
            } else {
                /* C5: in C mode use AST emission to preserve qualifiers + typedef aliases. */
                if (!vd->u.var_decl.type ||
                    !cg_decl_ast(ctx, vd->u.var_decl.type, vd->u.var_decl.name))
                    cg_decl(ctx, t, vd->u.var_decl.name);
            }
        }
        /* GCC attributes after the declarator, before `=` or `;`.
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
        if (ndefers > 0 && stmt->u.return_.value &&
            ctx->cur_fn_ret && !ty_is_void(ctx->cur_fn_ret)) {
            /* spec §defer P10.3: materialise return value into a temp BEFORE
             * emitting defers, then return the temp.
             *
             * Emit:
             *   <type> __sharp_ret = <expr>;
             *   <defers LIFO>
             *   return __sharp_ret;
             */
            cg_indent(ctx);
            cg_decl(ctx, ctx->cur_fn_ret, "__sharp_ret");
            cg_puts(ctx, " = ");
            cg_expr_toplevel(ctx, stmt->u.return_.value);
            cg_puts(ctx, ";");
            cg_nl(ctx);
            cg_emit_defers(ctx, defers, ndefers);
            cg_indent(ctx);
            cg_puts(ctx, "return __sharp_ret;");
            cg_nl(ctx);
        } else {
            cg_emit_defers(ctx, defers, ndefers);
            cg_indent(ctx);
            cg_puts(ctx, "return");
            if (stmt->u.return_.value) {
                cg_puts(ctx, " ");
                cg_expr_toplevel(ctx, stmt->u.return_.value);
            }
            cg_puts(ctx, ";");
            cg_nl(ctx);
        }
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

    case AST_COMPUTED_GOTO:
        cg_emit_defers(ctx, defers, ndefers);
        cg_indent(ctx);
        cg_puts(ctx, "goto *");
        cg_expr(ctx, stmt->u.computed_goto.target);
        cg_puts(ctx, ";");
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
         * sema annotated cond->sem_type with ty_int (live=then) or ty_void
         * (live=else).  Emit only the live branch -- no `if` wrapper, no
         * dead-branch code.
         *
         * v0.13: inside generic function/method specializations the
         * sema-time annotation was computed against TY_PARAM (T was
         * unbound), so it's stale.  Re-evaluate with the current
         * substitution context before deciding which branch is live. */
        AstNode *cond_if = stmt->u.if_.cond;
        if (cond_if && cond_if->kind == AST_AT_INTRINSIC) {
            int rv = -1;
            if (ctx->ngp > 0) {
                Scope *eval_scope = ctx->spec_scope ? ctx->spec_scope : ctx->file_scope;
                rv = sema_eval_intrinsic_specialized(
                    ctx->ts, eval_scope, NULL, cond_if,
                    ctx->gp_names, ctx->gp_vals, ctx->ngp);
            }
            /* Fall back to sema's annotation if re-eval didn't fire
             * (non-template context, or eval still non-constant). */
            if (rv < 0 && cond_if->sem_type) {
                Type *annot = cond_if->sem_type;
                /* Recognise the sema-set flag (ty_int = true, ty_void = false). */
                if (annot->kind == TY_VOID) rv = 0;
                else if (annot->kind == TY_INT) rv = 1;
                /* If type_ref carries ty_bool (the regular type), we
                 * also have the cached resolved_val from sema. */
                else if (cond_if->u.at_intrinsic.resolved_set)
                    rv = cond_if->u.at_intrinsic.resolved_val;
            } else if (rv < 0 && cond_if->u.at_intrinsic.resolved_set) {
                rv = cond_if->u.at_intrinsic.resolved_val;
            }
            if (rv == 0 || rv == 1) {
                bool live_is_then = (rv == 1);
                if (live_is_then) {
                    if (stmt->u.if_.then_)
                        cg_stmt(ctx, stmt->u.if_.then_, defers, ndefers);
                } else {
                    if (stmt->u.if_.else_)
                        cg_stmt(ctx, stmt->u.if_.else_, defers, ndefers);
                }
                break;
            }
        }
        cg_indent(ctx); cg_puts(ctx, "if (");
        cg_expr(ctx, cond_if);
        cg_puts(ctx, ") ");
        if (stmt->u.if_.then_ && stmt->u.if_.then_->kind == AST_BLOCK) {
            cg_block(ctx, stmt->u.if_.then_);
        } else if (ndefers > 0 && stmt->u.if_.then_) {
            /* When defers are in scope, wrap single-stmt then in braces.
             * break/continue/return emit defers before the jump, producing
             * multiple statements -- without braces the C output is
             * syntactically invalid (dangling statements after the jump). */
            cg_puts(ctx, "{\n"); ctx->indent++;
            cg_stmt(ctx, stmt->u.if_.then_, defers, ndefers);
            ctx->indent--;
            cg_indent(ctx); cg_puts(ctx, "}\n");
        } else {
            cg_nl(ctx); ctx->indent++;
            if (stmt->u.if_.then_) cg_stmt(ctx, stmt->u.if_.then_, defers, ndefers);
            ctx->indent--;
        }
        if (stmt->u.if_.else_) {
            cg_indent(ctx); cg_puts(ctx, "else ");
            if (stmt->u.if_.else_->kind == AST_BLOCK) {
                cg_block(ctx, stmt->u.if_.else_);
            } else if (ndefers > 0) {
                cg_puts(ctx, "{\n"); ctx->indent++;
                cg_stmt(ctx, stmt->u.if_.else_, defers, ndefers);
                ctx->indent--;
                cg_indent(ctx); cg_puts(ctx, "}\n");
            } else {
                cg_nl(ctx); ctx->indent++;
                cg_stmt(ctx, stmt->u.if_.else_, defers, ndefers);
                ctx->indent--;
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
            cg_nl(ctx); ctx->indent++;
            if (stmt->u.while_.body) cg_stmt(ctx, stmt->u.while_.body, NULL, 0);
            ctx->indent--;
        }
        break;

    case AST_DO_WHILE:
        cg_indent(ctx); cg_puts(ctx, "do ");
        /* body may be a BLOCK or a single statement.  Before
         * the fix, this branch unconditionally called cg_block, which
         * dereferences `body->u.block.stmts` -- when body is actually
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
            cg_nl(ctx); ctx->indent++;
            if (stmt->u.do_while.body) cg_stmt(ctx, stmt->u.do_while.body, NULL, 0);
            ctx->indent--;
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
                if (vd && vd->kind == AST_VAR_DECL) {
                    Type *t = vd->sem_type;
                    if (!t || ty_is_error(t))
                        t = vd->u.var_decl.type
                          ? ty_from_ast(ctx->ts, vd->u.var_decl.type, cg_type_scope(ctx), NULL)
                          : ty_int(ctx->ts);
                    cg_decl(ctx, t, vd->u.var_decl.name);
                    if (vd->u.var_decl.init) {
                        cg_puts(ctx, " = ");
                        cg_expr(ctx, vd->u.var_decl.init);
                    }
                }
            } else if (init->kind == AST_BLOCK) {
                /* Compound init: multi-variable for-init
                 * (e.g. `for (int m=0, n=10; …)`).  Emit as
                 * C99 comma-separated declarations. */
                for (size_t i = 0; i < init->u.block.stmts.len; i++) {
                    if (i > 0) cg_puts(ctx, ", ");
                    AstNode *sd = init->u.block.stmts.data[i];
                    if (sd->kind == AST_DECL_STMT) {
                        AstNode *vd = sd->u.decl_stmt.decl;
                        if (vd && vd->kind == AST_VAR_DECL) {
                            Type *t = vd->sem_type;
                            if (!t || ty_is_error(t))
                                t = vd->u.var_decl.type
                                  ? ty_from_ast(ctx->ts, vd->u.var_decl.type,
                                                cg_type_scope(ctx), NULL)
                                  : ty_int(ctx->ts);
                            if (i == 0)
                                cg_decl(ctx, t, vd->u.var_decl.name);
                            else
                                cg_emit_comma_cont_declarator(ctx,
                                    vd->u.var_decl.type, vd->u.var_decl.name);
                            if (vd->u.var_decl.init) {
                                cg_puts(ctx, " = ");
                                cg_expr(ctx, vd->u.var_decl.init);
                            }
                        }
                    }
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
        /* Body may be a BLOCK or a single statement -- handle both.
         * C mode: emit single-statement body directly (no brace wrapping). */
        if (stmt->u.for_.body && stmt->u.for_.body->kind == AST_BLOCK)
            cg_block(ctx, stmt->u.for_.body);
        else {
            cg_nl(ctx);
            ctx->indent++;
            if (stmt->u.for_.body) cg_stmt(ctx, stmt->u.for_.body, NULL, 0);
            ctx->indent--;
        }
        break;

    case AST_DEFER:
        /* defer is handled by cg_block's defer collection -- skip here */
        break;

    /* block-scope enum definition.  C allows
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
            cg_puts(ctx, (j + 1 < stmt->u.enum_def.items.len ||
                          stmt->u.enum_def.trailing_comma) ? ",\n" : "\n");
        }
        ctx->indent--;
        cg_indent(ctx); cg_puts(ctx, "};"); cg_nl(ctx);
        break;
    }

    default:
        cg_indent(ctx);
        cg_printf(ctx, "#error \"unhandled AST kind %d (%s) in cg_stmt\"\n",
                  stmt->kind, ast_kind_name(stmt->kind));
        break;
    }
}

/* =========================================================================
 * Block emission
 * ====================================================================== */

/* Emit a C block ({ ... }) from a block AST node.  Collects and emits
 * defer statements in LIFO order for every leaf path (return, break,
 * continue, goto, or end-of-block). */
static void cg_block(CgCtx *ctx, const AstNode *block) {
    if (!block) { cg_puts(ctx, "{ }"); cg_nl(ctx); return; }

    /* Collect defers in this block. */
    const AstNode **defers = NULL;
    size_t ndefers = 0, dcap = 0;

    cg_puts(ctx, "{"); cg_nl(ctx);
    ctx->indent++;

    /* Set local_block_stmts so cg_field_decl_from_ast can find inline anon structs */
    const AstVec *_prev_local = ctx->local_block_stmts;
    ctx->local_block_stmts = &block->u.block.stmts;

    for (size_t i = 0; i < block->u.block.stmts.len; i++) {
        const AstNode *s = block->u.block.stmts.data[i];
        if (!s) continue;
        /* In C mode: skip anonymous struct stmts whose body will be inlined
         * into the parent struct (the next non-null stmt). */
        if (s->kind == AST_STRUCT_DEF && s->u.struct_def.name &&
            strncmp(s->u.struct_def.name, "__anon_", 7) == 0) {
            const char *anon_nm = s->u.struct_def.name;
            /* Check if the next non-null stmt has a field using this type */
            for (size_t j = i + 1; j < block->u.block.stmts.len; j++) {
                const AstNode *ns = block->u.block.stmts.data[j];
                if (!ns) continue;
                const AstNode *ns_sd = (ns->kind == AST_STRUCT_DEF) ? ns : NULL;
                if (!ns_sd) break;
                for (size_t fi = 0; fi < ns_sd->u.struct_def.fields.len; fi++) {
                    const AstNode *f = ns_sd->u.struct_def.fields.data[fi];
                    if (!f || f->kind != AST_FIELD_DECL || !f->u.field_decl.type) continue;
                    const AstNode *ft = f->u.field_decl.type;
                    while (ft && ft->kind == AST_TYPE_ARRAY) ft = ft->u.type_array.base;
                    if (ft && ft->kind == AST_TYPE_NAME && ft->u.type_name.name &&
                        strcmp(ft->u.type_name.name, anon_nm) == 0) goto skip_anon_local;
                }
                break;
            }
            /* Also check for anonymous field injection (field name __anon_field_N) */
            goto emit_local_struct;
            skip_anon_local: continue;
            emit_local_struct:;
        }
        if (s->kind == AST_DEFER) {
            if (ndefers == dcap) {
                dcap = dcap ? dcap * 2 : 4;
                defers = realloc(defers, dcap * sizeof *defers);
                if (!defers) abort();
            }
            defers[ndefers++] = s;
        } else if (
                   s->kind == AST_DECL_STMT &&
                   s->u.decl_stmt.decl &&
                   s->u.decl_stmt.decl->kind == AST_VAR_DECL &&
                   !s->u.decl_stmt.decl->u.var_decl.is_comma_cont) {
            /* A8/p71: Count how many comma-continuation var decls follow.
             * `int _a = (3), _b = (7)` -- emit as one comma-group. */
            const AstNode *vd0 = s->u.decl_stmt.decl;
            size_t j = i + 1;
            while (j < block->u.block.stmts.len) {
                const AstNode *ns = block->u.block.stmts.data[j];
                if (!ns || ns->kind != AST_DECL_STMT ||
                    !ns->u.decl_stmt.decl ||
                    ns->u.decl_stmt.decl->kind != AST_VAR_DECL ||
                    !ns->u.decl_stmt.decl->u.var_decl.is_comma_cont) break;
                j++;
            }
            if (j == i + 1) {
                /* No continuations: normal emit */
                cg_stmt(ctx, s, defers, ndefers);
            } else {
                /* Comma group: emit `type first = init, b = init, ...;` */
                bool _inline_done = false;
                cg_indent(ctx);
                /* Inline anonymous struct body if needed */
                if (vd0->u.var_decl.type) {
                    AstCvpaPeel _vp = ast_type_peel_cvpa(vd0->u.var_decl.type);
                    if (_vp.base && _vp.base->kind == AST_TYPE_NAME && _vp.base->u.type_name.name) {
                        const char *_vnm = _vp.base->u.type_name.name;
                        const AstNode *_vsd = cg_find_inline_struct_def(ctx, _vnm);
                        if (_vsd && _vsd->u.struct_def.fields.len > 0) {
                            const char *_vkw = struct_kw(_vsd);
                            bool _is_named = strncmp(_vnm, "__anon_", 7) != 0;
                            if (_vp.is_const) cg_puts(ctx, "const ");
                            if (_vp.is_volatile) cg_puts(ctx, "volatile ");
                            if (_is_named) cg_printf(ctx, "%s %s {\n", _vkw, _vnm);
                            else          cg_printf(ctx, "%s {\n", _vkw);
                            cg_emit_struct_fields(ctx, _vsd, "  ", false);
                            cg_puts(ctx, "}");
                            cg_emit_stars(ctx, _vp.nptr);
                            cg_printf(ctx, " %s", vd0->u.var_decl.name ? vd0->u.var_decl.name : "");
                            const AstNode *_arr = vd0->u.var_decl.type;
                            cg_emit_array_suffixes(ctx, _arr);
                            if (vd0->u.var_decl.init) { cg_puts(ctx, " = "); cg_expr(ctx, vd0->u.var_decl.init); }
                            /* Emit continuations */
                            for (size_t _k = i + 1; _k < j; _k++) {
                                const AstNode *_vdk = block->u.block.stmts.data[_k]->u.decl_stmt.decl;
                                cg_puts(ctx, ", ");
                                cg_puts(ctx, _vdk->u.var_decl.name ? _vdk->u.var_decl.name : "");
                                const AstNode *_a2 = _vdk->u.var_decl.type;
                                cg_emit_array_suffixes(ctx, _a2);
                                if (_vdk->u.var_decl.init) { cg_puts(ctx, " = "); cg_expr(ctx, _vdk->u.var_decl.init); }
                            }
                            cg_puts(ctx, ";"); cg_nl(ctx);
                            ((AstNode *)_vsd)->u.struct_def.from_inline_var = false;
                            i = j - 1;
                            _inline_done = true;
                        }
                    }
                }
                if (!_inline_done) {
                cg_emit_storage_class(ctx, vd0->u.var_decl.storage);
                /* Emit type + first declarator */
                {
                    Type *t = vd0->sem_type;
                    if (!t || ty_is_error(t))
                        t = vd0->u.var_decl.type
                          ? ty_from_ast(ctx->ts, vd0->u.var_decl.type, cg_type_scope(ctx), NULL)
                          : ty_int(ctx->ts);
                    if (!vd0->u.var_decl.type ||
                        !cg_decl_ast(ctx, vd0->u.var_decl.type, vd0->u.var_decl.name))
                        cg_decl(ctx, t, vd0->u.var_decl.name);
                }
                if (vd0->u.var_decl.init) {
                    cg_puts(ctx, " = ");
                    cg_expr(ctx, vd0->u.var_decl.init);
                }
                /* Emit continuations */
                for (size_t k = i + 1; k < j; k++) {
                    const AstNode *vdk = block->u.block.stmts.data[k]->u.decl_stmt.decl;
                    cg_puts(ctx, ", ");
                    cg_emit_comma_cont_declarator(ctx,
                        vdk->u.var_decl.type, vdk->u.var_decl.name);
                    if (vdk->u.var_decl.init) {
                        cg_puts(ctx, " = ");
                        cg_expr(ctx, vdk->u.var_decl.init);
                    }
                }
                cg_puts(ctx, ";");
                cg_nl(ctx);
                i = j - 1; /* skip continuation stmts */
                } /* !_inline_done */
            }
        } else {
            cg_stmt(ctx, s, defers, ndefers);
        }
    }

    ctx->local_block_stmts = _prev_local;

    /* End of block: emit remaining defers LIFO -- but only for fall-through
     * paths.  If the last non-null statement was a terminal (return/break/
     * continue/computed goto), cg_stmt already emitted the defers; emitting
     * them again here would produce duplicate calls.                        */
    {
        bool _block_terminated = false;
        for (size_t _bi = block->u.block.stmts.len; _bi > 0; _bi--) {
            const AstNode *_ls = block->u.block.stmts.data[_bi - 1];
            if (!_ls) continue;
            AstKind _k = _ls->kind;
            if (_k == AST_RETURN || _k == AST_BREAK ||
                _k == AST_CONTINUE || _k == AST_COMPUTED_GOTO)
                _block_terminated = true;
            break;
        }
        if (!_block_terminated)
            cg_emit_defers(ctx, defers, ndefers);
    }
    free(defers);

    ctx->indent--;
    cg_indent(ctx); cg_puts(ctx, "}"); cg_nl(ctx);
}

/* =========================================================================
 * Function (free or method) codegen
 * ====================================================================== */
static bool cg_expr_uses_this(const AstNode *expr);
static bool cg_stmt_uses_this(const AstNode *stmt);
static bool cg_block_uses_name(const AstNode *block, const char *name);
static bool cg_block_uses_this(const AstNode *block);
static bool cg_fn_uses_this(const AstNode *fn);

/* Returns true if 'sname' has an explicit typedef in scope (so it can be
 * used as a bare type name in generated C without `struct` prefix).
 * Also returns true for 'class' definitions (auto-typedef is injected). */
static bool cg_struct_has_explicit_typedef(CgCtx *ctx, const char *sname) {
    if (!sname || !ctx->file_scope) return false;
    Symbol *sym = scope_lookup(ctx->file_scope, sname);
    for (Symbol *s = sym; s; s = s->next) {
        if (strcmp(s->name, sname) != 0) continue;
        if (s->kind == SYM_TYPE && s->decl) {
            if (s->decl->kind == AST_TYPEDEF_DECL) return true;
            if (s->decl->kind == AST_STRUCT_DEF &&
                s->decl->u.struct_def.is_class) return true;
        }
    }
    /* Also check struct-tag namespace for class */
    Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, sname);
    if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
        tag->decl->u.struct_def.is_class) return true;
    return false;
}

/* Emit a struct type name with the correct `struct`/`union` prefix when
 * there is no explicit typedef, or bare name when there is one. */
static void cg_emit_struct_type(CgCtx *ctx, const char *sname) {
    if (cg_struct_has_explicit_typedef(ctx, sname)) {
        cg_puts(ctx, sname);
        return;
    }
    /* No explicit typedef: need struct/union keyword for valid C11 */
    const char *kw = "struct";
    if (ctx->file_scope) {
        Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, sname);
        if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
            tag->decl->u.struct_def.is_union) kw = "union";
    }
    cg_printf(ctx, "%s %s", kw, sname);
}

static void cg_emit_struct_fwd_decl(CgCtx *ctx, const char *name) {
    Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, name);
    bool is_union = tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF && tag->decl->u.struct_def.is_union;
    bool has_td = cg_struct_has_explicit_typedef(ctx, name);
    const char *kw = is_union ? "union" : "struct";
    if (has_td)
        cg_printf(ctx, "typedef %s %s %s;\n", kw, name, name);
    else
        cg_printf(ctx, "%s %s;\n", kw, name);
}

static void cg_func(CgCtx *ctx, const AstNode *fn, const char *sname) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;

    /* Generic functions: skip -- specialized versions emitted by Phase 11 pass. */
    if (fn->u.func_def.generic_params.len > 0) return;

    /* Set cur_struct/cur_struct_def for struct method bodies so that
     * method name mangling uses the struct name correctly. */
    const char *prev_st = ctx->cur_struct;
    const AstNode *prev_sd = ctx->cur_struct_def;
    if (sname && ctx->file_scope) {
        ctx->cur_struct = sname;
        Symbol *ss = scope_lookup_type(ctx->file_scope, sname);
        ctx->cur_struct_def = NULL;
        for (Symbol *s = ss; s; s = s->next) {
            if (s->kind == SYM_TYPE && s->decl &&
                s->decl->kind == AST_STRUCT_DEF) {
                ctx->cur_struct_def = s->decl;
                break;
            }
        }
    }

    cg_emit_linemarker(ctx, fn->loc);
    StorageClass sc = fn->u.func_def.storage;
    /* p43/p45: leading attributes and keywords before storage class */
    if (fn->u.func_def.gcc_attrs && fn->u.func_def.fmt & FMTF_ATTRS_LEADING) {
        cg_puts(ctx, fn->u.func_def.gcc_attrs); cg_puts(ctx, " ");
    }
    if (fn->u.func_def.is_inline && fn->u.func_def.fmt & FMTF_INLINE_LEADING) {
        cg_puts(ctx, fn->u.func_def.inline_kw ? fn->u.func_def.inline_kw : "inline");
        cg_puts(ctx, " ");
    }
    /* p45: leading __const (GCC function-const qualifier) before storage */
    AstNode *ret_const_node = NULL;
    if (fn->u.func_def.fmt & FMTF_CONST_LEADING && fn->u.func_def.ret_type &&
        fn->u.func_def.ret_type->kind == AST_TYPE_CONST) {
        ret_const_node = fn->u.func_def.ret_type;
        const char *ck = ret_const_node->u.type_const.kw;
        cg_puts(ctx, ck ? ck : "const"); cg_puts(ctx, " ");
    }
    cg_emit_storage_class(ctx, sc);
    if (sc == SC_TYPEDEF) { /* no return type */ }
    /* C11 _Noreturn -- emitted after storage class, before return type */
    if (fn->u.func_def.is_noreturn) cg_puts(ctx, "_Noreturn ");
    /* C23: constexpr -- compile-time evaluable function */
    if (fn->u.func_def.is_constexpr) cg_puts(ctx, "constexpr ");
    /* Non-leading inline keyword (after storage class) */
    /* If __attribute__ appeared before `inline` in the
     * source (e.g. `static __attribute__((always_inline)) inline f()`),
     * emit attrs first, then inline -- matching the original source order
     * and gcc -E output.  Otherwise keep the inline-first order. */
    if (fn->u.func_def.fmt & FMTF_INLINE_AFTER_ATTRS) {
        /* attrs-first order: __attribute__((X)) inline */
        if (fn->u.func_def.gcc_attrs && !(fn->u.func_def.fmt & FMTF_ATTRS_LEADING)) {
            cg_puts(ctx, fn->u.func_def.gcc_attrs); cg_puts(ctx, " ");
        }
        if (fn->u.func_def.is_inline && !(fn->u.func_def.fmt & FMTF_INLINE_LEADING)) {
            cg_puts(ctx, fn->u.func_def.inline_kw ? fn->u.func_def.inline_kw : "inline");
            cg_puts(ctx, " ");
        }
    } else {
        /* inline-first order (default): inline __attribute__((X)) */
        if (fn->u.func_def.is_inline && !(fn->u.func_def.fmt & FMTF_INLINE_LEADING)) {
            cg_puts(ctx, fn->u.func_def.inline_kw ? fn->u.func_def.inline_kw : "inline");
            cg_puts(ctx, " ");
        }
        if (fn->u.func_def.gcc_attrs && !(fn->u.func_def.fmt & FMTF_ATTRS_LEADING)) {
            cg_puts(ctx, fn->u.func_def.gcc_attrs); cg_puts(ctx, " ");
        }
    }
    if (fn->u.func_def.is_thread_local) cg_puts(ctx, "_Thread_local ");

    /* Return type -- use ret_const_node skip if already emitted leading const */
    AstNode *ret_node = fn->u.func_def.ret_type;
    if (ret_const_node && ret_node == ret_const_node)
        ret_node = ret_const_node->u.type_const.base; /* skip the already-emitted const */
    Type *ret_t = ret_node ? ty_from_ast(ctx->ts, ret_node, cg_type_scope(ctx), NULL)
                           : ty_void(ctx->ts);
    /* a function whose return type is itself a pointer to a
     * function must be emitted as a *nested* declarator
     *
     *     inner_ret (* name(outer_args))(inner_args)
     *
     * (ISO C 6.7.6.3 -- the declarator "inhabits" the place where the
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
        TyPtrPeel rpp = ty_peel_ptr(ret_t);
        ret_nstars = rpp.nstars;
        if (rpp.base && rpp.base->kind == TY_FUNC) ret_inner = rpp.base;
    }
    /* when ret_inner is set (return type is a function-pointer),
     * the Type* path may lose array size info for PTR(ARRAY) chains.
     * E.g. int (*(*get_handler(int))(int))[8] -- the innermost PTR(ARRAY)
     * is collapsed to PTR(int) by ty_from_ast.  Detect this pattern via
     * the AST so we can emit the array suffix correctly. */
    bool ret_has_deep_array = false;
    const AstNode *ret_deep_array_ast = NULL;
    if (ret_inner && ret_node) {
        AstPtrPeel rap = ast_peel_ptr(ret_node);
        const AstNode *ra = rap.base;
        if (ra && ra->kind == AST_TYPE_FUNC) {
            const AstNode *ir = ra->u.type_func.ret;
            if (ir && ir->kind == AST_TYPE_PTR &&
                ir->u.type_ptr.base &&
                ir->u.type_ptr.base->kind == AST_TYPE_ARRAY) {
                ret_has_deep_array = true;
                ret_deep_array_ast = ir->u.type_ptr.base; /* ARRAY node */
            }
        }
    }
    /* C8/C5: in C mode, if the return-type AST node is a typedef alias
     * for a function pointer (e.g. `panic_fn` = `int (*)(int)`), preserve
     * the alias name and suppress the nested declarator expansion.
     * This gives `panic_fn lua_atpanic(...)` not `int (*lua_atpanic(...))(int)`. */
    if (ret_inner && ret_node &&
        ret_node->kind == AST_TYPE_NAME && !ret_node->u.type_name.is_struct_tag &&
        ctx->file_scope) {
        const char *rnm = ret_node->u.type_name.name;
        Symbol *sym = scope_find_typedef(ctx->file_scope, rnm);
        if (sym) {
            ret_inner = NULL;   /* use flat typedef form */
            ret_nstars = 0;
        }
    }
    /* pointer-to-array return type -- e.g. jmp_buf* where
     * jmp_buf = __jmp_buf_tag[1].  Must be emitted as the ISO C
     * pointer-to-array nested declarator form:
     *
     *     __jmp_buf_tag (*png_set_longjmp_fn(...))[1]
     *
     * Using the typedef name verbatim (cg_type_from_ast) would emit
     * `jmp_buf *name(...)` which is fine for function bodies (typedef
     * is in scope), but in pass-1c forward declarations the typedef
     * may not yet be defined, causing cc to reject the generated file.
     * The nested form needs no typedef and is always valid. */
    Type *ret_arr_base  = NULL;
    int64_t ret_arr_size = -1;
    if (!ret_inner && ret_t && ty_is_ptr_to_array(ret_t)) {
        ret_arr_base = ret_t->u.ptr.base->u.array.base;
        ret_arr_size = ret_t->u.ptr.base->u.array.size;
    }
    /* C8/C5: if the return-type AST is a typedef alias for ptr-to-array,
     * suppress the nested form and use the flat typedef name instead.
     * e.g. IntPair *get_pair(void) not int (*get_pair(void))[2]
     * ret_node may be PTR(TYPE_NAME("IntPair")) for `IntPair *get_pair`. */
    if (ret_arr_base && ret_node && ctx->file_scope) {
        const AstNode *rn = ret_node;
        /* Peel outer PTR if present (for IntPair *get_pair) */
        if (rn->kind == AST_TYPE_PTR) rn = rn->u.type_ptr.base;
        if (rn && rn->kind == AST_TYPE_NAME && !rn->u.type_name.is_struct_tag) {
            const char *rnm = rn->u.type_name.name;
            Symbol *sym = scope_find_typedef(ctx->file_scope, rnm);
            if (sym) {
                ret_arr_base = NULL; ret_arr_size = -1;  /* use flat form */
            }
        }
    }
    if (ret_inner) {
        /* outer-ret + `(* …` opener; the matching `)( inner-args )`
         * closer is emitted after the outer parameter list below. */
        if (ret_has_deep_array) {
            /* int (*(*name(params))(inner_params))[N] -- the Type* path
             * collapsed the innermost PTR(ARRAY) to PTR; use AST for
             * the element type and ptr-to-array opener. */
            cg_type_from_ast(ctx, ret_deep_array_ast->u.type_array.base);
            cg_puts(ctx, " (*");
            cg_puts(ctx, " (");
        } else {
            cg_type(ctx, ret_inner->u.func.ret);
            cg_puts(ctx, " (");
        }
        cg_emit_stars(ctx, ret_nstars);
    } else if (ret_arr_base) {
        /* pointer-to-array: emit base type + ` (*` opener.
         * Closer `)[N]` is emitted after the parameter list. */
        cg_type(ctx, ret_arr_base);
        cg_puts(ctx, " (*");
    } else {
        /* C8/C5: in C mode, use cg_type_from_ast to preserve typedef
         * aliases in return types (e.g. `Opaque *make(...)` not
         * `struct OpaqueStruct *make(...)`). */
        if (ret_node)
            cg_type_from_ast(ctx, ret_node);
        else
            cg_type(ctx, ret_t);
        cg_puts(ctx, " ");
    }

    /* Function name */
    if (sname) {
        cg_method_name(ctx, sname, fn->u.func_def.name);
    } else if (strncmp(fn->u.func_def.name, "operator", 8) == 0 &&
               fn->u.func_def.name[8] != '\0' &&
               !((unsigned char)fn->u.func_def.name[8] >= 'a' &&
                 (unsigned char)fn->u.func_def.name[8] <= 'z') &&
               !((unsigned char)fn->u.func_def.name[8] >= 'A' &&
                 (unsigned char)fn->u.func_def.name[8] <= 'Z') &&
               fn->u.func_def.name[8] != '_') {
        /* free operator function -- mangle only when the char
         * immediately after "operator" is a symbol (e.g. '+', '==').
         * Plain identifier chars like 'M' in `operatorMask` mean this is
         * a regular C function that happens to start with "operator",
         * not a Sharp operator overload -- emit the name verbatim. */
        const char *sym = fn->u.func_def.name + 8;
        cg_emit_mangled_free_op_name(ctx, sym, fn);
    } else {
        /* C8: if the source had a parenthesized name `(func)`, preserve it. */
        if (fn->u.func_def.name_paren && !sname)
            cg_puts(ctx, "(");
        cg_puts(ctx, fn->u.func_def.name);
        if (fn->u.func_def.name_paren && !sname)
            cg_puts(ctx, ")");
    }

    /* Parameters */
    cg_puts(ctx, "(");

    /* p21 K&R: emit only param names in parens, then type declarations after */
    if (fn->u.func_def.is_kr_style &&
        fn->u.func_def.params.len > 0 && !sname) {
        bool kfirst = true;
        for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
            AstNode *p = fn->u.func_def.params.data[i];
            if (!p || p->u.param_decl.is_vararg) continue;
            /* K&R params: type name IS the param name */
            const char *pname = p->u.param_decl.name
                              ? p->u.param_decl.name
                              : (p->u.param_decl.type &&
                                 p->u.param_decl.type->kind == AST_TYPE_NAME)
                                ? p->u.param_decl.type->u.type_name.name : "?";
            if (!kfirst) cg_puts(ctx, ", ");
            cg_puts(ctx, pname);
            kfirst = false;
        }
        cg_puts(ctx, ")");
        /* Emit K&R type declarations before the body */
        for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
            AstNode *p = fn->u.func_def.params.data[i];
            if (!p || p->u.param_decl.is_vararg || !p->u.param_decl.name) continue;
            if (!p->u.param_decl.type) continue;
            cg_puts(ctx, " ");
            cg_type_from_ast(ctx, p->u.param_decl.type);
            cg_printf(ctx, " %s;", p->u.param_decl.name);
        }
        if (fn->u.func_def.gcc_attrs) {
            cg_puts(ctx, " "); cg_puts(ctx, fn->u.func_def.gcc_attrs);
        }
        cg_puts(ctx, " ");
        if (fn->u.func_def.body) cg_block(ctx, fn->u.func_def.body);
        else cg_puts(ctx, ";");
        cg_nl(ctx);
        return;
    }

    bool first = true;

    /* For methods: add 'this' as first param only if body uses 'this'.
     * If the body never references 'this', it's an associated function
     * (spec: "判定规则: 函数体内不使用 this → 关联函数"). */
    bool is_assoc_fn = sname && !cg_fn_uses_this(fn);
    /* Extension methods with explicit receiver already have `this`
     * injected in scope pass 2 — don't add another one in CG. */
    if (fn->u.func_def.has_receiver) is_assoc_fn = true;
    if (sname && !is_assoc_fn) {
        if (fn->u.func_def.is_const_method) {
            cg_puts(ctx, "const "); cg_emit_struct_type(ctx, sname); cg_puts(ctx, " *this");
        } else {
            cg_emit_struct_type(ctx, sname); cg_puts(ctx, " *this");
        }
        first = false;
    }

    bool has_vararg = false;
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (!p) continue;
        if (p->u.param_decl.is_vararg) { has_vararg = true; continue; }
        if (!first) cg_puts(ctx, ", ");
        first = false;
        /* emit register storage class on parameters */
        if (p->u.param_decl.storage == SC_REGISTER) cg_puts(ctx, "register ");
        Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type, cg_type_scope(ctx), NULL);
        /* C8: check for restrict/static qualifiers inside array brackets
         * `char *v[restrict]` / `int a[static 10]` -- emit faithfully. */
        const AstNode *par_ty = p->u.param_decl.type;
        /* Use AST-direct emission for ALL named array params so that
         * qualifier positions (ptr_const, restrict, static, VLA size) are
         * preserved faithfully.  ty_from_ast loses ptr_const on pointers. */
        /* Avoid intercepting fnptr-in-array: let cg_decl_ast handle it */
        const AstNode *arr_base = par_ty && par_ty->kind == AST_TYPE_ARRAY ?
                                  par_ty->u.type_array.base : NULL;
        bool is_fnptr_arr = arr_base && arr_base->kind == AST_TYPE_PTR &&
                            arr_base->u.type_ptr.base &&
                            arr_base->u.type_ptr.base->kind == AST_TYPE_FUNC;
        bool par_is_arr_qual = par_ty && par_ty->kind == AST_TYPE_ARRAY &&
                               p->u.param_decl.name != NULL &&
                               !is_fnptr_arr;
        if (par_is_arr_qual) {
            /* Emit element type + name + [qualifiers + size].
             * For nested arrays (int m[3][3]), walk through all AST_TYPE_ARRAY
             * layers to find the innermost element type, then emit all array
             * suffixes outermost→innermost. */
            const AstNode *arr_cursor = par_ty;
            int arr_depth = 0;
            while (arr_cursor && arr_cursor->kind == AST_TYPE_ARRAY) {
                arr_depth++;
                arr_cursor = arr_cursor->u.type_array.base;
            }
            /* arr_cursor is now the innermost non-array base type */
            cg_type_from_ast(ctx, arr_cursor);
            cg_printf(ctx, " %s", p->u.param_decl.name);
            /* Emit array suffixes outermost→innermost (= source order) */
            arr_cursor = par_ty;
            for (int d = 0; d < arr_depth && arr_cursor && arr_cursor->kind == AST_TYPE_ARRAY; d++) {
                cg_puts(ctx, "[");
                if (arr_cursor->u.type_array.has_static)   cg_puts(ctx, "static ");
                if (arr_cursor->u.type_array.has_restrict)  cg_puts(ctx, "restrict");
                if (arr_cursor->u.type_array.has_vla_star) {
                    cg_puts(ctx, "*");
                } else if (arr_cursor->u.type_array.size) {
                    if (arr_cursor->u.type_array.has_restrict) cg_puts(ctx, " ");
                    cg_const_expr(ctx, arr_cursor->u.type_array.size);
                }
                cg_puts(ctx, "]");
                arr_cursor = arr_cursor->u.type_array.base;
            }
        } else if ( par_ty &&
                   par_ty->kind == AST_TYPE_PTR && par_ty->u.type_ptr.base &&
                   par_ty->u.type_ptr.base->kind == AST_TYPE_FUNC) {
            /* C8: function pointer parameter -- emit via AST to preserve
             * typedef names in the callback's own parameter types. */
            const AstNode *fp_fn = par_ty->u.type_ptr.base;
            const AstNode *fp_ret = fp_fn->u.type_func.ret;
            /* Check if it returns another function pointer */
            bool fp_ret_is_fnptr = fp_ret && fp_ret->kind == AST_TYPE_PTR &&
                fp_ret->u.type_ptr.base &&
                fp_ret->u.type_ptr.base->kind == AST_TYPE_FUNC;
            if (fp_ret_is_fnptr) {
                /* ret_ret (*(*name)(params))(ret_params) */
                const AstNode *rr_fn = fp_ret->u.type_ptr.base;
                cg_type_from_ast(ctx, rr_fn->u.type_func.ret);
                cg_printf(ctx, " (*(*%s)(", p->u.param_decl.name ? p->u.param_decl.name : "");
                cg_emit_func_params_ast(ctx, fp_fn);
                cg_puts(ctx, "))(");
                cg_emit_func_params_ast(ctx, rr_fn);
                cg_puts(ctx, ")");
            } else {
                /* ret (*name)(params) */
                cg_type_from_ast(ctx, fp_ret ? fp_ret : (AstNode*)NULL);
                const char *pnm = p->u.param_decl.name;
                if (pnm) cg_printf(ctx, " (*%s)(", pnm);
                else     cg_puts(ctx, " (*)(");
                cg_emit_func_params_ast(ctx, fp_fn);
                cg_puts(ctx, ")");
            }
        } else if (par_ty && par_ty->kind == AST_TYPE_FUNC) {
            /* Bare function-type parameter: `int f(int (), int)`.
             * ISO C §6.7.6.3: function-type params adjust to pointer-to-function.
             * Emit as `rettype (*name)(params)`. */
            cg_type_from_ast(ctx, par_ty->u.type_func.ret);
            const char *_pnm = p->u.param_decl.name;
            if (_pnm) cg_printf(ctx, " (*%s)(", _pnm);
            else      cg_puts(ctx, " (*)(");
            cg_emit_func_params_ast(ctx, par_ty);
            cg_puts(ctx, ")");
        } else {
            /* C8/C5: in C mode, prefer AST-direct emission to preserve
             * typedef aliases, const/volatile/restrict qualifier positions,
             * and avoid sema-mangled Type* paths. */
            bool emitted = par_ty &&
                cg_decl_ast(ctx, par_ty, p->u.param_decl.name);
            if (!emitted) cg_decl(ctx, pt, p->u.param_decl.name);
        }
        /* C7: emit GCC attribute after the param declarator */
        if (p->u.param_decl.gcc_attrs) {
            cg_puts(ctx, " ");
            cg_puts(ctx, p->u.param_decl.gcc_attrs);
        }
    }
    if (has_vararg) {
        if (!first) cg_puts(ctx, ", ");
        cg_puts(ctx, "...");
        first = false;
    }
    /* C3: in C mode, emit "()" if source had no explicit void */
    if (first) {
        if (!fn->u.func_def.params_unspecified)
            cg_puts(ctx, "void");
    }
    cg_puts(ctx, ")");

    /* close the nested declarator for "function returning
     * function pointer".  Form: `outer_ret (* name(outer_args))
     *                                       (inner_args)`.
     * C8: check AST for explicit (void) vs unspecified () -- Type* loses
     * this distinction (both have nparams=0). */
    if (ret_inner) {
        cg_puts(ctx, ")(");
        /* Find the inner AST_TYPE_FUNC node by peeling through the
         * return-type AST (PTR* → FUNC).  If we find it and
         * params_unspecified=false with zero params, emit `void`. */
        bool inner_void_explicit = false;
        if (ret_node) {
            AstCvpaPeel rp = ast_type_peel_cvpa(ret_node);
            const AstNode *ra = rp.base;
            if (ra && ra->kind == AST_TYPE_FUNC &&
                !ra->u.type_func.params_unspecified &&
                ra->u.type_func.params.len == 0)
                inner_void_explicit = true;
        }
        if (inner_void_explicit)
            cg_puts(ctx, "void");
        else
            cg_func_params(ctx, ret_inner);
        cg_puts(ctx, ")");
        /* when ret_has_deep_array, close the ptr-to-array
         * wrapper that was opened before the fnptr declarator.
         * E.g. int (*(*name(params))(inner_params))[N] → append )[N] */
        if (ret_has_deep_array) {
            cg_puts(ctx, ")[");
            if (ret_deep_array_ast->u.type_array.size)
                cg_const_expr(ctx, ret_deep_array_ast->u.type_array.size);
            cg_puts(ctx, "]");
        }
    } else if (ret_arr_base) {
        /* close pointer-to-array nested declarator: ")[N]" */
        cg_puts(ctx, ")");
        cg_puts(ctx, "[");
        if (ret_arr_size >= 0) {
            char _ab[32]; snprintf(_ab, sizeof _ab, "%lld", (long long)ret_arr_size);
            cg_puts(ctx, _ab);
        }
        cg_puts(ctx, "]");
    }

    if (!fn->u.func_def.body || ctx->proto_only) {
        /* C8: trailing __attribute__ after params, before ; */
        if (fn->u.func_def.trailing_gcc_attrs) {
            cg_puts(ctx, " ");
            cg_puts(ctx, fn->u.func_def.trailing_gcc_attrs);
        }
        cg_puts(ctx, ";"); cg_nl(ctx);
        return;
    }

    cg_puts(ctx, " ");
    ctx->cur_fn_ret = ret_t;
    cg_block(ctx, fn->u.func_def.body);
    ctx->cur_fn_ret = NULL;
    /* Restore cur_struct/cur_struct_def if we set them for a method body. */
    if (sname && ctx->file_scope) {
        ctx->cur_struct = prev_st;
        ctx->cur_struct_def = prev_sd;
    }
}

/* =========================================================================
 * Struct codegen
 * ====================================================================== */
static void cg_struct(CgCtx *ctx, const AstNode *sd) {
    if (!sd || sd->kind != AST_STRUCT_DEF) return;

    /* Generic struct definitions are skipped here -- they have no standalone
     * body to emit.  Concrete instances (Vec__int, etc.) are emitted by
     * cg_emit_specialization when first encountered via cg_collect_expr. */
    if (sd->u.struct_def.generic_params.len > 0) return;

    cg_emit_linemarker(ctx, sd->loc);
    const char *sname = sd->u.struct_def.name;
    /* S2: same emitter handles structs and unions; the keyword is the
     * only thing that differs.  No `typedef union` forward decl is
     * emitted because unions can be self-referencing only via pointer
     * (same restriction as struct), so the symbol-level `union T` form
     * already covers forward-references inside the body. */
    const char *kw = struct_kw(sd);

    /* Skip the local `typedef ... X;` if the same name is also bound
     * to a function or variable in this scope (see cg_type TY_STRUCT
     * branch -- collision case emits `struct X` instead of plain `X`).
     * The pass-1 forward decl emitted `struct X;` rather than a
     * typedef in this case, so just emit the body. */
    if (ctx->file_scope) {
        Symbol *sym = scope_lookup_local(ctx->file_scope, sname);
        for (; sym; sym = sym->next) {
            if (strcmp(sym->name, sname) != 0) continue;
            if (sym->kind == SYM_FUNC || sym->kind == SYM_VAR)
                break;
        }
    }

    /* If this AST_STRUCT_DEF has no body (a `struct X;` forward decl
     * in the source), skip the entire body emission -- the pass-1
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

    /* Struct/class body */
    const char *emit_name = sname;
    /* class: emit typedef so bare name works inside the function body */
    if (sd->u.struct_def.is_class && emit_name)
        cg_printf(ctx, "typedef %s %s %s;\n", kw, emit_name, emit_name);
    cg_printf(ctx, "%s %s {\n", kw, emit_name);
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        AstNode *fd = sd->u.struct_def.fields.data[i];
        if (fd->u.field_decl.is_static) continue;
        cg_puts(ctx, "    ");
        cg_field_decl_from_ast(ctx, fd);
    }
    /* Emit trailing attrs (from 'struct { ... } __attribute__((packed))').
     * Also fall back to leading_attrs for backward compatibility (when
     * the attribute appeared after the struct keyword before the tag).
     * If both exist, output both space-separated. */
    if (sd->u.struct_def.tail_attrs || sd->u.struct_def.leading_attrs) {
        cg_puts(ctx, "} ");
        if (sd->u.struct_def.leading_attrs)
            cg_puts(ctx, sd->u.struct_def.leading_attrs);
        if (sd->u.struct_def.tail_attrs && sd->u.struct_def.leading_attrs)
            cg_puts(ctx, " ");
        if (sd->u.struct_def.tail_attrs)
            cg_puts(ctx, sd->u.struct_def.tail_attrs);
        cg_puts(ctx, ";\n\n");
    } else {
        cg_puts(ctx, "};\n\n");
    }

    /* Static member globals (file-scope C static variables) */
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        AstNode *fd = sd->u.struct_def.fields.data[i];
        if (!fd->u.field_decl.is_static) continue;
        const char *fname = fd->u.field_decl.name;
        if (!fname) continue;
        cg_printf(ctx, "static ");
        cg_type_from_ast(ctx, fd->u.field_decl.type);
        cg_printf(ctx, " %s__%s;\n", emit_name, fname);
    }
}

/* =========================================================================
 * Phase 11: Generic monomorphization
 * ====================================================================== */

/* Mangle a Type* to a malloc'd string (used for operator overloading
 * name construction where we need the full type, not just the kind). */
static char *cg_mangle_type_str(Type *t) {
    StrBuf sb = {0};
    ty_mangle(&sb, t);
    return sb_take(&sb);
}

static char *cg_mangle_inst(const char *sname, Type **args, size_t nargs) {
    StrBuf sb = {0};
    sb_push_cstr(&sb, sname);
    for (size_t i = 0; i < nargs; i++) {
        sb_push_cstr(&sb, "__");
        ty_mangle(&sb, args[i]);
    }
    return sb_take(&sb);
}

/* subst_type removed — its logic is identical to ty_subst() in type.c,
 * which is accessible via the cg.h → sema.h → type.h include chain. */

/* Phase M: forward declaration -- defined in the file-level section. */

static const AstNode *find_generic_struct(CgCtx *ctx, const char *name) {
    const AstNode *file = ctx->file_ast;
    if (file) {
        for (size_t i = 0; i < file->u.file.decls.len; i++) {
            const AstNode *d = file->u.file.decls.data[i];
            if (d && d->kind == AST_STRUCT_DEF &&
                strcmp(d->u.struct_def.name, name) == 0 &&
                d->u.struct_def.generic_params.len > 0)
                return d;
        }
    }
    Symbol *sym = scope_lookup(ctx->file_scope, name);
    if (sym && sym->decl && sym->decl->kind == AST_STRUCT_DEF &&
        sym->decl->u.struct_def.generic_params.len > 0)
        return sym->decl;
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

static void cg_emit_spec_method_impl(CgCtx *ctx, const AstNode *fn,
                                  const char *msname,
                                  const char **pnames, Type **pvals, size_t np,
                                  bool fwd_only) {
    if (!fn || fn->kind != AST_FUNC_DEF) return;
    cg_emit_linemarker(ctx, fn->loc);

    /* Install gp context so cg_type_from_ast resolves T in body (sizeof(T) etc.) */
    const char **saved_pnames = ctx->gp_names;
    Type       **saved_pvals  = ctx->gp_vals;
    size_t       saved_np     = ctx->ngp;
    Scope       *saved_spec_scope = ctx->spec_scope;
    ctx->gp_names = pnames;
    ctx->gp_vals  = pvals;
    ctx->ngp      = np;
    /* v0.13: install spec_scope and pre-populate body type_ref so per-
     * instance @-eval and method/operator dispatch work for generic
     * struct methods (same machinery used by cg_emit_spec_func). */
    Scope *mscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;
    ctx->spec_scope = mscope;
    if (fn->u.func_def.body)
        sema_func_template_body(ctx->ts, mscope, NULL, (AstNode*)fn);

    /* Return type via AST (preserves T substitution through gp_names) */
    cg_puts(ctx, "__attribute__((weak)) ");
    if (fn->u.func_def.ret_type)
        cg_type_from_ast(ctx, fn->u.func_def.ret_type);
    else
        cg_puts(ctx, "void");
    cg_puts(ctx, " ");
    cg_method_name(ctx, msname, fn->u.func_def.name);
    cg_puts(ctx, "(");
    bool first = true;
    /* Mirror cg_func: omit 'this' for associated functions (body never uses this).
     * Extension methods with has_receiver already have 'this' as a parameter. */
    bool spec_is_assoc = !cg_fn_uses_this(fn) || fn->u.func_def.has_receiver;
    if (!spec_is_assoc) {
        if (fn->u.func_def.is_const_method) {
            cg_puts(ctx, "const "); cg_emit_struct_type(ctx, msname); cg_puts(ctx, " *this");
        } else {
            cg_emit_struct_type(ctx, msname); cg_puts(ctx, " *this");
        }
        first = false;
    }
    for (size_t i = 0; i < fn->u.func_def.params.len; i++) {
        AstNode *p = fn->u.func_def.params.data[i];
        if (!p || p->u.param_decl.is_vararg) continue;
        if (!first) cg_puts(ctx, ", ");
        first = false;
        if (p->u.param_decl.type && p->u.param_decl.name) {
            if (!cg_decl_ast(ctx, p->u.param_decl.type, p->u.param_decl.name)) {
                Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type,
                                       cg_type_scope(ctx), NULL);
                pt = ty_subst(ctx->ts, pt, pnames, pvals, np);
                cg_decl(ctx, pt, p->u.param_decl.name);
            }
        }
    }
    if (first) {
        if (!fn->u.func_def.params_unspecified)
            cg_puts(ctx, "void");
    }
    cg_puts(ctx, ")");
    if (fwd_only) {
        cg_puts(ctx, ";"); cg_nl(ctx);
        ctx->gp_names = saved_pnames; ctx->gp_vals = saved_pvals; ctx->ngp = saved_np;
        ctx->spec_scope = saved_spec_scope;
        return;
    }
    if (!fn->u.func_def.body) {
        if (fn->u.func_def.trailing_gcc_attrs) {
            cg_puts(ctx, " ");
            cg_puts(ctx, fn->u.func_def.trailing_gcc_attrs);
        }
        cg_puts(ctx, ";"); cg_nl(ctx);
        ctx->gp_names = saved_pnames; ctx->gp_vals = saved_pvals; ctx->ngp = saved_np;
        ctx->spec_scope = saved_spec_scope;
        return;
    }
    cg_puts(ctx, " ");
    cg_block(ctx, fn->u.func_def.body);
    ctx->gp_names = saved_pnames; ctx->gp_vals = saved_pvals; ctx->ngp = saved_np;
    ctx->spec_scope = saved_spec_scope;
}

static void cg_expr_p(CgCtx *ctx, const AstNode *expr, int min_prec) {
    /* Wrap expr in parens if its own operator precedence < min_prec. */
    bool need_parens = false;
    if (expr && expr->kind == AST_BINOP)
        need_parens = binop_prec(expr->u.binop.op) < min_prec;
    else if (expr && expr->kind == AST_TERNARY)
        need_parens = (3 < min_prec);  /* ternary has prec 3 */
    if (need_parens) cg_puts(ctx, "(");
    cg_expr(ctx, expr);
    if (need_parens) cg_puts(ctx, ")");
}

static void cg_emit_specialization(CgCtx *ctx, Type *t);
static void cg_collect_type(CgCtx *ctx, Type *t);
static void cg_collect_block(CgCtx *ctx, const AstNode *block);
static void cg_collect_stmt(CgCtx *ctx, const AstNode *stmt);
static void cg_collect_expr(CgCtx *ctx, const AstNode *expr);
static void cg_collect_gfunc_call(CgCtx *ctx, const AstNode *call_expr);
static void gfinst_push(CgCtx *ctx, const AstNode *fn, const char *mn,
                        Type **targs, size_t np);
static bool cg_gfunc_seen(CgCtx *ctx, const char *mn);

static void cg_emit_specialization(CgCtx *ctx, Type *t) {
    if (!t || t->kind != TY_STRUCT || t->u.struct_.nargs == 0) return;
    const char *sname = t->u.struct_.name;
    char *mn = cg_mangle_inst(sname, t->u.struct_.args, t->u.struct_.nargs);
    if (cg_inst_seen(ctx, mn)) { free(mn); return; }

    const AstNode *sd = find_generic_struct(ctx, sname);
    if (!sd) {
        cg_printf(ctx, "typedef struct %s %s;\n", mn, mn);
        free(mn); return;
    }
    cg_emit_linemarker(ctx, sd->loc);
    size_t np = sd->u.struct_def.generic_params.len;
    if (np != t->u.struct_.nargs) { free(mn); return; }

    const char **pnames = malloc(np * sizeof *pnames);
    if (!pnames) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = sd->u.struct_def.generic_params.data[i]->u.type_param.name;

    /* Install substitution context so cg_type resolves TY_PARAM correctly. */
    const char **saved_pnames = ctx->gp_names;
    Type       **saved_pvals  = ctx->gp_vals;
    size_t       saved_np     = ctx->ngp;
    ctx->gp_names = pnames;
    ctx->gp_vals  = (Type **)t->u.struct_.args;
    ctx->ngp      = np;

    /* v0.13: before emitting THIS struct, recursively emit specializations
     * for any generic types referenced by its fields.  Without this step,
     * `struct HashMap<K,V> { HashMapEntry<K,V>* entries; }` instantiated as
     * <Point,int> emits the HashMap__Point__int typedef but never the
     * HashMapEntry__Point__int it depends on.
     *
     * Also walk method return types and parameter types -- methods like
     * `iter() -> HashMapCursor<K,V>` would otherwise emit a forward decl
     * referencing an undeclared HashMapCursor__Point__int.
     *
     * Resolve types in the struct's own scope (so K and V are visible
     * as TY_PARAMs), then substitute against the concrete instantiation
     * args, then collect -- which triggers nested specializations. */
    Scope *struct_scope = sd->sem_scope ? (Scope*)sd->sem_scope : cg_type_scope(ctx);
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        const AstNode *fd = sd->u.struct_def.fields.data[i];
        if (!fd || !fd->u.field_decl.type) continue;
        Type *ft = ty_from_ast(ctx->ts, fd->u.field_decl.type,
                               struct_scope, NULL);
        if (!ft) continue;
        ft = ty_subst(ctx->ts, ft, pnames, (Type **)t->u.struct_.args, np);
        cg_collect_type(ctx, ft);
    }
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        const AstNode *mfn = sd->u.struct_def.methods.data[i];
        if (!mfn || mfn->kind != AST_FUNC_DEF) continue;
        Scope *mscope = mfn->sem_scope ? (Scope*)mfn->sem_scope : struct_scope;
        if (mfn->u.func_def.ret_type) {
            Type *rt = ty_from_ast(ctx->ts, mfn->u.func_def.ret_type,
                                   mscope, NULL);
            if (rt) {
                rt = ty_subst(ctx->ts, rt, pnames,
                              (Type **)t->u.struct_.args, np);
                cg_collect_type(ctx, rt);
            }
        }
        for (size_t pi = 0; pi < mfn->u.func_def.params.len; pi++) {
            const AstNode *p = mfn->u.func_def.params.data[pi];
            if (!p || !p->u.param_decl.type) continue;
            Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type,
                                   mscope, NULL);
            if (!pt) continue;
            pt = ty_subst(ctx->ts, pt, pnames,
                          (Type **)t->u.struct_.args, np);
            cg_collect_type(ctx, pt);
        }
    }

    cg_printf(ctx, "typedef struct %s %s;\n", mn, mn);
    cg_printf(ctx, "struct %s {\n", mn);
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        const AstNode *fd = sd->u.struct_def.fields.data[i];
        cg_puts(ctx, "    ");
        /* Use AST-based type emission: cg_type_from_ast handles AST_TYPE_PARAM
         * via gp_names/gp_vals, and AST_TYPE_GENERIC via recursive specialization. */
        if (!cg_decl_ast(ctx, fd->u.field_decl.type, fd->u.field_decl.name)) {
            /* Fallback: use Type* path with substitution */
            Type *ft = ty_from_ast(ctx->ts, fd->u.field_decl.type,
                                   cg_type_scope(ctx), NULL);
            ft = ty_subst(ctx->ts, ft, pnames, (Type **)t->u.struct_.args, np);
            cg_decl(ctx, ft, fd->u.field_decl.name);
        }
        cg_puts(ctx, ";\n");
    }
    cg_printf(ctx, "};\n\n");
    /* v0.13: two passes for method emission.  Pass 1 emits all method
     * prototypes so that bodies can forward-call each other (e.g. insert()
     * calls _grow_if_needed() which is defined later).  Pass 2 emits the
     * actual bodies. */
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        cg_emit_spec_method_impl(ctx, sd->u.struct_def.methods.data[i],
                             mn,
                             pnames, (Type **)t->u.struct_.args, np,
                             /*fwd_only=*/true);
    }
    cg_nl(ctx);
    for (size_t i = 0; i < sd->u.struct_def.methods.len; i++) {
        cg_emit_spec_method_impl(ctx, sd->u.struct_def.methods.data[i],
                             mn,
                             pnames, (Type **)t->u.struct_.args, np,
                             /*fwd_only=*/false);
        cg_nl(ctx);
    }

    /* Restore substitution context. */
    ctx->gp_names = saved_pnames;
    ctx->gp_vals  = saved_pvals;
    ctx->ngp      = saved_np;

    free(pnames); free(mn);
}

static bool cg_type_has_params(Type *t) {
    /* Return true if t contains any unresolved TY_PARAM -- indicates this is
     * still a template-internal reference, not a concrete instantiation. */
    if (!t) return false;
    switch (t->kind) {
    case TY_PARAM:  return true;
    case TY_PTR:    return cg_type_has_params(t->u.ptr.base);
    case TY_CONST:  return cg_type_has_params(t->u.const_.base);
    case TY_ATOMIC: return cg_type_has_params(t->u.atomic.base);
    case TY_ARRAY:  return cg_type_has_params(t->u.array.base);
    case TY_FUNC: {
        if (cg_type_has_params(t->u.func.ret)) return true;
        for (size_t i = 0; i < t->u.func.nparams; i++)
            if (cg_type_has_params(t->u.func.params[i])) return true;
        return false;
    }
    case TY_STRUCT:
        for (size_t _i = 0; _i < t->u.struct_.nargs; _i++)
            if (cg_type_has_params(t->u.struct_.args[_i])) return true;
        return false;
    case TY_ENUM:   return false;
    case TY_VECTOR: return cg_type_has_params(t->u.vector.elem);
    default: return false;
    }
}

static void cg_collect_type(CgCtx *ctx, Type *t) {
    if (!t) return;
    switch (t->kind) {
    case TY_STRUCT:
        if (t->u.struct_.nargs > 0) {
            /* Skip if any arg is still a TY_PARAM -- this is a template-internal
             * reference (e.g. Stack<T> inside Stack's own method body).
             * Emitting Stack__T would cause an infinite loop. */
            bool has_params = false;
            for (size_t i = 0; i < t->u.struct_.nargs; i++) {
                if (cg_type_has_params(t->u.struct_.args[i])) { has_params = true; break; }
            }
            if (!has_params) {
                for (size_t i = 0; i < t->u.struct_.nargs; i++)
                    cg_collect_type(ctx, t->u.struct_.args[i]);
                cg_emit_specialization(ctx, t);
            }
        }
        break;
    case TY_PTR:    cg_collect_type(ctx, t->u.ptr.base);    break;
    case TY_CONST:  cg_collect_type(ctx, t->u.const_.base); break;
    case TY_ATOMIC: cg_collect_type(ctx, t->u.atomic.base); break;
    case TY_ARRAY:  cg_collect_type(ctx, t->u.array.base);  break;
    case TY_FUNC: {
        cg_collect_type(ctx, t->u.func.ret);
        for (size_t i = 0; i < t->u.func.nparams; i++)
            cg_collect_type(ctx, t->u.func.params[i]);
        break;
    }
    case TY_ENUM:   break;
    case TY_VECTOR: cg_collect_type(ctx, t->u.vector.elem); break;
    default: break;
    }
}

static void cg_collect_expr(CgCtx *ctx, const AstNode *expr) {
    if (!expr) return;
    if (expr->sem_type) cg_collect_type(ctx, expr->sem_type);
    switch (expr->kind) {
    case AST_BINOP:
        cg_collect_expr(ctx, expr->u.binop.lhs);
        cg_collect_expr(ctx, expr->u.binop.rhs);
        /* Collect generic operator overloads (extension methods) on
         * struct values, e.g.  v1 == v2  where v1: Vec<int> uses
         * Vec<T>.operator==.  Without this, cg_emit_gfunc_fwd_decls
         * never emits the forward declaration. */
        {
            Type *lt = sema_type_of(expr->u.binop.lhs);
            lt = cg_resolve_type(ctx, lt);
            Type *lt_unc = ty_unconst(ctx->ts, lt);
            if (!lt_unc || lt_unc->kind != TY_STRUCT) break;
            const char *sn_raw = lt_unc->u.struct_.name;
            if (!sn_raw || lt_unc->u.struct_.nargs == 0 || !ctx->file_scope)
                break;

            /* Check if the operator is defined inside the struct body
             * (not an extension method). */
            Scope *ss_s = cg_struct_scope(ctx, sn_raw);
            char opname[64];
            snprintf(opname, sizeof opname, "operator%s",
                     binop_str(expr->u.binop.op));
            if (ss_s && scope_lookup_local(ss_s, opname))
                break;   /* inline operator — nothing to collect */

            /* Search for extension operator in file scope. */
            size_t snr_len = strlen(sn_raw);
            for (size_t b = 0; b < ctx->file_scope->nbuckets; b++) {
                for (Symbol *es = ctx->file_scope->buckets[b];
                     es; es = es->next) {
                    if (es->kind != SYM_FUNC || !es->decl) continue;
                    if (es->decl->kind != AST_FUNC_DEF) continue;
                    if (!es->decl->u.func_def.is_operator) continue;
                    if (!es->decl->u.func_def.struct_name) continue;
                    if (strcmp(es->decl->u.func_def.name, opname) != 0) continue;
                    const char *sn = es->decl->u.func_def.struct_name;
                    size_t snl = strlen(sn);
                    if (strcmp(sn, sn_raw) != 0 &&
                        !(snl < snr_len && strncmp(sn, sn_raw, snl) == 0 &&
                          sn_raw[snl] == '_' && sn_raw[snl+1] == '_'))
                        continue;

                    const AstNode *efn = es->decl;
                    size_t np = efn->u.func_def.generic_params.len;
                    np = (size_t)cg_resolve_generic_param_count(ctx, (int)np, (int)lt_unc->u.struct_.nargs, sn_raw);
                    if (np == 0) continue;

                    const char **pnames = malloc(np * sizeof *pnames);
                    Type **pvals = calloc(np, sizeof *pvals);
                    if (!pnames || !pvals) {
                        free(pnames); free(pvals); continue;
                    }

                    cg_fill_pnames_from_struct(ctx, sn_raw, pnames, (int)np);

                    size_t nargs = lt_unc->u.struct_.nargs;
                    for (size_t i = 0; i < np && i < nargs; i++)
                        if (lt_unc->u.struct_.args[i] &&
                            !cg_type_has_params(lt_unc->u.struct_.args[i]))
                            pvals[i] = lt_unc->u.struct_.args[i];

                    bool all = true;
                    for (size_t i = 0; i < np; i++)
                        if (!pvals[i]) { all = false; break; }

                    if (all) {
                        char *inst_sn = cg_mangle_inst(sn_raw,
                            lt_unc->u.struct_.args, nargs);
                        const char *eff_sn = inst_sn ? inst_sn : sn_raw;
                        StrBuf sb = {0};
                        sb_push_cstr(&sb, eff_sn);
                        sb_push_cstr(&sb, "__op_");
                        sb_push_cstr(&sb, op_suffix(binop_str(
                                                  expr->u.binop.op)));
                        char *mn = sb_take(&sb);
                        free(inst_sn);
                        if (!cg_gfunc_seen(ctx, mn))
                            gfinst_push(ctx, efn, mn, pvals, np);
                        free(mn);
                    }
                    free(pnames);
                    free(pvals);
                    break;
                }
            }
        }
        break;
    case AST_UNARY:
        cg_collect_expr(ctx, expr->u.unary.operand);
        break;
    case AST_CAST:
        cg_collect_expr(ctx, expr->u.cast.operand);
        break;
    case AST_PAREN:    /* C8 */
        cg_collect_expr(ctx, expr->u.paren.inner);
        break;
    case AST_CALL:
        cg_collect_gfunc_call(ctx, expr);  /* Phase 11: collect generic fn calls */
        cg_collect_expr(ctx, expr->u.call.callee);
        for (size_t i = 0; i < expr->u.call.args.len; i++)
            cg_collect_expr(ctx, expr->u.call.args.data[i]);
        break;
    case AST_GENERIC_CALL:  /* Phase G */
        cg_collect_gfunc_call(ctx, expr);  /* registers the specialization */
        for (size_t i = 0; i < expr->u.generic_call.call_args.len; i++)
            cg_collect_expr(ctx, expr->u.generic_call.call_args.data[i]);
        break;
    case AST_METHOD_CALL: {
        /* Phase 11: collect generic extension method calls.
         * e.g. s.push(42) where s: Stack<int> → instantiate Stack.push<T=int> */
        Type *recv_t = expr->u.method_call.recv->sem_type;
        if (recv_t) {
            Type *st = ty_peel_to_struct(recv_t);
            if (st && st->u.struct_.name) {
                const char *sname = st->u.struct_.name;
                const char *mname = expr->u.method_call.method;
                if (ctx->file_scope) {
                    Scope *fs = ctx->file_scope;
                    for (size_t b = 0; b < fs->nbuckets; b++) {
                        for (Symbol *es = fs->buckets[b]; es; es = es->next) {
                            if (es->kind != SYM_FUNC || !es->decl) continue;
                            if (es->decl->kind != AST_FUNC_DEF) continue;
                            const AstNode *efn = es->decl;
                            if (!efn->u.func_def.struct_name) continue;
                            if (strcmp(efn->u.func_def.struct_name, sname) != 0) continue;
                            if (strcmp(efn->u.func_def.name, mname) != 0) continue;
                            size_t np = efn->u.func_def.generic_params.len;
                            /* Extension methods use the struct's generic params,
                             * not their own (generic_params is typically 0). */
                            np = (size_t)cg_resolve_generic_param_count(ctx, (int)np, (int)st->u.struct_.nargs, sname);
                            if (np == 0) continue;
                            const char **pnames = malloc(np * sizeof *pnames);
                            Type **pvals = calloc(np, sizeof *pvals);
                            if (!pnames || !pvals) { free(pnames); free(pvals); continue; }
                            if (efn->u.func_def.generic_params.len > 0) {
                                for (size_t k = 0; k < np; k++)
                                    pnames[k] = efn->u.func_def.generic_params.data[k]
                                                      ->u.generic_param.name;
                            } else {
                                cg_fill_pnames_from_struct(ctx, sname, pnames, (int)np);
                            }
                            for (size_t k = 0; k < np && k < st->u.struct_.nargs; k++) {
                                Type *arg = st->u.struct_.args[k];
                                /* Resolve TY_PARAM through the active
                                 * substitution context.  When walking a
                                 * generic extension method body (e.g.
                                 * insert<K,V> calling this->_grow_if_needed()),
                                 * the receiver's struct args are TY_PARAMs.
                                 * Without resolution we would register the
                                 * nested method with TY_PARAM args instead
                                 * of concrete types. */
                                if (arg && arg->kind == TY_PARAM &&
                                    ctx->gp_names && ctx->gp_vals) {
                                    for (size_t g = 0; g < ctx->ngp; g++) {
                                        if (strcmp(arg->u.param.name,
                                                   ctx->gp_names[g]) == 0) {
                                            arg = ctx->gp_vals[g];
                                            break;
                                        }
                                    }
                                }
                                pvals[k] = arg;
                            }
                            /* Build a temp resolved-args array for
                             * cg_mangle_inst so it uses concrete types. */
                            Type **resolved_args = NULL;
                            if (st->u.struct_.nargs > 0 &&
                                np == st->u.struct_.nargs) {
                                resolved_args = pvals;  /* already resolved above */
                            }
                            StrBuf sb = {0};
                            /* Mangle struct name: HashMap<Point,int> → HashMap__Point__int */
                            char *inst_sn = st->u.struct_.nargs > 0
                                ? cg_mangle_inst(sname, resolved_args ? resolved_args : st->u.struct_.args, st->u.struct_.nargs)
                                : NULL;
                            const char *effective_sn = inst_sn ? inst_sn : sname;
                            sb_push_cstr(&sb, effective_sn);
                            sb_push_cstr(&sb, "__");
                            sb_push_cstr(&sb, efn->u.func_def.name);
                            /* Extension methods use the struct's type args;
                             * don't append method-level generic params here. */
                            char *mn = sb_take(&sb);
                            free(inst_sn);
                            if (!cg_gfunc_seen(ctx, mn))
                                gfinst_push(ctx, efn, mn, pvals, np);
                            free(mn);
                            free(pnames);
                            free(pvals);
                            break;
                        }
                    }
                }
            }
        }

        /* Associated function call: recv is a type, not a value.
         * e.g. Stack<int>.new() → recv: AST_CAST{, TYPE_GENERIC{Stack,int}}
         *      File.open()     → recv: AST_IDENT{File}
         * The receiver has no value-type sem_type so the method-call
         * collection above is skipped.  Register the specialization
         * manually so cg_emit_gfunc_fwd_decls/cg_emit_gfunc_specs
         * emit the forward-decl and definition. */
        {
            const AstNode *recv = expr->u.method_call.recv;
            const char *tname = NULL;
            Type **targs = NULL;
            size_t nargs = 0;

            if (recv->kind == AST_IDENT) {
                Symbol *s = ctx->file_scope
                    ? scope_lookup(ctx->file_scope, recv->u.ident.name) : NULL;
                if (s && s->kind == SYM_TYPE)
                    tname = recv->u.ident.name;
            } else if (recv->kind == AST_CAST &&
                       recv->u.cast.type &&
                       recv->u.cast.type->kind == AST_TYPE_GENERIC &&
                       !recv->u.cast.operand) {
                tname = recv->u.cast.type->u.type_generic.name;
                nargs = recv->u.cast.type->u.type_generic.args.len;
                if (nargs > 0) {
                    targs = calloc(nargs, sizeof *targs);
                    for (size_t i = 0; i < nargs; i++)
                        targs[i] = ty_from_ast(ctx->ts,
                            recv->u.cast.type->u.type_generic.args.data[i],
                            cg_type_scope(ctx), NULL);
                }
            }

            if (tname && ctx->file_scope) {
                const char *mname = expr->u.method_call.method;
                Scope *fs = ctx->file_scope;
                for (size_t b = 0; b < fs->nbuckets; b++) {
                    for (Symbol *es = fs->buckets[b]; es;
                         es = es->next) {
                        if (es->kind != SYM_FUNC || !es->decl) continue;
                        if (es->decl->kind != AST_FUNC_DEF) continue;
                        const AstNode *efn = es->decl;
                        if (!efn->u.func_def.struct_name) continue;
                        if (strcmp(efn->u.func_def.struct_name, tname) != 0)
                            continue;
                        if (strcmp(efn->u.func_def.name, mname) != 0)
                            continue;

                        size_t np = efn->u.func_def.generic_params.len;
                        np = (size_t)cg_resolve_generic_param_count(ctx, (int)np, (int)nargs, tname);
                        if (np == 0) continue; /* non-generic */

                        const char **pnames = malloc(np * sizeof *pnames);
                        Type **pvals = calloc(np, sizeof *pvals);
                        if (!pnames || !pvals) {
                            free(pnames); free(pvals); continue;
                        }
                        cg_fill_pnames_from_struct(ctx, tname, pnames, (int)np);
                        for (size_t i = 0; i < np && i < nargs; i++)
                            if (targs[i] && !cg_type_has_params(targs[i]))
                                pvals[i] = targs[i];

                        bool all = true;
                        for (size_t i = 0; i < np; i++)
                            if (!pvals[i]) { all = false; break; }

                        if (all) {
                            /* Mangle: Struct__T1__T2__method
                             * e.g. Stack__int__new */
                            char *inst_sn = (nargs > 0)
                                ? cg_mangle_inst(tname, targs, nargs)
                                : NULL;
                            const char *effective_sn = inst_sn ? inst_sn : tname;
                            StrBuf sb = {0};
                            sb_push_cstr(&sb, effective_sn);
                            sb_push_cstr(&sb, "__");
                            sb_push_cstr(&sb, efn->u.func_def.name);
                            char *mn = sb_take(&sb);
                            free(inst_sn);
                            if (!cg_gfunc_seen(ctx, mn))
                                gfinst_push(ctx, efn, mn, pvals, np);
                            free(mn);
                        }
                        free(pnames);
                        free(pvals);
                        break;
                    }
                }
            }
            free(targs);
        }
        cg_collect_expr(ctx, expr->u.method_call.recv);
        for (size_t i = 0; i < expr->u.method_call.args.len; i++)
            cg_collect_expr(ctx, expr->u.method_call.args.data[i]);
        break;
    }
    case AST_INDEX: {
        cg_collect_expr(ctx, expr->u.index_.base);
        cg_collect_expr(ctx, expr->u.index_.index);

        /* If the base is a generic struct type with an extension operator[],
         * register the specialization so cg_emit_gfunc_fwd_decls /
         * cg_emit_gfunc_specs emit the forward decl and definition. */
        Type *base_t = sema_type_of(expr->u.index_.base);
        base_t = cg_resolve_type(ctx, base_t);
        Type *base_unc = ty_unconst(ctx->ts, base_t);
        const char *sn_raw = (base_unc && base_unc->kind == TY_STRUCT)
                         ? base_unc->u.struct_.name : NULL;
        if (!sn_raw || base_unc->u.struct_.nargs == 0 || !ctx->file_scope)
            break;

        /* Check if operator[] is defined inside the struct body
         * (not an extension method).  If so, nothing to collect. */
        Scope *ss_s = cg_struct_scope(ctx, sn_raw);
        Symbol *osym = ss_s ? scope_lookup_local(ss_s, "operator[]") : NULL;
        if (osym) break;   /* inline operator[] — nothing to collect */

        /* Search for extension operator[] in file scope. */
        size_t snr_len = strlen(sn_raw);
        for (size_t b = 0; b < ctx->file_scope->nbuckets; b++) {
            for (Symbol *es = ctx->file_scope->buckets[b];
                 es; es = es->next) {
            if (es->kind != SYM_FUNC || !es->decl) continue;
            if (es->decl->kind != AST_FUNC_DEF) continue;
            if (!es->decl->u.func_def.is_operator) continue;
            if (!es->decl->u.func_def.struct_name) continue;
            if (strcmp(es->decl->u.func_def.name, "operator[]") != 0) continue;
            const char *sn = es->decl->u.func_def.struct_name;
            size_t snl = strlen(sn);
            if (strcmp(sn, sn_raw) != 0 &&
                !(snl < snr_len && strncmp(sn, sn_raw, snl) == 0 &&
                  sn_raw[snl] == '_' && sn_raw[snl+1] == '_'))
                continue;

            const AstNode *efn = es->decl;
            size_t np = efn->u.func_def.generic_params.len;
            np = (size_t)cg_resolve_generic_param_count(ctx, (int)np, (int)base_unc->u.struct_.nargs, sn_raw);
            if (np == 0) continue;

            const char **pnames = malloc(np * sizeof *pnames);
            Type **pvals = calloc(np, sizeof *pvals);
            if (!pnames || !pvals) { free(pnames); free(pvals); break; }

            cg_fill_pnames_from_struct(ctx, sn_raw, pnames, (int)np);

            size_t nargs = base_unc->u.struct_.nargs;
            for (size_t i = 0; i < np && i < nargs; i++)
                if (base_unc->u.struct_.args[i] &&
                    !cg_type_has_params(base_unc->u.struct_.args[i]))
                    pvals[i] = base_unc->u.struct_.args[i];

            bool all = true;
            for (size_t i = 0; i < np; i++)
                if (!pvals[i]) { all = false; break; }

            if (all) {
                char *inst_sn = cg_mangle_inst(sn_raw,
                    base_unc->u.struct_.args, nargs);
                const char *eff_sn = inst_sn ? inst_sn : sn_raw;
                StrBuf sb = {0};
                sb_push_cstr(&sb, eff_sn);
                sb_push_cstr(&sb, "__op_idx");
                char *mn = sb_take(&sb);
                free(inst_sn);
                if (!cg_gfunc_seen(ctx, mn))
                    gfinst_push(ctx, efn, mn, pvals, np);
                free(mn);
            }
            free(pnames);
            free(pvals);
            break;
        }
        }
        break;
    }
    case AST_TERNARY:
        cg_collect_expr(ctx, expr->u.ternary.cond);
        cg_collect_expr(ctx, expr->u.ternary.then_);
        cg_collect_expr(ctx, expr->u.ternary.else_);
        break;
    case AST_COMMA:
        cg_collect_expr(ctx, expr->u.comma.lhs);
        cg_collect_expr(ctx, expr->u.comma.rhs);
        break;
    case AST_FIELD_ACCESS:
        cg_collect_expr(ctx, expr->u.field_access.recv);
        break;
    case AST_SIZEOF:
        cg_collect_expr(ctx, expr->u.sizeof_.operand);
        break;
    case AST_STRUCT_LIT:
        for (size_t i = 0; i < expr->u.struct_lit.field_vals.len; i++)
            cg_collect_expr(ctx, expr->u.struct_lit.field_vals.data[i]);
        break;
    case AST_INIT_LIST:
        for (size_t i = 0; i < expr->u.init_list.items.len; i++)
            cg_collect_expr(ctx, expr->u.init_list.items.data[i]);
        break;
    case AST_DESIGNATED_INIT:
        cg_collect_expr(ctx, expr->u.designated_init.value);
        break;
    case AST_COMPOUND_LIT:
        cg_collect_expr(ctx, expr->u.compound_lit.init);
        break;
    case AST_AT_INTRINSIC:
        for (size_t i = 0; i < expr->u.at_intrinsic.args.len; i++)
            cg_collect_expr(ctx, expr->u.at_intrinsic.args.data[i]);
        break;
    case AST_STMT_EXPR:
        cg_collect_block(ctx, expr->u.stmt_expr.block);
        break;
    case AST_GENERIC_EXPR:
        cg_collect_expr(ctx, expr->u.generic_expr.controlling);
        for (size_t i = 0; i < expr->u.generic_expr.associations.len; i++)
            cg_collect_expr(ctx, expr->u.generic_expr.associations.data[i]);
        break;
    case AST_GENERIC_ASSOC:
        cg_collect_expr(ctx, expr->u.generic_assoc.value);
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
            if (vd->sem_type) cg_collect_type(ctx, vd->sem_type);
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
        cg_collect_expr(ctx, stmt->u.for_.post);
        cg_collect_stmt(ctx, stmt->u.for_.body);
        break;
    case AST_DO_WHILE:
        cg_collect_expr(ctx, stmt->u.do_while.cond);
        cg_collect_stmt(ctx, stmt->u.do_while.body);
        break;
    case AST_DEFER:
        cg_collect_stmt(ctx, stmt->u.defer_.body);
        break;
    case AST_SWITCH:
        cg_collect_expr(ctx, stmt->u.switch_.cond);
        cg_collect_stmt(ctx, stmt->u.switch_.body);
        break;
    case AST_COMPUTED_GOTO:
        cg_collect_expr(ctx, stmt->u.computed_goto.target);
        break;
    case AST_CASE:
        cg_collect_expr(ctx, stmt->u.case_.value);
        break;
    case AST_LABEL:
        break;
    case AST_TYPEDEF_DECL: {
        if (stmt->u.typedef_decl.target && stmt->u.typedef_decl.target->sem_type)
            cg_collect_type(ctx, stmt->u.typedef_decl.target->sem_type);
        break;
    }
    case AST_STRUCT_DEF: {
        for (size_t i = 0; i < stmt->u.struct_def.fields.len; i++) {
            AstNode *f = stmt->u.struct_def.fields.data[i];
            if (f && f->u.field_decl.type && f->u.field_decl.type->sem_type)
                cg_collect_type(ctx, f->u.field_decl.type->sem_type);
        }
        break;
    }
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

/* Recursively extract TY_PARAM bindings from a type tree.
 * If formal contains TY_PARAM "T" at any depth and actual has a matching
 * concrete type at that depth, fill pvals for that parameter name. */
static void infer_ty_params(const char **pnames, Type **pvals, size_t np,
                             Type *formal, Type *actual) {
    if (!formal || !actual) return;
    /* Simple: formal is TY_PARAM "T" */
    if (formal->kind == TY_PARAM) {
        for (size_t k = 0; k < np; k++) {
            if (strcmp(formal->u.param.name, pnames[k]) == 0 && !pvals[k])
                pvals[k] = actual;
        }
        return;
    }
    /* Pointer: descend into base */
    if (formal->kind == TY_PTR && actual->kind == TY_PTR) {
        infer_ty_params(pnames, pvals, np, formal->u.ptr.base, actual->u.ptr.base);
        return;
    }
    /* Const: skip qualifier */
    if (formal->kind == TY_CONST) {
        infer_ty_params(pnames, pvals, np, formal->u.const_.base, actual);
        return;
    }
    if (formal->kind == TY_ATOMIC) {
        infer_ty_params(pnames, pvals, np, formal->u.atomic.base, actual);
        return;
    }
    /* Struct: match generic args */
    if (formal->kind == TY_STRUCT && actual->kind == TY_STRUCT &&
        formal->u.struct_.nargs == actual->u.struct_.nargs) {
        for (size_t i = 0; i < formal->u.struct_.nargs; i++)
            infer_ty_params(pnames, pvals, np,
                           formal->u.struct_.args[i], actual->u.struct_.args[i]);
    }
    /* Array: descend into base */
    if (formal->kind == TY_ARRAY && actual->kind == TY_ARRAY) {
        infer_ty_params(pnames, pvals, np, formal->u.array.base, actual->u.array.base);
    }
    /* Func: match return and parameter types */
    if (formal->kind == TY_FUNC && actual->kind == TY_FUNC) {
        infer_ty_params(pnames, pvals, np, formal->u.func.ret, actual->u.func.ret);
        if (formal->u.func.nparams == actual->u.func.nparams) {
            for (size_t i = 0; i < formal->u.func.nparams; i++)
                infer_ty_params(pnames, pvals, np,
                               formal->u.func.params[i], actual->u.func.params[i]);
        }
    }
    /* Vector: descend into element type */
    if (formal->kind == TY_VECTOR && actual->kind == TY_VECTOR) {
        infer_ty_params(pnames, pvals, np, formal->u.vector.elem, actual->u.vector.elem);
    }
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
        Type *actual = act ? (Type*)act->sem_type : NULL;
        if (!formal || !actual) continue;
        infer_ty_params(pnames, pvals, np, formal, actual);
    }
}

/* Emit a specialized free-standing generic function. */
static void cg_emit_spec_func(CgCtx *ctx, const AstNode *fn,
                               const char *mname,
                               const char **pnames, Type **pvals, size_t np) {
    if (!fn || fn->kind != AST_FUNC_DEF || !fn->u.func_def.body) return;
    cg_emit_linemarker(ctx, fn->loc);
    Scope *fscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;

    /* v0.13: populate type_ref annotations on the template body so cg's
     * TY_PARAM resolution machinery (cg_resolve_type) and the per-instance
     * @-intrinsic re-eval have something to substitute against.
     * Template bodies are otherwise skipped by sema_func, so without this
     * step every expression in the body has NULL sem_type and operator /
     * method dispatch falls through to verbatim emission. */
    sema_func_template_body(ctx->ts, fscope, NULL, (AstNode*)fn);

    /* Save and install substitution context. */
    const char **saved_gp_names = ctx->gp_names;
    Type **saved_gp_vals = ctx->gp_vals;
    size_t saved_ngp = ctx->ngp;
    Scope *saved_spec_scope = ctx->spec_scope;

    ctx->gp_names  = pnames;
    ctx->gp_vals   = pvals;
    ctx->ngp       = np;
    ctx->spec_scope = fscope;

    /* Return type */
    Type *ret_t = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, fscope, NULL);
    ret_t = ty_subst(ctx->ts, ret_t, pnames, pvals, np);
    cg_puts(ctx, "__attribute__((weak)) ");
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
        pt = ty_subst(ctx->ts, pt, pnames, pvals, np);
        cg_decl(ctx, pt, p->u.param_decl.name);
    }
    if (first) cg_puts(ctx, "void");
    cg_puts(ctx, ") ");

    /* Body: cg_type inside will use gp_names/gp_vals for TY_PARAM. */
    ctx->cur_fn_ret = ret_t;

    cg_block(ctx, fn->u.func_def.body);
    ctx->cur_fn_ret = NULL;
    cg_nl(ctx);

    /* Restore substitution context. */
    ctx->gp_names  = saved_gp_names;
    ctx->gp_vals   = saved_gp_vals;
    ctx->ngp       = saved_ngp;
    ctx->spec_scope = saved_spec_scope;
}

/* =========================================================================
 * Phase G -- Per-ctx generic function instance list.
 * (Replaces the old g_gfinsts / g_ngfinsts / g_gfinsts_cap file-globals.)
 * ======================================================================== */

static void gfinst_push(CgCtx *ctx, const AstNode *fn, const char *mn,
                        Type **targs, size_t ntargs) {
    /* Dedup by (fn pointer, mangled name). */
    for (size_t i = 0; i < ctx->ngfinsts; i++) {
        if (ctx->gfinsts[i].fn == fn &&
            strcmp(ctx->gfinsts[i].mangle_name, mn) == 0)
            return;
    }
    if (ctx->ngfinsts == ctx->gfinsts_cap) {
        ctx->gfinsts_cap = ctx->gfinsts_cap ? ctx->gfinsts_cap * 2 : 8;
        ctx->gfinsts = realloc(ctx->gfinsts,
                               ctx->gfinsts_cap * sizeof *ctx->gfinsts);
        if (!ctx->gfinsts) abort();
    }
    Type **ta = NULL;
    if (ntargs) {
        ta = malloc(ntargs * sizeof *ta);
        if (!ta) abort();
        for (size_t i = 0; i < ntargs; i++) ta[i] = targs[i];
    }
    {
        size_t idx = ctx->ngfinsts++;
        ctx->gfinsts[idx].fn          = fn;
        ctx->gfinsts[idx].mangle_name = cpp_xstrdup(mn);
        ctx->gfinsts[idx].targs       = (Type **)ta;
        ctx->gfinsts[idx].ntargs      = ntargs;
        ctx->gfinsts[idx].walked      = false;
    }
}

/* Called from cg_collect_expr for AST_CALL and AST_GENERIC_CALL nodes.
 *
 * For AST_CALL:         callee is AST_IDENT (untyped generic call, e.g.
 *                       `swap(&a, &b)` where swap is generic -- type args
 *                       inferred from actual argument types).
 * For AST_GENERIC_CALL: type args are explicit in node->u.generic_call.
 *
 * In both cases we record the function instance in ctx->gfinsts so that
 * cg_emit_gfunc_fwd_decls / cg_emit_gfunc_specs can emit it at end-of-file.
 */
static void cg_collect_gfunc_call(CgCtx *ctx, const AstNode *call) {
    if (!call) return;

    const char *fname   = NULL;
    const AstNode *callee = NULL;
    /* Value arguments used for type inference */
    const AstNode **actual_args = NULL;
    size_t          nactual     = 0;

    if (call->kind == AST_CALL) {
        callee = call->u.call.callee;
        /* AST_CALL with IDENT callee -- untyped generic call */
        if (callee && callee->kind == AST_IDENT)
            fname = callee->u.ident.name;
        if (!fname) return;
        nactual     = call->u.call.args.len;
        actual_args = (nactual > 0)
            ? (const AstNode **)call->u.call.args.data : NULL;
    } else if (call->kind == AST_GENERIC_CALL) {
        /* Phase G: explicit type-arg call -- name is directly available */
        fname   = call->u.generic_call.name;
        nactual = call->u.generic_call.call_args.len;
        actual_args = (nactual > 0)
            ? (const AstNode **)call->u.generic_call.call_args.data : NULL;
    } else {
        return;
    }

    Symbol *sym = scope_lookup(ctx->file_scope, fname);
    if (!sym || sym->kind != SYM_FUNC || !sym->decl ||
        sym->decl->kind != AST_FUNC_DEF) return;
    const AstNode *fn = sym->decl;
    size_t np = fn->u.func_def.generic_params.len;
    if (np == 0) return;  /* not generic */

    Scope *fscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;

    /* Collect param names and infer concrete types from actual args. */
    const char **pnames = malloc(np * sizeof *pnames);
    Type       **pvals  = calloc(np, sizeof *pvals);
    if (!pnames || !pvals) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = fn->u.func_def.generic_params.data[i]->u.generic_param.name;

    infer_gfunc_args(fn, pnames, pvals, np,
                     actual_args, nactual, ctx->ts, fscope);

    /* Override with explicit type arguments when available.
     * For AST_GENERIC_CALL the type args are in generic_call.type_args.
     * For AST_CALL the old CAST{TYPE_GENERIC,NULL} override path is gone;
     * untyped calls rely solely on inference. */
    if (call->kind == AST_GENERIC_CALL) {
        size_t na = call->u.generic_call.type_args.len;
        if (na == np) {
            for (size_t i = 0; i < na; i++) {
                Type *et = ty_from_ast(ctx->ts,
                    call->u.generic_call.type_args.data[i],
                    cg_type_scope(ctx), NULL);
                if (et && !cg_type_has_params(et))
                    pvals[i] = et;
            }
        }
    }

    /* Check all params were inferred; skip if any are NULL. */
    for (size_t i = 0; i < np; i++) {
        if (!pvals[i]) {
            free(pnames); free(pvals);
            return;
        }
    }

    /* Build mangled name: funcname__T1__T2 */
    StrBuf sb = {0};
    sb_push_cstr(&sb, fn->u.func_def.name);
    for (size_t i = 0; i < np; i++) {
        sb_push_cstr(&sb, "__");
        ty_mangle(&sb, pvals[i]);
    }
    char *mn = sb_take(&sb);

    if (!cg_gfunc_seen(ctx, mn)) {
        gfinst_push(ctx, fn, mn, pvals, np);
    }

    free(mn);
    free(pnames);
    free(pvals);
}

/* Emit forward declarations for all collected generic function specializations
 * so that calls within function bodies (e.g. main calling swap__int) have
 * proper prototypes before the definition is emitted at end of file. */
static void cg_emit_gfunc_fwd_decls(CgCtx *ctx) {
    for (size_t i = 0; i < ctx->ngfinsts; i++) {
        struct GFuncInst_ *gi = &ctx->gfinsts[i];
        const AstNode *fn = gi->fn;
        if (!fn || fn->kind != AST_FUNC_DEF) continue;
        size_t np = fn->u.func_def.generic_params.len;
        Type **targs = gi->targs;
        /* Extension methods use struct's generic params, not their own. */
        if (np == 0) { np = gi->ntargs; if (!targs || np == 0) continue; }
        if (!targs) continue;
        cg_emit_linemarker(ctx, fn->loc);
        Scope *fscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;

        const char **pnames = malloc(np * sizeof *pnames);
        if (!pnames) abort();
        if (fn->u.func_def.generic_params.len > 0) {
            for (size_t k = 0; k < np; k++)
                pnames[k] = fn->u.func_def.generic_params.data[k]->u.generic_param.name;
        } else if (fn->u.func_def.struct_name && ctx->file_scope) {
            cg_fill_pnames_from_struct(ctx, fn->u.func_def.struct_name, pnames, (int)np);
        }

        ctx->gp_names  = pnames;
        ctx->gp_vals   = targs;
        ctx->ngp       = np;
        ctx->spec_scope = fscope;

        Type *ret_t = ty_from_ast(ctx->ts, fn->u.func_def.ret_type, fscope, NULL);
        ret_t = ty_subst(ctx->ts, ret_t, pnames, targs, np);
        cg_type(ctx, ret_t);
        cg_printf(ctx, " %s(", gi->mangle_name);

        bool first = true;
        for (size_t j = 0; j < fn->u.func_def.params.len; j++) {
            AstNode *p = fn->u.func_def.params.data[j];
            if (!p || p->u.param_decl.is_vararg) continue;
            if (!first) cg_puts(ctx, ", ");
            first = false;
            Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type, fscope, NULL);
            pt = ty_subst(ctx->ts, pt, pnames, targs, np);
            cg_decl(ctx, pt, p->u.param_decl.name);
        }
        if (first) cg_puts(ctx, "void");
        cg_puts(ctx, ");\n");

        ctx->gp_names  = NULL;
        ctx->gp_vals   = NULL;
        ctx->ngp       = 0;
        ctx->spec_scope = NULL;
        free(pnames);
    }
}

/* Emit all collected generic function specializations. */
static void cg_emit_gfunc_specs(CgCtx *ctx) {
    for (size_t i = 0; i < ctx->ngfinsts; i++) {
        struct GFuncInst_ *gi = &ctx->gfinsts[i];
        const AstNode *fn = gi->fn;
        size_t np = fn->u.func_def.generic_params.len;
        /* Extension methods use struct's generic params, not their own. */
        if (np == 0) { np = gi->ntargs; if (!gi->targs || np == 0) continue; }
        if (!gi->targs) continue;
        const char **pnames = malloc(np * sizeof *pnames);
        if (!pnames) abort();
        if (fn->u.func_def.generic_params.len > 0) {
            for (size_t k = 0; k < np; k++)
                pnames[k] = fn->u.func_def.generic_params.data[k]->u.generic_param.name;
        } else if (fn->u.func_def.struct_name && ctx->file_scope) {
            cg_fill_pnames_from_struct(ctx, fn->u.func_def.struct_name, pnames, (int)np);
        }
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
    Scope *fscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;
    const char **pnames = malloc(np * sizeof *pnames);
    Type       **pvals  = calloc(np, sizeof *pvals);
    if (!pnames || !pvals) abort();
    for (size_t i = 0; i < np; i++)
        pnames[i] = fn->u.func_def.generic_params.data[i]->u.generic_param.name;

    /* Build actual-argument array based on call node kind. */
    size_t nact = 0;
    const AstNode **aargs = NULL;
    if (call->kind == AST_GENERIC_CALL) {
        nact  = call->u.generic_call.call_args.len;
        aargs = (nact > 0)
            ? (const AstNode **)call->u.generic_call.call_args.data : NULL;
    } else if (call->kind == AST_CALL) {
        nact  = call->u.call.args.len;
        aargs = (nact > 0)
            ? (const AstNode **)call->u.call.args.data : NULL;
    }
    infer_gfunc_args(fn, pnames, pvals, np, aargs, nact, ctx->ts, fscope);

    /* Override with explicit type args when available. */
    if (call->kind == AST_GENERIC_CALL) {
        size_t na = call->u.generic_call.type_args.len;
        if (na == np) {
            for (size_t i = 0; i < na; i++) {
                Type *et = ty_from_ast(ctx->ts,
                    call->u.generic_call.type_args.data[i],
                    fscope, NULL);
                if (et && !cg_type_has_params(et))
                    pvals[i] = et;
            }
        }
    }

    StrBuf sb = {0};
    sb_push_cstr(&sb, fname);
    for (size_t i = 0; i < np; i++) {
        sb_push_cstr(&sb, "__");
        if (pvals[i]) ty_mangle(&sb, pvals[i]);
        else sb_push_cstr(&sb, "unk");
    }
    char *mn = sb_take(&sb);
    /* Store in ctx to avoid leaking -- we push it to gfn_names for lifetime */
    if (ctx->ngfn == ctx->gfn_cap) {
        ctx->gfn_cap = ctx->gfn_cap ? ctx->gfn_cap * 2 : 8;
        ctx->gfn_names = realloc(ctx->gfn_names,
            ctx->gfn_cap * sizeof *ctx->gfn_names);
        if (!ctx->gfn_names) abort();
    }
    ctx->gfn_names[ctx->ngfn++] = mn;
    free(pnames); free(pvals);
    return mn;
}
/* =========================================================================
 * File-level code generation
 * ====================================================================== */
/* =========================================================================
 * C mode: single-pass faithful AST printer (GCC -ast-print style)
 *
 * Philosophy: if the user wrote valid C, just print what they wrote.
 * No added preamble, no injected forward declarations, no reordering.
 * Exactly one loop over file.decls, in source order.
 *
 * This is what GCC/Clang do when printing C ASTs: a single traversal that
 * emits each declaration when it is encountered.  The multi-pass approach
 * in cg_file (Sharp mode) exists because Sharp adds constructs (forward
 * decls, preamble) that require pre-scanning.  C mode needs none of that.
 * ======================================================================== */

/* Emit one struct/union definition faithfully -- no extra typedef wrapper.
 * The user's source determines whether a typedef is needed; cg_typedef_c
 * handles the typedef case separately. */

/* Emit a struct field, inlining the body if the field type refers to a
 * named struct that was defined inline inside parent_sd (detected by
 * position in file.decls and loc.line).  Returns true if emitted inline. */
static bool cg_emit_field_maybe_inline(CgCtx *ctx,
                                        const AstNode *parent_sd,
                                        const AstVec  *fields,
                                        size_t         idx) {
    const AstNode *fi = fields->data[idx];
    if (!fi || fi->kind != AST_FIELD_DECL || fi->u.field_decl.is_comma_cont) return false;
    if (!ctx->file_ast || !parent_sd) return false;
    const AstNode *fty = fi->u.field_decl.type;
    if (!fty) return false;
    const AstNode *fty_base = fty;
    bool fty_is_ptr = false;
    if (fty_base->kind == AST_TYPE_PTR) {
        fty_base = fty_base->u.type_ptr.base;
        fty_is_ptr = true;
    }
    if (!fty_base || fty_base->kind != AST_TYPE_NAME) return false;
    const char *tname = fty_base->u.type_name.name;
    if (!tname || strncmp(tname, "__anon_", 7) == 0) return false;

    /* Find tname in file.decls */
    for (size_t _k = 0; _k < ctx->file_ast->u.file.decls.len; _k++) {
        const AstNode *_td = ctx->file_ast->u.file.decls.data[_k];
        if (!_td || _td->kind != AST_STRUCT_DEF) continue;
        if (!_td->u.struct_def.name || strcmp(_td->u.struct_def.name, tname) != 0) continue;
        if (!_td->u.struct_def.fields.len) break;
        if (!(_td->loc.file && parent_sd->loc.file &&
              strcmp(_td->loc.file, parent_sd->loc.file) == 0 &&
              _td->loc.line > parent_sd->loc.line)) break;
        /* Guard: only inline when _td is flushed BEFORE parent_sd in file.decls
         * (pending_decls writes inner structs first). */
        { bool _b=false,_p=false;
          for(size_t _ii=0;_ii<ctx->file_ast->u.file.decls.len;_ii++){
            const AstNode*_dd=ctx->file_ast->u.file.decls.data[_ii];
            if(_dd==_td) { _b=!_p; } if(_dd==parent_sd) { _p=true; } }
          if(!_b) break; }
        const char *ikw = struct_kw(_td);
        cg_printf(ctx, "%s %s {\n", ikw, tname);
        cg_emit_struct_fields(ctx, _td, "        ", false);
        cg_printf(ctx, "    }");
        if (fty_is_ptr) cg_puts(ctx, " *");
        if (fi->u.field_decl.name) {
            /* Preserve parenthesized field names like `(requests)` from the source. */
            if (fi->u.field_decl.name_paren)
                cg_printf(ctx, " (%s)", fi->u.field_decl.name);
            else
                cg_printf(ctx, " %s", fi->u.field_decl.name);
        }
        /* Array suffix for array-of-inline-struct */
        if (fty->kind == AST_TYPE_ARRAY) {
            const AstNode *_arr = fty;
            cg_emit_array_suffixes(ctx, _arr);
        }
        if (fi->u.field_decl.bit_width) { cg_puts(ctx, " : "); cg_expr(ctx, fi->u.field_decl.bit_width); }
        if (fi->u.field_decl.gcc_attrs) { cg_puts(ctx, " "); cg_puts(ctx, fi->u.field_decl.gcc_attrs); }
        cg_puts(ctx, ";\n");
        return true;
    }
    return false;
}

static void cg_emit_struct_fields(CgCtx *ctx, const AstNode *sd,
                                   const char *indent,
                                   bool try_inline) {
    for (size_t i = 0; i < sd->u.struct_def.fields.len; i++) {
        const AstNode *fi = sd->u.struct_def.fields.data[i];
        if (field_is_comma_cont(fi)) continue;
        cg_puts(ctx, indent);
        if (try_inline && !cg_emit_field_maybe_inline(ctx, sd, &sd->u.struct_def.fields, i))
            cg_emit_field_group(ctx, &sd->u.struct_def.fields, i);
        else if (!try_inline)
            cg_emit_field_group(ctx, &sd->u.struct_def.fields, i);
    }
}

/* Emit a typedef declaration faithfully.
 * Handles: typedef struct { ... } Alias;  typedef OldType Alias;  etc. */
static void cg_typedef_c(CgCtx *ctx, const AstNode *d) {
    if (!d || d->kind != AST_TYPEDEF_DECL) return;
    cg_emit_linemarker(ctx, d->loc);
    const AstNode *target = d->u.typedef_decl.target;
    const char    *alias  = d->u.typedef_decl.alias;
    if (!alias) return;
    const char    *cname  = alias;
    /* Emit __extension__ prefix if present in source */
    if (d->u.typedef_decl.has_extension)
        cg_puts(ctx, "__extension__ ");

    /* If target is NULL (primitive type like `typedef unsigned long ulong_t;`),
     * use the type annotation stored by sema to emit faithfully. */
    if (!target) {
        Type *t = (Type *)d->sem_type;
        if (!t || ty_is_error(t)) {
            t = ty_from_name(ctx->ts, alias);
        }
        if (t && !ty_is_error(t)) {
            cg_puts(ctx, "typedef ");
            cg_decl(ctx, t, cname);
            cg_puts(ctx, ";\n");
        }
        return;
    }

    /* Self-referential struct/union typedef: typedef struct X X;
     * Target is AST_TYPE_NAME or AST_STRUCT_DEF with same name as alias.
     * Must emit "typedef struct/union Name Name;" -- look up scope for kw. */
    const char *self_ref_name = NULL;
    if (target->kind == AST_TYPE_NAME && target->u.type_name.name)
        self_ref_name = target->u.type_name.name;
    else if (target->kind == AST_STRUCT_DEF && target->u.struct_def.name &&
             target->u.struct_def.fields.len == 0 &&
             target->u.struct_def.methods.len == 0)
        self_ref_name = target->u.struct_def.name;

    if (self_ref_name && strcmp(self_ref_name, alias) == 0) {
        /* Determine struct vs union by looking up the registered tag. */
        const char *kw = "struct";
        if (target->kind == AST_STRUCT_DEF)
            kw = struct_kw(target);
        if (ctx->file_scope) {
            Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, alias);
            if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF) {
                kw = struct_kw(tag->decl);
                if (tag->decl->u.struct_def.generic_params.len > 0)
                    return; /* generic template -- only specializations are emitted */
            }
        }
        /* Self-referential struct typedef (`typedef struct X { ... } X;`).
         * If the struct body has from_inline_typedef=true, inline the body here
         * so the output is the faithful combined form.
         * Otherwise just emit `typedef struct X X;` (body already standalone). */
        const AstNode *body_sd = NULL;
        if (ctx->file_scope) {
            Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, alias);
            if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF &&
                tag->decl->u.struct_def.fields.len > 0)
                body_sd = tag->decl;
        }
        bool struct_before_typedef = false;
        if (body_sd && ctx->file_ast) {
            bool found_struct = false;
            for (size_t _ki = 0; _ki < ctx->file_ast->u.file.decls.len; _ki++) {
                const AstNode *_kd = ctx->file_ast->u.file.decls.data[_ki];
                if (!_kd) continue;
                if (_kd == body_sd) { found_struct = true; }
                if (_kd == d && found_struct) { struct_before_typedef = true; break; }
                if (_kd == d) break;
            }
        }
        if (body_sd && struct_before_typedef &&
            body_sd->u.struct_def.from_inline_typedef) {
            /* Inline the body: `typedef struct X { ... } X;` */
            cg_printf(ctx, "typedef %s %s {\n", kw, cname);
            cg_emit_struct_fields(ctx, body_sd, "    ", true);
            cg_emit_typedef_close(ctx, cname, body_sd->u.struct_def.tail_attrs, d->u.typedef_decl.gcc_attrs);
        } else {
            cg_printf(ctx, "typedef %s %s %s;\n", kw, cname, cname);
        }
        return;
    }

    if (target->kind == AST_STRUCT_DEF) {
        /* typedef struct [Tag] { ... } Alias; */
        if (target->u.struct_def.generic_params.len > 0) return; /* generic template */
        const char *kw   = struct_kw(target);
        const char *sname = target->u.struct_def.name;
        /* Look up actual is_union from scope (R8 forward stub fix) */
        if (sname && ctx->file_scope) {
            Symbol *tag = scope_lookup_struct_tag(ctx->file_scope, sname);
            if (tag && tag->decl && tag->decl->kind == AST_STRUCT_DEF)
                kw = struct_kw(tag->decl);
        }
        bool has_body = (target->u.struct_def.fields.len > 0 ||
                         target->u.struct_def.methods.len > 0);
        if (has_body) {
            if (sname) cg_printf(ctx, "typedef %s %s {\n", kw, sname);
            else       cg_printf(ctx, "typedef %s {\n", kw);
            cg_emit_struct_fields(ctx, target, "    ", true);
            {
                cg_emit_typedef_close(ctx, cname, target->u.struct_def.tail_attrs, d->u.typedef_decl.gcc_attrs);
            }
        } else if (sname) {
            cg_printf(ctx, "typedef %s %s %s;\n", kw, sname, cname);
        }
    } else if (target->kind == AST_ENUM_DEF) {
        /* typedef enum [Tag] { ... } Alias; (inline_typedef already set) */
        const char *ename = target->u.enum_def.name;
        cg_puts(ctx, "typedef enum");
        if (ename) { cg_puts(ctx, " "); cg_puts(ctx, ename); }
        cg_puts(ctx, " {\n");
        for (size_t i = 0; i < target->u.enum_def.items.len; i++) {
            AstNode *en = target->u.enum_def.items.data[i];
            if (!en) continue;
            cg_printf(ctx, "    %s", en->u.enumerator.name);
            if (en->u.enumerator.value) {
                cg_puts(ctx, " = ");
                cg_const_expr(ctx, en->u.enumerator.value);
            }
            if (i + 1 < target->u.enum_def.items.len ||
                target->u.enum_def.trailing_comma) cg_puts(ctx, ",");
            cg_nl(ctx);
        }
        {
            if (d->u.typedef_decl.gcc_attrs)
                cg_printf(ctx, "} %s %s;\n", cname, d->u.typedef_decl.gcc_attrs);
            else if (target->kind == AST_STRUCT_DEF && target->u.struct_def.tail_attrs)
                cg_printf(ctx, "} %s;\n", target->u.struct_def.tail_attrs);
            else
                cg_printf(ctx, "} %s;\n", cname);
        };
    } else if (target->kind == AST_TYPE_FUNC) {
        /* typedef void Fn(int x); -- function-type typedef (K&R style).
         * Emit directly from AST: typedef ret_type alias(params); */
        AstNode *ret_ast = target->u.type_func.ret;
        Type *ret_t = ret_ast ? ty_from_ast(ctx->ts, ret_ast,
                                             cg_type_scope(ctx), NULL) : NULL;
        cg_puts(ctx, "typedef ");
        if (ret_t) cg_type(ctx, ret_t);
        else        cg_puts(ctx, "void");
        cg_printf(ctx, " %s(", cname);
        bool first_p = true;
        for (size_t i = 0; i < target->u.type_func.params.len; i++) {
            AstNode *p = target->u.type_func.params.data[i];
            if (!p || p->u.param_decl.is_vararg) continue;
            if (!first_p) cg_puts(ctx, ", ");
            first_p = false;
            Type *pt = ty_from_ast(ctx->ts, p->u.param_decl.type,
                                   cg_type_scope(ctx), NULL);
            {
                bool _ok = p->u.param_decl.type &&
                    cg_decl_ast(ctx, p->u.param_decl.type, p->u.param_decl.name);
                if (!_ok && pt) cg_decl(ctx, pt, p->u.param_decl.name);
            }
        }
        if (first_p) cg_puts(ctx, "void");
        cg_puts(ctx, ");\n");
    } else {
        /* typedef SomeType Alias; -- use type machinery.
         * C8: if target has volatile/_Atomic, use cg_type_from_ast.
         * C8: if target is __anon_struct_N (anonymous struct in a typedef),
         * find its body in file_ast and emit it inline: typedef struct { ... } Alias. */
        bool tgt_has_qual = ast_type_has_volatile(target) ||
            target->kind == AST_TYPE_ATOMIC;
        /* Peek through AST_TYPE_VOLATILE wrapper (post-type __attribute__) to
         * detect anonymous struct/union in typedef:
         *   typedef struct { ... } __attribute__((packed)) Name; */
        const AstNode *tgt_anon_base = target;
        const char *post_attr_text = NULL;
        if (target->kind == AST_TYPE_VOLATILE && target->u.type_volatile.base &&
            (target->u.type_volatile.base->kind == AST_TYPE_NAME ||
             target->u.type_volatile.base->kind == AST_ENUM_DEF)) {
            tgt_anon_base = target->u.type_volatile.base;
            post_attr_text = target->u.type_volatile.kw;
        }
        bool tgt_is_anon = tgt_anon_base->kind == AST_TYPE_NAME &&
            tgt_anon_base->u.type_name.name &&
            strncmp(tgt_anon_base->u.type_name.name, "__anon_", 7) == 0 &&
            ctx->file_ast;
        if (tgt_has_qual && tgt_is_anon) {
            /* Anonymous struct with post-type __attribute__:
             *   typedef struct { ... } __attribute__((packed)) Name;
             * Inline the body with the attribute. */
            const char *anon_name = tgt_anon_base->u.type_name.name;
            const AstNode *anon_sd = cg_find_struct_def(ctx, anon_name);
            if (anon_sd) {
                const char *kw = struct_kw(anon_sd);
                if (anon_sd->u.struct_def.fields.len > 0) {
                    cg_printf(ctx, "typedef %s {\n", kw);
                    cg_emit_struct_fields(ctx, anon_sd, "    ", true);
                    if (post_attr_text)
                        cg_printf(ctx, "} %s %s;\n", post_attr_text, cname);
                    else
                        cg_printf(ctx, "} %s;\n", cname);
                } else {
                    if (post_attr_text)
                        cg_printf(ctx, "typedef %s {} %s %s;\n", kw, cname, post_attr_text);
                    else
                        cg_printf(ctx, "typedef %s {} %s;\n", kw, cname);
                }
            } else {
                /* Fallback: just emit the tag reference with qualifiers */
                cg_puts(ctx, "typedef ");
                cg_type_from_ast(ctx, target);
                cg_printf(ctx, " %s;\n", cname);
            }
        } else if (tgt_is_anon) {
            /* Find the __anon_struct_N def in file_ast and emit inline. */
            const char *anon_name = target->u.type_name.name;
            const AstNode *anon_sd = cg_find_struct_def(ctx, anon_name);
            if (anon_sd) {
                const char *kw = struct_kw(anon_sd);
                if (anon_sd->u.struct_def.fields.len > 0) {
                    cg_printf(ctx, "typedef %s {\n", kw);
                    cg_emit_struct_fields(ctx, anon_sd, "    ", true);
                    {
                        cg_emit_typedef_close(ctx, cname, anon_sd->u.struct_def.tail_attrs, d->u.typedef_decl.gcc_attrs);
                    };
                } else {
                    /* Empty struct body: emit typedef struct/union {} alias; */
                    {
                        if (d->u.typedef_decl.gcc_attrs && anon_sd->u.struct_def.tail_attrs)
                            cg_printf(ctx, "typedef %s {} %s %s %s;\n", kw, anon_sd->u.struct_def.tail_attrs, cname, d->u.typedef_decl.gcc_attrs);
                        else if (anon_sd->u.struct_def.tail_attrs)
                            cg_printf(ctx, "typedef %s {} %s %s;\n", kw, anon_sd->u.struct_def.tail_attrs, cname);
                        else if (d->u.typedef_decl.gcc_attrs)
                            cg_printf(ctx, "typedef %s {} %s %s;\n", kw, cname, d->u.typedef_decl.gcc_attrs);
                        else
                            cg_printf(ctx, "typedef %s {} %s;\n", kw, cname);
                    };
                }
            } else {
                /* Fallback: emit the tag reference */
                Type *t = ty_from_ast(ctx->ts, target, cg_type_scope(ctx), NULL);
                if (t) {
                    cg_puts(ctx, "typedef ");
                    cg_decl(ctx, t, cname);
                    cg_puts(ctx, ";\n");
                }
            }
        } else if (target->kind == AST_TYPE_ARRAY) {
            /* C8: typedef Base Name[N] -- array typedef form.
             * Emit: typedef <base> <alias>[<N>]
             * Use cg_type_from_ast for base to preserve typedef names. */
            const AstNode *arr = target;
            /* Find innermost non-array base */
            const AstNode *base_ast = arr;
            while (base_ast && base_ast->kind == AST_TYPE_ARRAY)
                base_ast = base_ast->u.type_array.base;

            /* Check if the base is a function pointer: typedef ret (*alias[N])(params) */
            if (base_ast && base_ast->kind == AST_TYPE_PTR &&
                base_ast->u.type_ptr.base &&
                base_ast->u.type_ptr.base->kind == AST_TYPE_FUNC) {
                const AstNode *fn_ast = base_ast->u.type_ptr.base;
                const AstNode *ret_ast = fn_ast->u.type_func.ret;
                cg_puts(ctx, "typedef ");
                if (ret_ast) cg_type_from_ast(ctx, ret_ast);
                else         cg_puts(ctx, "int");
                cg_printf(ctx, " (*%s", cname);
                const AstNode *cur_arr = arr;
                cg_emit_array_suffixes(ctx, cur_arr);
                cg_puts(ctx, ")");
                cg_puts(ctx, "(");
                /* Emit param types */
                const AstVec *params = &fn_ast->u.type_func.params;
                bool first_param = true;
                for (size_t pi = 0; pi < params->len; pi++) {
                    AstNode *p = params->data[pi];
                    if (!p || p->kind != AST_PARAM_DECL) continue;
                    if (!first_param) cg_puts(ctx, ", ");
                    first_param = false;
                    AstNode *pty = p->u.param_decl.type;
                    if (pty && pty->kind == AST_TYPE_VOID) {
                        if (pi == 0 && params->len == 1) {
                            cg_puts(ctx, "void");
                            break;
                        }
                        cg_puts(ctx, "void");
                    } else if (pty) {
                        cg_type_from_ast(ctx, pty);
                    } else {
                        cg_puts(ctx, "int");
                    }
                    if (p->u.param_decl.name)
                        cg_printf(ctx, " %s", p->u.param_decl.name);
                }
                cg_puts(ctx, ")");
                cg_puts(ctx, ";\n");
            } else {
                cg_puts(ctx, "typedef ");
                if (base_ast) cg_type_from_ast(ctx, base_ast);
                else          cg_puts(ctx, "int");
                cg_printf(ctx, " %s", cname);
                /* Emit array suffix(es) outermost-first */
                const AstNode *cur_arr = arr;
                cg_emit_array_suffixes(ctx, cur_arr);
                cg_puts(ctx, ";\n");
            }
        } else {
            /* C8/C5: for simple type names using cg_type_from_ast.
             * For complex types (function pointers, etc.), use the Type* path.
             * also include AST_TYPE_ARRAY for array-of-fnptr typedefs. */
            bool use_ast = (target->kind == AST_TYPE_NAME ||
                            target->kind == AST_TYPE_CONST ||
                            target->kind == AST_TYPE_VOLATILE ||
                            target->kind == AST_TYPE_ATOMIC ||
                            target->kind == AST_TYPE_PTR ||
                            target->kind == AST_TYPE_ARRAY);
            /* Don't use cg_type_from_ast for fn-ptr typedefs: PTR(FUNC) needs
             * special AST-based emission to preserve param names. */
            if (use_ast && target->kind == AST_TYPE_PTR) {
                const AstNode *inner = target->u.type_ptr.base;
                while (inner && inner->kind == AST_TYPE_PTR)
                    inner = inner->u.type_ptr.base;
                if (inner && inner->kind == AST_TYPE_FUNC) {
                    /* Emit: typedef ret (*Alias)(params) with AST param names */
                    const AstNode *fn = inner;
                    cg_puts(ctx, "typedef ");
                    cg_type_from_ast(ctx, fn->u.type_func.ret);
                    cg_printf(ctx, " (*%s)(", cname);
                    cg_emit_func_params_ast(ctx, fn);
                    cg_puts(ctx, ");\n");
                    goto typedef_done;  /* done */
                }
                /* Non-function-pointer: keep use_ast=true so cg_type_from_ast
                 * runs and preserves typedef names (e.g. StackValue not union StackValue). */
            }
            if (use_ast) {
                /* `typedef struct min_heap { ... } min_heap_t`
                 * where tag != alias.  target is AST_TYPE_NAME("min_heap").
                 * The struct body was pushed to file_ast with from_inline_typedef=true.
                 * Emit it inline here when adjacent to this typedef in file_ast.
                 * For a simple type alias `typedef ExistingType NewAlias;`, do NOT
                 * inline any struct body -- just preserve the type name.
                 *
                 * Peek through AST_TYPE_VOLATILE wrapper (inserted by tspec_resolve
                 * for post-type __attribute__ like `void __attribute__((noreturn))`):
                 *   typedef struct { ... } __attribute__((packed)) Name;
                 * The attribute is stored in kw and must be emitted between `}`
                 * and the alias. */
                bool inlined_struct = false;
                /* Peek through AST_TYPE_VOLATILE wrapping to reach TYPE_NAME. */
                const AstNode *inner_target = target;
                const char *post_attr = NULL;
                if (target->kind == AST_TYPE_VOLATILE &&
                    target->u.type_volatile.base &&
                    (target->u.type_volatile.base->kind == AST_TYPE_NAME ||
                     target->u.type_volatile.base->kind == AST_ENUM_DEF)) {
                    inner_target = target->u.type_volatile.base;
                    post_attr = target->u.type_volatile.kw;
                }
                if (inner_target->kind == AST_TYPE_NAME &&
                    inner_target->u.type_name.name && ctx->file_ast) {
                    const char *tname = inner_target->u.type_name.name;
                    /* Find this typedef node's position in file_ast. */
                    size_t typedef_idx = SIZE_MAX;
                    for (size_t k = 0; k < ctx->file_ast->u.file.decls.len; k++) {
                        if (ctx->file_ast->u.file.decls.data[k] == d) {
                            typedef_idx = k; break;
                        }
                    }
                    /* Look for the struct body at the position immediately
                     * before the typedef (they are adjacent from the same statement). */
                    const AstNode *body_sd2 = NULL;
                    if (typedef_idx != SIZE_MAX && typedef_idx > 0) {
                        const AstNode *prev = ctx->file_ast->u.file.decls.data[typedef_idx - 1];
                        if (prev && prev->kind == AST_STRUCT_DEF &&
                            prev->u.struct_def.from_inline_typedef &&
                            prev->u.struct_def.name &&
                            strcmp(prev->u.struct_def.name, tname) == 0 &&
                            prev->u.struct_def.fields.len > 0) {
                            body_sd2 = prev;
                        }
                    }
                    if (body_sd2) {
                        const char *kw2 = struct_kw(body_sd2);
                        cg_printf(ctx, "typedef %s %s {\n", kw2, tname);
                        cg_emit_struct_fields(ctx, body_sd2, "    ", false);
                        if (post_attr || body_sd2->u.struct_def.tail_attrs)
                            cg_printf(ctx, "} %s %s;\n",
                                post_attr ? post_attr : body_sd2->u.struct_def.tail_attrs, cname);
                        else
                            cg_printf(ctx, "} %s;\n", cname);
                        inlined_struct = true;
                    }
                }
                if (!inlined_struct) {
                    cg_puts(ctx, "typedef ");
                    /* check for complex typedef patterns.
                     *
                     * Pattern 1: PTR→ARRAY -- `typedef char (*pa)[256];`
                     *   AST: PTR(ARRAY(TYPE_NAME("char"), 256))
                     *   Output: typedef char (*pa)[256];
                     *
                     * Pattern 2: ARRAY→PTR→FUNC -- `typedef void (*handlers[8])(int);`
                     *   AST: ARRAY(8, PTR(FUNC(...)))
                     *   Output: typedef void (*handlers[8])(int);
                     */
                    const AstNode *ptr = target;
                    int td_nstars = 0;
                    while (ptr && ptr->kind == AST_TYPE_PTR) {
                        td_nstars++;
                        ptr = ptr->u.type_ptr.base;
                    }
                    if (ptr && ptr->kind == AST_TYPE_ARRAY) {
                        /* Pattern 1: Pointer-to-array typedef */
                        cg_type_from_ast(ctx, ptr->u.type_array.base);
                        cg_puts(ctx, " (");
                        cg_emit_stars(ctx, td_nstars);
                        cg_puts(ctx, cname);
                        cg_puts(ctx, ")[");
                        if (ptr->u.type_array.has_static)   cg_puts(ctx, "static ");
                        if (ptr->u.type_array.has_restrict) cg_puts(ctx, "restrict");
                        if (ptr->u.type_array.has_vla_star) {
                            cg_puts(ctx, "*");
                        } else if (ptr->u.type_array.size) {
                            if (ptr->u.type_array.has_restrict) cg_puts(ctx, " ");
                            cg_const_expr(ctx, ptr->u.type_array.size);
                        }
                        cg_puts(ctx, "]");
                        cg_emit_typedef_semi(ctx, d->u.typedef_decl.gcc_attrs);
                    } else if (target->kind == AST_TYPE_ARRAY &&
                               target->u.type_array.base &&
                               target->u.type_array.base->kind == AST_TYPE_PTR) {
                        /* Pattern 2: Array-of-function-pointer typedef
                         * `typedef void (*handlers[8])(int)`
                         * AST: ARRAY(8, PTR(FUNC(ret, params))) */
                        const AstNode *arr = target;
                        const AstNode *aptr = arr->u.type_array.base;
                        const AstNode *afn = aptr->u.type_ptr.base;
                        if (afn && afn->kind == AST_TYPE_FUNC) {
                            /* Count multi-star: PTR(PTR(FUNC)) etc. */
                            int ap_nstars = 1;
                            const AstNode *ainner = afn;
                            AstPtrPeel aipp = ast_peel_ptr(ainner);
                            ap_nstars += aipp.nstars;
                            ainner = aipp.base;
                            if (ainner && ainner->kind == AST_TYPE_FUNC) {
                                /* Multi-star fnptr array -- handle below */
                                afn = ainner;
                            }
                            /* Emit return type */
                            cg_type_from_ast(ctx, afn->u.type_func.ret);
                            cg_puts(ctx, " (");
                            cg_puts(ctx, "*");
                            cg_puts(ctx, cname);
                            cg_puts(ctx, "[");
                            if (arr->u.type_array.size)
                                cg_const_expr(ctx, arr->u.type_array.size);
                            cg_puts(ctx, "])(");
                            bool afp = true;
                            for (size_t ai = 0; ai < afn->u.type_func.params.len; ai++) {
                                AstNode *ap = afn->u.type_func.params.data[ai];
                                if (!ap) continue;
                                const AstNode *apty = (ap->kind == AST_PARAM_DECL) ? ap->u.param_decl.type : ap;
                                bool a_is_va = (ap->kind == AST_PARAM_DECL && ap->u.param_decl.is_vararg);
                                if (a_is_va) { if (!afp) cg_puts(ctx, ", "); cg_puts(ctx, "..."); afp = false; continue; }
                                if (!afp) cg_puts(ctx, ", ");
                                afp = false;
                                cg_type_from_ast(ctx, apty);
                            }
                            if (afp && !afn->u.type_func.params_unspecified)
                                cg_puts(ctx, "void");
                            cg_puts(ctx, ")");
                            cg_emit_typedef_semi(ctx, d->u.typedef_decl.gcc_attrs);
                        } else {
                            /* Fallback: not a fnptr array, emit normally */
                            cg_type_from_ast(ctx, target);
                            cg_printf(ctx, " %s", cname);
                            cg_emit_typedef_semi(ctx, d->u.typedef_decl.gcc_attrs);
                        }
                    } else {
                        cg_type_from_ast(ctx, target);
                        cg_printf(ctx, " %s", cname);
                        cg_emit_typedef_semi(ctx, d->u.typedef_decl.gcc_attrs);
                    }
                }
            } else {
                Type *t = ty_from_ast(ctx->ts, target, cg_type_scope(ctx), NULL);
                if (t) {
                    cg_puts(ctx, "typedef ");
                    cg_decl(ctx, t, cname);
                    cg_emit_typedef_semi(ctx, d->u.typedef_decl.gcc_attrs);
                }
            }
        }
    }
    typedef_done:;
}

/* Emit a global variable declaration faithfully. */
/* =========================================================================
 * cg_emit_func_params_ast — emit the parameter list of a TYPE_FUNC AST node.
 *
 * Emits:  `type1 name1, type2 name2, ..., ...`  (without surrounding parens)
 * Emits `void` for `()` functions with no parameters.
 * ======================================================================== */
static void cg_emit_func_params_ast(CgCtx *ctx, const AstNode *fn_ast) {
    if (!fn_ast || fn_ast->kind != AST_TYPE_FUNC) {
        cg_puts(ctx, "void"); return;
    }
    bool first = true;
    for (size_t i = 0; i < fn_ast->u.type_func.params.len; i++) {
        const AstNode *p = fn_ast->u.type_func.params.data[i];
        if (!p) continue;
        if (p->kind == AST_PARAM_DECL && p->u.param_decl.is_vararg) {
            if (!first) cg_puts(ctx, ", ");
            cg_puts(ctx, "...");
            first = false;
            continue;
        }
        if (!first) cg_puts(ctx, ", ");
        first = false;
        const AstNode *pty = (p->kind == AST_PARAM_DECL)
                             ? p->u.param_decl.type : p;
        const char    *pnm = (p->kind == AST_PARAM_DECL)
                             ? p->u.param_decl.name : NULL;
        bool emitted = pty && cg_decl_ast(ctx, pty, pnm);
        if (!emitted && pty) {
            Type *pt = ty_from_ast(ctx->ts, (AstNode*)pty,
                                   cg_type_scope(ctx), NULL);
            if (pt && !ty_is_error(pt)) cg_decl(ctx, pt, pnm);
        }
    }
    if (first && !fn_ast->u.type_func.params_unspecified)
        cg_puts(ctx, "void");
}

/* =========================================================================
 * cg_emit_comma_cont_declarator -- emit the declarator part (without the
 * shared base type) of a comma-continuation variable declaration.
 *
 * For `char arr[2][4], (*p)[4], *q;`, after `char arr[2][4]` is emitted,
 * this function emits `(*p)[4]` and `*q` for the continuation declarators.
 *
 * Handles the tricky pointer-to-array case: `(*p)[N]` requires parentheses
 * around the pointer star and name; the naive "collect pointers then arrays"
 * walk would produce `*p[N]` (array of N pointers -- the opposite type).
 * ======================================================================== */
static void cg_emit_comma_cont_declarator(CgCtx *ctx,
                                          const AstNode *ty,
                                          const char    *name) {
    if (!ty) { if (name) cg_puts(ctx, name); return; }

    /* pointer-to-array: `(*name)[N]`  or  `(*name)[N1][N2]` */
    if (ty->kind == AST_TYPE_PTR &&
        ty->u.type_ptr.base &&
        ty->u.type_ptr.base->kind == AST_TYPE_ARRAY) {
        cg_puts(ctx, "(*");
        if (ty->u.type_ptr.ptr_const)    cg_puts(ctx, " const");
        if (ty->u.type_ptr.ptr_volatile)  cg_puts(ctx, " volatile");
        if (ty->u.type_ptr.ptr_restrict)
            cg_puts(ctx, ty->u.type_ptr.restrict_kw
                         ? ty->u.type_ptr.restrict_kw : " restrict");
        if (name) { cg_puts(ctx, " "); cg_puts(ctx, name); }
        cg_puts(ctx, ")");
        const AstNode *arr = ty->u.type_ptr.base;
        while (arr && arr->kind == AST_TYPE_ARRAY) {
            cg_puts(ctx, "[");
            if (arr->u.type_array.has_static)   cg_puts(ctx, "static ");
            if (arr->u.type_array.has_restrict) cg_puts(ctx, "restrict ");
            if (arr->u.type_array.has_vla_star) cg_puts(ctx, "*");
            else if (arr->u.type_array.size)    cg_const_expr(ctx, arr->u.type_array.size);
            cg_puts(ctx, "]");
            arr = arr->u.type_array.base;
        }
        return;
    }

    /* General: collect outermost arrays then pointer stars */
    const AstNode *_ct = ty;
    const AstNode *_arrs[16]; size_t _na = 0;
    while (_ct && _ct->kind == AST_TYPE_ARRAY && _na < 16) {
        _arrs[_na++] = _ct; _ct = _ct->u.type_array.base;
    }
    const AstNode *_ptrs[32]; size_t _np = 0;
    while (_ct && _ct->kind == AST_TYPE_PTR && _np < 32) {
        _ptrs[_np++] = _ct; _ct = _ct->u.type_ptr.base;
    }
    for (size_t _w = _np; _w > 0; _w--) {
        const AstNode *_wr = _ptrs[_w-1];
        cg_puts(ctx, "*");
        if (_wr->u.type_ptr.nullability)
            { cg_puts(ctx, " "); cg_puts(ctx, _wr->u.type_ptr.nullability); }
        if (_wr->u.type_ptr.ptr_const)    cg_puts(ctx, " const");
        if (_wr->u.type_ptr.ptr_volatile)  cg_puts(ctx, " volatile");
        if (_wr->u.type_ptr.ptr_restrict)
            cg_puts(ctx, _wr->u.type_ptr.restrict_kw
                         ? _wr->u.type_ptr.restrict_kw : " restrict");
        cg_puts(ctx, " ");
    }
    if (name) cg_puts(ctx, name);
    for (size_t _ai = _na; _ai > 0; _ai--) {
        cg_puts(ctx, "[");
        if (_arrs[_ai-1]->u.type_array.has_static)   cg_puts(ctx, "static ");
        if (_arrs[_ai-1]->u.type_array.has_restrict) cg_puts(ctx, "restrict ");
        if (_arrs[_ai-1]->u.type_array.has_vla_star) cg_puts(ctx, "*");
        else if (_arrs[_ai-1]->u.type_array.size)
            cg_const_expr(ctx, _arrs[_ai-1]->u.type_array.size);
        cg_puts(ctx, "]");
    }
}

static void cg_var_c(CgCtx *ctx, const AstNode *d) {
    if (!d || d->kind != AST_VAR_DECL) return;
    /* Comma-continuation vars are emitted inline by the primary
     * (non-comma-cont) declaration node via the comma group logic
     * below (see file-scope comma-continuation emission). */
    if (d->u.var_decl.is_comma_cont) return;
    cg_emit_linemarker(ctx, d->loc);
    StorageClass sc = d->u.var_decl.storage;
    /* p43: leading attribute must come BEFORE storage class */
    if (d->u.var_decl.gcc_attrs && d->u.var_decl.fmt & FMTF_ATTRS_LEADING) {
        cg_puts(ctx, d->u.var_decl.gcc_attrs);
        cg_puts(ctx, " ");
    }
    cg_emit_storage_class(ctx, sc);
    if (sc == SC_TYPEDEF) return;
    if (d->u.var_decl.is_constexpr) cg_puts(ctx, "constexpr ");
    if (d->u.var_decl.is_thread_local) cg_puts(ctx, "_Thread_local ");
    /* emit C11 _Alignas after storage-class.
     * gcc -E emits spaces around parens: `_Alignas ( 16 )` */
    if (d->u.var_decl.alignas_text) {
        cg_printf(ctx, "_Alignas ( %s ) ", d->u.var_decl.alignas_text);
    }
    /* Non-leading (trailing) attributes emitted after storage class */
    if (d->u.var_decl.gcc_attrs && !(d->u.var_decl.fmt & FMTF_ATTRS_LEADING)) {
        cg_puts(ctx, d->u.var_decl.gcc_attrs);
        cg_puts(ctx, " ");
    }
    /* C mode: try to emit the type from the AST node directly so that
     * qualifiers like `struct`, `union`, `enum`, `_Atomic`, `volatile`
     * are preserved faithfully.  Fall back to the type-machinery path. */
    const AstNode *ty_ast = d->u.var_decl.type;
    Type *t = (Type *)d->sem_type;
    if (!t) t = ty_from_ast(ctx->ts, ty_ast, cg_type_scope(ctx), NULL);
    /* C8: when the type (or its array element) is an anonymous struct,
     * emit the struct body inline in the variable declaration. */
    {
        AstCvpaPeel vp = ast_type_peel_cvpa(ty_ast);
        if (ctx->file_ast && vp.base &&
            vp.base->kind == AST_TYPE_NAME && vp.base->u.type_name.name &&
            strncmp(vp.base->u.type_name.name, "__anon_", 7) == 0) {
            const char *anon_nm = vp.base->u.type_name.name;
            const AstNode *inner_sd = cg_find_struct_def(ctx, anon_nm);
            if (inner_sd && inner_sd->u.struct_def.fields.len > 0) {
                const char *kw = struct_kw(inner_sd);
                if (vp.is_const) cg_puts(ctx, "const ");
                if (vp.is_volatile) cg_puts(ctx, "volatile ");
                cg_printf(ctx, "%s {\n", kw);
                cg_emit_struct_fields(ctx, inner_sd, "  ", false);
                cg_printf(ctx, "} %s", d->u.var_decl.name ? d->u.var_decl.name : "");
                /* Emit array suffixes */
                const AstNode *arr = ty_ast;
                cg_emit_array_suffixes(ctx, arr);
                goto var_decl_emitted;
            }
        }
    }
    /* Named struct defined inline in this variable declaration.
     * Emit: `struct Tag { fields } varname = {...};`
     * Only inline if the struct appears immediately before this var in file_ast
     * (prevents re-emitting the body for subsequent vars of the same type). */
    if (ctx->file_ast && ty_ast) {
        AstCvpaPeel vp2 = ast_type_peel_cvpa(ty_ast);
        if (vp2.base && vp2.base->kind == AST_TYPE_NAME &&
            vp2.base->u.type_name.name) {
            const char *nm2 = vp2.base->u.type_name.name;
            const AstNode *iv_sd = NULL;
            size_t iv_sd_idx = SIZE_MAX, var_idx = SIZE_MAX;
            /* Search file-level decls */
            for (size_t _k = 0; _k < ctx->file_ast->u.file.decls.len; _k++) {
                const AstNode *_dd = ctx->file_ast->u.file.decls.data[_k];
                if (_dd == d) var_idx = _k;
                if (_dd && _dd->kind == AST_STRUCT_DEF &&
                    _dd->u.struct_def.from_inline_var &&
                    _dd->u.struct_def.name &&
                    strcmp(_dd->u.struct_def.name, nm2) == 0) {
                    iv_sd = _dd; iv_sd_idx = _k;
                }
            }
            /* Also search local block stmts (for local var declarations) */
            if (!iv_sd && ctx->local_block_stmts) {
                for (size_t _k = 0; _k < ctx->local_block_stmts->len; _k++) {
                    const AstNode *_dd = ctx->local_block_stmts->data[_k];
                    if (_dd == d) var_idx = _k;
                    if (_dd && _dd->kind == AST_STRUCT_DEF &&
                        _dd->u.struct_def.from_inline_var &&
                        _dd->u.struct_def.name &&
                        strcmp(_dd->u.struct_def.name, nm2) == 0) {
                        iv_sd = _dd; iv_sd_idx = _k;
                    }
                }
            }
            /* Only inline if struct is within 2 positions before this var */
            if (iv_sd && iv_sd->u.struct_def.fields.len > 0 &&
                var_idx != SIZE_MAX && iv_sd_idx != SIZE_MAX &&
                var_idx > iv_sd_idx && var_idx - iv_sd_idx <= 2) {
                const char *kw = struct_kw(iv_sd);
                /* Emit const/volatile qualifiers that were
                 * stripped during type unwrapping.  Without this, `static const
                 * struct NanInfName { } aNanInfName[]` loses `const`. */
                if (vp2.is_const)    cg_puts(ctx, "const ");
                if (vp2.is_volatile) cg_puts(ctx, "volatile ");
                cg_printf(ctx, "%s %s {\n", kw, iv_sd->u.struct_def.name);
                cg_emit_struct_fields(ctx, iv_sd, "  ", false);
                cg_puts(ctx, "}");
                /* Pointer stars (for `static struct X { } *p = 0`) */
                cg_emit_stars(ctx, vp2.nptr);
                cg_printf(ctx, " %s", d->u.var_decl.name ? d->u.var_decl.name : "");
                /* Emit array suffixes */
                const AstNode *arr2 = ty_ast;
                cg_emit_array_suffixes(ctx, arr2);
                goto var_decl_emitted;
            }
        }
    }
    /* C8: when volatile or _Atomic is present, use cg_type_from_ast to
     * preserve the qualifier -- the Type* path strips both. */
    bool has_qual = ty_ast &&
        (ast_type_has_volatile(ty_ast) ||
         ty_ast->kind == AST_TYPE_ATOMIC);
    if (has_qual && d->u.var_decl.name) {
        cg_type_from_ast(ctx, ty_ast);
        cg_printf(ctx, " %s", d->u.var_decl.name);
    } else {
        /* Emit from AST to preserve qualifiers, typedef aliases, and array forms. */
        bool emitted = ty_ast &&
                       cg_decl_ast(ctx, ty_ast, d->u.var_decl.name);
        if (!emitted && t) cg_decl(ctx, t, d->u.var_decl.name);
    }
    var_decl_emitted:;
    if (d->u.var_decl.init) {
        if (d->u.var_decl.gcc_attrs_trailing) {
            cg_puts(ctx, " "); cg_puts(ctx, d->u.var_decl.gcc_attrs_trailing);
        }
        cg_puts(ctx, " = ");
        cg_expr(ctx, d->u.var_decl.init);
    } else if (d->u.var_decl.gcc_attrs_trailing) {
        cg_puts(ctx, " "); cg_puts(ctx, d->u.var_decl.gcc_attrs_trailing);
    }
    /* Emit file-scope comma-continuation vars inline.
     * Source: `static int x, y, z;` -- emit as comma group, not 3 lines. */
    if (ctx->file_ast) {
        /* Find this var in file.decls and emit following is_comma_cont vars. */
        size_t my_pos = SIZE_MAX;
        for (size_t _ki = 0; _ki < ctx->file_ast->u.file.decls.len; _ki++) {
            if (ctx->file_ast->u.file.decls.data[_ki] == d) { my_pos = _ki; break; }
        }
        if (my_pos != SIZE_MAX) {
            for (size_t _ki = my_pos + 1; _ki < ctx->file_ast->u.file.decls.len; _ki++) {
                const AstNode *_nxt = ctx->file_ast->u.file.decls.data[_ki];
                if (!_nxt || _nxt->kind != AST_VAR_DECL ||
                    !_nxt->u.var_decl.is_comma_cont) break;
                cg_puts(ctx, ", ");
                cg_emit_comma_cont_declarator(ctx,
                    _nxt->u.var_decl.type, _nxt->u.var_decl.name);
                if (_nxt->u.var_decl.init) {
                    cg_puts(ctx, " = "); cg_expr(ctx, _nxt->u.var_decl.init);
                }
            }
        }
    }
    cg_puts(ctx, ";\n");
}

/* Emit a standalone enum definition faithfully.
 * Zero items = forward declaration: emit "enum Name;" (GCC extension). */
static void cg_enum_c(CgCtx *ctx, const AstNode *d) {
    if (!d || d->kind != AST_ENUM_DEF) return;
    cg_emit_linemarker(ctx, d->loc);
    const char *name = d->u.enum_def.name;
    if (d->u.enum_def.items.len == 0) {
        /* Forward declaration: enum Name; */
        cg_puts(ctx, "enum");
        if (name) { cg_puts(ctx, " "); cg_puts(ctx, name); }
        cg_puts(ctx, ";\n");
        return;
    }
    cg_puts(ctx, "enum");
    if (name) { cg_puts(ctx, " "); cg_puts(ctx, name); }
    cg_puts(ctx, " {\n");
    for (size_t i = 0; i < d->u.enum_def.items.len; i++) {
        AstNode *en = d->u.enum_def.items.data[i];
        if (!en) continue;
        cg_printf(ctx, "    %s", en->u.enumerator.name);
        if (en->u.enumerator.value) {
            cg_puts(ctx, " = ");
            cg_const_expr(ctx, en->u.enumerator.value);
        }
        if (i + 1 < d->u.enum_def.items.len) cg_puts(ctx, ",");
        else if (d->u.enum_def.trailing_comma) cg_puts(ctx, ",");  /* preserve trailing comma */
        cg_nl(ctx);
    }
    cg_puts(ctx, "};\n");
}


/* =========================================================================
 * @to_cstr preamble injection
 *
 * When @to_cstr(slice) is used in Sharp source, we inject:
 *   1. typedef for sp_str_t (the slice type: { const char *ptr; size_t len; })
 *   2. #include <string.h> (for memcpy)
 *   3. The __SP_MAGIC_TO_C_STR macro using GCC VLA statement-expression
 *
 * This macro is the canonical Sharp-to-C bridge: a zero-waste VLA that
 * allocates exactly len+1 bytes on the stack, copies the slice content,
 * and NUL-terminates it -- returning a const char*.
 * ====================================================================== */

/* Pre-scan: detect @to_cstr usage in the AST before codegen. */
static bool ast_uses_to_cstr(const AstNode *n) {
    if (!n) return false;
    if (n->kind == AST_AT_INTRINSIC &&
        strcmp(n->u.at_intrinsic.name, "to_cstr") == 0)
        return true;
    switch (n->kind) {
    case AST_FILE: {
        for (size_t i = 0; i < n->u.file.decls.len; i++)
            if (ast_uses_to_cstr(n->u.file.decls.data[i])) return true;
        break;
    }
    case AST_FUNC_DEF: {
        if (n->u.func_def.body) {
            for (size_t i = 0; i < n->u.func_def.body->u.block.stmts.len; i++)
                if (ast_uses_to_cstr(n->u.func_def.body->u.block.stmts.data[i]))
                    return true;
        }
        break;
    }
    case AST_BINOP:
        return ast_uses_to_cstr(n->u.binop.lhs) || ast_uses_to_cstr(n->u.binop.rhs);
    case AST_UNARY:
        return ast_uses_to_cstr(n->u.unary.operand);
    case AST_PAREN:
        return ast_uses_to_cstr(n->u.paren.inner);
    case AST_TERNARY:
        return ast_uses_to_cstr(n->u.ternary.cond) ||
               ast_uses_to_cstr(n->u.ternary.then_) ||
               ast_uses_to_cstr(n->u.ternary.else_);
    case AST_CALL: {
        for (size_t i = 0; i < n->u.call.args.len; i++)
            if (ast_uses_to_cstr(n->u.call.args.data[i])) return true;
        break;
    }
    case AST_METHOD_CALL: {
        if (n->u.method_call.recv && ast_uses_to_cstr(n->u.method_call.recv)) return true;
        for (size_t i = 0; i < n->u.method_call.args.len; i++)
            if (ast_uses_to_cstr(n->u.method_call.args.data[i])) return true;
        break;
    }
    case AST_FIELD_ACCESS:
        return ast_uses_to_cstr(n->u.field_access.recv);
    case AST_INDEX:
        return ast_uses_to_cstr(n->u.index_.base) || ast_uses_to_cstr(n->u.index_.index);
    case AST_CAST:
        return ast_uses_to_cstr(n->u.cast.operand);
    case AST_COMMA:
        return ast_uses_to_cstr(n->u.comma.lhs) || ast_uses_to_cstr(n->u.comma.rhs);
    case AST_INIT_LIST: {
        for (size_t i = 0; i < n->u.init_list.items.len; i++)
            if (ast_uses_to_cstr(n->u.init_list.items.data[i])) return true;
        break;
    }
    case AST_SIZEOF:
        return n->u.sizeof_.operand && ast_uses_to_cstr(n->u.sizeof_.operand);
    case AST_SWITCH:
        return (n->u.switch_.cond && ast_uses_to_cstr(n->u.switch_.cond)) ||
               (n->u.switch_.body && ast_uses_to_cstr(n->u.switch_.body));
    case AST_DEFER:
        return n->u.defer_.body && ast_uses_to_cstr(n->u.defer_.body);
    case AST_RETURN:
        return n->u.return_.value && ast_uses_to_cstr(n->u.return_.value);
    case AST_IF:
        return (n->u.if_.cond && ast_uses_to_cstr(n->u.if_.cond)) ||
               (n->u.if_.then_ && ast_uses_to_cstr(n->u.if_.then_)) ||
               (n->u.if_.else_ && ast_uses_to_cstr(n->u.if_.else_));
    case AST_WHILE:
        return (n->u.while_.cond && ast_uses_to_cstr(n->u.while_.cond)) ||
               (n->u.while_.body && ast_uses_to_cstr(n->u.while_.body));
    case AST_DO_WHILE:
        return (n->u.do_while.cond && ast_uses_to_cstr(n->u.do_while.cond)) ||
               (n->u.do_while.body && ast_uses_to_cstr(n->u.do_while.body));
    case AST_FOR:
        return (n->u.for_.init && ast_uses_to_cstr(n->u.for_.init)) ||
               (n->u.for_.cond && ast_uses_to_cstr(n->u.for_.cond)) ||
               (n->u.for_.post && ast_uses_to_cstr(n->u.for_.post)) ||
               (n->u.for_.body && ast_uses_to_cstr(n->u.for_.body));
    case AST_VAR_DECL:
        return n->u.var_decl.init && ast_uses_to_cstr(n->u.var_decl.init);
    case AST_EXPR_STMT:
        return n->u.expr_stmt.expr && ast_uses_to_cstr(n->u.expr_stmt.expr);
    case AST_DECL_STMT:
        return n->u.decl_stmt.decl && ast_uses_to_cstr(n->u.decl_stmt.decl);
    case AST_BLOCK: {
        for (size_t i = 0; i < n->u.block.stmts.len; i++)
            if (ast_uses_to_cstr(n->u.block.stmts.data[i])) return true;
        break;
    }
    case AST_STRUCT_LIT: {
        for (size_t i = 0; i < n->u.struct_lit.field_vals.len; i++)
            if (ast_uses_to_cstr(n->u.struct_lit.field_vals.data[i])) return true;
        break;
    }
    case AST_COMPOUND_LIT:
        return ast_uses_to_cstr(n->u.compound_lit.init);
    case AST_DESIGNATED_INIT:
        return n->u.designated_init.value && ast_uses_to_cstr(n->u.designated_init.value);
    case AST_STMT_EXPR:
        return n->u.stmt_expr.block && ast_uses_to_cstr(n->u.stmt_expr.block);
    case AST_GENERIC_EXPR: {
        if (n->u.generic_expr.controlling && ast_uses_to_cstr(n->u.generic_expr.controlling)) return true;
        for (size_t i = 0; i < n->u.generic_expr.associations.len; i++)
            if (ast_uses_to_cstr(n->u.generic_expr.associations.data[i])) return true;
        break;
    }
    case AST_GENERIC_ASSOC:
        return n->u.generic_assoc.value && ast_uses_to_cstr(n->u.generic_assoc.value);
    default:
        break;
    }
    return false;
}

static void cg_emit_to_cstr_preamble(CgCtx *ctx) {
    if (!ctx->uses_to_cstr) return;

    cg_puts(ctx, "#include <string.h>\n\n");

    cg_puts(ctx, "/**\n");
    cg_puts(ctx, " * @brief sp language built-in: slice to NUL-terminated C string\n");
    cg_puts(ctx, " * @note  Pure VLA stack allocation -- allocates exactly len+1 bytes,\n");
    cg_puts(ctx, " *        copies + NUL-terminates, returns const char*.\n");
    cg_puts(ctx, " *        Multiple calls coexist perfectly; naturally reentrancy-safe.\n");
    cg_puts(ctx, " *        Works with any struct having .ptr and .len fields.\n");
    cg_puts(ctx, " */\n");
    cg_puts(ctx, "#define __SP_MAGIC_TO_C_STR(__sp_slice__) ( ({ \\\n");
    cg_puts(ctx, "    __auto_type __src__ = (__sp_slice__); \\\n");
    cg_puts(ctx, "    const size_t __local_len__ = __src__.len; \\\n");
    cg_puts(ctx, "    char __local_vla_buf__[__local_len__ + 1]; \\\n");
    cg_puts(ctx, "    if (__local_len__ > 0) { \\\n");
    cg_puts(ctx, "        memcpy(__local_vla_buf__, __src__.ptr, __local_len__); \\\n");
    cg_puts(ctx, "    } \\\n");
    cg_puts(ctx, "    __local_vla_buf__[__local_len__] = '\\0'; \\\n");
    cg_puts(ctx, "    (const char*)__local_vla_buf__; \\\n");
    cg_puts(ctx, "}) )\n\n");
}

/* =========================================================================
 * cg_file -- Sharp-mode code generator (multi-pass)
 *
 * Design principles:
 *  1. Forward declarations injected only where actually needed.
 *  2. Sharp-specific transforms only for Sharp AST nodes:
 *     - AST_STRUCT_DEF with methods → emit C functions for each method
 *     - AST_FUNC_DEF with generic_params → skip (monomorphized on demand)
 *     - AST_DEFER → emit cleanup block at scope exit
 * ========================================================================= */

/* Check if type AST node references a given name (struct or typedef). */
static bool type_refs_name(const AstNode *ty, const char *name);

static bool type_refs_name(const AstNode *ty, const char *name) {
    if (!ty) return false;
    switch (ty->kind) {
    case AST_TYPE_NAME:
        return ty->u.type_name.name &&
               strcmp(ty->u.type_name.name, name) == 0;
    case AST_TYPEOF_TYPE:
        return false;
    case AST_TYPE_PTR:    return type_refs_name(ty->u.type_ptr.base, name);
    case AST_TYPE_CONST:  return type_refs_name(ty->u.type_const.base, name);
    case AST_TYPE_VOLATILE: return type_refs_name(ty->u.type_volatile.base, name);
    case AST_TYPE_ATOMIC: return type_refs_name(ty->u.type_atomic.base, name);
    case AST_TYPE_ARRAY:  return type_refs_name(ty->u.type_array.base, name);
    case AST_TYPE_FUNC: {
        if (type_refs_name(ty->u.type_func.ret, name)) return true;
        for (size_t i = 0; i < ty->u.type_func.params.len; i++) {
            const AstNode *pi = ty->u.type_func.params.data[i];
            if (pi && pi->kind == AST_PARAM_DECL && pi->u.param_decl.type)
                pi = pi->u.param_decl.type;
            if (type_refs_name(pi, name)) return true;
        }
        return false;
    }
    case AST_STRUCT_DEF: {
        for (size_t i = 0; i < ty->u.struct_def.fields.len; i++) {
            const AstNode *f = ty->u.struct_def.fields.data[i];
            if (f && f->kind == AST_FIELD_DECL)
                if (type_refs_name(f->u.field_decl.type, name)) return true;
        }
        return false;
    }
    case AST_TYPEDEF_DECL:
        return type_refs_name(ty->u.typedef_decl.target, name);
    default: return false;
    }
}

/* Like type_refs_name but ignores struct/enum tag references.
 * A `struct MySurface *` parameter does NOT depend on the
 * `typedef struct MySurface MySurface;` typedef — only a bare
 * `MySurface *` (is_struct_tag == false) does. */
static bool type_refs_typedef(const AstNode *ty, const char *name) {
    if (!ty) return false;
    switch (ty->kind) {
    case AST_TYPE_NAME:
        return ty->u.type_name.name &&
               !ty->u.type_name.is_struct_tag &&
               !ty->u.type_name.is_enum_tag &&
               strcmp(ty->u.type_name.name, name) == 0;
    case AST_TYPEOF_TYPE:
        return false;
    case AST_TYPE_PTR:    return type_refs_typedef(ty->u.type_ptr.base, name);
    case AST_TYPE_CONST:  return type_refs_typedef(ty->u.type_const.base, name);
    case AST_TYPE_VOLATILE: return type_refs_typedef(ty->u.type_volatile.base, name);
    case AST_TYPE_ATOMIC: return type_refs_typedef(ty->u.type_atomic.base, name);
    case AST_TYPE_ARRAY:  return type_refs_typedef(ty->u.type_array.base, name);
    case AST_TYPE_FUNC: {
        if (type_refs_typedef(ty->u.type_func.ret, name)) return true;
        for (size_t i = 0; i < ty->u.type_func.params.len; i++) {
            const AstNode *pi = ty->u.type_func.params.data[i];
            if (pi && pi->kind == AST_PARAM_DECL && pi->u.param_decl.type)
                pi = pi->u.param_decl.type;
            if (type_refs_typedef(pi, name)) return true;
        }
        return false;
    }
    case AST_STRUCT_DEF: {
        for (size_t i = 0; i < ty->u.struct_def.fields.len; i++) {
            const AstNode *f = ty->u.struct_def.fields.data[i];
            if (f && f->kind == AST_FIELD_DECL)
                if (type_refs_typedef(f->u.field_decl.type, name)) return true;
        }
        return false;
    }
    case AST_TYPEDEF_DECL:
        return type_refs_typedef(ty->u.typedef_decl.target, name);
    default: return false;
    }
}

/* Collect all struct/union names defined in this file (deduplicated). */
static void collect_struct_names(const AstNode *file,
                                  const char ***names_out, size_t *n_out) {
    size_t cap = 16, n = 0;
    const char **names = malloc(cap * sizeof *names);
    if (!names) abort();
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        const AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        const char *nm = NULL;
        if (d->kind == AST_STRUCT_DEF) {
            if (d->u.struct_def.is_synthetic) continue;
            nm = d->u.struct_def.name;
        } else if (d->kind == AST_TYPEDEF_DECL) {
            /* typedef struct X X -- record the struct tag */
            const AstNode *tgt = d->u.typedef_decl.target;
            const char *alias  = d->u.typedef_decl.alias;
            if (tgt && tgt->kind == AST_TYPE_NAME &&
                tgt->u.type_name.name && alias &&
                strcmp(tgt->u.type_name.name, alias) == 0)
                nm = alias;
        }
        if (!nm) continue;
        /* Deduplicate: skip if already collected (e.g. `typedef struct X{} X`
         * produces both an AST_STRUCT_DEF and an AST_TYPEDEF_DECL for X). */
        bool dup = false;
        for (size_t j = 0; j < n; j++)
            if (names[j] == nm || strcmp(names[j], nm) == 0) { dup = true; break; }
        if (dup) continue;
        if (n == cap) { cap *= 2; names = realloc(names, cap * sizeof *names); if (!names) abort(); }
        names[n++] = nm;
    }
    *names_out = names;
    *n_out = n;
}

/* Return true if the type AST node references a struct/union name. */
static bool type_refs_struct(const AstNode *ty, const char *name) {
    if (!ty) return false;
    switch (ty->kind) {
    case AST_TYPE_NAME:
        return ty->u.type_name.name &&
               strcmp(ty->u.type_name.name, name) == 0;
    case AST_TYPEOF_TYPE:
        return false;
    case AST_TYPE_PTR:    return type_refs_struct(ty->u.type_ptr.base, name);
    case AST_TYPE_CONST:  return type_refs_struct(ty->u.type_const.base, name);
    case AST_TYPE_VOLATILE: return type_refs_struct(ty->u.type_volatile.base, name);
    case AST_TYPE_ATOMIC: return type_refs_struct(ty->u.type_atomic.base, name);
    case AST_TYPE_ARRAY:  return type_refs_struct(ty->u.type_array.base, name);
    case AST_TYPE_FUNC: {
        if (type_refs_struct(ty->u.type_func.ret, name)) return true;
        for (size_t i = 0; i < ty->u.type_func.params.len; i++)
            if (type_refs_struct(ty->u.type_func.params.data[i], name)) return true;
        return false;
    }
    default: return false;
    }
}

/* Return true if decl d (appearing before the definition of struct 'name')
 * references 'name' as a type in fields, params, or return types. */
static bool decl_uses_struct(const AstNode *d, const char *name) {
    if (!d) return false;
    switch (d->kind) {
    case AST_STRUCT_DEF:
        for (size_t i = 0; i < d->u.struct_def.fields.len; i++)
            if (type_refs_struct(d->u.struct_def.fields.data[i]->u.field_decl.type, name))
                return true;
        return false;
    case AST_FUNC_DEF:
        if (type_refs_struct(d->u.func_def.ret_type, name)) return true;
        for (size_t i = 0; i < d->u.func_def.params.len; i++)
            if (type_refs_struct(d->u.func_def.params.data[i]->u.param_decl.type, name))
                return true;
        return false;
    case AST_VAR_DECL:
        return type_refs_struct(d->u.var_decl.type, name);
    case AST_TYPEDEF_DECL:
        return type_refs_struct(d->u.typedef_decl.target, name);
    default: return false;
    }
}

/* Emit method functions for a struct def (Sharp → C free functions). */

/* Returns true if the function body references 'this' (method),
 * false if it never does (associated function). */
static bool cg_stmt_uses_this(const AstNode *stmt);
static bool cg_stmt_uses_name(const AstNode *stmt, const char *name);
static bool cg_expr_uses_this(const AstNode *expr) {
    if (!expr) return false;
    if (expr->kind == AST_IDENT && expr->u.ident.name &&
        strcmp(expr->u.ident.name, "this") == 0) return true;
    /* Recursively check sub-expressions */
    switch (expr->kind) {
    case AST_BINOP:
        return cg_expr_uses_this(expr->u.binop.lhs) ||
               cg_expr_uses_this(expr->u.binop.rhs);
    case AST_UNARY:
        return cg_expr_uses_this(expr->u.unary.operand);
    case AST_CALL: {
        if (cg_expr_uses_this(expr->u.call.callee)) return true;
        for (size_t i = 0; i < expr->u.call.args.len; i++)
            if (cg_expr_uses_this(expr->u.call.args.data[i])) return true;
        return false;
    }
    case AST_METHOD_CALL: {
        if (cg_expr_uses_this(expr->u.method_call.recv)) return true;
        for (size_t i = 0; i < expr->u.method_call.args.len; i++)
            if (cg_expr_uses_this(expr->u.method_call.args.data[i])) return true;
        return false;
    }
    case AST_FIELD_ACCESS:  return cg_expr_uses_this(expr->u.field_access.recv);
    case AST_INDEX:
        return cg_expr_uses_this(expr->u.index_.base) ||
               cg_expr_uses_this(expr->u.index_.index);
    case AST_CAST:
        return cg_expr_uses_this(expr->u.cast.operand);
    case AST_PAREN:
        return cg_expr_uses_this(expr->u.paren.inner);
    case AST_TERNARY:
        return cg_expr_uses_this(expr->u.ternary.cond) ||
               cg_expr_uses_this(expr->u.ternary.then_) ||
               cg_expr_uses_this(expr->u.ternary.else_);
case AST_STRUCT_LIT: {
        for (size_t i = 0; i < expr->u.struct_lit.field_vals.len; i++)
            if (cg_expr_uses_this(expr->u.struct_lit.field_vals.data[i])) return true;
        return false;
    }
    case AST_COMMA:
        return cg_expr_uses_this(expr->u.comma.lhs) ||
               cg_expr_uses_this(expr->u.comma.rhs);
    case AST_SIZEOF:
        return !expr->u.sizeof_.is_type &&
               cg_expr_uses_this(expr->u.sizeof_.operand);
    case AST_INIT_LIST: {
        for (size_t i = 0; i < expr->u.init_list.items.len; i++)
            if (cg_expr_uses_this(expr->u.init_list.items.data[i])) return true;
        return false;
    }
    case AST_DESIGNATED_INIT:
        return cg_expr_uses_this(expr->u.designated_init.value);
    case AST_COMPOUND_LIT:
        return cg_expr_uses_this(expr->u.compound_lit.init);
    case AST_AT_INTRINSIC: {
        for (size_t i = 0; i < expr->u.at_intrinsic.args.len; i++)
            if (cg_expr_uses_this(expr->u.at_intrinsic.args.data[i])) return true;
        return false;
    }
    case AST_STMT_EXPR:
        return cg_stmt_uses_this(expr->u.stmt_expr.block);
    case AST_GENERIC_EXPR: {
        if (cg_expr_uses_this(expr->u.generic_expr.controlling)) return true;
        for (size_t i = 0; i < expr->u.generic_expr.associations.len; i++)
            if (cg_expr_uses_this(expr->u.generic_expr.associations.data[i])) return true;
        return false;
    }
    case AST_GENERIC_ASSOC:
        return cg_expr_uses_this(expr->u.generic_assoc.value);
    case AST_GENERIC_CALL: {
        for (size_t i = 0; i < expr->u.generic_call.call_args.len; i++)
            if (cg_expr_uses_this(expr->u.generic_call.call_args.data[i])) return true;
        return false;
    }
    default: return false;
    }
}

static bool cg_stmt_uses_this(const AstNode *stmt) {
    if (!stmt) return false;
    switch (stmt->kind) {
    case AST_RETURN:    return cg_expr_uses_this(stmt->u.return_.value);
    case AST_EXPR_STMT: return cg_expr_uses_this(stmt->u.expr_stmt.expr);
    case AST_BLOCK:     return cg_block_uses_this(stmt);
    case AST_IF:
        return cg_expr_uses_this(stmt->u.if_.cond) ||
               cg_stmt_uses_this(stmt->u.if_.then_) ||
               cg_stmt_uses_this(stmt->u.if_.else_);
    case AST_WHILE:
        return cg_expr_uses_this(stmt->u.while_.cond) ||
               cg_stmt_uses_this(stmt->u.while_.body);
    case AST_DO_WHILE:
        return cg_expr_uses_this(stmt->u.do_while.cond) ||
               cg_stmt_uses_this(stmt->u.do_while.body);
    case AST_FOR:
        return cg_stmt_uses_this(stmt->u.for_.init) ||
               cg_expr_uses_this(stmt->u.for_.cond) ||
               cg_expr_uses_this(stmt->u.for_.post) ||
               cg_stmt_uses_this(stmt->u.for_.body);
    case AST_DECL_STMT:
        if (stmt->u.decl_stmt.decl && stmt->u.decl_stmt.decl->kind == AST_VAR_DECL)
            return cg_expr_uses_this(stmt->u.decl_stmt.decl->u.var_decl.init);
        return false;
    case AST_DEFER:     return cg_stmt_uses_this(stmt->u.defer_.body);
    case AST_SWITCH:
        return cg_expr_uses_this(stmt->u.switch_.cond) ||
               cg_stmt_uses_this(stmt->u.switch_.body);
    case AST_COMPUTED_GOTO:
        return cg_expr_uses_this(stmt->u.computed_goto.target);
    case AST_CASE:
        return cg_expr_uses_this(stmt->u.case_.value);
    case AST_LABEL:
        return false;
    default: return false;
    }
}

static bool cg_block_uses_this(const AstNode *block) {
    if (!block || block->kind != AST_BLOCK) return false;
    for (size_t i = 0; i < block->u.block.stmts.len; i++)
        if (cg_stmt_uses_this(block->u.block.stmts.data[i])) return true;
    return false;
}

static bool cg_fn_uses_this(const AstNode *fn) {
    if (!fn || fn->kind != AST_FUNC_DEF) return false;
    return cg_block_uses_this(fn->u.func_def.body);
}

static bool cg_expr_uses_name(const AstNode *expr, const char *name) {
    if (!expr || !name) return false;
    if (expr->kind == AST_IDENT && expr->u.ident.name &&
        strcmp(expr->u.ident.name, name) == 0) return true;
    switch (expr->kind) {
    case AST_BINOP:
        return cg_expr_uses_name(expr->u.binop.lhs, name) ||
               cg_expr_uses_name(expr->u.binop.rhs, name);
    case AST_UNARY:
        return cg_expr_uses_name(expr->u.unary.operand, name);
    case AST_CALL: {
        if (cg_expr_uses_name(expr->u.call.callee, name)) return true;
        for (size_t i = 0; i < expr->u.call.args.len; i++)
            if (cg_expr_uses_name(expr->u.call.args.data[i], name)) return true;
        return false;
    }
    case AST_METHOD_CALL: {
        if (cg_expr_uses_name(expr->u.method_call.recv, name)) return true;
        for (size_t i = 0; i < expr->u.method_call.args.len; i++)
            if (cg_expr_uses_name(expr->u.method_call.args.data[i], name)) return true;
        return false;
    }
    case AST_FIELD_ACCESS: return cg_expr_uses_name(expr->u.field_access.recv, name);
    case AST_INDEX:
        return cg_expr_uses_name(expr->u.index_.base, name) ||
               cg_expr_uses_name(expr->u.index_.index, name);
    case AST_CAST:
        return cg_expr_uses_name(expr->u.cast.operand, name);
    case AST_PAREN:
        return cg_expr_uses_name(expr->u.paren.inner, name);
    case AST_TERNARY:
        return cg_expr_uses_name(expr->u.ternary.cond, name) ||
               cg_expr_uses_name(expr->u.ternary.then_, name) ||
               cg_expr_uses_name(expr->u.ternary.else_, name);
    case AST_STRUCT_LIT: {
        for (size_t i = 0; i < expr->u.struct_lit.field_vals.len; i++)
            if (cg_expr_uses_name(expr->u.struct_lit.field_vals.data[i], name)) return true;
        return false;
    }
    case AST_INIT_LIST: {
        for (size_t i = 0; i < expr->u.init_list.items.len; i++)
            if (cg_expr_uses_name(expr->u.init_list.items.data[i], name)) return true;
        return false;
    }
    case AST_DESIGNATED_INIT:
        return cg_expr_uses_name(expr->u.designated_init.value, name);
    case AST_COMPOUND_LIT:
        return cg_expr_uses_name(expr->u.compound_lit.init, name);
    case AST_COMMA:
        return cg_expr_uses_name(expr->u.comma.lhs, name) ||
               cg_expr_uses_name(expr->u.comma.rhs, name);
    case AST_SIZEOF:
        return !expr->u.sizeof_.is_type &&
               cg_expr_uses_name(expr->u.sizeof_.operand, name);
    case AST_AT_INTRINSIC: {
        for (size_t i = 0; i < expr->u.at_intrinsic.args.len; i++)
            if (cg_expr_uses_name(expr->u.at_intrinsic.args.data[i], name)) return true;
        return false;
    }
    case AST_STMT_EXPR:
        return cg_stmt_uses_name(expr->u.stmt_expr.block, name);
    case AST_GENERIC_EXPR: {
        if (cg_expr_uses_name(expr->u.generic_expr.controlling, name)) return true;
        for (size_t i = 0; i < expr->u.generic_expr.associations.len; i++)
            if (cg_expr_uses_name(expr->u.generic_expr.associations.data[i], name)) return true;
        return false;
    }
    case AST_GENERIC_ASSOC:
        return cg_expr_uses_name(expr->u.generic_assoc.value, name);
    case AST_GENERIC_CALL: {
        for (size_t i = 0; i < expr->u.generic_call.call_args.len; i++)
            if (cg_expr_uses_name(expr->u.generic_call.call_args.data[i], name)) return true;
        return false;
    }
    default: return false;
    }
}

static bool cg_stmt_uses_name(const AstNode *stmt, const char *name) {
    if (!stmt || !name) return false;
    switch (stmt->kind) {
    case AST_RETURN:    return cg_expr_uses_name(stmt->u.return_.value, name);
    case AST_EXPR_STMT: return cg_expr_uses_name(stmt->u.expr_stmt.expr, name);
    case AST_BLOCK:     return cg_block_uses_name(stmt, name);
    case AST_IF:
        return cg_expr_uses_name(stmt->u.if_.cond, name) ||
               cg_stmt_uses_name(stmt->u.if_.then_, name) ||
               cg_stmt_uses_name(stmt->u.if_.else_, name);
    case AST_WHILE:
        return cg_expr_uses_name(stmt->u.while_.cond, name) ||
               cg_stmt_uses_name(stmt->u.while_.body, name);
    case AST_DO_WHILE:
        return cg_expr_uses_name(stmt->u.do_while.cond, name) ||
               cg_stmt_uses_name(stmt->u.do_while.body, name);
    case AST_FOR:
        return cg_stmt_uses_name(stmt->u.for_.init, name) ||
               cg_expr_uses_name(stmt->u.for_.cond, name) ||
               cg_expr_uses_name(stmt->u.for_.post, name) ||
               cg_stmt_uses_name(stmt->u.for_.body, name);
    case AST_DECL_STMT:
        if (stmt->u.decl_stmt.decl && stmt->u.decl_stmt.decl->kind == AST_VAR_DECL)
            return cg_expr_uses_name(stmt->u.decl_stmt.decl->u.var_decl.init, name);
        return false;
    case AST_DEFER:     return cg_stmt_uses_name(stmt->u.defer_.body, name);
    case AST_SWITCH:
        return cg_expr_uses_name(stmt->u.switch_.cond, name) ||
               cg_stmt_uses_name(stmt->u.switch_.body, name);
    case AST_COMPUTED_GOTO:
        return cg_expr_uses_name(stmt->u.computed_goto.target, name);
    case AST_CASE:
        return cg_expr_uses_name(stmt->u.case_.value, name);
    case AST_LABEL:
        return false;
    default: return false;
    }
}

static bool cg_block_uses_name(const AstNode *block, const char *name) {
    if (!block || block->kind != AST_BLOCK) return false;
    for (size_t i = 0; i < block->u.block.stmts.len; i++)
        if (cg_stmt_uses_name(block->u.block.stmts.data[i], name)) return true;
    return false;
}

static void cg_emit_methods(CgCtx *ctx, const AstNode *sd) {
    if (!sd || sd->kind != AST_STRUCT_DEF) return;
    const char *sname = sd->u.struct_def.name;
    for (size_t m = 0; m < sd->u.struct_def.methods.len; m++) {
        AstNode *fn = sd->u.struct_def.methods.data[m];
        if (!fn || fn->u.func_def.generic_params.len > 0) continue;
        cg_emit_linemarker(ctx, fn->loc);
        cg_func(ctx, fn, sname);
        cg_nl(ctx);
    }
}

/* Emit one top-level declaration in Sharp mode. */
static void cg_emit_decl_sharp(CgCtx *ctx, AstNode *d) {
    if (!d) return;
    CppLoc saved_loc = d->loc;
    cg_emit_linemarker(ctx, saved_loc);

    switch (d->kind) {
    case AST_STRUCT_DEF: {
        if (d->u.struct_def.generic_params.len > 0) return; /* generic template */
        if (d->u.struct_def.is_synthetic) return;
        /* Structs defined inline inside a typedef (`typedef struct X{...} X;`)
         * have from_inline_typedef=true.  cg_typedef_c will emit the complete
         * `typedef struct X { ... } X;` form, so skip standalone emission here
         * to avoid emitting the body twice. */
        if (d->u.struct_def.from_inline_typedef) return;
        /* Nested structs defined inside another struct body are already
         * emitted inline when the parent struct is emitted. */
        if (d->u.struct_def.is_nested_in_struct &&
            d->u.struct_def.name &&
            strncmp(d->u.struct_def.name, "__anon_", 7) == 0) return;
        const char *kw = struct_kw(d);
        const char *nm = d->u.struct_def.name;
        const char *enm = nm;
        bool has_body = d->u.struct_def.has_body ||
                        d->u.struct_def.methods.len > 0;

        /* class: emit `typedef struct X X;` before body so bare name works */
        if (d->u.struct_def.is_class && has_body && enm) {
            cg_printf(ctx, "typedef %s %s %s;\n", kw, enm, enm);
        }
        /* Forward declaration (e.g., `struct tm;`) — must be emitted so
         * that functions using `struct tm *` parameters compile correctly. */
        if (!has_body) {
            if (enm)
                cg_printf(ctx, "%s %s;\n", kw, enm);
            else
                cg_printf(ctx, "%s %s;\n", kw, nm);
            break;
        }
        if (has_body) {
            cg_printf(ctx, "%s %s {\n", kw, enm);
            for (size_t i = 0; i < d->u.struct_def.fields.len; i++) {
                cg_puts(ctx, "    ");
                cg_field_decl_from_ast(ctx, d->u.struct_def.fields.data[i]);
            }
            if (d->u.struct_def.tail_attrs || d->u.struct_def.leading_attrs)
                cg_printf(ctx, "} %s;\n",
                    d->u.struct_def.tail_attrs ? d->u.struct_def.tail_attrs : d->u.struct_def.leading_attrs);
            else
                cg_puts(ctx, "};\n");
            cg_emit_methods(ctx, d);
        }
        break;
    }
    case AST_FUNC_DEF:
        if (d->u.func_def.generic_params.len > 0) return; /* generic template */
        cg_func(ctx, d, d->u.func_def.struct_name);
        break;
    case AST_VAR_DECL:
        cg_var_c(ctx, d);
        break;
    case AST_EXTERN_INST:
        /* Explicit instantiation: no C output -- collect pass already
         * emitted the specialised definition via collect_type. */
        break;
    case AST_TYPEDEF_DECL:
        cg_typedef_c(ctx, d);
        break;
    case AST_ENUM_DEF:
        /* Enums defined inline inside a typedef (`typedef enum { ... } Alias;`)
         * have inline_typedef=true.  cg_typedef_c will emit the complete
         * `typedef enum { ... } Alias;` form, so skip standalone emission here
         * to avoid emitting the enumerators twice. */
        if (d->u.enum_def.inline_typedef) return;
        cg_enum_c(ctx, d);
        break;
    case AST_GCC_VERBATIM:
        cg_puts(ctx, d->u.gcc_verbatim.text);
        if (!d->u.gcc_verbatim.is_stmt) cg_puts(ctx, ";");
        cg_nl(ctx);
        break;
    default:
        break;
    }
}

typedef struct { const char *alias; size_t pos; const AstNode *decl; } TdPos;

static bool cg_td_refs(size_t tgt_idx, const TdPos *tds, size_t ntds,
                        const char *name, CgCtx *ctx) {
    const AstNode *_td = tds[tgt_idx].decl;
    const AstNode *_tgt = _td->u.typedef_decl.target;
    if (type_refs_typedef(_tgt, name)) return true;
    if (_tgt && _tgt->kind == AST_TYPE_NAME && _tgt->u.type_name.name) {
        Symbol *_sym = ctx->file_scope ?
            scope_lookup_struct_tag(ctx->file_scope, _tgt->u.type_name.name) : NULL;
        if (_sym && _sym->decl && _sym->decl->kind == AST_STRUCT_DEF
            && _sym->decl->u.struct_def.from_inline_typedef) {
            for (size_t _fi = 0; _fi < _sym->decl->u.struct_def.fields.len; _fi++) {
                const AstNode *_f = _sym->decl->u.struct_def.fields.data[_fi];
                if (_f && _f->kind == AST_FIELD_DECL && _f->u.field_decl.type)
                    if (type_refs_typedef(_f->u.field_decl.type, name)) return true;
            }
        }
    }
    return false;
}

static void cg_emit_td_fwd(CgCtx *ctx, size_t ti,
                            TdPos *tds, size_t ntds, bool *td_fwd) {
    if (ti >= ntds || td_fwd[ti]) return;
    size_t nodes[256];
    size_t nnodes = 0;
    size_t bfs[256];
    size_t nbfs = 0;
    bfs[nbfs++] = ti;
    td_fwd[ti] = true;
    nodes[nnodes++] = ti;
    for (size_t qi = 0; qi < nbfs && nbfs < 256; qi++) {
        size_t ci = bfs[qi];
        const AstNode *_td = tds[ci].decl;
        const AstNode *_tgt = _td->u.typedef_decl.target;
        for (size_t _di = 0; _di < ntds; _di++) {
            if (td_fwd[_di]) continue;
            if (type_refs_typedef(_tgt, tds[_di].alias)) {
                td_fwd[_di] = true;
                bfs[nbfs++] = _di;
                nodes[nnodes++] = _di;
            }
        }
        if (_tgt && _tgt->kind == AST_TYPE_NAME && _tgt->u.type_name.name) {
            Symbol *_sym = ctx->file_scope ?
                scope_lookup_struct_tag(ctx->file_scope, _tgt->u.type_name.name) : NULL;
            if (_sym && _sym->decl && _sym->decl->kind == AST_STRUCT_DEF
                && _sym->decl->u.struct_def.from_inline_typedef) {
                for (size_t _fi = 0; _fi < _sym->decl->u.struct_def.fields.len; _fi++) {
                    const AstNode *_f = _sym->decl->u.struct_def.fields.data[_fi];
                    if (_f && _f->kind == AST_FIELD_DECL && _f->u.field_decl.type) {
                        for (size_t _di = 0; _di < ntds; _di++) {
                            if (td_fwd[_di]) continue;
                            if (type_refs_typedef(_f->u.field_decl.type, tds[_di].alias)) {
                                td_fwd[_di] = true;
                                bfs[nbfs++] = _di;
                                nodes[nnodes++] = _di;
                            }
                        }
                    }
                }
            }
        }
    }
    bool done[256];
    memset(done, 0, nnodes * sizeof(bool));
    size_t emit_count = 0;
    while (emit_count < nnodes) {
        bool progress = false;
        for (size_t i = 0; i < nnodes; i++) {
            if (done[i]) continue;
            size_t idx = nodes[i];
            bool ready = true;
            for (size_t j = 0; j < nnodes && ready; j++) {
                if (done[j] || i == j) continue;
                if (cg_td_refs(idx, tds, ntds, tds[nodes[j]].alias, ctx))
                    ready = false;
            }
            if (ready) {
                cg_emit_decl_sharp(ctx, (AstNode *)tds[idx].decl);
                done[i] = true;
                emit_count++;
                progress = true;
            }
        }
        if (!progress) break;
    }
}

static void cg_file(CgCtx *ctx, const AstNode *file) {
    if (!file) return;
    ctx->root_file = file->u.file.path;
    ctx->file_ast  = file;

    /* Pre-scan: detect @to_cstr usage and set the flag before codegen. */
    ctx->uses_to_cstr = ast_uses_to_cstr(file);

    /* Emit @to_cstr preamble (sp_str_t typedef + __SP_MAGIC_TO_C_STR macro) */
    cg_emit_to_cstr_preamble(ctx);

    /* ── Sharp-mode multi-pass emitter ──────────────────────────────────
     * Phase 1 → 1.5 → 2 → 3a → 3.5 → 3b → 4                           */

    /* Phase 1: emit forward declaration for structs that are referenced
     * before their definition. Collect defined struct names first. */
    const char **defined = NULL;
    size_t ndefined = 0;
    collect_struct_names(file, &defined, &ndefined);

    /* Track which structs have a forward decl to avoid duplicates in Phase 3. */
    bool *fwd_decl_emitted = calloc(ndefined, sizeof(bool));

    /* For each struct name, find if any earlier decl uses it. */
    for (size_t si = 0; si < ndefined; si++) {
        const char *name = defined[si];
        /* Find position of definition */
        size_t def_pos = file->u.file.decls.len;
        for (size_t i = 0; i < file->u.file.decls.len; i++) {
            const AstNode *d = file->u.file.decls.data[i];
            if (!d) continue;
            const char *dn = NULL;
            if (d->kind == AST_STRUCT_DEF) dn = d->u.struct_def.name;
            else if (d->kind == AST_TYPEDEF_DECL) {
                const AstNode *tgt = d->u.typedef_decl.target;
                if (tgt && tgt->kind == AST_TYPE_NAME &&
                    tgt->u.type_name.name && d->u.typedef_decl.alias &&
                    strcmp(tgt->u.type_name.name, d->u.typedef_decl.alias) == 0)
                    dn = d->u.typedef_decl.alias;
            }
            if (dn && strcmp(dn, name) == 0) { def_pos = i; break; }
        }
        /* Check if any decl before def_pos uses this struct */
        bool needs_fwd = false;
        for (size_t i = 0; i < def_pos && !needs_fwd; i++) {
            const AstNode *d = file->u.file.decls.data[i];
            if (decl_uses_struct(d, name)) needs_fwd = true;
        }
        /* Also check if any earlier fwd_decl_emitted already covers this name */
        bool already_emitted = false;
        for (size_t _ei = 0; _ei < si && !already_emitted; _ei++) {
            if (fwd_decl_emitted[_ei] && defined[_ei] &&
                strcmp(defined[_ei], name) == 0)
                already_emitted = true;
        }
        if (needs_fwd && !already_emitted) {
            cg_emit_struct_fwd_decl(ctx, name);
            fwd_decl_emitted[si] = true;
        }
    }

    /* Phase 1.5: emit forward declarations for non-generic functions that
     * are called before they are defined (forward reference fix). */
    {
        /* Collect all typedef names and their positions in file.decls. */
        TdPos *tds = NULL;
        size_t ntds = 0, tds_cap = 0;
        for (size_t i = 0; i < file->u.file.decls.len; i++) {
            const AstNode *d = file->u.file.decls.data[i];
            if (!d) continue;
            if (d->kind == AST_TYPEDEF_DECL && d->u.typedef_decl.alias) {
                if (ntds == tds_cap) {
                    tds_cap = tds_cap ? tds_cap * 2 : 16;
                    tds = realloc(tds, tds_cap * sizeof *tds);
                    if (!tds) abort();
                }
                tds[ntds].alias = d->u.typedef_decl.alias;
                tds[ntds].pos = i;
                tds[ntds].decl = d;
                ntds++;
            }
        }

        /* Track which typedefs have been forward-declared */
        bool *td_fwd = calloc(ntds, sizeof(bool));

        /* Emit a typedef and all its transitive dependencies in correct
         * topological order (deps before dependents) via post-order DFS.
         * Replaced the old BFS + reverse-emit macro which produced wrong
         * order when cross-level dependencies existed. */
        #define _EMIT_TD_FWD(ti) cg_emit_td_fwd(ctx, ti, tds, ntds, td_fwd)

        /* Helper: for a type AST, find and emit forward decls for any
         * referenced typedefs.  No pos check: even typedefs defined BEFORE
         * the function in source order haven't been emitted yet (Phase 3a
         * runs after Phase 1.5), so we must emit the full typedef here. */
        #define _CHECK_TYPE_TD(ty, fpos) do { \
            for (size_t _ti = 0; _ti < ntds; _ti++) { \
                if (td_fwd[_ti]) continue; \
                if (type_refs_typedef((ty), tds[_ti].alias)) { \
                    _EMIT_TD_FWD(_ti); \
                } \
            } \
        } while(0)

        /* Collect all non-generic function definitions and their positions. */
        typedef struct { const char *name; size_t pos; const AstNode *fn;
                         const char *sname; /* struct name for methods, NULL for free funcs */
                       } FnDef;
        FnDef *fns = NULL;
        size_t nfns = 0, fns_cap = 0;

        for (size_t i = 0; i < file->u.file.decls.len; i++) {
            const AstNode *d = file->u.file.decls.data[i];
            if (!d) continue;
            if (d->kind == AST_FUNC_DEF && d->u.func_def.generic_params.len == 0 &&
                d->u.func_def.body && d->u.func_def.name) {
                if (nfns == fns_cap) {
                    fns_cap = fns_cap ? fns_cap * 2 : 16;
                    fns = realloc(fns, fns_cap * sizeof *fns);
                    if (!fns) abort();
                }
                fns[nfns].name = d->u.func_def.name;
                fns[nfns].pos = i;
                fns[nfns].fn = d;
                /* Extension methods: pass struct_name so cg_func_decl can
                 * mangle the name correctly in the forward declaration. */
                fns[nfns].sname = d->u.func_def.struct_name
                    ? d->u.func_def.struct_name : NULL;
                nfns++;
            }
            /* Also collect struct methods */
            if (d->kind == AST_STRUCT_DEF) {
                const char *sn = d->u.struct_def.name;
                for (size_t m = 0; m < d->u.struct_def.methods.len; m++) {
                    AstNode *fn = d->u.struct_def.methods.data[m];
                    if (fn && fn->u.func_def.generic_params.len == 0 &&
                        fn->u.func_def.body && fn->u.func_def.name) {
                        if (nfns == fns_cap) {
                            fns_cap = fns_cap ? fns_cap * 2 : 16;
                            fns = realloc(fns, fns_cap * sizeof *fns);
                            if (!fns) abort();
                        }
                        fns[nfns].name = fn->u.func_def.name;
                        fns[nfns].pos = i;
                        fns[nfns].fn = fn;
                        fns[nfns].sname = sn;
                        nfns++;
                    }
                }
            }
        }

        /* For each function, check if any earlier function body calls it
         * OR if any global variable initializer references it. */
        for (size_t fi = 0; fi < nfns; fi++) {
            size_t def_pos = fns[fi].pos;
            const char *fname = fns[fi].name;

            bool referenced_before_def = false;
            for (size_t i = 0; i < def_pos && !referenced_before_def; i++) {
                const AstNode *d = file->u.file.decls.data[i];
                if (!d) continue;
                if (d->kind == AST_FUNC_DEF && d->u.func_def.body) {
                    if (cg_block_uses_name(d->u.func_def.body, fname)) {
                        referenced_before_def = true;
                    }
                }
                if (d->kind == AST_STRUCT_DEF) {
                    for (size_t m = 0; m < d->u.struct_def.methods.len; m++) {
                        AstNode *fn = d->u.struct_def.methods.data[m];
                        if (fn && fn->u.func_def.body &&
                            cg_block_uses_name(fn->u.func_def.body, fname)) {
                            referenced_before_def = true;
                            break;
                        }
                    }
                }
                /* Check global variable initializers (before function def) */
                if (d->kind == AST_VAR_DECL && d->u.var_decl.init) {
                    if (cg_expr_uses_name(d->u.var_decl.init, fname)) {
                        referenced_before_def = true;
                    }
                }
            }
            /* Also check global variable initializers AFTER function def --
             * Sharp mode emits globals in Phase 3a, function bodies in 3b,
             * so any var init referencing this function needs a fwd decl. */
            for (size_t i = def_pos + 1; i < file->u.file.decls.len && !referenced_before_def; i++) {
                const AstNode *d = file->u.file.decls.data[i];
                if (!d) continue;
                if (d->kind == AST_VAR_DECL && d->u.var_decl.init) {
                    if (cg_expr_uses_name(d->u.var_decl.init, fname)) {
                        referenced_before_def = true;
                    }
                }
            }

            if (referenced_before_def) {
                /* If the source already provides an explicit bodyless prototype
                 * (AST_FUNC_DEF without body) at an earlier position, Phase 3a
                 * will emit it — no need to inject a duplicate here. */
                bool has_source_proto = false;
                for (size_t i = 0; i < def_pos && !has_source_proto; i++) {
                    const AstNode *d = file->u.file.decls.data[i];
                    if (!d || d->kind != AST_FUNC_DEF || d->u.func_def.body) continue;
                    if (d->u.func_def.name &&
                        strcmp(d->u.func_def.name, fname) == 0)
                        has_source_proto = true;
                }
                if (has_source_proto) continue;
                /* Before emitting the function forward declaration, ensure
                 * all struct types in its signature already have a forward decl. */
                const AstNode *fn = fns[fi].fn;
                size_t fpos = fns[fi].pos;
                /* Emit forward typedefs for any typedefs used in the signature
                 * that are defined AFTER this function. */
                if (fn->u.func_def.ret_type)
                    _CHECK_TYPE_TD(fn->u.func_def.ret_type, fpos);
                for (size_t pi = 0; pi < fn->u.func_def.params.len; pi++) {
                    AstNode *par = fn->u.func_def.params.data[pi];
                    if (par && par->u.param_decl.type)
                        _CHECK_TYPE_TD(par->u.param_decl.type, fpos);
                }
                /* Check return type */
                if (fn->u.func_def.ret_type) {
                    for (size_t si = 0; si < ndefined; si++) {
                        if (type_refs_struct(fn->u.func_def.ret_type, defined[si]) && !fwd_decl_emitted[si]) {
                            bool already_e = false;
                            for (size_t _ei = 0; _ei < si; _ei++)
                                if (fwd_decl_emitted[_ei] && defined[_ei] && strcmp(defined[_ei], defined[si]) == 0)
                                    { already_e = true; break; }
                            if (!already_e) {
                                cg_emit_struct_fwd_decl(ctx, defined[si]);
                                fwd_decl_emitted[si] = true;
                            }
                        }
                    }
                }
                /* Check parameter types */
                for (size_t pi = 0; pi < fn->u.func_def.params.len; pi++) {
                    AstNode *par = fn->u.func_def.params.data[pi];
                    if (par && par->u.param_decl.type) {
                        for (size_t si = 0; si < ndefined; si++) {
                            if (type_refs_struct(par->u.param_decl.type, defined[si]) && !fwd_decl_emitted[si]) {
                                bool already_e = false;
                                for (size_t _ei = 0; _ei < si; _ei++)
                                    if (fwd_decl_emitted[_ei] && defined[_ei] && strcmp(defined[_ei], defined[si]) == 0)
                                        { already_e = true; break; }
                                if (!already_e) {
                                    cg_emit_struct_fwd_decl(ctx, defined[si]);
                                    fwd_decl_emitted[si] = true;
                                }
                            }
                        }
                    }
                }
                #undef _CHECK_TYPE_TD
                #undef _EMIT_TD_FWD
                cg_func_decl(ctx, fns[fi].fn, fns[fi].sname);
            }
        }
        free(fns);

        /* Store typedef aliases emitted in Phase 1.5 so Phase 3a can skip
         * them and avoid redefinition.  Reuse ctx->fwd_typedef_names. */
        if (ntds > 0) {
            size_t cnt = 0;
            for (size_t ti = 0; ti < ntds; ti++)
                if (td_fwd[ti]) cnt++;
            if (cnt > 0) {
                ctx->fwd_typedef_names = malloc((cnt + 1) * sizeof(char *));
                if (!ctx->fwd_typedef_names) abort();
                ctx->n_fwd_typedef_names = cnt;
                size_t j = 0;
                for (size_t ti = 0; ti < ntds; ti++)
                    if (td_fwd[ti]) ctx->fwd_typedef_names[j++] = tds[ti].alias;
                ctx->fwd_typedef_names[cnt] = NULL;
            }
        }
        free(td_fwd);
        free(tds);
    }
    free(fwd_decl_emitted);
    free(defined);

    /* Phase 2: collect generic specializations by walking all function bodies.
     *
     * v0.13: redirect ctx->out to a scratch buffer during collection.
     * cg_emit_specialization is currently eager -- it writes spec definitions
     * directly to ctx->out as it discovers them.  Without redirection,
     * those specs land before Phase 3's user decls (Point, isize, etc.),
     * producing references to undeclared types.  We capture the spec
     * output and replay it after Phase 3. */
    StrBuf saved_out = ctx->out;
    StrBuf spec_buf  = {0};
    ctx->out = spec_buf;
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *raw_d = file->u.file.decls.data[i];
        const AstNode *d = raw_d;
        if (!d) continue;
        if (d->kind == AST_FUNC_DEF && d->u.func_def.generic_params.len == 0)
            cg_collect_block(ctx, d->u.func_def.body);
        else if (d->kind == AST_STRUCT_DEF)
            for (size_t m = 0; m < d->u.struct_def.methods.len; m++)
                cg_collect_block(ctx, d->u.struct_def.methods.data[m]->u.func_def.body);
        if (raw_d && raw_d->kind == AST_EXTERN_INST) {
            AstNode *typ = raw_d->u.extern_inst.type;
            if (typ && typ->kind == AST_TYPE_GENERIC) {
                const char *fname = typ->u.type_generic.name;
                if (fname) {
                    Symbol *sym = scope_lookup(ctx->file_scope, fname);
                    if (sym && sym->kind == SYM_FUNC && sym->decl &&
                        sym->decl->kind == AST_FUNC_DEF &&
                        sym->decl->u.func_def.generic_params.len > 0) {
                        const AstNode *fn = sym->decl;
                        size_t np = fn->u.func_def.generic_params.len;
                        size_t na = typ->u.type_generic.args.len;
                        if (na == np) {
                            Type **targs = calloc(np, sizeof *targs);
                            if (targs) {
                                bool all_ok = true;
                                for (size_t j = 0; j < na; j++) {
                                    targs[j] = ty_from_ast(ctx->ts,
                                        typ->u.type_generic.args.data[j],
                                        cg_type_scope(ctx), NULL);
                                    if (!targs[j] || cg_type_has_params(targs[j])) all_ok = false;
                                }
                                if (all_ok) {
                                    StrBuf sb = {0};
                                    sb_push_cstr(&sb, fn->u.func_def.name);
                                    for (size_t j = 0; j < np; j++) {
                                        sb_push_cstr(&sb, "__");
                                        ty_mangle(&sb, targs[j]);
                                    }
                                    char *mn = sb_take(&sb);
                                    gfinst_push(ctx, fn, mn, targs, np);
                                    free(mn);
                                }
                                free(targs);
                            }
                        }
                    }
                }
            }
        }
    }

    /* Fixpoint: walk newly-discovered generic function instances to find
     * transitive dependencies.  For example, main() calls
     * HashMap<Point,int>.insert() which calls ._grow_if_needed() --
     * only the direct call is discovered during the initial body walk
     * above because generic function bodies are not walked there (they
     * contain unresolved TY_PARAMs).  We iterate until no new instances
     * are discovered, resolving TY_PARAMs through each instance's type
     * substitution context. */
    size_t prev_ngfinsts;
    do {
        prev_ngfinsts = ctx->ngfinsts;
        for (size_t i = 0; i < ctx->ngfinsts; i++) {
            struct GFuncInst_ *gi = &ctx->gfinsts[i];
            if (gi->walked) continue;
            gi->walked = true;
            const AstNode *fn = gi->fn;
            if (!fn || fn->kind != AST_FUNC_DEF || !fn->u.func_def.body)
                continue;
            size_t np = fn->u.func_def.generic_params.len;
            /* Extension methods use the struct's generic params,
             * not their own. */
            if (np == 0) { np = gi->ntargs; if (!gi->targs || np == 0) continue; }
            if (!gi->targs) continue;

            /* Build pnames from the template definition. */
            const char **pnames = malloc(np * sizeof *pnames);
            if (!pnames) abort();
            if (fn->u.func_def.generic_params.len > 0) {
                for (size_t k = 0; k < np; k++)
                    pnames[k] = fn->u.func_def.generic_params.data[k]
                                    ->u.generic_param.name;
            } else if (fn->u.func_def.struct_name && ctx->file_scope) {
                cg_fill_pnames_from_struct(ctx, fn->u.func_def.struct_name, pnames, (int)np);
            }

            /* Install gp context and walk the template body.
             * cg_collect_expr's AST_METHOD_CALL handler will resolve
             * TY_PARAM struct args through gp_names/gp_vals and push
             * newly-discovered instances (e.g. _grow_if_needed). */
            const char **saved_pnames = ctx->gp_names;
            Type       **saved_pvals  = ctx->gp_vals;
            size_t       saved_np     = ctx->ngp;
            ctx->gp_names = pnames;
            ctx->gp_vals  = gi->targs;
            ctx->ngp      = np;

            /* Annotate the template body with sem_types so
             * cg_collect_expr's AST_METHOD_CALL handler can
             * resolve the receiver type. */
            Scope *mscope = fn->sem_scope ? fn->sem_scope : ctx->file_scope;
            sema_func_template_body(ctx->ts, mscope, NULL, (AstNode*)fn);
            cg_collect_block(ctx, fn->u.func_def.body);

            ctx->gp_names = saved_pnames;
            ctx->gp_vals  = saved_pvals;
            ctx->ngp      = saved_np;
            free(pnames);
        }
    } while (ctx->ngfinsts > prev_ngfinsts);

    /* End of Phase 2 collection. Pull the captured spec output back out
     * and restore ctx->out to the main buffer.  Specs will be emitted
     * after Phase 3 below, so they reference user decls already defined. */
    StrBuf captured_specs = ctx->out;
    ctx->out = saved_out;

    /* Phase 3a: emit non-function user decls (structs, typedefs, enums,
     * globals) AND forward declarations for struct methods (so spec
     * bodies in Phase 3.5 can call them).  Free-function declarations
     * are left to Phase 3b in source order, matching the original output
     * shape (so existing .ref.i golden files don't drift).
     *
     * Extension method forward declarations are emitted AFTER struct
     * definitions (unlike struct methods) so that the C compiler sees
     * complete types for by-value parameters.
     *
     * Typedefs that reference generic struct specializations are deferred
     * so that Phase 3.5 emits the struct definitions first. */
    
    /* Deferred typedefs that reference generic struct specializations. */
    AstNode **deferred_tds = NULL;
    size_t deferred_td_count = 0, deferred_td_cap = 0;
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (!decl_is_user(ctx, d)) {
            /* System-header declarations are already provided by the
             * #include directives emitted from user_includes.  Skip them
             * entirely to avoid redefinition conflicts. */
            continue;
        }
        AstNode *uw = d;
        if (!uw) continue;
        if (uw->kind == AST_FUNC_DEF) {
            /* Skip generic templates entirely. */
            if (uw->u.func_def.generic_params.len > 0) continue;
            /* Body-less function decls (e.g. `void* malloc(unsigned long);`)
             * are pure forward declarations -- emit them in Phase 3a so
             * spec bodies in Phase 3.5 can call them. */
            if (!uw->u.func_def.body) {
                cg_func(ctx, uw, NULL);
                continue;
            }
            /* Extension methods: emit forward declaration inline
             * (in source order, after the struct they belong to),
             * deferring the body to Phase 3b.
             * Skip extension methods that belong to generic structs —
             * Phase 11 (cg_emit_gfunc_fwd_decls) handles their
             * specializations with proper type substitution. */
            if (uw->u.func_def.struct_name) {
                Symbol *ss = scope_lookup_type(ctx->file_scope,
                    uw->u.func_def.struct_name);
                if (ss && ss->decl &&
                    ss->decl->kind == AST_STRUCT_DEF &&
                    ss->decl->u.struct_def.generic_params.len > 0)
                    continue;
                const char *snm =
                    uw->u.func_def.struct_name;
                cg_func_decl(ctx, uw, snm);
                continue;
            }
            /* Full definitions deferred to Phase 3b in source order. */
            continue;
        }
        if (uw->kind == AST_STRUCT_DEF &&
            uw->u.struct_def.methods.len > 0) {
            /* Skip generic templates and synthetic structs entirely. */
            if (uw->u.struct_def.generic_params.len > 0) continue;
            if (uw->u.struct_def.is_synthetic) continue;
            /* Struct with methods: we emit the struct body now but defer
             * the method bodies to Phase 3b (after specs).  Methods on
             * non-generic structs may reference specialised types. */
            const char *sname = uw->u.struct_def.name;
            const char *enm = sname;
            const char *kw = struct_kw(uw);
            /* class: emit typedef first so bare name works */
            if (uw->u.struct_def.is_class && uw->u.struct_def.has_body && enm) {
                cg_printf(ctx, "typedef %s %s %s;\n", kw, enm, enm);
            }
            if (uw->u.struct_def.has_body || uw->u.struct_def.fields.len > 0) {
                cg_printf(ctx, "%s %s {\n", kw, enm);
                for (size_t fi = 0; fi < uw->u.struct_def.fields.len; fi++) {
                    cg_puts(ctx, "    ");
                    cg_field_decl_from_ast(ctx, uw->u.struct_def.fields.data[fi]);
                }
                if (uw->u.struct_def.tail_attrs || uw->u.struct_def.leading_attrs)
                    cg_printf(ctx, "} %s;\n",
                        uw->u.struct_def.tail_attrs ? uw->u.struct_def.tail_attrs : uw->u.struct_def.leading_attrs);
                else
                    cg_puts(ctx, "};\n");
            }
            /* Forward-declare every method so spec bodies (Phase 3.5)
             * can call them.  cg_func_decl emits `RetType S__method(...)
             * ;` without the body. */
            ctx->cur_struct = enm;
            ctx->cur_struct_def = uw;
            for (size_t mi = 0; mi < uw->u.struct_def.methods.len; mi++) {
                AstNode *m = uw->u.struct_def.methods.data[mi];
                if (!m || m->kind != AST_FUNC_DEF) continue;
                if (m->u.func_def.generic_params.len > 0) continue;
                cg_func_decl(ctx, m, enm);
            }
            ctx->cur_struct = NULL;
            ctx->cur_struct_def = NULL;
            continue;
        }
        if (uw->kind == AST_TYPEDEF_DECL && uw->u.typedef_decl.target) {
            AstNode *td_target = uw->u.typedef_decl.target;
            /* Check if this typedef (directly or through a chain of
             * aliases) references a generic type instantiation.  If so,
             * defer it until Phase 3.5 has emitted the synthesized
             * struct definitions. */
            bool is_generic_td = false;
            if (td_target->kind == AST_TYPE_GENERIC) {
                is_generic_td = true;
            } else if (td_target->kind == AST_TYPE_NAME &&
                       td_target->u.type_name.name && ctx->file_scope) {
                /* Follow typedef chains: e.g. typedef IntVec MyIntVec
                 * where IntVec was defined as typedef Vec<int> IntVec. */
                Symbol *ts = scope_lookup_type(ctx->file_scope,
                                               td_target->u.type_name.name);
                if (ts && ts->decl && ts->decl->kind == AST_TYPEDEF_DECL &&
                    ts->decl->u.typedef_decl.target) {
                    AstNode *chain_target = ts->decl->u.typedef_decl.target;
                    while (chain_target->kind == AST_TYPE_PTR)
                        chain_target = chain_target->u.type_ptr.base;
                    if (chain_target->kind == AST_TYPE_GENERIC)
                        is_generic_td = true;
                }
            }
            if (is_generic_td) {
                if (deferred_td_count == deferred_td_cap) {
                    deferred_td_cap = deferred_td_cap ? deferred_td_cap * 2 : 4;
                    deferred_tds = realloc(deferred_tds,
                        deferred_td_cap * sizeof *deferred_tds);
                    if (!deferred_tds) abort();
                }
                deferred_tds[deferred_td_count++] = d;
                continue;
            }
            /* Skip typedefs already fully emitted in Phase 1.5 (forward ref fix) */
            if (ctx->n_fwd_typedef_names > 0 && d->u.typedef_decl.alias) {
                for (size_t _fi = 0; _fi < ctx->n_fwd_typedef_names; _fi++) {
                    if (strcmp(ctx->fwd_typedef_names[_fi], d->u.typedef_decl.alias) == 0)
                        goto skip_td;
                }
            }
        }
        cg_emit_decl_sharp(ctx, d);
skip_td:;
    }

    /* Phase 3.5: now that user typedefs/structs are emitted, dump the
     * generic struct specializations collected in Phase 2.  This guarantees
     * that `HashMap__Point__int { HashMapEntry__Point__int* entries; ... }`
     * has all referenced types already declared. */
    if (captured_specs.buf && captured_specs.len > 0) {
        sb_push_cstr(&ctx->out, captured_specs.buf);
    }
    free(captured_specs.buf);
    
    /* Emit deferred typedefs that reference generic struct specializations.
     * These were deferred from Phase 3a so the struct definition exists. */
    for (size_t i = 0; i < deferred_td_count; i++)
        cg_emit_decl_sharp(ctx, deferred_tds[i]);
    free(deferred_tds);

    /* Phase 3.6: emit forward declarations for collected generic function
     * specializations.  Placed AFTER Phase 3.5 struct specializations so
     * that return types like `Pair__int__int make_pair__int__int(...)` have
     * the struct already defined.  Must still come before Phase 3b function
     * bodies so call sites have prototypes. */
    cg_emit_gfunc_fwd_decls(ctx);

    /* Phase 3b: emit user function bodies (and struct method bodies).
     * Deferred to this point so they can see both the user structs from
     * Phase 3a and the specialisation full definitions from Phase 3.5. */
    for (size_t i = 0; i < file->u.file.decls.len; i++) {
        AstNode *d = file->u.file.decls.data[i];
        if (!d) continue;
        if (!decl_is_user(ctx, d)) continue;
        AstNode *uw = d;
        if (uw && uw->kind == AST_FUNC_DEF) {
            /* Skip extension methods of generic structs --
             * Phase 11 handles their specializations. */
            const AstNode *method_struct_def = NULL;
            if (uw->u.func_def.struct_name && ctx->file_scope) {
                Symbol *ss = scope_lookup_type(ctx->file_scope,
                    uw->u.func_def.struct_name);
                for (Symbol *s = ss; s; s = s->next) {
                    if (s->kind != SYM_TYPE || !s->decl) continue;
                    if (s->decl->kind == AST_STRUCT_DEF) {
                        if (s->decl->u.struct_def.generic_params.len > 0)
                            continue;
                        method_struct_def = s->decl;
                        break;
                    }
                }
            }
            const char *prev_struct = ctx->cur_struct;
            const AstNode *prev_struct_def = ctx->cur_struct_def;
            ctx->cur_struct = uw->u.func_def.struct_name;
            ctx->cur_struct_def = method_struct_def;
            cg_emit_decl_sharp(ctx, d);
            ctx->cur_struct = prev_struct;
            ctx->cur_struct_def = prev_struct_def;
        } else if (uw && uw->kind == AST_STRUCT_DEF &&
                   uw->u.struct_def.methods.len > 0) {
            /* Skip generic templates and synthetic structs entirely. */
            if (uw->u.struct_def.generic_params.len > 0) continue;
            if (uw->u.struct_def.is_synthetic) continue;
            const char *sname = uw->u.struct_def.name;
            const char *enm = sname;
            ctx->cur_struct = enm;
            ctx->cur_struct_def = uw;
            for (size_t mi = 0; mi < uw->u.struct_def.methods.len; mi++) {
                AstNode *m = uw->u.struct_def.methods.data[mi];
                if (!m || m->kind != AST_FUNC_DEF) continue;
                if (m->u.func_def.generic_params.len > 0) continue;
                cg_func(ctx, m, enm);
                cg_nl(ctx);
            }
            ctx->cur_struct = NULL;
            ctx->cur_struct_def = NULL;
        }
    }

    /* Phase 4: emit all collected generic function specializations. */
    cg_emit_gfunc_specs(ctx);
}

char *cg_generate(CgCtx *ctx, const AstNode *file) {
    cg_file(ctx, file);
    return sb_take(&ctx->out);
}
