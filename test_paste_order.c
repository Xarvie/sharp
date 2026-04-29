#define P99_PASTE2(_1, _2) _1 ## _2
#define P99_PASTE3(_1, _2, _3) _1 ## _2 ## _3

/* Test: does P99_PASTE3 expand before P99_PASTE2's ##? */
int ABC = 1;
int ABD = 2;

int main() {
    /* If P99_PASTE3 expands first, result is: P99_PASTE2(ABC, D) */
    /* Then P99_PASTE2 should paste ABC and D to get ABCD */
    return P99_PASTE2(P99_PASTE3(A, B, C), D);
}
