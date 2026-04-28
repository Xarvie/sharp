#define P00_NARG(...) P00_NARG_(__VA_ARGS__, 5, 4, 3, 2, 1, 0)
#define P00_NARG_(_1, _2, _3, _4, _5, N, ...) N
#define P99_HAS_COMMA(...) P00_HAS_COMMA_(__VA_ARGS__, 1, 0)
#define P00_HAS_COMMA_(_1, _2, _3, HAS, ...) HAS
#define P99_PASTE2(_1, _2) _1 ## _2
#define P00_TOK_EQ_(MAC, ...)  P00_TOK_EQ__(MAC, __VA_ARGS__)
#define P00_TOK_EQ__(MAC, ...) P99_HAS_COMMA(P99_PASTE2(P00_TOK_EQ_, P00_NARG(MAC ## __VA_ARGS__ (~) MAC ## __VA_ARGS__))(~))

#define P00_TOK_EQ_0(...) ~
#define P00_TOK_EQ_1(...) ~
#define P00_TOK_EQ_2(...) ,
#define P00_TOK_EQ_3(...) ~
#define P00_TOK_EQ_4(...) ~

#define P00_IS_INT_EQ_(...) ,

int main() {
    int x = P00_TOK_EQ_(P00_IS_INT_EQ_, 1);
    return 0;
}
