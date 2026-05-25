/* p254 — switch statement edge cases
 *
 * Tests: fallthrough (shared body), break, default, enum-like patterns.
 */

/* T3: switch with return inside (no break needed) */
int test_ret(int val) {
    switch (val) {
        case 1: return 1;
        case 2: return 2;
        default: return 42;
    }
}

int main() {
    /* T1: basic switch with break */
    int r1 = 0;
    int x = 2;
    switch (x) {
        case 1: r1 = 10; break;
        case 2: r1 = 20; break;
        case 3: r1 = 30; break;
        default: r1 = 0; break;
    }
    if (r1 != 20) return 1;

    /* T2: fallthrough — multiple cases sharing body */
    int r2 = 0;
    int y = 4;
    switch (y) {
        case 1:
        case 2:
        case 3: r2 = 3; break;
        case 4:
        case 5: r2 = 5; break;
        default: r2 = 0; break;
    }
    if (r2 != 5) return 2;

    if (test_ret(1) != 1) return 3;
    if (test_ret(2) != 2) return 4;
    if (test_ret(99) != 42) return 5;

    /* T4: default-only switch */
    int r4 = 0;
    int z = 7;
    switch (z) {
        default: r4 = 99; break;
    }
    if (r4 != 99) return 6;

    /* T5: empty switch body */
    switch (x) { /* nothing */ }

    return 0;
}