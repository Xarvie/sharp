/* p113 — unary operator overload: method and free function */
class Vec2 {
    float x; float y;
}
Vec2 Vec2.operator-(this) {
    Vec2 r; r.x = -this->x; r.y = -this->y; return r;
}

struct Scalar { int v; }
struct Scalar operator-(struct Scalar a) {
    struct Scalar r; r.v = -a.v; return r;
}
int main(void) {
    Vec2 a; a.x = 3.0f; a.y = -4.0f;
    Vec2 b = -a;
    if (b.x != -3.0f) return 1;
    if (b.y !=  4.0f) return 2;
    struct Scalar s; s.v = 7;
    struct Scalar ns = -s;
    if (ns.v != -7) return 3;
    return 0;
}