#!/usr/bin/env python3
"""Quick P99 test — one header at a time, collect full error messages."""
import subprocess
import sys
import time
import platform
from pathlib import Path

PROJECT_DIR = Path(__file__).parent
P99_DIR = PROJECT_DIR / "p99-master" / "p99"

# Cross-platform sharpc detection
def find_sharpc():
    """Find sharpc executable across different build directories and platforms"""
    is_windows = platform.system() == 'Windows'
    build_dirs = [
        PROJECT_DIR / "cmake-build-debug" / "Debug",
        PROJECT_DIR / "cmake-build-debug",
        PROJECT_DIR / "build-debug",
        PROJECT_DIR / "build",
    ]
    exe_names = ["sharpc.exe", "sharpc"] if is_windows else ["sharpc"]
    
    for build_dir in build_dirs:
        for name in exe_names:
            candidate = build_dir / name
            if candidate.exists():
                return candidate
    return None

SHARPC = find_sharpc()
if SHARPC is None:
    print("Error: sharpc not found. Please build the project first.")
    sys.exit(1)

print(f"Platform: {platform.system()} ({platform.machine()})")
print(f"Compiler: {SHARPC}")
print(f"P99 dir: {P99_DIR}")
print(f"\nTesting {len([f.name for f in P99_DIR.glob('p99*.h') if f.name.endswith('.h') and not f.name.startswith('p99_compat')])} P99 headers...\n")

HEADERS = sorted([f.name for f in P99_DIR.glob("p99*.h") if f.name.endswith(".h") and not f.name.startswith("p99_compat")])

results = []
for h in HEADERS:
    out = PROJECT_DIR / f"tmp_p99_{h.replace('.h', '.c')}"
    t0 = time.time()
    r = subprocess.run(
        [str(SHARPC), str(P99_DIR / h), "-no-link", "-o", str(out)],
        capture_output=True, text=True, timeout=120
    )
    elapsed = time.time() - t0
    status = "PASS" if r.returncode == 0 else "FAIL"
    
    # Get first error line
    err_line = ""
    if r.returncode != 0:
        for line in (r.stderr + r.stdout).splitlines():
            line = line.strip()
            if "error:" in line.lower():
                err_line = line[:150]
                break
        if not err_line:
            err_line = (r.stderr + r.stdout).splitlines()[-1][:150] if (r.stderr + r.stdout).strip() else "exit " + str(r.returncode)
    
    print(f"{status:5s}  {elapsed:6.1f}s  {h:30s}  {err_line}")
    results.append((status, h, elapsed, err_line))

passed = sum(1 for s, _, _, _ in results if s == "PASS")
failed = len(results) - passed
print(f"\nPASS: {passed}/{len(results)}  FAIL: {failed}")
