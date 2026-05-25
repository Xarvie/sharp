/* p258 — for loop edge cases
 *
 * Tests: empty init/cond/incr, infinite loop with break, comma in for.
 */
/* FIXED(e2e): for-loop comma init scoping — compound init + scope.c build fix */
int main() {
    /* T1: for with no init */
    int i = 5;
    int s1 = 0;
    for (; i > 0; i = i - 1) {
        s1 = s1 + i;
    }
    if (s1 != 15) return 1;  /* 5+4+3+2+1=15 */

    /* T2: for with no condition (infinite loop with break) */
    int count = 0;
    int j = 0;
    for (;;) {
        count = count + 1;
        j = j + 1;
        if (j >= 10) break;
    }
    if (count != 10) return 2;

    /* T3: for with no increment */
    int power = 1;
    int k = 0;
    for (; k < 4;) {
        power = power * 2;
        k = k + 1;
    }
    if (power != 16) return 3;  /* 1*2^4=16 */

    /* T4: for with comma expression in init */
    int a = 0, b = 0;
    for (int m = 0, n = 10; m < 5; m = m + 1) {
        a = a + m;
        b = b + n - m;
    }
    /* a=0+1+2+3+4=10, b=10+9+8+7+6=40 */
    if (a != 10) return 4;
    if (b != 40) return 5;

    /* T5: zero-iteration loop */
    int hit = 0;
    for (int p = 0; p < 0; p = p + 1) {
        hit = 1;
    }
    if (hit != 0) return 6;

    return 0;
}