static __inline__ void test_void_return_builtin(void) {
    return (__extension__ (void)0);
}

static __inline__ void test_void_return_void_expr(void) {
    return (void)42;
}

int main(void) {
    test_void_return_builtin();
    test_void_return_void_expr();
    return 0;
}
