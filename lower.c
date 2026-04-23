/* lower.c — AST lowering pass.
 *
 * Runs between sema and cgen. After this pass every construct needing
 * scope-sensitive handling is expressed through primitive AST nodes, and
 * cgen is a straightforward "C printer" with no semantic logic.
 *
 * Responsibilities:
 *   - Inject explicit ND_DROP nodes at every scope-exit path (natural
 *     fall-through, break, continue).
 *   - Wrap ND_RETURN statements in functions with live destructible
 *     locals as
 *       { RetType __ret_N = expr; drops…; return __ret_N; }
 *     so cgen's ND_RETURN is a 3-line printer.
 *
 * Scope kinds:
 *   L_SCOPE_FUNC      function body root
 *   L_SCOPE_LOOP_BODY for/while body scope
 *   L_SCOPE_BLOCK     ordinary { … }
 */
#include "sharp.h"
#include "types.h"
#include "ast.h"

/* ============================================================ *
 *   Scope stack
 * ============================================================ */
enum { L_SCOPE_BLOCK = 0, L_SCOPE_FUNC = 1, L_SCOPE_LOOP_BODY = 2 };

typedef struct LScopeEntry {
    const char* var_name;
    const char* dtor_name;  /* NULL → no RAII drop needed             */
    Type*       var_type;   /* for l_expr_type() type-resolution path  */
} LScopeEntry;

typedef struct LScope {
    struct LScope* parent;
    LScopeEntry*   data;
    int            len;
    int            cap;
    int            kind;
} LScope;

typedef struct {
    Arena**   arena;
    SymTable* st;
    LScope*   top;
    Type*     cur_ret_type;  /* return type of function being lowered */
    int       tmp_counter;   /* monotonically increasing temp counter */
} L;

static void lscope_push(L* l, int kind) {
    LScope* s = (LScope*)calloc(1, sizeof(LScope));
    s->parent = l->top;
    s->kind   = kind;
    l->top    = s;
}

static void lscope_pop(L* l) {
    LScope* s = l->top;
    l->top = s->parent;
    free(s->data);
    free(s);
}

static void lscope_add(LScope* s, const char* var, const char* dtor, Type* ty) {
    if (s->len == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 8;
        LScopeEntry* nd = (LScopeEntry*)malloc((size_t)s->cap * sizeof(LScopeEntry));
        if (s->len) memcpy(nd, s->data, (size_t)s->len * sizeof(LScopeEntry));
        free(s->data);
        s->data = nd;
    }
    s->data[s->len].var_name  = var;
    s->data[s->len].dtor_name = dtor;
    s->data[s->len].var_type  = ty;
    s->len++;
}

/* ============================================================ *
 *   Dynamic node array (arena-backed)
 * ============================================================ */
typedef struct { Node** data; int len; int cap; } NodeVec;

static void nv_push(NodeVec* v, Arena** a, Node* n) {
    if (v->len == v->cap) {
        v->cap = v->cap ? v->cap * 2 : 8;
        Node** nd = ARENA_NEW_ARR(a, Node*, v->cap);
        if (v->len) memcpy(nd, v->data, (size_t)v->len * sizeof(Node*));
        v->data = nd;
    }
    v->data[v->len++] = n;
}

/* ============================================================ *
 *   Dtor-name resolution (must mirror cgen exactly)
 * ============================================================ */
static const char* l_dtor_name_for(L* l, Type* t) {
    if (!t) return NULL;
    if (t->kind != TY_NAMED) return NULL;
    SymStruct* s = sema_find_struct(l->st, t->name);
    if (!s) return NULL;
    if (!sema_find_dtor(l->st, s->name)) return NULL;
    return s->name;
}

/* ============================================================ *
 *   Node constructors
 * ============================================================ */
static Node* mk_node(L* l, NodeKind k, int line) {
    Node* n = ARENA_NEW(l->arena, Node);
    n->kind = k;
    n->line = line;
    return n;
}

static Node* mk_drop(L* l, const char* var, const char* dtor, int line) {
    Node* n = mk_node(l, ND_DROP, line);
    n->name = var;
    n->sval = dtor;
    return n;
}

static Node* mk_block(L* l, Node** stmts, int cnt, int line) {
    Node* b = mk_node(l, ND_BLOCK, line);
    b->children  = stmts;
    b->nchildren = cnt;
    return b;
}

static Node* mk_block_nv(L* l, NodeVec* v, int line) {
    return mk_block(l, v->data, v->len, line);
}

static Node* mk_ident(L* l, const char* name, int line) {
    Node* n = mk_node(l, ND_IDENT, line);
    n->name = name;
    return n;
}

static Node* mk_member(L* l, Node* recv, const char* fld, int line) {
    Node* n = mk_node(l, ND_MEMBER, line);
    n->lhs  = recv;
    n->name = fld;
    return n;
}

static Node* mk_unop(L* l, OpKind op, Node* operand, int line) {
    Node* n = mk_node(l, ND_UNOP, line);
    n->op  = op;
    n->rhs = operand;
    return n;
}

static Node* mk_vardecl(L* l, const char* name, Type* ty, Node* init, int line) {
    Node* n = mk_node(l, ND_VARDECL, line);
    n->name          = name;
    n->declared_type = ty;
    n->rhs           = init;
    return n;
}

static Node* mk_if_node(L* l, Node* cond, Node* then_b, Node* else_b, int line) {
    Node* n = mk_node(l, ND_IF, line);
    n->cond   = cond;
    n->then_b = then_b;
    n->else_b = else_b;
    return n;
}

static Node* mk_return_node(L* l, Node* val, int line) {
    Node* n = mk_node(l, ND_RETURN, line);
    n->rhs = val;
    return n;
}

static Node* mk_call_named(L* l, const char* fn, Node** args, int nargs, int line) {
    Node* n   = mk_node(l, ND_CALL, line);
    n->callee = mk_ident(l, fn, line);
    n->nargs  = nargs;
    n->args   = nargs ? ARENA_NEW_ARR(l->arena, Node*, nargs) : NULL;
    for (int i = 0; i < nargs; i++) n->args[i] = args[i];
    return n;
}

/* ============================================================ *
 *   Unwind helpers
 * ============================================================ */
static Node** collect_unwind(L* l, int stop_kind, int line, int* out_n) {
    int total = 0;
    for (LScope* cur = l->top; cur; cur = cur->parent) {
        for (int i = 0; i < cur->len; i++)
            if (cur->data[i].dtor_name) total++;
        if (cur->kind == stop_kind) break;
    }
    if (total == 0) { *out_n = 0; return NULL; }
    Node** drops = ARENA_NEW_ARR(l->arena, Node*, total);
    int idx = 0;
    for (LScope* cur = l->top; cur; cur = cur->parent) {
        for (int i = cur->len - 1; i >= 0; i--)
            if (cur->data[i].dtor_name)
                drops[idx++] = mk_drop(l, cur->data[i].var_name,
                                       cur->data[i].dtor_name, line);
        if (cur->kind == stop_kind) break;
    }
    *out_n = total;
    return drops;
}

static Node** collect_block_drops(L* l, int line, int* out_n) {
    LScope* s = l->top;
    int total = 0;
    for (int i = 0; i < s->len; i++)
        if (s->data[i].dtor_name) total++;
    if (total == 0) { *out_n = 0; return NULL; }
    Node** drops = ARENA_NEW_ARR(l->arena, Node*, total);
    int idx = 0;
    for (int i = s->len - 1; i >= 0; i--)
        if (s->data[i].dtor_name)
            drops[idx++] = mk_drop(l, s->data[i].var_name,
                                   s->data[i].dtor_name, line);
    *out_n = total;
    return drops;
}

static Node* wrap_with_unwind(L* l, Node* stmt, int stop_kind) {
    int ndrops;
    Node** drops = collect_unwind(l, stop_kind, stmt->line, &ndrops);
    if (ndrops == 0) return stmt;
    Node** kids = ARENA_NEW_ARR(l->arena, Node*, ndrops + 1);
    for (int i = 0; i < ndrops; i++) kids[i] = drops[i];
    kids[ndrops] = stmt;
    return mk_block(l, kids, ndrops + 1, stmt->line);
}

/* ============================================================ *
 *   Minimal type resolver for lower.c
 *
 *   Handles the common cases: local variables, free-function calls,
 *   and member access on Result/struct types.  Returns NULL on failure;
 *   callers fall back to cur_ret_type for Result temporaries.
 * ============================================================ */
static Type* l_expr_type(L* l, Node* e) {
    if (!e) return NULL;
    if (e->type) return e->type;   /* parser/sema annotation */
    switch (e->kind) {
        case ND_INT:   return ty_prim(TY_I32);
        case ND_FLOAT: return ty_prim(TY_F64);
        case ND_BOOL:  return ty_prim(TY_BOOL);
        case ND_CHAR:  return ty_prim(TY_U8);
        case ND_STR:   return ty_ptr(ty_prim(TY_U8));
        case ND_NULL:  return ty_ptr(ty_prim(TY_VOID));
        case ND_IDENT: {
            const char* name = e->name;
            for (LScope* s = l->top; s; s = s->parent)
                for (int i = s->len - 1; i >= 0; i--)
                    if (strcmp(s->data[i].var_name, name) == 0)
                        return s->data[i].var_type;
            return NULL;
        }
        case ND_CALL: {
            Node* callee = e->callee;
            if (!callee) return NULL;
            if (callee->kind == ND_IDENT) {
                SymFunc*   f  = sema_find_func  (l->st, callee->name);
                if (f)  return f->ret_type;
                SymExtern* ex = sema_find_extern(l->st, callee->name);
                if (ex) return ex->ret_type;
            }
            if (callee->kind == ND_MEMBER) {
                Node* recv  = callee->lhs;
                const char* mn = callee->name;
                if (recv && recv->kind == ND_IDENT) {
                    SymMethod* m = sema_find_method(l->st, recv->name, mn);
                    if (m && m->decl) return func_ret_type(m->decl);
                    Type* rtype = l_expr_type(l, recv);
                    if (rtype && rtype->kind == TY_NAMED) {
                        m = sema_find_method(l->st, rtype->name, mn);
                        if (m && m->decl) return func_ret_type(m->decl);
                    }
                }
            }
            return NULL;
        }
        case ND_MEMBER: {
            Type* lt = l_expr_type(l, e->lhs);
            if (!lt || lt->kind != TY_NAMED) return NULL;
            SymStruct* s = sema_find_struct(l->st, lt->name);
            if (!s) return NULL;
            SymField*  f = sema_find_field (s, e->name);
            if (!f) return NULL;
            if (lt->ntargs > 0 && s->ntype_params > 0) {
                int sc = s->ntype_params < lt->ntargs ? s->ntype_params : lt->ntargs;
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
        default: return NULL;
    }
}

/* ============================================================ *
 *   Forward declarations
 * ============================================================ */
static Node* lower_stmt (L* l, Node* s);
static Node* lower_block(L* l, Node* blk, int block_kind);

/* ============================================================ *
 *   Step 2c: ND_RETURN with RAII drops
 *
 *   If any destructible locals are live up to the function root,
 *   rewrite  return expr  to:
 *     { RetType __ret_N = expr;  drops…;  return __ret_N; }
 *   Void returns use  {drops…; return;}  without the save step.
 * ============================================================ */
static Node* lower_return_with_unwind(L* l, Node* ret) {
    int ndrops;
    Node** drops = collect_unwind(l, L_SCOPE_FUNC, ret->line, &ndrops);
    if (ndrops == 0) return ret;

    Node* val = return_value(ret);

    if (!val || (l->cur_ret_type && ty_is_void(l->cur_ret_type))) {
        Node** kids = ARENA_NEW_ARR(l->arena, Node*, ndrops + 1);
        for (int i = 0; i < ndrops; i++) kids[i] = drops[i];
        kids[ndrops] = ret;
        return mk_block(l, kids, ndrops + 1, ret->line);
    }

    char nbuf[32];
    snprintf(nbuf, sizeof nbuf, "__ret_%d", l->tmp_counter++);
    const char* rname = arena_strndup(l->arena, nbuf, (int)strlen(nbuf));

    Node* save = mk_vardecl(l, rname, l->cur_ret_type, val, ret->line);
    Node* nret = mk_return_node(l, mk_ident(l, rname, ret->line), ret->line);

    Node** kids = ARENA_NEW_ARR(l->arena, Node*, ndrops + 2);
    kids[0] = save;
    for (int i = 0; i < ndrops; i++) kids[1 + i] = drops[i];
    kids[ndrops + 1] = nret;
    return mk_block(l, kids, ndrops + 2, ret->line);
}

/* ============================================================ *
 *   Terminator check
 * ============================================================ */
static bool is_terminator(const Node* n) {
    if (!n) return false;
    if (n->kind == ND_RETURN || n->kind == ND_BREAK || n->kind == ND_CONTINUE)
        return true;
    if (n->kind == ND_BLOCK && n->nchildren > 0) {
        const Node* last = n->children[n->nchildren - 1];
        if (last && (last->kind == ND_RETURN || last->kind == ND_BREAK
                     || last->kind == ND_CONTINUE))
            return true;
    }
    return false;
}

/* ============================================================ *
 *   lower_block  —  main workhorse
 *
 *   For each child in the block:
 *   Lowers the statement, then registers ND_VARDECL children in the
 *   active scope so that collect_unwind sees them for subsequent
 *   early exits (break/continue/return) inside this block.
 *   Appends natural-end drops unless the block terminates.
 * ============================================================ */
static Node* lower_block(L* l, Node* blk, int block_kind) {
    lscope_push(l, block_kind);
    int    n   = block_count(blk);
    NodeVec out = {0};

    for (int i = 0; i < n; i++) {
        Node* child = blk->children[i];
        Node* lo    = lower_stmt(l, child);
        nv_push(&out, l->arena, lo);
        if (child->kind == ND_VARDECL)
            lscope_add(l->top,
                       vardecl_name(child),
                       l_dtor_name_for(l, vardecl_type(child)),
                       vardecl_type(child));
    }

    bool terminated = out.len > 0 && is_terminator(out.data[out.len - 1]);
    if (!terminated) {
        int ndrops;
        Node** drops = collect_block_drops(l, blk->line, &ndrops);
        for (int i = 0; i < ndrops; i++)
            nv_push(&out, l->arena, drops[i]);
    }

    blk->children  = out.data;
    blk->nchildren = out.len;
    lscope_pop(l);
    return blk;
}

/* ============================================================ *
 *   lower_stmt
 * ============================================================ */
static Node* lower_stmt(L* l, Node* s) {
    if (!s) return s;
    switch (s->kind) {
        case ND_BLOCK:
            return lower_block(l, s, L_SCOPE_BLOCK);

        case ND_IF:
            if (s->then_b) s->then_b = lower_stmt(l, s->then_b);
            if (s->else_b) s->else_b = lower_stmt(l, s->else_b);
            return s;

        case ND_WHILE: {
            Node* body = while_body(s);
            if (body && body->kind == ND_BLOCK)
                s->body = lower_block(l, body, L_SCOPE_LOOP_BODY);
            else if (body) {
                lscope_push(l, L_SCOPE_LOOP_BODY);
                s->body = lower_stmt(l, body);
                lscope_pop(l);
            }
            return s;
        }

        case ND_FOR: {
            lscope_push(l, L_SCOPE_BLOCK);
            Node* init = for_init(s);
            if (init) {
                s->init = lower_stmt(l, init);
                if (init->kind == ND_VARDECL)
                    lscope_add(l->top,
                               vardecl_name(init),
                               l_dtor_name_for(l, vardecl_type(init)),
                               vardecl_type(init));
            }
            Node* body = for_body(s);
            if (body && body->kind == ND_BLOCK)
                s->body = lower_block(l, body, L_SCOPE_LOOP_BODY);
            else if (body) {
                lscope_push(l, L_SCOPE_LOOP_BODY);
                s->body = lower_stmt(l, body);
                lscope_pop(l);
            }
            lscope_pop(l);
            return s;
        }

        case ND_RETURN:
            return lower_return_with_unwind(l, s);

        case ND_BREAK:
        case ND_CONTINUE:
            return wrap_with_unwind(l, s, L_SCOPE_LOOP_BODY);

        case ND_VARDECL:
            return s;   /* registration is caller's job */

        default:
            return s;
    }
}

/* ============================================================ *
 *   Function and program entry points
 * ============================================================ */

/* Inline version of lower_block for function bodies — avoids pushing
 * a redundant SCOPE_BLOCK on top of the already-pushed SCOPE_FUNC. */
static void lower_func_body_inner(L* l, Node* body) {
    int     n   = block_count(body);
    NodeVec out = {0};

    for (int i = 0; i < n; i++) {
        Node* child = body->children[i];
        nv_push(&out, l->arena, lower_stmt(l, child));
        if (child->kind == ND_VARDECL)
            lscope_add(l->top,
                       vardecl_name(child),
                       l_dtor_name_for(l, vardecl_type(child)),
                       vardecl_type(child));
    }

    bool terminated = out.len > 0 && is_terminator(out.data[out.len - 1]);
    if (!terminated) {
        int ndrops;
        Node** drops = collect_block_drops(l, body->line, &ndrops);
        for (int i = 0; i < ndrops; i++)
            nv_push(&out, l->arena, drops[i]);
    }

    body->children  = out.data;
    body->nchildren = out.len;
}

static void lower_func_body(L* l, Node* func) {
    Node* body = func_body(func);
    if (!body || body->kind != ND_BLOCK) return;

    Type* saved_rt  = l->cur_ret_type;
    l->cur_ret_type = func_ret_type(func);

    /* Push SCOPE_FUNC and register params. Value parameters with a
     * destructor are dropped at function exit, same as value locals —
     * C semantics say the callee owns the copy. (Ref/ptr parameters
     * have no dtor so l_dtor_name_for returns NULL for them.) */
    lscope_push(l, L_SCOPE_FUNC);
    for (int i = 0; i < func_nparams(func); i++) {
        Node* p = func_param(func, i);
        lscope_add(l->top, param_name(p),
                   l_dtor_name_for(l, param_type(p)),
                   param_type(p));
    }

    lower_func_body_inner(l, body);

    lscope_pop(l);
    l->cur_ret_type = saved_rt;
}

void lower_program(Node* program, SymTable* st, Arena** arena) {
    if (!program) return;
    L l;
    memset(&l, 0, sizeof(l));
    l.arena = arena;
    l.st    = st;

    int n = program_ndecls(program);
    for (int i = 0; i < n; i++) {
        Node* d = program_decl(program, i);
        if (d->kind == ND_FUNC_DECL) {
            lower_func_body(&l, d);
        } else if (d->kind == ND_IMPL) {
            int m = impl_nmethods(d);
            for (int j = 0; j < m; j++) {
                Node* meth = impl_method(d, j);
                if (meth->kind == ND_FUNC_DECL)
                    lower_func_body(&l, meth);
            }
        }
    }
}
