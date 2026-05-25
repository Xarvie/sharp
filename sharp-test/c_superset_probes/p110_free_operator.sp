/* p110 — free-function operator overloading
 *
 * Verifies that operator+ / operator- / operator== defined as top-level
 * free functions (not struct methods) are correctly dispatched and mangled.
 */

struct Vec2 { float x; float y; }

struct Vec2 operator+(struct Vec2 a, struct Vec2 b) {
    struct Vec2 r; r.x = a.x + b.x; r.y = a.y + b.y; return r;
}
struct Vec2 operator-(struct Vec2 a, struct Vec2 b) {
    struct Vec2 r; r.x = a.x - b.x; r.y = a.y - b.y; return r;
}
int operator==(struct Vec2 a, struct Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

/* Vec2m: method operator* (dot product) — different class, no name clash */
class Vec2m {
    float x; float y;
}
float Vec2m.operator*(this, Vec2m rhs) {
    return this->x * rhs.x + this->y * rhs.y;
}

int main(void) {
    struct Vec2 a; a.x = 1.0f; a.y = 2.0f;
    struct Vec2 b; b.x = 3.0f; b.y = 4.0f;

    struct Vec2 s = a + b;
    if (s.x != 4.0f) return 1;
    if (s.y != 6.0f) return 2;

    struct Vec2 d = b - a;
    if (d.x != 2.0f) return 3;
    if (d.y != 2.0f) return 4;

    struct Vec2 c; c.x = 1.0f; c.y = 2.0f;
    if (!(a == c)) return 5;
    if (a == b)    return 6;

    Vec2m u; u.x = 3.0f; u.y = 4.0f;
    Vec2m v; v.x = 1.0f; v.y = 0.0f;
    float dot = u * v;
    if (dot != 3.0f) return 7;

    return 0;
}