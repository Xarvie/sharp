
#line 7 "sharp-test/unit/operators/basic.ce"
struct Vec2 {
    float x;
    float y;
};

#line 20 "sharp-test/unit/operators/basic.ce"
typedef struct Vec2m Vec2m;
struct Vec2m {
    float x;
    float y;
};

#line 23 "sharp-test/unit/operators/basic.ce"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs);

#line 28 "sharp-test/unit/operators/basic.ce"
typedef struct IntBox IntBox;
struct IntBox {
    int value;
};

#line 31 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_add(IntBox * this, IntBox other);

#line 34 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_sub(IntBox * this, IntBox other);

#line 37 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_mul(IntBox * this, IntBox other);

#line 40 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_div(IntBox * this, IntBox other);

#line 43 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_mod(IntBox * this, IntBox other);

#line 46 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_eq(IntBox * this, IntBox other);

#line 49 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_ne(IntBox * this, IntBox other);

#line 52 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_lt(IntBox * this, IntBox other);

#line 55 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_gt(IntBox * this, IntBox other);

#line 58 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_le(IntBox * this, IntBox other);

#line 61 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_ge(IntBox * this, IntBox other);

#line 66 "sharp-test/unit/operators/basic.ce"
struct Point {
    int x;
    int y;
};

#line 76 "sharp-test/unit/operators/basic.ce"
typedef struct Accum Accum;
struct Accum {
    int total;
};

#line 79 "sharp-test/unit/operators/basic.ce"
Accum Accum__op_add(Accum * this, int n);

#line 9 "sharp-test/unit/operators/basic.ce"
struct Vec2 operator_add__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 10 "sharp-test/unit/operators/basic.ce"
    struct Vec2 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

#line 12 "sharp-test/unit/operators/basic.ce"
struct Vec2 operator_sub__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 13 "sharp-test/unit/operators/basic.ce"
    struct Vec2 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

#line 15 "sharp-test/unit/operators/basic.ce"
int operator_eq__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 16 "sharp-test/unit/operators/basic.ce"
    return a.x == b.x && a.y == b.y;
}

#line 23 "sharp-test/unit/operators/basic.ce"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs) {

#line 24 "sharp-test/unit/operators/basic.ce"
    return this->x * rhs.x + this->y * rhs.y;
}

#line 31 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_add(IntBox * this, IntBox other) {

#line 32 "sharp-test/unit/operators/basic.ce"
    IntBox r;
    r.value = this->value + other.value;
    return r;
}

#line 34 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_sub(IntBox * this, IntBox other) {

#line 35 "sharp-test/unit/operators/basic.ce"
    IntBox r;
    r.value = this->value - other.value;
    return r;
}

#line 37 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_mul(IntBox * this, IntBox other) {

#line 38 "sharp-test/unit/operators/basic.ce"
    IntBox r;
    r.value = this->value * other.value;
    return r;
}

#line 40 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_div(IntBox * this, IntBox other) {

#line 41 "sharp-test/unit/operators/basic.ce"
    IntBox r;
    r.value = this->value / other.value;
    return r;
}

#line 43 "sharp-test/unit/operators/basic.ce"
IntBox IntBox__op_mod(IntBox * this, IntBox other) {

#line 44 "sharp-test/unit/operators/basic.ce"
    IntBox r;
    r.value = this->value % other.value;
    return r;
}

#line 46 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_eq(IntBox * this, IntBox other) {

#line 47 "sharp-test/unit/operators/basic.ce"
    return this->value == other.value;
}

#line 49 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_ne(IntBox * this, IntBox other) {

#line 50 "sharp-test/unit/operators/basic.ce"
    return this->value != other.value;
}

#line 52 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_lt(IntBox * this, IntBox other) {

#line 53 "sharp-test/unit/operators/basic.ce"
    return this->value < other.value;
}

#line 55 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_gt(IntBox * this, IntBox other) {

#line 56 "sharp-test/unit/operators/basic.ce"
    return this->value > other.value;
}

#line 58 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_le(IntBox * this, IntBox other) {

#line 59 "sharp-test/unit/operators/basic.ce"
    return this->value <= other.value;
}

#line 61 "sharp-test/unit/operators/basic.ce"
bool IntBox__op_ge(IntBox * this, IntBox other) {

#line 62 "sharp-test/unit/operators/basic.ce"
    return this->value >= other.value;
}

#line 69 "sharp-test/unit/operators/basic.ce"
struct Point operator_add__Point__Point(struct Point a, struct Point b) {

#line 70 "sharp-test/unit/operators/basic.ce"
    struct Point r;

#line 71 "sharp-test/unit/operators/basic.ce"
    r.x = a.x + b.x;

#line 72 "sharp-test/unit/operators/basic.ce"
    r.y = a.y + b.y;

#line 73 "sharp-test/unit/operators/basic.ce"
    return r;
}

#line 79 "sharp-test/unit/operators/basic.ce"
Accum Accum__op_add(Accum * this, int n) {

#line 80 "sharp-test/unit/operators/basic.ce"
    Accum r;
    r.total = this->total + n;
    return r;
}

#line 83 "sharp-test/unit/operators/basic.ce"
int main() {

#line 85 "sharp-test/unit/operators/basic.ce"
    struct Vec2 a;
    a.x = 1.0f;
    a.y = 2.0f;

#line 86 "sharp-test/unit/operators/basic.ce"
    struct Vec2 b;
    b.x = 3.0f;
    b.y = 4.0f;

#line 88 "sharp-test/unit/operators/basic.ce"
    struct Vec2 s = operator_add__Vec2__Vec2(a, b);

#line 89 "sharp-test/unit/operators/basic.ce"
    if (s.x != 4.0f) 
        return 1;

#line 90 "sharp-test/unit/operators/basic.ce"
    if (s.y != 6.0f) 
        return 2;

#line 92 "sharp-test/unit/operators/basic.ce"
    struct Vec2 d = operator_sub__Vec2__Vec2(b, a);

#line 93 "sharp-test/unit/operators/basic.ce"
    if (d.x != 2.0f) 
        return 3;

#line 94 "sharp-test/unit/operators/basic.ce"
    if (d.y != 2.0f) 
        return 4;

#line 96 "sharp-test/unit/operators/basic.ce"
    struct Vec2 c;
    c.x = 1.0f;
    c.y = 2.0f;

#line 97 "sharp-test/unit/operators/basic.ce"
    if (!(operator_eq__Vec2__Vec2(a, c))) 
        return 5;

#line 98 "sharp-test/unit/operators/basic.ce"
    if (operator_eq__Vec2__Vec2(a, b)) 
        return 6;

#line 101 "sharp-test/unit/operators/basic.ce"
    Vec2m u;
    u.x = 3.0f;
    u.y = 4.0f;

#line 102 "sharp-test/unit/operators/basic.ce"
    Vec2m v;
    v.x = 1.0f;
    v.y = 0.0f;

#line 103 "sharp-test/unit/operators/basic.ce"
    float dot = Vec2m__op_mul(&u, v);

#line 104 "sharp-test/unit/operators/basic.ce"
    if (dot != 3.0f) 
        return 7;

#line 107 "sharp-test/unit/operators/basic.ce"
    IntBox ia;
    ia.value = 10;

#line 108 "sharp-test/unit/operators/basic.ce"
    IntBox ib;
    ib.value = 3;

#line 110 "sharp-test/unit/operators/basic.ce"
    IntBox iadd = IntBox__op_add(&ia, ib);

#line 111 "sharp-test/unit/operators/basic.ce"
    if (iadd.value != 13) 
        return 8;

#line 112 "sharp-test/unit/operators/basic.ce"
    IntBox isub = IntBox__op_sub(&ia, ib);

#line 113 "sharp-test/unit/operators/basic.ce"
    if (isub.value != 7) 
        return 9;

#line 114 "sharp-test/unit/operators/basic.ce"
    IntBox imul = IntBox__op_mul(&ia, ib);

#line 115 "sharp-test/unit/operators/basic.ce"
    if (imul.value != 30) 
        return 10;

#line 116 "sharp-test/unit/operators/basic.ce"
    IntBox idiv = IntBox__op_div(&ia, ib);

#line 117 "sharp-test/unit/operators/basic.ce"
    if (idiv.value != 3) 
        return 11;

#line 118 "sharp-test/unit/operators/basic.ce"
    IntBox imod = IntBox__op_mod(&ia, ib);

#line 119 "sharp-test/unit/operators/basic.ce"
    if (imod.value != 1) 
        return 12;

#line 122 "sharp-test/unit/operators/basic.ce"
    IntBox ic;
    ic.value = 10;

#line 123 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_eq(&ia, ic))) 
        return 13;

#line 124 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_eq(&ia, ia))) 
        return 14;

#line 125 "sharp-test/unit/operators/basic.ce"
    if (IntBox__op_eq(&ia, ib)) 
        return 15;

#line 126 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_ne(&ia, ib))) 
        return 16;

#line 127 "sharp-test/unit/operators/basic.ce"
    if (IntBox__op_ne(&ia, ic)) 
        return 17;

#line 128 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_lt(&ib, ia))) 
        return 18;

#line 129 "sharp-test/unit/operators/basic.ce"
    if (IntBox__op_lt(&ia, ib)) 
        return 19;

#line 130 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_gt(&ia, ib))) 
        return 20;

#line 131 "sharp-test/unit/operators/basic.ce"
    if (IntBox__op_gt(&ib, ia)) 
        return 21;

#line 132 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_le(&ib, ia))) 
        return 22;

#line 133 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_le(&ia, ic))) 
        return 23;

#line 134 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_ge(&ia, ib))) 
        return 24;

#line 135 "sharp-test/unit/operators/basic.ce"
    if (!(IntBox__op_ge(&ia, ic))) 
        return 25;

#line 138 "sharp-test/unit/operators/basic.ce"
    struct Point pa;
    pa.x = 1;
    pa.y = 2;

#line 139 "sharp-test/unit/operators/basic.ce"
    struct Point pb;
    pb.x = 3;
    pb.y = 4;

#line 140 "sharp-test/unit/operators/basic.ce"
    struct Point pr = operator_add__Point__Point(pa, pb);

#line 141 "sharp-test/unit/operators/basic.ce"
    if (pr.x != 4) 
        return 26;

#line 142 "sharp-test/unit/operators/basic.ce"
    if (pr.y != 6) 
        return 27;

#line 145 "sharp-test/unit/operators/basic.ce"
    Accum ac;
    ac.total = 10;

#line 146 "sharp-test/unit/operators/basic.ce"
    Accum ac2 = Accum__op_add(&ac, 5);

#line 147 "sharp-test/unit/operators/basic.ce"
    if (ac2.total != 15) 
        return 28;

#line 148 "sharp-test/unit/operators/basic.ce"
    if (ac.total != 10) 
        return 29;

#line 151 "sharp-test/unit/operators/basic.ce"
    IntBox x1;
    x1.value = 2;

#line 152 "sharp-test/unit/operators/basic.ce"
    IntBox x2;
    x2.value = 3;

#line 153 "sharp-test/unit/operators/basic.ce"
    IntBox x3;
    x3.value = 4;

#line 154 "sharp-test/unit/operators/basic.ce"
    IntBox xsum = IntBox__op_add((__extension__(({ IntBox __sharp_rv0 = (IntBox__op_add(&x1, x2)); &__sharp_rv0; }))), x3);

#line 155 "sharp-test/unit/operators/basic.ce"
    if (xsum.value != 9) 
        return 30;

#line 156 "sharp-test/unit/operators/basic.ce"
    IntBox xprod = IntBox__op_mul((__extension__(({ IntBox __sharp_rv1 = (IntBox__op_mul(&x1, x2)); &__sharp_rv1; }))), x3);

#line 157 "sharp-test/unit/operators/basic.ce"
    if (xprod.value != 24) 
        return 31;

#line 159 "sharp-test/unit/operators/basic.ce"
    return 0;
}
