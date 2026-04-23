/* test_include.sp - Test including a C .h header file */
#include "test_c_header.h"

i32 main() {
    i32 a = 10;
    i32 b = 20;
    i32 sum = c_add(a, b);
    i32 product = c_mul(sum, b);
    i32 max = MAX_VAL;
    return product;
}
