/* p265 — Runtime: extension method chains
 *
 * Verifies that extension method calls work correctly at runtime.
 */

class Accumulator {
    int total;
};

void Accumulator.reset(this) { this->total = 0; }
void Accumulator.add(this, int n) { this->total = this->total + n; }
int  Accumulator.get(this) const { return this->total; }

int main() {
    Accumulator acc;
    acc.reset();

    acc.add(10);
    acc.add(20);
    acc.add(30);

    if (acc.get() != 60) return 1;

    acc.reset();
    if (acc.get() != 0) return 2;

    acc.add(5);
    if (acc.get() != 5) return 3;

    /* Chain of operations */
    int sum = 0;
    for (int i = 1; i <= 100; i = i + 1) {
        sum = sum + i;
    }
    if (sum != 5050) return 4;

    return 0;
}