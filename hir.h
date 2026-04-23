/* hir.h — High-Level IR: Control Flow Graph over the lowered AST.
 *
 * After lower.c every compound Sharp construct is expressed through
 * primitive AST nodes.  The HIR pass translates this flat, tree-shaped
 * representation into a directed graph of Basic Blocks, making control
 * flow first-class rather than implicit in tree nesting.
 *
 * Design choices
 * ──────────────
 * • Instruction payloads KEEP the original lowered AST sub-trees (no
 *   3-address decomposition yet).  cgen.c is therefore unaffected; the
 *   HIR is built in parallel with the existing code-generation pipeline.
 * • Every HirBlock ends with exactly one terminator (JUMP / BRANCH /
 *   RETURN).  Non-terminator instructions are plain statement stubs.
 * • Predecessor lists are populated by the builder so that analyses that
 *   walk predecessors (live-variable, use-before-init) have O(1) access.
 * • All allocations are malloc-based and freed by hir_free().  The Arena
 *   is only used for string duplication (block labels, mangled names).
 *
 * Leap D deliverables (this file)
 * ────────────────────────────────
 *   hir_build()             CFG construction from the lowered AST.
 *   hir_mark_reachable()    BFS reachability from each function's entry.
 *   hir_check_returns()     Guaranteed-return diagnostic for non-void fns.
 *   hir_dump()              Human-readable CFG dump (used with -dump-hir).
 *   hir_free()              Release all HIR allocations.
 *
 * Leap E will add: def/use sets, live-variable analysis, use-before-init
 * detection, flow-sensitive typestate analysis on Result<T>.
 */
#ifndef SHARP_HIR_H
#define SHARP_HIR_H

#include "sharp.h"   /* Node*, Type*, SymTable*, DiagCode */

/* ================================================================== *
 *   Instruction kinds
 * ================================================================== */
typedef enum {
    /* ── Non-terminator instructions ─────────────────────────────── */
    HIRI_STMT,     /* any side-effecting non-branch statement: VARDECL,
                    * EXPR_STMT, DROP, or similar.
                    * node → the lowered AST statement node.           */

    /* ── Terminator instructions (exactly one per HirBlock) ──────── */
    HIRI_JUMP,     /* unconditional branch.
                    * succ[0] → target block.                          */
    HIRI_BRANCH,   /* conditional branch.
                    * node    → condition expression (AST Node*).
                    * succ[0] → true  block.
                    * succ[1] → false block.                           */
    HIRI_RETURN,   /* function return.
                    * node    → ND_RETURN AST node.
                    *           NULL for a synthetic fall-through return
                    *           (function body ran off the end).        */
} HirIKind;

/* ================================================================== *
 *   Instruction
 * ================================================================== */
typedef struct HirBlock HirBlock;

typedef struct HirInstr {
    HirIKind   kind;
    int        line;
    Node*      node;        /* AST payload — see per-kind notes above   */
    HirBlock*  succ[2];     /* JUMP: succ[0]; BRANCH: succ[0], succ[1]  */
} HirInstr;

/* ================================================================== *
 *   Basic Block
 * ================================================================== */
struct HirBlock {
    int          id;
    const char*  label;

    /* Instruction sequence.
     * instrs[0 .. ninstrs-2]  are HIRI_STMT.
     * instrs[ninstrs-1]       is the terminator (JUMP/BRANCH/RETURN). */
    HirInstr**   instrs;
    int          ninstrs;

    /* Predecessor blocks — populated during CFG construction. */
    HirBlock**   preds;
    int          npreds;

    /* ── Analysis flags ─────────────────────────────────────────── */
    bool         reachable;   /* true after hir_mark_reachable()        */

    /* ── Internal construction fields — zeroed after build ─────── */
    int          _instrs_cap;
    int          _preds_cap;
};

/* Convenience: fetch the block's terminator. */
static inline HirInstr* hir_term  (const HirBlock* b)       { return b->instrs[b->ninstrs - 1]; }
static inline int        hir_nstmts(const HirBlock* b)       { return b->ninstrs - 1; }
static inline HirInstr*  hir_stmt  (const HirBlock* b, int i){ return b->instrs[i]; }

/* ================================================================== *
 *   Function CFG
 * ================================================================== */
typedef struct {
    const char*  name;          /* mangled C-level name                */
    const char*  parent_type;   /* NULL for free functions             */
    Type*        ret_type;
    Node*        decl;          /* original ND_FUNC_DECL               */

    /* blocks[0] is always the entry block. */
    HirBlock**   blocks;
    int          nblocks;
} HirFunc;

/* ================================================================== *
 *   Program (all function CFGs)
 * ================================================================== */
typedef struct {
    HirFunc**  funcs;
    int        nfuncs;
} HirProg;

/* ================================================================== *
 *   Public API
 * ================================================================== */

/* Build the CFG for every function in the lowered program.
 * Must be called AFTER lower_program().
 * `arena` is used only for string duplication (block labels). */
HirProg* hir_build         (Node* program, SymTable* st, Arena** arena);

/* Mark blocks reachable from each function's entry block (BFS).
 * Sets block->reachable; unreachable blocks remain false.
 * Must be called before hir_check_returns(). */
void     hir_mark_reachable(HirProg* prog);

/* For every non-void function, verify that all reachable execution paths
 * terminate with an explicit ND_RETURN node.  Reports E_MISSING_RETURN
 * for each function that may fall through to the end without returning.
 * Must be called after hir_mark_reachable(). */
void     hir_check_returns (HirProg* prog);

/* Dump a human-readable CFG to `out`.
 * Shows each block's predecessors, statement summaries, and terminator.
 * Unreachable blocks are annotated with "DEAD". */
void     hir_dump          (HirProg* prog, FILE* out);

/* Release all malloc allocations owned by the HirProg.
 * After this call prog and all HirFunc/HirBlock/HirInstr pointers
 * from this prog are invalid. */
void     hir_free          (HirProg* prog);

#endif /* SHARP_HIR_H */
