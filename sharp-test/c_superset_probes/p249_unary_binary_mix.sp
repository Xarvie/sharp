/* p249 — unary + binary operator mix
 *
 * Tests expressions like -a + b where unary and binary operators coexist.
 */

#include <stdbool.h>

class Val {
    int x;
};

Val  Val.operator+(this, Val other) const {
    Val r; r.x = this->x + other.x; return r;
}
Val  Val.operator-(this) const {
    Val r; r.x = -this->x; return r;
}
bool Val.operator==(this, Val other) const {
    return this->x == other.x;
}

int main() {
    Val a; a.x = 10;
    Val b; b.x = 3;

    /* -a + b = -10 + 3 = -7 */
    Val r = -a + b;
    if (r.x != -7) return 1;

    /* a + -b = 10 + -3 = 7 */
    Val r2 = a + -b;
    if (r2.x != 7) return 2;

    /* -a + -b = -10 + -3 = -13 */
    Val r3 = -a + -b;
    if (r3.x != -13) return 3;

    return 0;
}