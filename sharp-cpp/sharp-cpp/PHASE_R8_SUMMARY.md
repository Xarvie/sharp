# Phase R8 — Cross-Preprocessor Test (mcpp source)

> Status: **10/10 mcpp source files processed with zero diagnostics**.
> Aggregate output ratio **0.933 vs gcc -E**.  No real bugs were found
> — sharp-cpp handles mcpp's K&R-era PP idioms cleanly.  This is the
> "preprocessor processes preprocessor" test that closes the R-series
> loop: every preceding R target has been a *user* of preprocessing,
> R8 is a *peer* implementation.

R6 was self-hosted dogfooding (sharp-cpp processes sharp-cpp).  R8 is
the cross-implementation analogue: sharp-cpp processes the source of
*another* preprocessor, mcpp.  Different style (1990s-era portable C
with conservative idioms), different PP corner cases internalised.

## Score

| Stage                          | Pass / Total | Rate    | Aggregate Ratio |
|--------------------------------|--------------|---------|-----------------|
| Initial run                    | 10 / 10      | 100%    | 0.933           |

The single skipped file is `cc1.c` — a MinGW-only stub that uses
`<process.h>` (Windows header).  GCC's `-E` also fails on it.

## What this validates

mcpp's source exercises:
- **Heavy K&R-era macro idioms** — old-school style with comma operators
  inside macro bodies, function-like macros that are the only reasonable
  way to write the operation
- **Multi-byte character handling** — mcpp supports JIS, EUC-JP, GB,
  Big5, KSC, UTF-8 source encodings via mbchar.c
- **Error message tables** — large `static const char *` arrays
  generated from msg.txt with conditional compilation per locale
- **Bootstrap concerns** — mcpp processes its own headers
  (system.H, internal.H) which are deeply layered

None of these triggered new bugs.  The aggregate ratio of 0.933 is
slightly below gcc's because sharp-cpp tracks fewer historical macros
that mcpp's code-generated configuration probes for (e.g. `INC_DIR`,
`__STDC_HOSTED__` variants).

## Files added/modified

- `mcpp_self_smoke.sh` — Phase R8 runner.  Iterates over
  `mcpp-src/src/*.c`, skips `cc1.c` (MinGW), reports per-file diags +
  size ratios.
- `Makefile` — `mcpp_self_smoke` target wired into `real_world_tests`.
- `PHASE_R8_SUMMARY.md` — this document.

## How to reproduce

    make sharp_cli
    ./mcpp_self_smoke.sh /home/claude/mcpp-src

Or via Make:

    make mcpp_self_smoke

## Cumulative R-series state at end of R8

| Target  | Files | Pass        | Aggregate Ratio | Real Bugs Found |
|---------|-------|-------------|-----------------|-----------------|
| Lua     | 35    | 35 / 35     | 1.010           | 1               |
| SQLite  | 126   | 126 / 126   | 0.969           | 0 (validated R1)|
| zlib    | 15    | 15 / 15     | 0.982           | 0               |
| **mcpp testsuite** | **106** | **106 / 106 (100.0%)** | n/a | 9 + 11 strict + 3 R7 |
| Redis   | 115   | 115 / 115   | 0.985           | 4 + 1 architectural |
| Self    | 20    | 20 / 20     | 1.002           | 1               |
| **mcpp source** | **10** | **10 / 10** | **0.933**   | **0**           |
| **Total**| **427** | **427 / 427 (100%)** | —     | **18 real + 11 strict** |

**Every non-skipped real-world C file in the R-series processes with
zero diagnostics.**  R8 confirms sharp-cpp handles the source of
another full PP implementation cleanly, completing the cross-validation
loop.

— Phase R8 closeout
