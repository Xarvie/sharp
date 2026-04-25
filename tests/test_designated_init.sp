/* compiler: any */
/* C Feature: Designated initializers */
/* Expected: Parser should handle .field = value syntax */

struct Point {
    i32 x;
    i32 y;
}

i32 main() {
    struct Point p = { .x = 1, .y = 2 };
    return 0;
}
