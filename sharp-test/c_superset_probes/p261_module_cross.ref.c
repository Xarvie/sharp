

#line 3 "c_superset_probes/p261_shape.sph"
typedef struct Point2D Point2D;
struct Point2D {
    int x;
    int y;
};

#line 8 "c_superset_probes/p261_shape.sph"
void Point2D__move_to(Point2D * this, int nx, int ny);

#line 9 "c_superset_probes/p261_shape.sph"
int Point2D__get_x(Point2D * this);

#line 10 "c_superset_probes/p261_shape.sph"
int Point2D__get_y(Point2D * this);

#line 8 "c_superset_probes/p261_shape.sph"
void Point2D__move_to(Point2D * this, int nx, int ny) {
    this->x = nx;
    this->y = ny;
}

#line 9 "c_superset_probes/p261_shape.sph"
int Point2D__get_x(Point2D * this) {
    return this->x;
}

#line 10 "c_superset_probes/p261_shape.sph"
int Point2D__get_y(Point2D * this) {
    return this->y;
}

#line 8 "c_superset_probes/p261_module_cross.sp"
int distance_sq(Point2D a, Point2D b) {

#line 9 "c_superset_probes/p261_module_cross.sp"
    int dx = Point2D__get_x(&a) - Point2D__get_x(&b);

#line 10 "c_superset_probes/p261_module_cross.sp"
    int dy = Point2D__get_y(&a) - Point2D__get_y(&b);

#line 11 "c_superset_probes/p261_module_cross.sp"
    return dx * dx + dy * dy;
}

#line 14 "c_superset_probes/p261_module_cross.sp"
int main() {

#line 15 "c_superset_probes/p261_module_cross.sp"
    Point2D p1;

#line 16 "c_superset_probes/p261_module_cross.sp"
    Point2D__move_to(&p1, 0, 0);

#line 18 "c_superset_probes/p261_module_cross.sp"
    Point2D p2;

#line 19 "c_superset_probes/p261_module_cross.sp"
    Point2D__move_to(&p2, 3, 4);

#line 21 "c_superset_probes/p261_module_cross.sp"
    int d = distance_sq(p1, p2);

#line 22 "c_superset_probes/p261_module_cross.sp"
    if (d != 25) 
        return 1;

#line 24 "c_superset_probes/p261_module_cross.sp"
    if (Point2D__get_x(&p1) != 0) 
        return 2;

#line 25 "c_superset_probes/p261_module_cross.sp"
    if (Point2D__get_y(&p1) != 0) 
        return 3;

#line 26 "c_superset_probes/p261_module_cross.sp"
    if (Point2D__get_x(&p2) != 3) 
        return 4;

#line 27 "c_superset_probes/p261_module_cross.sp"
    if (Point2D__get_y(&p2) != 4) 
        return 5;

#line 29 "c_superset_probes/p261_module_cross.sp"
    return 0;
}
