/* p230 — zero-init semantics for structs
 *
 * Verifies that `= {0}` zero-initializes all fields:
 *   - int → 0, float → 0.0
 *   - pointers → NULL
 *   - bool → false
 *   - nested struct fields → recursively zero
 */

#include <stdbool.h>

class Pair {
    int a;
    int b;
};

class Container {
    int val;
    int* ptr;
    Pair inner;
};

int main() {
    /* zero-init scalar */
    int x = {0};
    if (x != 0) return 1;

    float f = {0};
    if (f != 0.0f) return 2;

    bool b = {0};
    if (b != false) return 3;

    /* zero-init struct */
    Container c = {0};
    if (c.val != 0) return 4;
    if (c.ptr != 0) return 5;
    if (c.inner.a != 0) return 6;
    if (c.inner.b != 0) return 7;

    /* zero-init pointer */
    int* np = {0};
    if (np != 0) return 8;

    return 0;
}