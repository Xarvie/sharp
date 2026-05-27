// 来源: p226_const_pointer.sp
// const 指针嵌套测试 (const T*, T* const, const T* const)

#include <stdbool.h>

class ConstDemo {
    const int* a;      // pointer to const int
    int* const b;      // const pointer to int
    int* c;            // plain mut pointer
};

int ConstDemo.read_a(this) const {
    if (this->a) return *this->a;
    return -1;
}
void ConstDemo.set_a(this, const int* p) {
    this->a = p;
}
int ConstDemo.read_c(this) const {
    if (this->c) return *this->c;
    return -1;
}

// 混合三种 const 指针模式
int mix_const(const int* a, int* const b, int* c) {
    *b = *a;
    *c = *b + 1;
    return *c;
}

int main() {
    // --- pointer-to-const field ---
    ConstDemo d;
    int x = 42;
    d.a = &x;
    d.c = &x;
    int v1 = d.read_a();
    if (v1 != 42) return 1;

    int y = 99;
    d.set_a(&y);
    int v2 = d.read_a();
    if (v2 != 99) return 2;

    int v3 = d.read_c();
    if (v3 != 42) return 3;

    // --- mix_const free function ---
    int v = 10, w = 0, z = 0;
    int r = mix_const(&v, &w, &z);
    if (r != 11) return 4;
    if (w != 10) return 5;
    if (z != 11) return 6;

    // --- const pointer to struct ---
    ConstDemo* const pd = &d;
    int v4 = pd->read_a();
    if (v4 != 99) return 7;

    return 0;
}