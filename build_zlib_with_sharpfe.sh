#!/usr/bin/env bash
# build_zlib_with_sharpfe.sh
# ─────────────────────────────────────────────────────────────────────────
# Phase R3 — fifth real-world C codebase through the full pipeline.
#
# zlib 1.3.1 is the canonical compression library.  Roughly 22K lines
# across 11 .c files, mature ANSI/ISO C90 style, used in essentially
# every networked / archival / build-system on the planet.  Validation
# here is end-to-end: deflate a known-good string + inflate back +
# byte-compare with cc-only baseline.
#
# Pipeline:
#   *.c → sharp_cli -P → *.i → sharpc -P → *.fec.c → cc -c → *.o → cc → bin
#
# Smoke: round-trip a 122-byte test string through compress/uncompress.

set -uo pipefail

WORK_DIR="${WORK_DIR:-/tmp/zlib_sharpfe_build}"
ZLIB_TAG="${ZLIB_TAG:-v1.3.1}"

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SHARP_CLI="${SHARP_CLI:-$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli}"
SHARPC="${SHARPC:-$REPO_ROOT/sharp-fe/sharpc}"
[ -x "$SHARP_CLI" ] || { echo "ERROR: $SHARP_CLI not built" >&2; exit 1; }
[ -x "$SHARPC"    ] || { echo "ERROR: $SHARPC not built"    >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
echo "Using sharpc:    $SHARPC"

# ─── locate / fetch zlib ─────────────────────────────────────────────────
SRC_DIR="$WORK_DIR/src"
mkdir -p "$SRC_DIR"
if [ ! -f "$SRC_DIR/deflate.c" ]; then
    echo "Fetching zlib $ZLIB_TAG ..."
    curl -sL "https://github.com/madler/zlib/archive/refs/tags/$ZLIB_TAG.tar.gz" \
        | tar -xzC "$SRC_DIR" --strip-components=1
fi
echo "zlib source in: $SRC_DIR"

# ─── round-trip driver ───────────────────────────────────────────────────
cat > "$SRC_DIR/zlib_driver.c" <<'EOF'
#include "zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    const char *src = "the quick brown fox jumps over the lazy dog. "
                      "the quick brown fox jumps over the lazy dog. "
                      "0123456789 0123456789 0123456789";
    uLong src_len  = (uLong)strlen(src);
    uLong dest_cap = compressBound(src_len);
    unsigned char *dest = (unsigned char *)malloc(dest_cap);
    uLong dest_len = dest_cap;
    int rc = compress(dest, &dest_len, (const Bytef *)src, src_len);
    if (rc != Z_OK) { puts("FAIL: compress"); return 1; }

    unsigned char *back = (unsigned char *)malloc(src_len + 1);
    uLong back_len = src_len + 1;
    rc = uncompress(back, &back_len, dest, dest_len);
    if (rc != Z_OK) { puts("FAIL: uncompress"); return 2; }

    if (back_len != src_len || memcmp(src, back, src_len) != 0) {
        puts("FAIL: round-trip mismatch"); return 3;
    }

    unsigned long checksum = 0;
    for (uLong i = 0; i < dest_len; i++) checksum += dest[i];
    printf("OK src_len=%lu compressed=%lu back=%lu csum=%lu zlib=%s\n",
           src_len, dest_len, back_len, checksum, zlibVersion());
    free(dest); free(back);
    return 0;
}
EOF

cd "$SRC_DIR"
ZLIB_SRCS="adler32.c crc32.c deflate.c trees.c zutil.c inflate.c inftrees.c inffast.c compress.c uncompr.c"
ALL_SRCS="zlib_driver.c $ZLIB_SRCS"

SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include -isystem /usr/local/include -isystem /usr/include/x86_64-linux-gnu -isystem /usr/include"
DEFINES="-DZ_HAVE_UNISTD_H"

LOG_DIR="$WORK_DIR/_logs"; mkdir -p "$LOG_DIR"
mkdir -p "$SRC_DIR/out"

# ─── Phase 1: cc-only baseline ───────────────────────────────────────────
echo
echo "─── Phase 1: cc-only baseline ───"
cc -std=c99 -O2 -w $DEFINES $ALL_SRCS -o zlib_baseline
./zlib_baseline > "$WORK_DIR/baseline.out" 2>&1
echo "  baseline: $(cat $WORK_DIR/baseline.out)"

# ─── Phase 2: sharp-cpp ──────────────────────────────────────────────────
echo
echo "─── Phase 2: sharp-cpp preprocessing ───"
PASS=0; FAIL=0
for s in $ALL_SRCS; do
    if "$SHARP_CLI" $s -P -I. $SYSINCS $DEFINES -o out/${s%.c}.i 2>"$LOG_DIR/${s%.c}.cpp.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        echo "  cpp FAIL: $s"; head -5 "$LOG_DIR/${s%.c}.cpp.err"
    fi
done
echo "  preprocess: $PASS pass, $FAIL fail"
[ $FAIL -eq 0 ] || exit 1

# ─── Phase 3: sharp-fe ──────────────────────────────────────────────────
echo
echo "─── Phase 3: sharp-fe parse / sema / cg ───"
PASS=0; FAIL=0
for s in $ALL_SRCS; do
    if "$SHARPC" out/${s%.c}.i -P -I. -o out/${s%.c}.fec.c 2>"$LOG_DIR/${s%.c}.fe.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        echo "  fe FAIL: $s"; head -5 "$LOG_DIR/${s%.c}.fe.err"
    fi
done
echo "  fe:         $PASS pass, $FAIL fail"
[ $FAIL -eq 0 ] || exit 1

# ─── Phase 4: cc compile ─────────────────────────────────────────────────
echo
echo "─── Phase 4: cc compile ───"
PASS=0; FAIL=0
for s in $ALL_SRCS; do
    if cc -std=gnu99 -O2 -w -c out/${s%.c}.fec.c -o out/${s%.c}.o 2>"$LOG_DIR/${s%.c}.cc.err"; then
        PASS=$((PASS+1))
    else
        FAIL=$((FAIL+1))
        echo "  cc FAIL: $s"; head -5 "$LOG_DIR/${s%.c}.cc.err"
    fi
done
echo "  compile:    $PASS pass, $FAIL fail"
[ $FAIL -eq 0 ] || exit 1

# ─── Phase 5: link ───────────────────────────────────────────────────────
echo
echo "─── Phase 5: link ───"
cc -std=gnu99 -o zlib_fe out/*.o
echo "  built: $(ls -la zlib_fe | awk '{print $9, $5"B"}')"

# ─── Phase 6: round-trip + diff ──────────────────────────────────────────
echo
echo "─── Phase 6: deflate/inflate + diff vs cc-only baseline ───"
./zlib_fe > zlib.out 2>&1
if diff -u "$WORK_DIR/baseline.out" zlib.out > "$LOG_DIR/diff.out"; then
    echo "  output matches baseline byte-for-byte ✓"
    cat zlib.out | sed 's/^/    | /'
else
    echo "  output DIFFERS from baseline ✗"
    head -20 "$LOG_DIR/diff.out"
    exit 1
fi

echo
echo "Build complete.  Binary at: $SRC_DIR/zlib_fe"
