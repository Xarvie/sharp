import sys
data = open('test/output/lua_pp/lstrlib.pp.c', 'rb').read()
positions = [180699, 180839, 182247, 185653, 185957]
for pos in positions:
    b1 = data[pos]
    b2 = data[pos+1] if pos+1 < len(data) else 0
    is_empty_str = (b1 == 0x22 and b2 == 0x22)
    status = "IS_EMPTY_STR" if is_empty_str else "NOT_EMPTY"
    print(f"pos={pos}: {status}")
    start = max(0, pos - 15)
    ctx = data[start:pos+15].decode('utf-8', errors='replace')
    print(f"  Context: ...{ctx}...")
