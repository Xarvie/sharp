/* p243 — extension method returning pointer to this (escape pattern)
 *
 * Tests returning a pointer-to-this from an extension method.
 */

#include <stdbool.h>

class Counter {
    int val;
    int next;
};

Counter* Counter.inc(this) {
    this->val = this->val + 1;
    return this;
}

Counter* Counter.inc_next(this) {
    this->next = this->next + 1;
    return this;
}

int Counter.sum(this) const { return this->val + this->next; }

int main() {
    Counter c = {0};

    Counter* p = c.inc();
    if (p->val != 1) return 1;

    /* chain: inc → inc_next → inc */
    c.inc()->inc_next()->inc();
    if (c.val != 3) return 2;   /* 1+1+1 = 3 */
    if (c.next != 1) return 3;  /* 0+1 = 1 */

    /* pointer equality: p should point to c */
    if (p != &c) return 4;
    if (p->sum() != 4) return 5;  /* 3+1 = 4 */

    return 0;
}