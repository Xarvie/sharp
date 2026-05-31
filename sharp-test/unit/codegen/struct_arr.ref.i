
#line 7 "sharp-test/unit/codegen/struct_arr.ce"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 13 "sharp-test/unit/codegen/struct_arr.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 21 "sharp-test/unit/codegen/struct_arr.ce"
Point Point__add(Point * this, Point rhs);

#line 17 "sharp-test/unit/codegen/struct_arr.ce"
Point make_point(int xx, int yy) {

#line 18 "sharp-test/unit/codegen/struct_arr.ce"
    Point p;
    p.x = xx;
    p.y = yy;

#line 19 "sharp-test/unit/codegen/struct_arr.ce"
    return p;
}

#line 21 "sharp-test/unit/codegen/struct_arr.ce"
Point Point__add(Point * this, Point rhs) {

#line 22 "sharp-test/unit/codegen/struct_arr.ce"
    Point r;

#line 23 "sharp-test/unit/codegen/struct_arr.ce"
    r.x = this->x + rhs.x;

#line 24 "sharp-test/unit/codegen/struct_arr.ce"
    r.y = this->y + rhs.y;

#line 25 "sharp-test/unit/codegen/struct_arr.ce"
    return r;
}

#line 28 "sharp-test/unit/codegen/struct_arr.ce"
int main() {

#line 30 "sharp-test/unit/codegen/struct_arr.ce"
    Vec2 points[4] = { 0 };

#line 31 "sharp-test/unit/codegen/struct_arr.ce"
    points[0].x = 1;
    points[0].y = 2;

#line 32 "sharp-test/unit/codegen/struct_arr.ce"
    points[1].x = 3;
    points[1].y = 4;

#line 33 "sharp-test/unit/codegen/struct_arr.ce"
    points[2].x = 5;
    points[2].y = 6;

#line 34 "sharp-test/unit/codegen/struct_arr.ce"
    points[3].x = 7;
    points[3].y = 8;
    int sum_x = 0, sum_y = 0;

#line 37 "sharp-test/unit/codegen/struct_arr.ce"
    for (int i = 0; i < 4; i = i + 1) {

#line 38 "sharp-test/unit/codegen/struct_arr.ce"
        sum_x = sum_x + points[i].x;

#line 39 "sharp-test/unit/codegen/struct_arr.ce"
        sum_y = sum_y + points[i].y;
    }

#line 41 "sharp-test/unit/codegen/struct_arr.ce"
    if (sum_x != 16) 
        return 1;

#line 42 "sharp-test/unit/codegen/struct_arr.ce"
    if (sum_y != 20) 
        return 2;

#line 44 "sharp-test/unit/codegen/struct_arr.ce"
    Vec2 partial[3] = { 0 };

#line 45 "sharp-test/unit/codegen/struct_arr.ce"
    partial[1].x = 99;

#line 46 "sharp-test/unit/codegen/struct_arr.ce"
    if (partial[0].x != 0) 
        return 3;

#line 47 "sharp-test/unit/codegen/struct_arr.ce"
    if (partial[1].x != 99) 
        return 4;

#line 48 "sharp-test/unit/codegen/struct_arr.ce"
    if (partial[2].x != 0) 
        return 5;

#line 51 "sharp-test/unit/codegen/struct_arr.ce"
    Point p1 = make_point(10, 20);

#line 52 "sharp-test/unit/codegen/struct_arr.ce"
    if (p1.x != 10) 
        return 6;

#line 53 "sharp-test/unit/codegen/struct_arr.ce"
    if (p1.y != 20) 
        return 7;

#line 55 "sharp-test/unit/codegen/struct_arr.ce"
    Point p2 = make_point(5, 7);

#line 56 "sharp-test/unit/codegen/struct_arr.ce"
    Point p3 = Point__add(&p1, p2);

#line 57 "sharp-test/unit/codegen/struct_arr.ce"
    if (p3.x != 15) 
        return 8;

#line 58 "sharp-test/unit/codegen/struct_arr.ce"
    if (p3.y != 27) 
        return 9;

#line 60 "sharp-test/unit/codegen/struct_arr.ce"
    Point p4 = Point__add((__extension__(({ Point __sharp_rv0 = (make_point(1, 2)); &__sharp_rv0; }))), make_point(3, 4));

#line 61 "sharp-test/unit/codegen/struct_arr.ce"
    if (p4.x != 4) 
        return 10;

#line 62 "sharp-test/unit/codegen/struct_arr.ce"
    if (p4.y != 6) 
        return 11;

#line 64 "sharp-test/unit/codegen/struct_arr.ce"
    return 0;
}
