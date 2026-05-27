// 来源: p222_mix_class_struct_op.sp
// 跨类型自由函数运算符: class + struct 混合

#include <stdbool.h>

// class (auto-typedef) vs struct
class VecC {
    float x; float y;
}
struct VecS { float x; float y; }

// class + struct => class
VecC operator+(VecC a, struct VecS b) {
    VecC r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}
// struct + class => class (reverse)
VecC operator+(struct VecS a, VecC b) {
    VecC r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}
// class == struct => int (cross-type comparison)
int operator==(VecC a, struct VecS b) {
    return a.x == b.x && a.y == b.y;
}

int main(void) {
    VecC c1; c1.x = 1.0f; c1.y = 2.0f;
    struct VecS s1; s1.x = 10.0f; s1.y = 20.0f;
    struct VecS s2; s2.x = 30.0f; s2.y = 40.0f;

    // class + struct => class
    VecC cs = c1 + s1;
    if (cs.x != 11.0f) return 1;
    if (cs.y != 22.0f) return 2;

    // struct + class => class
    VecC sc = s1 + c1;
    if (sc.x != 11.0f) return 3;
    if (sc.y != 22.0f) return 4;

    // cross-type equality
    VecC eq_test; eq_test.x = 10.0f; eq_test.y = 20.0f;
    if (!(eq_test == s1)) return 5;
    if (eq_test == s2) return 6;

    return 0;
}