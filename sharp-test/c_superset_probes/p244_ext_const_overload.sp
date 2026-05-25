/* p244 — extension method const overloading
 *
 * Tests const and non-const variants of the same method name.
 */

#include <stdbool.h>

class Buffer {
    int data;
};

/* const accessor */
int Buffer.read(this) const {
    return this->data;
}

/* non-const mutator (different name to avoid overload ambiguity in C) */
void Buffer.write(this, int v) {
    this->data = v;
}

/* free function that takes const pointer */
int read_const(const Buffer* b) {
    return b->read();
}

int main() {
    Buffer b = {0};
    b.write(42);

    if (b.read() != 42) return 1;

    /* const access through const pointer */
    const Buffer cb = b;
    if (read_const(&cb) != 42) return 2;

    return 0;
}