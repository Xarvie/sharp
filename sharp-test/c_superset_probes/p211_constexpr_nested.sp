/* p211 — constexpr: nested function calls and variable chains */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
/* constexpr variable chain */
constexpr int BASE = 10;
constexpr int DOUBLE_BASE = BASE * 2;
constexpr int QUAD_BASE = DOUBLE_BASE * 2;

/* constexpr function calling another constexpr function */
constexpr int square(int x) {
    return x * x;
}

constexpr int fourth_power(int x) {
    return square(square(x));
}

/* constexpr function with multiple parameter usage */
constexpr int mix(int a, int b, int c) {
    return a * b + c;
}

/* constexpr with sizeof chain */
constexpr int bytes_for_array(int count) {
    return count * sizeof(int);
}

int main(void) {
    /* Test variable chain */
    if (DOUBLE_BASE != 20) return 1;
    if (QUAD_BASE != 40) return 2;

    /* Test nested function calls */
    if (fourth_power(2) != 16) return 3;
    if (fourth_power(3) != 81) return 4;

    /* Test mixed parameters */
    if (mix(2, 3, 4) != 10) return 5;

    /* Test sizeof in chain */
    if (bytes_for_array(5) != 20) return 6;

    /* Test using constexpr in array size */
    int arr[QUAD_BASE];
    if (sizeof(arr) != 160) return 7;

    return 0;
}
