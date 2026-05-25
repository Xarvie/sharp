#include <stdbool.h>


#line 9 "sharp-test/c_superset_probes/p246_operator_chain.sp"
typedef struct Vec3 Vec3;
struct Vec3 {
    int x;
    int y;
    int z;
};

#line 13 "sharp-test/c_superset_probes/p246_operator_chain.sp"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other);

#line 21 "sharp-test/c_superset_probes/p246_operator_chain.sp"
_Bool Vec3__op_eq(Vec3 * this, Vec3 other);

#line 13 "sharp-test/c_superset_probes/p246_operator_chain.sp"
Vec3 Vec3__op_add(Vec3 * this, Vec3 other) {

#line 14 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 r;

#line 15 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    r.x = this->x + other.x;

#line 16 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    r.y = this->y + other.y;

#line 17 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    r.z = this->z + other.z;

#line 18 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    return r;
}

#line 21 "sharp-test/c_superset_probes/p246_operator_chain.sp"
_Bool Vec3__op_eq(Vec3 * this, Vec3 other) {

#line 22 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    return this->x == other.x && this->y == other.y && this->z == other.z;
}

#line 27 "sharp-test/c_superset_probes/p246_operator_chain.sp"
int main() {

#line 28 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 a;
    a.x = 1;
    a.y = 2;
    a.z = 3;

#line 29 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 b;
    b.x = 10;
    b.y = 20;
    b.z = 30;

#line 30 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 c;
    c.x = 100;
    c.y = 200;
    c.z = 300;

#line 33 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 r = Vec3__op_add((__extension__(({ Vec3 __sharp_bop0 = (Vec3__op_add(&a, b)); &__sharp_bop0; }))), c);

#line 34 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    if (r.x != 111) 
        return 1;

#line 35 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    if (r.y != 222) 
        return 2;

#line 36 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    if (r.z != 333) 
        return 3;

#line 39 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 left = Vec3__op_add(&a, b);

#line 40 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    Vec3 right = Vec3__op_add(&b, a);

#line 41 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    if (!(Vec3__op_eq(&left, right))) 
        return 4;

#line 43 "sharp-test/c_superset_probes/p246_operator_chain.sp"
    return 0;
}
