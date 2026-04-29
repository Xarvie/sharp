#define F(A, B, C, D, ...) D
#define G(...) F(__VA_ARGS__, 1, 0)

int main() {
    int x = G(~);
    return 0;
}
