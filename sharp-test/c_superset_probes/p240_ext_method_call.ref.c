

#line 3 "sharp-test/c_superset_probes/../../std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../std/types.sph"
typedef unsigned long usize;

#line 10 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
typedef struct Stats Stats;
struct Stats {
    int count;
    float sum;
};

#line 15 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
void Stats__record(Stats * this, float v);

#line 20 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
void Stats__record_many(Stats * this, float * vals, isize n);

#line 28 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
int Stats__count_val(Stats * this);

#line 29 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
float Stats__avg(Stats * this);

#line 15 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
void Stats__record(Stats * this, float v) {

#line 16 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    this->count = this->count + 1;

#line 17 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    this->sum = this->sum + v;
}

#line 20 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
void Stats__record_many(Stats * this, float * vals, isize n) {

#line 21 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    isize i = 0;

#line 22 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    while (i < n) {

#line 23 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
        Stats__record(this, vals[i]);

#line 24 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
        i = i + 1;
    }
}

#line 28 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
int Stats__count_val(Stats * this) {
    return this->count;
}

#line 29 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
float Stats__avg(Stats * this) {

#line 30 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    if (this->count == 0) 
        return 0.0f;

#line 31 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    return this->sum / (float)this->count;
}

#line 34 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
int main() {

#line 35 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    Stats s = { 0 };

#line 37 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    float vals[4];

#line 38 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    vals[0] = 10.0f;

#line 39 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    vals[1] = 20.0f;

#line 40 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    vals[2] = 30.0f;

#line 41 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    vals[3] = 40.0f;

#line 43 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    Stats__record_many(&s, vals, 4);

#line 45 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    if (Stats__count_val(&s) != 4) 
        return 1;

#line 46 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    float a = Stats__avg(&s);

#line 47 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    if (a < 24.9f || a > 25.1f) 
        return 2;

#line 49 "sharp-test/c_superset_probes/p240_ext_method_call.sp"
    return 0;
}
