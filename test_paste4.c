#define P99_PASTE2(_1, _2) _1 ## _2
#define P99_PASTE3(_1, _2, _3) _1 ## _2 ## _3
#define P99_PASTE4(_1, _2, _3, _4) P99_PASTE2(P99_PASTE3(_1, _2, _3), _4)

int ABCD = 1;

int main() {
    return P99_PASTE4(A, B, C, D);
}
