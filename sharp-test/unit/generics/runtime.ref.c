
#include <stdlib.h>

#line 4 "unit/generics/runtime.sp"
typedef long isize;

#line 5 "unit/generics/runtime.sp"
typedef unsigned long usize;

#line 8 "unit/generics/runtime.sp"

#line 20 "unit/generics/runtime.sp"

#line 8 "unit/generics/runtime.sp"
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



#line 20 "unit/generics/runtime.sp"
typedef struct Wrap__int Wrap__int;
struct Wrap__int {
    int value;
};


typedef struct Wrap__float Wrap__float;
struct Wrap__float {
    float value;
};



#line 13 "unit/generics/runtime.sp"
void Pair__int__swap(Pair__int * this);
void Pair__float__swap(Pair__float * this);

#line 24 "unit/generics/runtime.sp"
void Wrap__int__set(Wrap__int * this, int v);

#line 25 "unit/generics/runtime.sp"
int Wrap__int__get(Wrap__int * this);

#line 24 "unit/generics/runtime.sp"
void Wrap__float__set(Wrap__float * this, float v);

#line 25 "unit/generics/runtime.sp"
float Wrap__float__get(Wrap__float * this);

#line 13 "unit/generics/runtime.sp"

#line 24 "unit/generics/runtime.sp"

#line 25 "unit/generics/runtime.sp"

#line 27 "unit/generics/runtime.sp"
int main() {

#line 29 "unit/generics/runtime.sp"
    Pair__int p;

#line 30 "unit/generics/runtime.sp"
    p.first = 10;

#line 31 "unit/generics/runtime.sp"
    p.second = 20;

#line 33 "unit/generics/runtime.sp"
    if (p.first != 10) 
        return 1;

#line 34 "unit/generics/runtime.sp"
    if (p.second != 20) 
        return 2;

#line 36 "unit/generics/runtime.sp"
    Pair__int__swap(&p);

#line 38 "unit/generics/runtime.sp"
    if (p.first != 20) 
        return 3;

#line 39 "unit/generics/runtime.sp"
    if (p.second != 10) 
        return 4;

#line 42 "unit/generics/runtime.sp"
    Pair__float pf;

#line 43 "unit/generics/runtime.sp"
    pf.first = 1.5f;

#line 44 "unit/generics/runtime.sp"
    pf.second = 2.5f;

#line 46 "unit/generics/runtime.sp"
    if (pf.first != 1.5f) 
        return 5;

#line 47 "unit/generics/runtime.sp"
    if (pf.second != 2.5f) 
        return 6;

#line 49 "unit/generics/runtime.sp"
    Pair__float__swap(&pf);

#line 51 "unit/generics/runtime.sp"
    if (pf.first != 2.5f) 
        return 7;

#line 52 "unit/generics/runtime.sp"
    if (pf.second != 1.5f) 
        return 8;

#line 55 "unit/generics/runtime.sp"
    Wrap__int wi;

#line 56 "unit/generics/runtime.sp"
    Wrap__int__set(&wi, 42);

#line 57 "unit/generics/runtime.sp"
    if (Wrap__int__get(&wi) != 42) 
        return 9;

#line 59 "unit/generics/runtime.sp"
    Wrap__float wf;

#line 60 "unit/generics/runtime.sp"
    Wrap__float__set(&wf, 3.14f);

#line 61 "unit/generics/runtime.sp"
    if (Wrap__float__get(&wf) != 3.14f) 
        return 10;

#line 64 "unit/generics/runtime.sp"
    Wrap__int wi2;

#line 65 "unit/generics/runtime.sp"
    Wrap__int__set(&wi2, 99);

#line 66 "unit/generics/runtime.sp"
    if (Wrap__int__get(&wi2) != 99) 
        return 11;

#line 68 "unit/generics/runtime.sp"
    return 0;
}

#line 13 "unit/generics/runtime.sp"
__attribute__((weak)) void Pair__int__swap(Pair__int * this) {

#line 14 "unit/generics/runtime.sp"
    int tmp = this->first;

#line 15 "unit/generics/runtime.sp"
    this->first = this->second;

#line 16 "unit/generics/runtime.sp"
    this->second = tmp;
}


#line 13 "unit/generics/runtime.sp"
__attribute__((weak)) void Pair__float__swap(Pair__float * this) {

#line 14 "unit/generics/runtime.sp"
    float tmp = this->first;

#line 15 "unit/generics/runtime.sp"
    this->first = this->second;

#line 16 "unit/generics/runtime.sp"
    this->second = tmp;
}


#line 24 "unit/generics/runtime.sp"
__attribute__((weak)) void Wrap__int__set(Wrap__int * this, int v) {
    this->value = v;
}


#line 25 "unit/generics/runtime.sp"
__attribute__((weak)) int Wrap__int__get(Wrap__int * this) {
    return this->value;
}


#line 24 "unit/generics/runtime.sp"
__attribute__((weak)) void Wrap__float__set(Wrap__float * this, float v) {
    this->value = v;
}


#line 25 "unit/generics/runtime.sp"
__attribute__((weak)) float Wrap__float__get(Wrap__float * this) {
    return this->value;
}

