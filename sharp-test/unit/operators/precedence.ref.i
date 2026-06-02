
#line 7 "sharp-test/unit/operators/precedence.ce"
typedef struct Num Num;
struct Num {
    int val;
};

#line 10 "sharp-test/unit/operators/precedence.ce"
Num Num__op_add(Num * this, Num other);

#line 13 "sharp-test/unit/operators/precedence.ce"
Num Num__op_sub(Num * this, Num other);

#line 16 "sharp-test/unit/operators/precedence.ce"
Num Num__op_mul(Num * this, Num other);

#line 19 "sharp-test/unit/operators/precedence.ce"
Num Num__op_div(Num * this, Num other);

#line 22 "sharp-test/unit/operators/precedence.ce"
bool Num__op_eq(Num * this, Num other);

#line 27 "sharp-test/unit/operators/precedence.ce"
typedef struct Score Score;
struct Score {
    int pts;
};

#line 30 "sharp-test/unit/operators/precedence.ce"
bool Score__op_eq(Score * this, Score other);

#line 33 "sharp-test/unit/operators/precedence.ce"
bool Score__op_lt(Score * this, Score other);

#line 36 "sharp-test/unit/operators/precedence.ce"
bool Score__op_gt(Score * this, Score other);

#line 39 "sharp-test/unit/operators/precedence.ce"
bool Score__op_le(Score * this, Score other);

#line 42 "sharp-test/unit/operators/precedence.ce"
bool Score__op_ge(Score * this, Score other);

#line 10 "sharp-test/unit/operators/precedence.ce"
Num Num__op_add(Num * this, Num other) {

#line 11 "sharp-test/unit/operators/precedence.ce"
    Num r;
    r.val = this->val + other.val;
    return r;
}

#line 13 "sharp-test/unit/operators/precedence.ce"
Num Num__op_sub(Num * this, Num other) {

#line 14 "sharp-test/unit/operators/precedence.ce"
    Num r;
    r.val = this->val - other.val;
    return r;
}

#line 16 "sharp-test/unit/operators/precedence.ce"
Num Num__op_mul(Num * this, Num other) {

#line 17 "sharp-test/unit/operators/precedence.ce"
    Num r;
    r.val = this->val * other.val;
    return r;
}

#line 19 "sharp-test/unit/operators/precedence.ce"
Num Num__op_div(Num * this, Num other) {

#line 20 "sharp-test/unit/operators/precedence.ce"
    Num r;
    r.val = this->val / other.val;
    return r;
}

#line 22 "sharp-test/unit/operators/precedence.ce"
bool Num__op_eq(Num * this, Num other) {

#line 23 "sharp-test/unit/operators/precedence.ce"
    return this->val == other.val;
}

#line 30 "sharp-test/unit/operators/precedence.ce"
bool Score__op_eq(Score * this, Score other) {

#line 31 "sharp-test/unit/operators/precedence.ce"
    return this->pts == other.pts;
}

#line 33 "sharp-test/unit/operators/precedence.ce"
bool Score__op_lt(Score * this, Score other) {

#line 34 "sharp-test/unit/operators/precedence.ce"
    return this->pts < other.pts;
}

#line 36 "sharp-test/unit/operators/precedence.ce"
bool Score__op_gt(Score * this, Score other) {

#line 37 "sharp-test/unit/operators/precedence.ce"
    return this->pts > other.pts;
}

#line 39 "sharp-test/unit/operators/precedence.ce"
bool Score__op_le(Score * this, Score other) {

#line 40 "sharp-test/unit/operators/precedence.ce"
    return this->pts <= other.pts;
}

#line 42 "sharp-test/unit/operators/precedence.ce"
bool Score__op_ge(Score * this, Score other) {

#line 43 "sharp-test/unit/operators/precedence.ce"
    return this->pts >= other.pts;
}

#line 46 "sharp-test/unit/operators/precedence.ce"
int main() {

#line 48 "sharp-test/unit/operators/precedence.ce"
    Num two;
    two.val = 2;

#line 49 "sharp-test/unit/operators/precedence.ce"
    Num three;
    three.val = 3;

#line 50 "sharp-test/unit/operators/precedence.ce"
    Num four;
    four.val = 4;

#line 51 "sharp-test/unit/operators/precedence.ce"
    Num ten;
    ten.val = 10;

#line 54 "sharp-test/unit/operators/precedence.ce"
    Num r1 = Num__op_add(&two, Num__op_mul(&three, four));

#line 55 "sharp-test/unit/operators/precedence.ce"
    if (r1.val != 14) 
        return 1;

#line 57 "sharp-test/unit/operators/precedence.ce"
    Num r2 = Num__op_add((__extension__(({ Num __sharp_rv0 = (Num__op_mul(&two, three)); &__sharp_rv0; }))), four);

#line 58 "sharp-test/unit/operators/precedence.ce"
    if (r2.val != 10) 
        return 2;

#line 60 "sharp-test/unit/operators/precedence.ce"
    Num r3 = Num__op_sub((__extension__(({ Num __sharp_rv1 = (Num__op_add(&ten, two)); &__sharp_rv1; }))), Num__op_mul(&three, four));

#line 61 "sharp-test/unit/operators/precedence.ce"
    if (r3.val != 0) 
        return 3;

#line 63 "sharp-test/unit/operators/precedence.ce"
    Num r4 = Num__op_div((__extension__(({ Num __sharp_rv2 = (Num__op_mul(&three, four)); &__sharp_rv2; }))), two);

#line 64 "sharp-test/unit/operators/precedence.ce"
    if (r4.val != 6) 
        return 4;

#line 67 "sharp-test/unit/operators/precedence.ce"
    Score s10;
    s10.pts = 10;

#line 68 "sharp-test/unit/operators/precedence.ce"
    Score s20;
    s20.pts = 20;

#line 69 "sharp-test/unit/operators/precedence.ce"
    Score s10b;
    s10b.pts = 10;

#line 71 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_eq(&s10, s10b))) 
        return 5;

#line 72 "sharp-test/unit/operators/precedence.ce"
    if (Score__op_eq(&s10, s20)) 
        return 6;

#line 73 "sharp-test/unit/operators/precedence.ce"
    if (Score__op_eq(&s10, s10b)) {
    }
    else {
        return 7;
    }

#line 75 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_lt(&s10, s20))) 
        return 8;

#line 76 "sharp-test/unit/operators/precedence.ce"
    if (Score__op_lt(&s20, s10)) 
        return 9;

#line 77 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_gt(&s20, s10))) 
        return 10;

#line 78 "sharp-test/unit/operators/precedence.ce"
    if (Score__op_gt(&s10, s20)) 
        return 11;

#line 79 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_le(&s10, s20))) 
        return 12;

#line 80 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_le(&s10, s10b))) 
        return 13;

#line 81 "sharp-test/unit/operators/precedence.ce"
    if (Score__op_le(&s20, s10)) 
        return 14;

#line 82 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_ge(&s20, s10))) 
        return 15;

#line 83 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_ge(&s10, s10b))) 
        return 16;

#line 86 "sharp-test/unit/operators/precedence.ce"
    Num five;
    five.val = 5;

#line 87 "sharp-test/unit/operators/precedence.ce"
    Num three2;
    three2.val = 3;

#line 88 "sharp-test/unit/operators/precedence.ce"
    Num result = Num__op_add(&five, Num__op_mul(&three2, three2));

#line 89 "sharp-test/unit/operators/precedence.ce"
    if (result.val != 14) 
        return 17;

#line 90 "sharp-test/unit/operators/precedence.ce"
    if (!(Num__op_eq(&result, Num__op_add(&five, Num__op_mul(&three2, three2))))) 
        return 18;

#line 93 "sharp-test/unit/operators/precedence.ce"
    Score s5;
    s5.pts = 5;

#line 94 "sharp-test/unit/operators/precedence.ce"
    Score s15;
    s15.pts = 15;

#line 95 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_lt(&s5, s10) && Score__op_lt(&s10, s15))) 
        return 19;

#line 96 "sharp-test/unit/operators/precedence.ce"
    if (!(Score__op_lt(&s5, s10))) 
        return 20;

#line 98 "sharp-test/unit/operators/precedence.ce"
    return 0;
}
