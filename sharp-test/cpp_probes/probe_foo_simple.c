#define FOO BAR
#define DISPATCH_CALL(x) call_##x
FOO
DISPATCH_CALL(X)