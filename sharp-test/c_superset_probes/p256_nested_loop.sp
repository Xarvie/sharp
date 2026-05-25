/* p256 — nested loops with break/continue
 *
 * Tests: break from inner loop only, nested for loops, while inside for.
 */

int main() {
    /* T1: break from inner loop only — outer continues */
    int count = 0;
    for (int i = 0; i < 5; i = i + 1) {
        for (int j = 0; j < 5; j = j + 1) {
            if (j == 3) break;
            count = count + 1;
        }
    }
    /* outer runs 5 times, inner breaks at j=3 → 3*5=15 */
    if (count != 15) return 1;

    /* T2: continue in inner loop */
    int sum = 0;
    for (int i = 0; i < 3; i = i + 1) {
        for (int j = 0; j < 3; j = j + 1) {
            if (j == 1) continue;
            sum = sum + i + j;
        }
    }
    /* i=0: j=0(sum+0) j=2(sum+2) → +2
       i=1: j=0(sum+1) j=2(sum+3) → +4
       i=2: j=0(sum+2) j=2(sum+4) → +6  total=12 */
    if (sum != 12) return 2;

    /* T3: while inside for */
    int accum = 0;
    for (int i = 0; i < 3; i = i + 1) {
        int j = 3;
        while (j > 0) {
            accum = accum + i;
            j = j - 1;
        }
    }
    /* i=0: 0*3=0, i=1: 3*1=3, i=2: 3*2=6 → total=9 */
    if (accum != 9) return 3;

    /* T4: nested for with different loop variables */
    int grid = 0;
    for (int row = 0; row < 4; row = row + 1) {
        for (int col = 0; col < 3; col = col + 1) {
            grid = grid + 1;
        }
    }
    if (grid != 12) return 4;

    return 0;
}