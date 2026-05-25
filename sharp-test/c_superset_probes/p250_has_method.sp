/* p250 — @has_method with extension methods
 *
 * Tests that @has_method can detect extension methods.
 */

#include <stdbool.h>

class Foo {
    int x;
};

/* extension method */
int Foo.get_x(this) { return this->x; }

int main() {
    Foo f; f.x = 42;

    if (@has_method(Foo, get_x)) {
        /* should enter this branch */
        if (f.get_x() != 42) return 1;
    } else {
        /* should not reach here */
        return 10;
    }

    if (@has_method(Foo, missing_method)) {
        /* should NOT enter this branch */
        return 20;
    }

    @static_assert(@has_method(Foo, get_x), "Foo must have get_x");

    return 0;
}