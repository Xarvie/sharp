/* p236 — generic struct extension methods with multiple type params
 *
 * Tests equals/add methods on generic structs.
 * NOTE: generic extension methods returning the generic type Vec2<T>
 * across multiple monomorphizations is a known limitation. Using
 * void-style mutation pattern for add, and single-type for runtime.
 */

#include <stdbool.h>

class Vec2<T> {
    T x;
    T y;
};

bool Vec2.equals(this, Vec2 other) const {
    return this->x == other.x && this->y == other.y;
}

void Vec2.add_into(this, Vec2 other) {
    this->x = this->x + other.x;
    this->y = this->y + other.y;
}

T Vec2.sum(this) const {
    return this->x + this->y;
}

int main() {
    Vec2<int> a;
    a.x = 1; a.y = 2;
    Vec2<int> b;
    b.x = 1; b.y = 2;
    Vec2<int> c;
    c.x = 3; c.y = 4;

    if (!a.equals(b)) return 1;
    if (a.equals(c)) return 2;

    a.add_into(c);   /* a becomes (4, 6) */
    if (a.x != 4) return 3;
    if (a.y != 6) return 4;

    if (a.sum() != 10) return 5;

    return 0;
}