/* p210 — constexpr: compile-time arithmetic and constants */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
/* constexpr variable */
constexpr int BUFFER_SIZE = 1024;

/* constexpr function: basic arithmetic */
constexpr int square(int x) {
    return x * x;
}

/* constexpr function: multiple operations */
constexpr int complex_calc(int a, int b) {
    int sum = a + b;
    int diff = a - b;
    return sum * diff;
}

/* constexpr function: sizeof in expression */
constexpr int array_element_count(int total_bytes) {
    return total_bytes / sizeof(int);
}

int main(void) {
    int buf[BUFFER_SIZE];
    if (sizeof(buf) != 4096) return 1;

    if (square(5) != 25) return 2;

    if (complex_calc(10, 3) != 91) return 3;

    if (array_element_count(64) != 16) return 4;

    return 0;
}
