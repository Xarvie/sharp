#!/usr/bin/env bash
# build_lua_with_sharpfe.sh
# ─────────────────────────────────────────────────────────────────────────
# Phase S5 — full pipeline integration:
#   foo.c  ──[sharp_cli -P]──>  foo.i        (preprocessed C)
#   foo.i  ──[sharpc -P]───>    foo.fec.c    (parsed, sema-checked, regenerated C)
#   foo.fec.c ──[cc -c]────>    foo.o
#
# Final link → lua, luac.  Then the Lua 5.4.7 testsuite runs under the
# resulting interpreter.  Success = `final OK !!!`.
#
# This is the sister script to build_lua_with_sharp.sh, which routes
# only sharp-cpp + cc.  By contrast this script exercises the *entire*
# Sharp toolchain (preprocessor + frontend) on real-world C source.
#
# Usage:
#   ./build_lua_with_sharpfe.sh                  # auto-downloads Lua 5.4.7
#   ./build_lua_with_sharpfe.sh /path/to/lua/src # uses an existing checkout
#
# Environment variables:
#   SHARP_CLI       — path to sharp_cli (cpp)
#   SHARPC          — path to sharpc (fe driver; runs cpp internally too,
#                                     but we feed it the .i so it is
#                                     idempotent)
#   LUA_VERSION     — Lua version (default 5.4.7)
#   WORK_DIR        — working dir (default /tmp/lua_sharpfe_build)
#   RUN_TESTSUITE   — if 1, also runs lua-5.4.7-tests at the end
#                     (default 1)

set -uo pipefail

LUA_VERSION="${LUA_VERSION:-5.4.7}"
WORK_DIR="${WORK_DIR:-/tmp/lua_sharpfe_build}"
RUN_TESTSUITE="${RUN_TESTSUITE:-1}"

# ─── locate tools ─────────────────────────────────────────────────────────
REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

if [ -z "${SHARP_CLI:-}" ]; then
    if [ -x "$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli" ]; then
        SHARP_CLI="$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli"
    else
        echo "ERROR: sharp_cli not found." >&2
        echo "       Build it: (cd sharp-cpp/sharp-cpp && make sharp_cli)" >&2
        exit 1
    fi
fi
if [ -z "${SHARPC:-}" ]; then
    if [ -x "$REPO_ROOT/sharp-fe/sharpc" ]; then
        SHARPC="$REPO_ROOT/sharp-fe/sharpc"
    else
        echo "ERROR: sharpc not found." >&2
        echo "       Build it: (cd sharp-fe && make sharpc)" >&2
        exit 1
    fi
fi
[ -x "$SHARP_CLI" ] || { echo "ERROR: sharp_cli not executable" >&2; exit 1; }
[ -x "$SHARPC"    ] || { echo "ERROR: sharpc not executable" >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
echo "Using sharpc:    $SHARPC"

# ─── locate Lua source ────────────────────────────────────────────────────
if [ $# -ge 1 ]; then
    LUA_SRC_DIR="$1"
else
    if [ ! -d "$WORK_DIR/lua-$LUA_VERSION" ]; then
        echo "Downloading Lua $LUA_VERSION..."
        mkdir -p "$WORK_DIR"
        curl -sL "https://www.lua.org/ftp/lua-$LUA_VERSION.tar.gz" \
            | tar xz -C "$WORK_DIR"
    fi
    LUA_SRC_DIR="$WORK_DIR/lua-$LUA_VERSION/src"
fi
[ -f "$LUA_SRC_DIR/lapi.c" ] || { echo "ERROR: $LUA_SRC_DIR is not a Lua source dir" >&2; exit 1; }
echo "Lua source: $LUA_SRC_DIR"

# ─── set up build dir ─────────────────────────────────────────────────────
BUILD_DIR="$WORK_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cp "$LUA_SRC_DIR"/*.c "$LUA_SRC_DIR"/*.h "$BUILD_DIR/"

cd "$BUILD_DIR"

# System include paths (mirror gcc -E -v on this host).
SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include"
SYSINCS="$SYSINCS -isystem /usr/local/include"
SYSINCS="$SYSINCS -isystem /usr/include/x86_64-linux-gnu"
SYSINCS="$SYSINCS -isystem /usr/include"
DEFS="-DLUA_COMPAT_5_3 -DLUA_USE_LINUX"

LOG_DIR="$BUILD_DIR/_logs"
mkdir -p "$LOG_DIR"

# ─── Phase 1: sharp-cpp preprocessing ─────────────────────────────────────
echo
echo "─── Phase 1: sharp-cpp preprocessing ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in *.c; do
    if "$SHARP_CLI" "$f" -P -I. $SYSINCS $DEFS -o "${f%.c}.i" 2>"$LOG_DIR/${f%.c}.cpp.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        FAIL_FILES="$FAIL_FILES $f"
    fi
done
echo "  preprocess: $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    echo "  failed:$FAIL_FILES"
    for f in $FAIL_FILES; do
        echo "  --- $f (cpp) ---"
        head -5 "$LOG_DIR/${f%.c}.cpp.err"
    done
    echo "Aborting due to preprocessor failures"
    exit 1
fi

# ─── Phase 2: sharp-fe (sharpc) parse / sema / cg ─────────────────────────
# Feed each .i back through sharpc (it will run cpp again on the already-
# preprocessed text — idempotent), and emit regenerated C to .fec.c.
echo
echo "─── Phase 2: sharp-fe (sharpc) parse / sema / cg ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in *.i; do
    base="${f%.i}"
    if "$SHARPC" "$f" -P -I. $SYSINCS $DEFS -o "$base.fec.c" 2>"$LOG_DIR/$base.fe.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        FAIL_FILES="$FAIL_FILES $base.c"
    fi
done
echo "  fe:         $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    echo "  failed:$FAIL_FILES"
    for f in $FAIL_FILES; do
        echo "  --- $f (fe) ---"
        head -8 "$LOG_DIR/${f%.c}.fe.err"
    done
    echo "Aborting due to frontend failures"
    exit 1
fi

# ─── Phase 3: cc compile every .fec.c -> .o ───────────────────────────────
echo
echo "─── Phase 3: cc compile ───"
PASS=0; FAIL=0; FAIL_FILES=""
for f in *.fec.c; do
    base="${f%.fec.c}"
    if cc -std=gnu99 -O2 -w -c "$f" -o "$base.o" 2>"$LOG_DIR/$base.cc.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        FAIL_FILES="$FAIL_FILES $base.c"
    fi
done
echo "  compile:    $PASS pass, $FAIL fail"
if [ "$FAIL" -ne 0 ]; then
    echo "  failed:$FAIL_FILES"
    for f in $FAIL_FILES; do
        echo "  --- $f (cc) ---"
        head -10 "$LOG_DIR/${f%.c}.cc.err"
    done
    echo "Aborting due to cc failures"
    exit 1
fi

# ─── Phase 4: link ────────────────────────────────────────────────────────
echo
echo "─── Phase 4: link ───"
CORE="lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o"
CORE="$CORE lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o"
CORE="$CORE lundump.o lvm.o lzio.o"
LIB="lauxlib.o lbaselib.o lcorolib.o ldblib.o liolib.o lmathlib.o loadlib.o"
LIB="$LIB loslib.o lstrlib.o ltablib.o lutf8lib.o linit.o"
ar rcs liblua.a $CORE $LIB 2>/dev/null
cc -std=gnu99 -o lua  lua.o  liblua.a -lm -Wl,-E -ldl
cc -std=gnu99 -o luac luac.o liblua.a -lm -Wl,-E -ldl
echo "  built: $(ls -la lua luac | awk '{print $9, $5"B"}' | tr '\n' ' ')"

# ─── Phase 5: smoke test ──────────────────────────────────────────────────
echo
echo "─── Phase 5: smoke test ───"
./lua -e 'print("hello from sharpc-built lua " .. _VERSION)'
./luac -v 2>&1 | head -1

# ─── Phase 6: full Lua testsuite (optional) ───────────────────────────────
if [ "$RUN_TESTSUITE" = "1" ]; then
    TEST_DIR="${TEST_DIR:-/tmp/lua-$LUA_VERSION-tests}"
    if [ ! -d "$TEST_DIR" ]; then
        echo
        echo "Downloading Lua $LUA_VERSION testsuite..."
        curl -sL "https://www.lua.org/tests/lua-$LUA_VERSION-tests.tar.gz" \
            | tar xz -C "$(dirname "$TEST_DIR")"
    fi
    echo
    echo "─── Phase 6: lua-$LUA_VERSION-tests ───"
    cd "$TEST_DIR"
    if timeout 600 "$BUILD_DIR/lua" -e '_U=true' all.lua 2>&1 | tee "$BUILD_DIR/_logs/testsuite.log" | tail -5 ; then
        :
    fi
    if grep -q "^final OK !!!" "$BUILD_DIR/_logs/testsuite.log"; then
        echo
        echo "  TESTSUITE: final OK !!!  ✓"
    else
        echo
        echo "  TESTSUITE: did NOT reach 'final OK !!!'  ✗"
        echo "  See: $BUILD_DIR/_logs/testsuite.log"
        exit 1
    fi
fi

echo
echo "Build complete.  Binaries at: $BUILD_DIR/lua  $BUILD_DIR/luac"
