#!/usr/bin/env python3
"""Fast P99 test — 5s timeout, collect ALL bugs."""
import subprocess, sys, time
from pathlib import Path

PROJECT_DIR = Path(__file__).parent
P99_DIR = PROJECT_DIR / "p99-master" / "p99"
SHARPC = PROJECT_DIR / "cmake-build-debug" / "Debug" / "sharpc.exe"
TIMEOUT = 5

HEADERS = sorted([f.name for f in P99_DIR.glob("p99*.h") if f.name.endswith(".h") and not f.name.startswith("p99_compat")])

bugs = []
passes = []
print(f"Testing {len(HEADERS)} headers (timeout={TIMEOUT}s)...\n")

for i, h in enumerate(HEADERS, 1):
    out = PROJECT_DIR / f"tmp_p99_{h.replace('.h', '.c')}"
    t0 = time.time()
    try:
        r = subprocess.run([str(SHARPC), str(P99_DIR / h), "-no-link", "-o", str(out)],
                          capture_output=True, text=True, timeout=TIMEOUT)
        elapsed = time.time() - t0
        if r.returncode == 0:
            print(f"[{i:2d}/{len(HEADERS)}] PASS {elapsed:4.1f}s {h}")
            passes.append(h)
        else:
            err = ""
            for line in (r.stderr + r.stdout).splitlines():
                if "error:" in line.lower():
                    err = line.strip()[:150]
                    break
            if not err:
                err = (r.stderr + r.stdout).splitlines()[-1][:150]
            bugs.append((h, "PARSE", err, elapsed))
            print(f"[{i:2d}/{len(HEADERS)}] FAIL {elapsed:4.1f}s {h}")
    except subprocess.TimeoutExpired:
        elapsed = TIMEOUT
        bugs.append((h, "HANG", f"Exceeded {TIMEOUT}s", elapsed))
        print(f"[{i:2d}/{len(HEADERS)}] HANG {elapsed:4.1f}s {h}")
        subprocess.run("taskkill /F /IM sharpc.exe >NUL 2>&1", shell=True)

print(f"\n{'='*70}")
print(f"PASS: {len(passes)}/{len(HEADERS)}  FAIL: {len(bugs)}/{len(HEADERS)}")
print(f"{'='*70}")

# Group by error type
from collections import Counter
types = Counter(b[1] for b in bugs)
print(f"\nFailure types: {dict(types)}")

# Group by error pattern
patterns = {}
for h, typ, err, _ in bugs:
    key = err[:80] if err else typ
    if key not in patterns: patterns[key] = []
    patterns[key].append(h)

print(f"\nError patterns:")
for pat, hdrs in sorted(patterns.items(), key=lambda x: -len(x[1])):
    print(f"  [{len(hdrs):2d}] {pat[:100]}")
    if len(hdrs) <= 3:
        for h in hdrs: print(f"       - {h}")
    else:
        print(f"       - {hdrs[0]} ... ({len(hdrs)} total)")

# Write MD
md = f"# P99 Bug Report - {time.strftime('%Y-%m-%d %H:%M')}\n\n"
md += f"**PASS**: {len(passes)}/{len(HEADERS)}  **FAIL**: {len(bugs)}/{len(HEADERS)}\n\n"
md += "## Bug List\n\n| # | Header | Type | Error | Time |\n|---|--------|------|-------|------|\n"
for idx, (h, typ, err, elapsed) in enumerate(bugs, 1):
    err_e = err.replace("|","\\|")[:100]
    md += f"| {idx} | {h} | {typ} | {err_e} | {elapsed:.1f}s |\n"
md += "\n## Error Patterns\n\n"
for pat, hdrs in sorted(patterns.items(), key=lambda x: -len(x[1])):
    md += f"### [{len(hdrs)} headers] {pat[:120]}\n\n"
    for h in hdrs: md += f"- {h}\n"

with open(PROJECT_DIR / "P99_BUGS.md", "w") as f: f.write(md)
print(f"\nBug report: P99_BUGS.md")
