#!/usr/bin/env python3
"""Comprehensive check for preprocessed .pp.c files."""
import os
import sys
import re

SRC_DIR = r"test\lua-5.4.6\src"

checks = {
    "char_literal_corruption": re.compile(r"'\"\""),
    "empty_string_missing": re.compile(r"addlenmod\s*\(\s*\w+\s*,\s*\)"),
    "string_inside_char": re.compile(r"'[^\n]*\"[^\n]*'"),  # '"' inside char literal
    "adjacent_unmerged_strings": re.compile(r'"[a-zA-Z_][^"]*"\s*"[a-zA-Z_][^"]*"'),  # unmerged strings
}

issues = []
pass_count = 0

for fname in sorted(os.listdir(SRC_DIR)):
    if not fname.endswith(".pp.c"):
        continue
    fpath = os.path.join(SRC_DIR, fname)
    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()

    file_issues = []
    for check_name, pattern in checks.items():
        matches = pattern.findall(content)
        if matches:
            for m in matches[:5]:  # limit output
                file_issues.append(f"  [{check_name}] {repr(m[:80])}")

    if file_issues:
        issues.append(f"{fname}: {len(file_issues)} issues")
        issues.extend(file_issues)
    else:
        pass_count += 1

    # Check addlenmod specifically
    addlen_matches = re.findall(r'addlenmod\s*\([^)]+\)', content)
    for am in addlen_matches:
        if ", )" in am or ",)" in am:
            issues.append(f"{fname}: addlenmod with missing arg: {am}")

print(f"=== Summary ===")
print(f"Files passed all checks: {pass_count}")
print(f"Files with issues: {len(set(i.split(':')[0] for i in issues if ':' in i))}")
if issues:
    print(f"\n=== Issues ===")
    for i in issues:
        print(i)
else:
    print("No issues found!")
