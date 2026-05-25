#!/bin/bash
# macOS build script for Sharp compiler
set -e

echo "=== Sharp Compiler — macOS Build ==="
echo ""

cmake -B build-macos -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build-macos

echo ""
echo "Build complete: build-macos/sharpc"
echo ""
echo "Run tests:"
echo "  python3 sharp-test/run_regression.py"