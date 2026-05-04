# Phase R3 — zlib 1.3.1 Real-World Integration

> Status: COMPLETE.  Sharp/C preprocessor processes all 15 zlib source
> files with **zero diagnostics**.  zlib is the third real-world target
> after Lua (R1, 35 files) and SQLite (R2, 126 files); together these
> three phases validate sharp-cpp on **176 production C files** spanning
> ~525 KLoC with no fatals, errors, or warnings.

## Numbers

| Metric                           | Value                          |
|----------------------------------|--------------------------------|
| zlib version                     | 1.3.1 (release tag)            |
| Total `.c` files processed       | 15                             |
| Source LoC                       | ~22,732                        |
| sharp-cpp diagnostics            | **0 fatal, 0 error, 0 warning** |
| Aggregate output ratio           | 0.970 (sharp-cpp / gcc -E)     |
| Per-file ratio range             | 0.91 – 1.02                    |

Per-file results:

| File          | sharp-cpp bytes | gcc -E bytes | ratio |
|---------------|----------------:|-------------:|------:|
| adler32.c     | 60,516          | 60,146       | 1.01  |
| compress.c    | 27,582          | 27,615       | 1.00  |
| crc32.c       | 139,511         | 143,089      | 0.97  |
| deflate.c     | 100,358         | 107,925      | 0.93  |
| gzclose.c     | 77,555          | 76,129       | 1.02  |
| gzlib.c       | 84,774          | 88,823       | 0.95  |
| gzread.c      | 86,469          | 89,091       | 0.97  |
| gzwrite.c     | 86,175          | 88,324       | 0.98  |
| infback.c     | 77,946          | 82,772       | 0.94  |
| inffast.c     | 62,817          | 65,424       | 0.96  |
| inflate.c     | 94,837          | 104,430      | 0.91  |
| inftrees.c    | 61,668          | 62,124       | 0.99  |
| trees.c       | 89,927          | 91,798       | 0.98  |
| uncompr.c     | 27,671          | 27,779       | 1.00  |
| zutil.c       | 79,623          | 78,065       | 1.02  |

## What this phase tested

zlib is a different style of C codebase than Lua or SQLite:

1. **K&R-era style.** Original zlib code dates to 1995; macro idioms reflect that era.  Heavy use of `OF((args))` for K&R/ANSI compatibility, conditional `OF` definitions based on `__STDC__`, etc.
2. **Tight performance-oriented headers.** `zconf.h` and `zlib.h` are extensively `#if`-conditional, defining types and call conventions per platform/compiler.  These conditionals must evaluate identically to gcc for the resulting types to match.
3. **Compact codebase, high compile fan-out.**  All 15 files include `zlib.h` + `zconf.h` + `zutil.h` chain; each must produce the same effective configuration.

The result — zero diagnostics, ratios consistently 0.91–1.02, no skipped files — confirms sharp-cpp handles the K&R-style macro idioms, platform-conditional type definitions, and compact include structure of zlib without modification.

## Why no bugs were found

R3, like R2, is a **regression-coverage phase** — its purpose is to verify that sharp-cpp still works on a real, third-style codebase after the R1 fix.  A bug in R3 would have indicated either an issue specific to compact K&R-era code or a regression introduced after R1.  Neither showed up.

The aggregate ratio of 0.97 is closer to 1.0 than SQLite's 0.96 — likely because zlib's code is straight-line C with relatively few macros that produce divergent assert/parenthesis-spacing behaviour.

## Files added in this phase

- `zlib_smoke.sh` — runner mirroring `sqlite_smoke.sh`'s structure
- `Makefile` — `zlib_smoke` target with auto-clone of `madler/zlib v1.3.1`
- `Makefile` — `real_world_tests` convenience target (Lua + SQLite + zlib + mcpp)

## How to reproduce

    make zlib_smoke

Or against an existing checkout:

    make zlib_smoke ZLIB_SRC=/path/to/zlib
