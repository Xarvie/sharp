/* p225 — operator overloading as extension methods + scoping test
 *
 * Tests:
 * 1. Can operator+ be defined as an extension method?
 * 2. If op+ defined at line ~30, does code before (line ~20) see it?
 *    Does code after (line ~40) see it?
 */

class Point {
    int x;
    int y;
};

/* ===== BEFORE: use operator before it's defined ===== */
int test_before() {
    Point a; a.x = 1; a.y = 2;
    Point b; b.x = 10; b.y = 20;
    Point c = a + b;   /* line ~20 — BEFORE operator+ definition */
    if (c.x != 11) return 1;
    if (c.y != 22) return 2;
    return 0;
}

/* ===== operator+ as extension method ===== */
Point Point.operator+(this, Point other) const {
    Point r;
    r.x = this->x + other.x;
    r.y = this->y + other.y;
    return r;
}

/* ===== AFTER: use operator after definition ===== */
int test_after() {
    Point a; a.x = 5; a.y = 6;
    Point b; b.x = 30; b.y = 40;
    Point c = a + b;   /* line ~40 — AFTER operator+ definition */
    if (c.x != 35) return 11;
    if (c.y != 46) return 12;
    return 0;
}

int main() {
    int r1 = test_before();
    if (r1 != 0) return r1;

    int r2 = test_after();
    if (r2 != 0) return r2;

    return 0;
}