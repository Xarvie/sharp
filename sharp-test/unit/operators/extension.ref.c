
#include <stdlib.h>

#line 5 "unit/operators/extension.sp"
typedef struct Pt Pt;
struct Pt {
    int x;
    int y;
};

#line 21 "unit/operators/extension.sp"
Pt Pt__op_add(Pt * this, Pt other);

#line 39 "unit/operators/extension.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 43 "unit/operators/extension.sp"
Point Point__op_add(Point * this, Point rhs);

#line 49 "unit/operators/extension.sp"
Point Point__op_sub(Point * this, Point rhs);

#line 11 "unit/operators/extension.sp"
int test_before() {

#line 12 "unit/operators/extension.sp"
    Pt a;
    a.x = 1;
    a.y = 2;

#line 13 "unit/operators/extension.sp"
    Pt b;
    b.x = 10;
    b.y = 20;

#line 14 "unit/operators/extension.sp"
    Pt c = Pt__op_add(&a, b);

#line 15 "unit/operators/extension.sp"
    if (c.x != 11) 
        return 1;

#line 16 "unit/operators/extension.sp"
    if (c.y != 22) 
        return 2;

#line 17 "unit/operators/extension.sp"
    return 0;
}

#line 21 "unit/operators/extension.sp"
Pt Pt__op_add(Pt * this, Pt other) {

#line 22 "unit/operators/extension.sp"
    Pt r;

#line 23 "unit/operators/extension.sp"
    r.x = this->x + other.x;

#line 24 "unit/operators/extension.sp"
    r.y = this->y + other.y;

#line 25 "unit/operators/extension.sp"
    return r;
}

#line 29 "unit/operators/extension.sp"
int test_after() {

#line 30 "unit/operators/extension.sp"
    Pt a;
    a.x = 5;
    a.y = 6;

#line 31 "unit/operators/extension.sp"
    Pt b;
    b.x = 30;
    b.y = 40;

#line 32 "unit/operators/extension.sp"
    Pt c = Pt__op_add(&a, b);

#line 33 "unit/operators/extension.sp"
    if (c.x != 35) 
        return 11;

#line 34 "unit/operators/extension.sp"
    if (c.y != 46) 
        return 12;

#line 35 "unit/operators/extension.sp"
    return 0;
}

#line 43 "unit/operators/extension.sp"
Point Point__op_add(Point * this, Point rhs) {

#line 44 "unit/operators/extension.sp"
    Point r;

#line 45 "unit/operators/extension.sp"
    r.x = this->x + rhs.x;

#line 46 "unit/operators/extension.sp"
    r.y = this->y + rhs.y;

#line 47 "unit/operators/extension.sp"
    return r;
}

#line 49 "unit/operators/extension.sp"
Point Point__op_sub(Point * this, Point rhs) {

#line 50 "unit/operators/extension.sp"
    Point r;

#line 51 "unit/operators/extension.sp"
    r.x = this->x - rhs.x;

#line 52 "unit/operators/extension.sp"
    r.y = this->y - rhs.y;

#line 53 "unit/operators/extension.sp"
    return r;
}

#line 56 "unit/operators/extension.sp"
int main() {

#line 58 "unit/operators/extension.sp"
    int r1 = test_before();

#line 59 "unit/operators/extension.sp"
    if (r1 != 0) 
        return r1;

#line 60 "unit/operators/extension.sp"
    int r2 = test_after();

#line 61 "unit/operators/extension.sp"
    if (r2 != 0) 
        return r2;

#line 64 "unit/operators/extension.sp"
    Point p1;
    p1.x = 3;
    p1.y = 5;

#line 65 "unit/operators/extension.sp"
    Point p2;
    p2.x = 7;
    p2.y = 2;

#line 67 "unit/operators/extension.sp"
    Point r_add = Point__op_add(&p1, p2);

#line 68 "unit/operators/extension.sp"
    if (r_add.x != 10) 
        return 21;

#line 69 "unit/operators/extension.sp"
    if (r_add.y != 7) 
        return 22;

#line 71 "unit/operators/extension.sp"
    Point r_sub = Point__op_sub(&p1, p2);

#line 72 "unit/operators/extension.sp"
    if (r_sub.x != -4) 
        return 23;

#line 73 "unit/operators/extension.sp"
    if (r_sub.y != 3) 
        return 24;

#line 76 "unit/operators/extension.sp"
    Point p3;
    p3.x = 1;
    p3.y = 1;

#line 77 "unit/operators/extension.sp"
    Point r_chain = Point__op_add((__extension__(({ Point __sharp_bop0 = (Point__op_add(&p1, p2)); &__sharp_bop0; }))), p3);

#line 78 "unit/operators/extension.sp"
    if (r_chain.x != 11) 
        return 25;

#line 79 "unit/operators/extension.sp"
    if (r_chain.y != 8) 
        return 26;

#line 81 "unit/operators/extension.sp"
    return 0;
}
