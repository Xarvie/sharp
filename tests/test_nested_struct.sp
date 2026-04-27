/* compiler: any */
/* C Feature: Nested struct definitions */
/* Expected: Parser should handle struct defined inside another struct */

struct Outer {
    int x;
    struct Inner {
        int a;
        int b;
    } inner;
}

int main() {
    return 0;
}