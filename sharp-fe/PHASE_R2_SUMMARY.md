# Phase R2 — third + fourth real-world target integration (picol, stb_image)

**Status:** complete and green on all gates.
**Targets:** picol 2.x (Salvatore Sanfilippo's ~800-line TCL
interpreter) and stb_image / stb_image_write (Sean Barrett's
header-only image libraries, ~10K lines combined).
**Goal:** continue compounding real-world coverage past Lua (S5) and
cJSON (R1).  Each new target with a finite fix list moves the
project from "implementation in progress" into "implementation in
service".

## Gate results

| Gate | Before R2 | After R2 |
|---|---|---|
| `make test` | 193 / 193 | **197 / 197** (probes 69 + integration 40 + import 25 + decls 63) |
| `make asan` | clean | **clean** |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | **clean** |
| `c_superset_probe.sh` | 54 / 55 | **58 / 59** (p55, p58, p59, p60 added; only deferred p21_kr_func still fails) |
| `build_lua_with_sharpfe.sh` | `final OK !!!` | **`final OK !!!`** (no regression) |
| `build_cjson_with_sharpfe.sh` | byte-for-byte match | **byte-for-byte match** (no regression) |
| `build_picol_with_sharpfe.sh` | n/a | **byte-for-byte match** (new target) |
| `build_stb_image_with_sharpfe.sh` | n/a | **byte-for-byte match** (new target) |

`test_decls` gained 4 cases under `test_r2_stb_image_regressions`;
the fixes are exercised end-to-end (cpp + fe + cc + execute) on
every `make test` invocation.

## Targets

### picol — *zero new bugs*

picol is a single-file ~800-line TCL interpreter in dense, idiomatic
C.  S5 + R1 had cleared enough of the surface that picol round-tripped
through the pipeline without changes.  This is the first useful
data point that sharp-fe is converging: a third arbitrary-shape C
codebase compiled and ran identically to its cc-only baseline on
the very first attempt.

`build_picol_with_sharpfe.sh` exercises a small TCL program covering
arithmetic, `proc` definitions, recursive calls (factorials),
`while` loops, and command substitution — exactly the surface a
picol user would touch.  Output diffs zero bytes against the cc-only
build.

### stb_image / stb_image_write — *four new bugs surfaced and fixed*

`stb_driver.c` (a 60-line round-trip test: build a 16×16 RGB
gradient, encode to in-memory PNG via stb_image_write, decode via
stb_image, verify pixel-perfect) compiles to ~12K lines after cpp
expansion.  This was the deepest probe yet of sharp-fe — every step
of zlib decompression and PNG chunk parsing must be semantically
identical to what cc produces.  Four distinct, deep bugs surfaced:

#### Gap 1 — `_Thread_local` C11 storage-class

stb declares its global error-string holder as

```c
static _Thread_local const char *stbi__g_failure_reason;
```

`_Thread_local` is a C11 storage-class specifier orthogonal to
`static` / `extern` (per ISO 6.7.1, it may combine with either).
Sharp-fe's `StorageClass` enum was single-valued, so before R2
parse rejected the keyword with "missing type specifier" and the
declaration was dropped.

**Fix:** carry `_Thread_local` as a separate boolean flag
(`is_thread_local`) on `DeclSpecs`, `AST_VAR_DECL`, and
`AST_FUNC_DEF`.  `parse_decl_specifiers` accepts the keyword and
sets the flag; cg emits `_Thread_local ` after the storage-class
keyword.  Three emission sites in `cg.c` (block-scope var decl,
top-level var decl, function decl).

Locked in by **`p55_thread_local.c`** and a `test_r2_*` regression.

#### Gap 2 — K&R-style function-type typedef

stb declares its callback type as

```c
typedef void stbi_write_func(void *context, void *data, int size);
```

ISO C admits two distinct typedef shapes for callback types:
function-pointer (`typedef void (*FnPtr)(int);`) and function-type
(`typedef void Fn(int);`).  cJSON used the first form throughout;
stb uses the second.  Both denote "pointer to a void(int) function"
when used through `Fn *` / `FnPtr`.

**Bug:** `parse_top_decl` had a generic test "if outermost type is
FUNC and next token is `;`, this is a function prototype" that
fired on the K&R typedef form too — it never registered `Fn` as a
typedef, and every later `Fn *p` failed with "unknown type 'Fn'".

**Fix:** add `ds.storage != SC_TYPEDEF` to that condition.  The
typedef path then falls through to `parse_init_declarator_list`,
which registers the alias correctly.  One-line guard, deep impact.

Locked in by **`p58_fn_type_typedef.c`** and a `test_r2_*` regression.

#### Gap 3 — Block-scope anonymous enum

stb_driver.c uses

```c
int main(void) {
    enum { W = 16, H = 16, C = 3 };
    unsigned char src[W * H * C];
    ...
}
```

— the standard idiom for keeping magic numbers near their usage
site, with the enumerators injected into the enclosing scope as
int constants.

**Three independent bugs combined** to make this fail:

1. **scope.c — `build_stmt` had no `AST_ENUM_DEF` case.**  Top-level
   enum was registered (lines 528–540), but block-scope wasn't.
   Sema then reported "undefined name 'W'".

2. **cg.c — `cg_stmt` had no `AST_ENUM_DEF` case.**  The default arm
   emitted `/* unsupported stmt kind 6 */`, so cc never saw the
   enum body even when sema accepted it.

3. **parse.c — `parse_enum_def` ate the trailing `;`.**  Top-level
   `enum { … };` requires a semicolon, but the ps_match was
   indiscriminate.  In stmt context this consumed the `;` after
   the enum body, after which the surrounding
   `parse_decl_specifiers` saw the *next* statement's `int x = …`
   and treated it as a continuation of the same declaration.  The
   AST got reordered so that `int x` was emitted BEFORE the enum
   in the regenerated C — even after fixing scope and cg, the use
   site preceded the definition and cc rejected the result.

**Fixes:**
- scope.c: replicate the top-level enum-registration block in
  `build_stmt`.
- cg.c: add an `AST_ENUM_DEF` case to `cg_stmt` that mirrors the
  top-level emission (header + indented enumerators + `};`).
- parse.c: remove the `ps_match(STOK_SEMI)` from `parse_enum_def`;
  let each call site own its own `;` (the stmt path through
  `parse_init_declarator_list` already does, the typedef path
  doesn't expect one, and the top-level path now calls
  `ps_expect(STOK_SEMI)` explicitly after `parse_enum_def`).

Locked in by **`p59_block_scope_enum.c`** and a `test_r2_*` regression.

#### Gap 4 — `do BODY while (COND);` with unbraced single-statement body

stb_image's zlib decoder writes the inner copy loop as

```c
if (dist == 1) {
    stbi_uc v = *p;
    if (len) { do *zout++ = v; while (--len); }
}
```

— a perfectly valid ISO-C `do STATEMENT while (COND);` where
STATEMENT is the unbraced expression-statement `*zout++ = v;`.
Before R2, sharp-fe's `cg_stmt` for `AST_DO_WHILE` unconditionally
called `cg_block(stmt->u.do_while.body)`.  When body was actually
an `AST_EXPR_STMT`, that call dereferenced the wrong union arm and
produced

```c
do {
} while ((--len));
```

— body silently dropped.  Compilation succeeded; runtime decoded
gibberish ("bad dist", a zlib LZ77 distance-out-of-range).

**Fix:** dispatch on `body->kind == AST_BLOCK`, mirroring the
existing logic in if/while/for/switch:
- BLOCK body → `cg_block(body)`
- anything else → emit `{`, recurse with `cg_stmt`, emit `}`.

Six lines.  This was the most frustrating bug of R2 because the
runtime symptom was so far removed from the source (zlib header
checksum verification fails because the decoded byte stream is
corrupt because a copy loop ran without copying because a code-gen
asymmetry only kicks in for unbraced do-while bodies).

Locked in by **`p60_do_while_single_stmt.c`** and a `test_r2_*` regression.

## Probe corpus growth

| # | name | language | covers |
|---|---|---|---|
| p52 | `anon_typedef_arr.c` | C | (R1) typedef + array decay through `->` |
| p53 | `const_ptr_param.c` | C | (R1) `T * const` parameter |
| p54 | `c_null_ident.c` | C | (R1) `null` as a C identifier |
| **p55** | `thread_local.c` | C | (R2) C11 `_Thread_local` + `static` |
| **p58** | `fn_type_typedef.c` | C | (R2) `typedef void Fn(int);` K&R form |
| **p59** | `block_scope_enum.c` | C | (R2) anonymous enum at function scope |
| **p60** | `do_while_single_stmt.c` | C | (R2) `do EXPR; while (COND);` |

p56 / p57 were used during diagnosis to disambiguate the function-
typedef bug (verifying `s.func(...)` and `s->func(...)` already
worked); they did not become permanent probes because the deeper
typedef-recognition issue was the actual root cause.

## Files changed in R2

* `sharp-fe/ast.h`
  - `AST_FUNC_DEF`: new `bool is_thread_local`
  - `AST_VAR_DECL`: new `bool is_thread_local`
* `sharp-fe/parse.c`
  - `DeclSpecs`: new `bool is_thread_local`
  - `parse_decl_specifiers`: accept `STOK__THREAD_LOCAL`, set flag
  - 5 AST construction sites (3 in func_def builder paths, 2 in
    var_decl builder paths) propagate `ds.is_thread_local` to the
    AST node
  - `parse_top_decl`: function-prototype branch gains
    `ds.storage != SC_TYPEDEF` guard
  - `parse_top_decl` enum branch: explicit `ps_expect(STOK_SEMI)`
    after `parse_enum_def`
  - `parse_enum_def`: removes `ps_match(STOK_SEMI)`
* `sharp-fe/cg.c`
  - `cg_stmt` block-scope var-decl: emit `_Thread_local`
  - `cg_stmt` AST_FUNC_DEF emission (top-level): emit `_Thread_local`
  - Pass-1b global var emission: emit `_Thread_local`
  - `cg_stmt` AST_DO_WHILE: dispatch on `body->kind == AST_BLOCK`
  - `cg_stmt` AST_ENUM_DEF: new case (block-scope enum emission)
* `sharp-fe/scope.c`
  - `build_stmt`: new AST_ENUM_DEF case (registers enumerators in
    enclosing scope)
* `sharp-fe/test_decls.c`
  - new `test_r2_stb_image_regressions` (4 cases)
  - main calls it
* `sharp-fe/c_superset_probes/`
  - new: `p55_thread_local.c`, `p58_fn_type_typedef.c`,
    `p59_block_scope_enum.c`, `p60_do_while_single_stmt.c`
* `sharp/build_picol_with_sharpfe.sh` — new (R2 deliverable)
* `sharp/build_stb_image_with_sharpfe.sh` — new (R2 deliverable)

R2 touched no files outside `sharp-fe/` and `sharp/`.  Crucially,
zero changes to `sharp-cpp/`: every fix in R2 is a sharp-fe
correction.  This is the first phase since the cpp / fe split where
that's true, and reflects the maturity of sharp-cpp.

## Architectural decisions locked in by R2

* **`_Thread_local` carrier is a separate flag, not an enum entry.**
  Per ISO C11 `_Thread_local` is orthogonal to `static`/`extern` (it
  may combine with either).  Modelling it as a `SC_THREAD_LOCAL`
  variant of the existing `StorageClass` enum would have lost that
  combination.  A separate `bool is_thread_local` is the right
  shape; it parallels `is_inline` (also orthogonal to storage class).

* **Trailing `;` belongs to the surrounding declaration, not to
  `parse_enum_def`.**  Parser helpers should consume their own
  syntax and stop, leaving the surrounding context to consume its
  own punctuation.  The `ps_match(STOK_SEMI)` in `parse_enum_def`
  was a convenience that turned into a footgun the moment a third
  call site appeared.  Now every call site explicitly handles `;`.

* **cg code paths for control-flow body must dispatch on `AST_BLOCK`
  vs other.**  The if/while/for/switch arms had this dispatch from
  S2; AST_DO_WHILE was the outlier (probably because
  `do { ... } while (...);` is the more common written form).
  The R2 fix not only repairs do-while but documents the pattern
  for any future control-flow construct.

* **Block-scope anonymous enum is a real-world pattern.**  Outside
  of `enum { … } x;` (typedef-like usage), it appears in production
  code as a way to keep magic numbers close to their usage.  S2's
  enum support was complete *for top-level definitions*; R2 closes
  the parallel block-scope path through scope.c, parse.c and cg.c.

* **R-series targets are open-ended but converging.**  Each new
  target's bug count: Lua = 2 (S5: labels-as-values + float
  precision), cJSON = 4 (R1: lex-mode + GCC float macros + array
  decay + const placement), picol = 0, stb_image = 4.  The bug
  count per target is bounded and decreases as the surface
  saturates.  Two consecutive targets pass first try would suggest
  saturation; for now picol's 0-bug result is suggestive but not
  conclusive.

## Known limitations carried forward

| Construct | Status | Notes |
|---|---|---|
| K&R-style function definitions (`int f(a) int a; { ... }`) | not handled (probe `p21`) | indefinitely deferred; no observed real-world use |
| GCC `__attribute__((vector_size(N)))` as cast type | not handled | bypassed via `-DSTBI_NO_SIMD` for stb_image; same pattern likely needed for any SSE/NEON intrinsic header |
| GCC statement-expression `({ stmts; expr; })` | not handled | bypassed via `-DNDEBUG` for stb_image (glibc's `assert()` macro uses this); will surface in any code that uses `assert` without NDEBUG |
| Multi-designator `.a.b[3] = v` initialiser | not handled | unchanged from S4 |

The two GCC-extension limitations (vector_size, statement-expression)
are deliberately not addressed in R2; they're substantial features
in their own right and stb_image builds cleanly without them.  A
future phase could add either or both.

## Resuming after R2

The validated-target list is now four codebases of distinctly
different shapes:

1. **Lua 5.4.7** — interpreter with extensive GCC extensions
2. **cJSON 1.7.18** — pure ISO C99, OOP-style with const discipline
3. **picol** — dense idiomatic C interpreter
4. **stb_image / stb_image_write** — header-only library, macro-heavy

A natural next phase is **R3** — pick a fifth codebase that
exercises a still-different surface.  High-leverage candidates from
ROADMAP § 8:

- **sqlite-amalgamation** — single 250K-line `.c`, exercises the
  largest surface available short of Linux kernel; will likely
  surface several new bugs.
- **mcpp** — itself a C preprocessor; structurally novel because
  it would round-trip self-similar code.
- **zlib** — LZ77/Huffman code is already exercised through stb;
  doing zlib standalone would confirm the zlib-class fixes are
  durable.

Or **D1 — dogfooding** — start translating sharp-fe modules
themselves into Sharp dialect, using the round-tripped C as the
build artefact.  This is the natural test for "is sharp-fe ready
to compile sharp-fe"; if so the project is bootstrapping.

The build scripts (`build_*_with_sharpfe.sh`) at the repo root form
a regression harness that any future change must keep green.
Adding a fifth target only requires writing one more such script.
