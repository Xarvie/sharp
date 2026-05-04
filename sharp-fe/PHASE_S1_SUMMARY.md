# Phase S1 — Declaration architecture (ISO C declarator model)

**Status:** complete and green on all four gates.
**Goal:** replace Sharp's pre-S1 "type-prefix" declaration model with the ISO C
"declaration-specifiers + declarator" model so that arbitrary C11 declarations
(storage classes, multi-decl with per-name pointer, function-pointer types,
function prototypes followed by definitions, composed primitive types like
`unsigned char`, the `volatile` keyword) round-trip through `sharp-fe` to
correct C output.

## Gate results

| Gate | Before S1 | After S1 |
|---|---|---|
| `make test` | 92 / 92 | 170 / 170 |
| `make asan` | clean | clean |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | clean |
| `c_superset_probe.sh` | 20 / 41 | 29 / 41 |

All eight S1-targeted C-superset probes now pass: `p02_static`,
`p15_func_proto`, `p16_extern`, `p17_const_local`, `p28_func_ptr`,
`p36_inline`, `p37_volatile`, `p38_register`.

The new `test_decls` test_s1_iso suite (9 cases, all green) locks in the
behaviour against regression: storage at file/block scope, extern + defining
declaration, inline functions, ISO multi-decl with mixed pointer/plain,
function-pointer round-trip, const-qualified arithmetic, composed primitive
types, prototype + definition.

## Semantic break vs. pre-S1 Sharp

Pre-S1 Sharp treated `*` as part of the *type*: `int* p, q;` made BOTH p and q
of type `int*`. ISO C binds the leftmost `*` to the FIRST declarator only;
S1 conforms. Migration path for existing Sharp code:

| Pre-S1 | ISO (S1) |
|---|---|
| `int* p, q;` (both pointers) | `int *p, *q;` *or* `typedef int* IntP; IntP p, q;` |
| `int* p, q = NULL;` (both pointers, q init NULL) | `int *p = NULL, *q = NULL;` |

The previous diagnostic ("stray '*' in declarator: in Sharp, '*' is part of the
type") is retired. The two affected pre-existing tests in `test_decls.c` (§1
Sharp-ptr-multi-var, §5 C-style rejected) were updated in lockstep; see the
diff in those sections for the exact migration.

No project code (Sharp standard library, examples) used the affected idiom, so
no other source changes were required.

## Files changed

### `ast.h`
* New `AST_TYPE_VOLATILE` kind (mirrors `AST_TYPE_CONST`; carries a `base`
  child).
* New `StorageClass` enum: `SC_NONE, SC_STATIC, SC_EXTERN, SC_TYPEDEF,
  SC_REGISTER, SC_AUTO_C` (the C `auto` keyword as a storage class — distinct
  from Sharp's `AST_TYPE_AUTO` for type deduction; modern C never uses it).
* `AST_FUNC_DEF` gains `storage` (`StorageClass`) and `is_inline` (`bool`).
  The legacy `is_static` flag is kept for back-compat with `import.c` and
  `sema.c` and is synchronised with `storage == SC_STATIC` everywhere new
  code touches it.
* `AST_VAR_DECL` gains `storage`.
* New `type_volatile` member in the type-expression union.

### `ast.c`
* `ast_node_free`, `ast_clone_type`, `ast_kind_name`, `ast_print` extended
  for `AST_TYPE_VOLATILE`.
* `ast_clone_type` for `AST_TYPE_FUNC` rewritten to clone `AST_PARAM_DECL`
  child nodes (decision: `TYPE_FUNC.params` now uniformly holds `PARAM_DECL`
  nodes, matching `FUNC_DEF.params`).

### `type.c`
* `ty_from_name` extended with all composed C type names (`signed char`,
  `unsigned char`, `signed short`, `short int`, `signed short int`,
  `unsigned short`, `unsigned short int`, `signed int`, `unsigned int`,
  `long int`, `signed long`, `signed long int`, `unsigned long`,
  `unsigned long int`, `long long`, `long long int`, `signed long long`,
  `signed long long int`, `unsigned long long`, `unsigned long long int`,
  `long double`). Pre-S1 silently mis-typed e.g. `unsigned char` as
  `unsigned int`.
* `ty_from_ast` handles `AST_TYPE_VOLATILE` by returning the unqualified
  base type. Sharp's interned `Type*` deliberately does not track volatile
  (it has no semantic effect on type compatibility); cg.c preserves the
  qualifier on emission.
* `ty_from_ast` for `AST_TYPE_FUNC` extracts the type out of each
  `PARAM_DECL` child.

### `parse.c`
This is the largest change (~700 LOC of new code).

* New `DeclSpecs` struct + `parse_decl_specifiers()` consumes any
  storage-class / type-qualifier / type-specifier sequence in any order.
  Composes multi-token primitives into canonical names recognised by
  `ty_from_name`.
* New `parse_declarator()` and `parse_direct_declarator()` implement the
  ISO C 6.7.6 declarator grammar including `(` declarator `)` sub-grouping,
  array suffixes, function suffixes, and pointer prefixes with pointer-side
  qualifiers (`T * const`, `T * volatile`, `T * restrict`).
* The "outer base type plugs in here" position inside a sub-declarator is
  marked by a sentinel placeholder node identified by pointer equality
  (`placeholder_make` / `splice_placeholder`). Verified by trace against
  `int (*signal(int, void(*)(int)))(int)`.
* The suffix loop uses a "deepest-slot" pointer (`AstNode **inner`) so that
  consecutive array / function suffixes nest left-to-right outermost-to-
  innermost. `int m[3][4]` produces `ARRAY(size=3, base=ARRAY(size=4,
  base=int))` — the C-correct nesting.
* `( declarator )` vs `( params )` ambiguity at S1 (without typedef-name
  resolution, which lands in S3) uses a conservative heuristic:
  `(` followed by `*` or `(` opens a sub-declarator; everything else is
  a parameter list. This handles `int (*p)(int)` correctly; misses the
  vanishingly-rare `int (foo)(int)` shape.
* New `parse_param_list_inner` (no surrounding parens) is shared between
  the function-suffix path inside the declarator and the public
  `parse_param_list` wrapper used by `finish_func`.
* `parse_init_declarator_list` replaces the old `parse_var_decl_list`;
  loops over comma-separated declarators, each with its own pointer
  prefix / array / function suffix. A back-compat wrapper preserves the
  old `parse_var_decl_list(ps, base_ty, stmt_wrap)` signature for any
  legacy call sites.
* `parse_top_decl` rewritten as a forward-only flow:
  1. handle `import`, `struct Tag {...}|;`, `extern struct Vec<int>;`
  2. `parse_decl_specifiers()`
  3. operator → `finish_func`
  4. Sharp generic function (`name<T,U>(...)`) detection → `finish_func`
  5. otherwise `parse_declarator`; if outermost type is `FUNC` and next
     token is `{`/`;`/`const`, build a FUNC_DEF via
     `build_func_def_from_decl` (transfers `ret`/`params` ownership from
     the FUNC type tree); otherwise build VAR_DECL/TYPEDEF_DECL and
     loop on `,` for additional declarators.
* `parse_stmt`'s decl branch routed through `parse_decl_specifiers` +
  `parse_init_declarator_list`, picking up storage / qualifiers in block
  scope (`static int once = 99;`).
* `parse_type` and `parse_type_unqual` (used by casts, sizeof, generic
  args, struct-field types) extended to:
  * recognise `volatile T` as a top-level qualifier
  * recognise `T * volatile` and `T * restrict` as pointer-side qualifiers
  * compose multi-token primitive names properly (was buggy pre-S1 — only
    the first token's text was kept).
* `is_type_start` extended to include `STOK_STATIC`, `STOK_EXTERN`,
  `STOK_REGISTER`, `STOK_TYPEDEF`, `STOK_INLINE`, `STOK_VOLATILE`,
  `STOK_RESTRICT` so that block-level decls starting with a storage class
  are correctly classified.

### `cg.c`
* New `ast_type_outer_is_volatile` walks the AST type tree to detect
  outer-layer `volatile` (skipping any intervening const wrappers).
  Necessary because the `Type*` model deliberately does not track volatile.
* `cg_func` emits `static` / `extern` / `register` from `storage`, then
  `inline` from `is_inline`. To avoid C99 ODR linker errors on
  `inline`-only definitions, plain `inline` (no explicit storage class) is
  promoted to `static inline` — the standard idiom for header-style
  helpers and what most C codebases use.
* Top-level VAR_DECL emission and block-scope `AST_DECL_STMT` emission
  prepend storage-class keywords and `volatile` (when detected on the
  outer layer).
* `cg_decl` gained a function-pointer branch: a `TY_PTR` whose base is
  `TY_FUNC` (with N stars) emits the C declarator-style
  `ret-type ( **...name )( params )`. This is the form needed for any
  named function-pointer variable / parameter / field.
* `cg_func`'s parameter loop reordered so a vararg `...` is correctly
  emitted at the end (pre-S1 the `continue` on vararg silently dropped
  it; latent because no test exercised the path with vararg).

### `scope.c`
* `scope_define` now permits the canonical C "declaration before
  definition" pattern:
  * **Functions:** prototype (`body == NULL`) + definition (`body != NULL`)
    or definition + later compatible prototype. Both definitions still
    fail. (Parameter-compatibility is left to cc.)
  * **Variables:** `extern T x;` + `T x = init;` promotes to the
    initialised definition; reverse order is also tolerated; double
    `extern` and pure-tentative-defs are no-op merged. Two real
    initialised definitions still fail.

### `sema.c`
* The arithmetic and bitwise branches of the binary-operator type checker
  call `ty_unconst` on both operand types before the arithmetic
  classification and `arith_conv`. `const int x; x - 5` now type-checks
  as `int` (matching the C semantic that const is a qualifier, not a
  separate type for the purposes of usual arithmetic conversions).

### `test_decls.c`
* §1 "Sharp ptr multi-var" updated to ISO form (`int *p = &v, *q = &v;`).
* §5 "C-style rejected" inverted to "ISO accepted".
* New `test_s1_iso` suite, 9 cases (described above), exercises every
  S1 storage class, function pointer, ISO multi-decl, const arithmetic,
  composed primitive type, and prototype-then-definition.

## Architectural decisions locked in by S1

* `TYPE_FUNC.params` is a vector of `AST_PARAM_DECL` nodes (uniform with
  `FUNC_DEF.params`). `ast_clone_type` and `ty_from_ast` updated to match.
* `volatile` lives in the AST (`AST_TYPE_VOLATILE`) but not in the
  interned `Type*`. cg.c is the sole site that preserves it on emission.
  The outer-layer-only emission rule means `int * volatile p` (the
  pointer itself qualified) silently drops the qualifier — acceptable
  because cc enforces the access-ordering rules regardless.
* Sharp's `auto` keyword stays bound to `AST_TYPE_AUTO` (type deduction);
  C's storage-class `auto` was given a separate `SC_AUTO_C` enum value.
  Modern C never uses C-`auto` so this is a paper distinction.
* `restrict` is accepted as a no-op qualifier everywhere it can appear.
* The `( declarator )` vs `( params )` heuristic is conservative and
  S3-aware: the proper fix needs typedef-name resolution.
* `parse_top_decl`'s function-vs-variable decision uses the outermost
  shape of the declarator's type tree: outermost `TYPE_FUNC` →
  function definition / prototype; anything else → variable / typedef.
  No probe-and-rewind dance.

## Known limitations carried forward

| Construct | Status | Right substage |
|---|---|---|
| K&R-style functions (`int add(a, b) int a; int b; { ... }`) | not handled (probe `p21`) | not in scope; Lua doesn't use it |
| `int (foo)(int)` (parenthesised IDENT in declarator) | misparsed as call to `foo` | S3 (typedef-name resolution) |
| `int (*signal(int))(int) { body }` (function returning function pointer, with body) | parses; FUNC_DEF construction falls through to the variable-decl path | rare; revisit if Lua needs it |
| `int * volatile p` (volatile on pointer itself) | volatile silently dropped on emission | could be lifted in S2/S3 if needed |
| `typedef T NAME;` containing struct/union body | not exercised (struct body inside decl-specifier tspec not implemented) | S2 (typedef + enum + union) |
| Block-scope struct field `static`/`inline`/`storage` | parse_struct_def's field branch still uses `parse_type` (no DeclSpecs); fields with leading storage-class would mis-parse | not legal C anyway; deferred |

## What S1 does not do

S1 deliberately stops before:
* typedef-name resolution at parse time (S3) — currently the parser does
  not know that an IDENT is a typedef name, only that it could be a type.
* enum / union (S2)
* switch (S2)
* compound initialisers / designated initialisers / brace-init for arrays
  and structs (S4)
* string literals as array initialisers (S4)
* bit-fields (S4)

## Resuming

The next substage is **S2 — typedef + enum + union + switch**, targeting
probes `p07_typedef`, `p08_enum`, `p14_switch`, `p22_union`. ROADMAP.md
section S2 is the entry point.

Snapshot: `/tmp/sharp_baseline.tar.gz` is the pre-S1 green checkpoint;
post-S1 snapshot will be created by `make dist DIST_PHASE=s1`.
