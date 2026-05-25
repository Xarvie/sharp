/* p212 — constexpr: ternary operator and conditionals */
/* TODO: constexpr codegen — currently emits `constexpr` keyword to C but zig cc rejects it;
 * constexpr vars should emit as `enum { NAME = value };`, functions as values/regular fns. */
constexpr int max(int a, int b) {
    return a > b ? a : b;
}

constexpr int min(int a, int b) {
    return a < b ? a : b;
}

constexpr int abs_val(int x) {
    return x < 0 ? -x : x;
}

constexpr int clamp(int val, int lo, int hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

/* constexpr variable with ternary */
constexpr int FLAG = 1;
constexpr int MODE = FLAG ? 100 : 200;

int main(void) {
    if (max(3, 7) != 7) return 1;
    if (min(3, 7) != 3) return 2;
    if (abs_val(-5) != 5) return 3;
    if (abs_val(5) != 5) return 4;
    if (clamp(150, 0, 100) != 100) return 5;
    if (clamp(-10, 0, 100) != 0) return 6;
    if (clamp(50, 0, 100) != 50) return 7;
    if (MODE != 100) return 8;

    return 0;
}
