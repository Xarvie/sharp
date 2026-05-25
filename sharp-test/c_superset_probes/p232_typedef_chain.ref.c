

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 11 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef isize MySize;

#line 12 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef MySize MyLen;

#line 13 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef MyLen MyCount;

#line 16 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef struct Vec3 Vec3;
struct Vec3 {
    float x;
    float y;
    float z;
};

#line 19 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef Vec3 Point3D;

#line 20 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
typedef Point3D Position;

#line 22 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
float Vec3__dot(Vec3 * this, Vec3 other);
float Vec3__dot(Vec3 * this, Vec3 other) {

#line 23 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

#line 27 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
MyCount add_counts(MyCount a, MyCount b) {

#line 28 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    return a + b;
}

#line 31 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
int main() {

#line 33 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    MyCount n = 10;

#line 34 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    isize m = n;

#line 35 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    if (m != 10) 
        return 1;

#line 37 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    MyLen l1 = 3;

#line 38 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    MyLen l2 = 5;

#line 39 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    MyCount r = add_counts(l1, l2);

#line 40 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    if (r != 8) 
        return 2;

#line 43 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    Position p;

#line 44 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    p.x = 1.0f;

#line 45 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    p.y = 2.0f;

#line 46 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    p.z = 3.0f;

#line 48 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    Position q;

#line 49 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    q.x = 4.0f;

#line 50 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    q.y = 5.0f;

#line 51 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    q.z = 6.0f;

#line 53 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    float d = Vec3__dot(&p, q);

#line 54 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    if (d < 31.99f || d > 32.01f) 
        return 3;

#line 57 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    Point3D r3;

#line 58 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    r3.x = 0.0f;

#line 59 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    r3.y = 0.0f;

#line 60 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    r3.z = 1.0f;

#line 61 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    if (Vec3__dot(&r3, p) < 2.99f || Vec3__dot(&r3, p) > 3.01f) 
        return 4;

#line 63 "sharp-test/c_superset_probes/p232_typedef_chain.sp"
    return 0;
}
