#include <stdlib.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 7 "unit/generics/nested.sp"
typedef long isize;

#line 8 "unit/generics/nested.sp"
typedef unsigned long usize;

#line 11 "unit/generics/nested.sp"

#line 46 "unit/generics/nested.sp"

#line 11 "unit/generics/nested.sp"
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


typedef struct Vec__Vec__int Vec__Vec__int;
struct Vec__Vec__int {
    Vec__int * data;
    isize len;
    isize cap;
};


typedef struct Vec__Vec__Vec__int Vec__Vec__Vec__int;
struct Vec__Vec__Vec__int {
    Vec__Vec__int * data;
    isize len;
    isize cap;
};



#line 46 "unit/generics/nested.sp"
typedef struct Pair__int__Vec__int Pair__int__Vec__int;
struct Pair__int__Vec__int {
    int first;
    Vec__int second;
};



#line 17 "unit/generics/nested.sp"
void Vec__int__push(Vec__int * this, int val);

#line 29 "unit/generics/nested.sp"
long Vec__int__size(Vec__int * this);

#line 33 "unit/generics/nested.sp"
int Vec__int__get(Vec__int * this, long i);

#line 38 "unit/generics/nested.sp"
void Vec__int__destroy(Vec__int * this);

#line 17 "unit/generics/nested.sp"
void Vec__float__push(Vec__float * this, float val);

#line 29 "unit/generics/nested.sp"
long Vec__float__size(Vec__float * this);

#line 33 "unit/generics/nested.sp"
float Vec__float__get(Vec__float * this, long i);

#line 38 "unit/generics/nested.sp"
void Vec__float__destroy(Vec__float * this);

#line 17 "unit/generics/nested.sp"
void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val);
void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val);

#line 29 "unit/generics/nested.sp"
long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this);

#line 33 "unit/generics/nested.sp"
Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i);

#line 29 "unit/generics/nested.sp"
long Vec__Vec__int__size(Vec__Vec__int * this);

#line 33 "unit/generics/nested.sp"
Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i);

#line 38 "unit/generics/nested.sp"
void Vec__Vec__int__destroy(Vec__Vec__int * this);
void Vec__Vec__Vec__int__destroy(Vec__Vec__Vec__int * this);

#line 17 "unit/generics/nested.sp"

#line 29 "unit/generics/nested.sp"

#line 33 "unit/generics/nested.sp"

#line 38 "unit/generics/nested.sp"

#line 51 "unit/generics/nested.sp"
int main() {

#line 53 "unit/generics/nested.sp"
    Vec__int v = { 0 };

#line 54 "unit/generics/nested.sp"
    Vec__int__push(&v, 42);

#line 55 "unit/generics/nested.sp"
    Vec__int__push(&v, 73);

#line 56 "unit/generics/nested.sp"
    if (Vec__int__size(&v) != 2) 
        return 1;

#line 57 "unit/generics/nested.sp"
    if (Vec__int__get(&v, 0) != 42) 
        return 2;

#line 58 "unit/generics/nested.sp"
    if (Vec__int__get(&v, 1) != 73) 
        return 3;

#line 59 "unit/generics/nested.sp"
    Vec__int__destroy(&v);

#line 62 "unit/generics/nested.sp"
    Vec__float fv = { 0 };

#line 63 "unit/generics/nested.sp"
    Vec__float__push(&fv, 1.5f);

#line 64 "unit/generics/nested.sp"
    Vec__float__push(&fv, 2.5f);

#line 65 "unit/generics/nested.sp"
    if (Vec__float__size(&fv) != 2) 
        return 4;

#line 66 "unit/generics/nested.sp"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 5;

#line 67 "unit/generics/nested.sp"
    Vec__float__destroy(&fv);

#line 70 "unit/generics/nested.sp"
    Vec__Vec__Vec__int v3d = { 0 };

#line 71 "unit/generics/nested.sp"
    Vec__Vec__int inner2d = { 0 };

#line 72 "unit/generics/nested.sp"
    Vec__int inner1d = { 0 };

#line 73 "unit/generics/nested.sp"
    Vec__int__push(&inner1d, 42);

#line 74 "unit/generics/nested.sp"
    Vec__int__push(&inner1d, 73);

#line 75 "unit/generics/nested.sp"
    Vec__Vec__int__push(&inner2d, inner1d);

#line 76 "unit/generics/nested.sp"
    Vec__Vec__Vec__int__push(&v3d, inner2d);

#line 78 "unit/generics/nested.sp"
    if (Vec__Vec__Vec__int__size(&v3d) != 1) 
        return 6;

#line 79 "unit/generics/nested.sp"
    Vec__Vec__int r2 = Vec__Vec__Vec__int__get(&v3d, 0);

#line 80 "unit/generics/nested.sp"
    if (Vec__Vec__int__size(&r2) != 1) 
        return 7;

#line 81 "unit/generics/nested.sp"
    Vec__int r1 = Vec__Vec__int__get(&r2, 0);

#line 82 "unit/generics/nested.sp"
    if (Vec__int__size(&r1) != 2) 
        return 8;

#line 83 "unit/generics/nested.sp"
    if (Vec__int__get(&r1, 0) != 42) 
        return 9;

#line 84 "unit/generics/nested.sp"
    Vec__int__destroy(&r1);

#line 85 "unit/generics/nested.sp"
    Vec__Vec__int__destroy(&r2);

#line 86 "unit/generics/nested.sp"
    Vec__Vec__Vec__int__destroy(&v3d);

#line 89 "unit/generics/nested.sp"
    Vec__Vec__int m2d = { 0 };

#line 90 "unit/generics/nested.sp"
    Vec__int row0 = { 0 };

#line 91 "unit/generics/nested.sp"
    Vec__int row1 = { 0 };

#line 92 "unit/generics/nested.sp"
    Vec__int__push(&row0, 1);

#line 93 "unit/generics/nested.sp"
    Vec__int__push(&row1, 10);

#line 94 "unit/generics/nested.sp"
    Vec__int__push(&row1, 20);

#line 95 "unit/generics/nested.sp"
    Vec__Vec__int__push(&m2d, row0);

#line 96 "unit/generics/nested.sp"
    Vec__Vec__int__push(&m2d, row1);

#line 98 "unit/generics/nested.sp"
    if (Vec__Vec__int__size(&m2d) != 2) 
        return 10;

#line 99 "unit/generics/nested.sp"
    Vec__int c0 = Vec__Vec__int__get(&m2d, 0);

#line 100 "unit/generics/nested.sp"
    if (Vec__int__get(&c0, 0) != 1) 
        return 11;

#line 101 "unit/generics/nested.sp"
    Vec__int__destroy(&c0);

#line 102 "unit/generics/nested.sp"
    Vec__int c1 = Vec__Vec__int__get(&m2d, 1);

#line 103 "unit/generics/nested.sp"
    if (Vec__int__get(&c1, 1) != 20) 
        return 12;

#line 104 "unit/generics/nested.sp"
    Vec__int__destroy(&c1);

#line 105 "unit/generics/nested.sp"
    Vec__Vec__int__destroy(&m2d);

#line 108 "unit/generics/nested.sp"
    Pair__int__Vec__int pv = { 0 };

#line 109 "unit/generics/nested.sp"
    pv.first = 42;

#line 111 "unit/generics/nested.sp"
    return 0;
}

#line 17 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 18 "unit/generics/nested.sp"
    if (this->len >= this->cap) {

#line 19 "unit/generics/nested.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "unit/generics/nested.sp"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 21 "unit/generics/nested.sp"
        if (!nd) 
            __builtin_trap();

#line 22 "unit/generics/nested.sp"
        this->data = nd;

#line 23 "unit/generics/nested.sp"
        this->cap = new_cap;
    }

#line 25 "unit/generics/nested.sp"
    this->data[this->len] = val;

#line 26 "unit/generics/nested.sp"
    this->len = this->len + 1;
}


#line 29 "unit/generics/nested.sp"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 30 "unit/generics/nested.sp"
    return this->len;
}


#line 33 "unit/generics/nested.sp"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 34 "unit/generics/nested.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "unit/generics/nested.sp"
    return this->data[i];
}


#line 38 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 39 "unit/generics/nested.sp"
    free(this->data);

#line 40 "unit/generics/nested.sp"
    this->data = (int *)0;

#line 41 "unit/generics/nested.sp"
    this->len = 0;

#line 42 "unit/generics/nested.sp"
    this->cap = 0;
}


#line 17 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 18 "unit/generics/nested.sp"
    if (this->len >= this->cap) {

#line 19 "unit/generics/nested.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "unit/generics/nested.sp"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 21 "unit/generics/nested.sp"
        if (!nd) 
            __builtin_trap();

#line 22 "unit/generics/nested.sp"
        this->data = nd;

#line 23 "unit/generics/nested.sp"
        this->cap = new_cap;
    }

#line 25 "unit/generics/nested.sp"
    this->data[this->len] = val;

#line 26 "unit/generics/nested.sp"
    this->len = this->len + 1;
}


#line 29 "unit/generics/nested.sp"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 30 "unit/generics/nested.sp"
    return this->len;
}


#line 33 "unit/generics/nested.sp"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 34 "unit/generics/nested.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "unit/generics/nested.sp"
    return this->data[i];
}


#line 38 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 39 "unit/generics/nested.sp"
    free(this->data);

#line 40 "unit/generics/nested.sp"
    this->data = (float *)0;

#line 41 "unit/generics/nested.sp"
    this->len = 0;

#line 42 "unit/generics/nested.sp"
    this->cap = 0;
}


#line 17 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val) {

#line 18 "unit/generics/nested.sp"
    if (this->len >= this->cap) {

#line 19 "unit/generics/nested.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "unit/generics/nested.sp"
        Vec__int * nd = (Vec__int *)realloc(this->data, sizeof(Vec__int) * new_cap);

#line 21 "unit/generics/nested.sp"
        if (!nd) 
            __builtin_trap();

#line 22 "unit/generics/nested.sp"
        this->data = nd;

#line 23 "unit/generics/nested.sp"
        this->cap = new_cap;
    }

#line 25 "unit/generics/nested.sp"
    this->data[this->len] = val;

#line 26 "unit/generics/nested.sp"
    this->len = this->len + 1;
}


#line 17 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val) {

#line 18 "unit/generics/nested.sp"
    if (this->len >= this->cap) {

#line 19 "unit/generics/nested.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 20 "unit/generics/nested.sp"
        Vec__Vec__int * nd = (Vec__Vec__int *)realloc(this->data, sizeof(Vec__Vec__int) * new_cap);

#line 21 "unit/generics/nested.sp"
        if (!nd) 
            __builtin_trap();

#line 22 "unit/generics/nested.sp"
        this->data = nd;

#line 23 "unit/generics/nested.sp"
        this->cap = new_cap;
    }

#line 25 "unit/generics/nested.sp"
    this->data[this->len] = val;

#line 26 "unit/generics/nested.sp"
    this->len = this->len + 1;
}


#line 29 "unit/generics/nested.sp"
__attribute__((weak)) long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this) {

#line 30 "unit/generics/nested.sp"
    return this->len;
}


#line 33 "unit/generics/nested.sp"
__attribute__((weak)) Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i) {

#line 34 "unit/generics/nested.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "unit/generics/nested.sp"
    return this->data[i];
}


#line 29 "unit/generics/nested.sp"
__attribute__((weak)) long Vec__Vec__int__size(Vec__Vec__int * this) {

#line 30 "unit/generics/nested.sp"
    return this->len;
}


#line 33 "unit/generics/nested.sp"
__attribute__((weak)) Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i) {

#line 34 "unit/generics/nested.sp"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 35 "unit/generics/nested.sp"
    return this->data[i];
}


#line 38 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__Vec__int__destroy(Vec__Vec__int * this) {

#line 39 "unit/generics/nested.sp"
    free(this->data);

#line 40 "unit/generics/nested.sp"
    this->data = (Vec__int *)0;

#line 41 "unit/generics/nested.sp"
    this->len = 0;

#line 42 "unit/generics/nested.sp"
    this->cap = 0;
}


#line 38 "unit/generics/nested.sp"
__attribute__((weak)) void Vec__Vec__Vec__int__destroy(Vec__Vec__Vec__int * this) {

#line 39 "unit/generics/nested.sp"
    free(this->data);

#line 40 "unit/generics/nested.sp"
    this->data = (Vec__Vec__int *)0;

#line 41 "unit/generics/nested.sp"
    this->len = 0;

#line 42 "unit/generics/nested.sp"
    this->cap = 0;
}

