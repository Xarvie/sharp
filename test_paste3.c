#define PASTE(a, b) a ## b
#define PASTE2(a, b) PASTE(a, b)
#define NARG_IMPL(_1,_2,_3,_4,N,...) N
#define NARG_(...) NARG_IMPL(__VA_ARGS__)
#define NARG(...) NARG_(__VA_ARGS__, 4,3,2,1,0)

/* Test NARG first */
#define TEST_NARG NARG(X, Y)
int n = TEST_NARG;

/* Test PASTE */
#define TOK_EQ_2 ,
#define TEST_PASTE PASTE2(TOK_EQ_, 2)
int test = TEST_PASTE;
int main() { return 0; }
