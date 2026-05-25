/* p224 — typedef aliases for generic types
 *
 * Verifies that typedef works for concrete instantiations of generic
 * class types, producing correct alias names and method dispatch.
 *
 * NOTE: extension methods can only be defined on the original
 * struct/class name (e.g. Vec<int>), NOT on the typedef alias.
 * This is by design — typedefs are transparent aliases, not new types.
 */
/* FIXED: generic typedef expansion — defer generic typedefs until struct specs emitted */
#include "../../sharp/std/types.sph"
#include "../../sharp/std/vec.sph"

/* typedef aliases for generic instantiations */
typedef Vec<int> IntVec;
typedef Vec<float> FloatVec;
typedef IntVec MyIntVec;   /* chain: typedef of typedef */

int main() {
    /* --- basic typedef usage --- */
    IntVec v = {0};
    v.push(42);
    v.push(73);
    if (v.size() != 2) return 1;
    if (v.get(0) != 42) return 2;
    if (v.get(1) != 73) return 3;
    v.destroy();

    /* --- typedef of typedef --- */
    MyIntVec v2 = {0};
    v2.push(99);
    if (v2.size() != 1) return 4;
    if (v2.get(0) != 99) return 5;
    v2.destroy();

    /* --- another generic instantiation --- */
    FloatVec fv = {0};
    fv.push(1.5f);
    fv.push(2.5f);
    if (fv.size() != 2) return 6;
    if (fv.get(0) < 1.4f || fv.get(0) > 1.6f) return 7;
    fv.destroy();

    /* --- typedef variable in operator context --- */
    IntVec a = {0}, b = {0};
    a.push(10); b.push(20);
    if (a.size() != 1) return 8;
    a.destroy(); b.destroy();

    /* --- zero-init typedef variable --- */
    IntVec z = {0};
    if (z.size() != 0) return 9;
    /* z.destroy() safe on zero-init */

    return 0;
}