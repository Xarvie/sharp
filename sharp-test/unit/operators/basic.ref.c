#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/operators/basic.sp"
struct Vec2 {
    float x;
    float y;
};

#line 20 "unit/operators/basic.sp"
typedef struct Vec2m Vec2m;
struct Vec2m {
    float x;
    float y;
};

#line 23 "unit/operators/basic.sp"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs);

#line 28 "unit/operators/basic.sp"
typedef struct IntBox IntBox;
struct IntBox {
    int value;
};

#line 31 "unit/operators/basic.sp"
IntBox IntBox__op_add(IntBox * this, IntBox other);

#line 34 "unit/operators/basic.sp"
IntBox IntBox__op_sub(IntBox * this, IntBox other);

#line 37 "unit/operators/basic.sp"
IntBox IntBox__op_mul(IntBox * this, IntBox other);

#line 40 "unit/operators/basic.sp"
IntBox IntBox__op_div(IntBox * this, IntBox other);

#line 43 "unit/operators/basic.sp"
IntBox IntBox__op_mod(IntBox * this, IntBox other);

#line 46 "unit/operators/basic.sp"
_Bool IntBox__op_eq(IntBox * this, IntBox other);

#line 49 "unit/operators/basic.sp"
_Bool IntBox__op_ne(IntBox * this, IntBox other);

#line 52 "unit/operators/basic.sp"
_Bool IntBox__op_lt(IntBox * this, IntBox other);

#line 55 "unit/operators/basic.sp"
_Bool IntBox__op_gt(IntBox * this, IntBox other);

#line 58 "unit/operators/basic.sp"
_Bool IntBox__op_le(IntBox * this, IntBox other);

#line 61 "unit/operators/basic.sp"
_Bool IntBox__op_ge(IntBox * this, IntBox other);

#line 66 "unit/operators/basic.sp"
struct Point {
    int x;
    int y;
};

#line 9 "unit/operators/basic.sp"
struct Vec2 operator_add__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 10 "unit/operators/basic.sp"
    struct Vec2 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

#line 12 "unit/operators/basic.sp"
struct Vec2 operator_sub__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 13 "unit/operators/basic.sp"
    struct Vec2 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

#line 15 "unit/operators/basic.sp"
int operator_eq__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 16 "unit/operators/basic.sp"
    return a.x == b.x && a.y == b.y;
}

#line 23 "unit/operators/basic.sp"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs) {

#line 24 "unit/operators/basic.sp"
    return this->x * rhs.x + this->y * rhs.y;
}

#line 31 "unit/operators/basic.sp"
IntBox IntBox__op_add(IntBox * this, IntBox other) {

#line 32 "unit/operators/basic.sp"
    IntBox r;
    r.value = this->value + other.value;
    return r;
}

#line 34 "unit/operators/basic.sp"
IntBox IntBox__op_sub(IntBox * this, IntBox other) {

#line 35 "unit/operators/basic.sp"
    IntBox r;
    r.value = this->value - other.value;
    return r;
}

#line 37 "unit/operators/basic.sp"
IntBox IntBox__op_mul(IntBox * this, IntBox other) {

#line 38 "unit/operators/basic.sp"
    IntBox r;
    r.value = this->value * other.value;
    return r;
}

#line 40 "unit/operators/basic.sp"
IntBox IntBox__op_div(IntBox * this, IntBox other) {

#line 41 "unit/operators/basic.sp"
    IntBox r;
    r.value = this->value / other.value;
    return r;
}

#line 43 "unit/operators/basic.sp"
IntBox IntBox__op_mod(IntBox * this, IntBox other) {

#line 44 "unit/operators/basic.sp"
    IntBox r;
    r.value = this->value % other.value;
    return r;
}

#line 46 "unit/operators/basic.sp"
_Bool IntBox__op_eq(IntBox * this, IntBox other) {

#line 47 "unit/operators/basic.sp"
    return this->value == other.value;
}

#line 49 "unit/operators/basic.sp"
_Bool IntBox__op_ne(IntBox * this, IntBox other) {

#line 50 "unit/operators/basic.sp"
    return this->value != other.value;
}

#line 52 "unit/operators/basic.sp"
_Bool IntBox__op_lt(IntBox * this, IntBox other) {

#line 53 "unit/operators/basic.sp"
    return this->value < other.value;
}

#line 55 "unit/operators/basic.sp"
_Bool IntBox__op_gt(IntBox * this, IntBox other) {

#line 56 "unit/operators/basic.sp"
    return this->value > other.value;
}

#line 58 "unit/operators/basic.sp"
_Bool IntBox__op_le(IntBox * this, IntBox other) {

#line 59 "unit/operators/basic.sp"
    return this->value <= other.value;
}

#line 61 "unit/operators/basic.sp"
_Bool IntBox__op_ge(IntBox * this, IntBox other) {

#line 62 "unit/operators/basic.sp"
    return this->value >= other.value;
}

#line 69 "unit/operators/basic.sp"
struct Point operator_add__Point__Point(struct Point a, struct Point b) {

#line 70 "unit/operators/basic.sp"
    struct Point r;

#line 71 "unit/operators/basic.sp"
    r.x = a.x + b.x;

#line 72 "unit/operators/basic.sp"
    r.y = a.y + b.y;

#line 73 "unit/operators/basic.sp"
    return r;
}

#line 76 "unit/operators/basic.sp"
int main() {

#line 78 "unit/operators/basic.sp"
    struct Vec2 a;
    a.x = 1.0f;
    a.y = 2.0f;

#line 79 "unit/operators/basic.sp"
    struct Vec2 b;
    b.x = 3.0f;
    b.y = 4.0f;

#line 81 "unit/operators/basic.sp"
    struct Vec2 s = operator_add__Vec2__Vec2(a, b);

#line 82 "unit/operators/basic.sp"
    if (s.x != 4.0f) 
        return 1;

#line 83 "unit/operators/basic.sp"
    if (s.y != 6.0f) 
        return 2;

#line 85 "unit/operators/basic.sp"
    struct Vec2 d = operator_sub__Vec2__Vec2(b, a);

#line 86 "unit/operators/basic.sp"
    if (d.x != 2.0f) 
        return 3;

#line 87 "unit/operators/basic.sp"
    if (d.y != 2.0f) 
        return 4;

#line 89 "unit/operators/basic.sp"
    struct Vec2 c;
    c.x = 1.0f;
    c.y = 2.0f;

#line 90 "unit/operators/basic.sp"
    if (!(operator_eq__Vec2__Vec2(a, c))) 
        return 5;

#line 91 "unit/operators/basic.sp"
    if (operator_eq__Vec2__Vec2(a, b)) 
        return 6;

#line 94 "unit/operators/basic.sp"
    Vec2m u;
    u.x = 3.0f;
    u.y = 4.0f;

#line 95 "unit/operators/basic.sp"
    Vec2m v;
    v.x = 1.0f;
    v.y = 0.0f;

#line 96 "unit/operators/basic.sp"
    float dot = Vec2m__op_mul(&u, v);

#line 97 "unit/operators/basic.sp"
    if (dot != 3.0f) 
        return 7;

#line 100 "unit/operators/basic.sp"
    IntBox ia;
    ia.value = 10;

#line 101 "unit/operators/basic.sp"
    IntBox ib;
    ib.value = 3;

#line 103 "unit/operators/basic.sp"
    IntBox iadd = IntBox__op_add(&ia, ib);

#line 104 "unit/operators/basic.sp"
    if (iadd.value != 13) 
        return 8;

#line 105 "unit/operators/basic.sp"
    IntBox isub = IntBox__op_sub(&ia, ib);

#line 106 "unit/operators/basic.sp"
    if (isub.value != 7) 
        return 9;

#line 107 "unit/operators/basic.sp"
    IntBox imul = IntBox__op_mul(&ia, ib);

#line 108 "unit/operators/basic.sp"
    if (imul.value != 30) 
        return 10;

#line 109 "unit/operators/basic.sp"
    IntBox idiv = IntBox__op_div(&ia, ib);

#line 110 "unit/operators/basic.sp"
    if (idiv.value != 3) 
        return 11;

#line 111 "unit/operators/basic.sp"
    IntBox imod = IntBox__op_mod(&ia, ib);

#line 112 "unit/operators/basic.sp"
    if (imod.value != 1) 
        return 12;

#line 115 "unit/operators/basic.sp"
    IntBox ic;
    ic.value = 10;

#line 116 "unit/operators/basic.sp"
    if (!(IntBox__op_eq(&ia, ic))) 
        return 13;

#line 117 "unit/operators/basic.sp"
    if (!(IntBox__op_eq(&ia, ia))) 
        return 14;

#line 118 "unit/operators/basic.sp"
    if (IntBox__op_eq(&ia, ib)) 
        return 15;

#line 119 "unit/operators/basic.sp"
    if (!(IntBox__op_ne(&ia, ib))) 
        return 16;

#line 120 "unit/operators/basic.sp"
    if (IntBox__op_ne(&ia, ic)) 
        return 17;

#line 121 "unit/operators/basic.sp"
    if (!(IntBox__op_lt(&ib, ia))) 
        return 18;

#line 122 "unit/operators/basic.sp"
    if (IntBox__op_lt(&ia, ib)) 
        return 19;

#line 123 "unit/operators/basic.sp"
    if (!(IntBox__op_gt(&ia, ib))) 
        return 20;

#line 124 "unit/operators/basic.sp"
    if (IntBox__op_gt(&ib, ia)) 
        return 21;

#line 125 "unit/operators/basic.sp"
    if (!(IntBox__op_le(&ib, ia))) 
        return 22;

#line 126 "unit/operators/basic.sp"
    if (!(IntBox__op_le(&ia, ic))) 
        return 23;

#line 127 "unit/operators/basic.sp"
    if (!(IntBox__op_ge(&ia, ib))) 
        return 24;

#line 128 "unit/operators/basic.sp"
    if (!(IntBox__op_ge(&ia, ic))) 
        return 25;

#line 131 "unit/operators/basic.sp"
    struct Point pa;
    pa.x = 1;
    pa.y = 2;

#line 132 "unit/operators/basic.sp"
    struct Point pb;
    pb.x = 3;
    pb.y = 4;

#line 133 "unit/operators/basic.sp"
    struct Point pr = operator_add__Point__Point(pa, pb);

#line 134 "unit/operators/basic.sp"
    if (pr.x != 4) 
        return 26;

#line 135 "unit/operators/basic.sp"
    if (pr.y != 6) 
        return 27;

#line 137 "unit/operators/basic.sp"
    return 0;
}
