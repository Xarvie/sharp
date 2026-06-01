
#line 4 "sharp-test/unit/auto/auto_struct.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 9 "sharp-test/unit/auto/auto_struct.ce"
int Point__sum(Point * this);

#line 10 "sharp-test/unit/auto/auto_struct.ce"
void Point__set(Point * this, int nx, int ny);

#line 12 "sharp-test/unit/auto/auto_struct.ce"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 17 "sharp-test/unit/auto/auto_struct.ce"
int Rect__area(Rect * this);

#line 9 "sharp-test/unit/auto/auto_struct.ce"
int Point__sum(Point * this) {
    return this->x + this->y;
}

#line 10 "sharp-test/unit/auto/auto_struct.ce"
void Point__set(Point * this, int nx, int ny) {
    this->x = nx;
    this->y = ny;
}

#line 17 "sharp-test/unit/auto/auto_struct.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 19 "sharp-test/unit/auto/auto_struct.ce"
Point make_point(int x, int y) {

#line 20 "sharp-test/unit/auto/auto_struct.ce"
    Point p;

#line 21 "sharp-test/unit/auto/auto_struct.ce"
    p.x = x;

#line 22 "sharp-test/unit/auto/auto_struct.ce"
    p.y = y;

#line 23 "sharp-test/unit/auto/auto_struct.ce"
    return p;
}

#line 26 "sharp-test/unit/auto/auto_struct.ce"
Rect make_rect(int w, int h) {

#line 27 "sharp-test/unit/auto/auto_struct.ce"
    Rect r;

#line 28 "sharp-test/unit/auto/auto_struct.ce"
    r.w = w;

#line 29 "sharp-test/unit/auto/auto_struct.ce"
    r.h = h;

#line 30 "sharp-test/unit/auto/auto_struct.ce"
    return r;
}

#line 33 "sharp-test/unit/auto/auto_struct.ce"
int main(void) {

#line 35 "sharp-test/unit/auto/auto_struct.ce"
    Point pt;

#line 36 "sharp-test/unit/auto/auto_struct.ce"
    pt.x = 10;

#line 37 "sharp-test/unit/auto/auto_struct.ce"
    pt.y = 20;

#line 38 "sharp-test/unit/auto/auto_struct.ce"
    struct Point apt = pt;

#line 39 "sharp-test/unit/auto/auto_struct.ce"
    if (apt.x != 10) 
        return 1;

#line 40 "sharp-test/unit/auto/auto_struct.ce"
    if (apt.y != 20) 
        return 2;

#line 41 "sharp-test/unit/auto/auto_struct.ce"
    int asum = Point__sum(&apt);

#line 42 "sharp-test/unit/auto/auto_struct.ce"
    if (asum != 30) 
        return 3;

#line 45 "sharp-test/unit/auto/auto_struct.ce"
    Point pt2;

#line 46 "sharp-test/unit/auto/auto_struct.ce"
    pt2.x = 5;

#line 47 "sharp-test/unit/auto/auto_struct.ce"
    pt2.y = 15;

#line 48 "sharp-test/unit/auto/auto_struct.ce"
    Point * pp = &pt2;

#line 49 "sharp-test/unit/auto/auto_struct.ce"
    if (pp->x != 5) 
        return 4;

#line 50 "sharp-test/unit/auto/auto_struct.ce"
    if (pp->y != 15) 
        return 5;

#line 51 "sharp-test/unit/auto/auto_struct.ce"
    Point__set(pp, 100, 200);

#line 52 "sharp-test/unit/auto/auto_struct.ce"
    if (pt2.x != 100) 
        return 6;

#line 53 "sharp-test/unit/auto/auto_struct.ce"
    if (pt2.y != 200) 
        return 7;

#line 56 "sharp-test/unit/auto/auto_struct.ce"
    struct Point rp = make_point(7, 13);

#line 57 "sharp-test/unit/auto/auto_struct.ce"
    if (rp.x != 7) 
        return 8;

#line 58 "sharp-test/unit/auto/auto_struct.ce"
    if (rp.y != 13) 
        return 9;

#line 59 "sharp-test/unit/auto/auto_struct.ce"
    struct Rect rr = make_rect(4, 5);

#line 60 "sharp-test/unit/auto/auto_struct.ce"
    if (Rect__area(&rr) != 20) 
        return 10;

#line 61 "sharp-test/unit/auto/auto_struct.ce"
    int chained = Point__sum((__extension__(({ Point __sharp_rv0 = (make_point(1, 2)); &__sharp_rv0; }))));

#line 62 "sharp-test/unit/auto/auto_struct.ce"
    if (chained != 3) 
        return 11;

#line 65 "sharp-test/unit/auto/auto_struct.ce"
    Point pts[3] = { 0 };

#line 66 "sharp-test/unit/auto/auto_struct.ce"
    pts[0].x = 1;
    pts[0].y = 2;

#line 67 "sharp-test/unit/auto/auto_struct.ce"
    pts[1].x = 3;
    pts[1].y = 4;

#line 68 "sharp-test/unit/auto/auto_struct.ce"
    pts[2].x = 5;
    pts[2].y = 6;

#line 69 "sharp-test/unit/auto/auto_struct.ce"
    struct Point elem = pts[1];

#line 70 "sharp-test/unit/auto/auto_struct.ce"
    if (elem.x != 3) 
        return 12;

#line 71 "sharp-test/unit/auto/auto_struct.ce"
    if (elem.y != 4) 
        return 13;

#line 72 "sharp-test/unit/auto/auto_struct.ce"
    int elem_field = pts[2].x;

#line 73 "sharp-test/unit/auto/auto_struct.ce"
    if (elem_field != 5) 
        return 14;

#line 76 "sharp-test/unit/auto/auto_struct.ce"
    int total_x = 0;

#line 77 "sharp-test/unit/auto/auto_struct.ce"
    int total_y = 0;

#line 78 "sharp-test/unit/auto/auto_struct.ce"
    for (int i = 0; i < 3; i = i + 1) {

#line 79 "sharp-test/unit/auto/auto_struct.ce"
        int px = pts[i].x;

#line 80 "sharp-test/unit/auto/auto_struct.ce"
        int py = pts[i].y;

#line 81 "sharp-test/unit/auto/auto_struct.ce"
        total_x = total_x + px;

#line 82 "sharp-test/unit/auto/auto_struct.ce"
        total_y = total_y + py;
    }

#line 84 "sharp-test/unit/auto/auto_struct.ce"
    if (total_x != 9) 
        return 15;

#line 85 "sharp-test/unit/auto/auto_struct.ce"
    if (total_y != 12) 
        return 16;

#line 88 "sharp-test/unit/auto/auto_struct.ce"
    Point cp;

#line 89 "sharp-test/unit/auto/auto_struct.ce"
    cp.x = 42;

#line 90 "sharp-test/unit/auto/auto_struct.ce"
    cp.y = 58;

#line 91 "sharp-test/unit/auto/auto_struct.ce"
    const Point * cptr = &cp;

#line 92 "sharp-test/unit/auto/auto_struct.ce"
    int cval = Point__sum(cptr);

#line 93 "sharp-test/unit/auto/auto_struct.ce"
    if (cval != 100) 
        return 17;

#line 94 "sharp-test/unit/auto/auto_struct.ce"
    int cx = cptr->x;

#line 95 "sharp-test/unit/auto/auto_struct.ce"
    if (cx != 42) 
        return 18;

#line 96 "sharp-test/unit/auto/auto_struct.ce"
    int cy = cptr->y;

#line 97 "sharp-test/unit/auto/auto_struct.ce"
    if (cy != 58) 
        return 19;

#line 99 "sharp-test/unit/auto/auto_struct.ce"
    return 0;
}
