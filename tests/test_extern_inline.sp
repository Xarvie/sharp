/* Test: extern inline function with body */
extern int add(int a, int b) {
    return a + b;
}
i32 main() { return add(1, 2); }
