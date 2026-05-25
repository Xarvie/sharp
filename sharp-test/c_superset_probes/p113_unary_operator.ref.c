

#line 2 "sharp-test/c_superset_probes/p113_unary_operator.sp"
typedef struct Vec2 Vec2;
struct Vec2 {
    float x;
    float y;
};

#line 5 "sharp-test/c_superset_probes/p113_unary_operator.sp"
Vec2 Vec2__op_sub(Vec2 * this);

#line 9 "sharp-test/c_superset_probes/p113_unary_operator.sp"
struct Scalar {
    int v;
};

#line 5 "sharp-test/c_superset_probes/p113_unary_operator.sp"
Vec2 Vec2__op_sub(Vec2 * this) {

#line 6 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    Vec2 r;
    r.x = -this->x;
    r.y = -this->y;
    return r;
}

#line 10 "sharp-test/c_superset_probes/p113_unary_operator.sp"
struct Scalar operator_sub__Scalar(struct Scalar a) {

#line 11 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    struct Scalar r;
    r.v = -a.v;
    return r;
}

#line 13 "sharp-test/c_superset_probes/p113_unary_operator.sp"
int main(void) {

#line 14 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    Vec2 a;
    a.x = 3.0f;
    a.y = -4.0f;

#line 15 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    Vec2 b = Vec2__op_sub(&a);

#line 16 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    if (b.x != -3.0f) 
        return 1;

#line 17 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    if (b.y != 4.0f) 
        return 2;

#line 18 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    struct Scalar s;
    s.v = 7;

#line 19 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    struct Scalar ns = operator_sub__Scalar(s);

#line 20 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    if (ns.v != -7) 
        return 3;

#line 21 "sharp-test/c_superset_probes/p113_unary_operator.sp"
    return 0;
}
