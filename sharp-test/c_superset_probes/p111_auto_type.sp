/* p111 — auto type deduction
 *
 * Verifies spec §auto: `auto x = expr` infers the concrete type from the
 * initialiser; generated C must use the concrete type, not __auto_type.
 */

int identity(int x) { return x; }

struct Point { int x; int y; }
struct Point make_point(int x, int y) { struct Point p; p.x = x; p.y = y; return p; }

int main(void) {
    auto a = 42;
    if (a != 42) return 1;

    auto b = identity(7);
    if (b != 7) return 2;

    auto p = make_point(3, 4);
    if (p.x != 3) return 3;
    if (p.y != 4) return 4;

    /* auto in for-loop init */
    int sum = 0;
    for (auto i = 0; i < 5; i = i + 1)
        sum = sum + i;
    if (sum != 10) return 5;

    return 0;
}