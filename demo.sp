#include "math.sp"
#include "utils.sp"

i32 main() {
    i32 a = 10;
    i32 b = 20;
    i32 sum = add(a, b);
    i32 diff = sub(sum, b);
    i32 product = mul(diff, b);

    i32 neg = sub(0, 50);
    i32 abs = abs_val(neg);
    i32 clamped = clamp(product, 0, 150);

    i32 result = add(sum, add(abs, clamped));

    return result;
}
