/* p276 — Compound literal and temporary struct patterns
 *
 * Tests that temporary struct values work correctly.
 */

class Point {
    int x;
    int y;
};

Point make_point(int xx, int yy) {
    Point p; p.x = xx; p.y = yy;
    return p;
}

Point Point.add(this, Point rhs) {
    Point r;
    r.x = this->x + rhs.x;
    r.y = this->y + rhs.y;
    return r;
}

int main() {
    /* Return struct from function */
    Point p1 = make_point(10, 20);
    if (p1.x != 10) return 1;
    if (p1.y != 20) return 2;

    /* Pass struct value */
    Point p2 = make_point(5, 7);
    Point p3 = p1.add(p2);
    if (p3.x != 15) return 3;
    if (p3.y != 27) return 4;

    /* Chain function calls returning structs */
    Point p4 = make_point(1, 2).add(make_point(3, 4));
    if (p4.x != 4) return 5;
    if (p4.y != 6) return 6;

    return 0;
}