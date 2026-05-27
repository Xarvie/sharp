// 来源: p239_method_chain.sp, p246_operator_chain.sp
// 方法链式调用 + 运算符链式调用测试

#include <stdbool.h>

// === p239: extension method chaining (return *this) ===
class Counter {
    int val;
};
Counter* Counter.inc(this) {
    this->val = this->val + 1;
    return this;
}
Counter* Counter.add(this, int n) {
    this->val = this->val + n;
    return this;
}
int Counter.get(this) const { return this->val; }

// === p246: operator chaining (a + b + c) ===
class Vec3 {
    int x, y, z;
};
Vec3 Vec3.operator+(this, Vec3 other) const {
    Vec3 r;
    r.x = this->x + other.x;
    r.y = this->y + other.y;
    r.z = this->z + other.z;
    return r;
}
bool Vec3.operator==(this, Vec3 other) const {
    return this->x == other.x
        && this->y == other.y
        && this->z == other.z;
}

int main() {
    // --- p239 测试: method chaining ---
    Counter c = {0};
    c.inc()->inc()->add(5);
    if (c.get() != 7) return 1;   // 0+1+1+5 = 7
    c.add(10)->inc()->inc()->inc();
    if (c.get() != 20) return 2;  // 7+10+1+1+1 = 20

    // --- p246 测试: operator chaining ---
    Vec3 a; a.x = 1; a.y = 2; a.z = 3;
    Vec3 b; b.x = 10; b.y = 20; b.z = 30;
    Vec3 cz; cz.x = 100; cz.y = 200; cz.z = 300;

    Vec3 r = a + b + cz;
    if (r.x != 111) return 3;   // 1+10+100
    if (r.y != 222) return 4;   // 2+20+200
    if (r.z != 333) return 5;   // 3+30+300

    // (a + b) == b + a
    Vec3 left  = a + b;
    Vec3 right = b + a;
    if (!(left == right)) return 6;

    return 0;
}