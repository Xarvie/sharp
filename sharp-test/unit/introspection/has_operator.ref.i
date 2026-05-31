
#line 4 "sharp-test/unit/introspection/has_operator.ce"
typedef struct HasPlus HasPlus;
struct HasPlus {
    int v;
};

#line 7 "sharp-test/unit/introspection/has_operator.ce"
int HasPlus__op_add(HasPlus * this, HasPlus rhs);

#line 9 "sharp-test/unit/introspection/has_operator.ce"
struct NoPlus {
    int v;
};

#line 11 "sharp-test/unit/introspection/has_operator.ce"
typedef struct HasEq HasEq;
struct HasEq {
    int v;
};

#line 14 "sharp-test/unit/introspection/has_operator.ce"
int HasEq__op_eq(HasEq * this, HasEq rhs);

#line 15 "sharp-test/unit/introspection/has_operator.ce"
int HasEq__op_lt(HasEq * this, HasEq rhs);

#line 43 "sharp-test/unit/introspection/has_operator.ce"
typedef struct HasAllOps HasAllOps;
struct HasAllOps {
    int v;
};

#line 46 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_add(HasAllOps * this, HasAllOps rhs);

#line 47 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_sub(HasAllOps * this, HasAllOps rhs);

#line 48 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_eq(HasAllOps * this, HasAllOps rhs);

#line 7 "sharp-test/unit/introspection/has_operator.ce"
int HasPlus__op_add(HasPlus * this, HasPlus rhs) {
    return this->v + rhs.v;
}

#line 14 "sharp-test/unit/introspection/has_operator.ce"
int HasEq__op_eq(HasEq * this, HasEq rhs) {
    return this->v == rhs.v;
}

#line 15 "sharp-test/unit/introspection/has_operator.ce"
int HasEq__op_lt(HasEq * this, HasEq rhs) {
    return this->v < rhs.v;
}

#line 17 "sharp-test/unit/introspection/has_operator.ce"
void static_assert_ok(HasPlus a, HasPlus b) {

#line 18 "sharp-test/unit/introspection/has_operator.ce"
    /* @static_assert */ 1;

#line 19 "sharp-test/unit/introspection/has_operator.ce"
    (void)a;
    (void)b;
}

#line 22 "sharp-test/unit/introspection/has_operator.ce"
int branch_plus_has() {

#line 23 "sharp-test/unit/introspection/has_operator.ce"
{
        return 1;
    }
}

#line 25 "sharp-test/unit/introspection/has_operator.ce"
int branch_plus_no() {

#line 26 "sharp-test/unit/introspection/has_operator.ce"
{
        return 0;
    }
}

#line 28 "sharp-test/unit/introspection/has_operator.ce"
int branch_eq_has() {

#line 29 "sharp-test/unit/introspection/has_operator.ce"
{
        return 1;
    }
}

#line 31 "sharp-test/unit/introspection/has_operator.ce"
int branch_eq_no() {

#line 32 "sharp-test/unit/introspection/has_operator.ce"
{
        return 0;
    }
}

#line 34 "sharp-test/unit/introspection/has_operator.ce"
int multi_op_check() {

#line 35 "sharp-test/unit/introspection/has_operator.ce"
    int r = 0;

#line 36 "sharp-test/unit/introspection/has_operator.ce"
{
        r = r + 1;
    }

#line 37 "sharp-test/unit/introspection/has_operator.ce"
{
        r = r + 2;
    }

#line 38 "sharp-test/unit/introspection/has_operator.ce"

#line 39 "sharp-test/unit/introspection/has_operator.ce"

#line 40 "sharp-test/unit/introspection/has_operator.ce"
    return r;
}

#line 46 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_add(HasAllOps * this, HasAllOps rhs) {
    return this->v + rhs.v;
}

#line 47 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_sub(HasAllOps * this, HasAllOps rhs) {
    return this->v - rhs.v;
}

#line 48 "sharp-test/unit/introspection/has_operator.ce"
int HasAllOps__op_eq(HasAllOps * this, HasAllOps rhs) {
    return this->v == rhs.v;
}

#line 50 "sharp-test/unit/introspection/has_operator.ce"
int try_add(HasAllOps a, HasAllOps b) {

#line 51 "sharp-test/unit/introspection/has_operator.ce"
{

#line 52 "sharp-test/unit/introspection/has_operator.ce"
        return HasAllOps__op_add(&a, b);
    }

#line 54 "sharp-test/unit/introspection/has_operator.ce"
    return 0;
}

#line 57 "sharp-test/unit/introspection/has_operator.ce"
int main(void) {

#line 58 "sharp-test/unit/introspection/has_operator.ce"
    HasPlus a;
    a.v = 0;

#line 59 "sharp-test/unit/introspection/has_operator.ce"
    HasPlus b;
    b.v = 0;

#line 60 "sharp-test/unit/introspection/has_operator.ce"
    static_assert_ok(a, b);

#line 61 "sharp-test/unit/introspection/has_operator.ce"
    if (branch_plus_has() != 1) 
        return 1;

#line 62 "sharp-test/unit/introspection/has_operator.ce"
    if (branch_plus_no() != 0) 
        return 2;

#line 63 "sharp-test/unit/introspection/has_operator.ce"
    if (branch_eq_has() != 1) 
        return 3;

#line 64 "sharp-test/unit/introspection/has_operator.ce"
    if (branch_eq_no() != 0) 
        return 4;

#line 65 "sharp-test/unit/introspection/has_operator.ce"
    if (multi_op_check() != 3) 
        return 5;

#line 68 "sharp-test/unit/introspection/has_operator.ce"
    HasAllOps x;
    x.v = 10;

#line 69 "sharp-test/unit/introspection/has_operator.ce"
    HasAllOps y;
    y.v = 20;

#line 70 "sharp-test/unit/introspection/has_operator.ce"
    if (try_add(x, y) != 30) 
        return 6;

#line 71 "sharp-test/unit/introspection/has_operator.ce"
    if (!/* @has_operator */ 1) 
        return 7;

#line 72 "sharp-test/unit/introspection/has_operator.ce"

#line 74 "sharp-test/unit/introspection/has_operator.ce"
    return 0;
}
