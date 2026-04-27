/* compiler: any */
/* §12 __restrict__ 关键字 */
#include <stddef.h>

extern int strlen(const char* __restrict__ s);

int main() {
    int* __restrict__ p = NULL;
    return 0;
}