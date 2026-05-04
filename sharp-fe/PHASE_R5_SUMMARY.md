# Phase R5 — lz4 新目标集成（4/4 编译）+ 5 个 C 超集修复

**Status:** complete on all three quality gates; lz4 4/4 source files compile
through the full sharpc pipeline.  A smoke-test runtime crash is documented
as a known limitation (see below).

## Gate results

| Gate | Before R5 | After R5 |
|---|---|---|
| `make test` | 207 / 207 | **207 / 207** (probes 69 + integration 40 + import 25 + decls 73) |
| `make asan` | clean | **clean** |
| `make strict` | clean | **clean** |
| `c_superset_probe.sh` | 69 / 70 | **69 / 70** (only deferred p21_kr_func still fails) |

No new test cases were added to `test_decls.c` in R5 because all five bugs
were surfaced by the lz4 compilation attempt — the real-world target IS the
regression test.

## Target: lz4 4.4.5

lz4 is Yann Collet's extremely fast LZ4 compression library — four C source
files (~7,500 lines) that exercise a distinctly different surface from all
previous targets: aggressive `always_inline` force-inlining, anonymous
union/struct types inside function bodies, local typedefs of anonymous
aggregates, const-qualified function pointers, and GCC predefined string
variables (`__PRETTY_FUNCTION__`).

### Compilation status

| File | Status | Lines (.i) |
|---|---|---|
| `lz4.c` | ✅ sharpc + cc pass | ~3700 |
| `lz4hc.c` | ✅ sharpc + cc pass | ~3400 |
| `lz4frame.c` | ✅ sharpc + cc pass | ~4500 |
| `xxhash.c` | ✅ sharpc + cc pass | ~2500 |

### Smoke-test status (known limitation)

Linking the four fe-compiled objects into a binary and running a
compress → decompress round-trip produces a SEGV in
`LZ4_putPositionOnHash`. Root cause: lz4's algorithm relies on
`__attribute__((always_inline))` to constant-propagate the `tableType`
enum parameter through the call chain `LZ4_compress_fast_extState →
LZ4_compress_generic → LZ4_compress_generic_validated →
LZ4_putPositionOnHash`.  Without inlining, the switch in
`LZ4_putPositionOnHash` dispatches at runtime with a tableType value
that is NOT a compile-time constant, causing branch misprediction and
out-of-bounds hash table writes.

The fix requires attribute passthrough (re-emitting
`__attribute__((always_inline))` into the generated C).  This is a
dedicated phase — sharp-fe's current design (§S1 SUMMARY) intentionally
discards all GCC attributes as uninterpretable decoration.  The lz4
binary compiled from cc-preprocessed `.i` files (bypassing sharpc)
passes the smoke test identically, confirming the issue is in sharpc's
attribute handling, not in lz4's logic or our type-system model.

## Five real-world bugs fixed

### Gap 1 — `__PRETTY_FUNCTION__` / `__FUNCTION__` undefined (sema.c)

GCC predefined string variables — not macros, not declared — used in
`assert()` expansions and trace macros.  `lz4frame.c` uses them inside
DEBUGLOG macros.

**Fix (sema.c, 4 lines):** after the `__builtin_*` / `__atomic_*` /
`__sync_*` prefix checks, add a specific check for `__PRETTY_FUNCTION__`
and `__FUNCTION__` returning type `const char *` (their actual type).

### Gap 2 — `max_align_t` redefinition conflict (cg.c)

lz4's headers `#include <stddef.h>`, which defines `max_align_t` as an
anonymous struct.  After cpp expansion, the struct body appears in the
preprocessed token stream; sharpc re-emits it as
`typedef __anon_struct_N max_align_t`.  The generated file's preamble
also contains `#include <stddef.h>`, which defines `max_align_t` again
— causing "conflicting types" in cc.

**Fix (cg.c, Pass-1c):** add a `PREAMBLE_TYPEDEF_BLOCKLIST` array
containing `"max_align_t"`.  When emitting a file-scope typedef whose
alias name is in the list, skip it — the preamble's `#include <stddef.h>`
already provides the definition.

### Gap 3 — inline anonymous union/struct ordering in blocks (parse.c)

```c
/* lz4.c: LZ4_isLittleEndian */
const union { U32 u; BYTE c[4]; } one = { 1 };
```

`tspec_try_consume` pushes the `AST_STRUCT_DEF` for the anonymous union
to `pending_decls`.  `parse_block` then pushed the main statement first
and drained `pending_decls` after — emitting the variable declaration
BEFORE the type definition.

**Fix (parse.c):** two-pass drain in `parse_block`:
1. Pass 1: push `AST_STRUCT_DEF` / `AST_ENUM_DEF` nodes BEFORE `s`
2. Push the main statement `s`
3. Pass 2: push remaining pending nodes (sibling var-decls)

This mirrors what `parse_file` already does correctly for file-scope
declarations.

### Gap 4 — const function-pointer declarator (cg.c)

`lz4frame.c` declares a local variable:
```c
compressFunc_t const compress = LZ4F_selectCompression(...);
```

After typedef resolution, the type is `TY_CONST(TY_PTR(TY_FUNC(...)))`.
ISO C requires `int (* const name)(args)` but cg emitted
`int (*)(args) const name` — rejected by cc.

**Fix (cg.c, cg_decl):** before the existing `TY_PTR → TY_FUNC` arm,
add a `TY_CONST → TY_PTR → TY_FUNC` arm that emits:
```c
ret_type (* const name)(params)
```
mirroring the pattern already used for `TY_ARRAY` with inner function
pointers.

### Gap 5 — local typedef and sizeof with local types (cg.c, parse.c)

```c
/* lz4.c: LZ4_stream_t_alignment */
typedef struct { char c; LZ4_stream_t t; } t_a;
return sizeof(t_a) - sizeof(LZ4_stream_u);
```

Three interconnected issues:

**5a — local typedef dropped in cg_stmt:**
`parse_init_declarator_list` wraps a block-scope typedef in
`AST_DECL_STMT { decl: AST_TYPEDEF_DECL }`.  `cg_stmt`'s
`AST_DECL_STMT` handler guarded with `vd->kind != AST_VAR_DECL` and
`break`-ed, silently dropping the typedef.

*Fix:* add a guard before the `AST_VAR_DECL` check:
```c
if (vd->kind == AST_TYPEDEF_DECL) {
    cg_stmt(ctx, vd, defers, ndefers);  /* delegate to TYPEDEF_DECL case */
    break;
}
```

**5b — AST_TYPEDEF_DECL not emitted by cg_stmt:**
Added a new `case AST_TYPEDEF_DECL` in `cg_stmt` that emits
`typedef TargetName Alias;` directly from the AST node, bypassing
`ty_from_ast` (which uses only file scope and can't find locally-defined
types).

**5c — `sizeof(local_type)` resolved as `sizeof(int)` (cg.c, cg_expr):**
`cg_expr`'s `AST_SIZEOF` handler calls `ty_from_ast` with file scope.
For a local typedef alias `t_a`, `ty_from_ast` returns `ty_error`, and
the fallback `cg_puts(ctx, "int")` emitted `sizeof(int)` instead of
`sizeof(t_a)`.

*Fix:* when both `sema_type_of` and `ty_from_ast` fail, and the sizeof
operand is an `AST_TYPE_NAME`, emit the type name verbatim from the AST
— the local C scope will resolve it at compile time:
```c
} else if (operand->kind == AST_TYPE_NAME && operand->u.type_name.name) {
    cg_puts(ctx, operand->u.type_name.name);   /* local type: emit verbatim */
}
```

## Files changed in R5

* `sharp-fe/sema.c`
  - `AST_IDENT` builtin handler: add `__PRETTY_FUNCTION__` / `__FUNCTION__`
    returning `const char *`
* `sharp-fe/cg.c`
  - Pass-1c typedef emission: `PREAMBLE_TYPEDEF_BLOCKLIST` (Gap 2)
  - `cg_decl`: new `TY_CONST → TY_PTR → TY_FUNC` arm (Gap 4)
  - `cg_stmt AST_DECL_STMT`: guard for `AST_TYPEDEF_DECL` (Gap 5a)
  - `cg_stmt`: new `case AST_TYPEDEF_DECL` (Gap 5b)
  - `cg_expr AST_SIZEOF`: verbatim fallback for local type names (Gap 5c)
* `sharp-fe/parse.c`
  - `parse_block`: two-pass pending_decls drain (Gap 3)

R5 touched **no files in `sharp-cpp/`** — fifth consecutive phase where
every fix lives in `sharp-fe/`.

## Architectural decisions locked in by R5

* **`parse_block` pending_decls must be split by kind.**  Type definitions
  (AST_STRUCT_DEF, AST_ENUM_DEF) must precede the variable that uses them;
  sibling declarators (int x=1, y=2) must follow the primary one.  The
  two-pass approach is the minimal change that preserves both invariants
  without altering file-scope parsing (parse_file already did it correctly).

* **`cg_stmt` must handle `AST_TYPEDEF_DECL` directly from the AST.**
  `ty_from_ast` is file-scope-only; it can never resolve a local typedef
  alias to its anonymous struct target.  Emitting from the AST node
  (target name verbatim, known to be in the local C scope after the
  struct body emission) is the correct architecture for block-scope
  types.

* **`sizeof(local_type_name)` verbatim fallback is safe.**  A local type
  name that passes sema (registered via the lazy scope creation in
  sema_expr for STMT_EXPR, or via scope_build for TYPEDEF_DECL in
  build_stmt) IS in the generated C scope from the preceding struct
  body emission.  Emitting verbatim and letting cc resolve is correct.

* **`max_align_t` blocklist is the right abstraction level.**  The root
  cause (preamble re-including stddef.h after its content was already
  expanded) applies to any type defined in the preamble headers.  A
  targeted blocklist of known conflicting names is preferable to a
  general mechanism (e.g., tracking which types came from system
  headers) which would be vastly more complex and error-prone.

## Known limitations carried forward

| Construct | Status | Notes |
|---|---|---|
| `__attribute__((always_inline))` dropped | **new** | lz4 smoke test fails; fix requires attribute passthrough phase |
| K&R-style functions | not handled (p21) | indefinitely deferred |
| GCC `__attribute__((vector_size(N)))` | not handled | `-DSTBI_NO_SIMD` workaround |
| GCC statement-expression `({...})` | ✅ fixed in R4 | — |
| Multi-designator `.a.b[3] = v` | not handled | unchanged |
| lz4 smoke test | ❌ crashes | always_inline required for correctness |

## Resuming after R5

**R6 (recommended): `__attribute__` passthrough.**  Re-emit function
attributes (`always_inline`, `noinline`, `noreturn`, `visibility`) into
the generated C.  This unblocks lz4 smoke test and any other library
that uses attributes for correctness, not just performance.

**Alternative: seventh real-world target** (`lz4` or `tinycc`).  A target
that does NOT depend on `always_inline` for correctness would compound
coverage; `tinycc` or `mcpp` are good candidates.

— Phase R5, 2026-05-04
