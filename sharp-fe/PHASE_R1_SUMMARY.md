# Phase R1 — second real-world target (cJSON 1.7.18) integration

**Status:** complete and green on all four gates.
**Goal:** route cJSON's source through `.c → sharp-cpp → sharp-fe → C → cc → binary`,
link the supplied `test.c` driver, and have its output match the
cc-only baseline byte-for-byte.  This is the second real-world
target after Lua 5.4.7 (S5), chosen for surface-syntax variety:
cJSON is pure ISO C99 with heavy `const` discipline and anonymous-
tagged typedefs — patterns Lua's source did not exercise.

## Gate results

| Gate | Before R1 | After R1 |
|---|---|---|
| `make test` | 190 / 190 | **193 / 193** (probes 69 + integration 40 + import 25 + decls 59) |
| `make asan` | clean | **clean** |
| `make strict` (`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`) | clean | **clean** |
| `c_superset_probe.sh` | 51 / 52 | **54 / 55** (3 new probes — p52, p53, p54 — all pass) |
| `build_lua_with_sharpfe.sh` | `final OK !!!` | **`final OK !!!`** (no regression) |
| `build_cjson_with_sharpfe.sh` | n/a (script did not exist) | **output matches cc-only baseline byte-for-byte** |

`test_decls` gained 3 cases under `test_r1_cjson_regressions`
covering const-pointer parameters, typedef-through-array `->`
access, and null-pointer comparison phrasing safe in either lex
mode.

## Three real-world gaps surfaced

Routing cJSON's two compilation units (`cJSON.c`, `test.c`) through
sharpc for the first time exposed three issues the Lua corpus had
not touched.  All three are clean fixes; none required a hack.

### Gap 1 — Sharp keywords leak into C parsing

cJSON 1.7.18 line 6360:

```c
cJSON *null = cJSON_CreateNull();
if (...) { cJSON_Delete(null); }
```

`null` is a perfectly valid C identifier.  Sharp reserves it as
the null-pointer-constant keyword (`STOK_NULL`).  Before R1, lex
unconditionally promoted it (along with `defer`, `import`,
`operator`, `this`) regardless of the source language.

#### Fix

1. **`lex.h`/`lex.c`**: `lex_run` gained a `bool sharp_keywords`
   parameter.  `keyword_kind` now takes the same flag and
   short-circuits the five Sharp-only keyword entries when
   `sharp_kw == false`, returning `STOK_IDENT` for them.
2. **`sharpc.c`**: extension-based mode detection — `.sp` → Sharp
   keywords on, anything else (`.c`, `.i`, `.h`) → off.  This
   matches how the cpp side implicitly distinguishes input
   languages.
3. **`import.c`**: `parse_sp_file` is only called for `.sp`
   imports — passes `true` unconditionally.
4. **All test fixtures** use Sharp dialect (`defer`, `this`,
   `operator+` appear throughout) and pass `true` explicitly.
5. **Probe runner** (`c_superset_probe.sh`): now globs both
   `p*.c` and `p*.sp` so Sharp-dialect probes (such as
   `p50_if_has_operator.sp`, renamed from `.c`) continue to be
   exercised through sharpc with the right mode.

| File | Change |
|---|---|
| `lex.h`           | `lex_run` signature gains `bool sharp_keywords`; doc explains the flag and the cJSON example |
| `lex.c`           | `keyword_kind` takes flag; gate STOK_DEFER/IMPORT/NULL/OPERATOR/THIS behind it |
| `sharpc.c`        | Extension detection picks the flag |
| `import.c`        | Pass `true` (only .sp ever flows here) |
| `test_decls.c`, `test_import.c`, `test_integration.c` | Pass `true` (their fixtures use Sharp dialect) |
| `c_superset_probe.sh` | Glob both `.c` and `.sp` |
| `c_superset_probes/p50_if_has_operator.c` → `.sp` | Renamed: it tested Sharp features (`operator+`, `@has_operator`), not C |
| `c_superset_probes/p54_c_null_ident.c` | New: minimal probe locking in `null` as a C identifier |

### Gap 2 — Missing GCC float-range predefined macros

cJSON 1.7.18 line 4823:

```c
return (fabs(a - b) <= maxVal * __DBL_EPSILON__);
```

`__DBL_EPSILON__` and the rest of the `__{FLT,DBL,LDBL}_*__` value
macros (`EPSILON`, `MAX`, `MIN`, `DENORM_MIN`, `NORM_MAX`,
`HAS_DENORM`) are gcc's predefined identifiers — gcc injects them
without an explicit `<float.h>` include.  Sharp-cpp had the
*shape* macros (`__DBL_DIG__`, `__DBL_MANT_DIG__`, etc.) but not
the *value* macros.

#### Fix

Added the missing predefines to `sharp-cpp/sharp-cpp/cpp_targets.c`
(the per-target macro registration table).  Values are copied
verbatim from `gcc -E -dM -x c /dev/null` output on the same
target (x86_64-linux-gnu, 80-bit `long double`).  18 new
definitions in three blocks (FLT / DBL / LDBL).

This is the only file outside `sharp-fe/` that R1 touched.

### Gap 3 — TY_ARRAY decay missing in the field-access path

cJSON 1.7.18 line 5474 (and dozens more like it):

```c
printbuffer buffer[1];
buffer->buffer = (unsigned char*) hooks->allocate(default_buffer_size);
```

`buffer` is a 1-element array; in expression position it decays
to a pointer to its element type, after which `->field` is the
ordinary struct field access.  sharp-fe rejected this with `no
member 'buffer' in struct`.

The bug was in `sema_field_access_expr` → `struct_scope_of`: the
helper stripped `TY_CONST` and `TY_PTR` to get to the underlying
`TY_STRUCT` but did not strip `TY_ARRAY`.  When the receiver was
`pair[N]`, the function returned NULL and the field lookup
failed.

Reduced to **`p52_anon_typedef_arr.c`**:

```c
typedef struct { int x; int y; } pair;
int main(void) {
    pair p[1];
    p->x = 3;     /* used to fail */
    p->y = 4;
    return p->x + p->y - 7;
}
```

#### Fix

`struct_scope_of` (`sema.c`) gained a `TY_ARRAY` strip mirroring
the existing `TY_PTR` strip:

```c
if (t && t->kind == TY_ARRAY) {
    t = t->u.array.base;
    if (t && t->kind == TY_CONST) t = t->u.const_.base;
}
```

The parallel peel logic in `sema_field_access_expr`'s
generic-substitution block (`base_t2 = …`) gained the same
`TY_ARRAY` strip so the right struct decl is found for any
substitution.

### Gap 4 — `cg_type` placed `const` on the wrong side of a pointer

After Gap 3, fe accepted cJSON's source but cc rejected the
regenerated C with `error: assignment of member 'valuedouble' in
read-only object`.  Inspection of `cJSON.fec.c` showed the
function signature had been re-emitted as

```c
static int parse_number(const cJSON * item, ...);
```

while the source said

```c
static cJSON_bool parse_number(cJSON * const item, ...);
```

`T * const p` (const **pointer** to mutable T) had been re-emitted
as `const T *p` (pointer to const T).  Two structurally distinct
types; cc rightly rejects body writes through the latter.

The bug was in `cg.c`'s `cg_type` arm for `TY_CONST`:

```c
case TY_CONST:
    cg_puts(ctx, "const ");      /* always left-side */
    cg_type(ctx, t->u.const_.base);
    break;
```

For `TY_CONST(TY_PTR(T))` (the parser's correct AST for
`T * const`), this produced `const ` + `T *` = `const T *` —
flipping the semantics.

#### Fix

```c
case TY_CONST: {
    Type *inner = t->u.const_.base;
    if (inner && inner->kind == TY_PTR) {
        cg_type(ctx, inner);     /* "T *" */
        cg_puts(ctx, " const");  /* → "T * const" */
    } else {
        cg_puts(ctx, "const ");
        cg_type(ctx, inner);
    }
    break;
}
```

Cases covered:

| Input AST | Old emission | New emission | Meaning |
|---|---|---|---|
| `TY_CONST(int)` | `const int` | `const int` | const int (unchanged) |
| `TY_PTR(TY_CONST(int))` | `const int *` | `const int *` | pointer to const int (TY_PTR arm, unchanged) |
| `TY_CONST(TY_PTR(int))` | `const int *` ✗ | `int * const` ✓ | const pointer to int |
| `TY_CONST(TY_PTR(TY_CONST(int)))` | `const int *` (lossy ✗) | `const int * const` ✓ | const pointer to const int |
| `TY_CONST(TY_PTR(TY_CONST(TY_PTR(int))))` | n/a | `int * const * const` ✓ | const ptr to const ptr to int |

Reduced to **`p53_const_ptr_param.c`** — passes round-trip.

## Files changed

* `sharp-fe/lex.h`, `sharp-fe/lex.c` — `lex_run` accepts `bool sharp_keywords`; `keyword_kind` gates Sharp-only keywords behind the flag.
* `sharp-fe/sharpc.c` — extension-based mode detection (`.sp` → Sharp, else C).
* `sharp-fe/import.c` — pass `true` (only handles `.sp`).
* `sharp-fe/test_decls.c`, `test_import.c`, `test_integration.c` — pass `true` for Sharp-dialect fixtures; new `test_r1_cjson_regressions` suite (3 cases).
* `sharp-fe/sema.c` — `struct_scope_of` strips `TY_ARRAY`; the parallel peel in `sema_field_access_expr` does the same.
* `sharp-fe/cg.c` — `TY_CONST` arm distinguishes "wraps pointer" (emit `T * const` on the right) vs "wraps non-pointer" (emit `const T` on the left).
* `sharp-fe/c_superset_probe.sh` — globs both `.c` and `.sp` probes.
* `sharp-fe/c_superset_probes/p50_if_has_operator.c` → `.sp` — corrected categorisation; this probe tests Sharp dialect features.
* `sharp-fe/c_superset_probes/p52_anon_typedef_arr.c` — new (typedef + arr + `->`).
* `sharp-fe/c_superset_probes/p53_const_ptr_param.c` — new (`T * const` parameter).
* `sharp-fe/c_superset_probes/p54_c_null_ident.c` — new (`null` as a C identifier).
* `sharp-cpp/sharp-cpp/cpp_targets.c` — added 18 IEEE-754 value macros (`__{FLT,DBL,LDBL}_{EPSILON,MAX,MIN,DENORM_MIN,NORM_MAX,HAS_DENORM}__`).
* `sharp/build_cjson_with_sharpfe.sh` — new top-level driver script (sister to `build_lua_with_sharpfe.sh`); auto-downloads cJSON 1.7.18 and runs the full pipeline ending in a byte-for-byte diff against the cc-only baseline.

## Architectural decisions locked in by R1

* **Lex mode is positional / per-input**, not global state.  A
  multi-file build can mix `.sp` and `.c` translation units and
  each one gets the appropriate keyword set.  cpp side stays in
  `CPP_LANG_SHARP` for everything (it works fine for C and is
  a backwards-compat decision: nothing C-specific in cpp depends
  on language mode for these inputs).
* **GCC predefined macros are part of the target ABI**, not of
  the language standard.  Sharp-cpp owns the canonical list, and
  it should grow when real-world C code reveals an omission.
  Future targets (ARM, RISC-V, …) will need their own value
  macros; the existing structure of `cpp_install_target_macros`
  per-arch already accommodates that.
* **TY_ARRAY decay rules are sema's responsibility**, not cg's.
  The cg side already handled the array→pointer fallback in
  `cg_type` (line 184), but sema must decay too — otherwise the
  type lookup fails before cg ever runs.  R1's change extends
  the existing `TY_PTR / TY_CONST` strip pattern in
  `struct_scope_of`; future array-related sema fixes should
  follow the same shape.
* **`const` on a pointer is positional in C**, and cg must
  preserve which side of the pointer the `const` is on.  AST
  fidelity (parser builds `TY_CONST(TY_PTR(T))` vs
  `TY_PTR(TY_CONST(T))` correctly) was already right; the
  emission needed the matching distinction.  No new AST or type
  shape — purely a cg-side fix.
* **Probe corpus is bilingual.**  `c_superset_probes/` now
  contains both `.c` (C) and `.sp` (Sharp) probes; the runner
  treats them uniformly because sharpc auto-detects mode by
  extension.  This means probes are organised by *what they
  test* (numbered by chronology of discovery), not by *what
  language they're written in*.

## Known limitations carried forward

| Construct | Status | Notes |
|---|---|---|
| K&R-style functions | not handled (probe `p21`) | indefinitely deferred; neither Lua nor cJSON uses it |
| Multi-designator `.a.b[3] = v` | not supported | unchanged from S4 |
| Range designator `[1 ... 3] = 0` | not supported | unchanged from S4 |
| `( declarator )` vs `( params )` ambiguity | uses S1 conservative heuristic | unchanged from S4 |

R1 added no new limitations.

## Snapshots

Suggested next phase: **Phase R2** — pick a third real-world C
codebase (sqlite-amalgamation, mcpp, nuklear, picol — see
`ROADMAP.md` § 8 for the full candidate list).  Pattern: write a
`build_X_with_sharpfe.sh`, reduce each new failure to a numbered
probe in `c_superset_probes/p55..pNN_*.c`, fix in the right
layer.  R1 took the pipeline from "Lua works" (one target) to
"Lua + cJSON work" (two targets, ~2.5x more surface variety);
each subsequent target compounds the confidence.

## Resuming after R1

The C-superset roadmap was already closed at S5.  R1 is the
first of an open-ended series of real-world target validations.
Because each new target takes a finite number of fixes (Lua
needed 2; cJSON needed 4), the project is now in a phase where
each added target adds confidence rather than re-architecting.

The two build scripts (`build_lua_with_sharpfe.sh`,
`build_cjson_with_sharpfe.sh`) live at the repo root and serve
as the integration smoke test.  Both must remain green after
every change.
