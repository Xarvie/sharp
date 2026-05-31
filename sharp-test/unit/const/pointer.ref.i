
#line 6 "sharp-test/unit/const/pointer.ce"
typedef struct ConstDemo ConstDemo;
struct ConstDemo {
    const int * a;
    int * const b;
    int * c;
};

#line 12 "sharp-test/unit/const/pointer.ce"
int ConstDemo__read_a(ConstDemo * this);

#line 16 "sharp-test/unit/const/pointer.ce"
void ConstDemo__set_a(ConstDemo * this, const int * p);

#line 19 "sharp-test/unit/const/pointer.ce"
int ConstDemo__read_c(ConstDemo * this);

#line 12 "sharp-test/unit/const/pointer.ce"
int ConstDemo__read_a(ConstDemo * this) {

#line 13 "sharp-test/unit/const/pointer.ce"
    if (this->a) 
        return *this->a;

#line 14 "sharp-test/unit/const/pointer.ce"
    return -1;
}

#line 16 "sharp-test/unit/const/pointer.ce"
void ConstDemo__set_a(ConstDemo * this, const int * p) {

#line 17 "sharp-test/unit/const/pointer.ce"
    this->a = p;
}

#line 19 "sharp-test/unit/const/pointer.ce"
int ConstDemo__read_c(ConstDemo * this) {

#line 20 "sharp-test/unit/const/pointer.ce"
    if (this->c) 
        return *this->c;

#line 21 "sharp-test/unit/const/pointer.ce"
    return -1;
}

#line 25 "sharp-test/unit/const/pointer.ce"
int mix_const(const int * a, int * const b, int * c) {

#line 26 "sharp-test/unit/const/pointer.ce"
    *b = *a;

#line 27 "sharp-test/unit/const/pointer.ce"
    *c = *b + 1;

#line 28 "sharp-test/unit/const/pointer.ce"
    return *c;
}

#line 31 "sharp-test/unit/const/pointer.ce"
int main() {

#line 33 "sharp-test/unit/const/pointer.ce"
    ConstDemo d;

#line 34 "sharp-test/unit/const/pointer.ce"
    int x = 42;

#line 35 "sharp-test/unit/const/pointer.ce"
    d.a = &x;

#line 36 "sharp-test/unit/const/pointer.ce"
    d.c = &x;

#line 37 "sharp-test/unit/const/pointer.ce"
    int v1 = ConstDemo__read_a(&d);

#line 38 "sharp-test/unit/const/pointer.ce"
    if (v1 != 42) 
        return 1;

#line 40 "sharp-test/unit/const/pointer.ce"
    int y = 99;

#line 41 "sharp-test/unit/const/pointer.ce"
    ConstDemo__set_a(&d, &y);

#line 42 "sharp-test/unit/const/pointer.ce"
    int v2 = ConstDemo__read_a(&d);

#line 43 "sharp-test/unit/const/pointer.ce"
    if (v2 != 99) 
        return 2;

#line 45 "sharp-test/unit/const/pointer.ce"
    int v3 = ConstDemo__read_c(&d);

#line 46 "sharp-test/unit/const/pointer.ce"
    if (v3 != 42) 
        return 3;
    int v = 10, w = 0, z = 0;

#line 50 "sharp-test/unit/const/pointer.ce"
    int r = mix_const(&v, &w, &z);

#line 51 "sharp-test/unit/const/pointer.ce"
    if (r != 11) 
        return 4;

#line 52 "sharp-test/unit/const/pointer.ce"
    if (w != 10) 
        return 5;

#line 53 "sharp-test/unit/const/pointer.ce"
    if (z != 11) 
        return 6;

#line 56 "sharp-test/unit/const/pointer.ce"
    ConstDemo * const pd = &d;

#line 57 "sharp-test/unit/const/pointer.ce"
    int v4 = ConstDemo__read_a(pd);

#line 58 "sharp-test/unit/const/pointer.ce"
    if (v4 != 99) 
        return 7;

#line 61 "sharp-test/unit/const/pointer.ce"
    const ConstDemo * const pdc = &d;

#line 62 "sharp-test/unit/const/pointer.ce"
    int v5 = ConstDemo__read_a(pdc);

#line 63 "sharp-test/unit/const/pointer.ce"
    if (v5 != 99) 
        return 8;

#line 64 "sharp-test/unit/const/pointer.ce"
    int v6 = ConstDemo__read_c(pdc);

#line 65 "sharp-test/unit/const/pointer.ce"
    if (v6 != 42) 
        return 9;

#line 68 "sharp-test/unit/const/pointer.ce"
    const ConstDemo * rp = &d;

#line 69 "sharp-test/unit/const/pointer.ce"
    if (ConstDemo__read_a(rp) != 99) 
        return 10;

#line 70 "sharp-test/unit/const/pointer.ce"
    if (ConstDemo__read_c(rp) != 42) 
        return 11;

#line 72 "sharp-test/unit/const/pointer.ce"
    return 0;
}
