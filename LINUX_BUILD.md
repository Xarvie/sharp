# Building and Testing Sharp Compiler on Linux

## Prerequisites
- GCC or Clang compiler (for building sharpc)
- CMake 3.10+
- Python 3 (for test runner)

## Build Instructions

```bash
# Configure and build
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug

# Or use Make directly
make
```

## Running Tests

```bash
# Run all tests (auto-detects platform)
python3 run_tests.py

# Run only core tests
python3 run_tests.py --core

# Run with verbose output
python3 run_tests.py --verbose
```

## Platform-Specific Notes

### Windows
- Uses libtcc (dynamic loading) for C-to-binary compilation
- UCRT header tests are available
- Windows-specific API tests are enabled

### Linux/macOS
- Uses clang/gcc for C-to-binary compilation
- UCRT header tests are skipped (Windows-only)
- Windows-specific API tests are automatically skipped
- TCC is optional (only needed if targeting Windows from Linux)

## Environment Variables
- `SHARPC_TCC_DIR`: Override TCC directory path (optional on Linux)
- `SHARPC_BUILD_DIR`: Override build directory path

## Cross-Compilation
```bash
# Target different platforms
sharpc input.sp --target x86_64-linux-gnu -o output
sharpc input.sp --target x86_64-windows-msvc -o output.exe
```
