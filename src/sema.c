/* sema.c - minimal semantic analysis for Sharp (phase 2).
 *
 * Responsibilities:
 *   - Scan the program AST once and build a SymTable that lists every
 *     struct (with its fields) and every method (with its self-kind and
 *     parent struct).
 *   - Surface simple errors (duplicate struct, method on unknown struct,
 *     duplicate method).
 *   - Not responsible for type-checking or name-resolution inside function
 *     bodies; codegen handles that with a local-variable scope stack.
 */
#include "sharp.h"
#include "types.h"
#include "ast.h"

typedef struct { void* data; int len; int cap; size_t elemsz; } Vec;

static void vec_push(Vec* v, const void* elem) {
    if (v->len == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        void* nd = malloc((size_t)v->cap * v->elemsz);
        if (v->len) memcpy(nd, v->data, (size_t)v->len * v->elemsz);
        free(v->data);
        v->data = nd;
    }
    memcpy((char*)v->data + (size_t)v->len * v->elemsz, elem, v->elemsz);
    v->len++;
}

static void* vec_freeze(Arena** a, Vec* v, int* out_len) {
    *out_len = v->len;
    if (v->len == 0) { free(v->data); return NULL; }
    void* dst = arena_alloc(a, (size_t)v->len * v->elemsz);
    memcpy(dst, v->data, (size_t)v->len * v->elemsz);
    free(v->data);
    return dst;
}

/* ---------- struct collection ---------- */

static void add_struct(SymTable* st, Vec* sv, Arena** arena, Node* s) {
    /* check duplicate */
    for (int i = 0; i < sv->len; i++) {
        SymStruct* e = &((SymStruct*)sv->data)[i];
        if (strcmp(e->name, s->name) == 0) {
            error_at(s->line, "duplicate struct '%s'", s->name);
            return;
        }
    }
    SymStruct rec;
    rec.name    = s->name;
    rec.decl    = s;
    rec.fields  = ARENA_NEW_ARR(arena, SymField, s->nfields > 0 ? s->nfields : 1);
    rec.nfields = s->nfields;
    for (int i = 0; i < s->nfields; i++) {
        Node* f = s->fields[i];
        rec.fields[i].name = f->name;
        rec.fields[i].type = f->declared_type;
    }
    /* Phase 4: carry generic parameters over. */
    rec.type_params  = s->type_params;
    rec.ntype_params = s->ntype_params;
    vec_push(sv, &rec);
    (void)st;
}

/* ---------- phase 4: monomorphization collection ----------
 *
 * Walk the entire program AST after structs/methods are registered and
 * record every concrete generic instantiation we see — as types (fields,
 * params, return types, locals) or as instantiation expressions. We store
 * the unique set in SymTable.monos with a pre-computed mangled C name
 * (e.g. Stack<i32> -> "Stack_i32", Stack<i32*> -> "Stack_p_i32").
 *
 * Type equality and mangling now live in types.c — thanks to interning
 * `ty_eq` is identity, and `ty_mangle` caches its string. We just
 * compose a per-mono C name from the generic family name + per-targ
 * mangling. */

static const char* mangle_mono(Arena** arena, const char* gname, Type** targs, int ntargs) {
    StrBuf sb; sb_init(&sb);
    sb_puts(&sb, gname);
    for (int i = 0; i < ntargs; i++) { sb_putc(&sb, '_'); sb_puts(&sb, ty_mangle(targs[i])); }
    char* out = arena_strndup(arena, sb.data, (int)sb.len);
    sb_free(&sb);
    return out;
}

/* Register an instantiation if not already present. */
static void mono_add(Vec* mv, Arena** arena, const char* gname, Type** targs, int ntargs) {
    for (int i = 0; i < mv->len; i++) {
        SymMono* e = &((SymMono*)mv->data)[i];
        if (strcmp(e->generic_name, gname) != 0) continue;
        if (e->ntargs != ntargs) continue;
        bool all = true;
        for (int j = 0; j < ntargs; j++)
            if (!ty_eq(e->targs[j], targs[j])) { all = false; break; }
        if (all) return;
    }
    SymMono rec;
    rec.generic_name = gname;
    rec.ntargs       = ntargs;
    rec.targs        = ntargs ? ARENA_NEW_ARR(arena, Type*, ntargs) : NULL;
    for (int i = 0; i < ntargs; i++) rec.targs[i] = targs[i];
    rec.mangled = mangle_mono(arena, gname, targs, ntargs);
    vec_push(mv, &rec);
}

static void collect_from_type(Vec* mv, Arena** arena, Type* t) {
    if (!t) return;
    switch (t->kind) {
        case TY_PTR:
            collect_from_type(mv, arena, t->base);
            return;
        case TY_NAMED:
            for (int i = 0; i < t->ntargs; i++)
                collect_from_type(mv, arena, t->targs[i]);
            if (t->ntargs > 0)
                mono_add(mv, arena, t->name, t->targs, t->ntargs);
            return;
        default: return;
    }
}

static void collect_from_node(Vec* mv, Arena** arena, Node* n) {
    if (!n) return;
    if (n->type)          collect_from_type(mv, arena, n->type);
    if (n->declared_type) collect_from_type(mv, arena, n->declared_type);
    if (n->ret_type)      collect_from_type(mv, arena, n->ret_type);

    for (int i = 0; i < n->nchildren; i++) collect_from_node(mv, arena, n->children[i]);
    for (int i = 0; i < n->nfields;   i++) collect_from_node(mv, arena, n->fields[i]);
    for (int i = 0; i < n->nparams;   i++) collect_from_node(mv, arena, n->params[i]);
    for (int i = 0; i < n->nargs;     i++) collect_from_node(mv, arena, n->args[i]);
    collect_from_node(mv, arena, n->lhs);
    collect_from_node(mv, arena, n->rhs);
    collect_from_node(mv, arena, n->cond);
    collect_from_node(mv, arena, n->then_b);
    collect_from_node(mv, arena, n->else_b);
    collect_from_node(mv, arena, n->init);
    collect_from_node(mv, arena, n->update);
    collect_from_node(mv, arena, n->body);
    collect_from_node(mv, arena, n->callee);
}

/* ---------- method collection ---------- */

/* Name-prefix test for phase-3 operator methods. They all live under the
 * canonical "op_*" prefix so cgen can detect them at dispatch time. */
static bool is_operator_name(const char* nm) {
    return nm && nm[0] == 'o' && nm[1] == 'p' && nm[2] == '_';
}

/* Phase-3 extra checks on destructor and operator methods.
 * Returns true iff the method should still be registered. */
static bool validate_method_shape(Node* m) {
    bool ok = true;

    /* Destructor: name is the sentinel "~". Must be ref-self, return void,
     * and take no user parameters (parser already enforces nparams==1 via
     * synthesized self; we re-check defensively). */
    if (m->name && m->name[0] == '~' && m->name[1] == 0) {
        if (m->self_kind != SELF_REF) {
            error_at(m->line, "destructor self parameter must be a pointer (T* self)");
            ok = false;
        }
        if (m->ret_type && m->ret_type->kind != TY_VOID) {
            error_at(m->line, "destructor must return void");
            ok = false;
        }
        if (m->nparams != 1) {
            error_at(m->line, "destructor takes no parameters (got %d)",
                     m->nparams - 1);
            ok = false;
        }
        return ok;
    }

    /* Operator overload: static, 1 or 2 user params, first param must be the
     * parent struct type. */
    if (is_operator_name(m->name)) {
        if (m->self_kind != SELF_NONE) {
            error_at(m->line,
                     "operator overload '%s' must be declared 'static'",
                     m->name);
            ok = false;
        }
        if (m->nparams < 1 || m->nparams > 2) {
            error_at(m->line,
                     "operator overload '%s' must take 1 or 2 parameters (got %d)",
                     m->name, m->nparams);
            ok = false;
        }
        if (m->nparams >= 1 && m->params[0] && m->params[0]->declared_type) {
            Type* t0 = m->params[0]->declared_type;
            if (!(t0->kind == TY_NAMED && m->parent_type &&
                  strcmp(t0->name, m->parent_type) == 0)) {
                error_at(m->line,
                         "first parameter of operator '%s' must be of type '%s'",
                         m->name, m->parent_type);
                ok = false;
            }
        }
    }
    return ok;
}

static void add_method(SymTable* st, Vec* mv, Node* m) {
    if (!validate_method_shape(m)) return;

    /* check duplicate within same parent */
    for (int i = 0; i < mv->len; i++) {
        SymMethod* e = &((SymMethod*)mv->data)[i];
        if (strcmp(e->parent, m->parent_type) == 0 &&
            strcmp(e->name,   m->name) == 0) {
            if (m->name[0] == '~' && m->name[1] == 0) {
                error_at(m->line, "duplicate destructor for '%s'",
                         m->parent_type);
            } else {
                error_at(m->line, "duplicate method '%s::%s'",
                         m->parent_type, m->name);
            }
            return;
        }
    }
    SymMethod rec;
    rec.parent    = m->parent_type;
    rec.name      = m->name;
    rec.self_kind = m->self_kind;
    rec.decl      = m;
    vec_push(mv, &rec);
    (void)st;
}

/* ---------- public API ---------- */

/* Phase 8: forward decl so sema_build can invoke the type-checking pass. */
static void sema_typecheck(SymTable* st, Node* prog, Arena** arena);

SymTable* sema_build(Node* program, Arena** arena) {
    SymTable* st = ARENA_NEW(arena, SymTable);
    Vec sv = { NULL, 0, 0, sizeof(SymStruct) };
    Vec mv = { NULL, 0, 0, sizeof(SymMethod) };

    /* Pass 1: structs */
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind == ND_STRUCT_DECL) add_struct(st, &sv, arena, d);
    }

    /* Pass 2: impl blocks -> methods. Non-generic impls have their methods
     * registered directly in the method table. Generic-template methods are
     * also registered (so lookup works during mangling decisions) but their
     * bodies are only emitted once per monomorphisation in cgen. */
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_IMPL) continue;

        /* Verify parent struct exists */
        bool found = false;
        for (int j = 0; j < sv.len; j++) {
            if (strcmp(((SymStruct*)sv.data)[j].name, d->name) == 0) {
                found = true; break;
            }
        }
        if (!found)
            error_at(d->line, "impl for unknown struct '%s'", d->name);

        for (int k = 0; k < d->nchildren; k++) {
            Node* m = d->children[k];
            if (m->kind == ND_FUNC_DECL) add_method(st, &mv, m);
        }
    }

    /* Pass 3: phase-4 monomorphization collection. Walk the program AST
     * once gathering every concrete `Name<Args...>` reference. Templates
     * themselves (struct decls and impl blocks with type_params) contribute
     * no instantiations — their T is still abstract. */
    Vec mono = { NULL, 0, 0, sizeof(SymMono) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind == ND_STRUCT_DECL && d->ntype_params > 0) continue;
        if (d->kind == ND_IMPL        && d->ntype_params > 0) continue;
        collect_from_node(&mono, arena, d);
    }

    /* Pass 4: phase-7 extern declarations. */
    Vec ev = { NULL, 0, 0, sizeof(SymExtern) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_EXTERN_DECL) continue;
        SymExtern rec;
        rec.name     = d->name;
        rec.ret_type = d->ret_type;
        rec.decl     = d;
        vec_push(&ev, &rec);
    }

    /* Pass 4a: global const declarations. */
    Vec cv = { NULL, 0, 0, sizeof(SymConst) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_CONST_DECL) continue;
        SymConst rec;
        rec.name  = d->name;
        rec.type  = d->declared_type;
        rec.value = d->rhs;
        rec.decl  = d;
        vec_push(&cv, &rec);
    }

    /* Pass 4b: extern variable declarations. */
    Vec xv = { NULL, 0, 0, sizeof(SymValue) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_EXTERN_VAR) continue;
        SymValue rec;
        rec.name = d->name;
        rec.type = d->declared_type;
        rec.decl = d;
        vec_push(&xv, &rec);
    }

    /* Pass 4b: phase-9 typedef declarations. */
    Vec tv = { NULL, 0, 0, sizeof(SymTypedef) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_TYPEDEF_DECL) continue;
        /* Check for duplicate typedef name. */
        bool dup = false;
        for (int j = 0; j < tv.len; j++) {
            if (strcmp(((SymTypedef*)tv.data)[j].name, d->name) == 0) {
                diag_emit(DIAG_ERROR, E_DUP_METHOD, d->line, 0, 0,
                          "duplicate typedef '%s'", d->name);
                dup = true; break;
            }
        }
        if (dup) continue;
        SymTypedef rec;
        rec.name     = d->name;
        rec.base     = d->declared_type;
        rec.decl     = d;
        vec_push(&tv, &rec);
    }

    /* Pass 4c: register builtin C types that are not primitives.
     * `wchar_t` is a platform-dependent integer type; map it to `int`. */
    {
        SymTypedef rec;
        rec.name     = "wchar_t";
        rec.base     = type_prim(arena, TY_I32);
        rec.decl     = NULL;
        vec_push(&tv, &rec);
    }

    /* Pass 5: phase-8 — collect user free functions (non-method,
     * non-extern). Methods are in mv; externs are in ev. Walking
     * top-level ND_FUNC_DECL catches exactly the free functions. */
    Vec fv = { NULL, 0, 0, sizeof(SymFunc) };
    for (int i = 0; i < program->nchildren; i++) {
        Node* d = program->children[i];
        if (d->kind != ND_FUNC_DECL) continue;
        /* Check for duplicate free-function name. */
        bool dup = false;
        for (int j = 0; j < fv.len; j++) {
            if (strcmp(((SymFunc*)fv.data)[j].name, d->name) == 0) {
                diag_emit(DIAG_ERROR, E_DUP_METHOD, d->line, 0, 0,
                          "duplicate function '%s'", d->name);
                dup = true; break;
            }
        }
        if (dup) continue;
        SymFunc rec;
        rec.name     = d->name;
        rec.ret_type = d->ret_type;
        rec.decl     = d;
        vec_push(&fv, &rec);
    }

    st->structs  = (SymStruct*)vec_freeze(arena, &sv,   &st->nstructs);
    st->methods  = (SymMethod*)vec_freeze(arena, &mv,   &st->nmethods);
    st->monos    = (SymMono*)  vec_freeze(arena, &mono, &st->nmonos);
    st->externs  = (SymExtern*)vec_freeze(arena, &ev,   &st->nexterns);
    st->funcs    = (SymFunc*)  vec_freeze(arena, &fv,   &st->nfuncs);
    st->typedefs = (SymTypedef*)vec_freeze(arena, &tv,  &st->ntypedefs);
    st->consts   = (SymConst*) vec_freeze(arena, &cv,   &st->nconsts);
    st->values   = (SymValue*) vec_freeze(arena, &xv,   &st->nvalues);

    /* Pass 6: phase-8 — type-check every function body. Only runs if
     * the preceding passes didn't already error out; the AST may be
     * too malformed to type-check usefully after, say, an unknown-struct
     * error. */
    if (g_error_count == 0)
        sema_typecheck(st, program, arena);

    return st;
}

SymStruct* sema_find_struct(SymTable* st, const char* name) {
    if (!st || !name) return NULL;
    for (int i = 0; i < st->nstructs; i++)
        if (strcmp(st->structs[i].name, name) == 0) return &st->structs[i];
    return NULL;
}

SymMethod* sema_find_method(SymTable* st, const char* parent, const char* name) {
    if (!st || !parent || !name) return NULL;
    for (int i = 0; i < st->nmethods; i++) {
        SymMethod* m = &st->methods[i];
        if (strcmp(m->parent, parent) == 0 && strcmp(m->name, name) == 0)
            return m;
    }
    return NULL;
}

SymField* sema_find_field(SymStruct* s, const char* name) {
    if (!s || !name) return NULL;
    for (int i = 0; i < s->nfields; i++)
        if (strcmp(s->fields[i].name, name) == 0) return &s->fields[i];
    return NULL;
}

/* Phase-3 convenience: the destructor is stored as a method whose internal
 * name is the single character "~". Returns NULL if the struct has none. */
SymMethod* sema_find_dtor(SymTable* st, const char* struct_name) {
    return sema_find_method(st, struct_name, "~");
}

/* Phase-7: look up an extern function by name. */
SymExtern* sema_find_extern(SymTable* st, const char* name) {
    if (!st || !name) return NULL;
    for (int i = 0; i < st->nexterns; i++)
        if (strcmp(st->externs[i].name, name) == 0) return &st->externs[i];
    return NULL;
}

SymFunc* sema_find_func(SymTable* st, const char* name) {
    if (!st || !name) return NULL;
    for (int i = 0; i < st->nfuncs; i++)
        if (strcmp(st->funcs[i].name, name) == 0) return &st->funcs[i];
    return NULL;
}

/* =====================================================================
 *   Phase 8 — type checking
 *
 *   A single pass that walks every function body and verifies:
 *     - assignment / initializer type compatibility
 *     - binary operator operand types
 *     - function/method call arity and argument types
 *     - return value matches declared return type
 *     - if/while/for condition is bool
 *
 *   The checker reuses sema's knowledge (struct, method, func, extern,
 *   mono tables). Type equality is structural; primitives allow implicit
 *   widening within the same signedness class (i32 → i64, u8 → u32, etc.),
 *   which matches C's integer promotion rules for the subset we emit.
 * ===================================================================== */

/* Local scope for the checker. Separate from cgen's scope to keep the
 * phases decoupled. Scoped lookups chase the parent chain; shadowing is
 * allowed (inner binding wins). */
typedef struct TCScope {
    struct TCScope* parent;
    /* Parallel arrays keep the struct layout small. */
    const char**    names;
    Type**          types;
    int             len;
    int             cap;
} TCScope;

typedef struct {
    SymTable*    st;
    Arena**      arena;
    TCScope*     scope;
    /* current function being checked — used for return-type validation
     * and `?` context check. NULL at top level. */
    Type*        cur_ret_type;
    /* type-parameter substitution env (non-empty while we are conceptually
     * checking a generic template body). When names[i] matches, types[i]
     * is substituted. Currently we don't descend into template bodies
     * (they are checked generically by treating type-parameters as opaque
     * named types), so this stays empty — included for future hooks. */
    /* Active struct for methods (implicit-self field access). */
    SymStruct*   self_struct;
    int          self_kind;   /* SELF_NONE / SELF_VALUE / SELF_REF */
    /* Typedef resolution cache: resolved types for named types we've seen. */
    const char** td_names;
    Type**       td_resolved;
    int          td_len;
    int          td_cap;
} TC;

/* Resolve typedefs in a type (follows the chain up to 16 levels).
 * Uses a simple cache in tc->td_* to avoid repeated lookups. */
static Type* tc_resolve(TC* tc, Type* t) {
    if (!t || !tc) return t;
    if (t->kind != TY_NAMED) return t;
    /* Check cache first */
    for (int i = 0; i < tc->td_len; i++) {
        if (tc->td_names[i] == t->name) return tc->td_resolved[i];
    }
    /* Resolve by following the chain */
    Type* resolved = sema_resolve_type(tc->st, t);
    /* Cache the result */
    if (tc->td_len >= tc->td_cap) {
        tc->td_cap = tc->td_cap ? tc->td_cap * 2 : 16;
        tc->td_names = (const char**)realloc(tc->td_names, tc->td_cap * sizeof(const char*));
        tc->td_resolved = (Type**)realloc(tc->td_resolved, tc->td_cap * sizeof(Type*));
    }
    tc->td_names[tc->td_len] = t->name;
    tc->td_resolved[tc->td_len] = resolved;
    tc->td_len++;
    return resolved;
}

static void tc_scope_push(TC* tc) {
    TCScope* s = (TCScope*)calloc(1, sizeof(TCScope));
    s->parent = tc->scope;
    tc->scope = s;
}
static void tc_scope_pop(TC* tc) {
    TCScope* s = tc->scope;
    tc->scope = s->parent;
    free(s->names);
    free(s->types);
    free(s);
}
static void tc_scope_add(TC* tc, const char* name, Type* ty) {
    TCScope* s = tc->scope;
    if (s->len == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        s->names = (const char**)realloc(s->names, (size_t)s->cap * sizeof(char*));
        s->types = (Type**)      realloc(s->types, (size_t)s->cap * sizeof(Type*));
    }
    s->names[s->len] = name;
    s->types[s->len] = ty;
    s->len++;
}
/* Lookup that checks the current scope first (for duplicate detection). */
static Type* tc_scope_lookup_local(TC* tc, const char* name) {
    TCScope* s = tc->scope;
    if (!s) return NULL;
    for (int i = s->len - 1; i >= 0; i--)
        if (strcmp(s->names[i], name) == 0) return s->types[i];
    return NULL;
}
static Type* tc_scope_lookup(TC* tc, const char* name) {
    for (TCScope* s = tc->scope; s; s = s->parent)
        for (int i = s->len - 1; i >= 0; i--)
            if (strcmp(s->names[i], name) == 0) return s->types[i];
    /* Check global const declarations */
    for (int i = 0; i < tc->st->nconsts; i++) {
        if (strcmp(tc->st->consts[i].name, name) == 0)
            return tc->st->consts[i].type;
    }
    /* Check extern variable declarations */
    for (int i = 0; i < tc->st->nvalues; i++) {
        if (strcmp(tc->st->values[i].name, name) == 0)
            return tc->st->values[i].type;
    }
    return NULL;
}

/* ─────────────────────────────────────────────────────────────────────
 * Type utilities — all delegate to the central `types` module.
 *
 * These were previously local implementations; after the types module
 * gained interning + caching + rendering, every call reduces to either a
 * pointer comparison (ty_eq) or a cached lookup. We keep the old function
 * names as 1-line shims so the existing call sites below don't need to
 * change. They compile to zero-cost calls — modern compilers inline. */

static const char* type_str(TC* tc, Type* t)  { (void)tc; return ty_render(t); }
static bool is_int_kind    (TypeKind k) { return ty_is_integer        (ty_prim(k)); }
static bool is_float_kind  (TypeKind k) { return ty_is_float          (ty_prim(k)); }
static bool is_numeric_kind(TypeKind k) { return ty_is_numeric        (ty_prim(k)); }
static bool is_signed_int  (TypeKind k) { return ty_is_signed_integer(ty_prim(k)); }

static bool tc_type_eq(Type* a, Type* b)                 { return ty_eq(a, b); }
static bool tc_assignable(TC* tc, Type* to, Type* from) {
    Type* resolved_to   = tc_resolve(tc, to);
    Type* resolved_from = tc_resolve(tc, from);
    return ty_assignable(resolved_to, resolved_from);
}

static Type* tc_subst_type(TC* tc, Type* t,
                           const char** names, Type** types, int n) {
    (void)tc;    /* the types arena is independent of tc->arena */
    return ty_subst(t, names, types, n);
}

/* If `t` is an instantiation like `Stack<i32>`, look up its template
 * declaration in the symbol table and fill `*names`/`*types` for
 * substitution. Returns true if `t` is a known generic instantiation.
 * The arrays are arena-allocated and only valid for this pass. */
static bool tc_mk_subst(TC* tc, Type* t,
                        const char*** out_names, Type*** out_types, int* out_n) {
    *out_names = NULL; *out_types = NULL; *out_n = 0;
    if (!t || t->kind != TY_NAMED || t->ntargs == 0) return false;
    SymStruct* s = sema_find_struct(tc->st, t->name);
    if (!s || s->ntype_params == 0) return false;
    if (s->ntype_params != t->ntargs) return false;
    const char** names = ARENA_NEW_ARR(tc->arena, const char*, s->ntype_params);
    Type**       types = ARENA_NEW_ARR(tc->arena, Type*,        s->ntype_params);
    for (int i = 0; i < s->ntype_params; i++) {
        names[i] = s->type_params[i];
        types[i] = t->targs[i];
    }
    *out_names = names;
    *out_types = types;
    *out_n     = s->ntype_params;
    return true;
}

static Type* tc_expr(TC* tc, Node* e);
static void  tc_stmt(TC* tc, Node* s);

static Type* tc_ty(TC* tc, TypeKind k) { (void)tc; return ty_prim(k); }

/* Struct lookup that handles both plain names and generic instantiations
 * (for the purpose of field/method resolution, the template struct is
 * what sema knows about). */
static SymStruct* tc_struct_of(TC* tc, Type* t) {
    if (!t) return NULL;
    if (t->kind == TY_NAMED) return sema_find_struct(tc->st, t->name);
    if (t->kind == TY_PTR &&
        t->base && t->base->kind == TY_NAMED)
        return sema_find_struct(tc->st, t->base->name);
    return NULL;
}

/* Is `name` a known struct type (and not shadowed by a local)? */
static bool tc_ident_is_struct_type(TC* tc, Node* e) {
    if (!e || e->kind != ND_IDENT) return false;
    if (tc_scope_lookup(tc, e->name)) return false;
    if (tc->self_struct && sema_find_field(tc->self_struct, e->name)) return false;
    return sema_find_struct(tc->st, e->name) != NULL;
}

/* Built-in generic static dispatch.
 * Built-in generics hook (stub — reserved for future use). */
static Type* tc_builtin_static_call(TC* tc, const char* type_name,
                                    Type* recv_type, const char* method,
                                    Node** args, int nargs, int line) {
    (void)tc; (void)type_name; (void)recv_type; (void)method;
    (void)args; (void)nargs; (void)line;
    return NULL;
}

/* Check a call to a user function / method / extern. `ret_type` is the
 * declared return type; `params` / `nparams` is the declared parameter
 * list (excluding implicit self, if any — caller has already stripped it);
 * `args` / `nargs` is what the user passed.
 *
 * The optional subst env (names[0..n) → types[0..n)) is applied to every
 * declared parameter type before comparison, so a method `void push(T v)`
 * on `Stack<i32>` is checked with `T` substituted as `i32`. */
static void tc_check_args(TC* tc, const char* what, int line,
                          Node** params, int nparams,
                          Node** args, int nargs,
                          const char** sn, Type** st, int sc,
                          bool is_variadic) {
    /* Variadic functions accept >= nparams args */
    if (!is_variadic && nargs != nparams) {
        diag_emit(DIAG_ERROR, E_ARG_COUNT, line, 0, 0,
                  "%s expects %d argument%s, got %d",
                  what, nparams, nparams == 1 ? "" : "s", nargs);
        return;
    }
    if (is_variadic && nargs < nparams) {
        diag_emit(DIAG_ERROR, E_ARG_COUNT, line, 0, 0,
                  "%s expects at least %d argument%s, got %d",
                  what, nparams, nparams == 1 ? "" : "s", nargs);
        return;
    }
    int checked = is_variadic ? nparams : nargs;
    for (int i = 0; i < checked; i++) {
        Type* at = tc_expr(tc, args[i]);
        Type* pt = i < nparams ? (params[i] ? params[i]->declared_type : NULL) : NULL;
        if (pt && sc > 0) pt = tc_subst_type(tc, pt, sn, st, sc);
        if (at && pt && !tc_assignable(tc, pt, at)) {
            diag_emit(DIAG_ERROR, E_ARG_TYPE, args[i]->line, 0, 0,
                      "%s: argument %d has type '%s', expected '%s'",
                      what, i + 1, type_str(tc, at), type_str(tc, pt));
        }
    }
}

/* Type-check an expression node and return its inferred Type, or NULL
 * if the type cannot be determined (further diagnostics may follow but
 * we should not cascade errors from a single failure).
 *
 * After the ast.h migration, every case that reads node fields does so
 * via a typed accessor. The accessors assert on wrong kind, so a
 * regression where we dispatch the wrong kind shows up as an assertion
 * instead of silently reading the wrong field. */
static Type* tc_expr(TC* tc, Node* e) {
    if (!e) return NULL;
    switch (e->kind) {
        case ND_INT:   return tc_ty(tc, TY_I32);
        case ND_FLOAT: return tc_ty(tc, TY_F64);
        case ND_BOOL:  return tc_ty(tc, TY_BOOL);
        case ND_CHAR:  return tc_ty(tc, TY_U8);
        case ND_STR:   return ty_ptr(ty_const(ty_prim(TY_U8)));
        case ND_NULL:  return ty_ptr(tc_ty(tc, TY_VOID));

        case ND_IDENT: {
            /* Generic instantiation carried as e->type (from speculation).
             * The parser annotates `Stack<i32>` ident with its Type*. */
            if (ast_type(e)) return ast_type(e);
            const char* name = ident_name(e);
            Type* t = tc_scope_lookup(tc, name);
            if (t) return tc_resolve(tc, t);
            /* Implicit self field. */
            if (tc->self_struct) {
                SymField* f = sema_find_field(tc->self_struct, name);
                if (f) return f->type;
            }
            /* Struct name used as a value — allowed only in T.foo(...) form,
             * which is handled at call dispatch. A bare name that resolves
             * to a struct type here is not an error yet; higher contexts
             * (call, member) interpret it as a type reference. */
            if (sema_find_struct(tc->st, name)) return NULL;
            diag_emit(DIAG_ERROR, E_UNKNOWN_IDENT, e->line, 0, 0,
                      "undeclared identifier '%s'", name);
            return NULL;
        }

        case ND_MEMBER: {
            Node*       recv = member_receiver(e);
            const char* fld  = member_name    (e);
            Type*       lt   = tc_expr(tc, recv);
            if (!lt) return NULL;

            SymStruct* s = tc_struct_of(tc, lt);
            if (!s) {
                diag_emit(DIAG_ERROR, E_MEMBER_ON_NON_STRUCT, e->line, 0, 0,
                          "cannot access field '%s' on non-struct type '%s'",
                          fld, type_str(tc, lt));
                return NULL;
            }
            SymField* f = sema_find_field(s, fld);
            if (!f) {
                diag_emit(DIAG_ERROR, E_UNKNOWN_FIELD, e->line, 0, 0,
                          "struct '%s' has no field '%s'", s->name, fld);
                return NULL;
            }
            /* Apply the receiver's type-arg substitution to the field type
             * so that `stk.buf` on a `Stack<i32>` comes out as `i32*` rather
             * than the template's `T*`. */
            Type* recv_t = lt;
            if (ty_is_pointer_like(recv_t)) recv_t = ty_base(recv_t);
            const char** sn = NULL; Type** sty = NULL; int sc = 0;
            if (tc_mk_subst(tc, recv_t, &sn, &sty, &sc))
                return ty_subst(f->type, sn, sty, sc);
            return f->type;
        }

        case ND_STRUCT_LIT: {
            /* If the literal carries a generic type (from parser speculation
             * on `Point<i32> { … }` forms), trust that. */
            if (ast_type(e)) return ast_type(e);
            const char* sname = struct_lit_name(e);
            SymStruct*  s     = sema_find_struct(tc->st, sname);
            if (!s) {
                diag_emit(DIAG_ERROR, E_UNKNOWN_TYPE, e->line, 0, 0,
                          "unknown struct type '%s'", sname);
                return NULL;
            }
            /* Check each field initialiser. Positional must match order;
             * named must resolve to a real field. */
            int nfields = struct_lit_nfields(e);
            for (int i = 0; i < nfields; i++) {
                Node*       fi      = struct_lit_field(e, i);
                const char* fi_name = field_init_name (fi);
                Node*       fi_val  = field_init_value(fi);
                Type*       vt      = tc_expr(tc, fi_val);
                SymField*   target  = NULL;
                if (fi_name) {
                    target = sema_find_field(s, fi_name);
                    if (!target) {
                        diag_emit(DIAG_ERROR, E_UNKNOWN_FIELD, fi->line, 0, 0,
                                  "struct '%s' has no field '%s'",
                                  s->name, fi_name);
                        continue;
                    }
                } else {
                    if (i < s->nfields) target = &s->fields[i];
                    else {
                        diag_emit(DIAG_ERROR, E_ARG_COUNT, fi->line, 0, 0,
                                  "too many initialisers for struct '%s' "
                                  "(%d field%s)", s->name, s->nfields,
                                  s->nfields == 1 ? "" : "s");
                        continue;
                    }
                }
                if (vt && target->type && !tc_assignable(tc, target->type, vt)) {
                    diag_emit(DIAG_ERROR, E_TYPE_MISMATCH, fi->line, 0, 0,
                              "field '%s' of '%s' expects '%s', got '%s'",
                              target->name, s->name,
                              type_str(tc, target->type), type_str(tc, vt));
                }
            }
            return ty_named(s->name);
        }

        case ND_BINOP: {
            Node*  lhs = binop_lhs(e);
            Node*  rhs = binop_rhs(e);
            OpKind op  = binop_op (e);
            Type*  lt  = tc_expr(tc, lhs);
            Type*  rt  = tc_expr(tc, rhs);

            /* If the LHS is a struct, look for an operator overload and
             * let its signature drive the result type. */
            if (ty_is_named(lt)) {
                const char* mname = NULL;
                switch (op) {
                    case OP_ADD: mname = "op_add"; break;
                    case OP_SUB: mname = "op_sub"; break;
                    case OP_MUL: mname = "op_mul"; break;
                    case OP_DIV: mname = "op_div"; break;
                    case OP_MOD: mname = "op_mod"; break;
                    case OP_EQ:  mname = "op_eq";  break;
                    case OP_NEQ: mname = "op_neq"; break;
                    case OP_LT:  mname = "op_lt";  break;
                    case OP_GT:  mname = "op_gt";  break;
                    case OP_LE:  mname = "op_le";  break;
                    case OP_GE:  mname = "op_ge";  break;
                    default: break;
                }
                if (mname) {
                    SymMethod* m = sema_find_method(tc->st, ty_name(lt), mname);
                    if (m && m->decl && func_nparams(m->decl) == 2) {
                        Node* rparam = func_param(m->decl, 1);
                        Type* rparam_ty = param_type(rparam);
                        if (rt && rparam_ty && !ty_assignable(rparam_ty, rt)) {
                            diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE,
                                      e->line, 0, 0,
                                      "operator '%s' on '%s' expects right operand '%s', got '%s'",
                                      mname + 3, ty_name(lt),
                                      type_str(tc, rparam_ty), type_str(tc, rt));
                        }
                        return func_ret_type(m->decl);
                    }
                    /* Struct operand with no matching overload — hard error. */
                    diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                              "no operator '%s' defined on struct '%s'",
                              mname + 3, ty_name(lt));
                    return NULL;
                }
            }

            /* Comparison / logical ops: bool result regardless of operands. */
            switch (op) {
                case OP_EQ: case OP_NEQ: case OP_LT: case OP_GT:
                case OP_LE: case OP_GE:
                    if (lt && rt && !ty_assignable(lt, rt) && !ty_assignable(rt, lt)) {
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "comparison between incompatible types '%s' and '%s'",
                                  type_str(tc, lt), type_str(tc, rt));
                    }
                    return tc_ty(tc, TY_BOOL);
                case OP_AND: case OP_OR:
                    if (lt && !ty_is_bool(lt))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "logical operator requires bool, left operand is '%s'",
                                  type_str(tc, lt));
                    if (rt && !ty_is_bool(rt))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "logical operator requires bool, right operand is '%s'",
                                  type_str(tc, rt));
                    return tc_ty(tc, TY_BOOL);
                default: break;
            }

            /* Arithmetic / bitwise on primitives or pointers. */
            if (lt && rt) {
                bool lnum = ty_is_numeric(lt);
                bool rnum = ty_is_numeric(rt);
                bool lptr = ty_is_pointer_like(lt);
                bool rptr = ty_is_pointer_like(rt);
                /* Pointer arithmetic: ptr ± int, ptr - ptr (yields isize). */
                if (lptr && ty_is_integer(rt)) return lt;
                if (rptr && ty_is_integer(lt)) return rt;
                if (lptr && rptr && op == OP_SUB) return tc_ty(tc, TY_ISIZE);
                if (!lnum || !rnum) {
                    diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                              "arithmetic operator requires numeric operands, "
                              "got '%s' and '%s'",
                              type_str(tc, lt), type_str(tc, rt));
                    return NULL;
                }
                /* Pick the "wider" of the two for the result — matches C's
                 * usual arithmetic conversions for the subset we care about. */
                if (ty_is_float(lt) || ty_is_float(rt)) {
                    if (ty_kind(lt) == TY_F64 || ty_kind(rt) == TY_F64)
                        return tc_ty(tc, TY_F64);
                    return tc_ty(tc, TY_F32);
                }
                return lt;   /* approximation good enough for phase 8 */
            }
            return lt ? lt : rt;
        }

        case ND_UNOP: {
            Node*  operand = unop_operand(e);
            OpKind op      = unop_op     (e);
            Type*  ot      = tc_expr(tc, operand);
            if (!ot) return NULL;
            switch (op) {
                case OP_NOT:
                    /* `!x` on a struct dispatches to op_not. */
                    if (ty_is_named(ot)) {
                        SymMethod* m = sema_find_method(tc->st, ty_name(ot), "op_not");
                        if (m && m->decl && func_nparams(m->decl) == 1)
                            return func_ret_type(m->decl);
                    }
                    if (!ty_is_bool(ot))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "'!' requires bool, got '%s'", type_str(tc, ot));
                    return tc_ty(tc, TY_BOOL);
                case OP_NEG:
                case OP_POS:
                    /* Unary minus on a struct → op_sub(1-arity form). */
                    if (ty_is_named(ot)) {
                        SymMethod* m = sema_find_method(tc->st, ty_name(ot), "op_sub");
                        if (m && m->decl && func_nparams(m->decl) == 1)
                            return func_ret_type(m->decl);
                    }
                    if (!ty_is_numeric(ot))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "unary arithmetic requires numeric, got '%s'",
                                  type_str(tc, ot));
                    return ot;
                case OP_BNOT:
                    if (ty_is_named(ot)) {
                        SymMethod* m = sema_find_method(tc->st, ty_name(ot), "op_bnot");
                        if (m && m->decl && func_nparams(m->decl) == 1)
                            return func_ret_type(m->decl);
                    }
                    if (!ty_is_integer(ot))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "'~' requires integer, got '%s'", type_str(tc, ot));
                    return ot;
                case OP_ADDR:
                    return ty_ptr(ot);
                case OP_DEREF:
                    if (!ty_is_pointer_like(ot)) {
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "cannot dereference non-pointer type '%s'",
                                  type_str(tc, ot));
                        return NULL;
                    }
                    return ty_base(ot);
                case OP_POSTINC:
                case OP_POSTDEC:
                    if (!ty_is_integer(ot) && !ty_is_pointer_like(ot))
                        diag_emit(DIAG_ERROR, E_BIN_OPERAND_TYPE, e->line, 0, 0,
                                  "'++'/'--' requires integer or pointer, got '%s'",
                                  type_str(tc, ot));
                    return ot;
                default:
                    return ot;
            }
        }

        case ND_ASSIGN: {
            Node* lhs = assign_lhs(e);
            Node* rhs = assign_rhs(e);
            Type* lt  = tc_expr(tc, lhs);
            Type* rt  = tc_expr(tc, rhs);
            if (!lt) return rt;
            /* Operand-op-assign (+= etc.): we permit them implicitly as C
             * does and only type-check the final assignment. */
            if (rt && !ty_assignable(lt, rt)) {
                diag_emit(DIAG_ERROR, E_TYPE_MISMATCH, e->line, 0, 0,
                          "cannot assign '%s' to '%s'",
                          type_str(tc, rt), type_str(tc, lt));
            }
            return lt;
        }

        case ND_INDEX: {
            Node* recv = index_receiver(e);
            Node* idx  = index_index   (e);
            Type* lt   = tc_expr(tc, recv);
            Type* it   = tc_expr(tc, idx);
            if (it && !ty_is_integer(it)) {
                diag_emit(DIAG_ERROR, E_INDEX_TYPE, e->line, 0, 0,
                          "index must be integer, got '%s'", type_str(tc, it));
            }
            if (!lt) return NULL;
            if (ty_is_pointer_like(lt)) return ty_base(lt);
            diag_emit(DIAG_ERROR, E_INDEX_TYPE, e->line, 0, 0,
                      "type '%s' is not indexable (only raw pointers are)",
                      type_str(tc, lt));
            return NULL;
        }

        case ND_CAST: {
            /* (Type)expr — type-check operand, result type is the cast target */
            Type* et = tc_expr(tc, cast_expr(e));
            Type* ct = cast_type(e);
            if (!ct) return et;
            /* Attach the cast type to the node for downstream passes */
            set_ast_type(e, ct);
            return ct;
        }

        case ND_CALL: {
            Node* callee = call_callee(e);
            int   nargs  = call_nargs (e);
            Node** args  = call_args  (e);

            /* Static call: T.foo(...) or Name<T>.foo(...). */
            if (callee && callee->kind == ND_MEMBER) {
                Node*       recv  = member_receiver(callee);
                const char* mname = member_name    (callee);
                bool recv_is_generic = (recv && recv->kind == ND_IDENT &&
                                        ast_type(recv) && ty_is_named(ast_type(recv)) &&
                                        ty_ntargs(ast_type(recv)) > 0);
                if (recv_is_generic) {
                    Type* rty = ast_type(recv);
                    /* Built-in generics hook. */
                    Type* bi = tc_builtin_static_call(tc, ty_name(rty), rty, mname,
                                                     args, nargs, e->line);
                    if (bi) return bi;
                    /* User generic: look up on the template struct, then
                     * substitute the template's type-parameters with the
                     * concrete types carried by the receiver. */
                    SymMethod* m = sema_find_method(tc->st, ty_name(rty), mname);
                    if (m && m->self_kind == SELF_NONE) {
                        const char** sn = NULL; Type** sty = NULL; int sc = 0;
                        tc_mk_subst(tc, rty, &sn, &sty, &sc);
                        tc_check_args(tc, mname, e->line, func_params(m->decl), func_nparams(m->decl), args, nargs, sn, sty, sc, false);
                        return ty_subst(func_ret_type(m->decl), sn, sty, sc);
                    }
                    diag_emit(DIAG_ERROR, E_UNKNOWN_METHOD, e->line, 0, 0,
                              "no static method '%s' on '%s<T>'",
                              mname, ty_name(rty));
                    return NULL;
                }
                if (tc_ident_is_struct_type(tc, recv)) {
                    const char* tn = ident_name(recv);
                    SymMethod* m = sema_find_method(tc->st, tn, mname);
                    if (m && m->self_kind == SELF_NONE) {
                        tc_check_args(tc, mname, e->line, func_params(m->decl), func_nparams(m->decl), args, nargs, NULL, NULL, 0, false);
                        return func_ret_type(m->decl);
                    }
                    diag_emit(DIAG_ERROR, E_UNKNOWN_METHOD, e->line, 0, 0,
                              "no static method '%s' on '%s'", mname, tn);
                    return NULL;
                }
                /* Instance call: recv.method(args...). */
                Type* lt = tc_expr(tc, recv);
                SymStruct* s = tc_struct_of(tc, lt);
                if (!s) {
                    if (lt)
                        diag_emit(DIAG_ERROR, E_MEMBER_ON_NON_STRUCT, e->line, 0, 0,
                                  "method call on non-struct type '%s'",
                                  type_str(tc, lt));
                    return NULL;
                }
                SymMethod* m = sema_find_method(tc->st, s->name, mname);
                if (!m) {
                    diag_emit(DIAG_ERROR, E_UNKNOWN_METHOD, e->line, 0, 0,
                              "struct '%s' has no method '%s'", s->name, mname);
                    return NULL;
                }
                /* Build subst env from the receiver type (if generic) so
                 * parameter and return types with `T` in them resolve.
                 * For a ptr/ref receiver, walk down to its base for subst. */
                const char** sn = NULL; Type** sty = NULL; int sc = 0;
                Type* recv_t = lt;
                if (ty_is_pointer_like(recv_t)) recv_t = ty_base(recv_t);
                tc_mk_subst(tc, recv_t, &sn, &sty, &sc);
                /* Skip the implicit self param (params[0]). */
                tc_check_args(tc, mname, e->line, func_params(m->decl) + 1, func_nparams(m->decl) - 1, args, nargs, sn, sty, sc, false);
                return ty_subst(func_ret_type(m->decl), sn, sty, sc);
            }
            /* Ident call: implicit-self method, free function, or extern. */
            if (callee && callee->kind == ND_IDENT) {
                const char* cname = ident_name(callee);
                /* Implicit self inside an impl body. */
                if (tc->self_struct && !tc_scope_lookup(tc, cname)) {
                    SymMethod* m = sema_find_method(tc->st, tc->self_struct->name, cname);
                    if (m) {
                        Node** params = func_params (m->decl);
                        int    np     = func_nparams(m->decl);
                        if (m->self_kind != SELF_NONE) { params++; np--; }
                        tc_check_args(tc, cname, e->line, params, np, args, nargs, NULL, NULL, 0, false);
                        return func_ret_type(m->decl);
                    }
                }
                SymFunc* f = sema_find_func(tc->st, cname);
                if (f) {
                    tc_check_args(tc, f->name, e->line, func_params(f->decl), func_nparams(f->decl), args, nargs, NULL, NULL, 0, false);
                    return f->ret_type;
                }
                SymExtern* ex = sema_find_extern(tc->st, cname);
                if (ex) {
                    /* Extern params live in the ND_EXTERN_DECL node.
                     * Get the array base and arity via the ast.h accessors. */
                    int np = extern_nparams(ex->decl);
                    Node** params = (np > 0) ? &ex->decl->params[0] : NULL;
                    tc_check_args(tc, ex->name, e->line,
                                  params, np,
                                  args, nargs, NULL, NULL, 0,
                                  ex->decl->is_variadic);
                    return ex->ret_type;
                }
                /* Function not declared — this is a hard error. */
                diag_emit(DIAG_ERROR, E_UNKNOWN_IDENT, e->line, 0, 0,
                          "call to undeclared function '%s'", cname);
                return tc_ty(tc, TY_I32);
            }
            /* Generic indirect call — not yet supported. */
            diag_emit(DIAG_ERROR, E_NOT_CALLABLE, e->line, 0, 0,
                      "expression is not callable");
            return NULL;
        }

        case ND_PRINT:
        case ND_PRINTLN: {
            Node* val = (e->kind == ND_PRINT) ? print_value(e) : println_value(e);
            (void)tc_expr(tc, val);
            return tc_ty(tc, TY_VOID);
        }

        default:
            return NULL;
    }
}

/* Check a statement. Uses tc_expr for the expression pieces and validates
 * control-flow specific constraints (conditions must be bool, return must
 * match fn type). */
static void tc_stmt(TC* tc, Node* s) {
    if (!s) return;
    switch (s->kind) {
        case ND_BLOCK: {
            tc_scope_push(tc);
            for (int i = 0; i < block_count(s); i++)
                tc_stmt(tc, block_stmt(s, i));
            tc_scope_pop(tc);
            return;
        }

        case ND_VARDECL: {
            const char* name = vardecl_name(s);
            Type*       dt   = vardecl_type(s);
            Node*       init = vardecl_init(s);

            /* Shadowing within the *same* scope is an error. */
            if (tc_scope_lookup_local(tc, name)) {
                diag_emit(DIAG_ERROR, E_DUP_LOCAL, s->line, 0, 0,
                          "redefinition of local '%s' in the same scope", name);
            }
            Type* rdt = tc_resolve(tc, dt);
            if (init) {
                Type* rt = tc_expr(tc, init);
                if (rt && rdt && !tc_assignable(tc, rdt, rt)) {
                    diag_emit(DIAG_ERROR, E_TYPE_MISMATCH, s->line, 0, 0,
                              "cannot initialise '%s' (declared '%s') with '%s'",
                              name, type_str(tc, dt), type_str(tc, rt));
                }
            }
            tc_scope_add(tc, name, rdt);
            return;
        }

        case ND_EXPR_STMT:
            (void)tc_expr(tc, expr_stmt_expr(s));
            return;

        case ND_RETURN: {
            Type* want = tc->cur_ret_type;
            Node* val  = return_value(s);
            if (val) {
                Type* got = tc_expr(tc, val);
                if (want && ty_is_void(want)) {
                    diag_emit(DIAG_ERROR, E_RETURN_TYPE, s->line, 0, 0,
                              "return with value in void-returning function");
                    return;
                }
                if (got && want && !tc_assignable(tc, want, got)) {
                    diag_emit(DIAG_ERROR, E_RETURN_TYPE, s->line, 0, 0,
                              "return value has type '%s', function declared '%s'",
                              type_str(tc, got), type_str(tc, want));
                }
            } else {
                if (want && !ty_is_void(want)) {
                    diag_emit(DIAG_ERROR, E_RETURN_TYPE, s->line, 0, 0,
                              "bare 'return' in function declared '%s'",
                              type_str(tc, want));
                }
            }
            return;
        }

        case ND_IF: {
            Type* ct = tc_expr(tc, if_cond(s));
            if (ct && !ty_is_bool(ct)) {
                diag_emit(DIAG_ERROR, E_COND_NOT_BOOL, s->line, 0, 0,
                          "'if' condition must be bool, got '%s'",
                          type_str(tc, ct));
            }
            tc_stmt(tc, if_then(s));
            Node* els = if_else(s);
            if (els) tc_stmt(tc, els);
            return;
        }
        case ND_WHILE: {
            Type* ct = tc_expr(tc, while_cond(s));
            if (ct && !ty_is_bool(ct))
                diag_emit(DIAG_ERROR, E_COND_NOT_BOOL, s->line, 0, 0,
                          "'while' condition must be bool, got '%s'",
                          type_str(tc, ct));
            tc_stmt(tc, while_body(s));
            return;
        }
        case ND_FOR: {
            /* for-init lives in its own scope so body locals can shadow. */
            tc_scope_push(tc);
            Node* init = for_init(s);
            Node* cond = for_cond(s);
            Node* upd  = for_update(s);
            if (init) tc_stmt(tc, init);
            if (cond) {
                Type* ct = tc_expr(tc, cond);
                if (ct && !ty_is_bool(ct))
                    diag_emit(DIAG_ERROR, E_COND_NOT_BOOL, s->line, 0, 0,
                              "'for' condition must be bool, got '%s'",
                              type_str(tc, ct));
            }
            if (upd) (void)tc_expr(tc, upd);
            tc_stmt(tc, for_body(s));
            tc_scope_pop(tc);
            return;
        }

        case ND_BREAK:
        case ND_CONTINUE:
            return;

        default:
            return;
    }
}

/* Type-check a single function body. */
static void tc_check_func(TC* tc, Node* f) {
    if (!f || !f->body) return;
    /* Set self context. */
    SymStruct* saved_ss = tc->self_struct;
    int        saved_sk = tc->self_kind;
    Type*      saved_rt = tc->cur_ret_type;
    tc->self_struct = f->parent_type ? sema_find_struct(tc->st, f->parent_type) : NULL;
    tc->self_kind   = f->self_kind;
    tc->cur_ret_type = f->ret_type;

    tc_scope_push(tc);
    for (int i = 0; i < f->nparams; i++) {
        Node* pr = f->params[i];
        if (pr && pr->name)
            tc_scope_add(tc, pr->name, pr->declared_type);
    }
    tc_stmt(tc, f->body);
    tc_scope_pop(tc);

    tc->self_struct = saved_ss;
    tc->self_kind   = saved_sk;
    tc->cur_ret_type = saved_rt;
}

static void sema_typecheck(SymTable* st, Node* prog, Arena** arena) {
    TC tc = {0};
    tc.st = st;
    tc.arena = arena;

    for (int i = 0; i < prog->nchildren; i++) {
        Node* d = prog->children[i];
        if (d->kind == ND_FUNC_DECL) {
            tc_check_func(&tc, d);
        } else if (d->kind == ND_IMPL && d->ntype_params == 0) {
            /* Non-generic impl bodies are checked with their self context. */
            for (int j = 0; j < d->nchildren; j++)
                if (d->children[j]->kind == ND_FUNC_DECL)
                    tc_check_func(&tc, d->children[j]);
        }
        /* Generic impl bodies are checked generically — type-parameters
         * are opaque named types so field accesses resolve via the
         * template struct definition.  Full generic type-checking with
         * constraints is a phase 11 concern. */
    }
}

SymTypedef* sema_find_typedef(SymTable* st, const char* name) {
    if (!st || !name) return NULL;
    for (int i = 0; i < st->ntypedefs; i++)
        if (strcmp(st->typedefs[i].name, name) == 0)
            return &st->typedefs[i];
    return NULL;
}

Type* sema_resolve_type(SymTable* st, Type* t) {
    if (!t || !st) return t;
    /* Follow typedef chain up to 16 levels to prevent infinite recursion. */
    for (int depth = 0; depth < 16 && t && t->kind == TY_NAMED; depth++) {
        SymTypedef* td = sema_find_typedef(st, t->name);
        if (!td) break;
        t = td->base;
    }
    /* After typedef chain, if still a bare named type, try C primitive
     * mapping so `int` resolves to TY_I32, `long long` to TY_I64, etc.
     * This keeps parser/cgen unchanged (they emit the raw C spelling) while
     * giving the type-checker a numeric primitive to work with. */
    if (t && t->kind == TY_NAMED && t->name) {
        Type* resolved = ty_resolve_c_named(t->name);
        if (resolved) {
            /* Preserve const qualifier if the original had it. */
            if (t->is_const && !resolved->is_const)
                resolved = ty_const(resolved);
            return resolved;
        }
    }
    return t;
}

