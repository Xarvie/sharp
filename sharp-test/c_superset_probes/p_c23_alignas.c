/* C23 alignas keyword — must be parsed as declaration specifier.
 * Tests: alignas(N) before type in variable declarations. */
#include <stdalign.h>

alignas(16) int aligned_global;

struct Packed {
    char c;
    int  x;
};

int main(void) {
    alignas(32) double local_d;
    (void)local_d;
    return 0;
}
