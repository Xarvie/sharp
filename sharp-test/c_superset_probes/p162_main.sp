/*
 * p162_main.sp — Consumer for multi-header include test.
 *
 * Tests P2-2: uses two headers (p162_types + p162_strwrap),
 * uses typedefs from std.types (isize) and p162_types (fsize, Point)
 * and struct/function from p162_strwrap (WrappedPoint, get_offset).
 */

#include <stdio.h>

#include "p161_std_types.sph"
#include "p162_types.sph"
#include "p162_strwrap.sph"

/* ── Consumer code ───────────────────────────────────────────────── */

int main(void) {
    isize val = 42;
    fsize dval = 3.14;

    struct WrappedPoint wp;
    wp.x = 10;
    wp.y = 20;
    wp.name = "multi";

    Point offset_result = wp.get_offset();
    long offset = offset_result.x + offset_result.y;

    struct Point pt;
    pt.x = 5;
    pt.y = 7;

    printf("val=%ld offset=%ld dval=%.2f pt_s=%ld\n",
           val, offset, dval, pt.x + pt.y);

    if (val != 42) return 1;
    if (offset != 30) return 2;
    if (pt.x != 5) return 3;

    printf("P162_TRANSITIVE_PASS\n");
    return 0;
}
