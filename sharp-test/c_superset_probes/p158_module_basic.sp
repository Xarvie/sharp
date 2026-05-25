/*
 * p158_module_basic.sp — Basic function/struct/typedef cross-reference test.
 *
 * Tests:
 *   1. Functions with different signatures
 *   2. Structs with fields (including cross-references)
 *   3. Typedefs with target types
 *   4. Global variables with actual types
 *   5. Enums with all constant values
 */

#include <stdio.h>
#include "p158_math.sph"

int main(void) {
    /* Test functions */
    int sum = add(10, 20);
    float product = multiply(3.0f, 4.0f);

    greet((char *)"World");

    /* Test structs */
    struct Point2D pt;
    pt.x = 1.5f;
    pt.y = 2.5f;

    struct Rectangle rect;
    rect.width = 10.0f;
    rect.height = 20.0f;
    rect.origin = pt;

    /* Test typedefs */
    Int32 value = 42;
    Float32 fvalue = 3.14f;

    /* Test variables */
    double pi_copy = PI;
    int ver = VERSION;

    /* Test enums */
    enum Color c = RED;
    enum Status s = DEFAULT_STATUS;

    /* Print results to verify everything works */
    printf("sum=%d product=%.2f\n", sum, product);
    printf("pt=(%.1f,%.1f) rect=%.1fx%.1f\n", pt.x, pt.y, rect.width, rect.height);
    printf("value=%d fvalue=%.2f\n", value, fvalue);
    printf("PI=%.6f VERSION=%d\n", pi_copy, ver);
    printf("Color=%d Status=%d\n", c, s);

    /* Verify expected values */
    if (sum != 30) return 1;
    if (product < 11.99f || product > 12.01f) return 2;
    if (value != 42) return 3;
    if (ver != 1) return 4;
    if (rect.origin.x != 1.5f || rect.origin.y != 2.5f) return 5;
    if (c != RED) return 6;
    if (s != OK) return 7;

    printf("P158_BASIC_PASS\n");
    return 0;
}
