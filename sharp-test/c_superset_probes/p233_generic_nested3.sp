/* p233 — deeply nested generics (3-level Vec)
 *
 * Verifies three-level nesting of generic types with full type expressions.
 */

#include "../../std/types.sph"
#include "../../std/vec.sph"

int main() {
    /* --- 3D nested Vec: Vec<Vec<Vec<int>>> --- */
    Vec<Vec<Vec<int>>> v3d = {0};

    Vec<Vec<int>> inner2d = {0};
    Vec<int> inner1d = {0};
    inner1d.push(42);
    inner1d.push(73);
    inner2d.push(inner1d);
    v3d.push(inner2d);

    if (v3d.size() != 1) return 1;
    Vec<Vec<int>> r2 = v3d.get(0);
    if (r2.size() != 1) return 2;
    Vec<int> r1 = r2.get(0);
    if (r1.size() != 2) return 3;
    if (r1.get(0) != 42) return 4;

    /* --- 2D nested Vec directly --- */
    Vec<Vec<int>> m2d = {0};
    Vec<int> row0 = {0};
    Vec<int> row1 = {0};
    row0.push(1);
    row1.push(10);
    row1.push(20);
    m2d.push(row0);
    m2d.push(row1);

    if (m2d.size() != 2) return 5;
    if (m2d.get(0).get(0) != 1) return 6;
    if (m2d.get(1).get(1) != 20) return 7;

    return 0;
}