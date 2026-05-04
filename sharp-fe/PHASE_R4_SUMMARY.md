# Phase R4 — GCC builtins + statement-expressions

**Status:** complete and green on all gates.
**Goal:** implement the two remaining deferred GCC extensions that block
sqlite-amalgamation end-to-end and any codebase using `assert` without
`-DNDEBUG`: GCC atomic builtins (`__atomic_*`, `__sync_*`) and
GCC statement-expressions (`({...})`).  Also validates that
`__builtin_va_arg(ap, TYPE)` already worked (probe p70 added to corpus).

## Gate results

| Gate | Before R4 | After R4 |
|---|---|---|
| `make test` | 203 / 203 | **207 / 207** (probes 69 + integration 40 + import 25 + decls 73) |
| `make asan` | clean | **clean** |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | **clean** |
| `c_superset_probe.sh` | 66 / 67 | **69 / 70** (p70, p71, p72 added; only deferred p21_kr_func still fails) |

`test_decls` gained 4 cases under `test_r4_gcc_builtins`; all are
exercised end-to-end (sharpc → cc → run) on every `make test` invocation.

## Three features delivered

### Feature 1 — `__atomic_*` and `__sync_*` builtins (probe p72)

**Root cause:** `sema.c`'s AST_IDENT path accepted names starting with
`__builtin_` as implicitly-typed callable externals (type = int, resolved
by cc).  Names starting with `__atomic_` (C11 atomic builtins like
`__atomic_load_n`, `__atomic_store_n`, `__atomic_compare_exchange_n`) and
`__sync_` (legacy GCC sync builtins) did not match this check and were
rejected with "undefined name 'xxx'".

**Fix (sema.c, 2 lines):**

```c
/* before */
if (strncmp(name, "__builtin_", 10) == 0) { t = ty_int(ts); break; }

/* after — Phase R4 */
if (strncmp(name, "__builtin_", 10) == 0 ||
    strncmp(name, "__atomic_",   9) == 0 ||
    strncmp(name, "__sync_",     7) == 0) { t = ty_int(ts); break; }
```

The fix is symmetric with the `__builtin_*` pattern.  All three families
share the same property: never declared explicitly in C source, always
resolved by cc.  The memory-order constants (`__ATOMIC_SEQ_CST` etc.) are
predefined by `sharp-cpp`'s `cpp_targets.c` as integer macros, so they
expand to integer literals before sema ever sees them.

Locked in by **`p72_atomic.c`** and a `test_r4_*` regression.

### Feature 2 — GCC statement-expression `({...})` (probe p71)

**Root cause:** `parse_primary`'s `STOK_LPAREN` case first probed for a
type (cast or compound literal) and then fell through to `parse_expr`.
Neither path accepted `{` as the second token — `{` cannot start an
expression and cannot start a type — so any `({ ... })` was rejected with
"unexpected token '{'".

**Why it matters:** glibc's `assert()` expansion uses a statement-
expression.  Any C code compiled with `assert` and without `-DNDEBUG` hits
this path.  Type-safe `MAX`/`MIN` macros (ubiquitous in production C code)
use the same idiom to prevent double-evaluation.

**Implementation — four layers:**

#### parse.c

At the very top of the `STOK_LPAREN` case in `parse_primary`, before the
`is_type_start` heuristic, check whether the next token is `{`:

```c
/* GNU statement-expression: ({ stmts; expr; }) */
if (ps_at(ps, STOK_LBRACE)) {
    AstNode *block = parse_block(ps);
    ps_expect(ps, STOK_RPAREN, "closing ')' of statement-expression");
    AstNode *n = ast_node_new(AST_STMT_EXPR, t.loc);
    n->u.stmt_expr.block = block;
    return parse_postfix(ps, n);
}
```

`{` cannot start a type, so the check is unambiguous.  The existing
compound-literal path `(Type){...}` is untouched: it requires closing `)` 
before `{`, so there is zero overlap with the new `({` form.

#### ast.h / ast.c

New kind `AST_STMT_EXPR` with a single `AstNode *block` member:

```c
struct { AstNode *block; } stmt_expr;
```

Three standard arms added: `ast_node_free` (recurse into block),
`ast_kind_name` table ("STMT_EXPR"), `ast_print` (one line + child).

#### sema.c — lazy scope creation

`scope_build` only walks statement-level trees; it never descends into
expression sub-trees.  A stmt-expr block appears at expression level (e.g.
as the RHS of an assignment), so its contents were never registered in the
scope tree.  This means `scope_lookup` would fail on any name declared
inside the block.

Fix: in `sema_expr` for `AST_STMT_EXPR`, build the block's scope on
demand if not already present (idempotent via `block->type_ref` guard):

```c
case AST_STMT_EXPR: {
    AstNode *block = expr->u.stmt_expr.block;
    t = ty_void(ts);
    if (!block) break;

    Scope *blk_scope = block->type_ref
                     ? (Scope *)block->type_ref
                     : scope_new(SCOPE_BLOCK, ss->scope, block);
    if (!block->type_ref) {
        block->type_ref = blk_scope;
        /* Register DECL_STMT variables so scope_lookup can find them. */
        for (size_t i = 0; i < block->u.block.stmts.len; i++) {
            AstNode *s = block->u.block.stmts.data[i];
            if (!s || s->kind != AST_DECL_STMT) continue;
            AstNode *vd = s->u.decl_stmt.decl;
            if (vd && vd->kind == AST_VAR_DECL && vd->u.var_decl.name)
                scope_define(blk_scope, SYM_VAR,
                             vd->u.var_decl.name, vd, ss->ctx->diags);
        }
    }
    SS inner = *ss;
    inner.scope = blk_scope;
    /* Walk stmts; last expr-stmt's type is the whole expression's type. */
    for (size_t i = 0; i < block->u.block.stmts.len; i++) {
        AstNode *s = block->u.block.stmts.data[i];
        if (!s) continue;
        if (i + 1 == block->u.block.stmts.len && s->kind == AST_EXPR_STMT)
            t = sema_expr(&inner, s->u.expr_stmt.expr);
        else
            sema_stmt(&inner, s);
    }
    break;
}
```

The scope is attached to `block->type_ref` (same slot used by scope_build
for ordinary blocks) so it is freed by `scope_free_chain` with the rest of
the scope tree — no separate cleanup needed.

#### cg.c

Emit `({` then each statement (via the existing `cg_stmt`), then ` })`:

```c
case AST_STMT_EXPR:
    cg_puts(ctx, "({");
    for (size_t i = 0; i < block->u.block.stmts.len; i++) {
        cg_puts(ctx, " ");
        cg_stmt(ctx, block->u.block.stmts.data[i], NULL, 0);
    }
    cg_puts(ctx, " })");
    break;
```

The generated form `({ int _a = 3; int _b = 7; ((_a > _b) ? _a : _b); })`
is the canonical GNU extension syntax accepted by both GCC and clang.

### Feature 3 — `__builtin_va_arg(ap, TYPE)` (probe p70)

No code change needed — the existing opaque `va_arg` / `__builtin_*`
handling already accepted this form correctly.  Probe `p70_builtin_va_arg`
was added to the corpus to prevent future regressions.

## Probe corpus growth

| # | name | language | covers |
|---|---|---|---|
| **p70** | `builtin_va_arg.c` | C | (R4) `__builtin_va_arg(ap, int)` end-to-end |
| **p71** | `stmt_expr.c` | C | (R4) `({ int _a=(a),_b=(b); _a>_b?_a:_b; })` |
| **p72** | `atomic.c` | C | (R4) `__atomic_store_n` + `__atomic_load_n` |

## Files changed in R4

* `sharp-fe/ast.h`
  - new `AST_STMT_EXPR` enum value (after `AST_AT_INTRINSIC`)
  - new `struct { AstNode *block; } stmt_expr` union member
* `sharp-fe/ast.c`
  - `ast_node_free`: new `AST_STMT_EXPR` arm (recurse into block)
  - `ast_kind_name` table: added `"STMT_EXPR"`
  - `ast_print`: new `AST_STMT_EXPR` arm
* `sharp-fe/parse.c`
  - `parse_primary` `STOK_LPAREN` case: new `ps_at(STOK_LBRACE)` guard
    at the top, builds `AST_STMT_EXPR` before the type-start heuristic
* `sharp-fe/sema.c`
  - `AST_IDENT` builtin-prefix check: `__atomic_` and `__sync_` added
  - new `AST_STMT_EXPR` case in `sema_expr` with lazy scope creation
* `sharp-fe/cg.c`
  - new `AST_STMT_EXPR` case in `cg_expr`
* `sharp-fe/test_decls.c`
  - new `test_r4_gcc_builtins` (4 cases); called from `main`
* `sharp-fe/c_superset_probes/`
  - new: `p70_builtin_va_arg.c`, `p71_stmt_expr.c`, `p72_atomic.c`

R4 touched **no files in `sharp-cpp/`** — third consecutive phase where
every fix lives in `sharp-fe/`, consistent with the maturity of sharp-cpp.

## Architectural decisions locked in by R4

* **`__atomic_*` and `__sync_*` are structurally identical to
  `__builtin_*` in our model.**  All three families are GCC compiler
  intrinsics that are never declared in source, always resolved by cc.
  The type returned by sema (int) is a placeholder — cc validates the
  real argument types.  Extending the prefix check rather than adding
  per-function whitelists is the right level of abstraction.

* **Statement-expression scope is created lazily, not eagerly.**
  `scope_build` is a statement-level walk; adding expression-level
  traversal to it would require a recursive expression walker touching
  every expression node in the program.  The lazy approach (create the
  scope on first encounter in `sema_expr`) is bounded and idempotent: the
  `block->type_ref` guard ensures the scope is created exactly once even
  if the same stmt-expr node is visited multiple times.

* **cg emits `({ ... })` verbatim, delegating semantics to cc.**
  This matches Sharp-fe's transpiler philosophy: parse enough to not error,
  emit enough for cc to validate.  The GNU extension is accepted by every
  cc target we support (gcc, clang, tcc); cc's error messages for malformed
  stmt-exprs will be clear.

* **p70 confirms `__builtin_va_arg` was already handled.**  The opaque
  `__builtin_*` mechanism introduced in an earlier phase covers
  `__builtin_va_arg(ap, int)` without special-casing the type argument.
  Probe p70 prevents this from silently regressing.

## Known limitations carried forward

| Construct | Status | Notes |
|---|---|---|
| K&R-style function definitions | not handled (probe `p21`) | indefinitely deferred |
| GCC `__attribute__((vector_size(N)))` as cast type | not handled | bypassed via `-DSTBI_NO_SIMD` |
| GCC `__int128` native type | not handled | workaround: `-D__int128=long` |
| Multi-designator `.a.b[3] = v` initialiser | not handled | unchanged |
| stmt-expr with nested stmt-expr | untested | not observed in real-world targets |
| stmt-expr containing `break`/`continue`/`return` | silently accepted | cc enforces the restriction |

R4 closed two of the three "sqlite blocker" items listed in R3's known
limitations.  The remaining sqlite-specific blocker (`__builtin_va_arg`
with a type argument) was already handled.  The `__int128` workaround
(`-D__int128=long`) covers its two sqlite uses.

## Resuming after R4

Suggested next phases, in priority order:

1. **sqlite-amalgamation end-to-end (R5)** — with R4's fixes, the main
   remaining blockers for sqlite end-to-end are:
   - `__atomic_*` calls that also use `__int128` (minor, workaround works)
   - Any remaining parse/cg gaps in the 258K-line amalgamation
   - Writing `build_sqlite_with_sharpfe.sh` and running it to surface gaps
2. **Error-message golden tests** — diagnostic quality has grown through
   S1..R4 without a regression corpus; 50 known-error cases would protect
   against silent regressions (PLAN.md §R3).
3. **`#line` injection + debugger transparency** — making sharpc emit
   `#line N "source.sp"` markers so gdb/lldb shows Sharp source locations
   (PLAN.md §R5).
4. **Sixth real-world target** — `lz4`, `tinycc`, `mcpp`, or `nuklear`.

## How to reproduce

```bash
cd sharp-fe
make test          # 207 / 207
make asan          # 0 memory errors
make strict        # 0 warnings
./c_superset_probe.sh   # 69 / 70 (p21 deferred)
```

— Phase R4, 2026-05-04
