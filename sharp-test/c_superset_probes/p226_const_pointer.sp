/* p226 — const pointer nesting (const T*, T* const, const T* const)
 *
 * Verifies that the compiler handles all three const-pointer
 * combinations in fields, parameters, return types, and locals.
 */

#include <stdbool.h>

/* ===== basic const pointer field ===== */
class ConstDemo {
    const int* a;      /* pointer to const int    — can't write *a */
    int* const b;      /* const pointer to int    — can't change b  */
    int* c;            /* plain mut pointer */
};

/* Extension method: read through const T* field */
int ConstDemo.read_a(this) const {
    if (this->a) return *this->a;
    return -1;
}

/* Extension method: modify the pointer value of a (not the int it points to) */
void ConstDemo.set_a(this, const int* p) {
    this->a = p;
}

/* Extension method: read through plain pointer via const this */
int ConstDemo.read_c(this) const {
    if (this->c) return *this->c;
    return -1;
}

/* ===== free function mixing all three const-pointer patterns ===== */
int mix_const(const int* a, int* const b, int* c) {
    *b = *a;           /* read from const-t, write through const-ptr b */
    *c = *b + 1;       /* write through plain ptr */
    return *c;
}

int main() {
    /* --- ConstDemo: pointer-to-const field --- */
    ConstDemo d;
    int x = 42;
    d.a = &x;
    d.c = &x;
    int v1 = d.read_a();
    if (v1 != 42) return 1;

    /* change pointer to point elsewhere */
    int y = 99;
    d.set_a(&y);
    int v2 = d.read_a();
    if (v2 != 99) return 2;

    /* read through plain pointer via const method */
    int v3 = d.read_c();
    if (v3 != 42) return 3;

    /* --- mix_const free function --- */
    int v = 10, w = 0, z = 0;
    int r = mix_const(&v, &w, &z);
    if (r != 11) return 4;
    if (w != 10) return 5;
    if (z != 11) return 6;

    /* --- const pointer to struct --- */
    ConstDemo* const pd = &d;    /* pd itself is const (can't reassign) */
    int v4 = pd->read_a();
    if (v4 != 99) return 7;

    return 0;
}