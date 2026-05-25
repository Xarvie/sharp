/* p247 — mixed operator precedence: a + b * c
 *
 * Tests that a + b * c respects operator precedence:
 * * and / are higher than + and -
 */

#include <stdbool.h>

class Num {
    int val;
};

Num Num.operator+(this, Num other) const {
    Num r; r.val = this->val + other.val; return r;
}
Num Num.operator-(this, Num other) const {
    Num r; r.val = this->val - other.val; return r;
}
Num Num.operator*(this, Num other) const {
    Num r; r.val = this->val * other.val; return r;
}
Num Num.operator/(this, Num other) const {
    Num r; r.val = this->val / other.val; return r;
}
bool Num.operator==(this, Num other) const {
    return this->val == other.val;
}

int Num.get(this) const { return this->val; }

int main() {
    Num two;   two.val   = 2;
    Num three; three.val = 3;
    Num four;  four.val  = 4;
    Num ten;   ten.val   = 10;

    /* a + b * c = 2 + 3*4 = 2 + 12 = 14 */
    Num r1 = two + three * four;
    if (r1.val != 14) return 1;

    /* a * b + c = 2*3 + 4 = 6 + 4 = 10 */
    Num r2 = two * three + four;
    if (r2.val != 10) return 2;

    /* a + b - c*d = 10 + 2 - 3*4 = 12 - 12 = 0 */
    Num r3 = ten + two - three * four;
    if (r3.val != 0) return 3;

    /* a * b / c = 3*4 / 2 = 12 / 2 = 6 */
    Num r4 = three * four / two;
    if (r4.val != 6) return 4;

    return 0;
}