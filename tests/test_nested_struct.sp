/* compiler: any */
/* C Feature: Nested struct definitions */
/* Expected: Parser should handle struct defined inside another struct */

struct Outer {
    i32 x;
    struct Inner {
        i32 a;
        i32 b;
    } inner;
}

i32 main() {
    return 0;
}
