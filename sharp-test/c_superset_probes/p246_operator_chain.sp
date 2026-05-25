/* p246 — operator chaining: a + b + c
 *
 * Tests that chained binary operators correctly translate to
 * multiple operator function calls.
 */

#include <stdbool.h>

class Vec3 {
    int x, y, z;
};

Vec3 Vec3.operator+(this, Vec3 other) const {
    Vec3 r;
    r.x = this->x + other.x;
    r.y = this->y + other.y;
    r.z = this->z + other.z;
    return r;
}

bool Vec3.operator==(this, Vec3 other) const {
    return this->x == other.x
        && this->y == other.y
        && this->z == other.z;
}

int main() {
    Vec3 a; a.x = 1; a.y = 2; a.z = 3;
    Vec3 b; b.x = 10; b.y = 20; b.z = 30;
    Vec3 c; c.x = 100; c.y = 200; c.z = 300;

    /* chain: a + b + c */
    Vec3 r = a + b + c;
    if (r.x != 111) return 1;   /* 1+10+100 */
    if (r.y != 222) return 2;   /* 2+20+200 */
    if (r.z != 333) return 3;   /* 3+30+300 */

    /* chain: (a + b) == b + a */
    Vec3 left  = a + b;
    Vec3 right = b + a;
    if (!(left == right)) return 4;

    return 0;
}