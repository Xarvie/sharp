/* p270 — Codegen: multiple generic instantiations in one file
 *
 * Verifies that multiple different instantiations of the same generic
 * type are generated correctly without conflicts.
 */

#include "../../sharp/std/types.sph"

class Wrap<T> {
    T value;
};

void Wrap<T>.set(this, T v) { this->value = v; }
T    Wrap<T>.get(this) const { return this->value; }

int main() {
    Wrap<int> wi;
    wi.set(42);
    if (wi.get() != 42) return 1;

    Wrap<float> wf;
    wf.set(3.14f);
    if (wf.get() != 3.14f) return 2;

    /* Two different instantiations, same file */
    Wrap<int> wi2;
    wi2.set(99);
    if (wi2.get() != 99) return 3;

    return 0;
}