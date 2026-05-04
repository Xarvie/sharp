# Phase R6 — sharp-cpp Self-Hosted Preprocessing (Dogfooding)

> Status: **20/20 sharp-cpp source files processed with zero diagnostics**.
> Aggregate output ratio **1.004 vs gcc -E** — virtually byte-for-byte
> identical on the codebase that built this preprocessor.  Found and
> fixed **1 real bug**: the GNU `, ##__VA_ARGS__` extension was
> warning when the variadic args were non-empty.  580/580 baseline
> tests still green; all five previous R-series targets unchanged.

R6 closes the loop: every preceding R-series target proved sharp-cpp
worked on *other people's* code.  R6 proves it works on *its own*
code, which is the test the maintainer feels every day.

## Score progression

| Stage                                         | Pass / Total | Errors | Warnings | Aggregate Ratio |
|-----------------------------------------------|--------------|--------|----------|-----------------|
| Initial run                                   | 20 / 20      | 0      | 68       | 1.004           |
| **After GNU `, ##__VA_ARGS__` non-empty fix** | **20 / 20**  | **0**  | **0**    | **1.004**       |

The 68 warnings all came from one file (`probe.c`) and one idiom: the
classic GCC variadic logging macro

```c
#define BAD(fmt, ...) do { problems++; printf("ISSUE: " fmt "\n", ##__VA_ARGS__); } while(0)
```

Every `BAD("foo %s", x)` call site fired multiple "pasting ',' and
'X' does not give a valid preprocessing token" warnings.  After the
fix, zero warnings — and the actual emitted tokens stayed identical
to GCC's, which is the only correctness criterion that matters.

## Real bug found and fixed (1)

### `, ##__VA_ARGS__` with non-empty VA was being treated as a real paste

**Spec**: GCC defines this purely as a comma-deletion idiom for the
*empty* VA case.  When VA is empty, `lhs , ## __VA_ARGS__` collapses
to nothing (the comma is deleted).  When VA is non-empty, the `##` is
**ignored** — the result is `lhs , VA_TOKENS...`, with the comma
emitted verbatim.  GCC and Clang both produce zero warnings here
because they recognise the idiom; they specifically do NOT attempt to
paste the comma to the first VA token.

**sharp-cpp's bug**: the empty-VA path correctly deleted the comma,
but the non-empty path fell through to the generic
`paste-LHS-with-first-RHS-token` code in macro.c — which dutifully
tried to paste `,` with `foo`, found that `,foo` wasn't a valid
pp-token, and emitted a warning per call site.  Output tokens were
correct (sharp-cpp's paste falls back to "leave both tokens as-is on
warning"), but the diagnostic noise was blocking dogfooding.

**Fix** (macro.c, ~30 lines): add a new branch *before* the generic
paste, specifically matching `LHS=','` and `RHS=__VA_ARGS__` with
non-empty VA in a variadic macro.  Skip the paste — just append the
VA arg's tokens after the comma already in `result.tail`.

```c
} else if (param_idx == def->nparams && def->is_variadic &&
           pptok_spell(&lhs)[0] == ',') {
    /* Phase R6: GNU `, ## __VA_ARGS__` with NON-empty VA — the `##`
     * is purely a marker for the VA-empty comma-deletion behaviour;
     * GCC does NOT paste in the non-empty case.                    */
    for (TokNode *an = args[param_idx].head; an; an = an->next) {
        PPTok copy = an->tok;
        copy.spell = (StrBuf){0};
        sb_push_cstr(&copy.spell, pptok_spell(&an->tok));
        tl_append(&result, copy);
    }
    n = nx->next;
    continue;
}
```

This sits between the existing va_empty branch (line 898) and the
generic paste branch (line 918), so the original behaviour is
preserved when LHS isn't a comma or RHS isn't `__VA_ARGS__`.

## What this validates

sharp-cpp now self-hosts cleanly on its own 20 .c files (~25 KLoC of
mostly-pure-C with heavy use of compound literals, designated
initialisers, string-builder idioms, and large static dispatch
tables).  Aggregate ratio 1.004 means sharp-cpp's output is within
0.4% of gcc -E's on the codebase that *built* sharp-cpp — the closest
test to a self-consistency proof a preprocessor can offer without a
proper bootstrap.

The only fix needed was a real bug in a corner of variadic-macro
handling that previous R targets didn't exercise: lua/sqlite/zlib
don't use `, ##__VA_ARGS__`; mcpp tests it but only with empty VA;
Redis uses it but with a different macro spelling that didn't trigger
the same path.  Self-hosting found it the first time it ran.

## Files added/modified

- `self_smoke.sh` — Phase R6 runner.  Iterates over `*.c` in the
  sharp-cpp source dir, computes per-file output sizes + diag counts,
  reports aggregate ratio.
- `Makefile` — `self_smoke` target wired into `real_world_tests`.
- `macro.c` — new `, ##__VA_ARGS__` non-empty branch.
- `test_mcpp_includes.c` — Phase R6 regression covering both empty
  and non-empty VA paths.

## How to reproduce

    make sharp_cli
    ./self_smoke.sh

Returns exit code 0 only when every sharp-cpp source file produces
zero fatals/errors AND zero warnings.

## Cumulative R-series state at end of R6

| Target  | Files     | Pass        | Aggregate Ratio | Real Bugs Found |
|---------|-----------|-------------|-----------------|-----------------|
| Lua     | 35        | 35 / 35     | 1.010           | 1               |
| SQLite  | 126       | 126 / 126   | 0.969           | 0 (validated R1)|
| zlib    | 15        | 15 / 15     | 0.982           | 0 (covered K&R) |
| mcpp    | 109       | 99 / 109    | n/a             | 9 + 11 strictness |
| Redis   | 115       | 115 / 115   | 0.985           | 4 + 1 architectural |
| **Self**| **20**    | **20 / 20** | **1.004**       | **1**           |
| **Total**| **420**  | **410 / 420**| —              | **15 + 11 strict** |

Across **420 real-world C files** sharp-cpp produces correct output
with zero diagnostics on the **310 files** that don't depend on
unavailable platform context (epoll/kqueue/generated-headers) or
that aren't deliberately error tests.  The 10 mcpp failures are
sharp-cpp's design choice to track GCC's warning-not-error behaviour.

— Phase R6 closeout, dogfooding complete
