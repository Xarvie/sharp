
#line 7 "sharp-test/unit/extensions/dispatch.ce"
typedef struct Point2D Point2D;
struct Point2D {
    int x;
    int y;
};

#line 8 "sharp-test/unit/extensions/dispatch.ce"
typedef struct Point3D Point3D;
struct Point3D {
    int x;
    int y;
    int z;
};

#line 10 "sharp-test/unit/extensions/dispatch.ce"
int Point2D__magnitude_sq(Point2D * this);

#line 11 "sharp-test/unit/extensions/dispatch.ce"
int Point3D__magnitude_sq(Point3D * this);

#line 14 "sharp-test/unit/extensions/dispatch.ce"
struct Vec {
    int x;
    int y;
};

#line 20 "sharp-test/unit/extensions/dispatch.ce"
int Vec__dot(struct Vec * this, struct Vec b);

#line 23 "sharp-test/unit/extensions/dispatch.ce"
typedef struct Data Data;
struct Data {
    int val;
};

#line 24 "sharp-test/unit/extensions/dispatch.ce"
int Data__get_val(Data * this);

#line 25 "sharp-test/unit/extensions/dispatch.ce"
void Data__set_val(Data * this, int v);

#line 31 "sharp-test/unit/extensions/dispatch.ce"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 32 "sharp-test/unit/extensions/dispatch.ce"
int Rect__area(Rect * this);

#line 33 "sharp-test/unit/extensions/dispatch.ce"
void Rect__scale(Rect * this, int f);

#line 10 "sharp-test/unit/extensions/dispatch.ce"
int Point2D__magnitude_sq(Point2D * this) {
    return this->x * this->x + this->y * this->y;
}

#line 11 "sharp-test/unit/extensions/dispatch.ce"
int Point3D__magnitude_sq(Point3D * this) {
    return this->x * this->x + this->y * this->y + this->z * this->z;
}

#line 17 "sharp-test/unit/extensions/dispatch.ce"
int vec_dot_free(struct Vec a, struct Vec b) {
    return a.x * b.x + a.y * b.y;
}

#line 20 "sharp-test/unit/extensions/dispatch.ce"
int Vec__dot(struct Vec * this, struct Vec b) {
    return this->x * b.x + this->y * b.y;
}

#line 24 "sharp-test/unit/extensions/dispatch.ce"
int Data__get_val(Data * this) {
    return this->val;
}

#line 25 "sharp-test/unit/extensions/dispatch.ce"
void Data__set_val(Data * this, int v) {
    this->val = v;
}

#line 27 "sharp-test/unit/extensions/dispatch.ce"
int compute_sum(Data a, Data b) {

#line 28 "sharp-test/unit/extensions/dispatch.ce"
    return Data__get_val(&a) + Data__get_val(&b);
}

#line 32 "sharp-test/unit/extensions/dispatch.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 33 "sharp-test/unit/extensions/dispatch.ce"
void Rect__scale(Rect * this, int f) {
    this->w = this->w * f;
    this->h = this->h * f;
}

#line 35 "sharp-test/unit/extensions/dispatch.ce"
int main() {

#line 37 "sharp-test/unit/extensions/dispatch.ce"
    Point2D p2;
    p2.x = 3;
    p2.y = 4;

#line 38 "sharp-test/unit/extensions/dispatch.ce"
    Point3D p3;
    p3.x = 1;
    p3.y = 2;
    p3.z = 2;

#line 39 "sharp-test/unit/extensions/dispatch.ce"
    int mag2 = Point2D__magnitude_sq(&p2);

#line 40 "sharp-test/unit/extensions/dispatch.ce"
    int mag3 = Point3D__magnitude_sq(&p3);

#line 41 "sharp-test/unit/extensions/dispatch.ce"
    if (mag2 != 25) 
        return 1;

#line 42 "sharp-test/unit/extensions/dispatch.ce"
    if (mag3 != 9) 
        return 2;

#line 45 "sharp-test/unit/extensions/dispatch.ce"
    struct Vec va = { 2, 3 };

#line 46 "sharp-test/unit/extensions/dispatch.ce"
    struct Vec vb = { 5, 7 };

#line 47 "sharp-test/unit/extensions/dispatch.ce"
    int dot_ext = Vec__dot(&va, vb);

#line 48 "sharp-test/unit/extensions/dispatch.ce"
    int dot_free = vec_dot_free(va, vb);

#line 49 "sharp-test/unit/extensions/dispatch.ce"
    if (dot_ext != 31) 
        return 3;

#line 50 "sharp-test/unit/extensions/dispatch.ce"
    if (dot_free != 31) 
        return 4;

#line 53 "sharp-test/unit/extensions/dispatch.ce"
    Data a;
    Data__set_val(&a, 10);

#line 54 "sharp-test/unit/extensions/dispatch.ce"
    Data b;
    Data__set_val(&b, 20);

#line 55 "sharp-test/unit/extensions/dispatch.ce"
    int s = compute_sum(a, b);

#line 56 "sharp-test/unit/extensions/dispatch.ce"
    if (s != 30) 
        return 5;

#line 57 "sharp-test/unit/extensions/dispatch.ce"
    if (Data__get_val(&a) != 10) 
        return 6;

#line 60 "sharp-test/unit/extensions/dispatch.ce"
    Rect r;
    r.w = 3;
    r.h = 4;

#line 61 "sharp-test/unit/extensions/dispatch.ce"
    if (Rect__area(&r) != 12) 
        return 7;

#line 62 "sharp-test/unit/extensions/dispatch.ce"
    Rect__scale(&r, 2);

#line 63 "sharp-test/unit/extensions/dispatch.ce"
    if (Rect__area(&r) != 48) 
        return 8;

#line 65 "sharp-test/unit/extensions/dispatch.ce"
    const Rect * pr = &r;

#line 66 "sharp-test/unit/extensions/dispatch.ce"
    if (Rect__area(pr) != 48) 
        return 9;

#line 68 "sharp-test/unit/extensions/dispatch.ce"
    return 0;
}
