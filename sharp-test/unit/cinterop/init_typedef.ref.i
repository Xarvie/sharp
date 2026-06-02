
#line 6 "sharp-test/unit/cinterop/init_typedef.ce"
typedef long isize;

#line 9 "sharp-test/unit/cinterop/init_typedef.ce"
typedef struct Pair Pair;
struct Pair {
    int a;
    int b;
};

#line 13 "sharp-test/unit/cinterop/init_typedef.ce"
typedef struct Container Container;
struct Container {
    int val;
    int * ptr;
    Pair inner;
};

#line 20 "sharp-test/unit/cinterop/init_typedef.ce"
typedef isize MySize;

#line 21 "sharp-test/unit/cinterop/init_typedef.ce"
typedef MySize MyLen;

#line 22 "sharp-test/unit/cinterop/init_typedef.ce"
typedef MyLen MyCount;

#line 24 "sharp-test/unit/cinterop/init_typedef.ce"
typedef struct Vec3 Vec3;
struct Vec3 {
    float x;
    float y;
    float z;
};

#line 27 "sharp-test/unit/cinterop/init_typedef.ce"
typedef Vec3 Point3D;

#line 28 "sharp-test/unit/cinterop/init_typedef.ce"
typedef Point3D Position;

#line 30 "sharp-test/unit/cinterop/init_typedef.ce"
float Vec3__dot(Vec3 * this, Vec3 other);
float Vec3__dot(Vec3 * this, Vec3 other) {

#line 31 "sharp-test/unit/cinterop/init_typedef.ce"
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

#line 33 "sharp-test/unit/cinterop/init_typedef.ce"
MyCount add_counts(MyCount a, MyCount b) {

#line 34 "sharp-test/unit/cinterop/init_typedef.ce"
    return a + b;
}

#line 37 "sharp-test/unit/cinterop/init_typedef.ce"
MyCount multiply_counts(MyCount a, int factor) {

#line 38 "sharp-test/unit/cinterop/init_typedef.ce"
    return a * factor;
}

#line 41 "sharp-test/unit/cinterop/init_typedef.ce"
int main() {

#line 43 "sharp-test/unit/cinterop/init_typedef.ce"
    int x = { 0 };

#line 44 "sharp-test/unit/cinterop/init_typedef.ce"
    if (x != 0) 
        return 1;

#line 45 "sharp-test/unit/cinterop/init_typedef.ce"
    float f = { 0 };

#line 46 "sharp-test/unit/cinterop/init_typedef.ce"
    if (f != 0.0f) 
        return 2;

#line 47 "sharp-test/unit/cinterop/init_typedef.ce"
    bool b = { 0 };

#line 48 "sharp-test/unit/cinterop/init_typedef.ce"
    if (b != false) 
        return 3;

#line 50 "sharp-test/unit/cinterop/init_typedef.ce"
    Container c = { 0 };

#line 51 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c.val != 0) 
        return 4;

#line 52 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c.ptr != 0) 
        return 5;

#line 53 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c.inner.a != 0) 
        return 6;

#line 54 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c.inner.b != 0) 
        return 7;

#line 56 "sharp-test/unit/cinterop/init_typedef.ce"
    int * np = { 0 };

#line 57 "sharp-test/unit/cinterop/init_typedef.ce"
    if (np != 0) 
        return 8;

#line 60 "sharp-test/unit/cinterop/init_typedef.ce"
    MyCount n = 10;

#line 61 "sharp-test/unit/cinterop/init_typedef.ce"
    if (n != 10) 
        return 9;

#line 62 "sharp-test/unit/cinterop/init_typedef.ce"
    MyLen l1 = 3;
    MyLen l2 = 5;

#line 63 "sharp-test/unit/cinterop/init_typedef.ce"
    MyCount r = add_counts(l1, l2);

#line 64 "sharp-test/unit/cinterop/init_typedef.ce"
    if (r != 8) 
        return 10;

#line 66 "sharp-test/unit/cinterop/init_typedef.ce"
    Position p;

#line 67 "sharp-test/unit/cinterop/init_typedef.ce"
    p.x = 1.0f;
    p.y = 2.0f;
    p.z = 3.0f;

#line 68 "sharp-test/unit/cinterop/init_typedef.ce"
    Position q;

#line 69 "sharp-test/unit/cinterop/init_typedef.ce"
    q.x = 4.0f;
    q.y = 5.0f;
    q.z = 6.0f;

#line 70 "sharp-test/unit/cinterop/init_typedef.ce"
    float d = Vec3__dot(&p, q);

#line 71 "sharp-test/unit/cinterop/init_typedef.ce"
    if (d < 31.99f || d > 32.01f) 
        return 11;

#line 73 "sharp-test/unit/cinterop/init_typedef.ce"
    Point3D r3;

#line 74 "sharp-test/unit/cinterop/init_typedef.ce"
    r3.x = 0.0f;
    r3.y = 0.0f;
    r3.z = 1.0f;

#line 75 "sharp-test/unit/cinterop/init_typedef.ce"
    if (Vec3__dot(&r3, p) < 2.99f || Vec3__dot(&r3, p) > 3.01f) 
        return 12;

#line 78 "sharp-test/unit/cinterop/init_typedef.ce"
    MyCount mc = multiply_counts(5, 3);

#line 79 "sharp-test/unit/cinterop/init_typedef.ce"
    if (mc != 15) 
        return 13;

#line 82 "sharp-test/unit/cinterop/init_typedef.ce"
    Container c2 = { 0 };

#line 83 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c2.val != 0) 
        return 14;

#line 84 "sharp-test/unit/cinterop/init_typedef.ce"
    if (c2.inner.a != 0) 
        return 15;

#line 86 "sharp-test/unit/cinterop/init_typedef.ce"
    return 0;
}
