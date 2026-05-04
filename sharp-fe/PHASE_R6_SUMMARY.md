# Phase R6 — GCC `__attribute__` Passthrough + 3 Pre-existing Bug Fixes

**Status:** Complete on all three quality gates.  lz4 4.4.5 smoke test passes.

## Gate results

| Gate | Before R6 | After R6 |
|---|---|---|
| `make test` (probes) | 69 / 69 | **69 / 69** |
| `make asan` | clean | **clean** |
| `make strict` | clean | **clean** |
| `c_superset_probe.sh` | 69 / 70 | **70 / 71** (p73 new pass; only p21 still deferred) |
| lz4 smoke test | ❌ SEGV | **✅ PASS** |

New probe added: `p73_attribute_passthrough.c` — verifies `always_inline`,
`noreturn`, and `unused` attributes survive the sharpc pipeline.

## What R6 delivers

### Feature: `__attribute__((...))` passthrough (parse.c + ast.h + ast.c + cg.c)

GCC function and variable attributes are now **captured** during parsing and
**re-emitted** into the generated C, rather than silently discarded.

**Architecture:**

1. `eat_attribute_specifiers(PS *ps, char **out_attrs)` — new `out_attrs`
   parameter.  When non-NULL, the full text of every `__attribute__((...))` is
   reconstructed by concatenating the raw `.text` fields of the consumed
   tokens and appended (space-separated) to `*out_attrs`.
   `__asm__("name")` linker-rename suffixes are still silently discarded —
   the generated C resolves its own symbol names from headers.

2. `DeclSpecs.gcc_attrs` (parse.c) — accumulates leading attributes from
   the decl-specifier sequence (e.g. `__attribute__((always_inline)) static
   inline int f(...)`).

3. `AST_FUNC_DEF.gcc_attrs` and `AST_VAR_DECL.gcc_attrs` (ast.h) — store
   the combined leading + trailing attribute text.  Freed by
   `ast_node_free`.

4. `cg_func` emits `gcc_attrs` in **leading position** — after storage-class
   specifiers (`static`, `inline`), before the return type.
   `static inline __attribute__((always_inline)) int f(…)` is the canonical
   form accepted by all versions of GCC and Clang for both definitions and
   prototypes.  GCC 8+ rejects trailing attributes on function definitions.

5. `cg_decl` / Pass-1b emit `gcc_attrs` for variables **after the declarator**
   and before `=` or `;`, which is the standard C position for variable
   attributes (`int x __attribute__((aligned(16)));`).

**Coverage:**
- Leading attributes (decl-specifier position): `always_inline`, `optimize`,
  `cold`, `visibility`, `constructor`, `destructor`, etc.
- Trailing attributes (after declarator): `noreturn`, `malloc`, `unused`,
  `deprecated`, `format`, etc.
- All call sites updated: `parse_decl_specifiers`, `parse_top_decl` (ISO-C
  path, operator path, Sharp generic path), `parse_init_declarator_list`,
  `parse_struct_def` (method and operator paths, with field attrs discarded
  as before — alignment is cc's concern).

## Three pre-existing bugs fixed (exposed by R6's expanded test coverage)

### Bug 1 — `arith_conv` out-of-bounds read (sema.c)

`arith_conv` used a rank table indexed by `TyKind`.  The table size was
`TY_ULONGLONG + 1 = 13`, but the guard was `kind < TY_COUNT = 21`.  A
`TY_CONST` node (kind 17) would pass the guard and read `rank[17]` — 4
bytes beyond the array.  This is a write-invisible UBSan hit (`SEGV` for
some `const`-typed expressions in lz4).

**Fix (sema.c):** strip `TY_CONST` layers at the start of `arith_conv`
(correct semantics: `const T` behaves as `T` in arithmetic context), and
replace the `< TY_COUNT` guard with a proper bounds-check against the actual
array size (`sizeof rank / sizeof rank[0] - 1`).

### Bug 2 — Integer `U`-suffix dropped in `cg_expr` (cg.c)

`cg_expr`'s `AST_INT_LIT` case emitted:
```c
if (is_unsigned)
    cg_printf("%llu", val);   // ← unsigned long long, no suffix
else
    cg_printf("%lld", val);   // ← signed long long, no suffix
```

A source literal `2654435761U` (`unsigned int`) was emitted as `2654435761`
— with type `long` on 64-bit (the value exceeds `INT_MAX`).  In
`uint32_t * 2654435761` (long), the multiplication is 64-bit with no
truncation, producing hash indices in the range 0–10^13 for an 8192-entry
table.  lz4's compress loop immediately overwrites arbitrary stack memory.

**Fix (cg.c):** emit integer literals with their correct type suffix:
- `is_longlong + is_unsigned` → `%lluULL`
- `is_longlong + !is_unsigned` → `%lldLL`
- `is_long + is_unsigned` → `%luUL`
- `is_long + !is_unsigned` → `%ldL`
- `!is_long + !is_longlong + is_unsigned + val ≤ UINT32_MAX` → `%uU`
- `!is_long + !is_longlong + is_unsigned + val > UINT32_MAX` → `%lluULL`
- `!is_long + !is_longlong + !is_unsigned` → `%lld`

`cg_const_expr` (array-size context) received the same fix for consistency.

### Bug 3 — `cg_expr` `U`-suffix fix accidentally placed in `cg_const_expr`

During R6 development the fix was initially applied to `cg_const_expr`
(line 522) instead of `cg_expr` (line 705).  The two functions are adjacent;
`cg_const_expr` handles array size expressions; `cg_expr` handles all
expression-context integer literals including variable initialisers, function
arguments, and arithmetic sub-expressions.  Corrected by applying the same
fix to `cg_expr`.

## lz4 4.4.5 smoke test

lz4 compresses data using a hash function (`LZ4_hash4`) that relies on
**32-bit truncating multiplication**: `uint32_t x * 2654435761U` truncates
to 32 bits, yielding hash indices in `[0, 8191]`.

**Before R6:** two independent failures caused the smoke test to SEGV:
1. `always_inline` was discarded → `tableType` could not be constant-folded
   (noted in R5 as the known root cause)
2. `2654435761U` was emitted as `2654435761` (long) → 64-bit multiplication
   → hash indices up to 10^13 → out-of-bounds hash table write

**After R6:** both are fixed.  All four lz4 source files compile through
`sharpc → cc`, link, and the compress/decompress round-trip produces byte-
identical output.

## Files changed in R6

* `sharp-fe/ast.h` — `gcc_attrs char*` field in `func_def` and `var_decl`
* `sharp-fe/ast.c` — `free(gcc_attrs)` in `ast_node_free`
* `sharp-fe/parse.c`
  - `attrs_append_tokens` helper (new)
  - `eat_attribute_specifiers` gains `char **out_attrs` parameter
  - `DeclSpecs.gcc_attrs` field
  - All `parse_decl_specifiers` / `parse_top_decl` / `parse_init_declarator_list` / `parse_struct_def` paths updated
* `sharp-fe/sema.c` — `arith_conv` const-stripping + bounds fix
* `sharp-fe/cg.c`
  - `cg_func`: emit `gcc_attrs` before return type
  - `cg_stmt (DECL_STMT)`: emit `gcc_attrs` after block-scope declarator
  - Pass-1b: emit `gcc_attrs` after file-scope declarator
  - `cg_expr AST_INT_LIT`: full suffix table (was strip-all)
  - `cg_const_expr AST_INT_LIT`: same fix for consistency
* `sharp-fe/c_superset_probes/p73_attribute_passthrough.c` — new probe
* `sharp/build_lz4_with_sharpfe.sh` — new smoke-test build script

Zero changes to `sharp-cpp/`.  Sixth consecutive phase where every fix
lives in `sharp-fe/`.

## Resuming after R6

**R7 (recommended): seventh real-world target.**  Now that attribute
passthrough is working, any target using `always_inline`, `noreturn`,
`visibility`, `cold`, `hot`, or `format` for correctness (not just
performance) will work correctly.  Good candidates:
- **zstd** — uses `__attribute__((noinline))` and `__attribute__((cold))`
  for error paths; `always_inline` on inner loops
- **tinycc** — a C compiler written in C; structurally self-similar to
  sharp-fe itself

**Or R7: integer constant type promotion hardening.**  The `U`-suffix fix
solved the most common case.  Potential follow-on: verify `long` promotion
rules for values between `INT_MAX` and `UINT_MAX` without explicit suffix,
and for signed vs unsigned comparisons that depend on implicit conversion.

— Phase R6, 2026-05-04
