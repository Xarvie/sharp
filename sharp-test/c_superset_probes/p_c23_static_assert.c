/* C23 static_assert keyword — must be parsed at file scope.
 * Tests: static_assert with constant expressions. */

static_assert(sizeof(int) >= 4, "int too small");
static_assert(sizeof(char) == 1, "char size wrong");
static_assert(1 == 1, "basic truth");

int main(void) {
    return 0;
}
