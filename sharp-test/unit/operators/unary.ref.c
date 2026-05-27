#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/operators/unary.sp"
typedef struct Vec2 Vec2;
struct Vec2 {
    float x;
    float y;
};

#line 10 "unit/operators/unary.sp"
Vec2 Vec2__op_sub(Vec2 * this);

#line 14 "unit/operators/unary.sp"
struct Scalar {
    int v;
};

#line 20 "unit/operators/unary.sp"
typedef struct Val Val;
struct Val {
    int x;
};

#line 23 "unit/operators/unary.sp"
Val Val__op_add(Val * this, Val other);

#line 26 "unit/operators/unary.sp"
Val Val__op_sub(Val * this);

#line 29 "unit/operators/unary.sp"
_Bool Val__op_eq(Val * this, Val other);

#line 10 "unit/operators/unary.sp"
Vec2 Vec2__op_sub(Vec2 * this) {

#line 11 "unit/operators/unary.sp"
    Vec2 r;
    r.x = -this->x;
    r.y = -this->y;
    return r;
}

#line 15 "unit/operators/unary.sp"
struct Scalar operator_sub__Scalar(struct Scalar a) {

#line 16 "unit/operators/unary.sp"
    struct Scalar r;
    r.v = -a.v;
    return r;
}

#line 23 "unit/operators/unary.sp"
Val Val__op_add(Val * this, Val other) {

#line 24 "unit/operators/unary.sp"
    Val r;
    r.x = this->x + other.x;
    return r;
}

#line 26 "unit/operators/unary.sp"
Val Val__op_sub(Val * this) {

#line 27 "unit/operators/unary.sp"
    Val r;
    r.x = -this->x;
    return r;
}

#line 29 "unit/operators/unary.sp"
_Bool Val__op_eq(Val * this, Val other) {

#line 30 "unit/operators/unary.sp"
    return this->x == other.x;
}

#line 33 "unit/operators/unary.sp"
int main() {

#line 35 "unit/operators/unary.sp"
    Vec2 a;
    a.x = 3.0f;
    a.y = -4.0f;

#line 36 "unit/operators/unary.sp"
    Vec2 b = Vec2__op_sub(&a);

#line 37 "unit/operators/unary.sp"
    if (b.x != -3.0f) 
        return 1;

#line 38 "unit/operators/unary.sp"
    if (b.y != 4.0f) 
        return 2;

#line 39 "unit/operators/unary.sp"
    struct Scalar s;
    s.v = 7;

#line 40 "unit/operators/unary.sp"
    struct Scalar ns = operator_sub__Scalar(s);

#line 41 "unit/operators/unary.sp"
    if (ns.v != -7) 
        return 3;

#line 44 "unit/operators/unary.sp"
    Val u;
    u.x = 10;

#line 45 "unit/operators/unary.sp"
    Val v;
    v.x = 3;

#line 47 "unit/operators/unary.sp"
    Val r1 = Val__op_add((__extension__(({ Val __sharp_bop0 = (Val__op_sub(&u)); &__sharp_bop0; }))), v);

#line 48 "unit/operators/unary.sp"
    if (r1.x != -7) 
        return 4;

#line 50 "unit/operators/unary.sp"
    Val r2 = Val__op_add(&u, Val__op_sub(&v));

#line 51 "unit/operators/unary.sp"
    if (r2.x != 7) 
        return 5;

#line 53 "unit/operators/unary.sp"
    Val r3 = Val__op_add((__extension__(({ Val __sharp_bop1 = (Val__op_sub(&u)); &__sharp_bop1; }))), Val__op_sub(&v));

#line 54 "unit/operators/unary.sp"
    if (r3.x != -13) 
        return 6;

#line 56 "unit/operators/unary.sp"
    return 0;
}
