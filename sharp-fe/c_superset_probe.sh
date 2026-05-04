#!/usr/bin/env bash
# c_superset_probe.sh — run every probe in c_superset_probes/p*.c through
# the full sharp pipeline (sharpc -> cc -> run) and report pass/fail.
#
# A probe passes iff:
#   1. sharpc parses, sema-checks, and code-generates without error.
#   2. cc compiles the generated C11 without error.
#   3. The resulting binary exits with status 0 (probes are designed so
#      that "correct answer" → return 0).
#
# Usage:
#   ./c_superset_probe.sh                  # quiet, summary only
#   VERBOSE=1 ./c_superset_probe.sh        # print one line per failing probe
#
# Environment:
#   SHARPC      — path to sharpc binary (default: ./sharpc)
#   PROBE_DIR   — probe directory (default: ./c_superset_probes)
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
SHARPC="${SHARPC:-$HERE/sharpc}"
PROBE_DIR="${PROBE_DIR:-$HERE/c_superset_probes}"

[ -x "$SHARPC" ] || { echo "ERROR: sharpc not built; run 'make sharpc' first" >&2; exit 2; }
[ -d "$PROBE_DIR" ] || { echo "ERROR: probe dir $PROBE_DIR missing" >&2; exit 2; }

PASS=0; FAIL=0; FAIL_LIST=""
# Phase R1: probes can be .c (C source) or .sp (Sharp source).  sharpc
# auto-detects the language by extension, so the same pipeline handles
# both — we just need to enumerate both kinds here.
for p in "$PROBE_DIR"/p*.c "$PROBE_DIR"/p*.sp; do
    [ -f "$p" ] || continue
    name=$(basename "$p")
    err=$(timeout 5 "$SHARPC" "$p" 2>&1 >/tmp/_csp_out.c)
    rc=$?
    if [ $rc -eq 0 ]; then
        if cc -std=c11 -w /tmp/_csp_out.c -o /tmp/_csp_bin 2>/tmp/_csp_cc_err && /tmp/_csp_bin >/dev/null 2>&1; then
            PASS=$((PASS+1))
        else
            FAIL=$((FAIL+1))
            FAIL_LIST="$FAIL_LIST $name(run)"
            [ -n "${VERBOSE:-}" ] && echo "RUN-FAIL $name :: $(head -1 /tmp/_csp_cc_err 2>/dev/null)"
        fi
    elif [ $rc -eq 124 ]; then
        FAIL=$((FAIL+1))
        FAIL_LIST="$FAIL_LIST $name(timeout)"
        [ -n "${VERBOSE:-}" ] && echo "TIMEOUT  $name"
    else
        FAIL=$((FAIL+1))
        first=$(echo "$err" | grep -m1 "error:" | head -c 120)
        FAIL_LIST="$FAIL_LIST $name"
        [ -n "${VERBOSE:-}" ] && echo "FAIL     $name :: $first"
    fi
done
TOTAL=$((PASS + FAIL))
echo "C-superset probes: $PASS / $TOTAL pass"
[ -n "$FAIL_LIST" ] && echo "  fails:$FAIL_LIST"
[ "$FAIL" -eq 0 ]
