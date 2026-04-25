/* compiler: any */
/* Test: extern __inline__ with body */
int add(int a, int b) {
    return a + b;
}
i32 main() { return add(1, 2); }
