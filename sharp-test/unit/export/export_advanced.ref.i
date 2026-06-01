
#line 5 "sharp-test/unit/export/export_advanced.ce"

#line 15 "sharp-test/unit/export/export_advanced.ce"

#line 16 "sharp-test/unit/export/export_advanced.ce"

#line 19 "sharp-test/unit/export/export_advanced.ce"

#line 27 "sharp-test/unit/export/export_advanced.ce"

#line 28 "sharp-test/unit/export/export_advanced.ce"

#line 31 "sharp-test/unit/export/export_advanced.ce"
typedef struct Builder Builder;
struct Builder {
    int x;
    int y;
};

#line 36 "sharp-test/unit/export/export_advanced.ce"
Builder * Builder__set_x(Builder * this, int v);

#line 37 "sharp-test/unit/export/export_advanced.ce"
Builder * Builder__set_y(Builder * this, int v);

#line 38 "sharp-test/unit/export/export_advanced.ce"
int Builder__sum(Builder * this);

#line 41 "sharp-test/unit/export/export_advanced.ce"
typedef struct Rect Rect;
struct Rect {
    int w;
    int h;
};

#line 46 "sharp-test/unit/export/export_advanced.ce"
int Rect__area(Rect * this);

#line 47 "sharp-test/unit/export/export_advanced.ce"
int Rect__perimeter(Rect * this);

#line 48 "sharp-test/unit/export/export_advanced.ce"
int Rect__double_area(Rect * this);

#line 51 "sharp-test/unit/export/export_advanced.ce"

#line 61 "sharp-test/unit/export/export_advanced.ce"

#line 62 "sharp-test/unit/export/export_advanced.ce"

#line 69 "sharp-test/unit/export/export_advanced.ce"

#line 70 "sharp-test/unit/export/export_advanced.ce"

#line 73 "sharp-test/unit/export/export_advanced.ce"

#line 80 "sharp-test/unit/export/export_advanced.ce"

#line 5 "sharp-test/unit/export/export_advanced.ce"
typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};


typedef struct Pair__long__double Pair__long__double;
struct Pair__long__double {
    long first;
    double second;
};



#line 19 "sharp-test/unit/export/export_advanced.ce"
typedef struct Box__int Box__int;
struct Box__int {
    int value;
};


typedef struct Box__float Box__float;
struct Box__float {
    float value;
};



#line 51 "sharp-test/unit/export/export_advanced.ce"
typedef struct Triple__int__int__int Triple__int__int__int;
struct Triple__int__int__int {
    int first;
    int second;
    int third;
};


typedef struct Triple__int__float__long Triple__int__float__long;
struct Triple__int__float__long {
    int first;
    float second;
    long third;
};



#line 73 "sharp-test/unit/export/export_advanced.ce"
typedef struct Holder__Pair__int__float Holder__Pair__int__float;
struct Holder__Pair__int__float {
    Pair__int__float inner;
};



#line 65 "sharp-test/unit/export/export_advanced.ce"
int first_of_three__int__float__long(int a, float b, long c);
long first_of_three__long__int__double(long a, int b, double c);

#line 12 "sharp-test/unit/export/export_advanced.ce"
void Pair__int__float__set_first(Pair__int__float * this, int v);

#line 13 "sharp-test/unit/export/export_advanced.ce"
void Pair__int__float__set_second(Pair__int__float * this, float v);

#line 10 "sharp-test/unit/export/export_advanced.ce"
int Pair__int__float__get_first(Pair__int__float * this);

#line 11 "sharp-test/unit/export/export_advanced.ce"
float Pair__int__float__get_second(Pair__int__float * this);

#line 12 "sharp-test/unit/export/export_advanced.ce"
void Pair__long__double__set_first(Pair__long__double * this, long v);

#line 13 "sharp-test/unit/export/export_advanced.ce"
void Pair__long__double__set_second(Pair__long__double * this, double v);

#line 10 "sharp-test/unit/export/export_advanced.ce"
long Pair__long__double__get_first(Pair__long__double * this);

#line 11 "sharp-test/unit/export/export_advanced.ce"
double Pair__long__double__get_second(Pair__long__double * this);

#line 24 "sharp-test/unit/export/export_advanced.ce"
void Box__int__set(Box__int * this, int v);

#line 23 "sharp-test/unit/export/export_advanced.ce"
int Box__int__get(Box__int * this);

#line 24 "sharp-test/unit/export/export_advanced.ce"
void Box__float__set(Box__float * this, float v);

#line 23 "sharp-test/unit/export/export_advanced.ce"
float Box__float__get(Box__float * this);

#line 57 "sharp-test/unit/export/export_advanced.ce"
int Triple__int__int__int__get_first(Triple__int__int__int * this);

#line 58 "sharp-test/unit/export/export_advanced.ce"
int Triple__int__int__int__get_second(Triple__int__int__int * this);

#line 59 "sharp-test/unit/export/export_advanced.ce"
int Triple__int__int__int__get_third(Triple__int__int__int * this);

#line 57 "sharp-test/unit/export/export_advanced.ce"
int Triple__int__float__long__get_first(Triple__int__float__long * this);

#line 58 "sharp-test/unit/export/export_advanced.ce"
float Triple__int__float__long__get_second(Triple__int__float__long * this);

#line 59 "sharp-test/unit/export/export_advanced.ce"
long Triple__int__float__long__get_third(Triple__int__float__long * this);

#line 78 "sharp-test/unit/export/export_advanced.ce"
void Holder__Pair__int__float__set(Holder__Pair__int__float * this, Pair__int__float v);

#line 77 "sharp-test/unit/export/export_advanced.ce"
Pair__int__float Holder__Pair__int__float__get(Holder__Pair__int__float * this);

#line 83 "sharp-test/unit/export/export_advanced.ce"
int Box__int__increment_and_get(Box__int * this);

#line 10 "sharp-test/unit/export/export_advanced.ce"

#line 11 "sharp-test/unit/export/export_advanced.ce"

#line 12 "sharp-test/unit/export/export_advanced.ce"

#line 13 "sharp-test/unit/export/export_advanced.ce"

#line 23 "sharp-test/unit/export/export_advanced.ce"

#line 24 "sharp-test/unit/export/export_advanced.ce"

#line 25 "sharp-test/unit/export/export_advanced.ce"

#line 36 "sharp-test/unit/export/export_advanced.ce"
Builder * Builder__set_x(Builder * this, int v) {
    this->x = v;
    return this;
}

#line 37 "sharp-test/unit/export/export_advanced.ce"
Builder * Builder__set_y(Builder * this, int v) {
    this->y = v;
    return this;
}

#line 38 "sharp-test/unit/export/export_advanced.ce"
int Builder__sum(Builder * this) {
    return this->x + this->y;
}

#line 46 "sharp-test/unit/export/export_advanced.ce"
int Rect__area(Rect * this) {
    return this->w * this->h;
}

#line 47 "sharp-test/unit/export/export_advanced.ce"
int Rect__perimeter(Rect * this) {
    return 2 * (this->w + this->h);
}

#line 48 "sharp-test/unit/export/export_advanced.ce"
int Rect__double_area(Rect * this) {
    return Rect__area(this) * 2;
}

#line 57 "sharp-test/unit/export/export_advanced.ce"

#line 58 "sharp-test/unit/export/export_advanced.ce"

#line 59 "sharp-test/unit/export/export_advanced.ce"

#line 65 "sharp-test/unit/export/export_advanced.ce"

#line 77 "sharp-test/unit/export/export_advanced.ce"

#line 78 "sharp-test/unit/export/export_advanced.ce"

#line 83 "sharp-test/unit/export/export_advanced.ce"

#line 88 "sharp-test/unit/export/export_advanced.ce"
int main(void) {

#line 90 "sharp-test/unit/export/export_advanced.ce"
    Pair__int__float p1;

#line 91 "sharp-test/unit/export/export_advanced.ce"
    Pair__int__float__set_first(&p1, 42);

#line 92 "sharp-test/unit/export/export_advanced.ce"
    Pair__int__float__set_second(&p1, 3.14f);

#line 93 "sharp-test/unit/export/export_advanced.ce"
    if (Pair__int__float__get_first(&p1) != 42) 
        return 1;

#line 94 "sharp-test/unit/export/export_advanced.ce"
    if (Pair__int__float__get_second(&p1) < 3.13f || Pair__int__float__get_second(&p1) > 3.15f) 
        return 2;

#line 96 "sharp-test/unit/export/export_advanced.ce"
    Pair__long__double p2;

#line 97 "sharp-test/unit/export/export_advanced.ce"
    Pair__long__double__set_first(&p2, 100L);

#line 98 "sharp-test/unit/export/export_advanced.ce"
    Pair__long__double__set_second(&p2, 2.718);

#line 99 "sharp-test/unit/export/export_advanced.ce"
    if (Pair__long__double__get_first(&p2) != 100L) 
        return 3;

#line 100 "sharp-test/unit/export/export_advanced.ce"
    if (Pair__long__double__get_second(&p2) < 2.717 || Pair__long__double__get_second(&p2) > 2.719) 
        return 4;

#line 103 "sharp-test/unit/export/export_advanced.ce"
    Box__int bi;

#line 104 "sharp-test/unit/export/export_advanced.ce"
    Box__int__set(&bi, 10);

#line 105 "sharp-test/unit/export/export_advanced.ce"
    if (Box__int__get(&bi) != 10) 
        return 5;

#line 107 "sharp-test/unit/export/export_advanced.ce"
    Box__float bf;

#line 108 "sharp-test/unit/export/export_advanced.ce"
    Box__float__set(&bf, 1.5f);

#line 109 "sharp-test/unit/export/export_advanced.ce"
    if (Box__float__get(&bf) < 1.49f || Box__float__get(&bf) > 1.51f) 
        return 6;

#line 112 "sharp-test/unit/export/export_advanced.ce"
    Builder b;

#line 113 "sharp-test/unit/export/export_advanced.ce"
    b.x = 0;
    b.y = 0;

#line 114 "sharp-test/unit/export/export_advanced.ce"
    Builder__set_y(Builder__set_x(&b, 3), 7);

#line 115 "sharp-test/unit/export/export_advanced.ce"
    if (Builder__sum(&b) != 10) 
        return 7;

#line 117 "sharp-test/unit/export/export_advanced.ce"
    Builder b2;

#line 118 "sharp-test/unit/export/export_advanced.ce"
    b2.x = 0;
    b2.y = 0;

#line 119 "sharp-test/unit/export/export_advanced.ce"
    int s = Builder__sum(Builder__set_y(Builder__set_x(&b2, 20), 30));

#line 120 "sharp-test/unit/export/export_advanced.ce"
    if (s != 50) 
        return 8;

#line 123 "sharp-test/unit/export/export_advanced.ce"
    Rect r;

#line 124 "sharp-test/unit/export/export_advanced.ce"
    r.w = 5;
    r.h = 6;

#line 125 "sharp-test/unit/export/export_advanced.ce"
    if (Rect__area(&r) != 30) 
        return 9;

#line 126 "sharp-test/unit/export/export_advanced.ce"
    if (Rect__perimeter(&r) != 22) 
        return 10;

#line 127 "sharp-test/unit/export/export_advanced.ce"
    if (Rect__double_area(&r) != 60) 
        return 11;

#line 129 "sharp-test/unit/export/export_advanced.ce"
    const Rect * cr = &r;

#line 130 "sharp-test/unit/export/export_advanced.ce"
    if (Rect__area(cr) != 30) 
        return 12;

#line 131 "sharp-test/unit/export/export_advanced.ce"
    if (Rect__perimeter(cr) != 22) 
        return 13;

#line 134 "sharp-test/unit/export/export_advanced.ce"
    Triple__int__int__int t1;

#line 135 "sharp-test/unit/export/export_advanced.ce"
    t1.first = 1;
    t1.second = 2;
    t1.third = 3;

#line 136 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__int__int__get_first(&t1) != 1) 
        return 14;

#line 137 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__int__int__get_second(&t1) != 2) 
        return 15;

#line 138 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__int__int__get_third(&t1) != 3) 
        return 16;

#line 140 "sharp-test/unit/export/export_advanced.ce"
    Triple__int__float__long t2;

#line 141 "sharp-test/unit/export/export_advanced.ce"
    t2.first = 10;
    t2.second = 2.5f;
    t2.third = 100L;

#line 142 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__float__long__get_first(&t2) != 10) 
        return 17;

#line 143 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__float__long__get_second(&t2) < 2.49f || Triple__int__float__long__get_second(&t2) > 2.51f) 
        return 18;

#line 144 "sharp-test/unit/export/export_advanced.ce"
    if (Triple__int__float__long__get_third(&t2) != 100L) 
        return 19;

#line 147 "sharp-test/unit/export/export_advanced.ce"
    int r1 = first_of_three__int__float__long(5, 3.0f, 100L);

#line 148 "sharp-test/unit/export/export_advanced.ce"
    if (r1 != 5) 
        return 20;

#line 150 "sharp-test/unit/export/export_advanced.ce"
    long r2 = first_of_three__long__int__double(10L, 20, 3.14);

#line 151 "sharp-test/unit/export/export_advanced.ce"
    if (r2 != 10L) 
        return 21;

#line 154 "sharp-test/unit/export/export_advanced.ce"
    Holder__Pair__int__float hp;

#line 155 "sharp-test/unit/export/export_advanced.ce"
    Pair__int__float inner;

#line 156 "sharp-test/unit/export/export_advanced.ce"
    inner.first = 99;

#line 157 "sharp-test/unit/export/export_advanced.ce"
    inner.second = 1.23f;

#line 158 "sharp-test/unit/export/export_advanced.ce"
    Holder__Pair__int__float__set(&hp, inner);

#line 159 "sharp-test/unit/export/export_advanced.ce"
    Pair__int__float got = Holder__Pair__int__float__get(&hp);

#line 160 "sharp-test/unit/export/export_advanced.ce"
    if (got.first != 99) 
        return 22;

#line 161 "sharp-test/unit/export/export_advanced.ce"
    if (got.second < 1.22f || got.second > 1.24f) 
        return 23;

#line 164 "sharp-test/unit/export/export_advanced.ce"
    Box__int bx;

#line 165 "sharp-test/unit/export/export_advanced.ce"
    Box__int__set(&bx, 5);

#line 166 "sharp-test/unit/export/export_advanced.ce"
    int val = Box__int__increment_and_get(&bx);

#line 167 "sharp-test/unit/export/export_advanced.ce"
    if (val != 6) 
        return 24;

#line 168 "sharp-test/unit/export/export_advanced.ce"
    if (Box__int__get(&bx) != 6) 
        return 25;

#line 170 "sharp-test/unit/export/export_advanced.ce"
    return 0;
}

#line 65 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int first_of_three__int__float__long(int a, float b, long c) {

#line 66 "sharp-test/unit/export/export_advanced.ce"
    return a;
}


#line 65 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) long first_of_three__long__int__double(long a, int b, double c) {

#line 66 "sharp-test/unit/export/export_advanced.ce"
    return a;
}


#line 12 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Pair__int__float__set_first(Pair__int__float * this, int v) {
    this->first = v;
}


#line 13 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Pair__int__float__set_second(Pair__int__float * this, float v) {
    this->second = v;
}


#line 10 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Pair__int__float__get_first(Pair__int__float * this) {
    return this->first;
}


#line 11 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) float Pair__int__float__get_second(Pair__int__float * this) {
    return this->second;
}


#line 12 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Pair__long__double__set_first(Pair__long__double * this, long v) {
    this->first = v;
}


#line 13 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Pair__long__double__set_second(Pair__long__double * this, double v) {
    this->second = v;
}


#line 10 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) long Pair__long__double__get_first(Pair__long__double * this) {
    return this->first;
}


#line 11 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) double Pair__long__double__get_second(Pair__long__double * this) {
    return this->second;
}


#line 24 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Box__int__set(Box__int * this, int v) {
    this->value = v;
}


#line 23 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Box__int__get(Box__int * this) {
    return this->value;
}


#line 24 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Box__float__set(Box__float * this, float v) {
    this->value = v;
}


#line 23 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) float Box__float__get(Box__float * this) {
    return this->value;
}


#line 57 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_first(Triple__int__int__int * this) {
    return this->first;
}


#line 58 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_second(Triple__int__int__int * this) {
    return this->second;
}


#line 59 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_third(Triple__int__int__int * this) {
    return this->third;
}


#line 57 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Triple__int__float__long__get_first(Triple__int__float__long * this) {
    return this->first;
}


#line 58 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) float Triple__int__float__long__get_second(Triple__int__float__long * this) {
    return this->second;
}


#line 59 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) long Triple__int__float__long__get_third(Triple__int__float__long * this) {
    return this->third;
}


#line 78 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) void Holder__Pair__int__float__set(Holder__Pair__int__float * this, Pair__int__float v) {
    this->inner = v;
}


#line 77 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) Pair__int__float Holder__Pair__int__float__get(Holder__Pair__int__float * this) {
    return this->inner;
}


#line 83 "sharp-test/unit/export/export_advanced.ce"
__attribute__((weak)) int Box__int__increment_and_get(Box__int * this) {

#line 84 "sharp-test/unit/export/export_advanced.ce"
    this->value = this->value + 1;

#line 85 "sharp-test/unit/export/export_advanced.ce"
    return this->value;
}

