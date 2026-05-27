
#include <stdlib.h>

#line 7 "unit/extensions/dispatch.sp"
typedef struct Point2D Point2D;
struct Point2D {
    int x;
    int y;
};

#line 8 "unit/extensions/dispatch.sp"
typedef struct Point3D Point3D;
struct Point3D {
    int x;
    int y;
    int z;
};

#line 10 "unit/extensions/dispatch.sp"
int Point2D__magnitude_sq(Point2D * this);

#line 11 "unit/extensions/dispatch.sp"
int Point3D__magnitude_sq(Point3D * this);

#line 14 "unit/extensions/dispatch.sp"
struct Vec {
    int x;
    int y;
};

#line 20 "unit/extensions/dispatch.sp"
int Vec__dot(struct Vec * this, struct Vec b);

#line 23 "unit/extensions/dispatch.sp"
typedef struct Data Data;
struct Data {
    int val;
};

#line 24 "unit/extensions/dispatch.sp"
int Data__get_val(Data * this);

#line 25 "unit/extensions/dispatch.sp"
void Data__set_val(Data * this, int v);

#line 10 "unit/extensions/dispatch.sp"
int Point2D__magnitude_sq(Point2D * this) {
    return this->x * this->x + this->y * this->y;
}

#line 11 "unit/extensions/dispatch.sp"
int Point3D__magnitude_sq(Point3D * this) {
    return this->x * this->x + this->y * this->y + this->z * this->z;
}

#line 17 "unit/extensions/dispatch.sp"
int vec_dot_free(struct Vec a, struct Vec b) {
    return a.x * b.x + a.y * b.y;
}

#line 20 "unit/extensions/dispatch.sp"
int Vec__dot(struct Vec * this, struct Vec b) {
    return this->x * b.x + this->y * b.y;
}

#line 24 "unit/extensions/dispatch.sp"
int Data__get_val(Data * this) {
    return this->val;
}

#line 25 "unit/extensions/dispatch.sp"
void Data__set_val(Data * this, int v) {
    this->val = v;
}

#line 27 "unit/extensions/dispatch.sp"
int compute_sum(Data a, Data b) {

#line 28 "unit/extensions/dispatch.sp"
    return Data__get_val(&a) + Data__get_val(&b);
}

#line 31 "unit/extensions/dispatch.sp"
int main() {

#line 33 "unit/extensions/dispatch.sp"
    Point2D p2;
    p2.x = 3;
    p2.y = 4;

#line 34 "unit/extensions/dispatch.sp"
    Point3D p3;
    p3.x = 1;
    p3.y = 2;
    p3.z = 2;

#line 35 "unit/extensions/dispatch.sp"
    int mag2 = Point2D__magnitude_sq(&p2);

#line 36 "unit/extensions/dispatch.sp"
    int mag3 = Point3D__magnitude_sq(&p3);

#line 37 "unit/extensions/dispatch.sp"
    if (mag2 != 25) 
        return 1;

#line 38 "unit/extensions/dispatch.sp"
    if (mag3 != 9) 
        return 2;

#line 41 "unit/extensions/dispatch.sp"
    struct Vec va = { 2, 3 };

#line 42 "unit/extensions/dispatch.sp"
    struct Vec vb = { 5, 7 };

#line 43 "unit/extensions/dispatch.sp"
    int dot_ext = Vec__dot(&va, vb);

#line 44 "unit/extensions/dispatch.sp"
    int dot_free = vec_dot_free(va, vb);

#line 45 "unit/extensions/dispatch.sp"
    if (dot_ext != 31) 
        return 3;

#line 46 "unit/extensions/dispatch.sp"
    if (dot_free != 31) 
        return 4;

#line 49 "unit/extensions/dispatch.sp"
    Data a;
    Data__set_val(&a, 10);

#line 50 "unit/extensions/dispatch.sp"
    Data b;
    Data__set_val(&b, 20);

#line 51 "unit/extensions/dispatch.sp"
    int s = compute_sum(a, b);

#line 52 "unit/extensions/dispatch.sp"
    if (s != 30) 
        return 5;

#line 53 "unit/extensions/dispatch.sp"
    if (Data__get_val(&a) != 10) 
        return 6;

#line 55 "unit/extensions/dispatch.sp"
    return 0;
}
