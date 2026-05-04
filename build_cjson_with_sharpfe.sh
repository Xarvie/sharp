#!/usr/bin/env bash
# build_cjson_with_sharpfe.sh
# ─────────────────────────────────────────────────────────────────────────
# Phase R1 — second real-world C codebase through the full pipeline:
#   foo.c   ──[sharp_cli -P]──>  foo.i        (preprocessed C)
#   foo.i   ──[sharpc -P]────>   foo.fec.c    (parsed, sema-checked, regenerated C)
#   foo.fec.c ──[cc -c]──────>   foo.o
#
# Then link `cjson_test` and diff its output against the cc-only baseline.
# Success = output matches the baseline byte-for-byte.
#
# This complements build_lua_with_sharpfe.sh: Lua exercised GCC extensions
# and float-literal precision; cJSON has different idioms (heavy const,
# function pointers, recursive parsing, etc.) and is pure ISO C99.

set -uo pipefail

CJSON_VERSION="${CJSON_VERSION:-1.7.18}"
WORK_DIR="${WORK_DIR:-/tmp/cjson_sharpfe_build}"

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

# ─── locate tools ─────────────────────────────────────────────────────────
SHARP_CLI="${SHARP_CLI:-$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli}"
SHARPC="${SHARPC:-$REPO_ROOT/sharp-fe/sharpc}"
[ -x "$SHARP_CLI" ] || { echo "ERROR: $SHARP_CLI not built" >&2; exit 1; }
[ -x "$SHARPC"    ] || { echo "ERROR: $SHARPC not built"    >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
echo "Using sharpc:    $SHARPC"

# ─── locate cJSON source ──────────────────────────────────────────────────
if [ $# -ge 1 ]; then
    CJSON_DIR="$1"
else
    if [ ! -d "$WORK_DIR/cJSON-$CJSON_VERSION" ]; then
        echo "Downloading cJSON $CJSON_VERSION..."
        mkdir -p "$WORK_DIR"
        curl -sL "https://github.com/DaveGamble/cJSON/archive/refs/tags/v$CJSON_VERSION.tar.gz" \
            | tar xz -C "$WORK_DIR"
    fi
    CJSON_DIR="$WORK_DIR/cJSON-$CJSON_VERSION"
fi
[ -f "$CJSON_DIR/cJSON.c" ] || { echo "ERROR: $CJSON_DIR is not a cJSON source dir" >&2; exit 1; }
echo "cJSON source: $CJSON_DIR"

# ─── set up build dir ─────────────────────────────────────────────────────
BUILD_DIR="$WORK_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cp "$CJSON_DIR/cJSON.c" "$CJSON_DIR/cJSON.h" "$CJSON_DIR/test.c" "$BUILD_DIR/"

cd "$BUILD_DIR"

SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include"
SYSINCS="$SYSINCS -isystem /usr/local/include"
SYSINCS="$SYSINCS -isystem /usr/include/x86_64-linux-gnu"
SYSINCS="$SYSINCS -isystem /usr/include"

LOG_DIR="$BUILD_DIR/_logs"
mkdir -p "$LOG_DIR"

# ─── Phase 1: sharp-cpp preprocessing ─────────────────────────────────────
echo
echo "─── Phase 1: sharp-cpp preprocessing ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in cJSON.c test.c; do
    if "$SHARP_CLI" "$f" -P -I. $SYSINCS -o "${f%.c}.i" 2>"$LOG_DIR/${f%.c}.cpp.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1)); FAIL_FILES="$FAIL_FILES $f"
    fi
done
echo "  preprocess: $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    for f in $FAIL_FILES; do echo "  --- $f (cpp) ---"; head -8 "$LOG_DIR/${f%.c}.cpp.err"; done
    exit 1
fi

# ─── Phase 2: sharp-fe (sharpc) ───────────────────────────────────────────
echo
echo "─── Phase 2: sharp-fe parse / sema / cg ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in cJSON.i test.i; do
    base="${f%.i}"
    if "$SHARPC" "$f" -P -I. $SYSINCS -o "$base.fec.c" 2>"$LOG_DIR/$base.fe.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1)); FAIL_FILES="$FAIL_FILES $base.c"
    fi
done
echo "  fe:         $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    for f in $FAIL_FILES; do echo "  --- $f (fe) ---"; head -15 "$LOG_DIR/${f%.c}.fe.err"; done
    exit 1
fi

# ─── Phase 3: cc compile ──────────────────────────────────────────────────
echo
echo "─── Phase 3: cc compile ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in cJSON.fec.c test.fec.c; do
    base="${f%.fec.c}"
    if cc -std=gnu99 -O2 -w -c "$f" -o "$base.o" 2>"$LOG_DIR/$base.cc.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1)); FAIL_FILES="$FAIL_FILES $base.c"
    fi
done
echo "  compile:    $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    for f in $FAIL_FILES; do echo "  --- $f (cc) ---"; head -15 "$LOG_DIR/${f%.c}.cc.err"; done
    exit 1
fi

# ─── Phase 4: link & smoke ────────────────────────────────────────────────
echo
echo "─── Phase 4: link ───"
cc -std=gnu99 -o cjson_test cJSON.o test.o -lm
echo "  built: $(ls -la cjson_test | awk '{print $9, $5"B"}')"

echo
echo "─── Phase 5: output diff vs cc-only baseline ───"
./cjson_test > "$BUILD_DIR/cjson_test.out" 2>&1
BASELINE="${BASELINE:-/tmp/cjson_baseline.out}"
if [ ! -f "$BASELINE" ]; then
    echo "  NOTE: no baseline at $BASELINE; producing one with cc-only build"
    cc -std=gnu99 -O2 -w "$CJSON_DIR/cJSON.c" "$CJSON_DIR/test.c" -o /tmp/_cjson_baseline_tmp -lm
    /tmp/_cjson_baseline_tmp > "$BASELINE" 2>&1
    rm -f /tmp/_cjson_baseline_tmp
fi
if diff -u "$BASELINE" "$BUILD_DIR/cjson_test.out" > "$BUILD_DIR/_logs/diff.out"; then
    echo "  output matches baseline byte-for-byte ✓"
else
    echo "  output DIFFERS from baseline ✗"
    echo "  first 40 lines of diff:"
    head -40 "$BUILD_DIR/_logs/diff.out"
    exit 1
fi

echo
echo "Build complete.  Binary at: $BUILD_DIR/cjson_test"
