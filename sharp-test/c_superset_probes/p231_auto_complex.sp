/* p231 — auto type deduction with complex expressions
 *
 * Verifies auto deduction for:
 *   - arithmetic expressions
 *   - function return values
 *   - ternary operator
 *   - pointer types
 */

#include <stdbool.h>

/* helper functions for auto deduction */
int get_int() { return 42; }
float get_float() { return 3.14f; }

/* ===== struct for auto with member access ===== */
class Data {
    int value;
};

int Data.read(this) const { return this->value; }
void Data.init(this, int v) { this->value = v; }

int main() {
    /* --- basic arithmetic --- */
    auto i1 = 1 + 2;
    if (i1 != 3) return 1;

    auto f1 = 1.0f + 2.0f;
    if (f1 < 2.9f || f1 > 3.1f) return 2;

    /* --- mixed int/float promotes --- */
    auto m1 = 1 + 2.5f;
    /* m1 should be float */
    if (m1 < 3.4f || m1 > 3.6f) return 3;

    /* --- function return value --- */
    auto r1 = get_int();
    if (r1 != 42) return 4;

    auto r2 = get_float();
    if (r2 < 3.13f || r2 > 3.15f) return 5;

    /* --- ternary --- */
    auto t1 = 1 ? 10 : 20;
    if (t1 != 10) return 6;

    /* --- struct member --- */
    Data d;
    d.init(77);
    auto v1 = d.read();
    if (v1 != 77) return 7;

    return 0;
}