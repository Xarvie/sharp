/* p214 — constexpr: bitwise operations and shifts */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
constexpr int set_bit(int n, int bit) {
    return n | (1 << bit);
}

constexpr int clear_bit(int n, int bit) {
    return n & ~(1 << bit);
}

constexpr int toggle_bit(int n, int bit) {
    return n ^ (1 << bit);
}

constexpr int test_bit(int n, int bit) {
    return (n >> bit) & 1;
}

/* constexpr with bitwise chain */
constexpr int mask_4_bits(int n) {
    return n & 0xF;
}

/* constexpr variable with bitwise expression */
constexpr int ALL_BITS = 0xFF;
constexpr int LOWER_NIBBLE = ALL_BITS & 0x0F;

int main(void) {
    if (set_bit(0, 3) != 8) return 1;
    if (clear_bit(0xFF, 4) != 0xEF) return 2;
    if (toggle_bit(0, 5) != 32) return 3;
    if (toggle_bit(32, 5) != 0) return 4;
    if (test_bit(0xA, 1) != 1) return 5;
    if (test_bit(0xA, 2) != 0) return 6;
    if (mask_4_bits(0xAB) != 0xB) return 7;
    if (LOWER_NIBBLE != 0x0F) return 8;

    return 0;
}
