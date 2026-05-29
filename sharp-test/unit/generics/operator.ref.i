
#line 8 "sharp-test/unit/generics/operator.ce"

#line 27 "sharp-test/unit/generics/operator.ce"

#line 45 "sharp-test/unit/generics/operator.ce"

#line 27 "sharp-test/unit/generics/operator.ce"
typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};



#line 8 "sharp-test/unit/generics/operator.ce"
typedef struct Vec2__int Vec2__int;
struct Vec2__int {
    int x;
    int y;
};



#line 45 "sharp-test/unit/generics/operator.ce"
typedef struct Arr__int Arr__int;
struct Arr__int {
    int * data;
    long len;
    long cap;
};



#line 32 "sharp-test/unit/generics/operator.ce"
int Pair__int__float__first_val(Pair__int__float * this);

#line 13 "sharp-test/unit/generics/operator.ce"
_Bool Vec2__int__equals(Vec2__int * this, Vec2__int other);

#line 17 "sharp-test/unit/generics/operator.ce"
void Vec2__int__add_into(Vec2__int * this, Vec2__int other);

#line 22 "sharp-test/unit/generics/operator.ce"
int Vec2__int__sum(Vec2__int * this);

#line 40 "sharp-test/unit/generics/operator.ce"
void Pair__int__float__set_first(Pair__int__float * this, int val);

#line 36 "sharp-test/unit/generics/operator.ce"
float Pair__int__float__second_val(Pair__int__float * this);

#line 59 "sharp-test/unit/generics/operator.ce"
void Arr__int__push(Arr__int * this, int item);

#line 51 "sharp-test/unit/generics/operator.ce"
long Arr__int__size(Arr__int * this);

#line 55 "sharp-test/unit/generics/operator.ce"
int Arr__int__at(Arr__int * this, long i);

#line 65 "sharp-test/unit/generics/operator.ce"
void first__cint(const int * a, const int * b);
void first__float(float * a, float * b);

#line 13 "sharp-test/unit/generics/operator.ce"

#line 17 "sharp-test/unit/generics/operator.ce"

#line 22 "sharp-test/unit/generics/operator.ce"

#line 32 "sharp-test/unit/generics/operator.ce"

#line 36 "sharp-test/unit/generics/operator.ce"

#line 40 "sharp-test/unit/generics/operator.ce"

#line 51 "sharp-test/unit/generics/operator.ce"

#line 55 "sharp-test/unit/generics/operator.ce"

#line 59 "sharp-test/unit/generics/operator.ce"

#line 65 "sharp-test/unit/generics/operator.ce"

#line 70 "sharp-test/unit/generics/operator.ce"
int read_pair(const Pair__int__float * p) {

#line 71 "sharp-test/unit/generics/operator.ce"
    return Pair__int__float__first_val(p);
}

#line 74 "sharp-test/unit/generics/operator.ce"
int main() {

#line 76 "sharp-test/unit/generics/operator.ce"
    Vec2__int a;

#line 77 "sharp-test/unit/generics/operator.ce"
    a.x = 1;
    a.y = 2;

#line 78 "sharp-test/unit/generics/operator.ce"
    Vec2__int b;

#line 79 "sharp-test/unit/generics/operator.ce"
    b.x = 1;
    b.y = 2;

#line 80 "sharp-test/unit/generics/operator.ce"
    Vec2__int c;

#line 81 "sharp-test/unit/generics/operator.ce"
    c.x = 3;
    c.y = 4;

#line 83 "sharp-test/unit/generics/operator.ce"
    if (!Vec2__int__equals(&a, b)) 
        return 1;

#line 84 "sharp-test/unit/generics/operator.ce"
    if (Vec2__int__equals(&a, c)) 
        return 2;

#line 86 "sharp-test/unit/generics/operator.ce"
    Vec2__int__add_into(&a, c);

#line 87 "sharp-test/unit/generics/operator.ce"
    if (a.x != 4) 
        return 3;

#line 88 "sharp-test/unit/generics/operator.ce"
    if (a.y != 6) 
        return 4;

#line 90 "sharp-test/unit/generics/operator.ce"
    if (Vec2__int__sum(&a) != 10) 
        return 5;

#line 93 "sharp-test/unit/generics/operator.ce"
    Pair__int__float p = { 0 };

#line 94 "sharp-test/unit/generics/operator.ce"
    Pair__int__float__set_first(&p, 42);

#line 95 "sharp-test/unit/generics/operator.ce"
    p.second = 3.14f;

#line 97 "sharp-test/unit/generics/operator.ce"
    if (Pair__int__float__first_val(&p) != 42) 
        return 6;

#line 98 "sharp-test/unit/generics/operator.ce"
    if (Pair__int__float__second_val(&p) < 3.13f || Pair__int__float__second_val(&p) > 3.15f) 
        return 7;

#line 100 "sharp-test/unit/generics/operator.ce"
    int r = read_pair(&p);

#line 101 "sharp-test/unit/generics/operator.ce"
    if (r != 42) 
        return 8;

#line 104 "sharp-test/unit/generics/operator.ce"
    int arr[5];

#line 105 "sharp-test/unit/generics/operator.ce"
    Arr__int v = { arr, 0, 5 };

#line 106 "sharp-test/unit/generics/operator.ce"
    Arr__int__push(&v, 10);

#line 107 "sharp-test/unit/generics/operator.ce"
    Arr__int__push(&v, 20);

#line 108 "sharp-test/unit/generics/operator.ce"
    Arr__int__push(&v, 30);

#line 109 "sharp-test/unit/generics/operator.ce"
    long s = Arr__int__size(&v);

#line 110 "sharp-test/unit/generics/operator.ce"
    int val = Arr__int__at(&v, 1);

#line 111 "sharp-test/unit/generics/operator.ce"
    if (val + (int)s - 23 != 0) 
        return 9;
    int ia = 3, ib = 4;

#line 115 "sharp-test/unit/generics/operator.ce"
    first__cint(&ia, &ib);
    float fa = 1.0f, fb = 2.0f;

#line 118 "sharp-test/unit/generics/operator.ce"
    first__float(&fa, &fb);

#line 120 "sharp-test/unit/generics/operator.ce"
    return 0;
}

#line 32 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) int Pair__int__float__first_val(Pair__int__float * this) {

#line 33 "sharp-test/unit/generics/operator.ce"
    return this->first;
}


#line 13 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) _Bool Vec2__int__equals(Vec2__int * this, Vec2__int other) {

#line 14 "sharp-test/unit/generics/operator.ce"
    return this->x == other.x && this->y == other.y;
}


#line 17 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) void Vec2__int__add_into(Vec2__int * this, Vec2__int other) {

#line 18 "sharp-test/unit/generics/operator.ce"
    this->x = this->x + other.x;

#line 19 "sharp-test/unit/generics/operator.ce"
    this->y = this->y + other.y;
}


#line 22 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) int Vec2__int__sum(Vec2__int * this) {

#line 23 "sharp-test/unit/generics/operator.ce"
    return this->x + this->y;
}


#line 40 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) void Pair__int__float__set_first(Pair__int__float * this, int val) {

#line 41 "sharp-test/unit/generics/operator.ce"
    this->first = val;
}


#line 36 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) float Pair__int__float__second_val(Pair__int__float * this) {

#line 37 "sharp-test/unit/generics/operator.ce"
    return this->second;
}


#line 59 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) void Arr__int__push(Arr__int * this, int item) {

#line 60 "sharp-test/unit/generics/operator.ce"
    this->data[this->len] = item;

#line 61 "sharp-test/unit/generics/operator.ce"
    this->len = this->len + 1;
}


#line 51 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) long Arr__int__size(Arr__int * this) {

#line 52 "sharp-test/unit/generics/operator.ce"
    return this->len;
}


#line 55 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) int Arr__int__at(Arr__int * this, long i) {

#line 56 "sharp-test/unit/generics/operator.ce"
    return this->data[i];
}


#line 65 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) void first__cint(const int * a, const int * b) {

#line 66 "sharp-test/unit/generics/operator.ce"
    (void)a;
    (void)b;
}


#line 65 "sharp-test/unit/generics/operator.ce"
__attribute__((weak)) void first__float(float * a, float * b) {

#line 66 "sharp-test/unit/generics/operator.ce"
    (void)a;
    (void)b;
}

