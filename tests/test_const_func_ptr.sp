/* compiler: any */
/* C Feature: const function pointer declarations */
/* The generated C code must produce correct syntax like:
 *   int (* const fp)(int, int)
 */
#include <stddef.h>

int main() {
    int (* const fp)(int, int) = NULL;
    return 0;
}