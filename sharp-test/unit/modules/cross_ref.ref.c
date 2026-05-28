
#line 4 "sharp-test/unit/modules/cross_ref.sph"
typedef struct Point2D Point2D;
struct Point2D {
    int x;
    int y;
};

#line 9 "sharp-test/unit/modules/cross_ref.sph"
void Point2D__move_to(Point2D * this, int nx, int ny);

#line 10 "sharp-test/unit/modules/cross_ref.sph"
int Point2D__get_x(Point2D * this);

#line 11 "sharp-test/unit/modules/cross_ref.sph"
int Point2D__get_y(Point2D * this);

#line 9 "sharp-test/unit/modules/cross_ref.sph"
void Point2D__move_to(Point2D * this, int nx, int ny) {
    this->x = nx;
    this->y = ny;
}

#line 10 "sharp-test/unit/modules/cross_ref.sph"
int Point2D__get_x(Point2D * this) {
    return this->x;
}

#line 11 "sharp-test/unit/modules/cross_ref.sph"
int Point2D__get_y(Point2D * this) {
    return this->y;
}

#line 6 "sharp-test/unit/modules/cross_ref.sp"
int distance_sq(Point2D a, Point2D b) {

#line 7 "sharp-test/unit/modules/cross_ref.sp"
    int dx = Point2D__get_x(&a) - Point2D__get_x(&b);

#line 8 "sharp-test/unit/modules/cross_ref.sp"
    int dy = Point2D__get_y(&a) - Point2D__get_y(&b);

#line 9 "sharp-test/unit/modules/cross_ref.sp"
    return dx * dx + dy * dy;
}

#line 12 "sharp-test/unit/modules/cross_ref.sp"
int main() {

#line 13 "sharp-test/unit/modules/cross_ref.sp"
    Point2D p1;

#line 14 "sharp-test/unit/modules/cross_ref.sp"
    Point2D__move_to(&p1, 0, 0);

#line 16 "sharp-test/unit/modules/cross_ref.sp"
    Point2D p2;

#line 17 "sharp-test/unit/modules/cross_ref.sp"
    Point2D__move_to(&p2, 3, 4);

#line 19 "sharp-test/unit/modules/cross_ref.sp"
    int d = distance_sq(p1, p2);

#line 20 "sharp-test/unit/modules/cross_ref.sp"
    if (d != 25) 
        return 1;

#line 22 "sharp-test/unit/modules/cross_ref.sp"
    if (Point2D__get_x(&p1) != 0) 
        return 2;

#line 23 "sharp-test/unit/modules/cross_ref.sp"
    if (Point2D__get_y(&p1) != 0) 
        return 3;

#line 24 "sharp-test/unit/modules/cross_ref.sp"
    if (Point2D__get_x(&p2) != 3) 
        return 4;

#line 25 "sharp-test/unit/modules/cross_ref.sp"
    if (Point2D__get_y(&p2) != 4) 
        return 5;

#line 27 "sharp-test/unit/modules/cross_ref.sp"
    return 0;
}
