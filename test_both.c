#define PASTE2(a, b) a ## b
#define PASTE3(a, b, c) a ## b ## c

int ABCD = 1;

int main() {
    /* Case 1: Direct nested call */
    int r1 = PASTE2(PASTE3(A, B, C), D);

    /* Case 2: Two-step expansion */
    int r2 = PASTE2(ABCD, 1);

    return 0;
}
