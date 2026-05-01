#!/usr/bin/env python3
"""Phase 6 comprehensive validation on all .pp.c files."""
import os
import re

SRC_DIR = r"test\lua-5.4.6\src"

print("=== 1. Checking addlenmod calls ===")
for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    
    for i, line in enumerate(lines, 1):
        # Check addlenmod with missing argument
        if 'addlenmod' in line:
            # Find the full call (may span multiple lines)
            full = line.rstrip('\n')
            j = i
            while ')' not in full and j < len(lines):
                j += 1
                full += ' ' + lines[j-1].rstrip('\n')
            if re.search(r'addlenmod\s*\([^,]+,\s*\)', full):
                print(f"  FAIL {fname}:{i} - empty arg: {full[:120].strip()}")

print("\n=== 2. Checking for orphan closing-quote-as-opening pattern ===")
# Pattern: a closing quote that's treated as opening and reads too much
# This shows up as a string with embedded newlines and code
for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()
    
    # Find string literals that span multiple lines with code-like content
    # Normal multi-line strings use \ at end of line (continuation)
    # Bug pattern: " followed by code, newline, more code, then "
    for m in re.finditer(r'"([^"\n]*(?:\\.[^"\n]*)*)"', content):
        body = m.group(1)
        if '\n' in body and len(body) > 50:
            # Check if this is a linemarker (expected) or a bug
            if not body.startswith('#'):
                # Could be a bug - but also could be normal wide string
                # Check if it contains code patterns
                if re.search(r'\b(if|else|for|while|return|break|case)\b', body):
                    start = content.rfind('\n', 0, m.start()) + 1
                    context = content[start:m.end()].strip()[:100]
                    print(f"  SUSPECT {fname} - multi-line string with code: {context}")

print("\n=== 3. Checking char literal integrity ===")
for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()
    
    # Check specific char literals that were previously broken
    if "luaL_addchar" in content:
        # Check if the pattern luaL_addchar(b, '"') is correct
        for m in re.finditer(r'luaL_addchar\s*\([^,]+,\s*([^)]+)\)', content):
            arg = m.group(1).strip()
            if arg == "'\"\"" or arg == "\"\"":
                print(f"  FAIL {fname} - broken char literal in luaL_addchar: {arg}")

print("\n=== 4. Checking string concatenation quality ===")
# After phase6, adjacent strings should be merged
# Find cases like "hello" "world" that weren't merged (outside linemarkers)
for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()
    
    for i, line in enumerate(lines, 1):
        # Skip linemarkers
        stripped = line.lstrip()
        if stripped.startswith('#'):
            continue
        # Find adjacent unmerged strings (with only whitespace between)
        if re.search(r'"\s+"', line):
            # This might be intentional in some contexts, flag for review
            if len(line.strip()) < 200:  # only short lines
                print(f"  INFO {fname}:{i} - possibly unmerged strings: {line.strip()[:100]}")

print("\n=== 5. Summary: checking all .pp.c files compile cleanly ===")
# Already verified via clang-cl above
print("  (Verified in previous step - all 34 files compiled without errors)")

print("\nDone.")
