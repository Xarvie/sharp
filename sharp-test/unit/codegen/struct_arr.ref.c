
#include <stdlib.h>

#line 7 "unit/codegen/struct_arr.sp"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 13 "unit/codegen/struct_arr.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 21 "unit/codegen/struct_arr.sp"
Point Point__add(Point * this, Point rhs);

#line 17 "unit/codegen/struct_arr.sp"
Point make_point(int xx, int yy) {

#line 18 "unit/codegen/struct_arr.sp"
    Point p;
    p.x = xx;
    p.y = yy;

#line 19 "unit/codegen/struct_arr.sp"
    return p;
}

#line 21 "unit/codegen/struct_arr.sp"
Point Point__add(Point * this, Point rhs) {

#line 22 "unit/codegen/struct_arr.sp"
    Point r;

#line 23 "unit/codegen/struct_arr.sp"
    r.x = this->x + rhs.x;

#line 24 "unit/codegen/struct_arr.sp"
    r.y = this->y + rhs.y;

#line 25 "unit/codegen/struct_arr.sp"
    return r;
}

#line 28 "unit/codegen/struct_arr.sp"
int main() {

#line 30 "unit/codegen/struct_arr.sp"
    Vec2 points[4] = { 0 };

#line 31 "unit/codegen/struct_arr.sp"
    points[0].x = 1;
    points[0].y = 2;

#line 32 "unit/codegen/struct_arr.sp"
    points[1].x = 3;
    points[1].y = 4;

#line 33 "unit/codegen/struct_arr.sp"
    points[2].x = 5;
    points[2].y = 6;

#line 34 "unit/codegen/struct_arr.sp"
    points[3].x = 7;
    points[3].y = 8;
    int sum_x = 0, sum_y = 0;

#line 37 "unit/codegen/struct_arr.sp"
    for (int i = 0; i < 4; i = i + 1) {

#line 38 "unit/codegen/struct_arr.sp"
        sum_x = sum_x + points[i].x;

#line 39 "unit/codegen/struct_arr.sp"
        sum_y = sum_y + points[i].y;
    }

#line 41 "unit/codegen/struct_arr.sp"
    if (sum_x != 16) 
        return 1;

#line 42 "unit/codegen/struct_arr.sp"
    if (sum_y != 20) 
        return 2;

#line 44 "unit/codegen/struct_arr.sp"
    Vec2 partial[3] = { 0 };

#line 45 "unit/codegen/struct_arr.sp"
    partial[1].x = 99;

#line 46 "unit/codegen/struct_arr.sp"
    if (partial[0].x != 0) 
        return 3;

#line 47 "unit/codegen/struct_arr.sp"
    if (partial[1].x != 99) 
        return 4;

#line 48 "unit/codegen/struct_arr.sp"
    if (partial[2].x != 0) 
        return 5;

#line 51 "unit/codegen/struct_arr.sp"
    Point p1 = make_point(10, 20);

#line 52 "unit/codegen/struct_arr.sp"
    if (p1.x != 10) 
        return 6;

#line 53 "unit/codegen/struct_arr.sp"
    if (p1.y != 20) 
        return 7;

#line 55 "unit/codegen/struct_arr.sp"
    Point p2 = make_point(5, 7);

#line 56 "unit/codegen/struct_arr.sp"
    Point p3 = Point__add(&p1, p2);

#line 57 "unit/codegen/struct_arr.sp"
    if (p3.x != 15) 
        return 8;

#line 58 "unit/codegen/struct_arr.sp"
    if (p3.y != 27) 
        return 9;

#line 60 "unit/codegen/struct_arr.sp"
    Point p4 = Point__add((__extension__(({ Point __sharp_chain0 = (make_point(1, 2)); &__sharp_chain0; }))), make_point(3, 4));

#line 61 "unit/codegen/struct_arr.sp"
    if (p4.x != 4) 
        return 10;

#line 62 "unit/codegen/struct_arr.sp"
    if (p4.y != 6) 
        return 11;

#line 64 "unit/codegen/struct_arr.sp"
    return 0;
}
