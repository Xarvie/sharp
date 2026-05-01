#!/usr/bin/env python3
"""Deep check: verify each reported issue by examining actual token boundaries."""
import os
import re

SRC_DIR = r"test\lua-5.4.6\src"

# Check specific problematic patterns that indicate real bugs
real_checks = {
    # Char literal with missing closing quote
    "unclosed_char": re.compile(r"'[^'\\\n]{3,}'"),
    # String literal concatenated with nothing (shouldn't happen)
    "triple_quote": re.compile(r'\"\"\"'),
    # Missing string: pattern like , ) in function calls (excluding valid cases)
    "empty_arg_func": re.compile(r'\w+\(\w+,\s*\)'),
    # Char literal containing double-quote followed by string start
    "char_quote_string": re.compile(r"'\"'"),
}

# Also check: does each char literal have balanced quotes?
def check_char_literals(fname, content):
    """Find char literals that might have broken quotes."""
    issues = []
    # Walk through and track single-quote contexts
    i = 0
    while i < len(content):
        if content[i] == "'" and (i == 0 or content[i-1] != '\\'):
            # Start of char literal
            j = i + 1
            while j < len(content):
                if content[j] == '\\' and j + 1 < len(content):
                    j += 2  # skip escape
                    continue
                if content[j] == "'":
                    break
                j += 1
            if j >= len(content) or content[j] != "'":
                issues.append(f"  UNCLOSED char literal at pos {i}: {repr(content[i:min(i+20, len(content))])}")
            elif j - i > 10:
                # Very long char literal - suspicious
                char_content = content[i+1:j]
                if '"' in char_content and len(char_content) > 4:
                    # Has " and is long - might be spanning multiple tokens
                    pass  # not necessarily a bug
            i = j + 1
        else:
            i += 1
    return issues

all_issues = []

for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    # Check unclosed char literals
    char_issues = check_char_literals(fname, content)
    if char_issues:
        all_issues.append(f"{fname}:")
        all_issues.extend(char_issues)

if all_issues:
    print(f"Found {len(all_issues)} potential issues:")
    for i in all_issues:
        print(i)
else:
    print("No unclosed char literals found in any file.")

# Verify the specific patterns from the previous check
print("\n=== Verifying reported patterns ===")
files_to_check = ["ldblib.pp.c", "ldump.pp.c", "llex.pp.c", "lstrlib.pp.c", "luac.pp.c", "lundump.pp.c"]
for fname in files_to_check:
    fpath = os.path.join(SRC_DIR, fname)
    if not os.path.exists(fpath):
        continue
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()
    
    # Check for specific patterns that were reported
    if "'\"\"" in content:
        print(f"{fname}: REAL BUG - char literal with '\"\"")
    
    # Check addlenmod
    for m in re.finditer(r'addlenmod\s*\([^)]+\)', content):
        text = m.group()
        if re.search(r',\s*\)', text):
            print(f"{fname}: REAL BUG - {text}")

print("\nDone.")
