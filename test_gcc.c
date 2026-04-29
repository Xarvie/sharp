#define PASTE2(a, b) a ## b
#define PASTE3(a, b, c) a ## b ## c

int main() {
    return PASTE2(PASTE3(A, B, C), D);
}
