/* p273 — Array of structs with initialization
 *
 * Tests array-of-struct layout and zero-initialization patterns.
 */

class Vec2 {
    int x;
    int y;
};

int main() {
    /* Fixed-size array of structs */
    Vec2 points[4] = {0};

    points[0].x = 1; points[0].y = 2;
    points[1].x = 3; points[1].y = 4;
    points[2].x = 5; points[2].y = 6;
    points[3].x = 7; points[3].y = 8;

    /* Verify access */
    int sum_x = 0, sum_y = 0;
    for (int i = 0; i < 4; i = i + 1) {
        sum_x = sum_x + points[i].x;
        sum_y = sum_y + points[i].y;
    }
    if (sum_x != 16) return 1;  /* 1+3+5+7=16 */
    if (sum_y != 20) return 2;  /* 2+4+6+8=20 */

    /* Partial initialization */
    Vec2 partial[3] = {0};
    partial[1].x = 99;
    if (partial[0].x != 0) return 3;
    if (partial[1].x != 99) return 4;
    if (partial[2].x != 0) return 5;

    return 0;
}