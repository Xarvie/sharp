#define PASTE(a, b) a ## b
#define PASTE2(a, b) PASTE(a, b)
#define NARG_IMPL(_1,_2,_3,_4,N,...) N
#define NARG_(...) NARG_IMPL(__VA_ARGS__)
#define NARG(...) NARG_(__VA_ARGS__, 4,3,2,1,0)

/* Step by step test */
#define STEP1 NARG(X, Y)
int n = STEP1;

/* Full test */
#define TOK_EQ__(MAC, ...) PASTE2(TOK_EQ_, NARG(MAC __VA_ARGS__))
#define TOK_EQ_(MAC, ...) TOK_EQ__(MAC, __VA_ARGS__)
#define TOK_EQ_1 ~
#define TOK_EQ_2 ,

int test = TOK_EQ_(X, Y);
int main() { return 0; }
