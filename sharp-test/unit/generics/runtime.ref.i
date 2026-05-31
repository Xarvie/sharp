
#line 4 "sharp-test/unit/generics/runtime.ce"
typedef long isize;

#line 5 "sharp-test/unit/generics/runtime.ce"
typedef unsigned long usize;

#line 8 "sharp-test/unit/generics/runtime.ce"

#line 20 "sharp-test/unit/generics/runtime.ce"

#line 8 "sharp-test/unit/generics/runtime.ce"
typedef struct Pair__int Pair__int;
struct Pair__int {
    int first;
    int second;
};


typedef struct Pair__float Pair__float;
struct Pair__float {
    float first;
    float second;
};



#line 20 "sharp-test/unit/generics/runtime.ce"
typedef struct Wrap__int Wrap__int;
struct Wrap__int {
    int value;
};


typedef struct Wrap__float Wrap__float;
struct Wrap__float {
    float value;
};



#line 13 "sharp-test/unit/generics/runtime.ce"
void Pair__int__swap(Pair__int * this);
void Pair__float__swap(Pair__float * this);

#line 24 "sharp-test/unit/generics/runtime.ce"
void Wrap__int__set(Wrap__int * this, int v);

#line 25 "sharp-test/unit/generics/runtime.ce"
int Wrap__int__get(Wrap__int * this);

#line 24 "sharp-test/unit/generics/runtime.ce"
void Wrap__float__set(Wrap__float * this, float v);

#line 25 "sharp-test/unit/generics/runtime.ce"
float Wrap__float__get(Wrap__float * this);

#line 27 "sharp-test/unit/generics/runtime.ce"
Pair__int Pair__int__reversed(Pair__int * this);

#line 13 "sharp-test/unit/generics/runtime.ce"

#line 24 "sharp-test/unit/generics/runtime.ce"

#line 25 "sharp-test/unit/generics/runtime.ce"

#line 27 "sharp-test/unit/generics/runtime.ce"

#line 34 "sharp-test/unit/generics/runtime.ce"
int main() {

#line 36 "sharp-test/unit/generics/runtime.ce"
    Pair__int p;

#line 37 "sharp-test/unit/generics/runtime.ce"
    p.first = 10;

#line 38 "sharp-test/unit/generics/runtime.ce"
    p.second = 20;

#line 40 "sharp-test/unit/generics/runtime.ce"
    if (p.first != 10) 
        return 1;

#line 41 "sharp-test/unit/generics/runtime.ce"
    if (p.second != 20) 
        return 2;

#line 43 "sharp-test/unit/generics/runtime.ce"
    Pair__int__swap(&p);

#line 45 "sharp-test/unit/generics/runtime.ce"
    if (p.first != 20) 
        return 3;

#line 46 "sharp-test/unit/generics/runtime.ce"
    if (p.second != 10) 
        return 4;

#line 49 "sharp-test/unit/generics/runtime.ce"
    Pair__float pf;

#line 50 "sharp-test/unit/generics/runtime.ce"
    pf.first = 1.5f;

#line 51 "sharp-test/unit/generics/runtime.ce"
    pf.second = 2.5f;

#line 53 "sharp-test/unit/generics/runtime.ce"
    if (pf.first != 1.5f) 
        return 5;

#line 54 "sharp-test/unit/generics/runtime.ce"
    if (pf.second != 2.5f) 
        return 6;

#line 56 "sharp-test/unit/generics/runtime.ce"
    Pair__float__swap(&pf);

#line 58 "sharp-test/unit/generics/runtime.ce"
    if (pf.first != 2.5f) 
        return 7;

#line 59 "sharp-test/unit/generics/runtime.ce"
    if (pf.second != 1.5f) 
        return 8;

#line 62 "sharp-test/unit/generics/runtime.ce"
    Wrap__int wi;

#line 63 "sharp-test/unit/generics/runtime.ce"
    Wrap__int__set(&wi, 42);

#line 64 "sharp-test/unit/generics/runtime.ce"
    if (Wrap__int__get(&wi) != 42) 
        return 9;

#line 66 "sharp-test/unit/generics/runtime.ce"
    Wrap__float wf;

#line 67 "sharp-test/unit/generics/runtime.ce"
    Wrap__float__set(&wf, 3.14f);

#line 68 "sharp-test/unit/generics/runtime.ce"
    if (Wrap__float__get(&wf) != 3.14f) 
        return 10;

#line 71 "sharp-test/unit/generics/runtime.ce"
    Wrap__int wi2;

#line 72 "sharp-test/unit/generics/runtime.ce"
    Wrap__int__set(&wi2, 99);

#line 73 "sharp-test/unit/generics/runtime.ce"
    if (Wrap__int__get(&wi2) != 99) 
        return 11;

#line 76 "sharp-test/unit/generics/runtime.ce"
    Pair__int p2 = { 10, 20 };

#line 77 "sharp-test/unit/generics/runtime.ce"
    Pair__int rev = Pair__int__reversed(&p2);

#line 78 "sharp-test/unit/generics/runtime.ce"
    if (rev.first != 20) 
        return 12;

#line 79 "sharp-test/unit/generics/runtime.ce"
    if (rev.second != 10) 
        return 13;

#line 82 "sharp-test/unit/generics/runtime.ce"
    Wrap__int w1;
    Wrap__int__set(&w1, 5);

#line 83 "sharp-test/unit/generics/runtime.ce"
    Wrap__int w2;
    Wrap__int__set(&w2, 10);

#line 84 "sharp-test/unit/generics/runtime.ce"
    if (Wrap__int__get(&w1) + Wrap__int__get(&w2) != 15) 
        return 14;

#line 86 "sharp-test/unit/generics/runtime.ce"
    return 0;
}

#line 13 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) void Pair__int__swap(Pair__int * this) {

#line 14 "sharp-test/unit/generics/runtime.ce"
    int tmp = this->first;

#line 15 "sharp-test/unit/generics/runtime.ce"
    this->first = this->second;

#line 16 "sharp-test/unit/generics/runtime.ce"
    this->second = tmp;
}


#line 13 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) void Pair__float__swap(Pair__float * this) {

#line 14 "sharp-test/unit/generics/runtime.ce"
    float tmp = this->first;

#line 15 "sharp-test/unit/generics/runtime.ce"
    this->first = this->second;

#line 16 "sharp-test/unit/generics/runtime.ce"
    this->second = tmp;
}


#line 24 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) void Wrap__int__set(Wrap__int * this, int v) {
    this->value = v;
}


#line 25 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) int Wrap__int__get(Wrap__int * this) {
    return this->value;
}


#line 24 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) void Wrap__float__set(Wrap__float * this, float v) {
    this->value = v;
}


#line 25 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) float Wrap__float__get(Wrap__float * this) {
    return this->value;
}


#line 27 "sharp-test/unit/generics/runtime.ce"
__attribute__((weak)) Pair__int Pair__int__reversed(Pair__int * this) {

#line 28 "sharp-test/unit/generics/runtime.ce"
    Pair__int r;

#line 29 "sharp-test/unit/generics/runtime.ce"
    r.first = this->second;

#line 30 "sharp-test/unit/generics/runtime.ce"
    r.second = this->first;

#line 31 "sharp-test/unit/generics/runtime.ce"
    return r;
}

