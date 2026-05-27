/* p252 — @is_arithmetic with typedef types
 *
 * Tests that @is_arithmetic correctly identifies typedef'd arithmetic types.
 */

#include <stdbool.h>
#include "../../std/types.sph"

typedef int MyInt;
typedef float MyFloat;

class Point { int x; int y; };

int main() {
    if (@is_arithmetic(MyInt)) {
        /* int typedef → should be arithmetic */
    } else { return 10; }

    if (@is_arithmetic(MyFloat)) {
        /* float typedef → should be arithmetic */
    } else { return 20; }

    if (@is_arithmetic(Point)) {
        /* struct is NOT arithmetic */
        return 30;
    }

    if (@is_arithmetic(isize)) {
        /* isize → should be arithmetic */
    } else { return 40; }

    /* @is_arithmetic can be used in static_assert */
    @static_assert(@is_arithmetic(int), "int must be arithmetic");

    return 0;
}