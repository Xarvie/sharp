import sys

data = open('test/output/lua_pp/lstrlib.pp.c', 'rb').read()
content = data.decode('utf-8', errors='replace')

# Find all addlenmod calls
idx = 0
count = 0
while True:
    idx = content.find('addlenmod(form, )', idx)
    if idx < 0:
        break
    count += 1
    # Get 100 chars before and after
    start = max(0, idx - 50)
    end = min(len(content), idx + 100)
    context = content[start:end].replace('\n', '\\n')
    print(f"addlenmod(form, ) at offset {idx}:")
    print(f"  Context: ...{context}...")
    
    # Check if there's a "" within 30 bytes before the ");"
    before_paren = content[max(0, idx-5):idx]
    print(f"  Before ');': [{before_paren}]")
    
    # Check bytes
    raw_before = data[max(0, idx-5):idx]
    print(f"  Raw bytes: {list(raw_before)}")
    print()
    
    idx += 1
    if count > 10:
        break

if count == 0:
    print("No 'addlenmod(form, )' found - all correctly expanded!")
else:
    print(f"Found {count} occurrences of 'addlenmod(form, )'")
