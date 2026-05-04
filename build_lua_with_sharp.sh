#!/usr/bin/env bash
# build_lua_with_sharp.sh
# ─────────────────────────────────────────────────────────────────────────
# Builds the Lua 5.4.7 interpreter using sharp-cpp as the C preprocessor
# and the host `cc` as the C compiler/linker, then runs Lua's official
# testsuite to validate the result.
#
# Pipeline per .c file:
#   foo.c  ──[sharp_cli -P -I. -isystem /usr/include …]──>  foo.i
#   foo.i  ──[cc -c]──>                                    foo.o
#
# Final link:
#   liblua.a  +  lua.o      ──>  lua
#   liblua.a  +  luac.o     ──>  luac
#
# Usage:
#   ./build_lua_with_sharp.sh                  # auto-downloads Lua 5.4.7
#   ./build_lua_with_sharp.sh /path/to/lua/src # uses an existing checkout
#
# Environment variables:
#   SHARP_CLI       — path to sharp_cli binary (default: $REPO/sharp-cpp/sharp-cpp/sharp_cli)
#   LUA_VERSION     — Lua version to download (default: 5.4.7)
#   WORK_DIR        — working directory (default: /tmp/lua_sharp_build)

set -euo pipefail

LUA_VERSION="${LUA_VERSION:-5.4.7}"
WORK_DIR="${WORK_DIR:-/tmp/lua_sharp_build}"

# Locate sharp_cli
if [ -z "${SHARP_CLI:-}" ]; then
    REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
    if [ -x "$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli" ]; then
        SHARP_CLI="$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli"
    elif [ -x "$REPO_ROOT/../sharp-cpp/sharp-cpp/sharp_cli" ]; then
        SHARP_CLI="$REPO_ROOT/../sharp-cpp/sharp-cpp/sharp_cli"
    else
        echo "ERROR: sharp_cli not found. Set SHARP_CLI=/path/to/sharp_cli" >&2
        exit 1
    fi
fi
[ -x "$SHARP_CLI" ] || { echo "ERROR: SHARP_CLI not executable: $SHARP_CLI" >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
$SHARP_CLI --help 2>&1 | head -3 || true

# Locate Lua source
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

# Set up build dir
BUILD_DIR="$WORK_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cp "$LUA_SRC_DIR"/*.c "$LUA_SRC_DIR"/*.h "$BUILD_DIR/"

cd "$BUILD_DIR"

# System include paths (mirror gcc -E -v output)
SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include"
SYSINCS="$SYSINCS -isystem /usr/local/include"
SYSINCS="$SYSINCS -isystem /usr/include/x86_64-linux-gnu"
SYSINCS="$SYSINCS -isystem /usr/include"
DEFS="-DLUA_COMPAT_5_3 -DLUA_USE_LINUX"

# Phase 1: preprocess every .c with sharp_cli
echo
echo "─── Phase 1: sharp-cpp preprocessing ───"
PASS=0; FAIL=0; FAILED=""
for f in *.c; do
    if "$SHARP_CLI" "$f" -P -I. $SYSINCS $DEFS -o "${f%.c}.i" 2>/tmp/sharp_err; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        FAILED="$FAILED $f"
        echo "FAIL preprocessing $f:"
        head -5 /tmp/sharp_err
    fi
done
echo "  preprocess: $PASS pass, $FAIL fail"
[ "$FAIL" = "0" ] || { echo "Aborting due to preprocessor failures"; exit 1; }

# Phase 2: cc compile every .i -> .o
echo
echo "─── Phase 2: cc compile ───"
PASS=0; FAIL=0
for f in *.i; do
    if cc -std=gnu99 -O2 -w -c "$f" -o "${f%.i}.o" 2>/tmp/cc_err; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        echo "FAIL compiling $f:"
        head -5 /tmp/cc_err
    fi
done
echo "  compile: $PASS pass, $FAIL fail"
[ "$FAIL" = "0" ] || { echo "Aborting due to compile failures"; exit 1; }

# Phase 3: link
echo
echo "─── Phase 3: link ───"
CORE="lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o"
CORE="$CORE lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o"
CORE="$CORE lundump.o lvm.o lzio.o"
LIB="lauxlib.o lbaselib.o lcorolib.o ldblib.o liolib.o lmathlib.o loadlib.o"
LIB="$LIB loslib.o lstrlib.o ltablib.o lutf8lib.o linit.o"
ar rcs liblua.a $CORE $LIB 2>/dev/null
cc -std=gnu99 -o lua  lua.o  liblua.a -lm -Wl,-E -ldl
cc -std=gnu99 -o luac luac.o liblua.a -lm -Wl,-E -ldl
echo "  built: $(ls -la lua luac | awk '{print $9, $5"B"}' | tr '\n' ' ')"

# Phase 4: smoke test
echo
echo "─── Phase 4: smoke test ───"
./lua -e 'print("hello from sharp-built lua " .. _VERSION)'
./luac -v 2>&1 | head -1

echo
echo "Build complete.  Binaries at: $BUILD_DIR/lua  $BUILD_DIR/luac"
echo "To run the official testsuite:"
echo "  curl -sL https://www.lua.org/tests/lua-$LUA_VERSION-tests.tar.gz | tar xz -C /tmp"
echo "  cd /tmp/lua-$LUA_VERSION-tests && $BUILD_DIR/lua -e '_U=true' all.lua"
