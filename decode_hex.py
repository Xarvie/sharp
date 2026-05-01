hex_data = "63 6B 66 6F 72 6D 61 74 28 4C 2C 20 66 6F 72 6D 2C 20 22 22 2D 2B 23 30 20 22 2C 20 31 29 3B 0A 20 61 64 64 6C 65 6E 6D 6F 64 28 66 6F 72 6D 2C 20 22 22 29 3B 0A 20 6E 62 20 3D 20 6C 75 61 5F 6E 75 6D 62 65 72 32 73 74 72 78 28 4C 2C 20 62 75"

bytes_list = [int(b, 16) for b in hex_data.split()]

print("Offset  ASCII        HEX")
print("=" * 60)
ascii_line = ""
for i, byte in enumerate(bytes_list):
    offset = 187280 + i
    if i % 16 == 0:
        if ascii_line:
            print(f"  {ascii_line}")
        print(f"{offset:6d}  ", end="")
        ascii_line = ""
    print(f"{byte:02X} ", end="")
    if 32 <= byte < 127:
        ascii_line += chr(byte)
    elif byte == 0x0A:
        ascii_line += "\\n"
    elif byte == 0x22:
        ascii_line += '"'
    else:
        ascii_line += "."
    
    if (i + 1) % 16 == 0:
        print(f"  {ascii_line}")
        ascii_line = ""

if ascii_line:
    # pad remaining
    remaining = 16 - (len(bytes_list) % 16)
    print("   " * remaining + f"  {ascii_line}")
