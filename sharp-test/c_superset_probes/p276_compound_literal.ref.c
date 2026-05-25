

#line 6 "c_superset_probes/p276_compound_literal.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 16 "c_superset_probes/p276_compound_literal.sp"
Point Point__add(Point * this, Point rhs);

#line 11 "c_superset_probes/p276_compound_literal.sp"
Point make_point(int xx, int yy) {

#line 12 "c_superset_probes/p276_compound_literal.sp"
    Point p;
    p.x = xx;
    p.y = yy;

#line 13 "c_superset_probes/p276_compound_literal.sp"
    return p;
}

#line 16 "c_superset_probes/p276_compound_literal.sp"
Point Point__add(Point * this, Point rhs) {

#line 17 "c_superset_probes/p276_compound_literal.sp"
    Point r;

#line 18 "c_superset_probes/p276_compound_literal.sp"
    r.x = this->x + rhs.x;

#line 19 "c_superset_probes/p276_compound_literal.sp"
    r.y = this->y + rhs.y;

#line 20 "c_superset_probes/p276_compound_literal.sp"
    return r;
}

#line 23 "c_superset_probes/p276_compound_literal.sp"
int main() {

#line 25 "c_superset_probes/p276_compound_literal.sp"
    Point p1 = make_point(10, 20);

#line 26 "c_superset_probes/p276_compound_literal.sp"
    if (p1.x != 10) 
        return 1;

#line 27 "c_superset_probes/p276_compound_literal.sp"
    if (p1.y != 20) 
        return 2;

#line 30 "c_superset_probes/p276_compound_literal.sp"
    Point p2 = make_point(5, 7);

#line 31 "c_superset_probes/p276_compound_literal.sp"
    Point p3 = Point__add(&p1, p2);

#line 32 "c_superset_probes/p276_compound_literal.sp"
    if (p3.x != 15) 
        return 3;

#line 33 "c_superset_probes/p276_compound_literal.sp"
    if (p3.y != 27) 
        return 4;

#line 36 "c_superset_probes/p276_compound_literal.sp"
    Point p4 = Point__add((__extension__(({ Point __sharp_chain0 = (make_point(1, 2)); &__sharp_chain0; }))), make_point(3, 4));

#line 37 "c_superset_probes/p276_compound_literal.sp"
    if (p4.x != 4) 
        return 5;

#line 38 "c_superset_probes/p276_compound_literal.sp"
    if (p4.y != 6) 
        return 6;

#line 40 "c_superset_probes/p276_compound_literal.sp"
    return 0;
}
