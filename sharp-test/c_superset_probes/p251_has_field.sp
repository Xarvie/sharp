/* p251 — @has_field with nested structs
 *
 * Tests @has_field on struct fields.
 */

#include <stdbool.h>

class Inner {
    int a;
};

class Outer {
    Inner inner;
    int   b;
};

int main() {
    Outer o = {0};

    if (@has_field(Outer, inner)) {
        /* should enter */
        o.inner.a = 1;
    } else { return 10; }

    if (@has_field(Outer, b)) {
        o.b = 2;
    } else { return 20; }

    if (@has_field(Inner, a)) {
        /* should enter */
    } else { return 30; }

    if (@has_field(Outer, missing)) {
        return 40;
    }

    @static_assert(@has_field(Outer, inner), "Outer must have inner");

    if (o.inner.a != 1) return 1;
    if (o.b != 2) return 2;

    return 0;
}