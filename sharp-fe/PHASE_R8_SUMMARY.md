# Phase R8 — Brotli 1.1.0 Integration + 4 C-Superset Fixes

**Status:** Complete on all three quality gates.  Brotli 1.1.0 smoke test passes.

## Gate results

| Gate | Before R8 | After R8 |
|---|---|---|
| `make test` (probes) | 69 / 69 | **69 / 69** |
| `make asan` | clean | **clean** |
| `make strict` | clean | **clean** |
| `c_superset_probe.sh` | 73 / 74 | **76 / 77** (p77/p78/p79 added) |
| brotli 1.1.0 smoke test | — (new target) | **✅ PASS** (31/31 files) |
| zstd 0.23.0 smoke test | ✅ PASS | **✅ PASS** (regression check) |

## Target: Brotli 1.1.0 (23K lines, 31 C files)

Brotli is a multi-file compression library (encoder + decoder) that exercises
patterns distinct from the single-file amalgamations used by previous targets:
- Anonymous unions inside struct bodies (`ZopfliCostModel` with union {a; b;})
- Opaque forward typedef pattern (`typedef struct FooStruct Foo;`)
- `sizeof expr` without parentheses (in `platform.h`)
- Heavy use of `static inline` across multiple files

### Compile & smoke test

31 C files compiled through `sharpc → cc → link → run`.  The smoke test
compresses 145 bytes and decompresses back, verifying byte-for-byte identity.

## Four fixes

### Fix 1 — `sizeof expr` without parentheses (parse.c)

`sizeof t` (no parentheses) is valid ISO C when the operand is an expression
(§6.5.3.4); parentheses are only required for type operands.  Brotli's
`platform.h` uses `sizeof t` where `t` is a local variable of type
`uint16_t`, `uint32_t`, or `uint64_t`.

**Fix:** in `parse_primary` `STOK_SIZEOF`, if the next token is NOT `(`,
parse an expression at unary precedence (prec=14) and return the sizeof
node immediately.  The existing parenthesised path is unchanged.
Locked by probe `p77_sizeof_noparen.c`.

### Fix 2 — Anonymous union/struct member injection (scope.c + cg.c)

C §6.7.2.1¶15: members of an anonymous struct or union are injected into
the containing struct's namespace.  `ZopfliCostModel` in Brotli uses:

```c
typedef struct ZopfliCostModel {
  ...
  union {
    size_t literal_histograms[768];
    ZopfliCostModelArena arena;
  };
} ZopfliCostModel;
```

Two components required a fix:

**scope.c `build_struct`:** when a field's name starts with `__anon_` (the
synthesised name for anonymous aggregates from parse.c), look up the inner
struct definition in file scope and inject each of its fields into the outer
struct scope with `scope_define(SYM_FIELD, sub_name, sub_fd, diags)`.  This
makes `sema` resolve `self->arena` as a valid field access.

**cg.c `cg_field_decl_from_ast`:** fields named `__anon_field_N` are now
emitted as inline anonymous bodies (`union { ... };`) rather than named
declarators.  C anonymous union semantics require BOTH no tag and no
declarator name; a field `union __anon_struct_N field_name;` would NOT be
anonymous.  The fix: look up the inner struct definition in `file_ast`,
emit its fields inline between `union {` and `};\n`.

**cg.c `file_ast` initialisation:** moved `ctx->file_ast = file` to before
Pass 0 (previously it was after Pass 2) so that `cg_field_decl_from_ast`
has access to the file AST when emitting struct bodies.
Locked by probe `p78_anon_union.c`.

### Fix 3 — Opaque forward typedef struct registration (scope.c + cg.c)

`typedef struct BrotliEncoderPreparedDictionaryStruct BrotliEncoderPreparedDictionary;`
— a typedef where the struct tag and the typedef alias have different names.
`BrotliEncoderPreparedDictionaryStruct` has no body; it's an opaque type used
only as a pointer.  Two problems:

**scope.c `build_file` pass 1:** when an `AST_TYPEDEF_DECL` with target
`AST_TYPE_NAME("Foo")` (where `Foo ≠ alias`) is encountered and `Foo` is not
yet in scope, synthesise a no-body `AST_STRUCT_DEF` for `Foo` and register
it as `SYM_TYPE`.  Guard: skip if `Foo` is a built-in type name (char, int,
long, unsigned char, signed short, etc.).  The guard uses an explicit allowlist
of all multi-word primitive names to avoid emitting `typedef struct signed char
signed char;`.

**cg.c pass 1:** when an `AST_TYPEDEF_DECL` with `Foo ≠ alias` is encountered
and `Foo` resolves to a no-body struct, emit `typedef struct Foo Foo;` in pass 1
(BEFORE function forward declarations in pass 1c).  Without this, function
declarations using `Foo *` appeared before the `Foo` type was declared.

**cg.c pass 2:** same check to emit `typedef struct Foo Foo;` in case it was
missed in pass 1 (defensive redundancy).
Locked by probe `p79_typedef_opaque_struct.c`.

### Fix 4 — Function pointer callee type resolution (sema.c)

Calling a variable that holds a function pointer (`cmp(ctx, ptr, grpEnd)`)
where `cmp` is declared as `int (*cmp)(...)` returned `ty_error` because
`sema_call` only checked `callee_t->kind == TY_FUNC` but not the
`TY_PTR(TY_FUNC(...))` case.  This caused spurious "comparison of non-scalar
types" cascade errors.

**Fix (sema.c `sema_call`):** after the direct `TY_FUNC` check, strip
`TY_CONST` and one layer of `TY_PTR`; if the inner type is `TY_FUNC`,
return its return type.

## New probes

| Probe | What it tests |
|---|---|
| `p77_sizeof_noparen.c` | `sizeof expr` without parentheses |
| `p78_anon_union.c` | Anonymous union member injection |
| `p79_typedef_opaque_struct.c` | `typedef struct Foo Bar;` opaque pointer |

## Files changed in R8

* `sharp-fe/parse.c` — sizeof without parens
* `sharp-fe/scope.c`
  - `build_struct`: anonymous union member injection
  - `build_file`: opaque forward struct tag registration for typedef
* `sharp-fe/cg.c`
  - `file_ast` set before pass 0
  - `cg_field_decl_from_ast`: anonymous union body inlining
  - Pass 1: opaque typedef struct forward declaration
  - Pass 2: same, defensive
* `sharp-fe/sema.c` — function pointer callee return type
* `sharp-fe/c_superset_probes/` — new p77, p78, p79
* `sharp/build_brotli_with_sharpfe.sh` — new build script

## Validated real-world targets (cumulative)

Lua 5.4.7 ✅, cJSON 1.7.18 ✅, picol ✅, stb_image ✅, zlib 1.3.1 ✅,
lz4 4.4.5 ✅, zstd 0.23.0 ✅, **brotli 1.1.0 ✅**

## Resuming after R8

**R9 candidates:**

- **Ninth real-world target** — tinycc, sqlite amalgamation, or libpng
- **`eval_array_size` enum-without-explicit-value** — enumerator counting
- **`__attribute__((vector_size(N)))` as type** — enables zstd without
  `-DZSTD_NO_INTRINSICS`
- **`offsetof` in constant expressions** — needed for some array size calculations

— Phase R8, 2026-05-05
