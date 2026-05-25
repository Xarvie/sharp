/* p235 — generic struct zero-init
 *
 * Verifies that generic structs zero-initialize correctly.
 */

#include <stdbool.h>

class Maybe<T> {
    bool has;
    T    val;
};

bool Maybe.has_val(this) const { return this->has; }
T    Maybe.get_val(this) const { return this->val; }
void Maybe.set_val(this, T v) { this->has = true; this->val = v; }

int main() {
    Maybe<int> mi = {0};
    if (mi.has_val()) return 1;  /* should be false */

    mi.set_val(42);
    if (!mi.has_val()) return 2;
    if (mi.get_val() != 42) return 3;

    Maybe<float> mf = {0};
    if (mf.has_val()) return 4;

    mf.set_val(1.5f);
    if (mf.get_val() < 1.49f || mf.get_val() > 1.51f) return 5;

    return 0;
}