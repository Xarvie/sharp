/* p109 — defer: break and continue inside loops
 *
 * Verifies spec §defer P10.4:
 * break/continue emit defers for the *current* block only.
 */

int log_buf[16];
int logn = 0;
void mark(int v) { log_buf[logn++] = v; }

int main(void) {
    /* T1: break triggers defer; iterations before break also trigger */
    logn = 0;
    int i = 0;
    while (i < 5) {
        defer mark(7);
        i = i + 1;
        if (i == 3) break;
    }
    /* ran for i=1,2,3 (break on i==3 still runs defer) → 3 marks */
    if (logn != 3) return 1;
    if (log_buf[0] != 7 || log_buf[1] != 7 || log_buf[2] != 7) return 2;

    /* T2: continue triggers defer and continues loop */
    logn = 0;
    int sum = 0;
    for (int j = 0; j < 4; j++) {
        defer mark(9);
        if (j == 2) continue;   /* defer fires, then next iteration */
        sum = sum + j;
    }
    /* all 4 iterations fire defer */
    if (logn != 4) return 3;
    /* sum = 0+1+3 = 4 (j==2 skipped by continue) */
    if (sum != 4) return 4;

    return 0;
}
