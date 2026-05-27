#include <stdbool.h>

#include <stdlib.h>

#line 6 "unit/cinterop/init_typedef.sp"
typedef long isize;

#line 9 "unit/cinterop/init_typedef.sp"
typedef struct Pair Pair;
struct Pair {
    int a;
    int b;
};

#line 13 "unit/cinterop/init_typedef.sp"
typedef struct Container Container;
struct Container {
    int val;
    int * ptr;
    Pair inner;
};

#line 20 "unit/cinterop/init_typedef.sp"
typedef isize MySize;

#line 21 "unit/cinterop/init_typedef.sp"
typedef MySize MyLen;

#line 22 "unit/cinterop/init_typedef.sp"
typedef MyLen MyCount;

#line 24 "unit/cinterop/init_typedef.sp"
typedef struct Vec3 Vec3;
struct Vec3 {
    float x;
    float y;
    float z;
};

#line 27 "unit/cinterop/init_typedef.sp"
typedef Vec3 Point3D;

#line 28 "unit/cinterop/init_typedef.sp"
typedef Point3D Position;

#line 30 "unit/cinterop/init_typedef.sp"
float Vec3__dot(Vec3 * this, Vec3 other);
float Vec3__dot(Vec3 * this, Vec3 other) {

#line 31 "unit/cinterop/init_typedef.sp"
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

#line 33 "unit/cinterop/init_typedef.sp"
MyCount add_counts(MyCount a, MyCount b) {

#line 34 "unit/cinterop/init_typedef.sp"
    return a + b;
}

#line 37 "unit/cinterop/init_typedef.sp"
int main() {

#line 39 "unit/cinterop/init_typedef.sp"
    int x = { 0 };

#line 40 "unit/cinterop/init_typedef.sp"
    if (x != 0) 
        return 1;

#line 41 "unit/cinterop/init_typedef.sp"
    float f = { 0 };

#line 42 "unit/cinterop/init_typedef.sp"
    if (f != 0.0f) 
        return 2;

#line 43 "unit/cinterop/init_typedef.sp"
    _Bool b = { 0 };

#line 44 "unit/cinterop/init_typedef.sp"
    if (b != 0) 
        return 3;

#line 46 "unit/cinterop/init_typedef.sp"
    Container c = { 0 };

#line 47 "unit/cinterop/init_typedef.sp"
    if (c.val != 0) 
        return 4;

#line 48 "unit/cinterop/init_typedef.sp"
    if (c.ptr != 0) 
        return 5;

#line 49 "unit/cinterop/init_typedef.sp"
    if (c.inner.a != 0) 
        return 6;

#line 50 "unit/cinterop/init_typedef.sp"
    if (c.inner.b != 0) 
        return 7;

#line 52 "unit/cinterop/init_typedef.sp"
    int * np = { 0 };

#line 53 "unit/cinterop/init_typedef.sp"
    if (np != 0) 
        return 8;

#line 56 "unit/cinterop/init_typedef.sp"
    MyCount n = 10;

#line 57 "unit/cinterop/init_typedef.sp"
    if (n != 10) 
        return 9;

#line 58 "unit/cinterop/init_typedef.sp"
    MyLen l1 = 3;
    MyLen l2 = 5;

#line 59 "unit/cinterop/init_typedef.sp"
    MyCount r = add_counts(l1, l2);

#line 60 "unit/cinterop/init_typedef.sp"
    if (r != 8) 
        return 10;

#line 62 "unit/cinterop/init_typedef.sp"
    Position p;

#line 63 "unit/cinterop/init_typedef.sp"
    p.x = 1.0f;
    p.y = 2.0f;
    p.z = 3.0f;

#line 64 "unit/cinterop/init_typedef.sp"
    Position q;

#line 65 "unit/cinterop/init_typedef.sp"
    q.x = 4.0f;
    q.y = 5.0f;
    q.z = 6.0f;

#line 66 "unit/cinterop/init_typedef.sp"
    float d = Vec3__dot(&p, q);

#line 67 "unit/cinterop/init_typedef.sp"
    if (d < 31.99f || d > 32.01f) 
        return 11;

#line 69 "unit/cinterop/init_typedef.sp"
    Point3D r3;

#line 70 "unit/cinterop/init_typedef.sp"
    r3.x = 0.0f;
    r3.y = 0.0f;
    r3.z = 1.0f;

#line 71 "unit/cinterop/init_typedef.sp"
    if (Vec3__dot(&r3, p) < 2.99f || Vec3__dot(&r3, p) > 3.01f) 
        return 12;

#line 73 "unit/cinterop/init_typedef.sp"
    return 0;
}
