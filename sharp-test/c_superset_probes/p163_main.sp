/*
 * p163_main.sp — Consumer for Vec/Pair cross-header test.
 *
 * Tests P2-1: uses monomorphized Vec_int and Pair_int_double
 * with concrete types.
 */

#include <stdio.h>

#include "p163_genvec.sph"

/* ── Consumer code ───────────────────────────────────────────────── */

int main(void) {
    struct Vec_int v;
    v.data = NULL;
    v.len = 0;
    v.cap = 16;

    struct Pair_int_double p;
    p.first = 42;
    p.second = 3.14;

    printf("vec: len=%d cap=%d\n", v.len, v.cap);
    printf("pair: first=%d second=%.2f\n", p.first, p.second);

    if (v.cap != 16) return 1;
    if (p.first != 42) return 2;
    if (p.second < 3.13 || p.second > 3.15) return 3;

    printf("P163_GENINST_PASS\n");
    return 0;
}
