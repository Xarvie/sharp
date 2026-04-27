/* compiler: any */
/* TDD-Expr-4: bool literals and logic operators */
#include <stdbool.h>

int main() {
    bool t = true;
    bool f = false;
    if (t && !f) {
        return 1;
    }
    return 0;
}