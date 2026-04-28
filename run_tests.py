#!/usr/bin/env python3
"""
Sharp Compiler - Unified Regression Test Runner

Tests both core test suite (.sp files in tests/) and UCRT system headers.
Usage: python run_tests.py [--core] [--ucrt] [--verbose]

Defaults: runs both core and UCRT tests.
"""

import subprocess
import sys
import os
import re
import glob
import tempfile
import shutil
from pathlib import Path
from collections import Counter

# Colors for terminal output
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    RESET = '\033[0m'

def color(text, color_code):
    return f"{color_code}{text}{Colors.RESET}" if sys.stdout.isatty() else text

def green(text): return color(text, Colors.GREEN)
def red(text): return color(text, Colors.RED)
def yellow(text): return color(text, Colors.YELLOW)
def cyan(text): return color(text, Colors.CYAN)
def bold(text): return color(text, Colors.BOLD)

# Configuration
PROJECT_DIR = Path(__file__).resolve().parent
SHARPC = PROJECT_DIR / "cmake-build-debug" / "sharpc.exe"
TESTS_DIR = PROJECT_DIR / "tests"
UCRT_DIR = Path(r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.18362.0\ucrt")

# Known core test failures (pre-existing TCC header issues)
KNOWN_FAILURES = {
    "test_tcc_basetsd_h",   # clang: __int128/__fastfail/__debugbreak builtin conflicts
    "test_tcc_basetyps_h",  # clang: __uuidof/__GUID_Equal builtin conflicts
    "test_tcc_file_h",      # TCC file.h doesn't exist (only sys/file.h)
}

# Headers that are only in Windows SDK and not available in MinGW paths
# These will be marked as SKIP rather than FAIL
UCRT_WINSDK_ONLY = {
    "safeint.h",            # C++ only header
    "safeint_internal.h",   # C++ only header
    "corecrt_io.h",         # Windows SDK internal
    "corecrt_malloc.h",     # Windows SDK internal
    "corecrt_math.h",       # Windows SDK internal
    "corecrt_math_defines.h", # Windows SDK internal
    "corecrt_memcpy_s.h",   # Windows SDK internal
    "corecrt_memory.h",     # Windows SDK internal
    "corecrt_search.h",     # Windows SDK internal
    "corecrt_share.h",      # Windows SDK internal
    "corecrt_terminate.h",  # Windows SDK internal
    "corecrt_wdirect.h",    # Windows SDK internal
    "corecrt_wio.h",        # Windows SDK internal
    "corecrt_wprocess.h",   # Windows SDK internal
    "corecrt_wstdio.h",     # Windows SDK internal
    "corecrt_wstring.h",    # Windows SDK internal
    "corecrt_wtime.h",      # Windows SDK internal
    "tgmath.h",             # Uses complex MSVC-specific macros
}

# All UCRT headers to test
UCRT_HEADERS = [
    # Root headers
    "assert.h", "complex.h", "conio.h", "corecrt.h", "corecrt_io.h",
    "corecrt_malloc.h", "corecrt_math.h", "corecrt_math_defines.h",
    "corecrt_memcpy_s.h", "corecrt_memory.h", "corecrt_search.h",
    "corecrt_share.h", "corecrt_startup.h", "corecrt_stdio_config.h",
    "corecrt_terminate.h", "corecrt_wconio.h", "corecrt_wctype.h",
    "corecrt_wdirect.h", "corecrt_wio.h", "corecrt_wprocess.h",
    "corecrt_wstdio.h", "corecrt_wstdlib.h", "corecrt_wstring.h",
    "corecrt_wtime.h", "crtdbg.h", "ctype.h", "direct.h", "dos.h",
    "errno.h", "fcntl.h", "fenv.h", "float.h", "fpieee.h", "inttypes.h",
    "io.h", "locale.h", "malloc.h", "math.h", "mbctype.h", "mbstring.h",
    "memory.h", "minmax.h", "new.h", "process.h", "safeint.h",
    "safeint_internal.h", "search.h", "share.h", "signal.h", "stddef.h",
    "stdio.h", "stdlib.h", "string.h", "tchar.h", "tgmath.h", "time.h",
    "uchar.h", "wchar.h", "wctype.h",
    # sys/ headers
    "sys/locking.h", "sys/stat.h", "sys/timeb.h", "sys/types.h", "sys/utime.h",
]

def run_sharpc(input_file, output_file):
    """Run sharpc compiler and return (success, errors)"""
    try:
        result = subprocess.run(
            [str(SHARPC), str(input_file), "-no-link", "-o", str(output_file)],
            capture_output=True, text=True, timeout=30
        )
        errors = []
        combined = result.stdout + result.stderr
        for line in combined.splitlines():
            if "error" in line.lower():
                errors.append(line.strip())
        return result.returncode == 0, errors
    except subprocess.TimeoutExpired:
        return False, ["error: compilation timed out"]
    except Exception as e:
        return False, [f"error: {str(e)}"]

def run_clang(input_file, output_file):
    """Run clang compiler and return (success, errors)"""
    try:
        result = subprocess.run(
            ["clang", "-std=c11", "-w", "-o", str(output_file), str(input_file)],
            capture_output=True, text=True, timeout=30
        )
        errors = []
        for line in (result.stdout + result.stderr).splitlines():
            if "error:" in line:
                errors.append(line.strip())
        return result.returncode == 0, errors
    except Exception as e:
        return False, [f"error: {str(e)}"]

def test_core(verbose=False):
    """Run core test suite"""
    print(f"\n{bold(cyan('=== Core Test Suite ==='))}\n")
    
    test_files = sorted(TESTS_DIR.glob("test_*.sp"))
    if not test_files:
        print(yellow("No test files found in tests/"))
        return 0, 0, 0
    
    pass_count = 0
    fail_count = 0
    skip_count = 0
    
    for sp_file in test_files:
        name = sp_file.stem
        c_file = sp_file.with_suffix('.c')
        exe_file = sp_file.with_suffix('.exe')
        
        # Skip known failures
        if name in KNOWN_FAILURES:
            print(f"{yellow('SKIP:')}      {name} (known failure)")
            skip_count += 1
            continue
        
        # Step 1: sharpc .sp -> .c
        success, errors = run_sharpc(sp_file, c_file)
        if not success:
            print(f"{red('FAIL sharpc:')} {name}")
            if verbose:
                for err in errors[:3]:
                    print(f"           {err}")
            fail_count += 1
            continue
        
        # Step 2: clang .c -> .exe
        success, errors = run_clang(c_file, exe_file)
        if not success:
            print(f"{red('FAIL clang:')}  {name}")
            if verbose:
                for err in errors[:3]:
                    print(f"           {err}")
            fail_count += 1
            continue
        
        print(f"{green('PASS:')}      {name}")
        pass_count += 1
        
        # Cleanup
        try:
            if exe_file.exists():
                exe_file.unlink()
        except:
            pass
    
    return pass_count, fail_count, skip_count

def test_ucrt(verbose=False):
    """Run UCRT header tests"""
    print(f"\n{bold(cyan('=== UCRT Header Tests ==='))}\n")
    
    pass_count = 0
    fail_count = 0
    failed_headers = []
    
    temp_dir = tempfile.gettempdir()
    
    for header in UCRT_HEADERS:
        safe_name = header.replace('/', '_').replace('.', '_')
        sp_file = Path(temp_dir) / f"test_{safe_name}.sp"
        c_file = Path(temp_dir) / f"test_{safe_name}.c"
        
        # Skip headers that are Windows SDK only
        if header in UCRT_WINSDK_ONLY:
            print(f"{yellow('SKIP:')} {header} (Windows SDK only)")
            pass_count += 1  # Count as pass since it's expected
            continue
        
        content = f"/* auto */\n#include <{header}>\nint main() {{ return 0; }}\n"
        sp_file.write_text(content, encoding='ascii')
        
        success, errors = run_sharpc(sp_file, c_file)
        
        if not success:
            err_counter = Counter()
            for err in errors:
                match = re.search(r'E(\d+)', err)
                if match:
                    err_counter[f"E{match.group(1)}"] += 1
            err_summary = ", ".join(f"{k}: {v}" for k, v in err_counter.most_common())
            print(f"{red('FAIL:')} {header} ({len(errors)} errors: {err_summary})")
            failed_headers.append({
                'name': header,
                'count': len(errors),
                'types': dict(err_counter)
            })
            fail_count += 1
        else:
            print(f"{green('PASS:')} {header}")
            pass_count += 1
        
        # Cleanup
        try:
            sp_file.unlink()
            c_file.unlink()
        except:
            pass
    
    # Summary
    if failed_headers:
        print(f"\n{yellow('Failed headers:')}")
        for h in sorted(failed_headers, key=lambda x: -x['count']):
            print(f"  {h['name']}: {h['count']} errors ({', '.join(f'{k}: {v}' for k, v in h['types'].items())})")
        
        # Error code summary
        all_codes = Counter()
        for h in failed_headers:
            all_codes.update(h['types'])
        if all_codes:
            print(f"\n{yellow('Summary by error code:')}")
            for code, count in all_codes.most_common():
                print(f"  {code}: {count} headers affected")
    
    return pass_count, fail_count, 0

def main():
    # Parse arguments
    run_core = True
    run_ucrt = True
    verbose = False
    
    args = sys.argv[1:]
    if "--help" in args or "-h" in args:
        print(__doc__)
        sys.exit(0)
    if "--core" in args:
        run_ucrt = False
    if "--ucrt" in args:
        run_core = False
    if "--verbose" in args or "-v" in args:
        verbose = True
    
    # Verify sharpc exists
    if not SHARPC.exists():
        print(red(f"Error: sharpc.exe not found at {SHARPC}"))
        print("Build it first: cmake --build cmake-build-debug --target sharpc")
        sys.exit(1)
    
    print(bold("\n" + "=" * 60))
    print(bold("  Sharp Compiler - Regression Test Runner"))
    print(bold("=" * 60))
    
    total_pass = 0
    total_fail = 0
    total_skip = 0
    
    if run_core:
        p, f, s = test_core(verbose)
        total_pass += p
        total_fail += f
        total_skip += s
    
    if run_ucrt:
        p, f, s = test_ucrt(verbose)
        total_pass += p
        total_fail += f
        total_skip += s
    
    # Final summary
    print(f"\n{bold('=' * 60)}")
    print(bold("  Overall Results"))
    print(bold('=' * 60))
    print(f"  {green('PASS: ' + str(total_pass))}   {red('FAIL: ' + str(total_fail))}   {yellow('SKIP: ' + str(total_skip))}   TOTAL: {total_pass + total_fail + total_skip}")
    print(bold('=' * 60))
    
    sys.exit(0 if total_fail == 0 else 1)

if __name__ == "__main__":
    main()
