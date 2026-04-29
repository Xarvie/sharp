/* Test comma operator in #if - preprocessor output only */
#include <stdio.h>
#if 1, 1
#define RESULT 1
#else
#define RESULT 0
#endif

int main() {
    printf("RESULT=%d\n", RESULT);
    return RESULT - 1;
}
