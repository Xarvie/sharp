/*
 * Test: macro scope across #include
 * - A.sp defines X=1
 * - A.sp includes B.h (B.h uses X)
 * - A.sp undefs X
 * - After undef, X should not be defined anymore
 * 
 * Expected (same as zig cc):
 * - b_value = 1 (B.h sees X=1)
 */

#define X 1
#include "macro_include_b.h"
#undef X

#if defined(X)
int this_should_not_exist = 999;
#endif

int main(void) {
    return 0;
}
