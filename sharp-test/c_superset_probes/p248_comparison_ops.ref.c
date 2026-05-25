#include <stdbool.h>


#line 8 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
typedef struct Score Score;
struct Score {
    int pts;
};

#line 12 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_eq(Score * this, Score other);

#line 15 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_lt(Score * this, Score other);

#line 18 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_gt(Score * this, Score other);

#line 21 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_le(Score * this, Score other);

#line 24 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_ge(Score * this, Score other);

#line 12 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_eq(Score * this, Score other) {

#line 13 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return this->pts == other.pts;
}

#line 15 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_lt(Score * this, Score other) {

#line 16 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return this->pts < other.pts;
}

#line 18 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_gt(Score * this, Score other) {

#line 19 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return this->pts > other.pts;
}

#line 21 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_le(Score * this, Score other) {

#line 22 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return this->pts <= other.pts;
}

#line 24 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
_Bool Score__op_ge(Score * this, Score other) {

#line 25 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return this->pts >= other.pts;
}

#line 28 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
int main() {

#line 29 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    Score s10;
    s10.pts = 10;

#line 30 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    Score s20;
    s20.pts = 20;

#line 31 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    Score s10b;
    s10b.pts = 10;

#line 33 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_eq(&s10, s10b))) 
        return 1;

#line 34 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (Score__op_eq(&s10, s20)) 
        return 2;

#line 37 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (Score__op_eq(&s10, s10b)) {
    }
    else {
        return 3;
    }

#line 39 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_lt(&s10, s20))) 
        return 4;

#line 40 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (Score__op_lt(&s20, s10)) 
        return 5;

#line 42 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_gt(&s20, s10))) 
        return 6;

#line 43 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (Score__op_gt(&s10, s20)) 
        return 7;

#line 45 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_le(&s10, s20))) 
        return 8;

#line 46 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_le(&s10, s10b))) 
        return 9;

#line 47 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (Score__op_le(&s20, s10)) 
        return 10;

#line 49 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_ge(&s20, s10))) 
        return 11;

#line 50 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    if (!(Score__op_ge(&s10, s10b))) 
        return 12;

#line 52 "sharp-test/c_superset_probes/p248_comparison_ops.sp"
    return 0;
}
