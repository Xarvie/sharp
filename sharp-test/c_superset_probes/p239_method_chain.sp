/* p239 — extension method chaining (return *this)
 *
 * Tests that extension methods returning *this enable
 * fluent chaining: obj.m1().m2().m3()
 */

#include <stdbool.h>

class Counter {
    int val;
};

Counter* Counter.inc(this) {
    this->val = this->val + 1;
    return this;
}

Counter* Counter.add(this, int n) {
    this->val = this->val + n;
    return this;
}

int Counter.get(this) const { return this->val; }

int main() {
    Counter c = {0};

    /* chain: inc → inc → add(5) */
    c.inc()->inc()->add(5);

    if (c.get() != 7) return 1;   /* 0+1+1+5 = 7 */

    /* chain: add(10) → inc → inc → inc */
    c.add(10)->inc()->inc()->inc();
    if (c.get() != 20) return 2;  /* 7+10+1+1+1 = 20 */

    return 0;
}