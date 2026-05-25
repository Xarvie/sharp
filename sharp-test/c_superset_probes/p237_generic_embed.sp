/* p237 — generic struct embeds bare struct
 *
 * Tests that a generic struct can contain a non-generic struct as a field.
 */

#include <stdbool.h>

/* bare struct */
class Point {
    int x;
    int y;
};

/* generic struct embedding Point */
class Labeled<T> {
    T     label;
    Point pos;
};

T    Labeled.get_label(this) const { return this->label; }
void Labeled.set_label(this, T val) { this->label = val; }
void Labeled.set_xy(this, int x, int y) { this->pos.x = x; this->pos.y = y; }
int  Labeled.pos_x(this) const { return this->pos.x; }

int main() {
    Labeled<int> a = {0};
    a.set_label(42);
    a.set_xy(10, 20);

    if (a.get_label() != 42) return 1;
    if (a.pos_x() != 10) return 2;

    /* with different type param */
    Labeled<float> b = {0};
    b.set_label(3.14f);
    if (b.get_label() < 3.13f || b.get_label() > 3.15f) return 3;

    return 0;
}