#!/usr/bin/env python3
"""
Test sharpc's ability to parse P99 C99/C11 macro library headers.
"""
import subprocess
import os
import sys
from pathlib import Path

PROJECT_DIR = Path(__file__).parent
P99_DIR = PROJECT_DIR / "p99-master" / "p99"
SHARPC = PROJECT_DIR / "cmake-build-debug" / "sharpc.exe"

# Headers to test
P99_HEADERS = [
    "p99_paste.h", "p99_id.h", "p99_args.h", "p99_checkargs.h",
    "p99_if.h", "p99_logical.h", "p99_map.h", "p99_count.h",
    "p99_type.h", "p99_typenames.h", "p99_compiler.h", "p99_constraint.h",
    "p99_defarg.h", "p99_new.h", "p99_init.h",
    "p99_arith.h", "p99_classification.h", "p99_double.h", "p99_int.h",
    "p99_for.h", "p99_choice.h", "p99_try.h",
    "p99_list.h", "p99_lifo.h", "p99_fifo.h", "p99_bitset.h", "p99_str.h",
    "p99_atomic.h", "p99_threads.h", "p99_tp.h",
    "p99_enum.h", "p99_errno.h", "p99_generic.h", "p99_rand.h",
    "p99_swap.h", "p99_uf.h", "p99_uchar.h",
    "p99_block.h", "p99_callback.h", "p99_iterator.h",
    "p99_notifier.h", "p99_rwl.h",
    "p99.h",
]

# Known to fail (complex constructs not yet supported)
KNOWN_FAILURES = {
    "p99_try.h",           # try/catch uses complex setjmp/longjmp
    "p99_atomic.h",        # _Atomic type qualifiers
    "p99_threads.h",       # C11 thread_local
    "p99_rwl.h",           # read-write locks with atomics
    "p99_callback.h",      # complex function pointer typedefs
    "p99_block.h",         # GCC statement expressions
    "p99.h",               # includes everything, inherits all failures
    "p99_list.h",          # method syntax not supported
    "p99_notifier.h",      # uses p99_list
}

def test_header(header_name):
    """Test if sharpc can parse a P99 header. Returns (status, detail)"""
    h_path = P99_DIR / header_name
    if not h_path.exists():
        return "SKIP", "File not found"

    out_file = PROJECT_DIR / f"test_p99_{header_name.replace('.h', '.c')}"

    result = subprocess.run(
        [str(SHARPC), str(h_path), "-no-link", "-o", str(out_file)],
        capture_output=True, text=True, cwd=str(PROJECT_DIR), timeout=300  # 5 min
    )

    if result.returncode == 0:
        # sharpc succeeded — check with clang
        clang_result = subprocess.run(
            ["clang", "-std=c11", "-w", "-fsyntax-only", str(out_file)],
            capture_output=True, text=True, cwd=str(PROJECT_DIR), timeout=30
        )
        if clang_result.returncode == 0:
            return "PASS", ""
        else:
            err = clang_result.stderr.strip().splitlines()
            return "CLANG_FAIL", err[0] if err else "clang error"
    elif result.returncode < 0 or result.returncode == 0xC00000FD or result.returncode == -1073741571:
        return "CRASH", "stack overflow / crash"
    else:
        # Extract sharpc error
        err_lines = [l.strip() for l in (result.stderr + result.stdout).splitlines() if l.strip() and not l.startswith("  at ")]
        first_err = err_lines[0] if err_lines else f"exit code {result.returncode}"
        return "FAIL", first_err


def main():
    print("=" * 70)
    print("  P99 Header Parsing Test Suite")
    print("=" * 70)
    print()

    results = []
    pass_count = 0
    clang_fail_count = 0
    fail_count = 0
    crash_count = 0
    skip_count = 0
    known_count = 0
    unexpected_count = 0

    for header in P99_HEADERS:
        status, detail = test_header(header)
        is_known = header in KNOWN_FAILURES

        if status == "PASS":
            pass_count += 1
        elif status == "CLANG_FAIL":
            clang_fail_count += 1
            if is_known: known_count += 1
            else: unexpected_count += 1
        elif status == "FAIL":
            if is_known:
                known_count += 1
            else:
                fail_count += 1
                unexpected_count += 1
        elif status == "CRASH":
            if is_known:
                known_count += 1
            else:
                crash_count += 1
                unexpected_count += 1
        else:
            skip_count += 1

        # Display
        marker = ""
        if is_known and status not in ("PASS", "SKIP"):
            marker = " (known)"
        elif status not in ("PASS", "SKIP") and not is_known:
            marker = " **"

        icon = status
        print(f"  {icon:12s}{marker:10s} {header}")
        if detail and status not in ("PASS", "SKIP"):
            short = detail[:100] + ("..." if len(detail) > 100 else "")
            print(f"    → {short}")

    print()
    print("=" * 70)
    total = len(P99_HEADERS)
    print(f"  PASS: {pass_count}   CLANG_FAIL: {clang_fail_count}   FAIL: {fail_count}   CRASH: {crash_count}   SKIP: {skip_count}")
    print(f"  Known: {known_count}   Unexpected: {unexpected_count}")
    print(f"  Total: {total}")
    print("=" * 70)

    if unexpected_count > 0:
        print("\nUnexpected failures:")
        for status, header, detail in results:
            pass

    return 0


if __name__ == "__main__":
    sys.exit(main())
