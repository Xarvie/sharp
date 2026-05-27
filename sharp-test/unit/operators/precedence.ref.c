#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/operators/precedence.sp"
typedef struct Num Num;
struct Num {
    int val;
};

#line 10 "unit/operators/precedence.sp"
Num Num__op_add(Num * this, Num other);

#line 13 "unit/operators/precedence.sp"
Num Num__op_sub(Num * this, Num other);

#line 16 "unit/operators/precedence.sp"
Num Num__op_mul(Num * this, Num other);

#line 19 "unit/operators/precedence.sp"
Num Num__op_div(Num * this, Num other);

#line 22 "unit/operators/precedence.sp"
_Bool Num__op_eq(Num * this, Num other);

#line 27 "unit/operators/precedence.sp"
typedef struct Score Score;
struct Score {
    int pts;
};

#line 30 "unit/operators/precedence.sp"
_Bool Score__op_eq(Score * this, Score other);

#line 33 "unit/operators/precedence.sp"
_Bool Score__op_lt(Score * this, Score other);

#line 36 "unit/operators/precedence.sp"
_Bool Score__op_gt(Score * this, Score other);

#line 39 "unit/operators/precedence.sp"
_Bool Score__op_le(Score * this, Score other);

#line 42 "unit/operators/precedence.sp"
_Bool Score__op_ge(Score * this, Score other);

#line 10 "unit/operators/precedence.sp"
Num Num__op_add(Num * this, Num other) {

#line 11 "unit/operators/precedence.sp"
    Num r;
    r.val = this->val + other.val;
    return r;
}

#line 13 "unit/operators/precedence.sp"
Num Num__op_sub(Num * this, Num other) {

#line 14 "unit/operators/precedence.sp"
    Num r;
    r.val = this->val - other.val;
    return r;
}

#line 16 "unit/operators/precedence.sp"
Num Num__op_mul(Num * this, Num other) {

#line 17 "unit/operators/precedence.sp"
    Num r;
    r.val = this->val * other.val;
    return r;
}

#line 19 "unit/operators/precedence.sp"
Num Num__op_div(Num * this, Num other) {

#line 20 "unit/operators/precedence.sp"
    Num r;
    r.val = this->val / other.val;
    return r;
}

#line 22 "unit/operators/precedence.sp"
_Bool Num__op_eq(Num * this, Num other) {

#line 23 "unit/operators/precedence.sp"
    return this->val == other.val;
}

#line 30 "unit/operators/precedence.sp"
_Bool Score__op_eq(Score * this, Score other) {

#line 31 "unit/operators/precedence.sp"
    return this->pts == other.pts;
}

#line 33 "unit/operators/precedence.sp"
_Bool Score__op_lt(Score * this, Score other) {

#line 34 "unit/operators/precedence.sp"
    return this->pts < other.pts;
}

#line 36 "unit/operators/precedence.sp"
_Bool Score__op_gt(Score * this, Score other) {

#line 37 "unit/operators/precedence.sp"
    return this->pts > other.pts;
}

#line 39 "unit/operators/precedence.sp"
_Bool Score__op_le(Score * this, Score other) {

#line 40 "unit/operators/precedence.sp"
    return this->pts <= other.pts;
}

#line 42 "unit/operators/precedence.sp"
_Bool Score__op_ge(Score * this, Score other) {

#line 43 "unit/operators/precedence.sp"
    return this->pts >= other.pts;
}

#line 46 "unit/operators/precedence.sp"
int main() {

#line 48 "unit/operators/precedence.sp"
    Num two;
    two.val = 2;

#line 49 "unit/operators/precedence.sp"
    Num three;
    three.val = 3;

#line 50 "unit/operators/precedence.sp"
    Num four;
    four.val = 4;

#line 51 "unit/operators/precedence.sp"
    Num ten;
    ten.val = 10;

#line 54 "unit/operators/precedence.sp"
    Num r1 = Num__op_add(&two, Num__op_mul(&three, four));

#line 55 "unit/operators/precedence.sp"
    if (r1.val != 14) 
        return 1;

#line 57 "unit/operators/precedence.sp"
    Num r2 = Num__op_add((__extension__(({ Num __sharp_bop0 = (Num__op_mul(&two, three)); &__sharp_bop0; }))), four);

#line 58 "unit/operators/precedence.sp"
    if (r2.val != 10) 
        return 2;

#line 60 "unit/operators/precedence.sp"
    Num r3 = Num__op_sub((__extension__(({ Num __sharp_bop1 = (Num__op_add(&ten, two)); &__sharp_bop1; }))), Num__op_mul(&three, four));

#line 61 "unit/operators/precedence.sp"
    if (r3.val != 0) 
        return 3;

#line 63 "unit/operators/precedence.sp"
    Num r4 = Num__op_div((__extension__(({ Num __sharp_bop2 = (Num__op_mul(&three, four)); &__sharp_bop2; }))), two);

#line 64 "unit/operators/precedence.sp"
    if (r4.val != 6) 
        return 4;

#line 67 "unit/operators/precedence.sp"
    Score s10;
    s10.pts = 10;

#line 68 "unit/operators/precedence.sp"
    Score s20;
    s20.pts = 20;

#line 69 "unit/operators/precedence.sp"
    Score s10b;
    s10b.pts = 10;

#line 71 "unit/operators/precedence.sp"
    if (!(Score__op_eq(&s10, s10b))) 
        return 5;

#line 72 "unit/operators/precedence.sp"
    if (Score__op_eq(&s10, s20)) 
        return 6;

#line 73 "unit/operators/precedence.sp"
    if (Score__op_eq(&s10, s10b)) {
    }
    else {
        return 7;
    }

#line 75 "unit/operators/precedence.sp"
    if (!(Score__op_lt(&s10, s20))) 
        return 8;

#line 76 "unit/operators/precedence.sp"
    if (Score__op_lt(&s20, s10)) 
        return 9;

#line 77 "unit/operators/precedence.sp"
    if (!(Score__op_gt(&s20, s10))) 
        return 10;

#line 78 "unit/operators/precedence.sp"
    if (Score__op_gt(&s10, s20)) 
        return 11;

#line 79 "unit/operators/precedence.sp"
    if (!(Score__op_le(&s10, s20))) 
        return 12;

#line 80 "unit/operators/precedence.sp"
    if (!(Score__op_le(&s10, s10b))) 
        return 13;

#line 81 "unit/operators/precedence.sp"
    if (Score__op_le(&s20, s10)) 
        return 14;

#line 82 "unit/operators/precedence.sp"
    if (!(Score__op_ge(&s20, s10))) 
        return 15;

#line 83 "unit/operators/precedence.sp"
    if (!(Score__op_ge(&s10, s10b))) 
        return 16;

#line 85 "unit/operators/precedence.sp"
    return 0;
}
