#define CAT(a, b) a ## b
#define X hello
#define Y world
int test = CAT(X, Y);
int main() { return 0; }
