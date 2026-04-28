#define PASTE(a, b) a ## b
#define PASTE2(a, b) PASTE(a, b)
#define HAS_COMMA(...) PASTE2(P00_TOK_EQ_, PASTE2(P00_, PASTE2(X, __VA_ARGS__)))

#define P00_TOK_EQ_1 ~
#define P00_TOK_EQ_2 ,

#define TEST MAC
int test = HAS_COMMA(TEST);
int main() { return 0; }
