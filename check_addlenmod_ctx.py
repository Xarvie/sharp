import re

with open(r"c:\Users\ftp\Desktop\sharp\test\lua-5.4.6\src\lstrlib.pp.c", "r", encoding="utf-8", errors="replace") as f:
    lines = f.readlines()

# Find lines with addlenmod
for i, line in enumerate(lines):
    if "addlenmod(form," in line:
        # Print 5 lines before and after
        start = max(0, i-5)
        end = min(len(lines), i+3)
        print(f"=== Around line {i+1} ===")
        for j in range(start, end):
            marker = ">>>" if j == i else "   "
            print(f"{marker} L{j+1}: {lines[j]}", end="")
        print()
