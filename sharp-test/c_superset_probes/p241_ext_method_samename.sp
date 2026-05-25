/* p241 — extension methods: same name on different structs
 *
 * Verifies that extension methods with the same name on
 * different structs dispatch correctly without conflict.
 */

#include <stdbool.h>

class A { int val; };
class B { int val; };

int A.get_val(this) const { return this->val + 100; }
int B.get_val(this) const { return this->val + 200; }
void A.set_val(this, int v) { this->val = v; }
void B.set_val(this, int v) { this->val = v; }

int main() {
    A a = {0};
    B b = {0};

    a.set_val(1);
    b.set_val(2);

    if (a.get_val() != 101) return 1;   /* A::get_val = val+100 */
    if (b.get_val() != 202) return 2;   /* B::get_val = val+200 */

    return 0;
}