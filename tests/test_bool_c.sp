/* compiler: any */
/* TDD-§11: _Bool type support */
#include <stdbool.h>

int main() {
    bool flag = true;
    if (flag) {
        return 42;
    }
    return 0;
}