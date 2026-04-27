/* compiler: any */
/* Test: extern inline function with body */
extern int add(int a, int b) {
    return a + b;
}
int main() { return add(1, 2); }