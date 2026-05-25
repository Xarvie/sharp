/* p266 — Runtime: defer execution order
 *
 * Verifies that defer statements execute correctly at runtime.
 */

int seq[10];
int seqn = 0;
void mark(int v) { seq[seqn++] = v; }

int test_defer_return(int v) {
    defer mark(100);
    if (v > 0) return v;
    return 0;
}

void test_defer_lifo(void) {
    defer mark(1);
    defer mark(2);
    defer mark(3);
}

int main() {
    /* T1: defer fires on return */
    seqn = 0;
    int r = test_defer_return(42);
    if (r != 42) return 1;
    if (seqn != 1) return 2;
    if (seq[0] != 100) return 3;

    /* T2: LIFO order */
    seqn = 0;
    test_defer_lifo();
    if (seqn != 3) return 4;
    if (seq[0] != 3) return 5;
    if (seq[1] != 2) return 6;
    if (seq[2] != 1) return 7;

    /* T3: defer in for loop */
    seqn = 0;
    for (int i = 0; i < 3; i = i + 1) {
        defer mark(i);
    }
    /* Fires 3 times: i=2,1,0 */
    if (seqn != 3) return 8;

    return 0;
}