/*
 * p159_main.sp — Multi-header consumer.
 *
 * Tests:
 *   1. Include multiple headers (p159_core.math, p159_collections.vec)
 *   2. Use functions from both headers
 *   3. Use structs
 */

#include <stdbool.h>
#include <stdio.h>
#include "p159_core_math.sph"
#include "p159_collections_vec.sph"

/* Use functions from p159_core_math.sph */
int compute_range(int x) {
    return clamp(x, 0, 100);
}

float interpolate(float t) {
    return lerp(0.0f, 100.0f, t);
}

/* Use structs from p159_core.math */
bool check_range(struct Range *r, int val) {
    return r->contains(val);
}

/* Use String and functions from p159_collections.vec */
void test_string(void) {
    struct String s;
    s.data = "hello";
    s.len = 5;
    int slen = s.length();
    printf("string_length=%d\n", slen);
}

int main(void) {
    /* Test math header functions */
    int clamped = compute_range(150);      /* Should be 100 */
    float interpolated = interpolate(0.5f); /* Should be 50.0 */

    /* Test Range struct */
    struct Range r;
    r.start = 10;
    r.end = 20;
    bool in_range = check_range(&r, 15);
    bool out_of_range = check_range(&r, 25);

    /* Test vec header */
    test_string();

    /* Print results */
    printf("clamped=%d interpolated=%.1f\n", clamped, interpolated);
    printf("in_range=%d out_of_range=%d\n", in_range, out_of_range);

    /* Verify expected values */
    if (clamped != 100) return 1;
    if (interpolated < 49.99f || interpolated > 50.01f) return 2;
    if (!in_range) return 3;
    if (out_of_range) return 4;

    printf("P159_MULTI_PASS\n");
    return 0;
}
