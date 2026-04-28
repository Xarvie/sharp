#define P99_NARG(...) P99_NARG_(__VA_ARGS__, 3,2,1,0)
#define P99_NARG_(...) P99_ARG_N(__VA_ARGS__)
#define P99_ARG_N(_1,_2,_3,_4,N,...) N

/* Test: does P99_NARG work with 2 args? */
int x = P99_NARG(A, B);

/* Test: does it work inside ## ? */
#define PASTE(a, b) a ## b
#define TOK_0 ~
#define TOK_1 ,
#define TOK_2 ,

int y = TOK_2;
int z = PASTE(TOK_, P99_NARG(A, B));
