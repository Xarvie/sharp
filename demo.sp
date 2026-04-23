#include "math.sp"
#include "utils.sp"

i32 main() {
    /* Test math.sp functions */
    i32 a = 10;
    i32 b = 20;
    i32 sum = add(a, b);       /* 10 + 20 = 30 */
    i32 diff = sub(sum, b);    /* 30 - 20 = 10 */
    i32 product = mul(diff, b); /* 10 * 20 = 200 */

    /* Test utils.sp functions */
    i32 neg = sub(0, 50);       /* -50 */
    i32 abs = abs_val(neg);     /* 50 */
    i32 clamped = clamp(product, 0, 150); /* clamp(200, 0, 150) = 150 */

    /* Final result: sum(30) + abs(50) + clamped(150) = 230 */
    i32 result = add(sum, add(abs, clamped));

    return result;
}
