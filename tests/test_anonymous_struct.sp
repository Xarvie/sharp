/* compiler: any */
/* C Feature: Anonymous struct/union inside struct */
/* Expected: Parser should handle anonymous nested structs */

struct Outer {
    i32 x;
    struct {
        i32 a;
        i32 b;
    };
    union {
        i32 c;
        f64 d;
    };
}

i32 main() {
    return 0;
}
