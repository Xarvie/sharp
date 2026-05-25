

#line 7 "sharp-test/c_superset_probes/p110_free_operator.sp"
struct Vec2 {
    float x;
    float y;
};

#line 20 "sharp-test/c_superset_probes/p110_free_operator.sp"
typedef struct Vec2m Vec2m;
struct Vec2m {
    float x;
    float y;
};

#line 23 "sharp-test/c_superset_probes/p110_free_operator.sp"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs);

#line 9 "sharp-test/c_superset_probes/p110_free_operator.sp"
struct Vec2 operator_add__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 10 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

#line 12 "sharp-test/c_superset_probes/p110_free_operator.sp"
struct Vec2 operator_sub__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 13 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 r;
    r.x = a.x - b.x;
    r.y = a.y - b.y;
    return r;
}

#line 15 "sharp-test/c_superset_probes/p110_free_operator.sp"
int operator_eq__Vec2__Vec2(struct Vec2 a, struct Vec2 b) {

#line 16 "sharp-test/c_superset_probes/p110_free_operator.sp"
    return a.x == b.x && a.y == b.y;
}

#line 23 "sharp-test/c_superset_probes/p110_free_operator.sp"
float Vec2m__op_mul(Vec2m * this, Vec2m rhs) {

#line 24 "sharp-test/c_superset_probes/p110_free_operator.sp"
    return this->x * rhs.x + this->y * rhs.y;
}

#line 27 "sharp-test/c_superset_probes/p110_free_operator.sp"
int main(void) {

#line 28 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 a;
    a.x = 1.0f;
    a.y = 2.0f;

#line 29 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 b;
    b.x = 3.0f;
    b.y = 4.0f;

#line 31 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 s = operator_add__Vec2__Vec2(a, b);

#line 32 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (s.x != 4.0f) 
        return 1;

#line 33 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (s.y != 6.0f) 
        return 2;

#line 35 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 d = operator_sub__Vec2__Vec2(b, a);

#line 36 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (d.x != 2.0f) 
        return 3;

#line 37 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (d.y != 2.0f) 
        return 4;

#line 39 "sharp-test/c_superset_probes/p110_free_operator.sp"
    struct Vec2 c;
    c.x = 1.0f;
    c.y = 2.0f;

#line 40 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (!(operator_eq__Vec2__Vec2(a, c))) 
        return 5;

#line 41 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (operator_eq__Vec2__Vec2(a, b)) 
        return 6;

#line 43 "sharp-test/c_superset_probes/p110_free_operator.sp"
    Vec2m u;
    u.x = 3.0f;
    u.y = 4.0f;

#line 44 "sharp-test/c_superset_probes/p110_free_operator.sp"
    Vec2m v;
    v.x = 1.0f;
    v.y = 0.0f;

#line 45 "sharp-test/c_superset_probes/p110_free_operator.sp"
    float dot = Vec2m__op_mul(&u, v);

#line 46 "sharp-test/c_superset_probes/p110_free_operator.sp"
    if (dot != 3.0f) 
        return 7;

#line 48 "sharp-test/c_superset_probes/p110_free_operator.sp"
    return 0;
}
