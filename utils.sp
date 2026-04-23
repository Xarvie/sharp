/* utils.sp - Utility functions */

i32 abs_val(i32 x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

i32 clamp(i32 x, i32 min, i32 max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}
