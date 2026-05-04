# Phase S4 — initializers + bit-fields

**Status:** complete and green on all four gates.
**Goal:** add the C constructs needed to land the C-superset acceptance
at 40/41 — only K&R-style functions (`p21`, deferred indefinitely)
remain. Targets: `p23_bitfield`, `p29_init_list`, `p30_struct_init`,
`p31_string_arr`, `p39_compound_lit`, `p40_designated`, `p_brace_init`.
All seven pass.

## Gate results

| Gate | Before S4 | After S4 |
|---|---|---|
| `make test` | 178 / 178 | 187 / 187 |
| `make asan` | clean | clean |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | clean |
| `c_superset_probe.sh` | 33 / 41 | **40 / 41** |

The `test_decls` suite gained 9 cases under `test_s4_initializers`
covering bit-field round-trip, array brace init (block + top-level
const), struct positional init, struct designated init, array
designated init with skipped indices, compound literal, string-as-char-array
init, and 2-D nested init list `int m[2][3]={{...},{...}}`.

## Files changed

### `ast.h`
* `AST_FIELD_DECL` extended with optional `bit_width` AstNode pointer
  (NULL for plain fields, set for bit-fields).
* New AST kinds: `AST_INIT_LIST`, `AST_DESIGNATED_INIT`, `AST_COMPOUND_LIT`.
* New union members:
  * `init_list { items }` — vector of items (each item is an expression
    or an `AST_DESIGNATED_INIT`; lists nest for `{ {1,2}, {3,4} }`)
  * `designated_init { designator_kind (0=field, 1=array), field_name OR
    index_expr, value }` — single-level designators only (`.f` and
    `[idx]`); multi-designator chains like `.a.b[3] = v` are not
    supported but absent from all targeted probes
  * `compound_lit { type, init }` — for `(Type){ init-list }` expressions

### `ast.c`
* `ast_node_free` extended for FIELD_DECL.bit_width and the three new
  node kinds.
* `ast_kind_name` table extended.
* `ast_print` cases for INIT_LIST, DESIGNATED_INIT, COMPOUND_LIT.

### `parse.c`
* New `parse_init_list` (forward-declared near top of file). Consumes
  `{`, then a comma-separated list of items, then `}`. Each item is
  one of:
  * `.IDENT = value` — field designator
  * `[expr] = value` — array designator
  * Bare `{...}` — nested init list (no designator)
  * Bare expression — positional value
  Trailing commas (C99) tolerated.
* Three init-handling sites updated to route `{...}` to
  `parse_init_list`:
  * `parse_init_declarator_list` (block-scope decls)
  * `parse_top_decl` first declarator path
  * `parse_top_decl` additional-declarator path
  These all share the pattern `ps_at(STOK_LBRACE) ? parse_init_list :
  parse_expr_prec(ps,2)` after `=`.
* `parse_primary`'s `(` cast-detection branch now also handles compound
  literals: when `(` `type` `)` is followed by `{`, build
  `AST_COMPOUND_LIT(type, init_list)` and route through `parse_postfix`
  for postfix operators (e.g. `(struct P){...}.field`).
* `parse_struct_def` field branch accepts `: <expr>` after the field
  name (and after any array suffix) for bit-field declarations. The
  width is parsed at `parse_expr_prec(ps, 2)` and recorded on
  `field_decl.bit_width`. The C compiler validates the constraint that
  the width must be an integer constant expression.

### `cg.c`
* `cg_struct` field-emission loop emits `: <expr>` after the declarator
  when `bit_width` is set on the field.
* `cg_expr` gains three new arms:
  * `AST_INIT_LIST` → emits `{ a, b, c }` with each item rendered via
    recursive `cg_expr`. Used both as a top-level initializer value
    and inside a compound literal.
  * `AST_DESIGNATED_INIT` → emits `.field = expr` or `[idx] = expr`.
  * `AST_COMPOUND_LIT` → emits `(Type)<init-list>` (the compiler-side
    cast prefix plus the brace-init from the inner INIT_LIST node).

### `sema.c`
Two relaxations to `assign_compat`:

1. **Value-const transparency.** ISO C 6.3.2.1 says "an rvalue of
   qualified type has the unqualified type". So initializing or returning
   a `const T` from an unqualified `T` (and vice-versa) must succeed
   provided the underlying types match. We added a new branch at the top
   of `assign_compat` that strips top-level `const` from BOTH sides via
   `ty_unconst` and accepts on equality, with the explicit caveat that
   pointer types still go through the existing pointee-qualifier checks
   (so `const int *` → `int *` still correctly rejects). The existing
   "const T → T (drop const)" rule was tightened to apply only to pointer
   types, where the qualifier carries semantic weight.

2. **String-literal-to-char-array.** A `TY_PTR` to char/uchar (the type
   of a string-literal expression) is accepted as the initializer for a
   `TY_ARRAY` of char/uchar. This is the C-language rule allowing
   `char s[] = "abc";` to copy the bytes into the array; the size is
   either explicitly given or deduced by the C compiler from the
   literal's length.

### `test_decls.c`
* New `test_s4_initializers` suite with 9 cases (described under "Gate
  results" above). Wired into `main`.

### `c_superset_probes/p_brace_init.c`
* Pre-existing bug in the probe file: `int main() { return arr[0]; }`
  returns the value 1 (the first init element), but the probe runner
  requires exit-0 for success. Fixed to
  `return (arr[0]+arr[1]+arr[2]) - 6;` — same shape as every other
  probe in the suite.

## Architectural decisions locked in by S4

* **Init list as expression.** `AST_INIT_LIST` lives in the expression
  half of the AST and can appear anywhere a value can. cg renders it as
  a brace-list. This is mildly non-standard (C grammar restricts init
  lists to initializer position and inside compound literals), but the
  parser only routes through `parse_init_list` from the three init
  sites + the compound-literal arm, so the looser AST model never
  produces invalid C.
* **Designators are single-level only.** `.f = v` and `[i] = v` cover
  every probe in the targeted set. Multi-designator (`.a.b[3] = v`) and
  range-designator (`[1...3] = 0`, GCC extension) require either chain
  storage on the AST node or a vector — deferred until a real use case
  appears.
* **No init-list type checking in sema.** The C compiler is the source
  of truth for whether `{1, 2, 3}` matches the declared type. Sharp's
  sema returns the LHS type (or just leaves the init-list untyped) and
  emits straight through. This avoids reimplementing C's complex
  initializer-conversion rules.
* **Bit-field width is stored as an AstNode, not evaluated.** The C
  compiler enforces that the width is a positive integer constant
  expression that fits in the underlying type. Sharp doesn't constant-fold
  in the front end; we trust cc.
* **Compound literal as a separate AST kind.** Reusing `AST_CAST` with
  an init-list operand was tempting but loses information that cg needs
  (a cast emits `(Type)expr` while a compound literal emits
  `(Type){init-list}` — the brace presence matters). The dedicated
  `AST_COMPOUND_LIT` keeps the two paths straightforward.
* **assign_compat: const-as-storage-qualifier.** S4's relaxation aligns
  Sharp's value-const rules with ISO C 6.3.2.1. Pointer pointee-const
  remains strict (which is the safety-relevant case).

## Known limitations carried forward

| Construct | Status | Right substage |
|---|---|---|
| K&R-style functions | not handled (probe `p21`) | indefinitely deferred — Lua doesn't use it |
| Multi-designator `.a.b[3] = v` | not supported | add when a real use case appears |
| Range designator `[1 ... 3] = 0` (GCC ext) | not supported | not in ISO C |
| Inline function definitions for header-style helpers | emits `static inline` | acceptable per S1 design |
| Volatile on pointer pointee (`int * volatile p`) | dropped silently | acceptable |
| `( declarator )` vs `( params )` ambiguity (`int (foo)(int)`) | uses S1 conservative heuristic | needs typedef-name parse-time tracking |

## C-superset acceptance — final tally

```
40 / 41 pass
  fails: p21_kr_func.c
```

Of 41 probes spanning the full ISO C11 surface that Lua 5.4.7 exercises
(plus a handful of additional canaries), 40 round-trip cleanly through
`sharp-fe` to a C output that compiles and runs to the expected exit
code. The single failure is K&R-style function definitions which are
absent from the Lua source.

## What S4 does not do

S4 was the last front-end-only substage. The remaining work is
**S5 — final integration with the Lua testsuite**:
* `build_lua_with_sharp.sh` currently bypasses `sharp-fe` (cpp →
  cc directly). Update it to route every preprocessed `.c` through
  `sharpc` then through cc, and verify the resulting `lua` binary
  passes `lua-5.4.7-tests`.
* Address whatever real-world Lua source patterns surface that the 41
  synthetic probes did not exercise.

## Resuming

Snapshot: `sharp-phase-s4.tar.gz` — the post-S4 green checkpoint.
Earlier snapshots: `sharp-phase-s1.tar.gz`, `sharp-phase-s2.tar.gz`.
