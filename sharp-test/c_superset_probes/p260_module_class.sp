/* p260 — Module with class + extension methods
 *
 * Tests importing a module header with a class and its extension methods.
 */

#include <stdbool.h>
#include "p260_mymath.sph"

int main() {
    /* T1: use exported function */
    int s = square(7);
    if (s != 49) return 1;

    int c = cube(3);
    if (c != 27) return 2;

    /* T2: use class with extension methods from module */
    Calculator calc;
    calc.reset();
    if (calc.get() != 0) return 3;

    calc.set(10);
    if (calc.get() != 10) return 4;

    int r = calc.add(5);
    if (r != 15) return 5;
    if (calc.get() != 15) return 6;

    return 0;
}