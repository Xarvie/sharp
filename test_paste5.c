/* Test nested PASTE that fails in p99_paste.h:88 */
#define P99_PASTE2(_1, _2) _1 ## _2
#define P99_PASTE3(_1, _2, _3) _1 ## _2 ## _3
#define P99_PASTE4(_1, _2, _3, _4) P99_PASTE2(P99_PASTE3(_1, _2, _3), _4)
#define P99_PASTE5(_1, _2, _3, _4, _5) P99_PASTE2(P99_PASTE4(_1, _2, _3, _4), _5)

int ABCDE = 1;

int main() {
    return P99_PASTE5(A, B, C, D, E);
}
