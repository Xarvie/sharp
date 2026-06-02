
#line 7 "sharp-test/unit/operators/chain.ce"
typedef struct Counter Counter;
struct Counter {
    int val;
};

#line 10 "sharp-test/unit/operators/chain.ce"
Counter * Counter__inc(Counter * this);

#line 14 "sharp-test/unit/operators/chain.ce"
Counter * Counter__add(Counter * this, int n);

#line 18 "sharp-test/unit/operators/chain.ce"
int Counter__get(Counter * this);

#line 21 "sharp-test/unit/operators/chain.ce"
typedef struct Vec3 Vec3;
struct Vec3 {
    int x;
    int y;
    int z;
};

#line 24 "sharp-test/unit/operators/chain.ce"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other);

#line 31 "sharp-test/unit/operators/chain.ce"
bool Vec3__op_eq(Vec3 * this, Vec3 other);

#line 10 "sharp-test/unit/operators/chain.ce"
Counter * Counter__inc(Counter * this) {

#line 11 "sharp-test/unit/operators/chain.ce"
    this->val = this->val + 1;

#line 12 "sharp-test/unit/operators/chain.ce"
    return this;
}

#line 14 "sharp-test/unit/operators/chain.ce"
Counter * Counter__add(Counter * this, int n) {

#line 15 "sharp-test/unit/operators/chain.ce"
    this->val = this->val + n;

#line 16 "sharp-test/unit/operators/chain.ce"
    return this;
}

#line 18 "sharp-test/unit/operators/chain.ce"
int Counter__get(Counter * this) {
    return this->val;
}

#line 24 "sharp-test/unit/operators/chain.ce"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other) {

#line 25 "sharp-test/unit/operators/chain.ce"
    Vec3 r;

#line 26 "sharp-test/unit/operators/chain.ce"
    r.x = this->x + other.x;

#line 27 "sharp-test/unit/operators/chain.ce"
    r.y = this->y + other.y;

#line 28 "sharp-test/unit/operators/chain.ce"
    r.z = this->z + other.z;

#line 29 "sharp-test/unit/operators/chain.ce"
    return r;
}

#line 31 "sharp-test/unit/operators/chain.ce"
bool Vec3__op_eq(Vec3 * this, Vec3 other) {

#line 32 "sharp-test/unit/operators/chain.ce"
    return this->x == other.x && this->y == other.y && this->z == other.z;
}

#line 37 "sharp-test/unit/operators/chain.ce"
int main() {

#line 39 "sharp-test/unit/operators/chain.ce"
    Counter c = { 0 };

#line 40 "sharp-test/unit/operators/chain.ce"
    Counter__add(Counter__inc(Counter__inc(&c)), 5);

#line 41 "sharp-test/unit/operators/chain.ce"
    if (Counter__get(&c) != 7) 
        return 1;

#line 42 "sharp-test/unit/operators/chain.ce"
    Counter__inc(Counter__inc(Counter__inc(Counter__add(&c, 10))));

#line 43 "sharp-test/unit/operators/chain.ce"
    if (Counter__get(&c) != 20) 
        return 2;

#line 46 "sharp-test/unit/operators/chain.ce"
    Vec3 a;
    a.x = 1;
    a.y = 2;
    a.z = 3;

#line 47 "sharp-test/unit/operators/chain.ce"
    Vec3 b;
    b.x = 10;
    b.y = 20;
    b.z = 30;

#line 48 "sharp-test/unit/operators/chain.ce"
    Vec3 cz;
    cz.x = 100;
    cz.y = 200;
    cz.z = 300;

#line 50 "sharp-test/unit/operators/chain.ce"
    Vec3 r = Vec3__op_add((__extension__(({ Vec3 __sharp_rv0 = (Vec3__op_add(&a, b)); &__sharp_rv0; }))), cz);

#line 51 "sharp-test/unit/operators/chain.ce"
    if (r.x != 111) 
        return 3;

#line 52 "sharp-test/unit/operators/chain.ce"
    if (r.y != 222) 
        return 4;

#line 53 "sharp-test/unit/operators/chain.ce"
    if (r.z != 333) 
        return 5;

#line 56 "sharp-test/unit/operators/chain.ce"
    Vec3 left = Vec3__op_add(&a, b);

#line 57 "sharp-test/unit/operators/chain.ce"
    Vec3 right = Vec3__op_add(&b, a);

#line 58 "sharp-test/unit/operators/chain.ce"
    if (!(Vec3__op_eq(&left, right))) 
        return 6;

#line 61 "sharp-test/unit/operators/chain.ce"
    Counter c2 = { 0 };

#line 62 "sharp-test/unit/operators/chain.ce"
    int val = Counter__get(Counter__add(Counter__inc(&c2), 5));

#line 63 "sharp-test/unit/operators/chain.ce"
    if (val != 6) 
        return 7;

#line 66 "sharp-test/unit/operators/chain.ce"
    Vec3 d;
    d.x = 1;
    d.y = 0;
    d.z = 0;

#line 67 "sharp-test/unit/operators/chain.ce"
    Vec3 e;
    e.x = 0;
    e.y = 1;
    e.z = 0;

#line 68 "sharp-test/unit/operators/chain.ce"
    Vec3 f;
    f.x = 0;
    f.y = 0;
    f.z = 1;

#line 69 "sharp-test/unit/operators/chain.ce"
    Vec3 sum = Vec3__op_add((__extension__(({ Vec3 __sharp_rv1 = (Vec3__op_add((__extension__(({ Vec3 __sharp_rv2 = (Vec3__op_add((__extension__(({ Vec3 __sharp_rv3 = (Vec3__op_add(&a, b)); &__sharp_rv3; }))), d)); &__sharp_rv2; }))), e)); &__sharp_rv1; }))), f);

#line 70 "sharp-test/unit/operators/chain.ce"
    if (sum.x != 12) 
        return 8;

#line 71 "sharp-test/unit/operators/chain.ce"
    if (sum.y != 23) 
        return 9;

#line 72 "sharp-test/unit/operators/chain.ce"
    if (sum.z != 34) 
        return 10;

#line 74 "sharp-test/unit/operators/chain.ce"
    return 0;
}
