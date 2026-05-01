import mmap

with open(r"c:\Users\ftp\Desktop\sharp\test\lua-5.4.6\src\lstrlib.pp.c", "rb") as f:
    data = f.read()

# Find all occurrences of "addlenmod(form,"
needle = b"addlenmod(form,"
pos = 0
while True:
    idx = data.find(needle, pos)
    if idx == -1:
        break
    # Show 60 bytes starting from the match
    context = data[idx:idx+60]
    print(f"Offset {idx}: {context!r}")
    pos = idx + 1
