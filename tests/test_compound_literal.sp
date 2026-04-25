/* compiler: any */
/* C Feature: Compound literals */
/* Expected: Parser should handle (Type){ ... } syntax */

struct Point {
    i32 x;
    i32 y;
}

i32 main() {
    struct Point* p = &(struct Point){ .x = 1, .y = 2 };
    return 0;
}
