#include <stdbool.h>


#line 8 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
typedef struct Val Val;
struct Val {
    int x;
};

#line 12 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
Val Val__op_add(Val * this, Val other);

#line 15 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
Val Val__op_sub(Val * this);

#line 18 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
_Bool Val__op_eq(Val * this, Val other);

#line 12 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
Val Val__op_add(Val * this, Val other) {

#line 13 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val r;
    r.x = this->x + other.x;
    return r;
}

#line 15 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
Val Val__op_sub(Val * this) {

#line 16 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val r;
    r.x = -this->x;
    return r;
}

#line 18 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
_Bool Val__op_eq(Val * this, Val other) {

#line 19 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    return this->x == other.x;
}

#line 22 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
int main() {

#line 23 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val a;
    a.x = 10;

#line 24 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val b;
    b.x = 3;

#line 27 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val r = Val__op_add((__extension__(({ Val __sharp_bop0 = (Val__op_sub(&a)); &__sharp_bop0; }))), b);

#line 28 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    if (r.x != -7) 
        return 1;

#line 31 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val r2 = Val__op_add(&a, Val__op_sub(&b));

#line 32 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    if (r2.x != 7) 
        return 2;

#line 35 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    Val r3 = Val__op_add((__extension__(({ Val __sharp_bop1 = (Val__op_sub(&a)); &__sharp_bop1; }))), Val__op_sub(&b));

#line 36 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    if (r3.x != -13) 
        return 3;

#line 38 "sharp-test/c_superset_probes/p249_unary_binary_mix.sp"
    return 0;
}
