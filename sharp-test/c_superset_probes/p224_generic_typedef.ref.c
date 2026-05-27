#include <stdbool.h>
#include <stdlib.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

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


typedef struct Vec__float Vec__float;
struct Vec__float {
    float * data;
    isize len;
    isize cap;
};



#line 15 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
typedef Vec__int IntVec;

#line 16 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
typedef Vec__float FloatVec;

#line 17 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
typedef IntVec MyIntVec;

#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
void Vec__int__push(Vec__int * this, int val);

#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
long Vec__int__size(Vec__int * this);

#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
int Vec__int__get(Vec__int * this, long i);

#line 73 "sharp-test/c_superset_probes/../../std/vec.sph"
void Vec__int__destroy(Vec__int * this);

#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
void Vec__float__push(Vec__float * this, float val);

#line 49 "sharp-test/c_superset_probes/../../std/vec.sph"
long Vec__float__size(Vec__float * this);

#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
float Vec__float__get(Vec__float * this, long i);

#line 73 "sharp-test/c_superset_probes/../../std/vec.sph"
void Vec__float__destroy(Vec__float * this);

#line 19 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
int main() {

#line 21 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    IntVec v = { 0 };

#line 22 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__push(&v, 42);

#line 23 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__push(&v, 73);

#line 24 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__size(&v) != 2) 
        return 1;

#line 25 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__get(&v, 0) != 42) 
        return 2;

#line 26 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__get(&v, 1) != 73) 
        return 3;

#line 27 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__destroy(&v);

#line 30 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    MyIntVec v2 = { 0 };

#line 31 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__push(&v2, 99);

#line 32 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__size(&v2) != 1) 
        return 4;

#line 33 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__get(&v2, 0) != 99) 
        return 5;

#line 34 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__destroy(&v2);

#line 37 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    FloatVec fv = { 0 };

#line 38 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__float__push(&fv, 1.5f);

#line 39 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__float__push(&fv, 2.5f);

#line 40 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__float__size(&fv) != 2) 
        return 6;

#line 41 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__float__get(&fv, 0) < 1.4f || Vec__float__get(&fv, 0) > 1.6f) 
        return 7;

#line 42 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__float__destroy(&fv);
    IntVec a = { 0 }, b = { 0 };

#line 46 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__push(&a, 10);
    Vec__int__push(&b, 20);

#line 47 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__size(&a) != 1) 
        return 8;

#line 48 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    Vec__int__destroy(&a);
    Vec__int__destroy(&b);

#line 51 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    IntVec z = { 0 };

#line 52 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
    if (Vec__int__size(&z) != 0) 
        return 9;

#line 55 "sharp-test/c_superset_probes/p224_generic_typedef.sp"
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


#line 73 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 74 "sharp-test/c_superset_probes/../../std/vec.sph"
    free(this->data);

#line 75 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->data = (int *)0;

#line 76 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->len = 0;

#line 77 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->cap = 0;
}


#line 11 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 12 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../std/vec.sph"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

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
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 50 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->len;
}


#line 29 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) float Vec__float__get(Vec__float * this, long i) {

#line 30 "sharp-test/c_superset_probes/../../std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/../../std/vec.sph"
    return this->data[i];
}


#line 73 "sharp-test/c_superset_probes/../../std/vec.sph"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 74 "sharp-test/c_superset_probes/../../std/vec.sph"
    free(this->data);

#line 75 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->data = (float *)0;

#line 76 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->len = 0;

#line 77 "sharp-test/c_superset_probes/../../std/vec.sph"
    this->cap = 0;
}

