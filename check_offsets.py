with open(r"c:\Users\ftp\Desktop\sharp\test\lua-5.4.6\src\lstrlib.pp.c", "rb") as f:
    data = f.read()

# The raw buffer is in _raw_text, but phase6 reads from it before transfer.
# Let's check what's at offset 187330 and nearby
for offset in [187320, 187325, 187330, 187335, 187625, 187630, 187634, 187640]:
    if offset < len(data):
        chunk = data[offset:offset+20]
        print(f"Offset {offset}: {chunk!r}")
    else:
        print(f"Offset {offset}: out of bounds (file len={len(data)})")
