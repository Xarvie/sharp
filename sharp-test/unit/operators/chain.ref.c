#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/operators/chain.sp"
typedef struct Counter Counter;
struct Counter {
    int val;
};

#line 10 "unit/operators/chain.sp"
Counter * Counter__inc(Counter * this);

#line 14 "unit/operators/chain.sp"
Counter * Counter__add(Counter * this, int n);

#line 18 "unit/operators/chain.sp"
int Counter__get(Counter * this);

#line 21 "unit/operators/chain.sp"
typedef struct Vec3 Vec3;
struct Vec3 {
    int x;
    int y;
    int z;
};

#line 24 "unit/operators/chain.sp"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other);

#line 31 "unit/operators/chain.sp"
_Bool Vec3__op_eq(Vec3 * this, Vec3 other);

#line 10 "unit/operators/chain.sp"
Counter * Counter__inc(Counter * this) {

#line 11 "unit/operators/chain.sp"
    this->val = this->val + 1;

#line 12 "unit/operators/chain.sp"
    return this;
}

#line 14 "unit/operators/chain.sp"
Counter * Counter__add(Counter * this, int n) {

#line 15 "unit/operators/chain.sp"
    this->val = this->val + n;

#line 16 "unit/operators/chain.sp"
    return this;
}

#line 18 "unit/operators/chain.sp"
int Counter__get(Counter * this) {
    return this->val;
}

#line 24 "unit/operators/chain.sp"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other) {

#line 25 "unit/operators/chain.sp"
    Vec3 r;

#line 26 "unit/operators/chain.sp"
    r.x = this->x + other.x;

#line 27 "unit/operators/chain.sp"
    r.y = this->y + other.y;

#line 28 "unit/operators/chain.sp"
    r.z = this->z + other.z;

#line 29 "unit/operators/chain.sp"
    return r;
}

#line 31 "unit/operators/chain.sp"
_Bool Vec3__op_eq(Vec3 * this, Vec3 other) {

#line 32 "unit/operators/chain.sp"
    return this->x == other.x && this->y == other.y && this->z == other.z;
}

#line 37 "unit/operators/chain.sp"
int main() {

#line 39 "unit/operators/chain.sp"
    Counter c = { 0 };

#line 40 "unit/operators/chain.sp"
    Counter__add(Counter__inc(Counter__inc(&c)), 5);

#line 41 "unit/operators/chain.sp"
    if (Counter__get(&c) != 7) 
        return 1;

#line 42 "unit/operators/chain.sp"
    Counter__inc(Counter__inc(Counter__inc(Counter__add(&c, 10))));

#line 43 "unit/operators/chain.sp"
    if (Counter__get(&c) != 20) 
        return 2;

#line 46 "unit/operators/chain.sp"
    Vec3 a;
    a.x = 1;
    a.y = 2;
    a.z = 3;

#line 47 "unit/operators/chain.sp"
    Vec3 b;
    b.x = 10;
    b.y = 20;
    b.z = 30;

#line 48 "unit/operators/chain.sp"
    Vec3 cz;
    cz.x = 100;
    cz.y = 200;
    cz.z = 300;

#line 50 "unit/operators/chain.sp"
    Vec3 r = Vec3__op_add((__extension__(({ Vec3 __sharp_bop0 = (Vec3__op_add(&a, b)); &__sharp_bop0; }))), cz);

#line 51 "unit/operators/chain.sp"
    if (r.x != 111) 
        return 3;

#line 52 "unit/operators/chain.sp"
    if (r.y != 222) 
        return 4;

#line 53 "unit/operators/chain.sp"
    if (r.z != 333) 
        return 5;

#line 56 "unit/operators/chain.sp"
    Vec3 left = Vec3__op_add(&a, b);

#line 57 "unit/operators/chain.sp"
    Vec3 right = Vec3__op_add(&b, a);

#line 58 "unit/operators/chain.sp"
    if (!(Vec3__op_eq(&left, right))) 
        return 6;

#line 60 "unit/operators/chain.sp"
    return 0;
}
