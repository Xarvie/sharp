/* compiler: any */
// TDD-§28: __STRICT_ANSI__ 条件编译
int main() {
#ifdef __STRICT_ANSI__
    return 42;
#else
    return 0;
#endif
}