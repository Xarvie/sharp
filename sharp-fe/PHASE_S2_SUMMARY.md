# Phase S2 — typedef + enum + union + switch

**Status:** complete and green on all four gates.
**Goal:** add the four C constructs needed to expand the C-superset
acceptance from 29/41 (post-S1) to 33/41. Probes targeted: `p07_typedef`,
`p08_enum`, `p14_switch`, `p22_union`. All four pass.

## Gate results

| Gate | Before S2 | After S2 |
|---|---|---|
| `make test` | 170 / 170 | 178 / 178 |
| `make asan` | clean | clean |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | clean |
| `c_superset_probe.sh` | 29 / 41 | 33 / 41 |

The `test_decls` suite gained 8 cases under `test_s2_typedef_enum_union_switch`
covering all four constructs: typedef alias arithmetic, typedef of pointer
type, enum with explicit values + bitwise OR, enum with implicit
auto-incremented values, union with `int` and `char[N]` members + field
access, switch with case + default + early return, switch fallback to
default, and an enum-typed function return.

## Files changed

### `ast.h`
* New AST kinds: `AST_ENUM_DEF`, `AST_ENUMERATOR`, `AST_SWITCH`, `AST_CASE`,
  `AST_DEFAULT`.
* `AST_STRUCT_DEF` gains a `bool is_union` — same node represents both
  `struct Name { ... }` and `union Name { ... }` since field-access
  semantics are identical at the AST level.
* New union members: `enum_def {name, items}`, `enumerator {name,
  value}`, `switch_ {cond, body}`, `case_ {value}` (DEFAULT carries no
  payload).

### `ast.c`
* `ast_node_free`, `ast_kind_name`, `ast_print` extended for the five new
  node kinds.

### `parse.c`
* `is_type_start` extended to also recognise `STOK_UNION` and `STOK_ENUM`
  as legitimate decl-start tokens; without this, `union U u;` inside a
  function body was misparsed as an expression.
* `tspec_try_consume` handles `union Tag` and `enum Tag` symmetrically with
  `struct Tag` (one merged switch arm covering all three keyword forms).
  Generic args (`Tag<T,U>`) are accepted only after `struct` since enum
  and union do not participate in Sharp's generic-type system.
* New `parse_enum_def` consumes `enum [Tag] { NAME [= expr], ... };`
  with C99-style trailing-comma tolerance. Anonymous enums (no tag)
  are accepted.
* `parse_struct_def` now also handles `union Name { ... }` — same
  parser, with the keyword inferred from the lookahead. The parsed
  `is_union` flag is recorded on the produced `AST_STRUCT_DEF`.
* `parse_struct_def`'s field branch now accepts array suffixes
  (`char c[4];`, multi-dimensional too) by sharing `parse_array_suffix`
  with the older variable-decl path. A forward declaration was added
  near the top of `parse.c` because `parse_struct_def` lives above
  `parse_array_suffix`.
* `parse_top_decl` routes:
  * `STOK_ENUM` followed by `{` (or `IDENT {`) → `parse_enum_def`;
    other shapes (`enum Tag x;`) fall through to the general declarator
    path.
  * `STOK_UNION` followed by `IDENT {`/`IDENT ;` (or no IDENT) →
    `parse_struct_def`.
* `parse_stmt` gains three branches:
  * `STOK_SWITCH`: builds `AST_SWITCH(cond, body)`. Body is parsed as a
    statement (typically a brace-block); permissive to match C grammar.
  * `STOK_CASE`: builds `AST_CASE(value)` from `parse_expr_prec(ps, 2)`
    then `:`. The label is its own statement; the next-statement is
    parsed by the surrounding block loop. GCC range-cases not supported.
  * `STOK_DEFAULT`: builds bare `AST_DEFAULT` after consuming `:`.

### `scope.c`
* New `case AST_ENUM_DEF` in the top-level decl loop:
  * If the enum has a tag, register it as `SYM_TYPE` so `enum Tag` is a
    valid type-specifier.
  * Each enumerator is registered as `SYM_VAR` (decl pointing at the
    `AST_ENUMERATOR`) so bare references like `return RED;` resolve.
* Union tags work without change because `AST_STRUCT_DEF` is the
  unified node kind.

### `type.c`
* `ty_from_ast` for `AST_TYPE_NAME` now distinguishes three kinds of
  `SYM_TYPE`:
  * `AST_TYPEDEF_DECL` → recurse into the typedef's target type
    (transparent alias). Already done in S1's typedef fix.
  * `AST_ENUM_DEF` → returns `ty_int(ts)`. Sharp does not give enums
    their own `Type*` kind (the C compiler still sees the original
    `enum Tag` keyword via the round-tripped definition; Sharp's
    type-system view is just int).
  * `AST_STRUCT_DEF` (struct or union) → `ty_struct_type`. The
    `is_union` bit lives on the underlying decl; cg looks it up there.

### `sema.c`
* `sema_expr` for `AST_IDENT`: when `SYM_VAR` resolves to an
  `AST_ENUMERATOR` (as opposed to an `AST_VAR_DECL`), return `ty_int`
  immediately. Without this guard the next branch would dereference
  `decl->u.var_decl.type` on a wrongly-tagged union.

### `cg.c`
* Top-level pass 1 (`cg_file`) now also emits `AST_ENUM_DEF` in full
  (`enum [Tag] { NAME [= value], ... };`) and chooses `typedef union`
  vs `typedef struct` for the forward typedef line based on `is_union`.
  Enums must appear before any code that references their constants;
  pass 1 sits before functions, so the ordering is correct.
* `cg_struct` switches the keyword from `struct` to `union` when
  `sd->u.struct_def.is_union` is true. The `cg_type` emission of
  `TY_STRUCT` is unchanged because the `typedef union T T;` /
  `typedef struct T T;` forward decls let `T` alone resolve correctly
  in either case.
* `cg_stmt` gains:
  * `AST_SWITCH`: emits `switch (cond) { ... }`. Body is rendered as a
    block (or wrapped in synthetic braces if the AST body is a single
    statement, matching the for-loop pattern in this file).
  * `AST_CASE`: emits `case <value>:` followed by a newline. Indentation
    follows the surrounding block.
  * `AST_DEFAULT`: emits `default:` followed by a newline.

### `test_decls.c`
* New `test_s2_typedef_enum_union_switch` suite with 8 cases (described
  under "Gate results" above). Wired into `main`.

## Architectural decisions locked in by S2

* **Same AST node for struct and union.** A `bool is_union` on
  `AST_STRUCT_DEF` is enough — field/method semantics are identical, only
  the C keyword and storage layout differ. cg.c picks the keyword;
  layout is the C compiler's job.
* **Enums collapse to int at the Type\* layer.** Sharp's type system
  doesn't get a dedicated `TY_ENUM` kind. The C compiler sees the
  original `enum Tag` keyword via the round-tripped enum definition
  and applies its usual rules. This avoids two different "is this an
  integer" predicates and keeps `arith_conv` simple.
* **Enumerators are `SYM_VAR`.** Same lookup machinery as variables;
  the resolver in `sema_expr` peeks at `decl->kind` to short-circuit to
  `ty_int` instead of trying to dereference `var_decl.type`. This keeps
  the symbol-table polymorphism narrow.
* **`case`/`default` are statements.** They live in the block as
  ordinary statements interleaved with other statements. cg renders each
  by emitting the bare label; the next statement (whatever it is) is
  rendered separately by the block walker. This matches C's grammar
  exactly and avoids a special "switch-body" node kind.
* **Field array suffixes go through `parse_array_suffix`.** Same helper
  as the legacy variable-decl path. Sharing the helper means
  `field_decl.type` carries the same `AST_TYPE_ARRAY` shape that other
  array-typed declarations use, which the existing `cg_decl` array path
  already handles correctly.

## Known limitations carried forward

| Construct | Status | Right substage |
|---|---|---|
| K&R-style functions | not handled (probe `p21`) | not in scope; rare and Lua doesn't use it |
| Bit-fields (`unsigned int x : 3;`) | not handled (probe `p23`) | S4 (struct/union initializers and layout) |
| Initializer lists `int v[3] = {1,2,3};` and braced struct init | not handled (probes p29/p30/p_brace_init) | S4 |
| String-literal as char-array initializer (`char s[] = "hi";`) | not handled (probe p31) | S4 |
| Designated initializers `(.field = v)` | not handled (probe p40) | S4 |
| Compound literals `(struct T){...}` | not handled (probe p39) | S4 |
| Enum-tag use in declarator beyond the simple `enum T x;` form | not exercised | S3/S4 |
| `typedef` defining a struct body inline (`typedef struct {...} T;`) | parses (typedef sees the struct as base type) but the anonymous struct path needs verification | S3 |

## What S2 does not do

S2 deliberately stops before:
* Initializer lists (S4) — `{1,2,3}`, `(.x=…)`, `(struct T){...}`, string
  literals as char-array initializers.
* Bit-fields (S4) — `unsigned int x : 3;`.
* Typedef-name resolution at parse time (S3) — currently `( declarator )`
  vs `( params )` in declarators uses the conservative S1 heuristic. With
  parse-time typedef knowledge, `int (foo)(int)` could be disambiguated
  per the C standard.
* K&R-style function definitions (deferred; Lua doesn't use them).

## Resuming

The next substage is **S4 — initializers + bit-fields**. (S3 — typedef-name
parse-time resolution — is small and can be folded into S4 or done as a
prelude; the eight remaining failing probes are all S4-shaped except
`p21_kr_func`.)

Targets:
* `p23_bitfield` — `struct S { unsigned x : 3, y : 5; };` Need bit-field
  syntax in `parse_struct_def` field branch (post-IDENT `:` integer-expr),
  AST representation (extend `AST_FIELD_DECL` with bit-width), and cg
  emission.
* `p29_init_list` — `int v[3] = {1, 2, 3};` Need `AST_INIT_LIST` (or
  reuse `AST_STRUCT_LIT`?) and parse_expr_prec entry point for `{`.
* `p30_struct_init`, `p_brace_init` — same braced-init machinery for
  struct/union types.
* `p31_string_arr` — `char s[] = "hello";` lets the array deduce its
  size from the string literal length.
* `p39_compound_lit`, `p40_designated` — `(struct T){...}` and
  `.field = expr` syntax.

Snapshot: `sharp-phase-s1.tar.gz` is the S1 green checkpoint. After
verifying S2 here, snapshot to `sharp-phase-s2.tar.gz`.
