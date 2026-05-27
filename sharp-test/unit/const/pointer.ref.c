
#include <stdlib.h>

#line 6 "unit/const/pointer.sp"
typedef struct ConstDemo ConstDemo;
struct ConstDemo {
    const int * a;
    int * const b;
    int * c;
};

#line 12 "unit/const/pointer.sp"
int ConstDemo__read_a(ConstDemo * this);

#line 16 "unit/const/pointer.sp"
void ConstDemo__set_a(ConstDemo * this, const int * p);

#line 19 "unit/const/pointer.sp"
int ConstDemo__read_c(ConstDemo * this);

#line 12 "unit/const/pointer.sp"
int ConstDemo__read_a(ConstDemo * this) {

#line 13 "unit/const/pointer.sp"
    if (this->a) 
        return *this->a;

#line 14 "unit/const/pointer.sp"
    return -1;
}

#line 16 "unit/const/pointer.sp"
void ConstDemo__set_a(ConstDemo * this, const int * p) {

#line 17 "unit/const/pointer.sp"
    this->a = p;
}

#line 19 "unit/const/pointer.sp"
int ConstDemo__read_c(ConstDemo * this) {

#line 20 "unit/const/pointer.sp"
    if (this->c) 
        return *this->c;

#line 21 "unit/const/pointer.sp"
    return -1;
}

#line 25 "unit/const/pointer.sp"
int mix_const(const int * a, int * const b, int * c) {

#line 26 "unit/const/pointer.sp"
    *b = *a;

#line 27 "unit/const/pointer.sp"
    *c = *b + 1;

#line 28 "unit/const/pointer.sp"
    return *c;
}

#line 31 "unit/const/pointer.sp"
int main() {

#line 33 "unit/const/pointer.sp"
    ConstDemo d;

#line 34 "unit/const/pointer.sp"
    int x = 42;

#line 35 "unit/const/pointer.sp"
    d.a = &x;

#line 36 "unit/const/pointer.sp"
    d.c = &x;

#line 37 "unit/const/pointer.sp"
    int v1 = ConstDemo__read_a(&d);

#line 38 "unit/const/pointer.sp"
    if (v1 != 42) 
        return 1;

#line 40 "unit/const/pointer.sp"
    int y = 99;

#line 41 "unit/const/pointer.sp"
    ConstDemo__set_a(&d, &y);

#line 42 "unit/const/pointer.sp"
    int v2 = ConstDemo__read_a(&d);

#line 43 "unit/const/pointer.sp"
    if (v2 != 99) 
        return 2;

#line 45 "unit/const/pointer.sp"
    int v3 = ConstDemo__read_c(&d);

#line 46 "unit/const/pointer.sp"
    if (v3 != 42) 
        return 3;
    int v = 10, w = 0, z = 0;

#line 50 "unit/const/pointer.sp"
    int r = mix_const(&v, &w, &z);

#line 51 "unit/const/pointer.sp"
    if (r != 11) 
        return 4;

#line 52 "unit/const/pointer.sp"
    if (w != 10) 
        return 5;

#line 53 "unit/const/pointer.sp"
    if (z != 11) 
        return 6;

#line 56 "unit/const/pointer.sp"
    ConstDemo * const pd = &d;

#line 57 "unit/const/pointer.sp"
    int v4 = ConstDemo__read_a(pd);

#line 58 "unit/const/pointer.sp"
    if (v4 != 99) 
        return 7;

#line 60 "unit/const/pointer.sp"
    return 0;
}
