

#line 6 "c_superset_probes/p267_runtime_operator.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 11 "c_superset_probes/p267_runtime_operator.sp"
Point Point__op_add(Point * this, Point rhs);

#line 18 "c_superset_probes/p267_runtime_operator.sp"
Point Point__op_sub(Point * this, Point rhs);

#line 11 "c_superset_probes/p267_runtime_operator.sp"
Point Point__op_add(Point * this, Point rhs) {

#line 12 "c_superset_probes/p267_runtime_operator.sp"
    Point r;

#line 13 "c_superset_probes/p267_runtime_operator.sp"
    r.x = this->x + rhs.x;

#line 14 "c_superset_probes/p267_runtime_operator.sp"
    r.y = this->y + rhs.y;

#line 15 "c_superset_probes/p267_runtime_operator.sp"
    return r;
}

#line 18 "c_superset_probes/p267_runtime_operator.sp"
Point Point__op_sub(Point * this, Point rhs) {

#line 19 "c_superset_probes/p267_runtime_operator.sp"
    Point r;

#line 20 "c_superset_probes/p267_runtime_operator.sp"
    r.x = this->x - rhs.x;

#line 21 "c_superset_probes/p267_runtime_operator.sp"
    r.y = this->y - rhs.y;

#line 22 "c_superset_probes/p267_runtime_operator.sp"
    return r;
}

#line 25 "c_superset_probes/p267_runtime_operator.sp"
int main() {

#line 26 "c_superset_probes/p267_runtime_operator.sp"
    Point p1;
    p1.x = 3;
    p1.y = 5;

#line 27 "c_superset_probes/p267_runtime_operator.sp"
    Point p2;
    p2.x = 7;
    p2.y = 2;

#line 29 "c_superset_probes/p267_runtime_operator.sp"
    Point r1 = Point__op_add(&p1, p2);

#line 30 "c_superset_probes/p267_runtime_operator.sp"
    if (r1.x != 10) 
        return 1;

#line 31 "c_superset_probes/p267_runtime_operator.sp"
    if (r1.y != 7) 
        return 2;

#line 33 "c_superset_probes/p267_runtime_operator.sp"
    Point r2 = Point__op_sub(&p1, p2);

#line 34 "c_superset_probes/p267_runtime_operator.sp"
    if (r2.x != -4) 
        return 3;

#line 35 "c_superset_probes/p267_runtime_operator.sp"
    if (r2.y != 3) 
        return 4;

#line 38 "c_superset_probes/p267_runtime_operator.sp"
    Point p3;
    p3.x = 1;
    p3.y = 1;

#line 39 "c_superset_probes/p267_runtime_operator.sp"
    Point r3 = Point__op_add((__extension__(({ Point __sharp_bop0 = (Point__op_add(&p1, p2)); &__sharp_bop0; }))), p3);

#line 40 "c_superset_probes/p267_runtime_operator.sp"
    if (r3.x != 11) 
        return 5;

#line 41 "c_superset_probes/p267_runtime_operator.sp"
    if (r3.y != 8) 
        return 6;

#line 43 "c_superset_probes/p267_runtime_operator.sp"
    return 0;
}
