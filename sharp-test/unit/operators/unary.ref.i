
#line 7 "sharp-test/unit/operators/unary.ce"
typedef struct Vec2 Vec2;
struct Vec2 {
    float x;
    float y;
};

#line 10 "sharp-test/unit/operators/unary.ce"
Vec2 Vec2__op_sub(Vec2 * this);

#line 14 "sharp-test/unit/operators/unary.ce"
struct Scalar {
    int v;
};

#line 20 "sharp-test/unit/operators/unary.ce"
typedef struct Val Val;
struct Val {
    int x;
};

#line 23 "sharp-test/unit/operators/unary.ce"
Val Val__op_add(Val * this, Val other);

#line 26 "sharp-test/unit/operators/unary.ce"
Val Val__op_sub(Val * this);

#line 29 "sharp-test/unit/operators/unary.ce"
bool Val__op_eq(Val * this, Val other);

#line 10 "sharp-test/unit/operators/unary.ce"
Vec2 Vec2__op_sub(Vec2 * this) {

#line 11 "sharp-test/unit/operators/unary.ce"
    Vec2 r;
    r.x = -this->x;
    r.y = -this->y;
    return r;
}

#line 15 "sharp-test/unit/operators/unary.ce"
struct Scalar operator_sub__Scalar(struct Scalar a) {

#line 16 "sharp-test/unit/operators/unary.ce"
    struct Scalar r;
    r.v = -a.v;
    return r;
}

#line 23 "sharp-test/unit/operators/unary.ce"
Val Val__op_add(Val * this, Val other) {

#line 24 "sharp-test/unit/operators/unary.ce"
    Val r;
    r.x = this->x + other.x;
    return r;
}

#line 26 "sharp-test/unit/operators/unary.ce"
Val Val__op_sub(Val * this) {

#line 27 "sharp-test/unit/operators/unary.ce"
    Val r;
    r.x = -this->x;
    return r;
}

#line 29 "sharp-test/unit/operators/unary.ce"
bool Val__op_eq(Val * this, Val other) {

#line 30 "sharp-test/unit/operators/unary.ce"
    return this->x == other.x;
}

#line 33 "sharp-test/unit/operators/unary.ce"
int main() {

#line 35 "sharp-test/unit/operators/unary.ce"
    Vec2 a;
    a.x = 3.0f;
    a.y = -4.0f;

#line 36 "sharp-test/unit/operators/unary.ce"
    Vec2 b = Vec2__op_sub(&a);

#line 37 "sharp-test/unit/operators/unary.ce"
    if (b.x != -3.0f) 
        return 1;

#line 38 "sharp-test/unit/operators/unary.ce"
    if (b.y != 4.0f) 
        return 2;

#line 39 "sharp-test/unit/operators/unary.ce"
    struct Scalar s;
    s.v = 7;

#line 40 "sharp-test/unit/operators/unary.ce"
    struct Scalar ns = operator_sub__Scalar(s);

#line 41 "sharp-test/unit/operators/unary.ce"
    if (ns.v != -7) 
        return 3;

#line 44 "sharp-test/unit/operators/unary.ce"
    Val u;
    u.x = 10;

#line 45 "sharp-test/unit/operators/unary.ce"
    Val v;
    v.x = 3;

#line 47 "sharp-test/unit/operators/unary.ce"
    Val r1 = Val__op_add((__extension__(({ Val __sharp_rv0 = (Val__op_sub(&u)); &__sharp_rv0; }))), v);

#line 48 "sharp-test/unit/operators/unary.ce"
    if (r1.x != -7) 
        return 4;

#line 50 "sharp-test/unit/operators/unary.ce"
    Val r2 = Val__op_add(&u, Val__op_sub(&v));

#line 51 "sharp-test/unit/operators/unary.ce"
    if (r2.x != 7) 
        return 5;

#line 53 "sharp-test/unit/operators/unary.ce"
    Val r3 = Val__op_add((__extension__(({ Val __sharp_rv1 = (Val__op_sub(&u)); &__sharp_rv1; }))), Val__op_sub(&v));

#line 54 "sharp-test/unit/operators/unary.ce"
    if (r3.x != -13) 
        return 6;

#line 57 "sharp-test/unit/operators/unary.ce"
    if (!(Val__op_eq((__extension__(({ Val __sharp_rv2 = (Val__op_sub(&u)); &__sharp_rv2; }))), Val__op_sub(&u)))) 
        return 7;

#line 58 "sharp-test/unit/operators/unary.ce"
    if (Val__op_eq((__extension__(({ Val __sharp_rv3 = (Val__op_sub(&u)); &__sharp_rv3; }))), u)) 
        return 8;

#line 61 "sharp-test/unit/operators/unary.ce"
    Val neg_u = Val__op_sub(&u);

#line 62 "sharp-test/unit/operators/unary.ce"
    Val dbl_neg = Val__op_sub(&neg_u);

#line 63 "sharp-test/unit/operators/unary.ce"
    if (dbl_neg.x != 10) 
        return 9;

#line 65 "sharp-test/unit/operators/unary.ce"
    return 0;
}
