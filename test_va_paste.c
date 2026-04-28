#define CAT(a, b) a ## b
#define PASTE2(a, b) CAT(a, b)
#define VARG(...) PASTE2(X, __VA_ARGS__)
int test = VARG(Y);
int main() { return 0; }
