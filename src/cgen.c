/* cgen.c - Sharp AST -> C source emitter (phases 1+2).
 *
 * Strategy:
 *   1. Emit prelude with primitive typedefs.
 *   2. Emit forward declarations for every struct and every (free + method) function.
 *   3. Emit struct bodies.
 *   4. Emit function / method bodies.
 *
 * Phase-2 mechanics:
 *   - Methods live in impl blocks. Each method becomes a C function named
 *     `ParentType_methodName`, with `static` storage. A value-self method
 *     takes `ParentType self` as its first parameter; a ref-self method
 *     takes `ParentType* self`. Static methods take no implicit receiver.
 *   - Inside a method body an unqualified identifier that is *not* a local
 *     variable or parameter but *is* a field of the parent struct is
 *     rewritten to `self.x` (value self) or `self->x` (ref self).
 *   - An ND_MEMBER whose lhs is an identifier naming a struct type and
 *     whose parent is an ND_CALL is treated as a static method call:
 *     `Vec3.new(1,2,3)` -> `Vec3_new(1,2,3)`.
 *   - A call whose callee is an ND_MEMBER on a value of struct type is a
 *     method call: `a.dot(b)` -> `Vec3_dot(a, b)` for value self,
 *     `a.normalize()` -> `Vec3_normalize(&a)` for ref self. If `a` itself
 *     already has pointer/ref type, the address-of is skipped.
 *   - Struct literals emit C99 compound literals: `(T){ .x=1, .y=2 }`
 *     (named) or `(T){ 1, 2 }` (positional).
 */
#include "sharp.h"
#include "types.h"
#include "ast.h"

/* ===================================================================== *
 *   Local-variable scope (codegen-side, for ident type resolution and
 *   phase-3 destructor tracking).
 *
 *   Each ScopeEntry that corresponds to a struct-typed local whose type
 *   defines `~T()` records the struct name in `dtor_struct`. On scope
 *   exit (normal or early), cgen emits `T___drop(&var);` for those
 *   entries in reverse declaration order.
 *
 *   Scope `kind` drives unwinding on `return` / `break` / `continue`:
 *     - SCOPE_FUNC      : the function body root (return unwinds through here)
 *     - SCOPE_LOOP_BODY : the body scope of a for/while loop
 *                         (break / continue unwind through here)
 *     - SCOPE_BLOCK     : an ordinary `{ ... }` block
 * ===================================================================== */
enum { SCOPE_BLOCK = 0, SCOPE_FUNC = 1, SCOPE_LOOP_BODY = 2 };

typedef struct ScopeEntry {
    const char* name;
    Type*       type;
    const char* dtor_struct;  /* non-NULL => run `T___drop(&name)` at scope end */
} ScopeEntry;

typedef struct Scope {
    struct Scope* parent;
    ScopeEntry*   data;
    int           len;
    int           cap;
    int           kind;
} Scope;

static void scope_push(Scope** top) {
    Scope* s = (Scope*)calloc(1, sizeof(Scope));
    s->parent = *top;
    s->kind   = SCOPE_BLOCK;
    *top = s;
}

static void scope_pop(Scope** top) {
    Scope* s = *top;
    *top = s->parent;
    free(s->data);
    free(s);
}

static void scope_add(Scope* s, const char* name, Type* type, const char* dtor_struct) {
    if (s->len == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        ScopeEntry* nd = (ScopeEntry*)malloc((size_t)s->cap * sizeof(ScopeEntry));
        if (s->len) memcpy(nd, s->data, (size_t)s->len * sizeof(ScopeEntry));
        free(s->data);
        s->data = nd;
    }
    s->data[s->len].name        = name;
    s->data[s->len].type        = type;
    s->data[s->len].dtor_struct = dtor_struct;
    s->len++;
}

static Type* scope_lookup(Scope* s, const char* name) {
    for (Scope* cur = s; cur; cur = cur->parent) {
        for (int i = cur->len - 1; i >= 0; i--)
            if (strcmp(cur->data[i].name, name) == 0)
                return cur->data[i].type;
    }
    return NULL;
}

/* ===================================================================== *
 *   Codegen state
 * ===================================================================== */
typedef struct {
    StrBuf     out;
    int        indent;
    SymTable*  st;
    Arena**    arena;         /* for ephemeral Type nodes built during codegen */
    Scope*     scope;         /* top of the local-variable stack */
    /* method-body context (NULL for free functions) */
    SymStruct* self_struct;
    int        self_kind;
    /* monotonic counter for synthesized names (return temporaries etc.) */
    int        tmp_counter;
    /* the return type of the function currently being emitted.
     * Used by ND_RETURN to declare the temporary that holds `return expr`
     * across destructor calls, without relying on expression type inference. */
    Type*      cur_ret_type;
    /* active type-parameter substitution for the struct/impl currently being
     * emitted. tsubst_names[i] -> tsubst_types[i]. When emitting a generic
     * template body we walk every TY_NAMED and, if its name matches a subst
     * key, replace with the mapped Type. ntsubst == 0 means we are emitting
     * an ordinary (non-generic) declaration. */
    const char** tsubst_names;
    Type**       tsubst_types;
    int          ntsubst;
    /* when emitting a struct or impl that lives under a specific instantiation,
     * self_mangled holds the already-mangled parent type name (e.g. "Stack_i32")
     * so method signatures and self-type casts use it. self_template_name is
     * the generic base name (e.g. "Stack") so that bare references to the
     * parent struct's own name inside its methods resolve to the mangled form
     * rather than the template name. */
    const char*  self_mangled;
    const char*  self_template_name;
} G;

/* Return the resolved Type for `t` in the current subst environment.
 *
 * Thanks to the interning types module, this has become very thin:
 *   - Apply the substitution env via ty_subst.
 *   - Special-case "bare template-struct name in its own specialised
 *     method body": inside Span_i32's methods, `Span self` should read
 *     `Span_i32 self`, so we rewrite to ty_named(mangled).
 *
 * All other kinds flow through ty_subst unchanged. */
/* Resolve type: substitute type parameters, then resolve typedefs
 * through the symbol table. */
static Type* resolve_type(G* g, Type* t) {
    if (!t) return NULL;
    Type* rt = ty_subst(t, g->tsubst_names, g->tsubst_types, g->ntsubst);
    if (rt->kind == TY_NAMED && rt->ntargs == 0 &&
        g->self_template_name && g->self_mangled &&
        rt->name && strcmp(rt->name, g->self_template_name) == 0) {
        return ty_named(g->self_mangled);
    }
    /* Resolve typedefs through the symbol table */
    return sema_resolve_type(g->st, rt);
}

/* Emit a C-safe mangled name. Delegates to ty_mangle, which caches. */
static void mangle_type_to(StrBuf* sb, Type* t) {
    sb_puts(sb, ty_mangle(t));
}

static void pad(G* g) {
    for (int i = 0; i < g->indent; i++) sb_puts(&g->out, "    ");
}

/* ===================================================================== *
 *   C Declarator engine
 *
 *   C declarations use a "center-around" grammar (dangling modifiers):
 *     int* p              → specifier "int", declarator "*p"
 *     int (*fp)(int)      → specifier "int", declarator "(*fp)(int)"
 *     _PVFV* p            → specifier "_PVFV", declarator "*p"
 *     int (*(*pp)(int))(void) → specifier "int", declarator "(*(*pp)(int))(void)"
 *
 *   The old emit_type/emit_type_with_name used left-to-right string
 *   concatenation, which cannot correctly handle nested function pointers
 *   or arrays of function pointers.
 *
 *   New approach:
 *     1. Decompose Type into a list of modifiers (PTR/ARRAY/FUNC)
 *     2. Identify the specifier (innermost non-decorator type)
 *     3. Render: specifier + declarators (innermost first, auto-parens)
 * ===================================================================== */

typedef enum { MOD_PTR, MOD_ARRAY, MOD_FUNC } ModKind;

typedef struct Mod {
    ModKind     kind;
    bool        is_const;       /* for MOD_PTR: const-ness of the pointer */
    int         array_size;     /* for MOD_ARRAY */
    Type**      func_params;    /* for MOD_FUNC */
    int         nfunc_params;
    bool        func_variadic;
} Mod;

/* Decompose a resolved Type into a list of modifiers.
 * The specifier is the innermost non-decorator type.
 * Modifiers are collected outermost-first, then reversed during emission.
 * Returns the number of modifiers filled (max 32). */
static int decompose_type(G* g, Type* t, Mod* mods, int max_mods) {
    int n = 0;
    Type* cur = t;
    for (;;) {
        if (!cur) break;
        Type* rt = resolve_type(g, cur);
        if (!rt) break;

        if (rt->kind == TY_PTR) {
            if (n >= max_mods) break;
            mods[n].kind = MOD_PTR;
            mods[n].is_const = rt->is_const;
            mods[n].array_size = 0;
            mods[n].func_params = NULL;
            mods[n].nfunc_params = 0;
            mods[n].func_variadic = false;
            n++;
            /* If base is TY_NAMED (a typedef name), stop decomposing.
             * The typedef name becomes the specifier. */
            if (rt->base && rt->base->kind == TY_NAMED && rt->base->ntargs == 0)
                break;
            cur = rt->base;
        } else if (rt->kind == TY_ARRAY) {
            if (n >= max_mods) break;
            mods[n].kind = MOD_ARRAY;
            mods[n].is_const = false;
            mods[n].array_size = rt->array_size;
            mods[n].func_params = NULL;
            mods[n].nfunc_params = 0;
            mods[n].func_variadic = false;
            n++;
            cur = rt->base;
        } else if (rt->kind == TY_FUNC) {
            if (n >= max_mods) break;
            mods[n].kind = MOD_FUNC;
            mods[n].is_const = false;
            mods[n].array_size = 0;
            mods[n].func_params = rt->func_params;
            mods[n].nfunc_params = rt->nfunc_params;
            mods[n].func_variadic = rt->func_variadic;
            n++;
            cur = rt->base;
        } else if (rt->kind == TY_BITFIELD) {
            cur = rt->base;
        } else {
            break;
        }
    }
    return n;
}

/* Return the specifier type (innermost non-decorator type) for a given Type. */
static Type* specifier_of(G* g, Type* t) {
    Type* cur = t;
    for (;;) {
        if (!cur) return cur;
        Type* rt = resolve_type(g, cur);
        if (!rt) return cur;
        if (rt->kind == TY_PTR || rt->kind == TY_ARRAY || rt->kind == TY_FUNC) {
            /* TY_PTR with TY_NAMED base: the NAMED is the specifier */
            if (rt->kind == TY_PTR && rt->base && rt->base->kind == TY_NAMED &&
                rt->base->ntargs == 0)
                return rt->base;
            cur = rt->base;
        } else if (rt->kind == TY_BITFIELD) {
            cur = rt->base;
        } else {
            return rt;
        }
    }
}

/* Emit the specifier part of a type (no name, no modifiers). */
static void emit_specifier(G* g, Type* t) {
    if (!t) { sb_puts(&g->out, "void"); return; }
    Type* rt = resolve_type(g, t);
    if (!rt) { sb_puts(&g->out, "void"); return; }

    /* If it's a TY_PTR wrapping TY_NAMED (typedef name), emit the name. */
    if (rt->kind == TY_PTR && rt->base && rt->base->kind == TY_NAMED &&
        rt->base->ntargs == 0) {
        if (rt->base->name)
            sb_puts(&g->out, rt->base->name);
        else
            sb_puts(&g->out, "void");
        return;
    }

    if (rt->kind == TY_NAMED) {
        if (rt->ntargs > 0) sb_puts(&g->out, ty_mangle(rt));
        else                sb_puts(&g->out, rt->name);
        return;
    }

    if (rt->is_const) sb_puts(&g->out, "const ");
    switch (rt->kind) {
        case TY_VOID:   sb_puts(&g->out, "void");      return;
        case TY_BOOL:   sb_puts(&g->out, "_Bool");     return;
        case TY_CHAR:   sb_puts(&g->out, "char");      return;
        case TY_SHORT:  sb_puts(&g->out, "short");     return;
        case TY_INT:    sb_puts(&g->out, "int");       return;
        case TY_LONG:   sb_puts(&g->out, "long");      return;
        case TY_LONGLONG: sb_puts(&g->out, "long long"); return;
        case TY_FLOAT:  sb_puts(&g->out, "float");     return;
        case TY_DOUBLE: sb_puts(&g->out, "double");    return;
        case TY_PTR:    sb_puts(&g->out, "void");      return; /* fallback */
        case TY_FUNC:   sb_puts(&g->out, "void");      return; /* fallback */
        case TY_BITFIELD: emit_specifier(g, rt->base); return;
        case TY_ARRAY:  emit_specifier(g, rt->base);   return;
    }
    sb_puts(&g->out, ty_render(rt));
}

/* Check if modifier at index `i` needs parentheses when rendered.
 * A PTR modifier needs parens if the next inner modifier is ARRAY or FUNC.
 * (e.g. `int (*p)[10]` — the * needs parens because inside is [10]) */
static bool mod_needs_parens(Mod* mods, int n, int i) {
    if (mods[i].kind != MOD_PTR) return false;
    int inner = i + 1;
    if (inner >= n) return false;
    return mods[inner].kind == MOD_ARRAY || mods[inner].kind == MOD_FUNC;
}

/* Forward declaration — needed by emit_declarator for recursive function param types. */
static void emit_decl(G* g, Type* t, const char* name);

/* Emit the declarator part (modifiers + name).
 * `mods` is ordered as produced by decompose_type: outermost-inward
 * from the perspective of the Type tree.
 *   - For TY_PTR → MOD_PTR (outer), base → inner
 *   - For TY_FUNC → MOD_FUNC (wraps name), base → return type (outer)
 *
 * In decompose_type output, modifiers are ordered from the outermost
 * type layer inward. For `int (*fp)(int)`:
 *   Type = TY_FUNC(base=TY_PTR(TY_INT), params=[TY_INT])
 *   decompose: [MOD_FUNC, MOD_PTR]  (FUNC wraps PTR, PTR wraps INT)
 *
 * For C rendering:
 *   - PTR modifiers go BEFORE the name (prefix)
 *   - FUNC/ARRAY go AFTER the name (suffix)
 *   - When PTR wraps FUNC or vice versa, parens may be needed
 */
static void emit_declarator(G* g, Mod* mods, int n, const char* name,
                            int depth, bool parens) {
    if (depth >= n) {
        if (name) sb_puts(&g->out, name);
        return;
    }

    /* Count PTR modifiers */
    int nptr = 0;
    int nfunc = 0;
    int narray = 0;
    for (int i = depth; i < n; i++) {
        if (mods[i].kind == MOD_PTR) nptr++;
        else if (mods[i].kind == MOD_FUNC) nfunc++;
        else if (mods[i].kind == MOD_ARRAY) narray++;
    }

    /* Need parens around (PTRs + name) if we have both PTR and FUNC/ARRAY */
    bool need_paren = parens;
    if (!need_paren && nptr > 0 && (nfunc > 0 || narray > 0)) {
        need_paren = true;
    }

    /* Step 1: open paren if needed */
    if (need_paren) sb_putc(&g->out, '(');

    /* Step 2: emit PTR prefix */
    for (int i = depth; i < n; i++) {
        if (mods[i].kind == MOD_PTR) {
            sb_putc(&g->out, '*');
            if (mods[i].is_const) sb_puts(&g->out, " const");
        }
    }

    /* Step 3: emit name */
    if (name) sb_puts(&g->out, name);

    /* Step 4: close paren (before ARRAY suffixes, they go inside) */
    /* NOTE: For function pointer arrays like void (*arr[8])(int),
     * the ARRAY suffix goes INSIDE the parentheses, and FUNC goes OUTSIDE.
     * So we close the paren AFTER ARRAY suffixes in Step 5b. */

    /* Step 5: emit suffixes.
     * decompose_type outputs modifiers from outermost to innermost.
     * For C declarator syntax, suffixes closest to the name must appear first.
     *
     * When need_paren is true (pointer wrapping a function/array), the
     * structure is: (*name + ARRAY suffixes)(FUNC suffixes)
     *   - ARRAY suffixes go INSIDE the parentheses (closest to name)
     *   - FUNC suffixes go OUTSIDE the parentheses (outermost)
     *
     * When need_paren is false:
     *   - All suffixes go directly after the name
     *
     * In both cases, ARRAY suffixes (innermost) come before FUNC suffixes (outermost).
     * We traverse mods in REVERSE (innermost first) and emit ARRAY then FUNC.
     *
     * Example: void (*handler_arr[8])(int)
     *   decompose: [MOD_FUNC(outer), MOD_ARRAY, MOD_PTR(inner)]
     *   output: (*handler_arr[8])(int)
     *     - [*] inside parens
     *     - [handler_arr] name
     *     - [8] ARRAY inside parens (closer to name)
     *     - close paren
     *     - (int) FUNC outside parens (outermost)
     */

    /* Step 5a: emit ARRAY suffixes INSIDE parentheses (if need_paren) */
    for (int i = n - 1; i >= depth; i--) {
        if (mods[i].kind == MOD_ARRAY && need_paren) {
            sb_printf(&g->out, "[%d]", mods[i].array_size);
        }
    }

    /* Step 5b: close paren (AFTER ARRAY suffixes) */
    if (need_paren) sb_putc(&g->out, ')');

    /* Step 5c: emit remaining suffixes (ARRAY without paren, FUNC) */
    for (int i = n - 1; i >= depth; i--) {
        if (mods[i].kind == MOD_ARRAY && !need_paren) {
            sb_printf(&g->out, "[%d]", mods[i].array_size);
        } else if (mods[i].kind == MOD_FUNC) {
            sb_putc(&g->out, '(');
            for (int j = 0; j < mods[i].nfunc_params; j++) {
                if (j) sb_puts(&g->out, ", ");
                emit_decl(g, mods[i].func_params[j], NULL);
            }
            if (mods[i].func_variadic) {
                if (mods[i].nfunc_params > 0) sb_puts(&g->out, ", ");
                sb_puts(&g->out, "...");
            }
            sb_putc(&g->out, ')');
        }
    }
}

/* Public entry point: emit `specifier declarator` for type `t`.
 * If `name` is non-NULL, append the identifier. */
static void emit_decl(G* g, Type* t, const char* name) {
    if (!t) {
        sb_printf(&g->out, "/*null-type*/ void");
        if (name) sb_printf(&g->out, " %s", name);
        return;
    }

    Type* spec = specifier_of(g, t);
    Mod mods[32];
    int n = decompose_type(g, t, mods, 32);

    /* If no modifiers, just emit specifier + name */
    if (n == 0) {
        emit_specifier(g, spec);
        if (name) sb_printf(&g->out, " %s", name);
        return;
    }

    /* Special case: single MOD_PTR with TY_NAMED base.
     * Emit as `name* identifier` directly (no decomposition needed). */
    if (n == 1 && mods[0].kind == MOD_PTR &&
        t->kind == TY_PTR && t->base && t->base->kind == TY_NAMED &&
        t->base->ntargs == 0 && t->base->name) {
        sb_puts(&g->out, t->base->name);
        sb_putc(&g->out, '*');
        if (mods[0].is_const) sb_puts(&g->out, " const");
        if (name) sb_printf(&g->out, " %s", name);
        return;
    }

    /* Emit specifier, then declarator modifiers (outermost-first). */
    emit_specifier(g, spec);
    sb_putc(&g->out, ' ');
    emit_declarator(g, mods, n, name, 0, false);
}

/* Emit a declaration with `__unaligned` inserted before the first `*`.
 * MSVC requires: `int * __unaligned ptr` not `int* ptr __unaligned`. */
static void emit_decl_unaligned(G* g, Type* t, const char* name) {
    if (!t) {
        sb_printf(&g->out, "/*null-type*/ void * __unaligned %s", name ? name : "");
        return;
    }
    Type* spec = specifier_of(g, t);
    Mod mods[32];
    int n = decompose_type(g, t, mods, 32);

    /* If no modifiers, just emit normally */
    if (n == 0) {
        emit_specifier(g, spec);
        if (name) sb_printf(&g->out, " %s", name);
        return;
    }

    /* Find the first PTR modifier and mark it for __unaligned insertion */
    /* Emit specifier */
    emit_specifier(g, spec);
    sb_putc(&g->out, ' ');

    /* Emit declarator, inserting __unaligned before the first * */
    for (int i = 0; i < n; i++) {
        bool needs_parens = mod_needs_parens(mods, n, i);
        if (needs_parens) sb_putc(&g->out, '(');

        /* Recursively emit inner declarators first */
        /* We need to restructure this — just handle the simple case:
         * single PTR modifier: `type * __unaligned name` */
        if (n == 1 && mods[0].kind == MOD_PTR) {
            if (name) sb_printf(&g->out, "* __unaligned %s", name);
            else sb_puts(&g->out, "* __unaligned");
            if (mods[0].is_const) sb_puts(&g->out, " const");
            return;
        }

        if (needs_parens) sb_putc(&g->out, ')');
    }
    /* Fallback: emit normally without __unaligned */
    emit_decl(g, t, name);
}

/* ===================================================================== *
 *   Compatibility wrappers — route old API to the new declarator engine.
 *   These will be phased out as call sites are migrated to emit_decl().
 * ===================================================================== */

/* emit_type(t)  →  emit_decl(t, NULL) */
static void emit_type(G* g, Type* t) {
    emit_decl(g, t, NULL);
}

/* emit_type_with_name(t, name)  →  emit_decl(t, name) */
static void emit_type_with_name(G* g, Type* t, const char* name) {
    emit_decl(g, t, name);
}

/* emit_type_core(t) — emit just the specifier (no modifiers, no name).
 * For primitives/named types this is the same as emit_specifier.
 * For PTR/ARRAY/FUNC types, the old code emitted the full decorated type
 * without a name. We handle the common cases: */
static void emit_type_core(G* g, Type* t) {
    if (!t) { sb_puts(&g->out, "void"); return; }
    Type* rt = resolve_type(g, t);
    if (!rt) { sb_puts(&g->out, "void"); return; }

    switch (rt->kind) {
    case TY_PTR:
        emit_type_core(g, rt->base);
        sb_putc(&g->out, '*');
        if (rt->is_const) sb_puts(&g->out, " const");
        return;
    case TY_NAMED:
        if (rt->ntargs > 0) sb_puts(&g->out, ty_mangle(rt));
        else                sb_puts(&g->out, rt->name);
        return;
    case TY_FUNC: {
        /* Function type without a name: emit as `ret (*)(params)` */
        emit_type_core(g, rt->base);
        sb_puts(&g->out, "(*)(");
        for (int i = 0; i < rt->nfunc_params; i++) {
            if (i) sb_puts(&g->out, ", ");
            emit_type_core(g, rt->func_params[i]);
        }
        if (rt->func_variadic) {
            if (rt->nfunc_params > 0) sb_puts(&g->out, ", ");
            sb_puts(&g->out, "...");
        }
        sb_puts(&g->out, ")");
        return;
    }
    case TY_BITFIELD:
        emit_type_core(g, rt->base);
        sb_printf(&g->out, " : %d", rt->bit_width);
        return;
    default:
        if (rt->is_const) sb_puts(&g->out, "const ");
        switch (rt->kind) {
            case TY_VOID:   sb_puts(&g->out, "void");      return;
            case TY_BOOL:   sb_puts(&g->out, "_Bool");     return;
            case TY_CHAR:   sb_puts(&g->out, "char");      return;
            case TY_SHORT:  sb_puts(&g->out, "short");     return;
            case TY_INT:    sb_puts(&g->out, "int");       return;
            case TY_LONG:   sb_puts(&g->out, "long");      return;
            case TY_LONGLONG: sb_puts(&g->out, "long long"); return;
            case TY_FLOAT:  sb_puts(&g->out, "float");     return;
            case TY_DOUBLE: sb_puts(&g->out, "double");    return;
            default:        sb_puts(&g->out, ty_render(rt)); return;
        }
    }
}

/* emit_type_with_name_unaligned(t, name) — MSVC __unaligned support */
static void emit_type_with_name_unaligned(G* g, Type* t, const char* name) {
    emit_decl_unaligned(g, t, name);
}

static const char* bin_op_c(OpKind op) {
    switch (op) {
        case OP_ADD: return "+";  case OP_SUB: return "-";
        case OP_MUL: return "*";  case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ:  return "=="; case OP_NEQ: return "!=";
        case OP_LT:  return "<";  case OP_GT:  return ">";
        case OP_LE:  return "<="; case OP_GE:  return ">=";
        case OP_AND: return "&&"; case OP_OR:  return "||";
        case OP_BAND: return "&"; case OP_BOR: return "|";
        case OP_BXOR: return "^"; case OP_SHL: return "<<";
        case OP_SHR:  return ">>";
        default:      return "?";
    }
}

static const char* assign_op_c(OpKind op) {
    switch (op) {
        case OP_ASSIGN: return "=";
        case OP_ADDEQ:  return "+=";
        case OP_SUBEQ:  return "-=";
        case OP_MULEQ:  return "*=";
        case OP_DIVEQ:  return "/=";
        case OP_MODEQ:  return "%=";
        case OP_ANDEQ:  return "&=";
        case OP_OREQ:   return "|=";
        case OP_XOREQ:  return "^=";
        case OP_SHLEQ:  return "<<=";
        case OP_SHREQ:  return ">>=";
        default:        return "=";
    }
}

static Type* expr_type(G* g, Node* e);

static SymStruct* struct_of_type(G* g, Type* t) {
    if (!t) return NULL;
    if (t->kind == TY_NAMED) return sema_find_struct(g->st, t->name);
    if (type_is_pointerlike(t) && t->base && t->base->kind == TY_NAMED)
        return sema_find_struct(g->st, t->base->name);
    return NULL;
}

/* ===================================================================== *
 *   Phase-3 RAII: destructor tracking and emission
 * ===================================================================== */

/* Return the struct name whose destructor would fire for a local of type `t`,
 * or NULL if none. Only *value-typed* struct locals are tracked — pointers,
 * references, and primitives own no resource at this level. */
static const char* dtor_struct_name_for(G* g, Type* t) {
    if (!t || t->kind != TY_NAMED) return NULL;
    SymStruct* s = sema_find_struct(g->st, t->name);
    if (!s) return NULL;
    if (!sema_find_dtor(g->st, s->name)) return NULL;
    return s->name;
}

/* Emit `T___drop(&name);` at the current indentation. */
static void emit_dtor_call(G* g, const char* struct_name, const char* var_name) {
    pad(g);
    sb_printf(&g->out, "%s___drop(&%s);\n", struct_name, var_name);
}

/* Emit dtor calls for a single scope's destructible entries, in reverse
 * declaration order. Does NOT modify the scope; the caller decides whether
 * this is a true scope-exit (in which case scope_pop happens after) or an
 * early-exit cleanup (in which case the scope stays alive for further code). */
static void emit_scope_dtors(G* g, Scope* s) {
    if (!s) return;
    for (int i = s->len - 1; i >= 0; i--) {
        if (s->data[i].dtor_struct)
            emit_dtor_call(g, s->data[i].dtor_struct, s->data[i].name);
    }
}

/* For early exits: emit dtor calls for all scopes from the current top
 * through (and including) the first scope whose kind matches `stop_kind`.
 * Returns true if such a scope was found, false if we walked off the top. */
static bool emit_unwind_through(G* g, int stop_kind) {
    for (Scope* cur = g->scope; cur; cur = cur->parent) {
        emit_scope_dtors(g, cur);
        if (cur->kind == stop_kind) return true;
    }
    return false;
}

/* True if the current scope (or any enclosing scope up to and including
 * `stop_kind`) has any destructible locals that would need unwinding. */
static bool any_dtors_through(G* g, int stop_kind) {
    for (Scope* cur = g->scope; cur; cur = cur->parent) {
        for (int i = 0; i < cur->len; i++)
            if (cur->data[i].dtor_struct) return true;
        if (cur->kind == stop_kind) return false;
    }
    return false;
}

/* Structural type equality — now O(1) thanks to interning in the types
 * module. Kept as a named wrapper so the existing call sites stay as-is. */
static inline bool type_eq_simple(Type* a, Type* b) { return ty_eq(a, b); }

/* True if e is `IDENT` where IDENT names a struct type (and is not shadowed
 * by a local / param / self-field). */
static bool ident_is_struct_type(G* g, Node* e) {
    if (!e || e->kind != ND_IDENT) return false;
    if (scope_lookup(g->scope, e->name)) return false;
    if (g->self_struct && sema_find_field(g->self_struct, e->name)) return false;
    return sema_find_struct(g->st, e->name) != NULL;
}

/* Map binop/unop OpKinds to operator method names. These must stay in sync
 * with operator_method_name() in parser.c. Defined here as static so that
 * expr_type and emit_expr can both consult them without having to traverse
 * the impl's method table by trial-and-error. */
static const char* cg_binop_method_name(OpKind op) {
    switch (op) {
        case OP_ADD: return "op_add";
        case OP_SUB: return "op_sub";
        case OP_MUL: return "op_mul";
        case OP_DIV: return "op_div";
        case OP_MOD: return "op_mod";
        case OP_EQ:  return "op_eq";
        case OP_NEQ: return "op_neq";
        case OP_LT:  return "op_lt";
        case OP_GT:  return "op_gt";
        case OP_LE:  return "op_le";
        case OP_GE:  return "op_ge";
        default:     return NULL;
    }
}
static const char* cg_unop_method_name(OpKind op) {
    switch (op) {
        case OP_NEG:  return "op_sub";   /* unary minus: distinguished by nparams==1 */
        case OP_NOT:  return "op_not";
        case OP_BNOT: return "op_bnot";
        default:      return NULL;
    }
}

/* Recognise a primitive type keyword as a type. Used by expr_type to give
 * literals plausible types so operator dispatch on their results still works. */
static Type* type_of_int_literal(G* g)   { return type_prim(g->arena, TY_INT);  }
static Type* type_of_float_literal(G* g) { return type_prim(g->arena, TY_DOUBLE);  }
static Type* type_of_bool_literal(G* g)  { return type_prim(g->arena, TY_BOOL); }
static Type* type_of_char_literal(G* g)  { return type_prim(g->arena, TY_CHAR); }

static Type* expr_type(G* g, Node* e) {
    if (!e) return NULL;
    switch (e->kind) {
        case ND_INT:   return type_of_int_literal(g);
        case ND_FLOAT: return type_of_float_literal(g);
        case ND_BOOL:  return type_of_bool_literal(g);
        case ND_CHAR:  return type_of_char_literal(g);
        case ND_STR:   return ty_ptr(ty_prim(TY_CHAR));
        case ND_NULL:  return ty_ptr(ty_prim(TY_VOID));

        case ND_IDENT: {
            const char* name = ident_name(e);
            Type* t = scope_lookup(g->scope, name);
            if (t) return t;
            if (g->self_struct) {
                SymField* f = sema_find_field(g->self_struct, name);
                if (f) return f->type;
            }
            return NULL;
        }

        case ND_MEMBER: {
            Node*       recv = member_receiver(e);
            const char* fld  = member_name    (e);
            Type*       lt   = expr_type(g, recv);
            SymStruct*  s    = struct_of_type(g, lt);
            if (!s) return NULL;
            SymField* f = sema_find_field(s, fld);
            if (!f) return NULL;
            /* Apply type-arg substitution so generic fields resolve to
             * concrete types: Result<i32>.val → i32, Span<u8>.ptr → u8*. */
            if (lt && lt->kind == TY_NAMED && lt->ntargs > 0 &&
                s->ntype_params > 0) {
                int sc = s->ntype_params < lt->ntargs ? s->ntype_params
                                                       : lt->ntargs;
                if (sc > 8) sc = 8;
                const char* names[8]; Type* types[8];
                for (int i = 0; i < sc; i++) {
                    names[i] = s->type_params[i];
                    types[i] = lt->targs[i];
                }
                return ty_subst(f->type, names, types, sc);
            }
            return f->type;
        }

        case ND_STRUCT_LIT:
            return ty_named(struct_lit_name(e));

        case ND_BINOP: {
            OpKind op = binop_op(e);
            /* Overloaded operator? If so, the result type is the method's
             * declared return type. Essential for chains like `(a + b) * 2`
             * where the outer * must know the inner produced a struct so it
             * can dispatch through T_op_mul. */
            const char* mname = cg_binop_method_name(op);
            if (mname) {
                Type* lt = expr_type(g, binop_lhs(e));
                SymStruct* s = ty_is_named(lt) ? sema_find_struct(g->st, ty_name(lt)) : NULL;
                if (s) {
                    SymMethod* m = sema_find_method(g->st, s->name, mname);
                    if (m && m->decl && func_nparams(m->decl) == 2)
                        return func_ret_type(m->decl);
                }
            }
            /* Comparison / logical ops always yield bool; for arithmetic we
             * approximate by the LHS type (C's usual promotion is fine for
             * the purposes of downstream dispatch). */
            switch (op) {
                case OP_EQ: case OP_NEQ: case OP_LT: case OP_GT:
                case OP_LE: case OP_GE: case OP_AND: case OP_OR:
                    return ty_prim(TY_BOOL);
                default: break;
            }
            return expr_type(g, binop_lhs(e));
        }

        case ND_UNOP: {
            OpKind op = unop_op(e);
            const char* mname = cg_unop_method_name(op);
            if (mname) {
                Type* ot = expr_type(g, unop_operand(e));
                SymStruct* s = ty_is_named(ot) ? sema_find_struct(g->st, ty_name(ot)) : NULL;
                if (s) {
                    SymMethod* m = sema_find_method(g->st, s->name, mname);
                    if (m && m->decl && func_nparams(m->decl) == 1)
                        return func_ret_type(m->decl);
                }
            }
            /* Address-of / dereference have structural type effects. */
            Type* rt = expr_type(g, unop_operand(e));
            if (op == OP_ADDR)  return rt ? ty_ptr(rt) : NULL;
            if (op == OP_DEREF) return ty_is_pointer_like(rt) ? ty_base(rt) : NULL;
            if (op == OP_NOT)   return ty_prim(TY_BOOL);
            return rt;
        }

        case ND_ASSIGN:
            /* `lhs = rhs` yields the LHS type. */
            return expr_type(g, assign_lhs(e));

        case ND_INDEX: {
            /* a[i]: raw pointer → base. */
            Type* lt = expr_type(g, index_receiver(e));
            if (ty_is_pointer_like(lt)) return ty_base(lt);
            return NULL;
        }

        case ND_CAST:
            /* (Type)expr — the result type is the target cast type. */
            return cast_type(e);

        case ND_SIZEOF:
            /* sizeof — result type is always size_t / long */
            return type_prim(g->arena, TY_LONG);

        case ND_CALL: {
            Node* callee = call_callee(e);
            if (callee && callee->kind == ND_MEMBER) {
                Node*       recv  = member_receiver(callee);
                const char* mname = member_name    (callee);
                if (ident_is_struct_type(g, recv)) {
                    SymMethod* m = sema_find_method(g->st, ident_name(recv), mname);
                    if (m && m->decl) return func_ret_type(m->decl);
                } else {
                    /* Receiver expression: recognise pointer-to-struct too,
                     * so `p->method()` or `(&x).method()` both resolve. */
                    Type* lt = expr_type(g, recv);
                    SymStruct* s = struct_of_type(g, lt);
                    if (s) {
                        SymMethod* m = sema_find_method(g->st, s->name, mname);
                        if (m && m->decl) return func_ret_type(m->decl);
                    }
                }
            } else if (callee && callee->kind == ND_IDENT) {
                const char* cname = ident_name(callee);
                /* Implicit-self method call inside an impl body. */
                if (g->self_struct && !scope_lookup(g->scope, cname)) {
                    SymMethod* m = sema_find_method(g->st, g->self_struct->name, cname);
                    if (m && m->decl) return func_ret_type(m->decl);
                }
                /* Free function — consult sema's function table. */
                SymFunc* f = sema_find_func(g->st, cname);
                if (f) return f->ret_type;
                SymExtern* ex = sema_find_extern(g->st, cname);
                if (ex) return ex->ret_type;
            }
            return NULL;
        }

        default:
            return NULL;
    }
}

/* ---------- expression emission ---------- */
static void emit_expr(G* g, Node* e);

static void emit_str_literal(G* g, const char* s, int n) {
    sb_putc(&g->out, '"');
    sb_putn(&g->out, s, n);
    sb_putc(&g->out, '"');
}

static void emit_char_literal(G* g, const char* s, int n) {
    sb_putc(&g->out, '\'');
    sb_putn(&g->out, s, n);
    sb_putc(&g->out, '\'');
}

/* Emit `Parent_method(<recv>, args...)`.
 *
 * Phase 4: when the receiver is a concrete generic instantiation (e.g.
 * Span_i32), use the mangled name as the function-name prefix rather than the
 * bare template name (e.g. "Span"). This makes `s.get(2)` on a Span_i32
 * correctly emit `Span_i32_get(s, 2)` instead of `Span_get(s, 2)`. */
static void emit_method_call(G* g, SymMethod* meth, Node* lhs, Node** args, int nargs) {
    const char* prefix = meth->parent;
    Type* lt = expr_type(g, lhs);
    if (lt && lt->kind == TY_NAMED && lt->ntargs > 0) {
        StrBuf sb; sb_init(&sb);
        mangle_type_to(&sb, lt);
        prefix = arena_strndup(g->arena, sb.data, (int)sb.len);
        sb_free(&sb);
    }
    sb_printf(&g->out, "%s_%s(", prefix, meth->name);
    if (meth->self_kind == SELF_REF) {
        if (type_is_pointerlike(lt)) {
            emit_expr(g, lhs);
        } else {
            sb_puts(&g->out, "&(");
            emit_expr(g, lhs);
            sb_putc(&g->out, ')');
        }
    } else { /* SELF_VALUE */
        emit_expr(g, lhs);
    }
    for (int i = 0; i < nargs; i++) {
        sb_puts(&g->out, ", ");
        emit_expr(g, args[i]);
    }
    sb_putc(&g->out, ')');
}

static void emit_ident(G* g, const char* name) {
    if (scope_lookup(g->scope, name)) { sb_puts(&g->out, name); return; }
    if (g->self_struct && sema_find_field(g->self_struct, name)) {
        if (g->self_kind == SELF_REF)
            sb_printf(&g->out, "self->%s", name);
        else
            sb_printf(&g->out, "self.%s", name);
        return;
    }
    sb_puts(&g->out, name);
}

static void emit_struct_lit(G* g, Node* e) {
    const char* sname = struct_lit_name(e);
    Type*       lt    = ast_type(e);

    /* Phase 4: if the literal carries a generic instantiation in e->type
     * (set by parse_primary when it saw `Name<T> { ... }`), emit the
     * mangled type name; otherwise fall back to the raw template name. */
    if (lt && ty_is_named(lt) && ty_ntargs(lt) > 0) {
        sb_printf(&g->out, "(%s){ ", ty_mangle(resolve_type(g, lt)));
    } else {
        /* A non-generic literal whose type name might still need subst
         * (e.g. inside a template body where T becomes Span_i32 etc.). */
        bool subst_hit = false;
        for (int i = 0; i < g->ntsubst; i++) {
            if (strcmp(g->tsubst_names[i], sname) == 0) {
                sb_printf(&g->out, "(%s){ ", ty_mangle(g->tsubst_types[i]));
                subst_hit = true;
                break;
            }
        }
        if (!subst_hit)
            sb_printf(&g->out, "(%s){ ", sname);
    }

    int nfields = struct_lit_nfields(e);
    for (int i = 0; i < nfields; i++) {
        Node* fi = struct_lit_field(e, i);
        if (i) sb_puts(&g->out, ", ");
        const char* fname = field_init_name(fi);
        if (fname) sb_printf(&g->out, ".%s = ", fname);
        emit_expr(g, field_init_value(fi));
    }
    sb_puts(&g->out, " }");
}

static void emit_member(G* g, Node* e) {
    Node*       recv = member_receiver(e);
    const char* fld  = member_name    (e);
    Type*       lt   = expr_type(g, recv);

    emit_expr(g, recv);
    sb_puts(&g->out, ty_is_pointer_like(lt) ? "->" : ".");
    sb_puts(&g->out, fld);
}

static void emit_call(G* g, Node* e) {
    Node*  callee = call_callee(e);
    Node** args   = call_args  (e);
    int    nargs  = call_nargs (e);

    /* Implicit-self method call: inside a method body, a bare-ident call
     * `foo(args)` means `self.foo(args)` when `foo` names a method of the
     * parent struct and is not shadowed by a local / parameter. */
    if (callee && callee->kind == ND_IDENT && g->self_struct) {
        const char* cname = ident_name(callee);
        if (!scope_lookup(g->scope, cname)) {
            SymMethod* m = sema_find_method(g->st, g->self_struct->name, cname);
            if (m) {
                if (m->self_kind == SELF_NONE) {
                    /* static method of parent type, invoked unqualified. */
                    sb_printf(&g->out, "%s_%s(", m->parent, m->name);
                    for (int i = 0; i < nargs; i++) {
                        if (i) sb_puts(&g->out, ", ");
                        emit_expr(g, args[i]);
                    }
                    sb_putc(&g->out, ')');
                    return;
                }
                /* Emit `self` (or &self / *self as required by callee/caller
                 * self-kind combination). `self` is always in scope here. */
                sb_printf(&g->out, "%s_%s(", m->parent, m->name);
                if (m->self_kind == SELF_REF) {
                    /* callee expects ref-self: pass pointer. */
                    if (g->self_kind == SELF_REF) sb_puts(&g->out, "self");
                    else                          sb_puts(&g->out, "&(self)");
                } else { /* SELF_VALUE */
                    if (g->self_kind == SELF_REF) sb_puts(&g->out, "(*self)");
                    else                          sb_puts(&g->out, "self");
                }
                for (int i = 0; i < nargs; i++) {
                    sb_puts(&g->out, ", ");
                    emit_expr(g, args[i]);
                }
                sb_putc(&g->out, ')');
                return;
            }
        }
    }

    if (callee && callee->kind == ND_MEMBER) {
        Node*       recv  = member_receiver(callee);
        const char* mname = member_name    (callee);

        /* Static call: T.foo(...) or Name<T>.foo(...)
         *
         * Phase 4: if `recv` is an ND_IDENT whose `type` field carries a
         * generic instantiation (set by parse_primary speculation), the
         * effective struct name is the mangled name from SymMono and we
         * look up the method on the template struct. */
        Type* recv_type = (recv && recv->kind == ND_IDENT) ? ast_type(recv) : NULL;
        bool recv_is_generic = (recv_type && ty_is_named(recv_type) && ty_ntargs(recv_type) > 0);

        if (recv_is_generic || ident_is_struct_type(g, recv)) {
            const char* tname = recv_is_generic ? ty_name(recv_type) : ident_name(recv);
            SymMethod*  m     = sema_find_method(g->st, tname, mname);
            if (m && m->self_kind == SELF_NONE) {
                /* Determine the C-level prefix for the call. */
                if (recv_is_generic) {
                    Type* rt = resolve_type(g, recv_type);
                    sb_printf(&g->out, "%s_%s(", ty_mangle(rt), mname);
                } else {
                    sb_printf(&g->out, "%s_%s(", ident_name(recv), mname);
                }
                for (int i = 0; i < nargs; i++) {
                    if (i) sb_puts(&g->out, ", ");
                    emit_expr(g, args[i]);
                }
                sb_putc(&g->out, ')');
                return;
            }
            /* Unknown static; fall through to the "ordinary call" path. */
        }

        if (!recv_is_generic) {
            /* Instance call: recv.foo(...). Use the resolved receiver type
             * to find the method in sema, then let emit_method_call handle
             * the self-kind adaptation and arg list. */
            Type* lt = expr_type(g, recv);
            SymStruct* s = struct_of_type(g, lt);
            if (s) {
                SymMethod* m = sema_find_method(g->st, s->name, mname);
                if (m && m->self_kind != SELF_NONE) {
                    emit_method_call(g, m, recv, args, nargs);
                    return;
                }
            }
        }
    }

    /* Ordinary call: just emit callee then parenthesised args. */
    /* Handle compiler intrinsics: __alignof__ → __alignof (MSVC) */
    if (callee && callee->kind == ND_IDENT) {
        const char* cname = ident_name(callee);
        if (strcmp(cname, "__alignof__") == 0) {
            sb_puts(&g->out, "__alignof(");
            for (int i = 0; i < nargs; i++) {
                if (i) sb_puts(&g->out, ", ");
                emit_expr(g, args[i]);
            }
            sb_putc(&g->out, ')');
            return;
        }
    }
    emit_expr(g, callee);
    sb_putc(&g->out, '(');
    for (int i = 0; i < nargs; i++) {
        if (i) sb_puts(&g->out, ", ");
        emit_expr(g, args[i]);
    }
    sb_putc(&g->out, ')');
}

static void emit_expr(G* g, Node* e) {
    if (!e) { sb_puts(&g->out, "/*null-expr*/0"); return; }
    switch (e->kind) {
        case ND_INT:
            sb_printf(&g->out, "%lld", int_value(e));
            return;
        case ND_FLOAT: {
            char buf[64];
            snprintf(buf, sizeof buf, "%.17g", float_value(e));
            sb_puts(&g->out, buf);
            /* If the render lacks a decimal/exponent, add ".0" so the C
             * compiler sees a double literal (not an int). */
            bool has_dot = false;
            for (const char* q = buf; *q; q++)
                if (*q == '.' || *q == 'e' || *q == 'E' || *q == 'n') { has_dot = true; break; }
            if (!has_dot) sb_puts(&g->out, ".0");
            return;
        }
        case ND_STR:   emit_str_literal (g, str_bytes(e), str_len(e)); return;
        case ND_CHAR:  emit_char_literal(g, str_bytes(e), str_len(e)); return;
        case ND_BOOL:  sb_puts(&g->out, bool_value(e) ? "true" : "false"); return;
        case ND_NULL:  sb_puts(&g->out, "NULL"); return;
        case ND_IDENT: emit_ident(g, ident_name(e)); return;

        case ND_BINOP: {
            /* Phase-3 operator dispatch: if LHS is a struct type that defines
             * a matching `static T operator<op>(T, U)` (nparams==2), emit a
             * direct call instead of the primitive C operator. */
            Node*  lhs = binop_lhs(e);
            Node*  rhs = binop_rhs(e);
            OpKind op  = binop_op (e);
            const char* mname = cg_binop_method_name(op);
            if (mname) {
                Type* lt = expr_type(g, lhs);
                SymStruct* s = ty_is_named(lt) ? sema_find_struct(g->st, ty_name(lt)) : NULL;
                if (s) {
                    SymMethod* m = sema_find_method(g->st, s->name, mname);
                    if (m && m->self_kind == SELF_NONE && m->decl &&
                        func_nparams(m->decl) == 2) {
                        sb_printf(&g->out, "%s_%s(", s->name, mname);
                        emit_expr(g, lhs);
                        sb_puts(&g->out, ", ");
                        emit_expr(g, rhs);
                        sb_putc(&g->out, ')');
                        return;
                    }
                }
            }
            sb_putc(&g->out, '(');
            emit_expr(g, lhs);
            sb_printf(&g->out, " %s ", bin_op_c(op));
            emit_expr(g, rhs);
            sb_putc(&g->out, ')');
            return;
        }

        case ND_UNOP: {
            Node*  operand = unop_operand(e);
            OpKind op      = unop_op     (e);

            /* Postfix ++/-- have operand-first emit shape. */
            if (op == OP_POSTINC || op == OP_POSTDEC) {
                sb_putc(&g->out, '(');
                emit_expr(g, operand);
                sb_puts(&g->out, op == OP_POSTINC ? "++" : "--");
                sb_putc(&g->out, ')');
                return;
            }

            /* Phase-3 unary operator dispatch. OP_NEG (-) maps to "op_sub"
             * but disambiguated by nparams==1 from the binary form. */
            const char* mname = cg_unop_method_name(op);
            if (mname) {
                Type* ot = expr_type(g, operand);
                SymStruct* s = ty_is_named(ot) ? sema_find_struct(g->st, ty_name(ot)) : NULL;
                if (s) {
                    SymMethod* m = sema_find_method(g->st, s->name, mname);
                    if (m && m->self_kind == SELF_NONE && m->decl &&
                        func_nparams(m->decl) == 1) {
                        sb_printf(&g->out, "%s_%s(", s->name, mname);
                        emit_expr(g, operand);
                        sb_putc(&g->out, ')');
                        return;
                    }
                }
            }

            /* Primitive fallback. */
            const char* csym = "?";
            switch (op) {
                case OP_NOT:   csym = "!"; break;
                case OP_NEG:   csym = "-"; break;
                case OP_POS:   csym = "+"; break;
                case OP_BNOT:  csym = "~"; break;
                case OP_ADDR:  csym = "&"; break;
                case OP_DEREF: csym = "*"; break;
                default: break;
            }
            sb_printf(&g->out, "(%s", csym);
            emit_expr(g, operand);
            sb_putc(&g->out, ')');
            return;
        }

        case ND_INDEX: {
            /* a[i] — raw-pointer subscript; plain C semantics. */
            Node* recv = index_receiver(e);
            Node* idx  = index_index   (e);
            sb_putc(&g->out, '(');
            emit_expr(g, recv);
            sb_puts(&g->out, ")[");
            emit_expr(g, idx);
            sb_putc(&g->out, ']');
            return;
        }

        case ND_ASSIGN: {
            Node*  lhs = assign_lhs(e);
            Node*  rhs = assign_rhs(e);
            OpKind op  = assign_op (e);
            sb_putc(&g->out, '(');
            emit_expr(g, lhs);
            sb_printf(&g->out, " %s ", assign_op_c(op));
            emit_expr(g, rhs);
            sb_putc(&g->out, ')');
            return;
        }

        case ND_CALL:       emit_call(g, e);        return;
        case ND_MEMBER:     emit_member(g, e);      return;
        case ND_STRUCT_LIT: emit_struct_lit(g, e);  return;

        case ND_CAST: {
            /* C-style type cast: (Type)expr */
            Type* ty = cast_type(e);
            sb_putc(&g->out, '(');
            emit_type(g, ty);
            sb_putc(&g->out, ')');
            emit_expr(g, cast_expr(e));
            return;
        }

        case ND_SIZEOF: {
            /* sizeof(Type) or sizeof(expr) */
            sb_puts(&g->out, "sizeof(");
            if (e->declared_type) {
                /* sizeof(Type) form */
                emit_type(g, e->declared_type);
            } else if (e->rhs) {
                /* sizeof(expr) form */
                emit_expr(g, e->rhs);
            }
            sb_putc(&g->out, ')');
            return;
        }

        case ND_PRINT:
        case ND_PRINTLN: {
            /* print(expr) / println(expr). Best-effort type-aware dispatch
             * to the right printf format spec; unknown types get "%p". */
            Node* val = (e->kind == ND_PRINT) ? print_value(e) : println_value(e);
            Type* vt  = expr_type(g, val);
            const char* fmt = NULL;
            if (vt) {
                switch (ty_kind(vt)) {
                    case TY_CHAR: case TY_SHORT: case TY_INT:
                        fmt = "%d"; break;
                    case TY_LONG: case TY_LONGLONG:
                        fmt = "%lld"; break;
                    case TY_FLOAT: case TY_DOUBLE:
                        fmt = "%g"; break;
                    case TY_BOOL:
                        fmt = NULL;    /* special-cased below */
                        break;
                    case TY_PTR:
                        /* char* → treat as string (print char arrays). */
                        if (ty_base(vt) && ty_kind(ty_base(vt)) == TY_CHAR)
                            fmt = "%s";
                        else
                            fmt = "%p";
                        break;
                    default:
                        fmt = "%p"; break;
                }
            }
            const char* nl = (e->kind == ND_PRINTLN) ? "\\n" : "";
            if (!fmt && vt && ty_is_bool(vt)) {
                /* printf("%s", expr ? "true" : "false") */
                sb_printf(&g->out, "printf(\"%%s%s\", (", nl);
                emit_expr(g, val);
                sb_puts(&g->out, ") ? \"true\" : \"false\")");
            } else {
                if (!fmt) fmt = "%p";
                sb_printf(&g->out, "printf(\"%s%s\", ", fmt, nl);
                emit_expr(g, val);
                sb_putc(&g->out, ')');
            }
            return;
        }

        default:
            sb_puts(&g->out, "/*??expr*/");
            return;

        case ND_INIT_LIST: {
            sb_putc(&g->out, '{');
            for (int i = 0; i < e->nchildren; i++) {
                if (i) sb_puts(&g->out, ", ");
                emit_expr(g, e->children[i]);
            }
            sb_putc(&g->out, '}');
            return;
        }

        case ND_DESIG_INIT: {
            if (e->name) {
                sb_printf(&g->out, ".%s = ", e->name);
            } else if (e->lhs) {
                sb_putc(&g->out, '[');
                emit_expr(g, e->lhs);
                sb_puts(&g->out, "] = ");
            }
            emit_expr(g, e->rhs);
            return;
        }

        case ND_COMPOUND_LIT: {
            sb_putc(&g->out, '(');
            emit_type(g, e->declared_type);
            sb_putc(&g->out, ')');
            if (e->rhs && e->rhs->kind == ND_INIT_LIST) {
                emit_expr(g, e->rhs);
            } else {
                sb_putc(&g->out, '{');
                if (e->rhs) emit_expr(g, e->rhs);
                sb_putc(&g->out, '}');
            }
            return;
        }
    }
}

/* ---------- statement emission ----------
 *
 * emit_stmt / emit_block return `true` when the emitted statement is
 * *definitely a terminator* (control cannot fall through to what follows).
 * This lets the caller — in particular emit_block — avoid generating
 * unreachable scope-end destructor calls after a `return`, `break`,
 * or `continue` that sits as the last statement of a block.
 *
 * For compound constructs (if/else, for/while) we stay conservative:
 * we only report "terminator" for `return` / `break` / `continue` themselves
 * and for an if/else whose both branches always terminate. Everything else
 * reports false, which is always safe (possibly-dead dtor calls only waste
 * a line; the C compiler DCEs them).
 */
static bool emit_stmt(G* g, Node* s);

static bool emit_block(G* g, Node* blk, bool already_indented, bool push_scope) {
    if (!already_indented) pad(g);
    sb_puts(&g->out, "{\n");
    g->indent++;
    if (push_scope) scope_push(&g->scope);

    bool terminated = false;
    for (int i = 0; i < blk->nchildren; i++) {
        terminated = emit_stmt(g, blk->children[i]);
        /* Keep emitting remaining stmts even after a terminator; user's
         * dead code is the user's problem and the C compiler will warn. */
    }

    /* Leap C: fall-through drops are now injected as explicit ND_DROP
     * nodes by lower.c. cgen no longer emits them here — the nodes are
     * already in blk->children and got emitted in the loop above. */
    (void)terminated;

    if (push_scope) scope_pop(&g->scope);
    g->indent--;
    pad(g);
    sb_puts(&g->out, "}\n");
    return terminated;
}

static void emit_vardecl_inline(G* g, Node* s);  /* fwd */

/* Emit the body of a loop (for/while) with its body-scope marked as
 * SCOPE_LOOP_BODY so that `break`/`continue` inside it unwind correctly.
 * A single-statement body is always wrapped in braces — this lets the
 * enclosing scope hold the SCOPE_LOOP_BODY marker without also requiring
 * dtor emission in the trivially-no-vars case. */
static void emit_loop_body(G* g, Node* body) {
    if (body && body->kind == ND_BLOCK) {
        /* Push an explicit scope tagged as LOOP_BODY, then emit stmts. */
        pad(g);
        sb_puts(&g->out, "{\n");
        g->indent++;
        scope_push(&g->scope);
        g->scope->kind = SCOPE_LOOP_BODY;

        /* Leap C: drops for locals declared in this body are already
         * injected as ND_DROP nodes by lower.c. */
        for (int i = 0; i < body->nchildren; i++)
            (void)emit_stmt(g, body->children[i]);

        scope_pop(&g->scope);
        g->indent--;
        pad(g);
        sb_puts(&g->out, "}\n");
    } else {
        /* Wrap the single stmt in braces so we can still tag the scope. */
        pad(g);
        sb_puts(&g->out, "{\n");
        g->indent++;
        scope_push(&g->scope);
        g->scope->kind = SCOPE_LOOP_BODY;

        (void)emit_stmt(g, body);

        scope_pop(&g->scope);
        g->indent--;
        pad(g);
        sb_puts(&g->out, "}\n");
    }
}

/* Statement emitter forward declaration. */
static bool emit_stmt_core(G* g, Node* s);

static bool emit_stmt(G* g, Node* s) {
    return emit_stmt_core(g, s);
}

static bool emit_stmt_core(G* g, Node* s) {
    if (!s) { pad(g); sb_puts(&g->out, "/*null-stmt*/;\n"); return false; }
    switch (s->kind) {
        case ND_DROP: {
            /* Leap C: the lowering pass already decided where this drop
             * should appear. cgen just emits the call. */
            pad(g);
            sb_printf(&g->out, "%s___drop(&%s);\n",
                      drop_struct_name(s), drop_var_name(s));
            return false;
        }

        case ND_BLOCK:
            return emit_block(g, s, false, true);

        case ND_VARDECL:
            pad(g);
            emit_vardecl_inline(g, s);
            sb_puts(&g->out, ";\n");
            return false;

        case ND_EXPR_STMT:
            pad(g);
            emit_expr(g, expr_stmt_expr(s));
            sb_puts(&g->out, ";\n");
            return false;

        case ND_RETURN: {
            /* All RAII unwinding on return is handled by lower.c. By the
             * time this node reaches cgen, lower has already rewritten
             * `return expr` to one of:
             *   - return expr;                          (no live dtors)
             *   - { T __ret_N = expr; drops…; return __ret_N; }
             * So here we emit a plain unconditional return. */
            Node* val = return_value(s);
            pad(g);
            if (val) { sb_puts(&g->out, "return "); emit_expr(g, val); sb_puts(&g->out, ";\n"); }
            else     { sb_puts(&g->out, "return;\n"); }
            return true;
        }

        case ND_BREAK:
            /* Leap C: any pre-break drops were already injected by the
             * lowering pass as sibling ND_DROP nodes in the same block. */
            pad(g); sb_puts(&g->out, "break;\n"); return true;

        case ND_CONTINUE:
            /* Leap C: any pre-continue drops were already injected by
             * the lowering pass as sibling ND_DROP nodes. */
            pad(g); sb_puts(&g->out, "continue;\n"); return true;

        case ND_IF: {
            Node* cond = if_cond(s);
            Node* thn  = if_then(s);
            Node* els  = if_else(s);

            pad(g);
            sb_puts(&g->out, "if (");
            emit_expr(g, cond);
            sb_puts(&g->out, ") ");
            bool then_term = false, else_term = false;
            if (thn && thn->kind == ND_BLOCK) {
                then_term = emit_block(g, thn, true, true);
            } else {
                sb_putc(&g->out, '\n');
                g->indent++; then_term = emit_stmt(g, thn); g->indent--;
            }
            if (els) {
                pad(g);
                sb_puts(&g->out, "else ");
                if (els->kind == ND_BLOCK) {
                    else_term = emit_block(g, els, true, true);
                } else {
                    sb_putc(&g->out, '\n');
                    g->indent++; else_term = emit_stmt(g, els); g->indent--;
                }
            }
            /* An if/else where BOTH branches terminate is itself a terminator. */
            return els && then_term && else_term;
        }

        case ND_WHILE:
            pad(g);
            sb_puts(&g->out, "while (");
            emit_expr(g, while_cond(s));
            sb_puts(&g->out, ")\n");
            emit_loop_body(g, while_body(s));
            return false;

        case ND_FOR: {
            /* Phase-3 rewrite: if the for-init is a vardecl, wrap the whole
             * for statement in a SCOPE_BLOCK so the init variables live in
             * a normal dtor-tracked scope. */
            Node* init = for_init(s);
            Node* cond = for_cond(s);
            Node* upd  = for_update(s);
            Node* body = for_body(s);
            bool wrap  = init && init->kind == ND_VARDECL;

            if (wrap) {
                pad(g);
                sb_puts(&g->out, "{\n");
                g->indent++;
                scope_push(&g->scope);  /* wrapper scope is plain SCOPE_BLOCK */
                pad(g);
                emit_vardecl_inline(g, init);
                sb_puts(&g->out, ";\n");
            } else if (init && init->kind == ND_EXPR_STMT) {
                pad(g);
                emit_expr(g, expr_stmt_expr(init));
                sb_puts(&g->out, ";\n");
            }

            pad(g);
            sb_puts(&g->out, "for (; ");
            if (cond) emit_expr(g, cond);
            sb_puts(&g->out, "; ");
            if (upd) emit_expr(g, upd);
            sb_puts(&g->out, ")\n");
            emit_loop_body(g, body);

            if (wrap) {
                emit_scope_dtors(g, g->scope);  /* init-var dtors on normal loop end */
                scope_pop(&g->scope);
                g->indent--;
                pad(g);
                sb_puts(&g->out, "}\n");
            }
            return false;
        }

        case ND_SWITCH: {
            pad(g);
            sb_puts(&g->out, "switch (");
            emit_expr(g, s->lhs);
            sb_puts(&g->out, ") {\n");
            g->indent++;
            for (int i = 0; i < s->nchildren; i++) {
                Node* c = s->children[i];
                if (c->kind == ND_CASE) {
                    pad(g);
                    sb_puts(&g->out, "case ");
                    emit_expr(g, c->lhs);
                    sb_puts(&g->out, ":\n");
                } else if (c->kind == ND_DEFAULT_CASE) {
                    pad(g);
                    sb_puts(&g->out, "default:\n");
                } else {
                    continue;
                }
                g->indent++;
                for (int j = 0; j < c->nchildren; j++) {
                    emit_stmt(g, c->children[j]);
                }
                g->indent--;
            }
            g->indent--;
            pad(g);
            sb_puts(&g->out, "}\n");
            return false;
        }

        case ND_ASM:
            if (s->raw_text) {
                pad(g);
                /* SEH blocks (__try/__except/__finally) don't need trailing semicolon */
                if (strncmp(s->raw_text, "__try", 5) == 0 ||
                    strncmp(s->raw_text, "__except", 8) == 0 ||
                    strncmp(s->raw_text, "__finally", 9) == 0) {
                    sb_printf(&g->out, "%s\n", s->raw_text);
                } else {
                    sb_printf(&g->out, "%s;\n", s->raw_text);
                }
            }
            return false;

        default:
            pad(g);
            sb_puts(&g->out, "/*??stmt*/;\n");
            return false;
    }
}

/* Emit `<type> <name> [= <init>]` without a trailing semicolon, and register
 * the binding in the current codegen scope. If the declared type is a struct
 * type that defines a destructor, the scope entry records that fact so the
 * appropriate `T___drop(&name)` call can be inserted at scope exit. */
/* A few short predicates for the RAII pipeline. After leap A these
 * are all one-liners over ty_*; call sites read naturally. */
static void emit_vardecl_inline(G* g, Node* s) {
    Type*       dt   = vardecl_type(s);
    const char* name = vardecl_name(s);
    Node*       init = vardecl_init(s);

    /* __declspec(thread) for thread-local storage */
    if (s->declspec) {
        sb_printf(&g->out, "__declspec(%s) ", s->declspec);
    }

    if (s->lhs) {
        /* Array declaration with explicit size: emit as C `T name[size]` */
        Type* base = (dt && ty_kind(dt) == TY_PTR) ? ty_base(dt) : dt;
        emit_type(g, base);
        if (s->cc) {
            sb_putc(&g->out, ' ');
            sb_puts(&g->out, s->cc);
        }
        sb_printf(&g->out, " %s[", name);
        emit_expr(g, s->lhs);
        sb_putc(&g->out, ']');
    } else {
        /* Check if cc is __unaligned — in MSVC it must appear before * */
        if (s->cc && strcmp(s->cc, "__unaligned") == 0) {
            /* Emit as: type_with_name but insert __unaligned before * */
            emit_type_with_name_unaligned(g, dt, name);
        } else if (s->cc) {
            emit_type_with_name(g, dt, name);
            sb_putc(&g->out, ' ');
            sb_puts(&g->out, s->cc);
        } else {
            emit_type_with_name(g, dt, name);
        }
    }
    if (init) {
        sb_puts(&g->out, " = ");
        emit_expr(g, init);
    }
    if (g->scope) {
        scope_add(g->scope, s->name, dt, dtor_struct_name_for(g, dt));
    }
}

/* ---------- top-level emission ---------- */
static void emit_struct_fwd(G* g, Node* s) {
    sb_printf(&g->out, "typedef struct %s %s;\n", s->name, s->name);
}

static void emit_union_decl(G* g, Node* u) {
    sb_printf(&g->out, "typedef union %s %s;\n", u->name, u->name);
}

static void emit_union_body(G* g, Node* u) {
    sb_printf(&g->out, "union %s {\n", u->name);
    for (int i = 0; i < u->nfields; i++) {
        Node* f = u->fields[i];
        sb_puts(&g->out, "    ");
        emit_type_with_name(g, f->declared_type, f->name);
        if (f->lhs) {
            sb_putc(&g->out, '[');
            emit_expr(g, f->lhs);
            sb_putc(&g->out, ']');
        }
        sb_puts(&g->out, ";\n");
    }
    sb_printf(&g->out, "};\n\n");
}

static void emit_struct_fwd_decl(G* g, Node* s) {
    sb_printf(&g->out, "typedef struct %s %s;\n", s->name, s->name);
}

static void emit_union_fwd_decl(G* g, Node* u) {
    sb_printf(&g->out, "typedef union %s %s;\n", u->name, u->name);
}

static bool is_crt_sdk_typedef(const char* name) {
    static const char* skip[] = {
        "wchar_t", "wint_t", "wctype_t",
        "__time32_t", "__time64_t", "time_t",
        "_locale_t", "_locale_tstruct",
        "size_t", "ptrdiff_t", "intptr_t", "uintptr_t",
        "ssize_t", "int8_t", "int16_t", "int32_t", "int64_t",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t",
        "va_list", "__gnuc_va_list",
        "pthreadlocinfo", "pthreadmbcinfo",
        NULL
    };
    for (int i = 0; skip[i]; i++)
        if (strcmp(name, skip[i]) == 0) return true;
    return false;
}

/* C compiler built-in types that cannot be redefined in C source.
 * Unlike wchar_t (which is user-definable as a typedef), these are
 * compiler intrinsics (§6.4.2.5, §7.19) that the C standard provides.
 * Even if user code declares them for sharpc's type system, we must
 * not emit them to C — the compiler already knows about them.
 *
 * This is separate from is_crt_sdk_typedef():
 *   - is_crt_sdk_typedef() filters SYSTEM HEADER typedefs (va_list, etc.)
 *   - is_c_compiler_builtin_type() filters C COMPILER BUILTINS (size_t, etc.) */
static bool is_c_compiler_builtin_type(const char* name) {
    static const char* builtins[] = {
        "size_t", "ptrdiff_t", "nullptr_t", "max_align_t",
        NULL
    };
    for (int i = 0; builtins[i]; i++)
        if (strcmp(name, builtins[i]) == 0) return true;
    return false;
}

static void emit_typedef_decl(G* g, Node* d) {
    /* Phase C1: Typedef emission filtering.
     *
     * Rule 1: System header typedefs with CRT names → skip
     *   (provided by the C runtime, don't re-emit)
     *
     * Rule 2: C compiler builtin types → always skip
     *   (size_t, ptrdiff_t, etc. are compiler intrinsics that cannot
     *    be re-typedef'd in C source, even by user code)
     *
     * Rule 3: User-defined typedefs → emit normally
     *   (all other typedefs from user source code are emitted as-is) */
    if (is_system_header_path(d->source_file) && is_crt_sdk_typedef(d->name)) {
        return;  /* Rule 1: system header typedef — skip */
    }
    if (is_c_compiler_builtin_type(d->name)) {
        return;  /* Rule 2: C compiler builtin — skip */
    }

    /* Rule 3: User-defined typedef — emit normally */
    Type* dt = d->declared_type;
    if (dt && dt->kind == TY_NAMED) {
        SymStruct* ss = sema_find_struct(g->st, dt->name);
        if (ss) {
            sb_printf(&g->out, "typedef struct %s %s", dt->name, d->name);
            if (d->lhs) {
                sb_putc(&g->out, '[');
                emit_expr(g, d->lhs);
                sb_putc(&g->out, ']');
            }
            sb_puts(&g->out, ";\n");
            return;
        }
    }
    sb_puts(&g->out, "typedef ");
    emit_type_with_name(g, dt, d->name);
    if (d->lhs) {
        sb_putc(&g->out, '[');
        emit_expr(g, d->lhs);
        sb_putc(&g->out, ']');
    }
    sb_puts(&g->out, ";\n");
}

static bool is_crt_sdk_struct(const char* name) {
    static const char* skip[] = {
        "_heapinfo", "_HEAPINFO",
        "_div_t", "_ldiv_t", "_lldiv_t",
        "_iobuf", "__acrt_iob_func",
        "_localeinfo_struct",
    };
    for (size_t i = 0; i < sizeof(skip)/sizeof(skip[0]); i++) {
        if (strcmp(skip[i], name) == 0) return true;
    }
    return false;
}

static void emit_struct_body(G* g, Node* s) {
    if (is_crt_sdk_struct(s->name)) return;
    sb_printf(&g->out, "struct %s {\n", s->name);
    if (s->nfields == 0) {
        sb_puts(&g->out, "    char _sharp_unit_;\n");
    }
    for (int i = 0; i < s->nfields; i++) {
        Node* f = s->fields[i];
        sb_puts(&g->out, "    ");

        /* Check if this field has nested struct/union fields.
         * If so, emit the nested struct definition inline before the field. */
        if (f->fields && f->nfields > 0) {
            /* Get the struct/union tag name from the type */
            const char* tag_name = f->name;
            if (f->declared_type && f->declared_type->name) {
                tag_name = f->declared_type->name;
            }
            bool is_union = (f->kind == ND_ANON_UNION) ||
                            (f->declared_type && f->declared_type->name &&
                             strncmp(f->declared_type->name, "union ", 6) == 0);

            /* Check if anonymous (name starts with __anon_) */
            bool is_anon = (tag_name && strncmp(tag_name, "__anon_", 7) == 0);

            if (is_anon) {
                /* Anonymous struct/union: emit inline definition with no tag */
                sb_puts(&g->out, is_union ? "union {\n" : "struct {\n");
                for (int j = 0; j < f->nfields; j++) {
                    Node* nf = f->fields[j];
                    sb_puts(&g->out, "        ");
                    emit_type_with_name(g, nf->declared_type, nf->name);
                    if (nf->lhs) {
                        sb_putc(&g->out, '[');
                        emit_expr(g, nf->lhs);
                        sb_putc(&g->out, ']');
                    }
                    if (nf->bit_width > 0) {
                        sb_printf(&g->out, " : %d", nf->bit_width);
                    }
                    sb_puts(&g->out, ";\n");
                }
                sb_puts(&g->out, "    }");
                if (!is_union && f->kind == ND_ANON_STRUCT) {
                    /* Anonymous struct — no field name needed */
                }
                sb_puts(&g->out, ";\n");
            } else {
                /* Named nested struct: emit full definition with tag and field name */
                const char* kw = is_union ? "union" : "struct";
                /* tag_name might already include "struct " or "union " prefix */
                const char* tag = tag_name;
                if (tag_name && strncmp(tag_name, "struct ", 7) == 0) tag = tag_name + 7;
                else if (tag_name && strncmp(tag_name, "union ", 6) == 0) tag = tag_name + 6;
                sb_printf(&g->out, "%s %s {\n", kw, tag);
                for (int j = 0; j < f->nfields; j++) {
                    Node* nf = f->fields[j];
                    sb_puts(&g->out, "        ");
                    emit_type_with_name(g, nf->declared_type, nf->name);
                    if (nf->lhs) {
                        sb_putc(&g->out, '[');
                        emit_expr(g, nf->lhs);
                        sb_putc(&g->out, ']');
                    }
                    if (nf->bit_width > 0) {
                        sb_printf(&g->out, " : %d", nf->bit_width);
                    }
                    sb_puts(&g->out, ";\n");
                }
                sb_printf(&g->out, "    } %s;\n", f->name ? f->name : "");
            }
        } else {
            emit_type_with_name(g, f->declared_type, f->name);
            if (f->lhs) {
                sb_putc(&g->out, '[');
                emit_expr(g, f->lhs);
                sb_putc(&g->out, ']');
            }
            sb_puts(&g->out, ";\n");
        }
    }
    sb_puts(&g->out, "};\n\n");
}

static void emit_func_signature(G* g, Node* f) {
    bool is_main = f->parent_type == NULL && strcmp(f->name, "main") == 0;
    if (!is_main) sb_puts(&g->out, "static ");
    emit_type(g, f->ret_type);
    if (f->cc) {
        sb_putc(&g->out, ' ');
        sb_puts(&g->out, f->cc);
    }
    sb_putc(&g->out, ' ');
    if (f->parent_type) {
        /* Destructor sentinel name "~" is mangled to the C-valid suffix
         * "__drop", producing `Parent___drop(...)`. The user cannot write
         * `__drop` as a method name because it would just be an identifier;
         * the sentinel "~" originates only from parse_dtor. */
        bool is_dtor = f->name && f->name[0] == '~' && f->name[1] == 0;
        /* Phase 4: when emitting a generic specialisation, the C-level
         * parent name is g->self_mangled (e.g. "Span_u8") rather than the
         * template's own name. */
        const char* pname = g->self_mangled ? g->self_mangled : f->parent_type;
        if (is_dtor) sb_printf(&g->out, "%s___drop(", pname);
        else         sb_printf(&g->out, "%s_%s(", pname, f->name);
    } else {
        sb_printf(&g->out, "%s(", f->name);
    }

    if (f->nparams == 0) {
        sb_puts(&g->out, "void");
    } else {
        for (int i = 0; i < f->nparams; i++) {
            Node* pr = f->params[i];
            if (i) sb_puts(&g->out, ", ");
            emit_type_with_name(g, pr->declared_type, pr->name);
        }
    }
    sb_putc(&g->out, ')');
}

static void emit_func_decl(G* g, Node* f) {
    emit_func_signature(g, f);
    sb_puts(&g->out, ";\n");
}

static void emit_func_body(G* g, Node* f) {
    SymStruct* saved_ss  = g->self_struct;
    int        saved_sk  = g->self_kind;
    Type*      saved_rt  = g->cur_ret_type;
    g->self_kind    = f->self_kind;
    g->cur_ret_type = f->ret_type;

    if (f->parent_type) {
        /* Phase 4: when inside a generic specialisation the template struct
         * (e.g. "Span") is what sema knows; its SymStruct carries the
         * fields needed for bare-name resolution inside the body. */
        g->self_struct = sema_find_struct(g->st, f->parent_type);
    } else {
        g->self_struct = NULL;
    }

    scope_push(&g->scope);
    g->scope->kind = SCOPE_FUNC;
    for (int i = 0; i < f->nparams; i++) {
        Node* pr = f->params[i];
        scope_add(g->scope, pr->name, pr->declared_type, NULL);
    }

    emit_func_signature(g, f);
    sb_puts(&g->out, " ");
    emit_block(g, f->body, true, false);
    sb_puts(&g->out, "\n");

    scope_pop(&g->scope);
    g->self_struct  = saved_ss;
    g->self_kind    = saved_sk;
    g->cur_ret_type = saved_rt;
}

/* Is this AST node a generic template declaration? Such declarations are
 * not emitted themselves — one specialisation per SymMono is emitted in
 * their place, with the active type subst plugging T → concrete. */
static bool is_generic_template(Node* d) {
    if (!d) return false;
    if (d->kind == ND_STRUCT_DECL) return d->ntype_params > 0;
    if (d->kind == ND_IMPL)        return d->ntype_params > 0;
    return false;
}

/* Find the ND_STRUCT_DECL for a given generic name, or NULL. */
static Node* find_generic_struct_decl(Node* prog, const char* name) {
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STRUCT_DECL && d->ntype_params > 0 &&
            strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}

/* Find the ND_IMPL matching a generic struct name, or NULL. */
static Node* find_generic_impl(Node* prog, const char* name) {
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_IMPL && d->ntype_params > 0 &&
            strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}

/* Enter a generic-specialisation context: record subst env and the
 * mangled parent name. Call pair with exit_spec_ctx(). */
static void enter_spec_ctx(G* g, Node* tmpl, SymMono* mono) {
    g->tsubst_names      = tmpl->type_params;
    g->tsubst_types      = mono->targs;
    g->ntsubst           = tmpl->ntype_params;
    g->self_mangled      = mono->mangled;
    g->self_template_name = mono->generic_name;
}
static void exit_spec_ctx(G* g) {
    g->tsubst_names       = NULL;
    g->tsubst_types       = NULL;
    g->ntsubst            = 0;
    g->self_mangled       = NULL;
    g->self_template_name = NULL;
}

void cgen_c(Node* prog, SymTable* st, FILE* out) {
    G gctx = {0}; G* g = &gctx;
    sb_init(&g->out);
    g->st    = st;

    Arena* cg_arena = NULL;
    g->arena = &cg_arena;

    sb_puts(&g->out,
        "/* Generated by sharpc. Do not edit by hand. */\n"
        "\n");

    /* Phase 7: emit extern function declarations. */
    /* Detect alloca / __builtin_alloca usage — need <malloc.h> for MSVC */
    bool need_malloc_h = false;
    for (int i = 0; i < st->nexterns; i++) {
        SymExtern* e = &st->externs[i];
        if (strcmp(e->name, "alloca") == 0 ||
            strcmp(e->name, "__builtin_alloca") == 0 ||
            strcmp(e->name, "_alloca") == 0) {
            need_malloc_h = true;
        }
    }
    if (need_malloc_h) {
        sb_puts(&g->out, "#include <malloc.h>\n");
    }

    /* ------- Forward declarations (must come before types/externs that reference them) ------- */
    /* Union forward declarations first (anonymous unions may be referenced by typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_UNION_DECL)
            emit_union_decl(g, d);
        if (d->kind == ND_UNION_FWD)
            emit_union_fwd_decl(g, d);
    }
    /* Struct forward declarations (anonymous structs may be referenced by typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STRUCT_DECL && !is_generic_template(d))
            emit_struct_fwd(g, d);
        if (d->kind == ND_STRUCT_FWD)
            emit_struct_fwd_decl(g, d);
    }
    /* Enum definitions (full definitions must come before typedefs that reference them,
     * as enums cannot be forward-declared in C11). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_ENUM_DECL && d->raw_text) {
            sb_printf(&g->out, "enum %s {\n", d->name);
            sb_puts(&g->out, d->raw_text);
            sb_puts(&g->out, "\n};\n\n");
        }
    }
    /* Typedef declarations (including array typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind != ND_TYPEDEF_DECL) continue;
        /* If the underlying type is an anonymous enum (name starts with __anon_enum_),
         * emit as `typedef enum X Name;` since C11 requires the enum tag. */
        Type* base = d->declared_type;
        while (base && (base->kind == TY_ARRAY || base->kind == TY_PTR))
            base = base->base;
        if (base && base->kind == TY_NAMED &&
            strncmp(base->name, "__anon_enum_", 12) == 0) {
            sb_printf(&g->out, "typedef enum %s %s;\n", base->name, d->name);
        } else {
            emit_typedef_decl(g, d);
        }
    }
    /* Specialised structs: one per SymMono. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        sb_printf(&g->out, "typedef struct %s %s;\n", m->mangled, m->mangled);
    }
    sb_putc(&g->out, '\n');

    for (int i = 0; i < st->nexterns; i++) {
        SymExtern* e = &st->externs[i];
    /* Skip alloca-like intrinsics — they are provided by <malloc.h> or compiler */
        if (strcmp(e->name, "alloca") == 0 ||
            strcmp(e->name, "__builtin_alloca") == 0 ||
            strcmp(e->name, "_alloca") == 0 ||
            strcmp(e->name, "__alignof__") == 0 ||
            strcmp(e->name, "__builtin_frame_address") == 0 ||
            strcmp(e->name, "__builtin_return_address") == 0) {
            continue;
        }
        /* __declspec(noreturn) → __declspec(noreturn) for MSVC compatibility */
        if (e->decl->declspec && strcmp(e->decl->declspec, "noreturn") == 0) {
            sb_puts(&g->out, "__declspec(noreturn) ");
        }
        sb_puts(&g->out, "extern ");

        /* Check if return type is a function pointer */
        Type* ret = resolve_type(g, e->ret_type);
        Type* func_type = NULL;
        if (ret && ret->kind == TY_FUNC) {
            func_type = ret;
        } else if (ret && ret->kind == TY_PTR && ret->base && ret->base->kind == TY_FUNC) {
            func_type = ret->base;
        }

        if (func_type) {
            /* ret (*name(func_params))(ret_params) */
            emit_type_core(g, func_type->base);
            sb_printf(&g->out, " (*%s(", e->name);
            for (int j = 0; j < e->decl->nparams; j++) {
                Node* pr = e->decl->params[j];
                if (j) sb_puts(&g->out, ", ");
                emit_type_with_name(g, pr->declared_type, pr->name);
            }
            sb_puts(&g->out, "))(");
            for (int i = 0; i < func_type->nfunc_params; i++) {
                if (i) sb_puts(&g->out, ", ");
                emit_type(g, func_type->func_params[i]);
            }
            if (func_type->func_variadic) {
                if (func_type->nfunc_params > 0) sb_puts(&g->out, ", ");
                sb_puts(&g->out, "...");
            }
            sb_puts(&g->out, ");\n");
        } else {
            emit_type(g, e->ret_type);
            if (e->decl->cc) {
                sb_putc(&g->out, ' ');
                sb_puts(&g->out, e->decl->cc);
            }
            sb_printf(&g->out, " %s(", e->name);
            if (e->decl->nparams == 0 && !e->decl->is_variadic) {
                sb_puts(&g->out, "void");
            } else {
                for (int j = 0; j < e->decl->nparams; j++) {
                    Node* pr = e->decl->params[j];
                    if (j) sb_puts(&g->out, ", ");
                    emit_type_with_name(g, pr->declared_type, pr->name);
                }
                if (e->decl->is_variadic) {
                    if (e->decl->nparams > 0) sb_puts(&g->out, ", ");
                    sb_puts(&g->out, "...");
                }
            }
            sb_puts(&g->out, ");\n");
        }
    }

    /* Extern variable declarations: `extern Type name;` */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind != ND_EXTERN_VAR) continue;
        /* __declspec(dllimport) → pass-through as-is */
        if (d->declspec) {
            sb_printf(&g->out, "__declspec(%s) ", d->declspec);
        }
        sb_puts(&g->out, "extern ");
        emit_type(g, d->declared_type);
        sb_printf(&g->out, " %s;\n", d->name);
    }

    /* Global variable declarations: `Type name;` (including __declspec(thread)) */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind != ND_VARDECL) continue;
        if (d->declspec) {
            sb_printf(&g->out, "__declspec(%s) ", d->declspec);
        }
        emit_type_with_name(g, d->declared_type, d->name);
        if (d->rhs) {
            sb_puts(&g->out, " = ");
            emit_expr(g, d->rhs);
        }
        sb_puts(&g->out, ";\n");
    }

    if (st->nexterns > 0) sb_putc(&g->out, '\n');

    /* _Static_assert pass-through */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STATIC_ASSERT && d->raw_text)
            sb_printf(&g->out, "%s\n", d->raw_text);
    }

    /* ------- Function / method forward declarations ------- */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_FUNC_DECL) {
            emit_func_decl(g, d);
        } else if (d->kind == ND_IMPL && !is_generic_template(d)) {
            for (int j = 0; j < d->nchildren; j++)
                if (d->children[j]->kind == ND_FUNC_DECL)
                    emit_func_decl(g, d->children[j]);
        }
    }
    /* Specialised method forward decls: for each mono, look up its impl
     * template and emit one forward decl per method. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        Node* tmpl_i = find_generic_impl(prog, m->generic_name);
        if (!tmpl_s || !tmpl_i) continue;
        enter_spec_ctx(g, tmpl_s, m);
        for (int j = 0; j < tmpl_i->nchildren; j++)
            if (tmpl_i->children[j]->kind == ND_FUNC_DECL)
                emit_func_decl(g, tmpl_i->children[j]);
        exit_spec_ctx(g);
    }
    sb_putc(&g->out, '\n');

    /* ------- Struct bodies ------- */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STRUCT_DECL && !is_generic_template(d))
            emit_struct_body(g, d);
    }
    /* Union bodies. */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_UNION_DECL)
            emit_union_body(g, d);
    }
    /* Specialised struct bodies. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];

        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        if (!tmpl_s) continue;
        enter_spec_ctx(g, tmpl_s, m);
        sb_printf(&g->out, "struct %s {\n", m->mangled);
        if (tmpl_s->nfields == 0) sb_puts(&g->out, "    char _sharp_unit_;\n");
        for (int k = 0; k < tmpl_s->nfields; k++) {
            Node* f = tmpl_s->fields[k];
            sb_puts(&g->out, "    ");
            emit_type(g, f->declared_type);
            sb_printf(&g->out, " %s;\n", f->name);
        }
        sb_puts(&g->out, "};\n\n");
        exit_spec_ctx(g);
    }

    /* ------- Function / method bodies ------- */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_FUNC_DECL) {
            emit_func_body(g, d);
        } else if (d->kind == ND_IMPL && !is_generic_template(d)) {
            for (int j = 0; j < d->nchildren; j++)
                if (d->children[j]->kind == ND_FUNC_DECL)
                    emit_func_body(g, d->children[j]);
        }
    }
    /* Specialised method bodies. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        Node* tmpl_i = find_generic_impl(prog, m->generic_name);
        if (!tmpl_s || !tmpl_i) continue;
        enter_spec_ctx(g, tmpl_s, m);
        for (int j = 0; j < tmpl_i->nchildren; j++)
            if (tmpl_i->children[j]->kind == ND_FUNC_DECL)
                emit_func_body(g, tmpl_i->children[j]);
        exit_spec_ctx(g);
    }

    if (out) fputs(g->out.data, out);
    sb_free(&g->out);
    arena_free_all(&cg_arena);
}

/* Generate C code into a StrBuf (zero disk I/O).
 * Caller owns the StrBuf and must sb_free() it when done. */
void cgen_buf(Node* prog, SymTable* st, StrBuf* sb) {
    G gctx = {0}; G* g = &gctx;
    sb_init(&g->out);
    g->st    = st;

    Arena* cg_arena = NULL;
    g->arena = &cg_arena;

    sb_puts(&g->out,
        "/* Generated by sharpc. Do not edit by hand. */\n"
        "\n");

    for (int i = 0; i < st->nexterns; i++) {
        SymExtern* e = &st->externs[i];
        sb_puts(&g->out, "extern ");
        emit_type(g, e->ret_type);
        sb_printf(&g->out, " %s(", e->name);
        if (e->decl->nparams == 0) {
            sb_puts(&g->out, "void");
        } else {
            for (int j = 0; j < e->decl->nparams; j++) {
                Node* pr = e->decl->params[j];
                if (j) sb_puts(&g->out, ", ");
                emit_type_with_name(g, pr->declared_type, pr->name);
            }
        }
        sb_puts(&g->out, ");\n");
    }

    /* Extern variable declarations: `extern Type name;` */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind != ND_EXTERN_VAR) continue;
        sb_puts(&g->out, "extern ");
        emit_type(g, d->declared_type);
        sb_printf(&g->out, " %s;\n", d->name);
    }

    if (st->nexterns > 0) sb_putc(&g->out, '\n');

    /* ------- Forward declarations ------- */
    /* Union forward declarations first (anonymous unions may be referenced by typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_UNION_DECL)
            emit_union_decl(g, d);
        if (d->kind == ND_UNION_FWD)
            emit_union_fwd_decl(g, d);
    }
    /* Struct forward declarations (anonymous structs may be referenced by typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STRUCT_DECL && !is_generic_template(d))
            emit_struct_fwd(g, d);
        if (d->kind == ND_STRUCT_FWD)
            emit_struct_fwd_decl(g, d);
    }
    /* Enum definitions (full definitions must come before typedefs that reference them,
     * as enums cannot be forward-declared in C11). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_ENUM_DECL && d->raw_text) {
            sb_printf(&g->out, "enum %s {\n", d->name);
            sb_puts(&g->out, d->raw_text);
            sb_puts(&g->out, "\n};\n\n");
        }
    }
    /* Typedef declarations (including array typedefs). */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind != ND_TYPEDEF_DECL) continue;
        Type* base = d->declared_type;
        while (base && (base->kind == TY_ARRAY || base->kind == TY_PTR))
            base = base->base;
        if (base && base->kind == TY_NAMED &&
            strncmp(base->name, "__anon_enum_", 12) == 0) {
            sb_printf(&g->out, "typedef enum %s %s;\n", base->name, d->name);
        } else {
            emit_typedef_decl(g, d);
        }
    }
    /* _Static_assert pass-through */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STATIC_ASSERT && d->raw_text)
            sb_printf(&g->out, "%s\n", d->raw_text);
    }
    /* Specialised structs: one per SymMono. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        sb_printf(&g->out, "typedef struct %s %s;\n", m->mangled, m->mangled);
    }
    sb_putc(&g->out, '\n');

    /* ------- Function / method forward declarations ------- */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_FUNC_DECL) {
            emit_func_decl(g, d);
        } else if (d->kind == ND_IMPL && !is_generic_template(d)) {
            for (int j = 0; j < d->nchildren; j++)
                if (d->children[j]->kind == ND_FUNC_DECL)
                    emit_func_decl(g, d->children[j]);
        }
    }
    /* Specialised method forward decls: for each mono, look up its impl
     * template and emit one forward decl per method. */
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        Node* tmpl_i = find_generic_impl(prog, m->generic_name);
        if (!tmpl_s || !tmpl_i) continue;
        enter_spec_ctx(g, tmpl_s, m);
        for (int j = 0; j < tmpl_i->nchildren; j++)
            if (tmpl_i->children[j]->kind == ND_FUNC_DECL)
                emit_func_decl(g, tmpl_i->children[j]);
        exit_spec_ctx(g);
    }
    sb_putc(&g->out, '\n');

    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_STRUCT_DECL && !is_generic_template(d))
            emit_struct_body(g, d);
    }
    /* Union bodies. */
    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_UNION_DECL)
            emit_union_body(g, d);
    }
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        if (!tmpl_s) continue;
        enter_spec_ctx(g, tmpl_s, m);
        sb_printf(&g->out, "struct %s {\n", m->mangled);
        if (tmpl_s->nfields == 0) sb_puts(&g->out, "    char _sharp_unit_;\n");
        for (int k = 0; k < tmpl_s->nfields; k++) {
            Node* f = tmpl_s->fields[k];
            sb_puts(&g->out, "    ");
            emit_type(g, f->declared_type);
            sb_printf(&g->out, " %s;\n", f->name);
        }
        sb_puts(&g->out, "};\n\n");
        exit_spec_ctx(g);
    }

    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_FUNC_DECL) {
            emit_func_body(g, d);
        } else if (d->kind == ND_IMPL && !is_generic_template(d)) {
            for (int j = 0; j < d->nchildren; j++)
                if (d->children[j]->kind == ND_FUNC_DECL)
                    emit_func_body(g, d->children[j]);
        }
    }
    for (int i = 0; i < st->nmonos; i++) {
        SymMono* m = &st->monos[i];
        Node* tmpl_s = find_generic_struct_decl(prog, m->generic_name);
        Node* tmpl_i = find_generic_impl(prog, m->generic_name);
        if (!tmpl_s || !tmpl_i) continue;
        enter_spec_ctx(g, tmpl_s, m);
        for (int j = 0; j < tmpl_i->nchildren; j++)
            if (tmpl_i->children[j]->kind == ND_FUNC_DECL)
                emit_func_body(g, tmpl_i->children[j]);
        exit_spec_ctx(g);
    }

    /* Transfer ownership to caller — NO disk I/O */
    *sb = g->out;
    g->out = (StrBuf){0};
    arena_free_all(&cg_arena);
}
