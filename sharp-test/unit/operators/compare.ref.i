
#line 4 "sharp-test/unit/operators/compare.ce"
typedef struct Score Score;
struct Score {
    int pts;
};

#line 8 "sharp-test/unit/operators/compare.ce"
bool Score__op_eq(Score * this, Score other);

#line 11 "sharp-test/unit/operators/compare.ce"
bool Score__op_ne(Score * this, Score other);

#line 14 "sharp-test/unit/operators/compare.ce"
bool Score__op_lt(Score * this, Score other);

#line 17 "sharp-test/unit/operators/compare.ce"
bool Score__op_gt(Score * this, Score other);

#line 20 "sharp-test/unit/operators/compare.ce"
bool Score__op_le(Score * this, Score other);

#line 23 "sharp-test/unit/operators/compare.ce"
bool Score__op_ge(Score * this, Score other);

#line 8 "sharp-test/unit/operators/compare.ce"
bool Score__op_eq(Score * this, Score other) {

#line 9 "sharp-test/unit/operators/compare.ce"
    return this->pts == other.pts;
}

#line 11 "sharp-test/unit/operators/compare.ce"
bool Score__op_ne(Score * this, Score other) {

#line 12 "sharp-test/unit/operators/compare.ce"
    return this->pts != other.pts;
}

#line 14 "sharp-test/unit/operators/compare.ce"
bool Score__op_lt(Score * this, Score other) {

#line 15 "sharp-test/unit/operators/compare.ce"
    return this->pts < other.pts;
}

#line 17 "sharp-test/unit/operators/compare.ce"
bool Score__op_gt(Score * this, Score other) {

#line 18 "sharp-test/unit/operators/compare.ce"
    return this->pts > other.pts;
}

#line 20 "sharp-test/unit/operators/compare.ce"
bool Score__op_le(Score * this, Score other) {

#line 21 "sharp-test/unit/operators/compare.ce"
    return this->pts <= other.pts;
}

#line 23 "sharp-test/unit/operators/compare.ce"
bool Score__op_ge(Score * this, Score other) {

#line 24 "sharp-test/unit/operators/compare.ce"
    return this->pts >= other.pts;
}

#line 27 "sharp-test/unit/operators/compare.ce"
int main() {

#line 29 "sharp-test/unit/operators/compare.ce"
    Score a;
    a.pts = 10;

#line 30 "sharp-test/unit/operators/compare.ce"
    Score b;
    b.pts = 10;

#line 31 "sharp-test/unit/operators/compare.ce"
    Score c;
    c.pts = 20;

#line 32 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_eq(&a, b))) 
        return 1;

#line 33 "sharp-test/unit/operators/compare.ce"
    if (Score__op_ne(&a, b)) 
        return 2;

#line 34 "sharp-test/unit/operators/compare.ce"
    if (Score__op_eq(&a, c)) 
        return 3;

#line 35 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_ne(&a, c))) 
        return 4;

#line 38 "sharp-test/unit/operators/compare.ce"
    if (Score__op_lt(&a, b)) 
        return 5;

#line 39 "sharp-test/unit/operators/compare.ce"
    if (Score__op_gt(&a, b)) 
        return 6;

#line 40 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_lt(&a, c))) 
        return 7;

#line 41 "sharp-test/unit/operators/compare.ce"
    if (Score__op_gt(&a, c)) 
        return 8;

#line 44 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_le(&a, b))) 
        return 9;

#line 45 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_ge(&a, b))) 
        return 10;

#line 46 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_le(&a, c))) 
        return 11;

#line 47 "sharp-test/unit/operators/compare.ce"
    if (Score__op_ge(&a, c)) 
        return 12;

#line 50 "sharp-test/unit/operators/compare.ce"
    Score lo;
    lo.pts = 5;

#line 51 "sharp-test/unit/operators/compare.ce"
    Score mid;
    mid.pts = 10;

#line 52 "sharp-test/unit/operators/compare.ce"
    Score hi;
    hi.pts = 15;

#line 53 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_lt(&lo, mid) && Score__op_lt(&mid, hi))) 
        return 13;

#line 54 "sharp-test/unit/operators/compare.ce"
    if (!(Score__op_lt(&lo, mid) && Score__op_le(&mid, hi))) 
        return 14;

#line 56 "sharp-test/unit/operators/compare.ce"
    return 0;
}
