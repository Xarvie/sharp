
#line 7 "sharp-test/unit/operators/extension.ce"
typedef struct Pt Pt;
struct Pt {
    int x;
    int y;
};

#line 23 "sharp-test/unit/operators/extension.ce"
Pt Pt__op_add(Pt * this, Pt other);

#line 41 "sharp-test/unit/operators/extension.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 45 "sharp-test/unit/operators/extension.ce"
Point Point__op_add(Point * this, Point rhs);

#line 51 "sharp-test/unit/operators/extension.ce"
Point Point__op_sub(Point * this, Point rhs);

#line 58 "sharp-test/unit/operators/extension.ce"
bool Pt__op_eq(Pt * this, Pt other);

#line 13 "sharp-test/unit/operators/extension.ce"
int test_before() {

#line 14 "sharp-test/unit/operators/extension.ce"
    Pt a;
    a.x = 1;
    a.y = 2;

#line 15 "sharp-test/unit/operators/extension.ce"
    Pt b;
    b.x = 10;
    b.y = 20;

#line 16 "sharp-test/unit/operators/extension.ce"
    Pt c = Pt__op_add(&a, b);

#line 17 "sharp-test/unit/operators/extension.ce"
    if (c.x != 11) 
        return 1;

#line 18 "sharp-test/unit/operators/extension.ce"
    if (c.y != 22) 
        return 2;

#line 19 "sharp-test/unit/operators/extension.ce"
    return 0;
}

#line 23 "sharp-test/unit/operators/extension.ce"
Pt Pt__op_add(Pt * this, Pt other) {

#line 24 "sharp-test/unit/operators/extension.ce"
    Pt r;

#line 25 "sharp-test/unit/operators/extension.ce"
    r.x = this->x + other.x;

#line 26 "sharp-test/unit/operators/extension.ce"
    r.y = this->y + other.y;

#line 27 "sharp-test/unit/operators/extension.ce"
    return r;
}

#line 31 "sharp-test/unit/operators/extension.ce"
int test_after() {

#line 32 "sharp-test/unit/operators/extension.ce"
    Pt a;
    a.x = 5;
    a.y = 6;

#line 33 "sharp-test/unit/operators/extension.ce"
    Pt b;
    b.x = 30;
    b.y = 40;

#line 34 "sharp-test/unit/operators/extension.ce"
    Pt c = Pt__op_add(&a, b);

#line 35 "sharp-test/unit/operators/extension.ce"
    if (c.x != 35) 
        return 11;

#line 36 "sharp-test/unit/operators/extension.ce"
    if (c.y != 46) 
        return 12;

#line 37 "sharp-test/unit/operators/extension.ce"
    return 0;
}

#line 45 "sharp-test/unit/operators/extension.ce"
Point Point__op_add(Point * this, Point rhs) {

#line 46 "sharp-test/unit/operators/extension.ce"
    Point r;

#line 47 "sharp-test/unit/operators/extension.ce"
    r.x = this->x + rhs.x;

#line 48 "sharp-test/unit/operators/extension.ce"
    r.y = this->y + rhs.y;

#line 49 "sharp-test/unit/operators/extension.ce"
    return r;
}

#line 51 "sharp-test/unit/operators/extension.ce"
Point Point__op_sub(Point * this, Point rhs) {

#line 52 "sharp-test/unit/operators/extension.ce"
    Point r;

#line 53 "sharp-test/unit/operators/extension.ce"
    r.x = this->x - rhs.x;

#line 54 "sharp-test/unit/operators/extension.ce"
    r.y = this->y - rhs.y;

#line 55 "sharp-test/unit/operators/extension.ce"
    return r;
}

#line 58 "sharp-test/unit/operators/extension.ce"
bool Pt__op_eq(Pt * this, Pt other) {

#line 59 "sharp-test/unit/operators/extension.ce"
    return this->x == other.x && this->y == other.y;
}

#line 62 "sharp-test/unit/operators/extension.ce"
int main() {

#line 64 "sharp-test/unit/operators/extension.ce"
    int r1 = test_before();

#line 65 "sharp-test/unit/operators/extension.ce"
    if (r1 != 0) 
        return r1;

#line 66 "sharp-test/unit/operators/extension.ce"
    int r2 = test_after();

#line 67 "sharp-test/unit/operators/extension.ce"
    if (r2 != 0) 
        return r2;

#line 70 "sharp-test/unit/operators/extension.ce"
    Point p1;
    p1.x = 3;
    p1.y = 5;

#line 71 "sharp-test/unit/operators/extension.ce"
    Point p2;
    p2.x = 7;
    p2.y = 2;

#line 73 "sharp-test/unit/operators/extension.ce"
    Point r_add = Point__op_add(&p1, p2);

#line 74 "sharp-test/unit/operators/extension.ce"
    if (r_add.x != 10) 
        return 21;

#line 75 "sharp-test/unit/operators/extension.ce"
    if (r_add.y != 7) 
        return 22;

#line 77 "sharp-test/unit/operators/extension.ce"
    Point r_sub = Point__op_sub(&p1, p2);

#line 78 "sharp-test/unit/operators/extension.ce"
    if (r_sub.x != -4) 
        return 23;

#line 79 "sharp-test/unit/operators/extension.ce"
    if (r_sub.y != 3) 
        return 24;

#line 82 "sharp-test/unit/operators/extension.ce"
    Point p3;
    p3.x = 1;
    p3.y = 1;

#line 83 "sharp-test/unit/operators/extension.ce"
    Point r_chain = Point__op_add((__extension__(({ Point __sharp_rv0 = (Point__op_add(&p1, p2)); &__sharp_rv0; }))), p3);

#line 84 "sharp-test/unit/operators/extension.ce"
    if (r_chain.x != 11) 
        return 25;

#line 85 "sharp-test/unit/operators/extension.ce"
    if (r_chain.y != 8) 
        return 26;

#line 88 "sharp-test/unit/operators/extension.ce"
    Pt pa;
    pa.x = 3;
    pa.y = 5;

#line 89 "sharp-test/unit/operators/extension.ce"
    Pt pb;
    pb.x = 3;
    pb.y = 5;

#line 90 "sharp-test/unit/operators/extension.ce"
    if (!(Pt__op_eq(&pa, pb))) 
        return 27;

#line 93 "sharp-test/unit/operators/extension.ce"
    Pt pc;
    pc.x = 1;
    pc.y = 2;

#line 94 "sharp-test/unit/operators/extension.ce"
    if (Pt__op_eq(&pa, pc)) 
        return 28;

#line 95 "sharp-test/unit/operators/extension.ce"
    if (!(Pt__op_eq(&pa, pb))) 
        return 29;

#line 97 "sharp-test/unit/operators/extension.ce"
    return 0;
}
