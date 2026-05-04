#!/usr/bin/env bash
# build_picol_with_sharpfe.sh
# ─────────────────────────────────────────────────────────────────────────
# Phase R2 — third real-world C codebase through the full pipeline.
#
# picol is a ~800-line single-file C implementation of a TCL interpreter
# by Salvatore Sanfilippo.  It exercises stdarg/printf, function-pointer
# tables, manual string parsing, recursive eval — a different surface
# from Lua's GCC-extension idioms (S5) and cJSON's const-discipline
# OO-style (R1).
#
# Pipeline:
#   picol.c   ──[sharp_cli -P]──>  picol.i
#   picol.i   ──[sharpc -P]────>   picol.fec.c
#   picol.fec.c ──[cc -c]──────>   picol.o
#   picol.o   ──[cc]──────────>   picol_bin
#
# Smoke: a small TCL script exercising arithmetic / proc / while / fact.
# Success criterion: output matches the cc-only baseline byte-for-byte.

set -uo pipefail

PICOL_REVISION="${PICOL_REVISION:-master}"
WORK_DIR="${WORK_DIR:-/tmp/picol_sharpfe_build}"

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SHARP_CLI="${SHARP_CLI:-$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli}"
SHARPC="${SHARPC:-$REPO_ROOT/sharp-fe/sharpc}"
[ -x "$SHARP_CLI" ] || { echo "ERROR: $SHARP_CLI not built" >&2; exit 1; }
[ -x "$SHARPC"    ] || { echo "ERROR: $SHARPC not built"    >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
echo "Using sharpc:    $SHARPC"

# ─── locate picol source ──────────────────────────────────────────────────
if [ $# -ge 1 ]; then
    PICOL_C="$1"
else
    PICOL_C="$WORK_DIR/picol.c"
    if [ ! -f "$PICOL_C" ]; then
        echo "Downloading picol ($PICOL_REVISION)..."
        mkdir -p "$WORK_DIR"
        curl -sL "https://raw.githubusercontent.com/antirez/picol/$PICOL_REVISION/picol.c" -o "$PICOL_C"
    fi
fi
[ -f "$PICOL_C" ] || { echo "ERROR: $PICOL_C missing" >&2; exit 1; }
echo "picol source: $PICOL_C"

# ─── set up build dir ─────────────────────────────────────────────────────
BUILD_DIR="$WORK_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cp "$PICOL_C" "$BUILD_DIR/picol.c"

# ─── canonical TCL smoke test ─────────────────────────────────────────────
cat > "$BUILD_DIR/smoke.tcl" <<'TCL'
set x 7
set y 5
set z [expr $x + $y]
puts "x=$x y=$y z=$z"
proc fact n {
    if {[expr $n <= 1]} { return 1 }
    return [expr $n * [fact [expr $n - 1]]]
}
puts "fact(6) = [fact 6]"
puts "fact(10) = [fact 10]"
set i 0
set s 0
while {[expr $i < 100]} {
    set s [expr $s + $i]
    set i [expr $i + 1]
}
puts "sum 0..99 = $s"
TCL

cd "$BUILD_DIR"

SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include"
SYSINCS="$SYSINCS -isystem /usr/local/include"
SYSINCS="$SYSINCS -isystem /usr/include/x86_64-linux-gnu"
SYSINCS="$SYSINCS -isystem /usr/include"

LOG_DIR="$BUILD_DIR/_logs"
mkdir -p "$LOG_DIR"

# ─── Phase 1: sharp-cpp ───────────────────────────────────────────────────
echo
echo "─── Phase 1: sharp-cpp preprocessing ───"
if "$SHARP_CLI" picol.c -P -I. $SYSINCS -o picol.i 2>"$LOG_DIR/picol.cpp.err"; then
    echo "  preprocess: 1 pass, 0 fail"
else
    echo "  preprocess: 0 pass, 1 fail"
    head -10 "$LOG_DIR/picol.cpp.err"
    exit 1
fi

# ─── Phase 2: sharp-fe ────────────────────────────────────────────────────
echo
echo "─── Phase 2: sharp-fe parse / sema / cg ───"
if "$SHARPC" picol.i -P -I. $SYSINCS -o picol.fec.c 2>"$LOG_DIR/picol.fe.err"; then
    echo "  fe:         1 pass, 0 fail"
else
    echo "  fe:         0 pass, 1 fail"
    head -25 "$LOG_DIR/picol.fe.err"
    exit 1
fi

# ─── Phase 3: cc compile ──────────────────────────────────────────────────
echo
echo "─── Phase 3: cc compile ───"
if cc -std=gnu99 -O2 -w -c picol.fec.c -o picol.o 2>"$LOG_DIR/picol.cc.err"; then
    echo "  compile:    1 pass, 0 fail"
else
    echo "  compile:    0 pass, 1 fail"
    head -25 "$LOG_DIR/picol.cc.err"
    exit 1
fi

# ─── Phase 4: link ────────────────────────────────────────────────────────
echo
echo "─── Phase 4: link ───"
cc -std=gnu99 -o picol_bin picol.o
echo "  built: $(ls -la picol_bin | awk '{print $9, $5"B"}')"

# ─── Phase 5: smoke test + output diff ────────────────────────────────────
echo
echo "─── Phase 5: TCL smoke test + diff vs cc-only baseline ───"
./picol_bin smoke.tcl > picol.out 2>&1
BASELINE="$BUILD_DIR/picol.baseline.out"
cc -std=c99 -O2 -w picol.c -o picol_baseline_bin
./picol_baseline_bin smoke.tcl > "$BASELINE" 2>&1

if diff -u "$BASELINE" picol.out > "$LOG_DIR/diff.out"; then
    echo "  output matches baseline byte-for-byte ✓"
    cat picol.out | sed 's/^/    | /'
else
    echo "  output DIFFERS from baseline ✗"
    head -40 "$LOG_DIR/diff.out"
    exit 1
fi

echo
echo "Build complete.  Binary at: $BUILD_DIR/picol_bin"
