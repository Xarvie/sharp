/* p253 — complex @static_assert conditions
 *
 * Tests @static_assert with &&, ||, ! combinations.
 */

#include <stdbool.h>

class A { int x; };
class B { int x; };
class C { float y; };

int main() {
    /* compound conditions */
    @static_assert(@has_field(A, x) && @has_field(B, x), "A and B must have x");
    @static_assert(1 || 0, "1 || 0 must be true");
    @static_assert(!0, "!0 must be true");

    /* dead-branch pruning: unreachable code removed */
    if (@has_field(A, missing)) {
        /* dead branch — should be removed by compiler */
        return 99;
    }

    if (!@has_field(A, missing)) {
        /* this branch should be live */
    } else {
        return 98;
    }

    return 0;
}