/*
 * p164_main.sp — Consumer for struct methods test.
 *
 * Includes p164_ops, creates Counter, calls methods including const method.
 */

#include <stdio.h>

#include "p164_ops.sph"

/* ── Consumer code ───────────────────────────────────────────────── */

int main(void) {
    struct Counter c;
    c.count = 0;
    c.max = 100;

    int v0 = c.value();
    c.inc();
    int v1 = c.value();
    c.inc();
    int v2 = c.value();
    int v3 = c.value();
    c.dec();
    int v4 = c.value();
    int v5 = c.value();

    c.reset();
    int v6 = c.value();

    printf("v0=%d v1=%d v2=%d v3=%d v4=%d v5=%d v6=%d\n",
           v0, v1, v2, v3, v4, v5, v6);

    if (v0 != 0)  return 1;
    if (v1 != 1)  return 2;
    if (v2 != 2)  return 3;
    if (v3 != 2)  return 4;
    if (v4 != 1)  return 5;
    if (v5 != 1)  return 6;
    if (v6 != 0)  return 7;

    printf("P164_METHODS_PASS\n");
    return 0;
}
