/* compiler: any */
/* C Feature: Compound literals */
/* Expected: Parser should handle (Type){ ... } syntax */

struct Point {
    int x;
    int y;
}

int main() {
    struct Point* p = &(struct Point){ .x = 1, .y = 2 };
    return 0;
}