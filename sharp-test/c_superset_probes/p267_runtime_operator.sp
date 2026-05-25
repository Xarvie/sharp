/* p267 — Runtime: operator overloading
 *
 * Verifies that operator overloading works at runtime.
 */

class Point {
    int x;
    int y;
};

Point Point.operator+(this, Point rhs) {
    Point r;
    r.x = this->x + rhs.x;
    r.y = this->y + rhs.y;
    return r;
}

Point Point.operator-(this, Point rhs) {
    Point r;
    r.x = this->x - rhs.x;
    r.y = this->y - rhs.y;
    return r;
}

int main() {
    Point p1; p1.x = 3; p1.y = 5;
    Point p2; p2.x = 7; p2.y = 2;

    Point r1 = p1 + p2;
    if (r1.x != 10) return 1;
    if (r1.y != 7)  return 2;

    Point r2 = p1 - p2;
    if (r2.x != -4) return 3;
    if (r2.y != 3)  return 4;

    /* Chained operators */
    Point p3; p3.x = 1; p3.y = 1;
    Point r3 = p1 + p2 + p3;
    if (r3.x != 11) return 5;
    if (r3.y != 8)  return 6;

    return 0;
}