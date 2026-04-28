#!/usr/bin/env python3
"""
P99 Full Test Runner — auto-kill on hang, collect bugs to MD.
"""
import subprocess
import sys
import time
import os
from pathlib import Path

PROJECT_DIR = Path(__file__).parent
P99_DIR = PROJECT_DIR / "p99-master" / "p99"
SHARPC = PROJECT_DIR / "cmake-build-debug" / "Debug" / "sharpc.exe"
TIMEOUT = 30  # seconds per header
BUG_FILE = PROJECT_DIR / "P99_BUGS.md"

HEADERS = sorted([f.name for f in P99_DIR.glob("p99*.h") if f.name.endswith(".h") and not f.name.startswith("p99_compat")])

bugs = []
passes = []

print(f"Testing {len(HEADERS)} P99 headers (timeout={TIMEOUT}s each)...")
print("=" * 80)

for i, h in enumerate(HEADERS, 1):
    out = PROJECT_DIR / f"tmp_p99_{h.replace('.h', '.c')}"
    t0 = time.time()
    
    try:
        r = subprocess.run(
            [str(SHARPC), str(P99_DIR / h), "-no-link", "-o", str(out)],
            capture_output=True, text=True, timeout=TIMEOUT
        )
        elapsed = time.time() - t0
        
        if r.returncode == 0:
            print(f"[{i}/{len(HEADERS)}] PASS  {elapsed:5.1f}s  {h}")
            passes.append(h)
        else:
            # Extract first error
            err_text = (r.stderr + r.stdout).strip()
            first_err = ""
            for line in err_text.splitlines():
                line = line.strip()
                if "error:" in line.lower():
                    first_err = line[:200]
                    break
            if not first_err:
                first_err = err_text.splitlines()[-1][:200] if err_text else f"exit code {r.returncode}"
            
            bugs.append((h, "PARSE_ERROR", first_err, elapsed))
            print(f"[{i}/{len(HEADERS)}] FAIL  {elapsed:5.1f}s  {h}")
            print(f"         → {first_err[:100]}")
            
    except subprocess.TimeoutExpired:
        elapsed = time.time() - t0
        bugs.append((h, "TIMEOUT", f"Exceeded {TIMEOUT}s timeout", elapsed))
        print(f"[{i}/{len(HEADERS)}] HANG  {TIMEOUT:5.1f}s  {h}")
        # Kill any lingering sharpc processes
        subprocess.run("taskkill /F /IM sharpc.exe >NUL 2>&1", shell=True)

print("\n" + "=" * 80)
print(f"RESULTS: {len(passes)}/{len(HEADERS)} PASS, {len(bugs)}/{len(HEADERS)} FAIL")
print("=" * 80)

# Write bugs to MD
with open(BUG_FILE, "w") as f:
    f.write("# P99 Bug Report\n\n")
    f.write(f"**Date**: {time.strftime('%Y-%m-%d %H:%M')}\n")
    f.write(f"**Total Headers**: {len(HEADERS)}\n")
    f.write(f"**PASS**: {len(passes)}/{len(HEADERS)}\n")
    f.write(f"**FAIL**: {len(bugs)}/{len(HEADERS)}\n\n")
    
    f.write("## Bug List\n\n")
    f.write("| # | Header | Type | Error | Time |\n")
    f.write("|---|--------|------|-------|------|\n")
    for idx, (h, bug_type, err, elapsed) in enumerate(bugs, 1):
        err_escaped = err.replace("|", "\\|").replace("\n", " ")[:100]
        f.write(f"| {idx} | {h} | {bug_type} | {err_escaped} | {elapsed:.1f}s |\n")
    
    f.write("\n## Pass List\n\n")
    for h in passes:
        f.write(f"- {h}\n")

print(f"\nBug report written to: {BUG_FILE}")
