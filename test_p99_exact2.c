/* Exact P99 compatibility test */
#define P99_PASTE2(_1, _2) _1 ## _2
#define P99_PASTE3(_1, _2, _3) _1 ## _2 ## _3

#define P00_NARG(...) P00_NARG_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define P00_NARG_(_1, _2, _3, _4, _5, N, ...) N

#define P99_HAS_COMMA(...) P00_HAS_COMMA_(__VA_ARGS__, 1, 0)
#define P00_HAS_COMMA_(_1, _2, _3, HAS, ...) HAS

#define P00_TOK_EQ_(MAC, ...)  P00_TOK_EQ__(MAC, __VA_ARGS__)
#define P00_TOK_EQ__(MAC, ...) P99_HAS_COMMA(P99_PASTE2(P00_TOK_EQ_, P00_NARG(MAC ## __VA_ARGS__ (~) MAC ## __VA_ARGS__))(~))

#define P00_TOK_EQ_0(...) ~
#define P00_TOK_EQ_1(...) ~
#define P00_TOK_EQ_2(...) ,
#define P00_TOK_EQ_3(...) ~
#define P00_TOK_EQ_4(...) ~

/* P99_TOK_EQ is the public API */
#define P99_TOK_EQ(TOK, ...)  P00_TOK_EQ_(P99_PASTE3(P00_IS_, TOK, _EQ_), __VA_ARGS__)

/* P99 predefines these for common tokens */
#define P00_IS_1_EQ_(...) ,

int main() {
    /* This is exactly what p99_compiler.h does: P99_TOK_EQ(1, 1) */
    int result = P99_TOK_EQ(1, 1);
    return 0;
}
