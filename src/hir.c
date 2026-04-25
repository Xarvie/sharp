/* hir.c — HIR: Control Flow Graph construction and analysis (Leap D).
 *
 * Translates the lowered AST (output of lower.c) into a directed graph
 * of Basic Blocks.  Each function produces one HirFunc whose blocks[]
 * array forms the CFG; blocks[0] is always the entry.
 *
 * Builder invariant:
 *   build_stmt(ctx, s, cur) consumes AST node s from the block whose
 *   first instruction would be added to `cur`.  It returns the
 *   "continuation" block — where code after s should be added — or NULL
 *   if every path through s terminates (every leaf is HIRI_RETURN or
 *   HIRI_JUMP to a loop-exit block).
 *
 * All allocations use malloc so hir_free() can release them completely.
 * The Arena** parameter is used only for string duplication (labels).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "sharp.h"
#include "types.h"
#include "ast.h"
#include "hir.h"

/* ================================================================== *
 *   malloc-backed instruction / block / function vectors
 * ================================================================== */
#define VEC_PUSH(arr, len, cap, T, item)                             \
    do {                                                             \
        if ((len) == (cap)) {                                        \
            (cap) = (cap) ? (cap) * 2 : 8;                          \
            (arr) = (T*)realloc((arr), (size_t)(cap) * sizeof(T));  \
        }                                                            \
        (arr)[(len)++] = (item);                                     \
    } while (0)

/* ================================================================== *
 *   Builder context
 * ================================================================== */
typedef struct {
    Arena**      arena;        /* for string duplication only           */
    SymTable*    st;
    int          counter;      /* block id + label numbering            */
    /* Blocks accumulated for the function currently being built */
    HirBlock**   blk_arr;
    int          blk_len;
    int          blk_cap;
    /* All funcs accumulated for the program */
    HirFunc**    func_arr;
    int          func_len;
    int          func_cap;
    /* Loop stack: each entry records the exit and continue targets */
    struct { HirBlock* exit; HirBlock* cont; } loops[64];
    int          loop_depth;
} Bctx;

/* ── Instruction construction ───────────────────────────────────── */
static HirInstr* new_instr(HirIKind k, int line, Node* node) {
    HirInstr* i = (HirInstr*)calloc(1, sizeof(HirInstr));
    i->kind = k;
    i->line = line;
    i->node = node;
    return i;
}

/* ── Block construction ─────────────────────────────────────────── */
static HirBlock* new_block(Bctx* ctx, const char* fmt, ...) {
    char buf[128];
    va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof buf, fmt, ap); va_end(ap);
    HirBlock* b = (HirBlock*)calloc(1, sizeof(HirBlock));
    b->id    = ctx->counter++;
    b->label = arena_strndup(ctx->arena, buf, (int)strlen(buf));
    VEC_PUSH(ctx->blk_arr, ctx->blk_len, ctx->blk_cap, HirBlock*, b);
    return b;
}

/* Append a HIRI_STMT instruction to block b. */
static void block_add_stmt(HirBlock* b, Node* stmt) {
    HirInstr* i = new_instr(HIRI_STMT, stmt->line, stmt);
    VEC_PUSH(b->instrs, b->ninstrs, b->_instrs_cap, HirInstr*, i);
}

/* Register `from` as a predecessor of `to`. */
static void add_pred(HirBlock* to, HirBlock* from) {
    /* Avoid duplicates (can happen when an if has no else). */
    for (int i = 0; i < to->npreds; i++)
        if (to->preds[i] == from) return;
    VEC_PUSH(to->preds, to->npreds, to->_preds_cap, HirBlock*, from);
}

/* ── Terminator setters ─────────────────────────────────────────── */
static void set_jump(HirBlock* b, HirBlock* target, int line) {
    assert(b->ninstrs == 0 || hir_term(b)->kind == HIRI_STMT /* not yet set */
           || "block already terminated");
    HirInstr* t = new_instr(HIRI_JUMP, line, NULL);
    t->succ[0]  = target;
    VEC_PUSH(b->instrs, b->ninstrs, b->_instrs_cap, HirInstr*, t);
    add_pred(target, b);
}

static void set_branch(HirBlock* b, Node* cond, HirBlock* tgt, HirBlock* flt, int line) {
    HirInstr* t = new_instr(HIRI_BRANCH, line, cond);
    t->succ[0]  = tgt;
    t->succ[1]  = flt;
    VEC_PUSH(b->instrs, b->ninstrs, b->_instrs_cap, HirInstr*, t);
    add_pred(tgt, b);
    add_pred(flt, b);
}

static void set_return(HirBlock* b, Node* ret_node, int line) {
    HirInstr* t = new_instr(HIRI_RETURN, line, ret_node);
    VEC_PUSH(b->instrs, b->ninstrs, b->_instrs_cap, HirInstr*, t);
}

/* Ensure block b has a terminator.  If it doesn't, add a synthetic
 * fall-through return (node = NULL) so every block is well-formed. */
static void ensure_terminated(HirBlock* b, int line) {
    if (b->ninstrs > 0) {
        HirIKind k = hir_term(b)->kind;
        if (k == HIRI_JUMP || k == HIRI_BRANCH || k == HIRI_RETURN) return;
    }
    /* Synthetic fall-through: not an explicit return in the source. */
    set_return(b, NULL, line);
}

/* ================================================================== *
 *   CFG builder — forward declaration
 * ================================================================== */
static HirBlock* build_stmt (Bctx* ctx, Node* s,   HirBlock* cur);
static HirBlock* build_block(Bctx* ctx, Node* blk, HirBlock* cur);

/* ── build_stmt ─────────────────────────────────────────────────── */
static HirBlock* build_stmt(Bctx* ctx, Node* s, HirBlock* cur) {
    if (!s || !cur) return cur;

    switch (s->kind) {
        /* ── Block: recurse through children ───────────────── */
        case ND_BLOCK:
            return build_block(ctx, s, cur);

        /* ── Leaf statements: append as HIRI_STMT ────────────── */
        case ND_VARDECL:
        case ND_EXPR_STMT:
        case ND_DROP:
            block_add_stmt(cur, s);
            return cur;

        /* ── Return: terminator ───────────────────────────────── */
        case ND_RETURN:
            set_return(cur, s, s->line);
            return NULL;   /* block terminates; no continuation */

        /* ── Break / Continue: jump to loop target ────────────── */
        case ND_BREAK:
            if (ctx->loop_depth > 0) {
                set_jump(cur, ctx->loops[ctx->loop_depth - 1].exit, s->line);
            } else {
                /* Should not happen after sema; be defensive. */
                set_return(cur, NULL, s->line);
            }
            return NULL;

        case ND_CONTINUE:
            if (ctx->loop_depth > 0) {
                set_jump(cur, ctx->loops[ctx->loop_depth - 1].cont, s->line);
            } else {
                set_return(cur, NULL, s->line);
            }
            return NULL;

        /* ── If / Else: BRANCH terminator + merge block ──────── */
        case ND_IF: {
            int  n       = ctx->counter;   /* use next id as the label suffix */
            Node* cond   = if_cond  (s);
            Node* then_s = if_then  (s);
            Node* else_s = if_else  (s);

            HirBlock* then_b  = new_block(ctx, "if.then.%d", n);
            HirBlock* merge_b = new_block(ctx, "if.merge.%d", n);
            HirBlock* else_b  = else_s ? new_block(ctx, "if.else.%d", n) : merge_b;

            set_branch(cur, cond, then_b, else_b, s->line);

            /* Build then branch */
            HirBlock* then_end = build_stmt(ctx, then_s, then_b);
            if (then_end) set_jump(then_end, merge_b, s->line);

            /* Build else branch (if present) */
            if (else_s) {
                HirBlock* else_end = build_stmt(ctx, else_s, else_b);
                if (else_end) set_jump(else_end, merge_b, s->line);
            }
            /* merge_b may have no predecessors if both arms terminate —
             * it will be marked unreachable later. */
            return merge_b;
        }

        /* ── While: header / body / exit ─────────────────────── */
        case ND_WHILE: {
            int n = ctx->counter;
            HirBlock* header = new_block(ctx, "while.header.%d", n);
            HirBlock* body_b = new_block(ctx, "while.body.%d",   n);
            HirBlock* exit_b = new_block(ctx, "while.exit.%d",   n);

            set_jump(cur, header, s->line);
            set_branch(header, while_cond(s), body_b, exit_b, s->line);

            ctx->loops[ctx->loop_depth].exit = exit_b;
            ctx->loops[ctx->loop_depth].cont = header;
            ctx->loop_depth++;
            HirBlock* body_end = build_stmt(ctx, while_body(s), body_b);
            ctx->loop_depth--;

            if (body_end) set_jump(body_end, header, s->line);
            return exit_b;
        }

        /* ── For: init / header / body / continue / exit ──────── */
        case ND_FOR: {
            int   n      = ctx->counter;
            Node* init   = for_init  (s);
            Node* cond   = for_cond  (s);
            Node* update = for_update(s);
            Node* body   = for_body  (s);

            /* Emit init as a statement in the current block. */
            if (init) {
                if (init->kind == ND_VARDECL || init->kind == ND_EXPR_STMT) {
                    block_add_stmt(cur, init);
                } else {
                    cur = build_stmt(ctx, init, cur);
                    if (!cur) return NULL;
                }
            }

            HirBlock* header = new_block(ctx, "for.header.%d", n);
            HirBlock* body_b = new_block(ctx, "for.body.%d",   n);
            HirBlock* cont_b = new_block(ctx, "for.cont.%d",   n);
            HirBlock* exit_b = new_block(ctx, "for.exit.%d",   n);

            set_jump(cur, header, s->line);
            if (cond)
                set_branch(header, cond, body_b, exit_b, s->line);
            else
                set_jump(header, body_b, s->line);   /* infinite loop */

            ctx->loops[ctx->loop_depth].exit = exit_b;
            ctx->loops[ctx->loop_depth].cont = cont_b;
            ctx->loop_depth++;
            HirBlock* body_end = build_stmt(ctx, body, body_b);
            ctx->loop_depth--;

            if (body_end) set_jump(body_end, cont_b, s->line);

            /* Continue block: emit update expression then loop back. */
            if (update) {
                /* update is an expression node — wrap in a synthetic stmt */
                Node* upd_stmt = update;
                if (update->kind != ND_EXPR_STMT) {
                    /* Raw expression (e.g. i++): model as STMT with the
                     * expression node directly; cgen handles it fine. */
                    block_add_stmt(cont_b, update);
                } else {
                    block_add_stmt(cont_b, upd_stmt);
                }
            }
            set_jump(cont_b, header, s->line);
            return exit_b;
        }

        /* ── Anything else: treat as a leaf statement ──────────── */
        default:
            block_add_stmt(cur, s);
            return cur;
    }
}

/* Process all children of a ND_BLOCK node, threading the continuation
 * block through each statement. */
static HirBlock* build_block(Bctx* ctx, Node* blk, HirBlock* cur) {
    if (!blk || blk->kind != ND_BLOCK) return build_stmt(ctx, blk, cur);
    for (int i = 0; i < block_count(blk) && cur; i++)
        cur = build_stmt(ctx, block_stmt(blk, i), cur);
    return cur;
}

/* ================================================================== *
 *   Function CFG builder
 * ================================================================== */
static HirFunc* build_func(Bctx* ctx, Node* decl, const char* parent) {
    Node* body = func_body(decl);
    if (!body) return NULL;

    /* Reset per-function state */
    ctx->blk_len    = 0;
    ctx->blk_cap    = 0;
    ctx->blk_arr    = NULL;
    ctx->loop_depth = 0;
    ctx->counter    = 0;

    HirBlock* entry = new_block(ctx, "entry");
    HirBlock* last  = build_block(ctx, body, entry);

    /* Ensure every path has a terminator (handles implicit void returns
     * and prevents zero-instruction blocks). */
    int body_line = body ? body->line : (decl ? decl->line : 0);
    for (int i = 0; i < ctx->blk_len; i++) {
        HirBlock* b = ctx->blk_arr[i];
        if (b->ninstrs == 0) {
            /* Empty block — may result from dead merge_b with no preds.
             * Add a synthetic fall-through RETURN so it's well-formed. */
            set_return(b, NULL, body_line);
        } else {
            HirIKind k = hir_term(b)->kind;
            if (k != HIRI_JUMP && k != HIRI_BRANCH && k != HIRI_RETURN) {
                /* Last instruction is still a STMT — add fall-through. */
                set_return(b, NULL, body_line);
            }
        }
    }
    (void)last;

    /* Assemble function record */
    HirFunc* f        = (HirFunc*)calloc(1, sizeof(HirFunc));
    f->name           = decl->parent_type
        ? arena_strndup(ctx->arena,
                        func_name(decl), (int)strlen(func_name(decl)))
        : func_name(decl);
    f->parent_type    = parent;
    f->ret_type       = func_ret_type(decl);
    f->decl           = decl;
    f->nblocks        = ctx->blk_len;
    f->blocks         = (HirBlock**)malloc((size_t)ctx->blk_len * sizeof(HirBlock*));
    memcpy(f->blocks, ctx->blk_arr, (size_t)ctx->blk_len * sizeof(HirBlock*));
    free(ctx->blk_arr);
    ctx->blk_arr = NULL;
    return f;
}

/* ================================================================== *
 *   Public: hir_build
 * ================================================================== */
HirProg* hir_build(Node* program, SymTable* st, Arena** arena) {
    if (!program) return NULL;
    Bctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.arena = arena;
    ctx.st    = st;

    /* Walk every ND_FUNC_DECL (free functions and methods). */
    int n = program_ndecls(program);
    for (int i = 0; i < n; i++) {
        Node* d = program_decl(program, i);
        if (d->kind == ND_FUNC_DECL) {
            HirFunc* f = build_func(&ctx, d, NULL);
            if (f) VEC_PUSH(ctx.func_arr, ctx.func_len, ctx.func_cap, HirFunc*, f);
        } else if (d->kind == ND_IMPL) {
            int m = impl_nmethods(d);
            for (int j = 0; j < m; j++) {
                Node* meth = impl_method(d, j);
                if (meth->kind == ND_FUNC_DECL) {
                    HirFunc* f = build_func(&ctx, meth, impl_target_name(d));
                    if (f) VEC_PUSH(ctx.func_arr, ctx.func_len, ctx.func_cap, HirFunc*, f);
                }
            }
        }
    }

    HirProg* prog = (HirProg*)calloc(1, sizeof(HirProg));
    prog->nfuncs  = ctx.func_len;
    prog->funcs   = (HirFunc**)malloc((size_t)ctx.func_len * sizeof(HirFunc*));
    memcpy(prog->funcs, ctx.func_arr, (size_t)ctx.func_len * sizeof(HirFunc*));
    free(ctx.func_arr);
    return prog;
}

/* ================================================================== *
 *   Analysis: reachability
 * ================================================================== */
void hir_mark_reachable(HirProg* prog) {
    if (!prog) return;
    for (int fi = 0; fi < prog->nfuncs; fi++) {
        HirFunc* f = prog->funcs[fi];
        if (f->nblocks == 0) continue;
        /* BFS from entry. */
        HirBlock** queue = (HirBlock**)malloc((size_t)f->nblocks * sizeof(HirBlock*));
        int head = 0, tail = 0;
        /* Reset */
        for (int i = 0; i < f->nblocks; i++) f->blocks[i]->reachable = false;
        /* Enqueue entry */
        f->blocks[0]->reachable = true;
        queue[tail++] = f->blocks[0];
        while (head < tail) {
            HirBlock* b = queue[head++];
            HirInstr* t = hir_term(b);
            for (int s = 0; s < 2; s++) {
                HirBlock* succ = t->succ[s];
                if (!succ || succ->reachable) continue;
                succ->reachable = true;
                queue[tail++]   = succ;
            }
        }
        free(queue);
    }
}

/* ================================================================== *
 *   Analysis: guaranteed-return check
 * ================================================================== */
void hir_check_returns(HirProg* prog) {
    if (!prog) return;
    for (int fi = 0; fi < prog->nfuncs; fi++) {
        HirFunc* f = prog->funcs[fi];
        /* Only check non-void functions. */
        if (!f->ret_type || ty_is_void(f->ret_type)) continue;
        /* Look for any reachable block with a synthetic (fall-through)
         * return — node == NULL signals the function body ran off the end. */
        for (int bi = 0; bi < f->nblocks; bi++) {
            HirBlock* b = f->blocks[bi];
            if (!b->reachable) continue;
            HirInstr* t = hir_term(b);
            if (t->kind == HIRI_RETURN && t->node == NULL) {
                int line = f->decl ? f->decl->line : 0;
                if (f->parent_type) {
                    diag_emit(DIAG_ERROR, E_MISSING_RETURN, line, 0, 0,
                              "non-void method '%s::%s' may not return a "
                              "value on all execution paths",
                              f->parent_type, f->name);
                } else {
                    diag_emit(DIAG_ERROR, E_MISSING_RETURN, line, 0, 0,
                              "non-void function '%s' may not return a "
                              "value on all execution paths",
                              f->name);
                }
                break;   /* one diagnostic per function is enough */
            }
        }
    }
}

/* ================================================================== *
 *   Dump: human-readable CFG
 * ================================================================== */
static const char* nodekind_abbrev(NodeKind k) {
    switch (k) {
        case ND_VARDECL:   return "decl";
        case ND_DROP:      return "drop";
        case ND_EXPR_STMT: return "expr";
        case ND_ASSIGN:    return "assign";
        case ND_CALL:      return "call";
        case ND_RETURN:    return "return";
        default:           return "stmt";
    }
}

static void dump_instr(FILE* out, HirInstr* i) {
    if (i->kind == HIRI_STMT) {
        Node* n = i->node;
        switch (n->kind) {
            case ND_VARDECL:
                fprintf(out, "    decl   %s : %s\n",
                        vardecl_name(n), ty_render(vardecl_type(n)));
                break;
            case ND_DROP:
                fprintf(out, "    drop   %s  (%s___drop)\n",
                        drop_var_name(n), drop_struct_name(n));
                break;
            case ND_EXPR_STMT:
                fprintf(out, "    expr   (line %d)\n", n->line);
                break;
            default:
                fprintf(out, "    %-6s (line %d)\n",
                        nodekind_abbrev(n->kind), n->line);
                break;
        }
        return;
    }
    /* Terminator */
    switch (i->kind) {
        case HIRI_JUMP:
            fprintf(out, "    → JUMP   bb%d [%s]\n",
                    i->succ[0]->id, i->succ[0]->label);
            break;
        case HIRI_BRANCH:
            fprintf(out, "    → BRANCH (line %d)  T:bb%d [%s]  F:bb%d [%s]\n",
                    i->line,
                    i->succ[0]->id, i->succ[0]->label,
                    i->succ[1]->id, i->succ[1]->label);
            break;
        case HIRI_RETURN:
            if (i->node)
                fprintf(out, "    → RETURN (line %d)\n", i->line);
            else
                fprintf(out, "    → RETURN (fall-through)\n");
            break;
        default:
            fprintf(out, "    → ???\n");
            break;
    }
}

void hir_dump(HirProg* prog, FILE* out) {
    if (!prog) return;
    for (int fi = 0; fi < prog->nfuncs; fi++) {
        HirFunc* f = prog->funcs[fi];
        fprintf(out, "\n=== func ");
        if (f->parent_type) fprintf(out, "%s::", f->parent_type);
        fprintf(out, "%s", f->name);
        if (f->decl) {
            int np = func_nparams(f->decl);
            fputc('(', out);
            for (int i = 0; i < np; i++) {
                Node* p = func_param(f->decl, i);
                if (i) fputs(", ", out);
                fprintf(out, "%s: %s", param_name(p), ty_render(param_type(p)));
            }
            fputc(')', out);
        }
        fprintf(out, " → %s ===\n", f->ret_type ? ty_render(f->ret_type) : "void");

        for (int bi = 0; bi < f->nblocks; bi++) {
            HirBlock* b = f->blocks[bi];
            /* Block header */
            fprintf(out, "\n  bb%d [%s]", b->id, b->label);
            if (!b->reachable) fputs("  DEAD", out);
            if (b->npreds > 0) {
                fputs("  preds=[", out);
                for (int p = 0; p < b->npreds; p++) {
                    if (p) fputc(',', out);
                    fprintf(out, "bb%d", b->preds[p]->id);
                }
                fputc(']', out);
            } else {
                fputs("  preds=[]", out);
            }
            fputc('\n', out);
            /* Instructions */
            for (int i = 0; i < b->ninstrs; i++)
                dump_instr(out, b->instrs[i]);
        }
        fputc('\n', out);
    }
}

/* ================================================================== *
 *   Cleanup: hir_free
 * ================================================================== */
void hir_free(HirProg* prog) {
    if (!prog) return;
    for (int fi = 0; fi < prog->nfuncs; fi++) {
        HirFunc* f = prog->funcs[fi];
        if (!f) continue;
        for (int bi = 0; bi < f->nblocks; bi++) {
            HirBlock* b = f->blocks[bi];
            if (!b) continue;
            /* free label, instrs, preds, and individual instrs */
            /* label is arena-owned — do not free */
            for (int i = 0; i < b->ninstrs; i++) free(b->instrs[i]);
            free(b->instrs);
            free(b->preds);
            free(b);
        }
        free(f->blocks);
        free(f);
    }
    free(prog->funcs);
    free(prog);
}
