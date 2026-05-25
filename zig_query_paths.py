#!/usr/bin/env python3
"""Query zig cc for include search paths for all known targets."""
import subprocess
import sys
import json

ZIG = r"C:\env\zig\zig.exe"

TARGETS = [
    None,  # native (no target)
    "x86_64-windows",
    "aarch64-windows",
    "x86_64-linux-gnu",
    "aarch64-linux-gnu",
    "x86_64-linux-musl",
    "aarch64-linux-musl",
    "x86_64-macos",
    "aarch64-macos",
    "x86_64-freebsd",
    "aarch64-freebsd",
    "x86_64-netbsd",
    "aarch64-netbsd",
    "x86_64-openbsd",
    "aarch64-openbsd",
    "wasm32-wasi-musl",
    "wasm32-freestanding",
    "x86_64-ios",
    "aarch64-ios",
    "x86_64-ios-simulator",
    "aarch64-ios-simulator",
]

def query(target):
    """Run zig cc -E -v -xc <empty> [-target T] and return include dirs."""
    import tempfile
    import os
    
    # Create temp empty file
    with tempfile.NamedTemporaryFile(suffix=".c", delete=False) as f:
        tmp = f.name
    
    cmd = [ZIG, "cc", "-E", "-v", "-xc", tmp]
    if target:
        cmd.extend(["-target", target])
    
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=10
        )
    except subprocess.TimeoutExpired:
        return ["TIMEOUT"]
    finally:
        os.unlink(tmp)
    
    # Parse stderr
    output = result.stderr
    lines = output.split("\n")
    dirs = []
    searching = False
    for line in lines:
        if "#include <..." in line:
            searching = True
            continue
        if "End of search list" in line:
            break
        if searching and line.startswith(" "):
            dirs.append(line.strip())
    
    return dirs

if __name__ == "__main__":
    results = {}
    for t in TARGETS:
        label = t if t else "native"
        print(f"Querying: {label}...")
        dirs = query(t)
        results[label] = dirs
        for d in dirs:
            print(f"  {d}")
        print()
    
    # Also output as C code
    print("\n\n=== C code for cpp_detect_target_sys_paths ===")
    native = results.get("native", [])
    print("    /* Native / no-target: */")
    for d in native:
        print(f'        add_sys_path_if_valid(ctx, "{d}");')
    print()
    
    for label, dirs in results.items():
        if label == "native":
            continue
        target_os = label.split("-")[-1] if "-" in label else label
        # Map to OS name
        if "macos" in label or "ios" in label:
            target_os = "macos"
        elif "linux" in label:
            target_os = "linux"
        elif "windows" in label:
            target_os = "windows"
        elif "freebsd" in label:
            target_os = "freebsd"
        elif "netbsd" in label:
            target_os = "netbsd"
        elif "openbsd" in label:
            target_os = "openbsd"
        elif "wasi" in label:
            target_os = "wasi"
        
        print(f"    /* target: {label} (os={target_os}) */")
        for d in dirs:
            print(f'        add_sys_path_if_valid(ctx, "{d}");')
        print()
    
    # Save full JSON
    with open("zig_include_paths.json", "w") as f:
        json.dump(results, f, indent=2)
    print("Saved zig_include_paths.json")
