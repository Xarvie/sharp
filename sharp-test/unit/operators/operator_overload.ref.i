
#line 5 "sharp-test/unit/operators/operator_overload.ce"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 10 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_add(Vec2 * this, Vec2 other);

#line 17 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_sub(Vec2 * this, Vec2 other);

#line 24 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_mul(Vec2 * this, int s);

#line 31 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2__op_eq(Vec2 * this, Vec2 other);

#line 35 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2__op_ne(Vec2 * this, Vec2 other);

#line 39 "sharp-test/unit/operators/operator_overload.ce"
typedef struct Vec2f Vec2f;
struct Vec2f {
    float x;
    float y;
};

#line 44 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_add(Vec2f * this, Vec2f other);

#line 51 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_sub(Vec2f * this, Vec2f other);

#line 58 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_mul(Vec2f * this, float s);

#line 65 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2f__op_eq(Vec2f * this, Vec2f other);

#line 69 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2f__op_ne(Vec2f * this, Vec2f other);

#line 10 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_add(Vec2 * this, Vec2 other) {

#line 11 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 r;

#line 12 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x + other.x;

#line 13 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y + other.y;

#line 14 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 17 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_sub(Vec2 * this, Vec2 other) {

#line 18 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 r;

#line 19 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x - other.x;

#line 20 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y - other.y;

#line 21 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 24 "sharp-test/unit/operators/operator_overload.ce"
Vec2 Vec2__op_mul(Vec2 * this, int s) {

#line 25 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 r;

#line 26 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x * s;

#line 27 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y * s;

#line 28 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 31 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2__op_eq(Vec2 * this, Vec2 other) {

#line 32 "sharp-test/unit/operators/operator_overload.ce"
    return this->x == other.x && this->y == other.y;
}

#line 35 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2__op_ne(Vec2 * this, Vec2 other) {

#line 36 "sharp-test/unit/operators/operator_overload.ce"
    return this->x != other.x || this->y != other.y;
}

#line 44 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_add(Vec2f * this, Vec2f other) {

#line 45 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f r;

#line 46 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x + other.x;

#line 47 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y + other.y;

#line 48 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 51 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_sub(Vec2f * this, Vec2f other) {

#line 52 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f r;

#line 53 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x - other.x;

#line 54 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y - other.y;

#line 55 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 58 "sharp-test/unit/operators/operator_overload.ce"
Vec2f Vec2f__op_mul(Vec2f * this, float s) {

#line 59 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f r;

#line 60 "sharp-test/unit/operators/operator_overload.ce"
    r.x = this->x * s;

#line 61 "sharp-test/unit/operators/operator_overload.ce"
    r.y = this->y * s;

#line 62 "sharp-test/unit/operators/operator_overload.ce"
    return r;
}

#line 65 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2f__op_eq(Vec2f * this, Vec2f other) {

#line 66 "sharp-test/unit/operators/operator_overload.ce"
    return this->x == other.x && this->y == other.y;
}

#line 69 "sharp-test/unit/operators/operator_overload.ce"
_Bool Vec2f__op_ne(Vec2f * this, Vec2f other) {

#line 70 "sharp-test/unit/operators/operator_overload.ce"
    return this->x != other.x || this->y != other.y;
}

#line 73 "sharp-test/unit/operators/operator_overload.ce"
int main(void) {

#line 75 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 a;
    a.x = 1;
    a.y = 2;

#line 76 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 b;
    b.x = 3;
    b.y = 4;

#line 77 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 s = Vec2__op_add(&a, b);

#line 78 "sharp-test/unit/operators/operator_overload.ce"
    if (s.x != 4) 
        return 1;

#line 79 "sharp-test/unit/operators/operator_overload.ce"
    if (s.y != 6) 
        return 2;

#line 82 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 d = Vec2__op_sub(&b, a);

#line 83 "sharp-test/unit/operators/operator_overload.ce"
    if (d.x != 2) 
        return 3;

#line 84 "sharp-test/unit/operators/operator_overload.ce"
    if (d.y != 2) 
        return 4;

#line 87 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 m = Vec2__op_mul(&a, 3);

#line 88 "sharp-test/unit/operators/operator_overload.ce"
    if (m.x != 3) 
        return 5;

#line 89 "sharp-test/unit/operators/operator_overload.ce"
    if (m.y != 6) 
        return 6;

#line 92 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 c;
    c.x = 1;
    c.y = 2;

#line 93 "sharp-test/unit/operators/operator_overload.ce"
    if (!(Vec2__op_eq(&a, c))) 
        return 7;

#line 94 "sharp-test/unit/operators/operator_overload.ce"
    if (Vec2__op_eq(&a, b)) 
        return 8;

#line 97 "sharp-test/unit/operators/operator_overload.ce"
    if (Vec2__op_ne(&a, c)) 
        return 9;

#line 98 "sharp-test/unit/operators/operator_overload.ce"
    if (!(Vec2__op_ne(&a, b))) 
        return 10;

#line 101 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fa;
    fa.x = 2.0f;
    fa.y = 3.0f;

#line 102 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fm = Vec2f__op_mul(&fa, 2.5f);

#line 103 "sharp-test/unit/operators/operator_overload.ce"
    if (fm.x != 5.0f) 
        return 13;

#line 104 "sharp-test/unit/operators/operator_overload.ce"
    if (fm.y != 7.5f) 
        return 14;

#line 107 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fb;
    fb.x = 1.0f;
    fb.y = 1.0f;

#line 108 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fs = Vec2f__op_add(&fa, fb);

#line 109 "sharp-test/unit/operators/operator_overload.ce"
    if (fs.x != 3.0f) 
        return 15;

#line 110 "sharp-test/unit/operators/operator_overload.ce"
    if (fs.y != 4.0f) 
        return 16;

#line 111 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fd = Vec2f__op_sub(&fa, fb);

#line 112 "sharp-test/unit/operators/operator_overload.ce"
    if (fd.x != 1.0f) 
        return 17;

#line 113 "sharp-test/unit/operators/operator_overload.ce"
    if (fd.y != 2.0f) 
        return 18;

#line 116 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fc;
    fc.x = 2.0f;
    fc.y = 3.0f;

#line 117 "sharp-test/unit/operators/operator_overload.ce"
    if (!(Vec2f__op_eq(&fa, fc))) 
        return 19;

#line 118 "sharp-test/unit/operators/operator_overload.ce"
    if (Vec2f__op_eq(&fa, fb)) 
        return 20;

#line 119 "sharp-test/unit/operators/operator_overload.ce"
    if (Vec2f__op_ne(&fa, fc)) 
        return 21;

#line 120 "sharp-test/unit/operators/operator_overload.ce"
    if (!(Vec2f__op_ne(&fa, fb))) 
        return 22;

#line 123 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 v1;
    v1.x = 1;
    v1.y = 0;

#line 124 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 v2;
    v2.x = 0;
    v2.y = 1;

#line 125 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 v3;
    v3.x = 1;
    v3.y = 1;

#line 126 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 chain = Vec2__op_add((__extension__(({ Vec2 __sharp_rv0 = (Vec2__op_add(&v1, v2)); &__sharp_rv0; }))), v3);

#line 127 "sharp-test/unit/operators/operator_overload.ce"
    if (chain.x != 2) 
        return 25;

#line 128 "sharp-test/unit/operators/operator_overload.ce"
    if (chain.y != 2) 
        return 26;

#line 131 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 sa;
    sa.x = 2;
    sa.y = 3;

#line 132 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 sb;
    sb.x = 1;
    sb.y = 1;

#line 133 "sharp-test/unit/operators/operator_overload.ce"
    Vec2 result = Vec2__op_add((__extension__(({ Vec2 __sharp_rv1 = (Vec2__op_mul(&sa, 2)); &__sharp_rv1; }))), sb);

#line 134 "sharp-test/unit/operators/operator_overload.ce"
    if (result.x != 5) 
        return 29;

#line 135 "sharp-test/unit/operators/operator_overload.ce"
    if (result.y != 7) 
        return 30;

#line 138 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f f1;
    f1.x = 1.0f;
    f1.y = 0.0f;

#line 139 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f f2;
    f2.x = 0.0f;
    f2.y = 1.0f;

#line 140 "sharp-test/unit/operators/operator_overload.ce"
    Vec2f fchain = Vec2f__op_add((__extension__(({ Vec2f __sharp_rv2 = (Vec2f__op_mul(&f1, 3.0f)); &__sharp_rv2; }))), Vec2f__op_mul(&f2, 2.0f));

#line 141 "sharp-test/unit/operators/operator_overload.ce"
    if (fchain.x != 3.0f) 
        return 33;

#line 142 "sharp-test/unit/operators/operator_overload.ce"
    if (fchain.y != 2.0f) 
        return 34;

#line 144 "sharp-test/unit/operators/operator_overload.ce"
    return 0;
}
