#define PASTE(a, b) a ## b
#define PASTE2(a, b) PASTE(a, b)
#define MAC(A, ...) PASTE2(TOK_EQ_, PASTE2(NARG_, MAC2(A ## __VA_ARGS__)))
#define MAC2(x) x
#define NARG_1 1
#define NARG_2 2
#define TOK_EQ_1 ~
#define TOK_EQ_2 ,

int x = MAC(A, B);
int y = TOK_EQ_2;
