
#line 3 "sharp-test/unit/const/const_struct.ce"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 8 "sharp-test/unit/const/const_struct.ce"
int Point__sum(Point * this);

#line 9 "sharp-test/unit/const/const_struct.ce"
void Point__move(Point * this, int dx, int dy);

#line 11 "sharp-test/unit/const/const_struct.ce"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 16 "sharp-test/unit/const/const_struct.ce"
int Rect__area(Rect * this);

#line 17 "sharp-test/unit/const/const_struct.ce"
int Rect__perimeter(Rect * this);

#line 19 "sharp-test/unit/const/const_struct.ce"
typedef struct Data Data;
struct Data {
    int value;
};

#line 23 "sharp-test/unit/const/const_struct.ce"
int Data__get(Data * this);

#line 24 "sharp-test/unit/const/const_struct.ce"
void Data__set(Data * this, int v);

#line 8 "sharp-test/unit/const/const_struct.ce"
int Point__sum(Point * this) {
    return this->x + this->y;
}

#line 9 "sharp-test/unit/const/const_struct.ce"
void Point__move(Point * this, int dx, int dy) {
    this->x = this->x + dx;
    this->y = this->y + dy;
}

#line 16 "sharp-test/unit/const/const_struct.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 17 "sharp-test/unit/const/const_struct.ce"
int Rect__perimeter(Rect * this) {
    return 2 * (this->w + this->h);
}

#line 23 "sharp-test/unit/const/const_struct.ce"
int Data__get(Data * this) {
    return this->value;
}

#line 24 "sharp-test/unit/const/const_struct.ce"
void Data__set(Data * this, int v) {
    this->value = v;
}

#line 26 "sharp-test/unit/const/const_struct.ce"
int compute_area(const Rect * r) {

#line 27 "sharp-test/unit/const/const_struct.ce"
    return Rect__area(r);
}

#line 30 "sharp-test/unit/const/const_struct.ce"
int point_sum_const(const Point * p) {

#line 31 "sharp-test/unit/const/const_struct.ce"
    return Point__sum(p);
}

#line 34 "sharp-test/unit/const/const_struct.ce"
const Point * get_const_point(Point * p) {

#line 35 "sharp-test/unit/const/const_struct.ce"
    return p;
}

#line 38 "sharp-test/unit/const/const_struct.ce"
const Data * get_const_data(Data * d) {

#line 39 "sharp-test/unit/const/const_struct.ce"
    return d;
}

#line 42 "sharp-test/unit/const/const_struct.ce"
int main(void) {

#line 44 "sharp-test/unit/const/const_struct.ce"
    Point pt = { 3, 4 };

#line 45 "sharp-test/unit/const/const_struct.ce"
    const Point cpt = pt;

#line 46 "sharp-test/unit/const/const_struct.ce"
    if (cpt.x != 3) 
        return 1;

#line 47 "sharp-test/unit/const/const_struct.ce"
    if (cpt.y != 4) 
        return 2;

#line 49 "sharp-test/unit/const/const_struct.ce"
    Point pt2 = { 10, 20 };

#line 50 "sharp-test/unit/const/const_struct.ce"
    const Point cpt2 = pt2;

#line 51 "sharp-test/unit/const/const_struct.ce"
    if (cpt2.x != 10) 
        return 3;

#line 52 "sharp-test/unit/const/const_struct.ce"
    if (cpt2.y != 20) 
        return 4;

#line 55 "sharp-test/unit/const/const_struct.ce"
    Point a = { 1, 2 };

#line 56 "sharp-test/unit/const/const_struct.ce"
    Point b = { 10, 20 };

#line 57 "sharp-test/unit/const/const_struct.ce"
    const Point * cp = &a;

#line 58 "sharp-test/unit/const/const_struct.ce"
    if (cp->x != 1) 
        return 5;

#line 59 "sharp-test/unit/const/const_struct.ce"
    if (cp->y != 2) 
        return 6;

#line 60 "sharp-test/unit/const/const_struct.ce"
    cp = &b;

#line 61 "sharp-test/unit/const/const_struct.ce"
    if (cp->x != 10) 
        return 7;

#line 62 "sharp-test/unit/const/const_struct.ce"
    if (cp->y != 20) 
        return 8;

#line 65 "sharp-test/unit/const/const_struct.ce"
    Point c = { 10, 20 };

#line 66 "sharp-test/unit/const/const_struct.ce"
    const Point * pc = &c;

#line 67 "sharp-test/unit/const/const_struct.ce"
    if (Point__sum(pc) != 30) 
        return 9;

#line 68 "sharp-test/unit/const/const_struct.ce"
    c.x = 15;

#line 69 "sharp-test/unit/const/const_struct.ce"
    if (Point__sum(pc) != 35) 
        return 10;

#line 70 "sharp-test/unit/const/const_struct.ce"
    Point__move(&c, 5, 5);

#line 71 "sharp-test/unit/const/const_struct.ce"
    if (pc->x != 20) 
        return 11;

#line 72 "sharp-test/unit/const/const_struct.ce"
    if (pc->y != 25) 
        return 12;

#line 75 "sharp-test/unit/const/const_struct.ce"
    Rect r = { 5, 6 };

#line 76 "sharp-test/unit/const/const_struct.ce"
    const Rect * pr = &r;

#line 77 "sharp-test/unit/const/const_struct.ce"
    if (Rect__area(pr) != 30) 
        return 13;

#line 78 "sharp-test/unit/const/const_struct.ce"
    if (Rect__perimeter(pr) != 22) 
        return 14;

#line 80 "sharp-test/unit/const/const_struct.ce"
    const Rect cr = r;

#line 81 "sharp-test/unit/const/const_struct.ce"
    const Rect * pcr = &cr;

#line 82 "sharp-test/unit/const/const_struct.ce"
    if (Rect__area(pcr) != 30) 
        return 15;

#line 83 "sharp-test/unit/const/const_struct.ce"
    if (Rect__perimeter(pcr) != 22) 
        return 16;

#line 86 "sharp-test/unit/const/const_struct.ce"
    Rect rr = { 7, 8 };

#line 87 "sharp-test/unit/const/const_struct.ce"
    if (compute_area(&rr) != 56) 
        return 17;

#line 89 "sharp-test/unit/const/const_struct.ce"
    const Rect cr2 = rr;

#line 90 "sharp-test/unit/const/const_struct.ce"
    if (compute_area(&cr2) != 56) 
        return 18;

#line 92 "sharp-test/unit/const/const_struct.ce"
    Point pp = { 100, 200 };

#line 93 "sharp-test/unit/const/const_struct.ce"
    if (point_sum_const(&pp) != 300) 
        return 19;

#line 95 "sharp-test/unit/const/const_struct.ce"
    const Point cpp = pp;

#line 96 "sharp-test/unit/const/const_struct.ce"
    if (point_sum_const(&cpp) != 300) 
        return 20;

#line 99 "sharp-test/unit/const/const_struct.ce"
    Point p = { 100, 200 };

#line 100 "sharp-test/unit/const/const_struct.ce"
    const Point * pptr = get_const_point(&p);

#line 101 "sharp-test/unit/const/const_struct.ce"
    if (pptr->x != 100) 
        return 21;

#line 102 "sharp-test/unit/const/const_struct.ce"
    if (Point__sum(pptr) != 300) 
        return 22;

#line 103 "sharp-test/unit/const/const_struct.ce"
    p.x = 150;

#line 104 "sharp-test/unit/const/const_struct.ce"
    if (pptr->x != 150) 
        return 23;

#line 106 "sharp-test/unit/const/const_struct.ce"
    Data d;

#line 107 "sharp-test/unit/const/const_struct.ce"
    Data__set(&d, 42);

#line 108 "sharp-test/unit/const/const_struct.ce"
    const Data * dptr = get_const_data(&d);

#line 109 "sharp-test/unit/const/const_struct.ce"
    if (Data__get(dptr) != 42) 
        return 24;

#line 110 "sharp-test/unit/const/const_struct.ce"
    Data__set(&d, 99);

#line 111 "sharp-test/unit/const/const_struct.ce"
    if (Data__get(dptr) != 99) 
        return 25;

#line 113 "sharp-test/unit/const/const_struct.ce"
    return 0;
}
