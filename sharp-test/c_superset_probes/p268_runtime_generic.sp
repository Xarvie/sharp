/* p268 — Runtime: generic container
 *
 * Verifies monomorphized generic code works at runtime.
 */

#include "../../std/types.sph"

class Pair<T> {
    T first;
    T second;
};

void Pair<T>.swap(this) {
    T tmp = this->first;
    this->first  = this->second;
    this->second = tmp;
}

int main() {
    Pair<int> p;
    p.first  = 10;
    p.second = 20;

    if (p.first != 10) return 1;
    if (p.second != 20) return 2;

    p.swap();

    if (p.first != 20) return 3;
    if (p.second != 10) return 4;

    /* Second instantiation: Pair<float> */
    Pair<float> pf;
    pf.first  = 1.5f;
    pf.second = 2.5f;

    if (pf.first != 1.5f) return 5;
    if (pf.second != 2.5f) return 6;

    pf.swap();

    if (pf.first != 2.5f) return 7;
    if (pf.second != 1.5f) return 8;

    return 0;
}