
#line 7 "sharp-test/unit/cinterop/cross_type.ce"
typedef struct VecC VecC;
struct VecC {
    float x;
    float y;
};

#line 10 "sharp-test/unit/cinterop/cross_type.ce"
struct VecS {
    float x;
    float y;
};

#line 13 "sharp-test/unit/cinterop/cross_type.ce"
VecC operator_add__VecC__VecS(VecC a, struct VecS b) {

#line 14 "sharp-test/unit/cinterop/cross_type.ce"
    VecC r;

#line 15 "sharp-test/unit/cinterop/cross_type.ce"
    r.x = a.x + b.x;

#line 16 "sharp-test/unit/cinterop/cross_type.ce"
    r.y = a.y + b.y;

#line 17 "sharp-test/unit/cinterop/cross_type.ce"
    return r;
}

#line 20 "sharp-test/unit/cinterop/cross_type.ce"
VecC operator_add__VecS__VecC(struct VecS a, VecC b) {

#line 21 "sharp-test/unit/cinterop/cross_type.ce"
    VecC r;

#line 22 "sharp-test/unit/cinterop/cross_type.ce"
    r.x = a.x + b.x;

#line 23 "sharp-test/unit/cinterop/cross_type.ce"
    r.y = a.y + b.y;

#line 24 "sharp-test/unit/cinterop/cross_type.ce"
    return r;
}

#line 27 "sharp-test/unit/cinterop/cross_type.ce"
int operator_eq__VecC__VecS(VecC a, struct VecS b) {

#line 28 "sharp-test/unit/cinterop/cross_type.ce"
    return a.x == b.x && a.y == b.y;
}

#line 31 "sharp-test/unit/cinterop/cross_type.ce"
int main(void) {

#line 32 "sharp-test/unit/cinterop/cross_type.ce"
    VecC c1;
    c1.x = 1.0f;
    c1.y = 2.0f;

#line 33 "sharp-test/unit/cinterop/cross_type.ce"
    struct VecS s1;
    s1.x = 10.0f;
    s1.y = 20.0f;

#line 34 "sharp-test/unit/cinterop/cross_type.ce"
    struct VecS s2;
    s2.x = 30.0f;
    s2.y = 40.0f;

#line 37 "sharp-test/unit/cinterop/cross_type.ce"
    VecC cs = operator_add__VecC__VecS(c1, s1);

#line 38 "sharp-test/unit/cinterop/cross_type.ce"
    if (cs.x != 11.0f) 
        return 1;

#line 39 "sharp-test/unit/cinterop/cross_type.ce"
    if (cs.y != 22.0f) 
        return 2;

#line 42 "sharp-test/unit/cinterop/cross_type.ce"
    VecC sc = operator_add__VecS__VecC(s1, c1);

#line 43 "sharp-test/unit/cinterop/cross_type.ce"
    if (sc.x != 11.0f) 
        return 3;

#line 44 "sharp-test/unit/cinterop/cross_type.ce"
    if (sc.y != 22.0f) 
        return 4;

#line 47 "sharp-test/unit/cinterop/cross_type.ce"
    VecC eq_test;
    eq_test.x = 10.0f;
    eq_test.y = 20.0f;

#line 48 "sharp-test/unit/cinterop/cross_type.ce"
    if (!(operator_eq__VecC__VecS(eq_test, s1))) 
        return 5;

#line 49 "sharp-test/unit/cinterop/cross_type.ce"
    if (operator_eq__VecC__VecS(eq_test, s2)) 
        return 6;

#line 52 "sharp-test/unit/cinterop/cross_type.ce"
    VecC c2;
    c2.x = 0.0f;
    c2.y = 0.0f;

#line 53 "sharp-test/unit/cinterop/cross_type.ce"
    VecC result = operator_add__VecC__VecS(operator_add__VecC__VecS(c2, s1), s2);

#line 54 "sharp-test/unit/cinterop/cross_type.ce"
    if (result.x != 40.0f) 
        return 7;

#line 55 "sharp-test/unit/cinterop/cross_type.ce"
    if (result.y != 60.0f) 
        return 8;

#line 58 "sharp-test/unit/cinterop/cross_type.ce"
    struct VecS s3;
    s3.x = 11.0f;
    s3.y = 22.0f;

#line 59 "sharp-test/unit/cinterop/cross_type.ce"
    VecC c3;
    c3.x = c1.x + s1.x;
    c3.y = c1.y + s1.y;

#line 60 "sharp-test/unit/cinterop/cross_type.ce"
    if (!(operator_eq__VecC__VecS(c3, s3))) 
        return 9;

#line 62 "sharp-test/unit/cinterop/cross_type.ce"
    return 0;
}
