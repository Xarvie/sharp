/* p255 — do-while loop
 *
 * Tests: do-while basic, do-while with break, do-while with continue.
 */

int main() {
    /* T1: basic do-while */
    int sum = 0;
    int i = 0;
    do {
        sum = sum + i;
        i = i + 1;
    } while (i < 5);
    if (sum != 10) return 1;  /* 0+1+2+3+4=10 */

    /* T2: do-while executes at least once even with false condition */
    int flag = 0;
    do {
        flag = 1;
    } while (0);
    if (flag != 1) return 2;

    /* T3: break inside do-while */
    int count = 0;
    int j = 0;
    do {
        if (j == 3) break;
        count = count + 1;
        j = j + 1;
    } while (j < 10);
    if (count != 3) return 3;

    /* T4: continue inside do-while */
    int sum2 = 0;
    int k = -1;
    do {
        k = k + 1;
        if (k == 3) continue;
        sum2 = sum2 + k;
    } while (k < 5);
    /* k: 0,1,2,3(skip),4,5 → sum2 = 0+1+2+4+5 = 12 */
    if (sum2 != 12) return 4;

    return 0;
}