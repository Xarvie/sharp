

#line 20 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
typedef struct HasPlus HasPlus;
struct HasPlus {
    int v;
};

#line 23 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasPlus__op_add(HasPlus * this, HasPlus rhs);

#line 25 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
struct NoPlus {
    int v;
};

#line 27 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
typedef struct HasEq HasEq;
struct HasEq {
    int v;
};

#line 30 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasEq__op_eq(HasEq * this, HasEq rhs);

#line 31 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasEq__op_lt(HasEq * this, HasEq rhs);

#line 23 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasPlus__op_add(HasPlus * this, HasPlus rhs) {
    return this->v + rhs.v;
}

#line 30 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasEq__op_eq(HasEq * this, HasEq rhs) {
    return this->v == rhs.v;
}

#line 31 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int HasEq__op_lt(HasEq * this, HasEq rhs) {
    return this->v < rhs.v;
}

#line 34 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
void static_assert_ok(HasPlus a, HasPlus b) {

#line 35 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    /* @static_assert */ 1;

#line 36 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    (void)a;
    (void)b;
}

#line 40 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int branch_plus_has() {

#line 41 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        return 1;
    }
}

#line 43 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int branch_plus_no() {

#line 44 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        return 0;
    }
}

#line 48 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int branch_eq_has() {

#line 49 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        return 1;
    }
}

#line 51 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int branch_eq_no() {

#line 52 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        return 0;
    }
}

#line 56 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int multi_op_check() {

#line 57 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    int r = 0;

#line 58 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        r = r + 1;
    }

#line 59 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
{
        r = r + 2;
    }

#line 60 "sharp-test/c_superset_probes/p50_if_has_operator.sp"

#line 61 "sharp-test/c_superset_probes/p50_if_has_operator.sp"

#line 62 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    return r;
}

#line 65 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
int main(void) {

#line 66 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    HasPlus a;
    a.v = 0;

#line 67 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    HasPlus b;
    b.v = 0;

#line 68 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    static_assert_ok(a, b);

#line 70 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    if (branch_plus_has() != 1) 
        return 1;

#line 71 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    if (branch_plus_no() != 0) 
        return 2;

#line 72 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    if (branch_eq_has() != 1) 
        return 3;

#line 73 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    if (branch_eq_no() != 0) 
        return 4;

#line 74 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    if (multi_op_check() != 3) 
        return 5;

#line 75 "sharp-test/c_superset_probes/p50_if_has_operator.sp"
    return 0;
}
