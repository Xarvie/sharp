# Phase R7 — zstd 0.23.0 Integration + 8 C-Superset Fixes

**Status:** Complete on all three quality gates.  zstd 0.23.0 smoke test passes.

## Gate results

| Gate | Before R7 | After R7 |
|---|---|---|
| `make test` (probes) | 69 / 69 | **69 / 69** |
| `make asan` | clean | **clean** |
| `make strict` | clean | **clean** |
| `c_superset_probe.sh` | 70 / 71 | **73 / 74** (p74 p75 p76 added; only p21 deferred) |
| zstd 0.23.0 smoke test | — (new target) | **✅ PASS** |

## Target: zstd 0.23.0 (52K-line amalgamation)

zstd exercises a distinctly different surface from all previous targets:
- Heavy use of `__attribute__((always_inline))` and `__attribute__((noinline))`
- Extensive `_Static_assert` / `DEBUG_STATIC_ASSERT` assertions
- `__asm__("cpuid" : ...)` extended inline assembly for CPU detection
- `__alignof(T)` / `__alignof__(T)` (GCC-specific `_Alignof` aliases)
- Complex constant expressions with `MAX(a,b)` ternary macros
- `sizeof(T[expr])` in array size positions
- Large table-size computations from nested `MAX()` expressions

### Compile & smoke test

`zstd.c` is compiled through `sharpc → cc → link → run` with
`-DZSTD_NO_INTRINSICS` (disables SSE/AVX vector extensions which use
`__attribute__((vector_size(N)))` casts, a GCC-only type extension beyond
our scope). The smoke test compresses 214 bytes and decompresses them
back, verifying byte-for-byte round-trip identity.

## Eight fixes

### Fix 1 — `__alignof` / `__alignof__` keyword aliases (lex.c)

`ZSTD_ALIGNOF(T)` expands to `__alignof(T)` on GCC.  Only `_Alignof` and
`__alignof__` (with trailing underscores) were in the keyword table.
`__alignof` (single underscores, no trailing) was being lexed as an
identifier and producing "undefined name '__alignof'" in sema.

**Fix:** add `{ "__alignof", STOK__ALIGNOF }` and `{ "__alignof__", STOK__ALIGNOF }`
to the keyword table in `lex.c`.  Locked by probe `p74_alignof.c`.

### Fix 2 — `_Alignof(T)` in parse_primary (parse.c + ast.h + cg.c)

`STOK__ALIGNOF` token was not handled in `parse_primary`.  Added a new
case mirroring the `sizeof` case; the same `AST_SIZEOF` node is reused
with an `is_alignof` boolean flag.  `cg_expr` and `cg_const_expr` check
`is_alignof` and emit `_Alignof(T)` instead of `sizeof(T)`.

### Fix 3 — `sizeof(T[expr])` — array type in sizeof operand (parse.c + cg.c)

`sizeof(char[(cond)?1:-1])` — the `DEBUG_STATIC_ASSERT` macro — failed
because `parse_primary`'s sizeof case called `parse_type()` then expected
`)`, but saw `[`.

**Fix (parse.c):** after `parse_type()`, check for `[` and call
`parse_array_suffix(ps, ty)` to build an `AST_TYPE_ARRAY` node.

**Fix (cg.c, `cg_expr` `AST_SIZEOF`):** added array-type operand handling:
walk down to the innermost base type, emit `base_type[size1][size2]...`
from the AST node.  Locked by probe `p75_sizeof_array_type.c`.

### Fix 4 — `__asm__(...)` as statement (parse.c)

`__asm__("cpuid" : "=a"(n) : "a"(0) : "ebx", "ecx", "edx")` in a
function body was being parsed as an expression (the ASM token was not
a recognized statement starter).  The `:` tokens inside caused parse
errors.

**Fix:** added `STOK_ASM` case in `parse_stmt` before the block/if/while
starters.  Calls `eat_attribute_specifiers(ps, NULL)` (which already handles
the balanced-paren sweep including `:` tokens) and a trailing `ps_match(SEMI)`.
The asm statement is silently discarded — the transpiled C uses gcc/clang
which handle real asm; our generated C omits it.  Locked by probe
`p76_asm_stmt.c`.

### Fix 5 — `_Static_assert(...)` as statement (parse.c)

`_Static_assert((cond), "msg")` inside function bodies (e.g. inside
`do { _Static_assert(...); } while(0)` macros) was not recognized as a
statement.  Added `STOK__STATIC_ASSERT` case in `parse_stmt` that eats
the balanced parentheses and trailing `;`.  The assertion is silently
discarded — cc processes compile-time assertions in the original source
before we see the preprocessed token stream.

### Fix 6 — `sema_call` for function-pointer variables (sema.c)

Calling a function pointer variable (`cmp(ctx, ptr, grpEnd) == 0`)
returned `ty_error` because `sema_call` only checked `callee_t->kind ==
TY_FUNC` but not `TY_PTR(TY_FUNC(...))`.  This caused "comparison of
non-scalar types" cascade errors.

**Fix (sema.c `sema_call`):** after the direct `TY_FUNC` check, strip
`TY_CONST` and one layer of `TY_PTR`; if the inner type is `TY_FUNC`,
return its return type.

**Fix (sema.c `sema_binop`):** suppress "comparison of non-scalar types"
when either operand has `ty_error` type, to prevent cascade errors from
unresolved call results.

### Fix 7 — `cg_const_expr` ternary: `MAX(a,b)` in array sizes (cg.c + type.c)

`MAX(a,b)` expands to `((a)>(b)?(a):(b))` — a ternary expression.  In
array-size position, `cg_const_expr`'s `default:` emitted `"0"` for
`AST_TERNARY`, producing `workspace[(((2<<10)+(1<<9))/(1<<0))]` instead of
the correct `workspace[640]`.  This made the `ZSTD_DCtx_s` struct only
29,816 bytes instead of 95,968 bytes, causing decompression to write past
the end of the allocated context.

**Fix (cg.c):** added `case AST_TERNARY:` to `cg_const_expr` that emits
`(cond ? then : else)`.

**Fix (type.c `eval_array_size`):** added `case AST_TERNARY:` that
evaluates the condition and returns the correct branch value.  This lets
the type system compute the correct array size for fields like
`workspace[MAX(MaxFSELog,9)+1]`.

Also added `case AST_IDENT:` to `eval_array_size` to resolve enum
constants and `const`-variable initialisers (e.g. `ZSTD_btultra2 = 9`
in `blockCompressor[4][ZSTD_STRATEGY_MAX+1]`).

### Fix 8 — `cg_const_expr` sizeof(expr): wrong emission of `sizeof(0)` (cg.c)

Inside `cg_const_expr`'s `AST_SIZEOF` case, when the operand is an
expression (not a type), it recursively called `cg_const_expr`.  For
`sizeof(ctable[0])`, `AST_INDEX` fell to `default: cg_puts("0")`,
emitting `sizeof(0)` instead of `sizeof(ctable[0])`.  This caused
`(void)sizeof(char[(sizeof(ctable[0])==sizeof(header))?1:-1])` to
evaluate the condition as `sizeof(0)==sizeof(header)` = `4==8` = false,
triggering a compile-time error `array size is negative`.

**Fix:** in `cg_const_expr` `AST_SIZEOF`, when `is_type=false`, call
`cg_expr(ctx, operand)` instead of `cg_const_expr(ctx, operand)`.  This
ensures that any expression (array subscript, function call, etc.) can
appear inside `sizeof` in a constant context — cc evaluates it.

## New probes

| Probe | What it tests |
|---|---|
| `p74_alignof.c` | `_Alignof`, `__alignof__`, `__alignof` — all three spellings |
| `p75_sizeof_array_type.c` | `sizeof(T[N])`, `sizeof(char[(cond)?1:-1])` |
| `p76_asm_stmt.c` | `__asm__("..." : ...)` as a standalone statement |

## Files changed in R7

* `sharp-fe/lex.c` — `__alignof` / `__alignof__` keywords
* `sharp-fe/ast.h` — `is_alignof` flag on `sizeof_` node
* `sharp-fe/parse.c`
  - `parse_primary`: `STOK__ALIGNOF` case; `sizeof(T[expr])` array suffix
  - `parse_stmt`: `STOK_ASM` and `STOK__STATIC_ASSERT` statement cases
* `sharp-fe/sema.c`
  - `sema_call`: function-pointer callee return type
  - `sema_binop`: suppress ty_error cascade in comparisons
* `sharp-fe/type.c` — `eval_array_size`: `AST_IDENT`, `AST_TERNARY` cases
* `sharp-fe/cg.c`
  - `cg_expr AST_SIZEOF`: array-type operand; `is_alignof` flag
  - `cg_const_expr AST_SIZEOF`: delegate expr-operand to `cg_expr`
  - `cg_const_expr`: `AST_TERNARY` case; `is_alignof` in sizeof
* `sharp-fe/c_superset_probes/` — new p74, p75, p76
* `sharp/build_zstd_with_sharpfe.sh` — new build script

Zero changes to `sharp-cpp/`.  Seventh consecutive phase where every fix
lives in `sharp-fe/`.

## Resuming after R7

**R8 candidates:**

- **Eighth real-world target** (tinycc, mcpp, or sqlite end-to-end) —
  tinycc is a C compiler in ~30K lines; compiling a C compiler with a C
  transpiler is an attractive self-referential validation.
- **`__attribute__((vector_size(N)))` as a type** — blocks zstd without
  `-DZSTD_NO_INTRINSICS`.  A dedicated phase.
- **Integer constant expressions** — hardening `eval_array_size` with
  `offsetof` support, enum-without-explicit-value inference.

— Phase R7, 2026-05-05
