

#line 9 "sharp-test/c_superset_probes/p225_operator_extension.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 25 "sharp-test/c_superset_probes/p225_operator_extension.sp"
Point Point__op_add(Point * this, Point other);

#line 15 "sharp-test/c_superset_probes/p225_operator_extension.sp"
int test_before() {

#line 16 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point a;
    a.x = 1;
    a.y = 2;

#line 17 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point b;
    b.x = 10;
    b.y = 20;

#line 18 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point c = Point__op_add(&a, b);

#line 19 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (c.x != 11) 
        return 1;

#line 20 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (c.y != 22) 
        return 2;

#line 21 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    return 0;
}

#line 25 "sharp-test/c_superset_probes/p225_operator_extension.sp"
Point Point__op_add(Point * this, Point other) {

#line 26 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point r;

#line 27 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    r.x = this->x + other.x;

#line 28 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    r.y = this->y + other.y;

#line 29 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    return r;
}

#line 33 "sharp-test/c_superset_probes/p225_operator_extension.sp"
int test_after() {

#line 34 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point a;
    a.x = 5;
    a.y = 6;

#line 35 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point b;
    b.x = 30;
    b.y = 40;

#line 36 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    Point c = Point__op_add(&a, b);

#line 37 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (c.x != 35) 
        return 11;

#line 38 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (c.y != 46) 
        return 12;

#line 39 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    return 0;
}

#line 42 "sharp-test/c_superset_probes/p225_operator_extension.sp"
int main() {

#line 43 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    int r1 = test_before();

#line 44 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (r1 != 0) 
        return r1;

#line 46 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    int r2 = test_after();

#line 47 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    if (r2 != 0) 
        return r2;

#line 49 "sharp-test/c_superset_probes/p225_operator_extension.sp"
    return 0;
}
