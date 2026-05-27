// Merged nested generic tests
// Sources: p233_generic_nested3.sp, p152_vec_stdlib.sp, p153_generic_edge.sp

#include <stdbool.h>
#include <stdlib.h>

typedef long isize;
typedef unsigned long usize;

/* Vec<T> template (p152/p153) */
class Vec<T> {
    T* data;
    isize len;
    isize cap;
}

void Vec<T>.push(this, T val) {
    if (this->len >= this->cap) {
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;
        T* nd = (T*)realloc(this->data, sizeof(T) * new_cap);
        if (!nd) __builtin_trap();
        this->data = nd;
        this->cap = new_cap;
    }
    this->data[this->len] = val;
    this->len = this->len + 1;
}

isize Vec<T>.size(this) const {
    return this->len;
}

T Vec<T>.get(this, isize i) const {
    if (i < 0 || i >= this->len) __builtin_trap();
    return this->data[i];
}

void Vec<T>.destroy(this) {
    free(this->data);
    this->data = (T*)0;
    this->len = 0;
    this->cap = 0;
}

/* Nested generic struct (p233/p153) */
class Pair<T, U> {
    T first;
    U second;
}

int main() {
    /* Test basic Vec<int> (p152) */
    Vec<int> v = {0};
    v.push(42);
    v.push(73);
    if (v.size() != 2) return 1;
    if (v.get(0) != 42) return 2;
    if (v.get(1) != 73) return 3;
    v.destroy();

    /* Test Vec<float> (p152) */
    Vec<float> fv = {0};
    fv.push(1.5f);
    fv.push(2.5f);
    if (fv.size() != 2) return 4;
    if (fv.get(0) < 1.4f || fv.get(0) > 1.6f) return 5;
    fv.destroy();

    /* Test 3-level nested: Vec<Vec<Vec<int>>> (p233) */
    Vec<Vec<Vec<int>>> v3d = {0};
    Vec<Vec<int>> inner2d = {0};
    Vec<int> inner1d = {0};
    inner1d.push(42);
    inner1d.push(73);
    inner2d.push(inner1d);
    v3d.push(inner2d);

    if (v3d.size() != 1) return 6;
    Vec<Vec<int>> r2 = v3d.get(0);
    if (r2.size() != 1) return 7;
    Vec<int> r1 = r2.get(0);
    if (r1.size() != 2) return 8;
    if (r1.get(0) != 42) return 9;
    r1.destroy();
    r2.destroy();
    v3d.destroy();

    /* Test 2-level nested: Vec<Vec<int>> matrix (p233) */
    Vec<Vec<int>> m2d = {0};
    Vec<int> row0 = {0};
    Vec<int> row1 = {0};
    row0.push(1);
    row1.push(10);
    row1.push(20);
    m2d.push(row0);
    m2d.push(row1);

    if (m2d.size() != 2) return 10;
    Vec<int> c0 = m2d.get(0);
    if (c0.get(0) != 1) return 11;
    c0.destroy();
    Vec<int> c1 = m2d.get(1);
    if (c1.get(1) != 20) return 12;
    c1.destroy();
    m2d.destroy();

    /* Test Pair with generic types (p153) */
    Pair<int, Vec<int>> pv = {0};
    pv.first = 42;

    return 0;
}
