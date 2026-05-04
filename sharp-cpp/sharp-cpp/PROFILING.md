# Profiling sharp-cpp

This document describes the workflow for measuring and visualising
performance and memory characteristics of the Sharp/C preprocessor.
It is intended as the data-collection prerequisite for any optimisation
work in the **B / C / D** track of the Phase 8+ candidate list (intern-
table hashing, PPTok object pool, StrBuf preallocation).

> **Don't optimise without data.**  The current 65 ms/iter (~2× `gcc -E`)
> is already adequate for IDE-real-time use.  Before touching the hot
> path, capture a baseline flamegraph, then re-capture after each
> change.  Numbers without flamegraphs lie.

---

## Tools required

| Tool | Purpose | Install (Debian/Ubuntu) |
|---|---|---|
| `perf` | Sampling CPU profiler (kernel-level) | `apt install linux-tools-generic` |
| FlameGraph scripts | Stack-collapse + SVG rendering | `git clone https://github.com/brendangregg/FlameGraph` |
| `valgrind` + `ms_print` | Heap allocation profiling | `apt install valgrind` |

On macOS, `perf` is unavailable; use Instruments → Time Profiler instead
(see "Alternatives" below).  The `make profile` / `make flamegraph` /
`make massif` targets are Linux-only.

---

## Quick reference

```bash
# 1. Top hot functions (text report, ~1 minute):
make profile

# 2. Interactive flamegraph SVG (open in browser):
make flamegraph FLAMEGRAPH_DIR=$HOME/src/FlameGraph
open flamegraph.svg          # or `xdg-open` on Linux

# 3. Heap peak + allocation hot spots (slow — valgrind):
make massif
```

All three targets force a **fresh `-O2 -g -fno-omit-frame-pointer`**
build into `profile_bin` so DWARF unwinding gives accurate stacks.
The default 200-iteration workload runs the same `BIG_PILE_SRC` that
`make bench` uses (~9k output lines per iter).

Tune the workload size with `PROFILE_ITERS=N` (default 200).  For
diagnosing a specific slow input, modify `bench.c` to read your input
from a file before running these targets.

---

## Reading the perf report

`make profile` produces a perf data file and prints the top 20
self-time functions.  Read column 1 ("Self %") — that is where CPU time
is actually spent inside that function, excluding callees.  Typical
output for a clean baseline (no regressions) shows roughly:

```
Self %  Symbol                  Notes
~25%    pptok_spell             Spelling-buffer materialisation
~15%    intern_cstr             Linear-scan intern table (B candidate)
~12%    macro_expand            Core expansion loop
~8%     reader_next_tok         Tokenisation
...
```

If `intern_cstr` rises noticeably above ~15%, that is the case for
**B (InternTable hashing)**.  If `pptok_spell` dominates, **D
(StrBuf preallocation)** is the right intervention.  If `tl_append` /
`pptok_copy` show up high, **C (PPTok object pool)** matches.

---

## Reading the flamegraph

Open `flamegraph.svg` in any browser.  Width = total CPU time spent in
that frame and its descendants.  Click any frame to zoom; type-search
with the magnifying glass top-right.

Look for **plateaus**: wide horizontal regions of a single function,
which indicate work concentrated in one place rather than spread thin.
Wide plateaus are good optimisation targets; tall narrow towers are
typically not (they spread cost over many leaves).

---

## Reading massif output

`make massif` writes `massif.out` and prints the textual graph.  Look
at the peak memory line:

```
    KB
12.34^                                                     #
     |                                                     #
     |                                                  ##:#
     |                                              ::::#:::#:
     ...
     0 +----------------------------------------------------------->
       0                                                          5.0
```

The `#` marks the snapshot at peak RSS.  Below the graph, the per-call-
site allocation breakdown identifies **what** is pinning memory.
Expect ~1–3 MB peak for the `BIG_PILE_SRC` workload; significantly
higher (e.g. 20+ MB) signals a leak introduced by a recent change.

---

## Methodology checklist

For results worth comparing across runs:

1. **Disable turbo boost** to avoid frequency-scaling noise:
   `echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo`
2. **Pin the bench process** to one core (`taskset -c 2 ./profile_bin 200`).
3. **Quiesce the system** — close browsers, stop background jobs.
4. **Run three times** and take the median.  A single run can vary
   ±10% on a busy laptop.
5. **Capture baseline before AND after** any change.  Posting a single
   "after" number with no "before" is meaningless.

The CI environment (GitHub Actions runners) is too noisy for absolute
numbers but is fine for catching gross regressions (>2× slowdown).

---

## Comparing against `gcc -E`

`make bench` already runs `gcc -E` as a subprocess for ratio context.
The current ratio sits around 2× on a typical Linux/x86_64 host.  Note
that this measurement **includes `gcc -E`'s fork/exec overhead** —
sharp-cpp runs in-process — so the in-pure-CPU comparison is closer
than the wall-clock ratio suggests.

To get a cleaner CPU-only comparison, run gcc without subprocess
overhead by piping its output to `/dev/null` and timing it externally:

```bash
time gcc -E benchmark_input.c > /dev/null
```

---

## Alternatives on macOS / Windows

- **macOS**: open Xcode Instruments → Time Profiler.  Attach to
  `./profile_bin 200` (built via `make profile_bin`) and click Record.
  Same insights as `perf report`, GUI-driven.
- **Windows (MSVC)**: use Visual Studio's CPU Usage diagnostic tool
  (Debug → Performance Profiler).  Build via the MSYS2 path or
  cross-compile.
- **Cross-platform sampler**: `samply` (from the Mozilla Firefox team)
  is a good `perf`-equivalent that runs on Linux/macOS/Windows.

The exact tool matters less than the **discipline**: capture a
baseline, change one thing, re-capture, compare.
