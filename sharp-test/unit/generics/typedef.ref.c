#include <stdlib.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 7 "unit/generics/typedef.sp"
typedef long isize;

#line 8 "unit/generics/typedef.sp"
typedef unsigned long usize;

#line 11 "unit/generics/typedef.sp"

#line 32 "unit/generics/typedef.sp"

#line 72 "unit/generics/typedef.sp"

#line 11 "unit/generics/typedef.sp"
typedef struct Stack__int Stack__int;
struct Stack__int {
    int data;
    int n;
};



#line 32 "unit/generics/typedef.sp"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
    isize len;
    isize cap;
};


typedef struct Vec__float Vec__float;
struct Vec__float {
    float * data;
    isize len;
    isize cap;
};



#line 72 "unit/generics/typedef.sp"
typedef struct Arr__int Arr__int;
struct Arr__int {
    int * data;
    int size;
};



#line 67 "unit/generics/typedef.sp"
typedef Vec__int IntVec;

#line 68 "unit/generics/typedef.sp"
typedef Vec__float FloatVec;

#line 69 "unit/generics/typedef.sp"
typedef IntVec MyIntVec;

#line 16 "unit/generics/typedef.sp"
Stack__int Stack__int__new(void);

#line 22 "unit/generics/typedef.sp"
void Stack__int__push(Stack__int * this, int v);

#line 27 "unit/generics/typedef.sp"
int Stack__int__top(Stack__int * this);

#line 38 "unit/generics/typedef.sp"
void Vec__int__push(Vec__int * this, int val);

#line 50 "unit/generics/typedef.sp"
long Vec__int__size(Vec__int * this);

#line 54 "unit/generics/typedef.sp"
int Vec__int__get(Vec__int * this, long i);

#line 59 "unit/generics/typedef.sp"
void Vec__int__destroy(Vec__int * this);

#line 38 "unit/generics/typedef.sp"
void Vec__float__push(Vec__float * this, float val);

#line 50 "unit/generics/typedef.sp"
long Vec__float__size(Vec__float * this);

#line 54 "unit/generics/typedef.sp"
float Vec__float__get(Vec__float * this, long i);

#line 59 "unit/generics/typedef.sp"
void Vec__float__destroy(Vec__float * this);

#line 77 "unit/generics/typedef.sp"
int Arr__int__len(Arr__int * this);

#line 16 "unit/generics/typedef.sp"

#line 22 "unit/generics/typedef.sp"

#line 27 "unit/generics/typedef.sp"

#line 38 "unit/generics/typedef.sp"

#line 50 "unit/generics/typedef.sp"

#line 54 "unit/generics/typedef.sp"

#line 59 "unit/generics/typedef.sp"

#line 77 "unit/generics/typedef.sp"

#line 81 "unit/generics/typedef.sp"
int main() {

#line 83 "unit/generics/typedef.sp"
    Stack__int s = Stack__int__new();

#line 84 "unit/generics/typedef.sp"
    Stack__int__push(&s, 42);

#line 85 "unit/generics/typedef.sp"
    if (Stack__int__top(&s) != 42) 
        return 1;

#line 86 "unit/generics/typedef.sp"
    if (s.n != 1) 
        return 2;

#line 89 "unit/generics/typedef.sp"
    IntVec v = { 0 };

#line 90 "unit/generics/typedef.sp"
    Vec__int__push(&v, 42);

#line 91 "unit/generics/typedef.sp"
    Vec__int__push(&v, 73);

#line 92 "unit/generics/typedef.sp"
    if (Vec__int__size(&v) != 2) 
        return 3;

#line 93 "unit/generics/typedef.sp"
    if (Vec__int__get(&v, 0) != 42) 
        return 4;

#line 94 "unit/generics/typedef.sp"
    if (Vec__int__get(&v, 1) != 73) 
        return 5;

#line 95 "unit/generics/typedef.sp"
    Vec__int__destroy(&v);

#line 98 "unit/generics/typedef.sp"
    MyIntVec v2 = { 0 };

#line 99 "unit/generics/typedef.sp"
    Vec__int__push(&v2, 99);

#line 100 "unit/generics/typedef.sp"
    if (Vec__int__size(&v2) != 1) 
        return 6;

#line 101 "unit/generics/typedef.sp"
    if (Vec__int__get(&v2, 0) != 99) 
        return 7;

#line 102 "unit/generics/typedef.sp"
    Vec__int__destroy(&v2);

#line 105 "unit/generics/typedef.sp"
    FloatVec fv = { 0 };

#line 106 "unit/generics/typedef.sp"
    Vec__float__push(&fv, 1.5f);

#line 107 "unit/generics/typedef.sp"
    if (Vec__float__size(&fv) != 1) 
        return 8;

#line 108 "unit/generics/typedef.sp"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 9;

#line 109 "unit/generics/typedef.sp"
    Vec__float__destroy(&fv);

#line 112 "unit/generics/typedef.sp"
    IntVec z = { 0 };

#line 113 "unit/generics/typedef.sp"
    if (Vec__int__size(&z) != 0) 
        return 10;

#line 116 "unit/generics/typedef.sp"
    Arr__int arr = { 0 };

#line 117 "unit/generics/typedef.sp"
    arr.size = 5;

#line 118 "unit/generics/typedef.sp"
    if (Arr__int__len(&arr) != 5) 
        return 11;

#line 120 "unit/generics/typedef.sp"
    return 0;
}

#line 16 "unit/generics/typedef.sp"
__attribute__((weak)) Stack__int Stack__int__new(void) {

#line 17 "unit/generics/typedef.sp"
    Stack__int s;

#line 18 "unit/generics/typedef.sp"
    s.n = 0;

#line 19 "unit/generics/typedef.sp"
    return s;
}


#line 22 "unit/generics/typedef.sp"
__attribute__((weak)) void Stack__int__push(Stack__int * this, int v) {

#line 23 "unit/generics/typedef.sp"
    this->data = v;

#line 24 "unit/generics/typedef.sp"
    this->n = this->n + 1;
}


#line 27 "unit/generics/typedef.sp"
__attribute__((weak)) int Stack__int__top(Stack__int * this) {

#line 28 "unit/generics/typedef.sp"
    return this->data;
}


#line 38 "unit/generics/typedef.sp"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 39 "unit/generics/typedef.sp"
    if (this->len >= this->cap) {

#line 40 "unit/generics/typedef.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "unit/generics/typedef.sp"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 42 "unit/generics/typedef.sp"
        if (!nd) 
            __builtin_trap();

#line 43 "unit/generics/typedef.sp"
        this->data = nd;

#line 44 "unit/generics/typedef.sp"
        this->cap = new_cap;
    }

#line 46 "unit/generics/typedef.sp"
    this->data[this->len] = val;

#line 47 "unit/generics/typedef.sp"
    this->len = this->len + 1;
}


#line 50 "unit/generics/typedef.sp"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 51 "unit/generics/typedef.sp"
    return this->len;
}


#line 54 "unit/generics/typedef.sp"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 55 "unit/generics/typedef.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "unit/generics/typedef.sp"
    return this->data[i];
}


#line 59 "unit/generics/typedef.sp"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 60 "unit/generics/typedef.sp"
    free(this->data);

#line 61 "unit/generics/typedef.sp"
    this->data = (int *)0;

#line 62 "unit/generics/typedef.sp"
    this->len = 0;

#line 63 "unit/generics/typedef.sp"
    this->cap = 0;
}


#line 38 "unit/generics/typedef.sp"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 39 "unit/generics/typedef.sp"
    if (this->len >= this->cap) {

#line 40 "unit/generics/typedef.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 41 "unit/generics/typedef.sp"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 42 "unit/generics/typedef.sp"
        if (!nd) 
            __builtin_trap();

#line 43 "unit/generics/typedef.sp"
        this->data = nd;

#line 44 "unit/generics/typedef.sp"
        this->cap = new_cap;
    }

#line 46 "unit/generics/typedef.sp"
    this->data[this->len] = val;

#line 47 "unit/generics/typedef.sp"
    this->len = this->len + 1;
}


#line 50 "unit/generics/typedef.sp"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 51 "unit/generics/typedef.sp"
    return this->len;
}


#line 54 "unit/generics/typedef.sp"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 55 "unit/generics/typedef.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 56 "unit/generics/typedef.sp"
    return this->data[i];
}


#line 59 "unit/generics/typedef.sp"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 60 "unit/generics/typedef.sp"
    free(this->data);

#line 61 "unit/generics/typedef.sp"
    this->data = (float *)0;

#line 62 "unit/generics/typedef.sp"
    this->len = 0;

#line 63 "unit/generics/typedef.sp"
    this->cap = 0;
}


#line 77 "unit/generics/typedef.sp"
__attribute__((weak)) int Arr__int__len(Arr__int * this) {

#line 78 "unit/generics/typedef.sp"
    return this->size;
}

