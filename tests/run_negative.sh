#!/bin/sh
# tests/run_negative.sh — verify that each .sp file in tests/negative/
# triggers the diagnostic code(s) declared in its "// EXPECT: Ennnn [...]"
# header. A test PASSes when:
#   1. sharpc exits non-zero
#   2. no .c output file is produced
#   3. every expected error code appears in sharpc's stderr
#
# Usage:
#   ./tests/run_negative.sh [path/to/sharpc] [tests/negative]

# NOTE: intentionally no `set -e` — we rely on $? from sharpc runs.
SHARPC="${1:-./sharpc}"
DIR="${2:-tests/negative}"

if [ ! -x "$SHARPC" ]; then
    echo "sharpc binary not found: $SHARPC" >&2
    exit 2
fi

fail=0
total=0
passed=0

for sp in "$DIR"/*.sp; do
    [ -f "$sp" ] || continue
    total=$((total + 1))
    name=$(basename "$sp" .sp)

    # Extract expected codes from first line matching "// EXPECT: ..."
    expect=$(grep -m1 '^// EXPECT:' "$sp" | sed 's|^// EXPECT:||')
    if [ -z "$expect" ]; then
        printf "  SKIP  %-32s  (no EXPECT header)\n" "$name"
        continue
    fi

    tmp_c="/tmp/sharp_neg_$$_$name.c"
    rm -f "$tmp_c"

    # Capture stderr AND the exit code. Shell-portable pattern: run the
    # compiler in a subshell that echoes its exit code on the last line.
    err=$("$SHARPC" "$sp" -o "$tmp_c" 2>&1 >/dev/null ; echo "__RC__=$?")
    rc=$(printf '%s\n' "$err" | sed -n 's/^__RC__=//p' | tail -1)
    err=$(printf '%s\n' "$err" | sed '/^__RC__=/d')

    # sharpc should have exited non-zero and left no output file.
    actual_fail=0
    reason=""
    if [ "$rc" = "0" ]; then
        actual_fail=1
        reason="exit 0 (expected non-zero)"
    elif [ -f "$tmp_c" ]; then
        actual_fail=1
        reason="produced $tmp_c (should not on error)"
        rm -f "$tmp_c"
    fi

    # Each expected code must appear in the diagnostic output.
    missing=""
    for code in $expect; do
        case "$err" in
            *"$code"*) ;;
            *) missing="$missing $code" ;;
        esac
    done

    if [ $actual_fail -ne 0 ]; then
        printf "  FAIL  %-32s  %s\n" "$name" "$reason"
        fail=$((fail + 1))
    elif [ -n "$missing" ]; then
        printf "  FAIL  %-32s  missing codes:%s\n" "$name" "$missing"
        printf "         stderr: %s\n" "$(printf '%s' "$err" | head -c 200)"
        fail=$((fail + 1))
    else
        printf "  PASS  %-32s  expected:%s\n" "$name" "$expect"
        passed=$((passed + 1))
    fi
done

echo
if [ $fail -eq 0 ]; then
    echo "all $total negative tests passed"
    exit 0
else
    echo "$fail of $total negative tests FAILED ($passed passed)"
    exit 1
fi
