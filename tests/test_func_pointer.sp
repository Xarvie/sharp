/* compiler: any */
/* C Declarator: Function pointer variable declaration */
/* Expected: Parser should handle int (*fp)(int, int) syntax */
#include <stddef.h>

int main() {
    int (*fp)(int, int) = NULL;
    return 0;
}