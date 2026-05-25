/* p257 — multiple return paths with defer
 *
 * Tests: defer fires on every return path, including early returns.
 */

int seq[8];
int seqn = 0;
void mark(int v) { seq[seqn++] = v; }

/* T1: defer fires on both return paths */
int test_multi_return(int v) {
    defer mark(1);
    if (v > 0) return 10;
    return 20;
}

/* T2: defer fires on early return */
int test_early_return(int v) {
    defer mark(2);
    if (v == 1) return 100;
    if (v == 2) return 200;
    return 300;
}

int main() {
    /* T1: multi return */
    seqn = 0;
    int r1 = test_multi_return(1);
    if (r1 != 10) return 1;
    if (seqn != 1) return 2;  /* defer fired once */
    if (seq[0] != 1) return 3;

    seqn = 0;
    int r2 = test_multi_return(0);
    if (r2 != 20) return 4;
    if (seqn != 1) return 5;
    if (seq[0] != 1) return 6;

    /* T2: early returns */
    seqn = 0;
    int r3 = test_early_return(1);
    if (r3 != 100) return 7;
    if (seqn != 1) return 8;
    if (seq[0] != 2) return 9;

    seqn = 0;
    int r4 = test_early_return(2);
    if (r4 != 200) return 10;
    if (seqn != 1) return 11;

    seqn = 0;
    int r5 = test_early_return(3);
    if (r5 != 300) return 12;
    if (seqn != 1) return 13;

    return 0;
}