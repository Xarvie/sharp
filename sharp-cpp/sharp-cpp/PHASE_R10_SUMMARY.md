# Phase R10 — Cascaded Macro Argument Sanity (mcpp 100% Zero-Skip)

> Status: **mcpp Validation Suite 109/109 (100.0%)** with **zero
> skips**.  Up from 108/108 in R9 (which had 1 skip).  Final mcpp test
> `e_25_6` now passes via a new sanity check on cascaded macro
> arguments.  610/610 baseline tests still green; all R-series targets
> at 100%.

## What changed

### Cascaded-arg unbalanced-paren warning

Prior to R10, when a macro argument's pre-expansion produced an
unbalanced `(`, sharp-cpp silently substituted the result into the
outer macro's body.  This is undefined behaviour per C99 §6.10.3.4
(rescanning of an argument should complete within that argument), and
mcpp's `e_25_6.c` exercises exactly this case:

```c
#define sub(x,y)            (x-y)
#define head                sub(
#define body(x,y)           x,y
#define tail                )
#define head_body_tail(a,b,c) a b c
head_body_tail(head, body(a,b), tail);
```

The argument `head` pre-expands to `sub(`, leaving a dangling `(`.
After substitution, the resulting token stream is `sub( a,b )`, which
sharp-cpp would then try to rescan as a `sub` macro call — producing
unpredictable results that depend on macro semantics that don't apply
to actual `sub` arguments.

**Fix**: in `macro.c substitute()`, immediately after `expand_list()`
returns from pre-expanding an argument, walk the expansion and count
parens.  If they don't balance — either because a `)` appears before
its matching `(`, or because depth is non-zero at the end — emit a
warning:

```
warning: macro argument's expansion has unbalanced parentheses
         (cascaded call may be unterminated)
```

The warning is intentionally a warning, not an error.  GCC and Clang
both silently accept this (their behaviour differs from sharp-cpp's
post-fix output but is still "accept and emit something").  Treating
it as a hard error would diverge from the dominant ecosystem; treating
it as a warning catches the UB while remaining lenient enough that
real-world code (Lua/SQLite/Redis/Self/mcpp's own source) doesn't
trip on it.

## Coverage

The new check runs in exactly the right place: only when a macro arg
is *itself substituted via expansion path* (the non-paste branch of
parameter substitution).  This means:

- ✅ **e_25_6** — cascaded-arg pre-expansion produces `sub(` →
  unbalanced detected → warning emitted
- ✅ **n_27** — `head a, b );` is *not* a cascaded-arg case (head is
  used directly as a macro call, not as an argument), so this check
  doesn't fire and the existing R4 cascaded-rescan path handles it
- ✅ **Real-world**: 311 files across Lua/SQLite/zlib/Redis/self/mcpp
  emit zero new warnings (verified)

## Score

| Target              | Pass / Total | Rate    | Skip count |
|---------------------|--------------|---------|------------|
| R7 final            | 106 / 106    | 100%    | 3 skips    |
| R9 final            | 108 / 108    | 100%    | 1 skip     |
| **R10 final**       | **109 / 109**| **100%**| **0 skips**|

## Files added/modified

- `macro.c substitute()` — unbalanced-paren check on argument
  expansion result, emits warning
- `mcpp_smoke.sh classify_test()` — empty `case` block (all skips
  removed); kept the function for documentation purposes
- `test_mcpp_includes.c` — Phase R10 regression suite (§31, 2 new
  test points: positive case from `e_25_6`-style trigger, negative
  case for balanced cascaded args)

## Cumulative R-series state at end of R10

| Target  | Files | Pass        | Aggregate Ratio | Real Bugs Found |
|---------|-------|-------------|-----------------|-----------------|
| Lua     | 35    | 35 / 35     | 1.010           | 1               |
| SQLite  | 126   | 126 / 126   | 0.969           | 0 (validated R1)|
| zlib    | 15    | 15 / 15     | 0.982           | 0               |
| **mcpp testsuite** | **109** | **109 / 109 (100.0%)** | n/a | 9 + 11 strict + 4 R7+R10 |
| Redis   | 115   | 115 / 115   | 0.985           | 4 + 1 architectural |
| Self    | 20    | 20 / 20     | 1.002           | 1               |
| mcpp source | 10 | 10 / 10     | 0.933           | 0               |
| **Total**| **430** | **430 / 430 (100%)** | —     | **19 real + 11 strict** |

**mcpp Validation Suite is now zero-skip 100%** — the test set that
sharp-cpp considered "settled" 4 phases ago (after R7 with 3 skips
documented as "spec divergence") has been further reduced as
sharp-cpp's `-ansi`/std mode (R9) and cascaded-arg sanity (R10)
addressed cases that turned out to be addressable rather than truly
divergent.  Only `e_25_6` was a genuine spec ambiguity — and it too
yielded to a small targeted check.

— Phase R10 closeout, mcpp 100% zero-skip
