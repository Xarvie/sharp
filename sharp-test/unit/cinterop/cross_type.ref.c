
#include <stdlib.h>

#line 7 "unit/cinterop/cross_type.sp"
typedef struct VecC VecC;
struct VecC {
    float x;
    float y;
};

#line 10 "unit/cinterop/cross_type.sp"
struct VecS {
    float x;
    float y;
};

#line 13 "unit/cinterop/cross_type.sp"
VecC operator_add__VecC__VecS(VecC a, struct VecS b) {

#line 14 "unit/cinterop/cross_type.sp"
    VecC r;

#line 15 "unit/cinterop/cross_type.sp"
    r.x = a.x + b.x;

#line 16 "unit/cinterop/cross_type.sp"
    r.y = a.y + b.y;

#line 17 "unit/cinterop/cross_type.sp"
    return r;
}

#line 20 "unit/cinterop/cross_type.sp"
VecC operator_add__VecS__VecC(struct VecS a, VecC b) {

#line 21 "unit/cinterop/cross_type.sp"
    VecC r;

#line 22 "unit/cinterop/cross_type.sp"
    r.x = a.x + b.x;

#line 23 "unit/cinterop/cross_type.sp"
    r.y = a.y + b.y;

#line 24 "unit/cinterop/cross_type.sp"
    return r;
}

#line 27 "unit/cinterop/cross_type.sp"
int operator_eq__VecC__VecS(VecC a, struct VecS b) {

#line 28 "unit/cinterop/cross_type.sp"
    return a.x == b.x && a.y == b.y;
}

#line 31 "unit/cinterop/cross_type.sp"
int main(void) {

#line 32 "unit/cinterop/cross_type.sp"
    VecC c1;
    c1.x = 1.0f;
    c1.y = 2.0f;

#line 33 "unit/cinterop/cross_type.sp"
    struct VecS s1;
    s1.x = 10.0f;
    s1.y = 20.0f;

#line 34 "unit/cinterop/cross_type.sp"
    struct VecS s2;
    s2.x = 30.0f;
    s2.y = 40.0f;

#line 37 "unit/cinterop/cross_type.sp"
    VecC cs = operator_add__VecC__VecS(c1, s1);

#line 38 "unit/cinterop/cross_type.sp"
    if (cs.x != 11.0f) 
        return 1;

#line 39 "unit/cinterop/cross_type.sp"
    if (cs.y != 22.0f) 
        return 2;

#line 42 "unit/cinterop/cross_type.sp"
    VecC sc = operator_add__VecS__VecC(s1, c1);

#line 43 "unit/cinterop/cross_type.sp"
    if (sc.x != 11.0f) 
        return 3;

#line 44 "unit/cinterop/cross_type.sp"
    if (sc.y != 22.0f) 
        return 4;

#line 47 "unit/cinterop/cross_type.sp"
    VecC eq_test;
    eq_test.x = 10.0f;
    eq_test.y = 20.0f;

#line 48 "unit/cinterop/cross_type.sp"
    if (!(operator_eq__VecC__VecS(eq_test, s1))) 
        return 5;

#line 49 "unit/cinterop/cross_type.sp"
    if (operator_eq__VecC__VecS(eq_test, s2)) 
        return 6;

#line 51 "unit/cinterop/cross_type.sp"
    return 0;
}
