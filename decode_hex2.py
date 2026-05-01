hex_data = "29 29 3B 0A 20 62 72 65 61 6B 3B 0A 20 7D 0A 20 63 61 73 65 20 27 61 27 3A 20 63 61 73 65 20 27 41 27 3A 0A 20 63 68 65 63 6B 66 6F 72 6D 61 74 28 4C 2C 20 66 6F 72 6D 2C 20 22 22 2D 2B 23 30 20 22 2C 20 31 29 3B 0A 20 61 64 64 6C 65 6E 6D 6F 64 28 66 6F 72 6D 2C 20 22 22 29 3B 0A 20 6E 62 20 3D 20 6C 75 61 5F 6E 75 6D 62 65 72 32 73 74 72 78 28 4C 2C 20 62 75"

bytes_list = [int(b, 16) for b in hex_data.split()]

offset = 187240
print("Offset  HEX                                          ASCII")
print("=" * 80)

for i in range(0, len(bytes_list), 16):
    chunk = bytes_list[i:i+16]
    hex_str = " ".join(f"{b:02X}" for b in chunk)
    ascii_str = ""
    for b in chunk:
        if 32 <= b < 127:
            ascii_str += chr(b)
        elif b == 0x0A:
            ascii_str += "\\n"
        else:
            ascii_str += "."
    
    print(f"{offset+i:6d}  {hex_str:<48s}  {ascii_str}")
