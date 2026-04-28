#define PASTE(a, b) a ## b
#define TEST(MAC, ...) MAC ## __VA_ARGS__
int main() { int AB = 1; return TEST(A, B); }
