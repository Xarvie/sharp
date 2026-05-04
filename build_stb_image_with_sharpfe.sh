#!/usr/bin/env bash
# build_stb_image_with_sharpfe.sh
# ─────────────────────────────────────────────────────────────────────────
# Phase R2 — fourth real-world C codebase through the full pipeline.
#
# stb_image / stb_image_write are Sean Barrett's header-only image
# libraries (~10K lines of macro-heavy ANSI/ISO C used everywhere from
# game dev to ML).  They exercise patterns the previous targets did
# not: function-typedef declarations (`typedef void Fn(args);`),
# block-scope anonymous enums, _Thread_local globals, dense union/bit
# tricks, and an industrial volume of macros.
#
# Two GCC extensions appear in stb's optional code paths:
#   - SSE intrinsics (vector_size attribute as cast type)
#   - glibc's assert() expansion (statement-expression `({...})`)
# We bypass both via -DNDEBUG -DSTBI_NO_SIMD; this is how stb is
# routinely built in production and exactly what the official build
# instructions recommend for portability.
#
# Pipeline:
#   stb_driver.c ─[sharp_cli -P]─> .i  ─[sharpc -P]─> .fec.c ─[cc]─> bin
#
# Smoke: encode a 16×16 RGB gradient to PNG in memory, decode back,
# compare pixel-perfect.  Success criterion: output matches the
# cc-only baseline byte-for-byte.

set -uo pipefail

WORK_DIR="${WORK_DIR:-/tmp/stb_sharpfe_build}"

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
SHARP_CLI="${SHARP_CLI:-$REPO_ROOT/sharp-cpp/sharp-cpp/sharp_cli}"
SHARPC="${SHARPC:-$REPO_ROOT/sharp-fe/sharpc}"
[ -x "$SHARP_CLI" ] || { echo "ERROR: $SHARP_CLI not built" >&2; exit 1; }
[ -x "$SHARPC"    ] || { echo "ERROR: $SHARPC not built"    >&2; exit 1; }
echo "Using sharp_cli: $SHARP_CLI"
echo "Using sharpc:    $SHARPC"

# ─── locate stb headers ──────────────────────────────────────────────────
STB_DIR="$WORK_DIR/src"
mkdir -p "$STB_DIR"
for h in stb_image.h stb_image_write.h; do
    if [ ! -f "$STB_DIR/$h" ]; then
        echo "Downloading $h..."
        curl -sL "https://raw.githubusercontent.com/nothings/stb/master/$h" -o "$STB_DIR/$h"
    fi
done
echo "stb headers in: $STB_DIR"

# ─── round-trip driver ───────────────────────────────────────────────────
cat > "$STB_DIR/stb_driver.c" <<'EOF'
/* PNG round-trip:
 *   1. Build a 16×16 RGB gradient in memory.
 *   2. Encode to PNG (in memory) via stb_image_write.
 *   3. Decode back via stb_image.
 *   4. Verify dimensions, channels, and pixel data.
 * The expected output is exact and deterministic.
 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { unsigned char *data; int len; int cap; } sink;

static void sink_write(void *ctx, void *src, int n) {
    sink *s = (sink*)ctx;
    if (s->len + n > s->cap) {
        s->cap = (s->len + n) * 2;
        s->data = (unsigned char*)realloc(s->data, s->cap);
    }
    memcpy(s->data + s->len, src, n);
    s->len += n;
}

int main(void) {
    enum { W = 16, H = 16, C = 3 };
    unsigned char src[W * H * C];
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int p = (y * W + x) * C;
            src[p+0] = (unsigned char)(x * 16);
            src[p+1] = (unsigned char)(y * 16);
            src[p+2] = (unsigned char)((x + y) * 8);
        }
    }
    sink png = {0, 0, 0};
    int ok = stbi_write_png_to_func(sink_write, &png, W, H, C, src, W * C);
    if (!ok) { puts("FAIL: write_png"); return 1; }

    int rw = 0, rh = 0, rc = 0;
    unsigned char *back = stbi_load_from_memory(png.data, png.len, &rw, &rh, &rc, 0);
    if (!back) { puts("FAIL: load_from_memory"); return 2; }
    if (rw != W || rh != H || rc != C) {
        printf("FAIL: header mismatch %dx%d %dch\n", rw, rh, rc); return 3;
    }
    if (memcmp(src, back, W * H * C) != 0) { puts("FAIL: pixel mismatch"); return 4; }

    unsigned long sum = 0;
    for (int i = 0; i < W * H * C; i++) sum += back[i];

    printf("OK %dx%d %dch sum=%lu png_bytes=%d\n", rw, rh, rc, sum, png.len);

    stbi_image_free(back);
    free(png.data);
    return 0;
}
EOF

cd "$STB_DIR"

SYSINCS="-isystem /usr/lib/gcc/x86_64-linux-gnu/13/include"
SYSINCS="$SYSINCS -isystem /usr/local/include"
SYSINCS="$SYSINCS -isystem /usr/include/x86_64-linux-gnu"
SYSINCS="$SYSINCS -isystem /usr/include"

DEFINES="-DNDEBUG -DSTBI_NO_SIMD"

LOG_DIR="$WORK_DIR/_logs"; mkdir -p "$LOG_DIR"

# ─── Phase 1: cc-only baseline ───────────────────────────────────────────
echo
echo "─── Phase 1: cc-only baseline ───"
cc -std=c99 -O2 -w stb_driver.c -o stb_baseline -lm
./stb_baseline > "$WORK_DIR/baseline.out" 2>&1
echo "  baseline: $(cat $WORK_DIR/baseline.out)"

# ─── Phase 2: sharp-cpp ──────────────────────────────────────────────────
echo
echo "─── Phase 2: sharp-cpp preprocessing ───"
if "$SHARP_CLI" stb_driver.c -P -I. $DEFINES $SYSINCS -o stb_driver.i 2>"$LOG_DIR/cpp.err"; then
    echo "  preprocess: 1 pass, 0 fail ($(wc -l < stb_driver.i) lines)"
else
    echo "  preprocess: 0 pass, 1 fail"; head -10 "$LOG_DIR/cpp.err"; exit 1
fi

# ─── Phase 3: sharp-fe ──────────────────────────────────────────────────
echo
echo "─── Phase 3: sharp-fe parse / sema / cg ───"
if "$SHARPC" stb_driver.i -P -I. $DEFINES $SYSINCS -o stb_driver.fec.c 2>"$LOG_DIR/fe.err"; then
    echo "  fe:         1 pass, 0 fail ($(wc -l < stb_driver.fec.c) lines)"
else
    echo "  fe:         0 pass, 1 fail"; head -25 "$LOG_DIR/fe.err"; exit 1
fi

# ─── Phase 4: cc compile ─────────────────────────────────────────────────
echo
echo "─── Phase 4: cc compile ───"
if cc -std=gnu99 -O2 -w -c stb_driver.fec.c -o stb_driver.o 2>"$LOG_DIR/cc.err"; then
    echo "  compile:    1 pass, 0 fail"
else
    echo "  compile:    0 pass, 1 fail"; head -25 "$LOG_DIR/cc.err"; exit 1
fi

# ─── Phase 5: link ───────────────────────────────────────────────────────
echo
echo "─── Phase 5: link ───"
cc -std=gnu99 -o stb_bin stb_driver.o -lm
echo "  built: $(ls -la stb_bin | awk '{print $9, $5"B"}')"

# ─── Phase 6: smoke + diff ───────────────────────────────────────────────
echo
echo "─── Phase 6: PNG round-trip + diff vs cc-only baseline ───"
./stb_bin > stb.out 2>&1
if diff -u "$WORK_DIR/baseline.out" stb.out > "$LOG_DIR/diff.out"; then
    echo "  output matches baseline byte-for-byte ✓"
    cat stb.out | sed 's/^/    | /'
else
    echo "  output DIFFERS from baseline ✗"
    head -40 "$LOG_DIR/diff.out"
    exit 1
fi

echo
echo "Build complete.  Binary at: $STB_DIR/stb_bin"
