/* p213 — constexpr: negative numbers and unary operators */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
constexpr int negate(int x) {
    return -x;
}

constexpr int positive(int x) {
    return +x;
}

constexpr int bitwise_not(int x) {
    return ~x;
}

constexpr int logical_not(int x) {
    return !x;
}

/* constexpr variables with negative values */
constexpr int NEG = -42;
constexpr int POS = +99;

/* constexpr with unary chain */
constexpr int double_negate(int x) {
    return -(-x);
}

int main(void) {
    if (negate(5) != -5) return 1;
    if (negate(-3) != 3) return 2;
    if (positive(7) != 7) return 3;
    if (bitwise_not(0) != -1) return 4;
    if (logical_not(0) != 1) return 5;
    if (logical_not(5) != 0) return 6;
    if (NEG != -42) return 7;
    if (POS != 99) return 8;
    if (double_negate(10) != 10) return 9;

    return 0;
}
