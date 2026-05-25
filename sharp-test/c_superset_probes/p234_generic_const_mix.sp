/* p234 — generic + const mixed
 *
 * Tests const generic struct field access via extension methods.
 */

#include <stdbool.h>

class Pair<T, U> {
    T first;
    U second;
};

/* const accessor — should be callable on const Pair */
T Pair.first_val(this) const {
    return this->first;
}

U Pair.second_val(this) const {
    return this->second;
}

void Pair.set_first(this, T val) {
    this->first = val;
}

/* free function accepting const generic */
int read_pair(const Pair<int, float>* p) {
    return p->first_val();
}

int main() {
    Pair<int, float> p = {0};
    p.set_first(42);
    p.second = 3.14f;

    if (p.first_val() != 42) return 1;
    if (p.second_val() < 3.13f || p.second_val() > 3.15f) return 2;

    /* non-const Pair to const generic function */
    int r = read_pair(&p);
    if (r != 42) return 3;

    return 0;
}