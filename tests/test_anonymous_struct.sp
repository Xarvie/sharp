/* compiler: any */
/* C Feature: Anonymous struct/union inside struct */
/* Expected: Parser should handle anonymous nested structs */

struct Outer {
    int x;
    struct {
        int a;
        int b;
    };
    union {
        int c;
        double d;
    };
}

int main() {
    return 0;
}