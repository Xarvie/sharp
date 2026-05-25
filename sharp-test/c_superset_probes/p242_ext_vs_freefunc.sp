/* p242 — extension method + free function with same name
 *
 * Tests that an extension method (method syntax) is correctly
 * called via obj.method() and the free function via func(obj).
 */

#include <stdbool.h>

class Val {
    int x;
};

/* extension method */
int Val.double_val(this) const { return this->x * 2; }

/* free function with same base name (different name mangling) */
int double_val(Val* v) { return v->x * 3; }

int main() {
    Val v; v.x = 10;

    /* extension method via . syntax */
    if (v.double_val() != 20) return 1;

    /* free function via explicit call */
    int r = double_val(&v);
    if (r != 30) return 2;

    return 0;
}