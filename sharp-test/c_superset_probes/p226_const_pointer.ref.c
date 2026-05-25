

#line 10 "sharp-test/c_superset_probes/p226_const_pointer.sp"
typedef struct ConstDemo ConstDemo;
struct ConstDemo {
    const int * a;
    int * const b;
    int * c;
};

#line 17 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int ConstDemo__read_a(ConstDemo * this);

#line 23 "sharp-test/c_superset_probes/p226_const_pointer.sp"
void ConstDemo__set_a(ConstDemo * this, const int * p);

#line 28 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int ConstDemo__read_c(ConstDemo * this);

#line 17 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int ConstDemo__read_a(ConstDemo * this) {

#line 18 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (this->a) 
        return *this->a;

#line 19 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    return -1;
}

#line 23 "sharp-test/c_superset_probes/p226_const_pointer.sp"
void ConstDemo__set_a(ConstDemo * this, const int * p) {

#line 24 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    this->a = p;
}

#line 28 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int ConstDemo__read_c(ConstDemo * this) {

#line 29 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (this->c) 
        return *this->c;

#line 30 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    return -1;
}

#line 34 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int mix_const(const int * a, int * const b, int * c) {

#line 35 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    *b = *a;

#line 36 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    *c = *b + 1;

#line 37 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    return *c;
}

#line 40 "sharp-test/c_superset_probes/p226_const_pointer.sp"
int main() {

#line 42 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    ConstDemo d;

#line 43 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int x = 42;

#line 44 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    d.a = &x;

#line 45 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    d.c = &x;

#line 46 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int v1 = ConstDemo__read_a(&d);

#line 47 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (v1 != 42) 
        return 1;

#line 50 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int y = 99;

#line 51 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    ConstDemo__set_a(&d, &y);

#line 52 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int v2 = ConstDemo__read_a(&d);

#line 53 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (v2 != 99) 
        return 2;

#line 56 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int v3 = ConstDemo__read_c(&d);

#line 57 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (v3 != 42) 
        return 3;
    int v = 10, w = 0, z = 0;

#line 61 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int r = mix_const(&v, &w, &z);

#line 62 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (r != 11) 
        return 4;

#line 63 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (w != 10) 
        return 5;

#line 64 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (z != 11) 
        return 6;

#line 67 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    ConstDemo * const pd = &d;

#line 68 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    int v4 = ConstDemo__read_a(pd);

#line 69 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    if (v4 != 99) 
        return 7;

#line 71 "sharp-test/c_superset_probes/p226_const_pointer.sp"
    return 0;
}
