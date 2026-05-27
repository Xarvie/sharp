#include <stdbool.h>
#include <stdlib.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>


#line 3 "sharp-test/c_superset_probes/../../std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../std/types.sph"
typedef unsigned long usize;

#line 3 "sharp-test/c_superset_probes/../../std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../std/types.sph"
typedef unsigned long usize;

#line 5 "sharp-test/c_superset_probes/../../std/vec.sph"

#line 5 "sharp-test/c_superset_probes/../../std/vec.sph"
typedef struct Vec__int Vec__int;
struct Vec__int {
    int * data;
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



#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
void Vec__int__push(Vec__int * this, int val);
void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val);
void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val);

#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this);

#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i);

#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
long Vec__Vec__int__size(Vec__Vec__int * this);

#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i);

#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
long Vec__int__size(Vec__int * this);

#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
int Vec__int__get(Vec__int * this, long i);

#line 9 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
int main() {

#line 11 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__Vec__int v3d = { 0 };

#line 13 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int inner2d = { 0 };

#line 14 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int inner1d = { 0 };

#line 15 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int__push(&inner1d, 42);

#line 16 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int__push(&inner1d, 73);

#line 17 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int__push(&inner2d, inner1d);

#line 18 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__Vec__int__push(&v3d, inner2d);

#line 20 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__Vec__Vec__int__size(&v3d) != 1) 
        return 1;

#line 21 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int r2 = Vec__Vec__Vec__int__get(&v3d, 0);

#line 22 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__Vec__int__size(&r2) != 1) 
        return 2;

#line 23 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int r1 = Vec__Vec__int__get(&r2, 0);

#line 24 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__int__size(&r1) != 2) 
        return 3;

#line 25 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__int__get(&r1, 0) != 42) 
        return 4;

#line 28 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int m2d = { 0 };

#line 29 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int row0 = { 0 };

#line 30 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int row1 = { 0 };

#line 31 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int__push(&row0, 1);

#line 32 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int__push(&row1, 10);

#line 33 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__int__push(&row1, 20);

#line 34 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int__push(&m2d, row0);

#line 35 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    Vec__Vec__int__push(&m2d, row1);

#line 37 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__Vec__int__size(&m2d) != 2) 
        return 5;

#line 38 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__int__get((__extension__(({ Vec__int __sharp_chain0 = (Vec__Vec__int__get(&m2d, 0)); &__sharp_chain0; }))), 0) != 1) 
        return 6;

#line 39 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    if (Vec__int__get((__extension__(({ Vec__int __sharp_chain1 = (Vec__Vec__int__get(&m2d, 1)); &__sharp_chain1; }))), 1) != 20) 
        return 7;

#line 41 "sharp-test/c_superset_probes/p233_generic_nested3.sp"
    return 0;
}

#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 12 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../std/vec.sph"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 15 "sharp-test/c_superset_probes/../../std/vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->len = this->len + 1;
}


#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__Vec__int__push(Vec__Vec__int * this, Vec__int val) {

#line 12 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../std/vec.sph"
        Vec__int * nd = (Vec__int *)realloc(this->data, sizeof(Vec__int) * new_cap);

#line 15 "sharp-test/c_superset_probes/../../std/vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->len = this->len + 1;
}


#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__Vec__Vec__int__push(Vec__Vec__Vec__int * this, Vec__Vec__int val) {

#line 12 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../std/vec.sph"
        Vec__Vec__int * nd = (Vec__Vec__int *)realloc(this->data, sizeof(Vec__Vec__int) * new_cap);

#line 15 "sharp-test/c_superset_probes/../../std/vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/../../std/vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->len = this->len + 1;
}


#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) long Vec__Vec__Vec__int__size(Vec__Vec__Vec__int * this) {

#line 50 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->len;
}


#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) Vec__Vec__int Vec__Vec__Vec__int__get(Vec__Vec__Vec__int * this, long i) {

#line 30 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->data[i];
}


#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) long Vec__Vec__int__size(Vec__Vec__int * this) {

#line 50 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->len;
}


#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) Vec__int Vec__Vec__int__get(Vec__Vec__int * this, long i) {

#line 30 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->data[i];
}


#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 50 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->len;
}


#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 30 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->data[i];
}

