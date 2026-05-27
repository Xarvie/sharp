
#include <stdlib.h>

#line 4 "unit/introspection/has_operator.sp"
typedef struct HasPlus HasPlus;
struct HasPlus {
    int v;
};

#line 7 "unit/introspection/has_operator.sp"
int HasPlus__op_add(HasPlus * this, HasPlus rhs);

#line 9 "unit/introspection/has_operator.sp"
struct NoPlus {
    int v;
};

#line 11 "unit/introspection/has_operator.sp"
typedef struct HasEq HasEq;
struct HasEq {
    int v;
};

#line 14 "unit/introspection/has_operator.sp"
int HasEq__op_eq(HasEq * this, HasEq rhs);

#line 15 "unit/introspection/has_operator.sp"
int HasEq__op_lt(HasEq * this, HasEq rhs);

#line 7 "unit/introspection/has_operator.sp"
int HasPlus__op_add(HasPlus * this, HasPlus rhs) {
    return this->v + rhs.v;
}

#line 14 "unit/introspection/has_operator.sp"
int HasEq__op_eq(HasEq * this, HasEq rhs) {
    return this->v == rhs.v;
}

#line 15 "unit/introspection/has_operator.sp"
int HasEq__op_lt(HasEq * this, HasEq rhs) {
    return this->v < rhs.v;
}

#line 17 "unit/introspection/has_operator.sp"
void static_assert_ok(HasPlus a, HasPlus b) {

#line 18 "unit/introspection/has_operator.sp"
    /* @static_assert */ 1;

#line 19 "unit/introspection/has_operator.sp"
    (void)a;
    (void)b;
}

#line 22 "unit/introspection/has_operator.sp"
int branch_plus_has() {

#line 23 "unit/introspection/has_operator.sp"
{
        return 1;
    }
}

#line 25 "unit/introspection/has_operator.sp"
int branch_plus_no() {

#line 26 "unit/introspection/has_operator.sp"
{
        return 0;
    }
}

#line 28 "unit/introspection/has_operator.sp"
int branch_eq_has() {

#line 29 "unit/introspection/has_operator.sp"
{
        return 1;
    }
}

#line 31 "unit/introspection/has_operator.sp"
int branch_eq_no() {

#line 32 "unit/introspection/has_operator.sp"
{
        return 0;
    }
}

#line 34 "unit/introspection/has_operator.sp"
int multi_op_check() {

#line 35 "unit/introspection/has_operator.sp"
    int r = 0;

#line 36 "unit/introspection/has_operator.sp"
{
        r = r + 1;
    }

#line 37 "unit/introspection/has_operator.sp"
{
        r = r + 2;
    }

#line 38 "unit/introspection/has_operator.sp"

#line 39 "unit/introspection/has_operator.sp"

#line 40 "unit/introspection/has_operator.sp"
    return r;
}

#line 43 "unit/introspection/has_operator.sp"
int main(void) {

#line 44 "unit/introspection/has_operator.sp"
    HasPlus a;
    a.v = 0;

#line 45 "unit/introspection/has_operator.sp"
    HasPlus b;
    b.v = 0;

#line 46 "unit/introspection/has_operator.sp"
    static_assert_ok(a, b);

#line 47 "unit/introspection/has_operator.sp"
    if (branch_plus_has() != 1) 
        return 1;

#line 48 "unit/introspection/has_operator.sp"
    if (branch_plus_no() != 0) 
        return 2;

#line 49 "unit/introspection/has_operator.sp"
    if (branch_eq_has() != 1) 
        return 3;

#line 50 "unit/introspection/has_operator.sp"
    if (branch_eq_no() != 0) 
        return 4;

#line 51 "unit/introspection/has_operator.sp"
    if (multi_op_check() != 3) 
        return 5;

#line 52 "unit/introspection/has_operator.sp"
    return 0;
}
