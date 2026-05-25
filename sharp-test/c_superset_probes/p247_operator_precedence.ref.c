#include <stdbool.h>


#line 9 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
typedef struct Num Num;
struct Num {
    int val;
};

#line 13 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_add(Num * this, Num other);

#line 16 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_sub(Num * this, Num other);

#line 19 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_mul(Num * this, Num other);

#line 22 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_div(Num * this, Num other);

#line 25 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
_Bool Num__op_eq(Num * this, Num other);

#line 29 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
int Num__get(Num * this);

#line 13 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_add(Num * this, Num other) {

#line 14 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r;
    r.val = this->val + other.val;
    return r;
}

#line 16 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_sub(Num * this, Num other) {

#line 17 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r;
    r.val = this->val - other.val;
    return r;
}

#line 19 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_mul(Num * this, Num other) {

#line 20 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r;
    r.val = this->val * other.val;
    return r;
}

#line 22 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
Num Num__op_div(Num * this, Num other) {

#line 23 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r;
    r.val = this->val / other.val;
    return r;
}

#line 25 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
_Bool Num__op_eq(Num * this, Num other) {

#line 26 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    return this->val == other.val;
}

#line 29 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
int Num__get(Num * this) {
    return this->val;
}

#line 31 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
int main() {

#line 32 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num two;
    two.val = 2;

#line 33 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num three;
    three.val = 3;

#line 34 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num four;
    four.val = 4;

#line 35 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num ten;
    ten.val = 10;

#line 38 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r1 = Num__op_add(&two, Num__op_mul(&three, four));

#line 39 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    if (r1.val != 14) 
        return 1;

#line 42 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r2 = Num__op_add((__extension__(({ Num __sharp_bop0 = (Num__op_mul(&two, three)); &__sharp_bop0; }))), four);

#line 43 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    if (r2.val != 10) 
        return 2;

#line 46 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r3 = Num__op_sub((__extension__(({ Num __sharp_bop1 = (Num__op_add(&ten, two)); &__sharp_bop1; }))), Num__op_mul(&three, four));

#line 47 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    if (r3.val != 0) 
        return 3;

#line 50 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    Num r4 = Num__op_div((__extension__(({ Num __sharp_bop2 = (Num__op_mul(&three, four)); &__sharp_bop2; }))), two);

#line 51 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    if (r4.val != 6) 
        return 4;

#line 53 "sharp-test/c_superset_probes/p247_operator_precedence.sp"
    return 0;
}
