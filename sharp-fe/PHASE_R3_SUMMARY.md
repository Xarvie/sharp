# Phase R3 — fifth real-world target (zlib 1.3.1) + sqlite-amalgamation surface validation

**Status:** complete and green on all gates.
**Targets:**
- **zlib 1.3.1** (Mark Adler & Jean-loup Gailly's compression library,
  ~22K lines across 11 .c files) — full end-to-end integration:
  byte-for-byte match with cc-only baseline.
- **sqlite-amalgamation 3.46.1** (the official single-file build of
  SQLite, ~258K lines) — surface validation: six distinct deep
  parse/sema/cg bugs surfaced and fixed; the full pipeline through
  the entire 110K-line preprocessed unit is left for a future phase
  because of two GCC builtins (`__builtin_va_arg` with a type
  argument, and `__atomic_*` family) that need significant
  feature work to support natively.

R3 added two distinct dimensions of value: **another fully-validated
end-to-end target** (zlib, byte-perfect) and **substantial surface
hardening** under sqlite's stress that benefits every future target.

## Gate results

| Gate | Before R3 | After R3 |
|---|---|---|
| `make test` | 197 / 197 | **203 / 203** (probes 69 + integration 40 + import 25 + decls 69) |
| `make asan` | clean | **clean** |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | **clean** |
| `c_superset_probe.sh` | 58 / 59 | **66 / 67** (p61, p62, p63, p64, p66, p67, p68, p69 added; only deferred p21_kr_func still fails) |
| `build_lua_with_sharpfe.sh` | `final OK !!!` | **`final OK !!!`** (no regression) |
| `build_cjson_with_sharpfe.sh` | byte-for-byte match | **byte-for-byte match** (no regression) |
| `build_picol_with_sharpfe.sh` | byte-for-byte match | **byte-for-byte match** (no regression) |
| `build_stb_image_with_sharpfe.sh` | byte-for-byte match | **byte-for-byte match** (no regression) |
| `build_zlib_with_sharpfe.sh` | n/a | **byte-for-byte match** (new target — 11/11 .c files round-trip) |

`test_decls` gained 6 cases under `test_r3_sqlite_zlib_regressions`,
exercised end-to-end on every `make test` invocation.

## Two-track delivery

### Track 1 — zlib end-to-end (the new validated target)

zlib 1.3.1 across 11 source files exercises mature ANSI/ISO C90
patterns: huffman tables in static const arrays, complex bit
manipulation, longjmp-free error returns, and a particular
fondness for `if (...) statement;` and `do statement; while(...);`
forms (the latter heavily relied on by inflate's symbol-copy loops,
which the R2 do-while fix made tractable).

`build_zlib_with_sharpfe.sh` exercises the canonical compress
→ uncompress → memcmp round-trip on a 122-byte test string.
Output diffs zero bytes against the cc-only baseline.

**Zero new bugs were specific to zlib** — every fix below was
surfaced first by sqlite, but zlib's 11-file build serves as the
integration witness that none of the changes regressed working
codepaths.

### Track 2 — sqlite-amalgamation surface validation

sqlite3.c is the largest single C translation unit reasonably
available (258K lines pre-cpp, ~110K post-cpp).  It exercises
patterns no previous target had: pervasive abstract function-pointer
dispatch tables, anonymous bit-fields for explicit padding, K&R-era
typedef-as-function-type idioms, and combinations of GCC builtins.

R3 surfaces and fixes six distinct bugs from this corpus.  Two
deeper limitations (covered below in *Known limitations carried
forward*) keep full sqlite end-to-end out of scope for R3 but the
entire fix set is clean; sqlite's parse path now reaches well past
10K lines into the preprocessed source before the GCC-builtin
walls.

## Eight bugs fixed (six surfaced from sqlite, two from the cg fallout)

### Gap 1 — `IDENT (` not recognised as type-start (parse.c is_type_start)

sqlite declares syscall and IO-method tables shaped like

```c
sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char*);
```

— typedef-name as the return type of a function-pointer field.
The leading IDENT is a typedef; the next token is `(` (opening the
`(*name)` declarator).  `is_type_start` accepted IDENT followed by
IDENT, `*`, `<`, `)`, `operator`, but not `(`.  Without that
recognition the field declaration was skipped with "unexpected
token in struct body".

**Fix:** add `IDENT (` gated by `td_has_n` (typedef-set probe),
mirroring the existing `IDENT )` cast-detection pattern.

Locked in by **`p61_typedef_fnptr_field.c`** + a `test_r3_*` regression.

### Gap 2 — bare attribute spec as a statement (parse.c parse_stmt)

sqlite's bytecode VM dispatch uses GCC's fallthrough marker:

```c
case 12: op = 9+1; __attribute__((fallthrough));
case 7:
```

This `__attribute__((...));` is a bare statement-position
attribute (no following declaration).  Pre-R3 parse_stmt routed
STOK_ATTRIBUTE into the declaration path, where parse_decl_specifiers
ate the attribute and then complained "missing type specifier in
declaration" because nothing followed.

**Fix:** parse_stmt special-cases STOK_ATTRIBUTE: eat the attribute
spec, eat the trailing `;`, return NULL (the block walker filters
NULL).  If something other than `;` follows, fall through to the
declaration path (decl-with-leading-attribute is still valid).

Locked in by **`p63_attribute_stmt.c`** + a `test_r3_*` regression.

### Gap 3 — anonymous bit-field for padding (parse.c + scope.c)

ISO C99 §6.7.2.1¶12 admits `int :32;` (no name, just a width) for
explicit struct padding / alignment.  sqlite uses this in its
vmprintf state struct.

Pre-R3 had two failures:
1. Parse rejected with "field declaration is missing a name".
2. Even when patched, multiple `int :N;` fields collided in
   `build_struct` because all carried name=`""`.

**Fixes:**
- `parse.c`: when `parse_declarator` returns no name AND next
  token is `:`, synthesize an empty name (cg_decl already supports
  empty-name emission as "type only").
- `scope.c` `build_struct`: skip `scope_define` for empty-name
  fields (they're not name-accessible, so no symbol entry needed).

Locked in by **`p62_anon_bitfield.c`** + a `test_r3_*` regression.

### Gap 4 — postfix type qualifier `T const *` (parse.c apply_suffix + is_type_start)

ISO C99 §6.7.3 permits qualifiers before OR after the type
specifier.  sqlite uses `unsigned char const *` extensively in its
UTF-8 helpers and `sqlite3_io_methods const *` in vfs structures.

Pre-R3 had two failures:
1. **parse_type's `apply_suffix`**: only consumed `*` and qualifiers
   *attached to* pointers; the postfix qualifier on the *base type*
   itself (between the type-name and the `*`) was unhandled.
2. **`is_type_start`**: gated `IDENT IDENT/*/(/)/<` but not
   `IDENT const/volatile/restrict`, so `(unsigned char const *)x`
   in expression position was rolled back to "not a cast".

**Fixes:**
- `parse_type`: at the apply_suffix label, eat any postfix const /
  volatile / restrict attached to the base type before the pointer
  loop.  Wrap the type accordingly.
- `is_type_start`: add IDENT followed by const/volatile/restrict,
  gated by `td_has_n` (mirrors `IDENT (` and `IDENT )`).

Locked in by **`p64_postfix_const.c`** + a `test_r3_*` regression.

### Gap 5 — abstract function-pointer cast (parse.c apply_suffix + cg.c TY_PTR)

sqlite's syscall override table is invoked through casts of the
form

```c
((int (*)(int, uid_t, gid_t)) aSyscall[20].pCurrent)(fd, u, g);
```

The cast type is an *abstract* declarator: `(*)(args)` with no
name, the `*` parenthesised to bind it under the function suffix.

Pre-R3 parse_type's apply_suffix handled `*name` and `*const` but
not the parenthesised abstract `(*)(args)` shape.  Even after
parse, `cg_type` for TY_PTR(TY_FUNC) emitted just `func *` — losing
the parameter list and producing invalid C.

**Fix (parse):** at apply_suffix, peek for `(` followed by `*`.
If so, consume the inner `*+` (with optional pointer-side
qualifiers), expect `)`, and look for `(args)` for the function
suffix.  Build TYPE_FUNC(ret=base, params), then wrap with
`nstars` TYPE_PTR layers.

**Fix (cg):** `cg_type` for TY_PTR detects when it peels through
PTR layers down to TY_FUNC and emits the abstract form
`ret (*+)(params)` — same shape `cg_decl` has long emitted for
named declarators (factored slightly differently to omit the name).

Locked in by **`p66_abstract_fnptr_cast.c`** + a `test_r3_*` regression.

### Gap 6 — tentative definitions (scope.c scope_define)

ISO C99 §6.9.2 admits a "tentative definition": a file-scope
declaration with no initialiser.  Multiple tentatives merge, and a
later definition with an initialiser (or end-of-translation-unit,
which then synthesises `= 0`) supersedes them.

sqlite uses this idiom for trace flags / config globals:

```c
static u32 sqlite3WhereTrace;        /* tentative */
/* ... 1500 lines later ... */
static u32 sqlite3WhereTrace = 0;    /* real def */
```

Pre-R3 scope_define accepted `extern T x; T x = init;` (the
external-linkage variant) but not the static / no-extern case.

**Fix:** extend the existing var-decl-collision logic to promote
the new decl whenever it has an initialiser and the existing decl
does not, regardless of storage class.  Mirror the inverse case
(real def already seen, later tentative is a no-op).

Locked in by **`p67_tentative_def.c`** + a `test_r3_*` regression.

### Gap 7 — function returning function pointer (cg.c cg_func) — *fallout from Gap 5*

The R3 cg_type fix for TY_PTR(TY_FUNC) immediately broke Lua's
end-to-end pipeline: Lua's `lua_atpanic` is declared as a function
*returning* a function pointer:

```c
extern int (*lua_atpanic(lua_State *L, int (*panicf)(lua_State *)))
                        (lua_State *);
```

ISO C requires the *nested* declarator form here (the function
name with its outer-args goes INSIDE the `(*...)` of the
return-type's pointer declarator).  Pre-R3 cg's free-function
emission called `cg_type(ret_t); cg_puts(" "); cg_puts(name); …`
— which now produced `int (*)(lua_State *) lua_atpanic(...)`,
flat and rejected by cc.

**Fix:** in cg_func, peel the return type through TY_PTR layers;
if it lands on TY_FUNC, emit the nested form

```
inner_ret  ` (`  N stars  name(outer_args)  `)`  `(inner_args)`
```

instead of the flat form.  Other ret-types use the unchanged
`cg_type(ret_t) name(args)` shape.

Locked in by **`p68_fn_returning_fnptr.c`** + Lua's existing
TESTSUITE.

### Gap 8 — array of function pointers (cg.c cg_decl) — *fallout from Gap 5*

Lua's lualib.c registers built-in module loaders via

```c
static const lua_CFunction searchers[] = { searcher_preload, ... };
```

where `lua_CFunction` is `int (*)(lua_State *)` and the array's
const decorates the pointer (not the pointee).  ISO C requires
the nested form `int (* const name[])(args)`.  Pre-R3 cg_decl's
array branch emitted `cg_type(elem) name[]` — which now
produced `int (*)(int) const name[]`, double-broken (flat
fnptr + misplaced const).

**Fix:** in cg_decl's TY_ARRAY arm, peel one optional TY_CONST
and any number of TY_PTR layers from the element type; if it
lands on TY_FUNC, emit the nested form

```
inner_ret  ` (`  N stars  ` const`?  ` `?  name[N][M]…  `)`  `(inner_args)`
```

with the const placement matching ISO C's pointer-side qualifier
syntax.

Locked in by **`p69_fnptr_array.c`** + Lua's existing TESTSUITE.

## Bug count by phase

| Phase | Targets | LOC delta | Fixes |
|---|---|---|---|
| S5  | Lua 5.4.7 | ~30K | 2 |
| R1  | cJSON 1.7.18 | ~2K | 4 |
| R2  | picol + stb_image | ~10.8K | 4 (picol = 0, stb = 4) |
| R3  | zlib 1.3.1 + sqlite surface | ~22K + 258K | **8** (sqlite-induced 6 + cg fallout 2) |

R3 is the largest fix-count phase to date.  The bug count per
target *isn't* monotonically decreasing — sqlite's surface is
larger than every previous target combined and surfaces patterns
no previous target touched.  zlib gained 0 bugs of its own, which
is the same convergence signal picol gave in R2: when the surface
is well-covered by prior fixes, a new target rides through.

## Files changed in R3

* `sharp-fe/parse.c`
  - `is_type_start`: IDENT '(' typedef gate (Gap 1)
  - `is_type_start`: IDENT const/volatile/restrict typedef gate (Gap 4b)
  - `parse_stmt`: bare attribute spec as statement (Gap 2)
  - `parse_field` (struct body): empty-name synthesis for anonymous
    bit-field when next-tok is `:` (Gap 3a)
  - `parse_type` `apply_suffix`: postfix const/volatile/restrict
    consumption (Gap 4a)
  - `parse_type` `apply_suffix`: abstract `(*)` declarator + function
    suffix (Gap 5a)
  - forward declaration of `parse_param_list_inner` (needed by Gap 5a)
* `sharp-fe/cg.c`
  - `cg_type` TY_PTR: detect PTR-to-FUNC, emit `ret (*)(args)`
    abstract form (Gap 5b)
  - `cg_func` (free function): nested declarator for function
    returning fn-ptr (Gap 7)
  - `cg_decl` TY_ARRAY: nested declarator for array of fn-ptr,
    with optional element-side const placement (Gap 8)
* `sharp-fe/scope.c`
  - `scope_define`: tentative-def + real-def-with-init promotion
    regardless of storage class (Gap 6)
  - `build_struct`: skip empty-name field registration (Gap 3b)
* `sharp-fe/test_decls.c`
  - new `test_r3_sqlite_zlib_regressions` (6 cases)
* `sharp-fe/c_superset_probes/`
  - new: `p61_typedef_fnptr_field.c`, `p62_anon_bitfield.c`,
    `p63_attribute_stmt.c`, `p64_postfix_const.c`,
    `p66_abstract_fnptr_cast.c`, `p67_tentative_def.c`,
    `p68_fn_returning_fnptr.c`, `p69_fnptr_array.c`
* `sharp/build_zlib_with_sharpfe.sh` — new (R3 deliverable)

R3 touched **no files in `sharp-cpp/`** — second consecutive phase
where every fix lives in `sharp-fe/`, reflecting a stable cpp / fe
boundary.

## Architectural decisions locked in by R3

* **Type-start recognition for IDENT + qualifier/parenthesis is
  guarded by `td_has_n`.**  The pre-R3 corpus only used `IDENT
  IDENT` and `IDENT *` patterns, both unconditional.  R3 adds
  `IDENT (`, `IDENT const`, `IDENT volatile`, `IDENT restrict` —
  all of which can also occur in expression contexts (function
  call, post-decrement, etc.).  The typedef-set probe (already
  used for `IDENT )`) is the right discriminator and turns out to
  be cheap because `td_has_n` is a small linear scan.

* **Tentative definitions are promoted regardless of storage
  class.**  C99 §6.9.2 actually constrains this to *external*
  linkage (extern or no specifier), but in practice file-scope
  `static T x;` followed by `static T x = init;` is universally
  treated as one variable by all extant C compilers.  Sharp-fe
  follows reality, not the strict letter.

* **Function-pointer emission shapes are positional.**  ISO C's
  declarator grammar requires nesting: a function returning a
  fn-ptr nests the function name inside the fn-ptr's parens; an
  array of fn-ptr nests the array suffix inside.  Pre-R3 cg's
  cg_type was *flat* for these shapes (it called itself
  recursively), which is fine when there's no name to interleave
  but wrong when there is.  R3 introduces three nested-form
  emission sites — `cg_type` (no name, abstract / cast form),
  `cg_func` (with function name and outer args), `cg_decl` array
  branch (with array name and array suffixes) — each handling its
  own placement.  Future emission paths that combine PTR(FUNC)
  with names should follow the same pattern.

* **Anonymous fields aren't symbols.**  A field with an empty
  name (anonymous bit-field, anonymous struct/union member)
  occupies storage but has no symbol-table entry — it's not
  reachable by name.  scope.c respects that; cg-side emission
  already handled empty names correctly.

## Known limitations carried forward

| Construct | Status | Notes |
|---|---|---|
| K&R-style function definitions | not handled (probe `p21`) | indefinitely deferred; no observed real-world use |
| GCC `__builtin_va_arg(ap, T)` | not handled | T is a *type* argument — needs special-cased parser + sema. sqlite's vmprintf machinery uses this; bypass requires re-engineering stdarg expansion |
| GCC `__atomic_*` family | not handled | `__atomic_load_n`, `__atomic_store_n`, etc. are GCC builtins.  Some uses bypassable via `-DSQLITE_THREADSAFE=0`; others appear to leak through |
| GCC `__int128` 128-bit integer | not handled at parse | bypassed via `-D__int128=long -D__signed__=signed` workaround.  Only reaches us via two unused typedefs in sqlite, so the workaround is benign |
| GCC `__attribute__((vector_size(N)))` as cast type | not handled | bypassed via `-DSTBI_NO_SIMD` for stb (R2 deferral) |
| GCC statement-expression `({...})` | not handled | bypassed via `-DNDEBUG` for stb (R2 deferral) |
| Multi-designator `.a.b[3] = v` initialiser | not handled | unchanged from S4 |

The four `__builtin_*` / `__attribute__` items above are
substantial GCC-extension features.  Each is large enough to
deserve its own phase; together they would unlock the remainder
of sqlite (and likely zstd, libpng, and other production C codebases)
without further parse/sema work.

## Resuming after R3

The validated-target list is now five codebases:

1. **Lua 5.4.7** — interpreter with extensive GCC extensions
2. **cJSON 1.7.18** — pure ISO C99, OOP-style with const discipline
3. **picol** — dense idiomatic C interpreter (TCL)
4. **stb_image / stb_image_write** — header-only, macro-heavy
5. **zlib 1.3.1** — multi-file ANSI/ISO C90 compression library

A natural next phase is **R4**, choosing among:

- **GCC builtins + atomics** (the deferred items above).  A
  focused phase to add `__builtin_va_arg` (with type argument),
  `__atomic_*`, and statement-expressions would unlock sqlite
  end-to-end and likely several others.
- **Sixth real-world target** — candidates include
  - `tinycc` (Fabrice Bellard's tiny C compiler — structurally
    self-similar to sharp-fe, like mcpp)
  - `lz4` (small fast LZ77 compressor — different shape from zlib)
  - `mcpp` (a C preprocessor — would round-trip self-similar code)
  - `nuklear` (immediate-mode GUI — single header, ~25K lines)
- **D1 — dogfooding** — start translating sharp-fe modules
  themselves into Sharp dialect.

The build scripts at the repo root (`build_*_with_sharpfe.sh`)
form a regression harness that any future change must keep green.
After R3 there are five such scripts; adding a sixth target only
requires writing one more.
