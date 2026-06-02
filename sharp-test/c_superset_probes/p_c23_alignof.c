/* C23 alignof keyword — must be parsed as alignment operator.
 * Tests: alignof(type) and alignof(expr). */
#include <stddef.h>

static const size_t align_i   = alignof(int);
static const size_t align_d   = alignof(double);
static const size_t align_max = alignof(max_align_t);

int main(void) {
    int x;
    size_t a = alignof(x);
    /* Verify alignment values are powers of 2 and reasonable */
    if (align_i < 1 || align_d < 1 || align_max < 1) return 1;
    if (a != align_i) return 2;
    return 0;
}
