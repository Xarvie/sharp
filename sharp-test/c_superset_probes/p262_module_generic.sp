/* p262 — Module with generic types
 *
 * Tests importing a module with generic struct + methods, instantiating it.
 */

#include <stdbool.h>
#include "p262_ringbuf.sph"

int main() {
    int buf[8] = {0};

    RingBuf<int> rb;
    rb.init(buf, 8);

    if (rb.size() != 0) return 1;

    bool ok = rb.push(10);
    if (!ok) return 2;
    ok = rb.push(20);
    if (!ok) return 3;
    ok = rb.push(30);
    if (!ok) return 4;

    if (rb.size() != 3) return 5;

    int val = 0;
    ok = rb.pop(&val);
    if (!ok) return 6;
    if (val != 10) return 7;

    ok = rb.pop(&val);
    if (!ok) return 8;
    if (val != 20) return 9;

    ok = rb.pop(&val);
    if (!ok) return 10;
    if (val != 30) return 11;

    if (rb.size() != 0) return 12;

    return 0;
}