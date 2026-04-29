#!/usr/bin/env python3
"""
P99 Cross-Platform Comparison Test
Compares sharpc vs clang for all 65 P99 headers on Linux.
"""
import subprocess
import sys
import time
import platform
from pathlib import Path
from collections import Counter

PROJECT_DIR = Path(__file__).parent
P99_DIR = PROJECT_DIR / "p99-master" / "p99"

# Cross-platform sharpc detection
def find_sharpc():
    is_windows = platform.system() == 'Windows'
    build_dirs = [
        PROJECT_DIR / "build-debug",
        PROJECT_DIR / "cmake-build-debug",
        PROJECT_DIR / "build",
    ]
    exe_names = ["sharpc"]
    
    for build_dir in build_dirs:
        for name in exe_names:
            candidate = build_dir / name
            if candidate.exists():
                return candidate
    return None

SHARPC = find_sharpc()
CLANG = "/usr/bin/clang"

if not SHARPC:
    print("Error: sharpc not found")
    sys.exit(1)

HEADERS = sorted([f.name for f in P99_DIR.glob("p99*.h") 
                  if f.name.endswith(".h") and not f.name.startswith("p99_compat")])

results = []
for h in HEADERS:
    out_sharpc = PROJECT_DIR / f"tmp_p99_linux_{h.replace('.h', '.c')}"
    
    # Test with sharpc
    t0 = time.time()
    r_sharpc = subprocess.run(
        [str(SHARPC), str(P99_DIR / h), "-no-link", "-o", str(out_sharpc)],
        capture_output=True, text=True, timeout=120
    )
    sharpc_time = time.time() - t0
    sharpc_pass = r_sharpc.returncode == 0
    
    # Test with clang (for comparison)
    t0 = time.time()
    r_clang = subprocess.run(
        ["clang", "-std=c11", "-w", "-fsyntax-only", "-I", str(P99_DIR), 
         "-I", str(PROJECT_DIR / "third_party" / "tcc" / "include"),
         str(P99_DIR / h)],
        capture_output=True, text=True, timeout=120
    )
    clang_time = time.time() - t0
    clang_pass = r_clang.returncode == 0
    
    # Determine status
    if sharpc_pass and clang_pass:
        status = "BOTH_PASS"
    elif not sharpc_pass and not clang_pass:
        status = "BOTH_FAIL"
    elif sharpc_pass and not clang_pass:
        status = "SHARPC_PASS_CLANG_FAIL"
    else:
        status = "SHARPC_FAIL_CLANG_PASS"
    
    # Get error summary
    sharpc_err = ""
    if not sharpc_pass:
        for line in (r_sharpc.stderr + r_sharpc.stdout).splitlines():
            line = line.strip()
            if "error:" in line.lower():
                if "p99_" in line:
                    sharpc_err = line.split("error:")[-1].strip()[:100]
                    break
    
    clang_err = ""
    if not clang_pass:
        for line in (r_clang.stderr + r_clang.stdout).splitlines():
            line = line.strip()
            if "error:" in line.lower():
                clang_err = line.split("error:")[-1].strip()[:100]
                break
    
    print(f"{status:25s}  {sharpc_time:6.1f}s  {h:30s}")
    if sharpc_err:
        print(f"    sharpc: {sharpc_err}")
    if clang_err:
        print(f"    clang:  {clang_err}")
    
    results.append((status, h, sharpc_time, sharpc_err, clang_err))

# Summary
print(f"\n{'='*80}")
print(f"Linux P99 Test Summary")
print(f"{'='*80}")

status_counts = Counter(r[0] for r in results)
print(f"BOTH_PASS:              {status_counts['BOTH_PASS']:2d}/65")
print(f"BOTH_FAIL:              {status_counts['BOTH_FAIL']:2d}/65")
print(f"SHARPC_PASS_CLANG_FAIL: {status_counts['SHARPC_PASS_CLANG_FAIL']:2d}/65")
print(f"SHARPC_FAIL_CLANG_PASS: {status_counts['SHARPC_FAIL_CLANG_PASS']:2d}/65")

# Categorize failures
both_fail_headers = [r[1] for r in results if r[0] == "BOTH_FAIL"]
sharpc_fail_headers = [r[1] for r in results if r[0] == "SHARPC_FAIL_CLANG_PASS"]

if both_fail_headers:
    print(f"\nBoth sharpc and clang fail ({len(both_fail_headers)}):")
    for h in both_fail_headers[:10]:
        print(f"  - {h}")
    if len(both_fail_headers) > 10:
        print(f"  ... and {len(both_fail_headers)-10} more")

if sharpc_fail_headers:
    print(f"\nOnly sharpc fails ({len(sharpc_fail_headers)}):")
    for r in results:
        if r[0] == "SHARPC_FAIL_CLANG_PASS":
            print(f"  - {r[1]}: {r[3]}")

# Cleanup
for f in PROJECT_DIR.glob("tmp_p99_linux_*.c"):
    f.unlink()
