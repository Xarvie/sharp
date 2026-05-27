
#include <stdlib.h>

#line 6 "unit/generics/struct.sp"

#line 12 "unit/generics/struct.sp"

#line 18 "unit/generics/struct.sp"

#line 23 "unit/generics/struct.sp"

#line 39 "unit/generics/struct.sp"

#line 6 "unit/generics/struct.sp"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int data;
    int len;
};


typedef struct Vec__float Vec__float;
struct Vec__float {
    float data;
    int len;
};



#line 12 "unit/generics/struct.sp"
typedef struct Pair__int__int Pair__int__int;
struct Pair__int__int {
    int first;
    int second;
};


typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};



#line 18 "unit/generics/struct.sp"
typedef struct Box__int Box__int;
struct Box__int {
    int * value;
};



#line 23 "unit/generics/struct.sp"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int top;
    int size;
};



#line 39 "unit/generics/struct.sp"
typedef struct Map__int__float Map__int__float;
struct Map__int__float {
    int * keys;
    float * values;
    int count;
};



#line 28 "unit/generics/struct.sp"
int Stack__int__push(Stack__int * this, int val);

#line 34 "unit/generics/struct.sp"
int Stack__int__peek(Stack__int * this);

#line 28 "unit/generics/struct.sp"

#line 34 "unit/generics/struct.sp"

#line 45 "unit/generics/struct.sp"
int main() {

#line 47 "unit/generics/struct.sp"
    Vec__int vi;

#line 48 "unit/generics/struct.sp"
    vi.data = 42;

#line 49 "unit/generics/struct.sp"
    vi.len = 1;

#line 50 "unit/generics/struct.sp"
    if (vi.data != 42) 
        return 1;

#line 51 "unit/generics/struct.sp"
    if (vi.len != 1) 
        return 2;

#line 53 "unit/generics/struct.sp"
    Vec__float vf;

#line 54 "unit/generics/struct.sp"
    vf.data = 3.14f;

#line 55 "unit/generics/struct.sp"
    vf.len = 1;

#line 56 "unit/generics/struct.sp"
    if (vf.len != 1) 
        return 3;

#line 59 "unit/generics/struct.sp"
    Pair__int__int p;

#line 60 "unit/generics/struct.sp"
    p.first = 10;

#line 61 "unit/generics/struct.sp"
    p.second = 20;

#line 62 "unit/generics/struct.sp"
    if (p.first != 10) 
        return 4;

#line 63 "unit/generics/struct.sp"
    if (p.second != 20) 
        return 5;

#line 65 "unit/generics/struct.sp"
    Pair__int__float pif;

#line 66 "unit/generics/struct.sp"
    pif.first = 7;

#line 67 "unit/generics/struct.sp"
    pif.second = 1.5f;

#line 68 "unit/generics/struct.sp"
    if (pif.first != 7) 
        return 6;

#line 71 "unit/generics/struct.sp"
    int x = 100;

#line 72 "unit/generics/struct.sp"
    Box__int box;

#line 73 "unit/generics/struct.sp"
    box.value = &x;

#line 74 "unit/generics/struct.sp"
    if (box.value[0] != 100) 
        return 7;

#line 77 "unit/generics/struct.sp"
    Stack__int s;

#line 78 "unit/generics/struct.sp"
    s.size = 0;

#line 79 "unit/generics/struct.sp"
    Stack__int__push(&s, 42);

#line 80 "unit/generics/struct.sp"
    if (Stack__int__peek(&s) != 42) 
        return 8;

#line 81 "unit/generics/struct.sp"
    if (s.size != 1) 
        return 9;

#line 82 "unit/generics/struct.sp"
    Stack__int__push(&s, 99);

#line 83 "unit/generics/struct.sp"
    if (Stack__int__peek(&s) != 99) 
        return 10;

#line 84 "unit/generics/struct.sp"
    if (s.size != 2) 
        return 11;

#line 87 "unit/generics/struct.sp"
    Pair__int__float lit = { .first = 7, .second = 3.14f };

#line 88 "unit/generics/struct.sp"
    if (lit.first != 7) 
        return 12;

#line 89 "unit/generics/struct.sp"
    if (lit.second < 3.13f || lit.second > 3.15f) 
        return 13;

#line 92 "unit/generics/struct.sp"
    Map__int__float m;

#line 93 "unit/generics/struct.sp"
    m.count = 5;

#line 94 "unit/generics/struct.sp"
    if (m.count != 5) 
        return 14;

#line 96 "unit/generics/struct.sp"
    return 0;
}

#line 28 "unit/generics/struct.sp"
__attribute__((weak)) int Stack__int__push(Stack__int * this, int val) {

#line 29 "unit/generics/struct.sp"
    this->top = val;

#line 30 "unit/generics/struct.sp"
    this->size = this->size + 1;

#line 31 "unit/generics/struct.sp"
    return this->size;
}


#line 34 "unit/generics/struct.sp"
__attribute__((weak)) int Stack__int__peek(Stack__int * this) {

#line 35 "unit/generics/struct.sp"
    return this->top;
}

