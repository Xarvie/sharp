

#line 17 "sharp-test/c_superset_probes/p231_auto_complex.sp"
typedef struct Data Data;
struct Data {
    int value;
};

#line 21 "sharp-test/c_superset_probes/p231_auto_complex.sp"
int Data__read(Data * this);

#line 22 "sharp-test/c_superset_probes/p231_auto_complex.sp"
void Data__init(Data * this, int v);

#line 13 "sharp-test/c_superset_probes/p231_auto_complex.sp"
int get_int() {
    return 42;
}

#line 14 "sharp-test/c_superset_probes/p231_auto_complex.sp"
float get_float() {
    return 3.14f;
}

#line 21 "sharp-test/c_superset_probes/p231_auto_complex.sp"
int Data__read(Data * this) {
    return this->value;
}

#line 22 "sharp-test/c_superset_probes/p231_auto_complex.sp"
void Data__init(Data * this, int v) {
    this->value = v;
}

#line 24 "sharp-test/c_superset_probes/p231_auto_complex.sp"
int main() {

#line 26 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    int i1 = 1 + 2;

#line 27 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (i1 != 3) 
        return 1;

#line 29 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    double f1 = 1.0f + 2.0f;

#line 30 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (f1 < 2.9f || f1 > 3.1f) 
        return 2;

#line 33 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    double m1 = 1 + 2.5f;

#line 35 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (m1 < 3.4f || m1 > 3.6f) 
        return 3;

#line 38 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    int r1 = get_int();

#line 39 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (r1 != 42) 
        return 4;

#line 41 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    float r2 = get_float();

#line 42 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (r2 < 3.13f || r2 > 3.15f) 
        return 5;

#line 45 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    int t1 = 1 ? 10 : 20;

#line 46 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (t1 != 10) 
        return 6;

#line 49 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    Data d;

#line 50 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    Data__init(&d, 77);

#line 51 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    int v1 = Data__read(&d);

#line 52 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    if (v1 != 77) 
        return 7;

#line 54 "sharp-test/c_superset_probes/p231_auto_complex.sp"
    return 0;
}
