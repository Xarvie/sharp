# Phase S5 — full pipeline integration with the Lua testsuite

**Status:** complete and green on all four gates.
**Goal:** route `.c → sharp-cpp → sharp-fe → C → cc → binary`, build Lua
5.4.7, and reach `final OK !!!` on the official testsuite — the same
marker the cpp-only baseline already reaches.  This was the last
remaining sub-stage in the C-superset roadmap (S1..S5).

## Gate results

| Gate | Before S5 | After S5 |
|---|---|---|
| `make test` | 187 / 187 | **190 / 190** (probes 69 + integration 40 + import 25 + decls 56) |
| `make asan` | leaks (5 leaks, 144 B; pre-existing in `import.c`) | **clean** |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | **clean** |
| `c_superset_probe.sh` | 50 / 51 | **51 / 52** (1 new probe added; only the indefinitely-deferred `p21_kr_func` still fails) |
| `build_lua_with_sharpfe.sh` | n/a (script did not exist) | **`final OK !!!`** |

Per-stage Lua build counts: 34 / 34 preprocess (cpp), **34 / 34 fe
(sharpc)**, 34 / 34 compile (cc), link OK, smoke OK, testsuite OK.

The S4 documentation claimed `make asan: clean`, but on a fresh
extraction the asan gate showed five pre-existing leaks in `import.c`.
Those are now fixed (see the splice_one section below); S5 leaves the
asan gate genuinely clean.

## Two real-world gaps surfaced (and fixed)

Routing all 34 Lua source files through `sharpc` for the first time
exposed two issues the synthetic 41-probe corpus had missed.  Both are
real-world C patterns that the front-end now handles correctly.

### Gap 1 — GCC labels-as-values (`&&label`, `goto *expr;`)

Lua's interpreter dispatch loop (`lvm.c` + `ljumptab.h`) builds a
static array of label addresses and jumps with a computed goto:

```c
static const void *const disptab[NUM_OPCODES] = {
    &&L_OP_MOVE,
    &&L_OP_LOADI,
    /* … */
};
goto *disptab[GET_OPCODE(i)];
```

This is the GCC labels-as-values extension (predates ISO C; both gcc
and clang accept it natively).  sharp-fe rejected it before S5
because:
- `&&` was only known as the binary logical-and; in unary position it
  produced "unexpected token '&&' in expression".
- `goto *expr;` was rejected because `parse_stmt`'s goto handler
  required a bare identifier.

#### Fix (across parse / sema / cg)

Two new AST kinds were added with paired counterparts in every layer:

* **`AST_ADDR_OF_LABEL { char *label; }`** — the prefix `&&label`
  expression.  Types as `void *`.
* **`AST_COMPUTED_GOTO { AstNode *target; }`** — `goto *expr;`.  The
  target is sema-walked (so identifiers in it resolve in scope) but
  not type-constrained on Sharp's side; cc rejects non-pointer targets.

Per-layer changes:

| File | Change |
|---|---|
| `ast.h` | New enum entries (`AST_ADDR_OF_LABEL`, `AST_COMPUTED_GOTO`); two new union members. |
| `ast.c` | `ast_node_free` arms (free `addr_of_label.label`, recurse into `computed_goto.target`); kind-name table; `ast_print` arms. |
| `parse.c` | (a) Top of `parse_expr_prec`: when the lead token is `STOK_AMPAMP` and the follow-up is `STOK_IDENT`, build `AST_ADDR_OF_LABEL` and skip the unary-prefix arm.  (b) `parse_stmt` goto branch: if the next token is `STOK_STAR`, consume it and parse `goto *expr;` into `AST_COMPUTED_GOTO`. |
| `sema.c` | (a) `sema_expr` arm for `AST_ADDR_OF_LABEL` returns `void *`.  (b) `sema_stmt` arm for `AST_COMPUTED_GOTO` walks the target.  (c) Defer-cross check (`dc_stmt`) extended: a computed goto is just as unstructured as a plain goto; same diagnostic. |
| `cg.c` | (a) `cg_expr` arm: emit `&&label` verbatim.  (b) `cg_stmt` arm: emit `goto *(expr);` after running pending defers (parens guard against operator-precedence surprises). |
| `test_decls.c` | New `test_s5_computed_goto` suite with three end-to-end cases (sharpc → cc → run): table dispatch with `i=1`, table dispatch with `i=0`, and `void *p = &&here; goto *p;`. |
| `c_superset_probes/p51_computed_goto.c` | New regression probe — minimal model of the Lua dispatch idiom. |

#### Disambiguation argument

`&&` could be either binary logical-and or unary address-of-label.
The disambiguation is *positional*: the Pratt-style `parse_expr_prec`
either builds an lhs from a unary prefix or from `parse_primary`
before entering the binop loop.  Binary `&&` is only consumed inside
that loop (after some lhs already exists), so when `&&` appears at the
*start* of expression parsing it is unambiguously the labels-as-values
operator.  A defensive `peek2 == STOK_IDENT` check makes a stray `&&`
without a label name fall through to the existing parser's error path
rather than mis-reporting it.

### Gap 2 — float-literal precision truncation

After the labels-as-values support landed and Lua linked, the smoke
test passed but the testsuite died at `math.lua:650`:

```
assert(eq(math.tan(math.pi/4), 1))   -- assertion failed!
```

`math.tan(math.pi/4)` came out as `0.99999867…` instead of
`0.99999999…99…`, far outside the assertion's epsilon.

Direct inspection of Lua's regenerated C narrowed the cause to one
line in `cg.c`:

```c
case AST_FLOAT_LIT:
    cg_printf(ctx, "%g", expr->u.float_lit.val);   /* <-- bug */
```

`%g` defaults to **6 significant digits**.  Lua's `lmathlib.c`
defines `PI` as `(3.141592653589793238462643383279502884)`.  After
sharp-cpp this 36-digit literal arrives at sharp-fe, gets parsed by
`strtod` to a precise double, and is then re-emitted as `3.14159` —
losing the 7th and following digits silently.  The resulting `math.pi`
is off by ~3.6e-6, which compounds inside `tan` to a sin/cos ratio
that fails the test.

#### Fix

```c
case AST_FLOAT_LIT: {
    char buf[40];
    snprintf(buf, sizeof buf, "%.17g", expr->u.float_lit.val);
    cg_puts(ctx, buf);
    bool has_dot = false;
    for (const char *p = buf; *p; p++)
        if (*p == '.' || *p == 'e' || *p == 'E') { has_dot = true; break; }
    if (!has_dot) cg_puts(ctx, ".0");
    break;
}
```

Two changes in one:

1. **`%.17g`** is `DBL_DECIMAL_DIG` — the minimum number of decimal
   digits that round-trips any IEEE 754 double exactly.  Now
   `math.pi` regenerates as `3.141592653589793` (the closest double),
   and `tan(pi/4)` matches the cpp-only baseline to within ULPs.

2. **Force a `.0` suffix** when the printed form has neither `.` nor
   `e/E`.  `%g` strips trailing zeros, so `1.0` prints as `1`.  In C,
   `1` is `int` and `1.0` is `double` — the difference flips
   `1.0 / 2` (= 0.5) into `1 / 2` (= 0).  After this fix, sharp-fe
   never emits an integer-shaped literal where the source had a float.

This fix is **two lines beyond what S5 strictly required**, but the
`.0` enforcement was clearly needed for correctness; finding it now
costs nothing and saves the next person a hard-to-track bug.

## Pre-existing leaks fixed

Five leaks (144 bytes total) showed up under `make asan` on a fresh
extraction, all in `import.c`'s splice path:

```
astvec_push (parse.c:3099)
parse_sp_file (import.c:144)
splice_one  (import.c:192)
resolve_file (import.c:262)
```

The pattern in `splice_one`:

```c
/* Before — leaks the realloc'd backing array */
imported->u.file.decls.data = NULL;
imported->u.file.decls.len  = 0;
imported->u.file.decls.cap  = 0;
ast_node_free(imported);
```

The intent was "zero so `ast_node_free` won't double-free children" —
correct, because the children's pointers had been moved into `nd[]` —
but the **backing array itself** is owned by `imported` and was being
abandoned.

Fix:

```c
/* After — frees the backing array, leaves children intact (already
 * moved into nd[]).  astvec_free zeroes the AstVec fields itself. */
astvec_free(&imported->u.file.decls);
ast_node_free(imported);
```

`astvec_free` lives in `ast.c`, so `import.c` gained `#include "ast.h"`.
This pre-existing bug was not caused by S5 work; finding and fixing
it together makes the asan gate honest.

## `build_lua_with_sharpfe.sh`

Sister script to `build_lua_with_sharp.sh`.  Pipeline per source file:

```
foo.c   ──[sharp_cli -P]──>  foo.i      (sharp-cpp preprocessing)
foo.i   ──[sharpc -P]────>   foo.fec.c  (sharp-fe parse / sema / cg)
foo.fec.c ──[cc -c]──────>   foo.o
```

The script auto-downloads Lua 5.4.7 (or accepts a pre-existing source
dir as `$1`), preprocesses every `.c`, runs each result through
`sharpc`, hands the regenerated C to `cc`, links `lua` and `luac`,
smoke-tests both, and then (with `RUN_TESTSUITE=1`, the default)
auto-downloads and runs `lua-5.4.7-tests/all.lua`.  Per-file logs are
kept in `$BUILD_DIR/_logs/`.  Exit code is non-zero unless the
testsuite reaches `final OK !!!`.

## Files changed

* `ast.h` — two enum entries, two union members.
* `ast.c` — three arms (free, kind-name, print) for each new kind.
* `parse.c` — labels-as-values prefix, computed-goto stmt branch.
* `sema.c` — `AST_ADDR_OF_LABEL` types as `void*`; `AST_COMPUTED_GOTO`
  walked; defer-cross check extended.
* `cg.c` — emit `&&label`, `goto *(expr);`, fix `%g` truncation,
  enforce float `.0` suffix.
* `import.c` — `astvec_free` instead of pointer-zeroing in
  `splice_one`; new `#include "ast.h"`.
* `test_decls.c` — `test_s5_computed_goto` suite (3 cases).
* `c_superset_probes/p51_computed_goto.c` — new minimal probe.
* `c_superset_probe.sh` — chmod +x (was lost in tarball extraction).
* `build_lua_with_sharpfe.sh` — new top-level driver script.

## Architectural decisions locked in by S5

* **`&&` disambiguation is positional, not lexical.**  Lex still
  produces a single `STOK_AMPAMP` token whether the use is binary
  logical-and or unary address-of-label.  Parse decides which based
  on whether `&&` appears at expression start vs. after an lhs is
  built.  Lex stays simple; no new token kind.
* **Computed-goto target is unconstrained on Sharp's side.**  Sema
  walks the target so its identifiers resolve, but the front-end
  does not require pointer type — cc enforces the rest.  This
  matches how Sharp delegates several other low-level constraints
  (bit-field width, init-list type matching) to cc.
* **Defer-cross check applies to computed goto.**  A `goto *expr;`
  jumps to a label whose name we don't statically know.  We can't
  prove it doesn't cross a defer, so the conservative rule from
  Phase 9 (any goto + any defer in the same function = error) is
  extended.  Diagnostic text mentions "computed goto" instead of a
  label name.
* **Float-literal regeneration uses `%.17g` + forced `.0` suffix.**
  This is the canonical IEEE 754 round-trip format with the
  defensive type-preservation check.  No future caller will need to
  worry that constant folding / regeneration silently changes a
  float into an int.
* **`splice_one` ownership rule, made explicit.**  Backing array of
  `imported->decls` is freed via `astvec_free` after children are
  moved into the new array; from now on, ownership transfer in this
  module is "move children, free old container", not "null out
  fields and hope".

## Known limitations carried forward (still — none added in S5)

| Construct | Status | Notes |
|---|---|---|
| K&R-style functions | not handled (probe `p21`) | indefinitely deferred — Lua doesn't use it; no other real-world target requested it |
| Multi-designator `.a.b[3] = v` | not supported | unchanged from S4 |
| Range designator `[1 ... 3] = 0` (GCC ext) | not supported | unchanged from S4 |
| `( declarator )` vs `( params )` ambiguity (`int (foo)(int)`) | uses S1 conservative heuristic | unchanged from S4 |

The labels-as-values extension is now supported, so it leaves this
list.  No new limitations were introduced in S5.

## Snapshots

* Pre-S5 baseline: implicit (the post-S4 tree as extracted).
* Post-S5 deliverable: `sharp-phases5.tar.gz` — the green checkpoint
  with `make test = 190/190`, `make asan` and `make strict` clean,
  C-superset 51/52, Lua testsuite `final OK !!!`.

## Resuming after S5

S5 closes the C-superset roadmap (S1..S5).  `sharp-fe` now accepts
ISO C11 plus the GCC extensions Lua 5.4.7's source uses, and the
end-to-end pipeline `.c → sharp-cpp → sharp-fe → C → cc → binary`
produces a Lua interpreter that passes the official testsuite.

Suggested next stages, in priority order:

1. **Phase R1 — second real-world target.**  Lua exercised computed
   goto + math precision; another mid-sized C codebase will surface
   different constructs.  Candidates: `mcpp` (the cpp project's own
   regression target — already in the cpp tree), `nuklear`,
   `stb_image`, `sqlite` (amalgamation).  Pattern: write a
   `build_X_with_sharpfe.sh`, run, reduce each fresh failure mode to a
   `c_superset_probes/p52..p60_*.c` probe, fix in the right layer.
2. **Phase R2 — error-message golden tests.**  PLAN.md §R3 was
   specced but never delivered; the diagnostic surface has grown a
   lot through S1..S5 and a corpus of ~50 known-error `.sp` /
   `.c` files with golden output would protect against silent
   regressions.
3. **Phase R7 — sharpc benchmark vs gcc.**  Now that sharpc compiles
   real-world C, a wall-time / RSS comparison against gcc on the same
   34 .c files of Lua is meaningful.
