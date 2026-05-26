#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 5 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 3 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 8 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline isize Str__find_byte(Str * this, char c);

#line 17 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline isize Str__size(Str * this);

#line 21 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__op_eq(Str * this, Str other);

#line 31 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__eq(Str * this, Str other);

#line 37 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline usize Str__hash(Str * this);

#line 48 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__starts_with(Str * this, Str prefix);

#line 58 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__ends_with(Str * this, Str suffix);

#line 69 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline Str Str__slice(Str * this, isize start, isize end);

#line 92 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
struct StrSplit {
    Str left;
    Str right;
    _Bool found;
};

#line 3 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 12 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"
const isize HASHMAP_EMPTY = 0;

#line 13 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"
const isize HASHMAP_OCCUPIED = 1;

#line 14 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"
const isize HASHMAP_TOMBSTONE = 2;

#line 16 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"

#line 22 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"

#line 54 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"

#line 5 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
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



#line 16 "sharp-test/c_superset_probes/../../sharp/std/hashmap.sph"
typedef struct HashMapEntry__Str__int HashMapEntry__Str__int;
struct HashMapEntry__Str__int {
    Str key;
    int value;
    unsigned char state;
};



#line 11 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
void Vec__int__push(Vec__int * this, int val);

#line 49 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
long Vec__int__size(Vec__int * this);

#line 29 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
int Vec__int__get(Vec__int * this, long i);

#line 73 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
void Vec__int__destroy(Vec__int * this);

#line 11 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
void Vec__float__push(Vec__float * this, float val);

#line 49 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
long Vec__float__size(Vec__float * this);

#line 44 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
float Vec__float__op_idx(Vec__float * this, long i);

#line 73 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
void Vec__float__destroy(Vec__float * this);

#line 8 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline isize Str__find_byte(Str * this, char c) {

#line 9 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize i = 0;

#line 10 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (i < this->len) {

#line 11 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        if (this->ptr[i] == c) 
            return i;

#line 12 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        i = i + 1;
    }

#line 14 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return -1;
}

#line 17 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline isize Str__size(Str * this) {

#line 18 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return this->len;
}

#line 21 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__op_eq(Str * this, Str other) {

#line 22 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    if (this->len != other.len) 
        return 0;

#line 23 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize i = 0;

#line 24 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (i < this->len) {

#line 25 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        if (this->ptr[i] != other.ptr[i]) 
            return 0;

#line 26 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        i = i + 1;
    }

#line 28 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return 1;
}

#line 31 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__eq(Str * this, Str other) {

#line 32 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return Str__op_eq(&*this, other);
}

#line 37 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline usize Str__hash(Str * this) {

#line 38 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    usize h = (usize)14695981039346656037UL;

#line 39 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize i = 0;

#line 40 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (i < this->len) {

#line 41 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        h = h ^ (usize)(unsigned char)this->ptr[i];

#line 42 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        h = h * (usize)1099511628211UL;

#line 43 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        i = i + 1;
    }

#line 45 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return h;
}

#line 48 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__starts_with(Str * this, Str prefix) {

#line 49 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    if (prefix.len > this->len) 
        return 0;

#line 50 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize i = 0;

#line 51 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (i < prefix.len) {

#line 52 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        if (this->ptr[i] != prefix.ptr[i]) 
            return 0;

#line 53 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        i = i + 1;
    }

#line 55 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return 1;
}

#line 58 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline _Bool Str__ends_with(Str * this, Str suffix) {

#line 59 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    if (suffix.len > this->len) 
        return 0;

#line 60 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize off = this->len - suffix.len;

#line 61 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize i = 0;

#line 62 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (i < suffix.len) {

#line 63 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        if (this->ptr[off + i] != suffix.ptr[i]) 
            return 0;

#line 64 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        i = i + 1;
    }

#line 66 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return 1;
}

#line 69 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline Str Str__slice(Str * this, isize start, isize end) {

#line 70 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    if (start < 0 || end < start || end > this->len) 
        __builtin_trap();

#line 71 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    Str s;

#line 72 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.ptr = this->ptr + start;

#line 73 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.len = end - start;

#line 74 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return s;
}

#line 77 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline Str str_from_lit(const char * lit) {

#line 78 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    Str s;

#line 79 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.ptr = lit;

#line 80 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.len = 0;

#line 81 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 82 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return s;
}

#line 85 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline Str str_from_parts(const char * p, isize len) {

#line 86 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    Str s;

#line 87 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.ptr = p;

#line 88 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    s.len = len;

#line 89 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return s;
}

#line 98 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
static inline struct StrSplit str_split_once(Str s, char sep) {

#line 99 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    isize idx = Str__find_byte(&s, sep);

#line 100 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    if (idx < 0) {

#line 101 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        struct StrSplit r;

#line 102 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        r.left = s;

#line 103 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        r.right = str_from_parts(s.ptr + s.len, 0);

#line 104 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        r.found = 0;

#line 105 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
        return r;
    }

#line 107 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    struct StrSplit r;

#line 108 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    r.left = str_from_parts(s.ptr, idx);

#line 109 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    r.right = str_from_parts(s.ptr + idx + 1, s.len - idx - 1);

#line 110 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    r.found = 1;

#line 111 "sharp-test/c_superset_probes/../../sharp/std/str.sph"
    return r;
}

#line 17 "sharp-test/c_superset_probes/p221_std_include.sp"
int main(void) {

#line 19 "sharp-test/c_superset_probes/p221_std_include.sp"
    isize len = 10;

#line 20 "sharp-test/c_superset_probes/p221_std_include.sp"
    usize cap = 100;

#line 21 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (len != 10) 
        return 1;

#line 22 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (cap != 100) 
        return 2;

#line 25 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__int vi = { 0 };

#line 26 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__int__push(&vi, 42);

#line 27 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__int__push(&vi, 73);

#line 28 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Vec__int__size(&vi) != 2) 
        return 3;

#line 29 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Vec__int__get(&vi, 0) != 42) 
        return 4;

#line 30 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Vec__int__get(&vi, 1) != 73) 
        return 5;

#line 31 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__int__destroy(&vi);

#line 33 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__float vf = { 0 };

#line 34 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__float__push(&vf, 1.5f);

#line 35 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Vec__float__size(&vf) != 1) 
        return 6;

#line 36 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Vec__float__op_idx(&vf, 0) < 1.4f || Vec__float__op_idx(&vf, 0) > 1.6f) 
        return 7;

#line 37 "sharp-test/c_superset_probes/p221_std_include.sp"
    Vec__float__destroy(&vf);

#line 40 "sharp-test/c_superset_probes/p221_std_include.sp"
    Str hello = str_from_lit("Hello");

#line 41 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (hello.len != 5) 
        return 8;

#line 42 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Str__find_byte(&hello, 'e') != 1) 
        return 9;

#line 43 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Str__find_byte(&hello, 'z') != -1) 
        return 10;

#line 44 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (Str__size(&hello) != 5) 
        return 11;

#line 46 "sharp-test/c_superset_probes/p221_std_include.sp"
    Str ell = Str__slice(&hello, 1, 4);

#line 47 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (ell.len != 3) 
        return 12;

#line 48 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (ell.ptr[0] != 'e') 
        return 13;

#line 50 "sharp-test/c_superset_probes/p221_std_include.sp"
    struct StrSplit sp = str_split_once(hello, 'l');

#line 51 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (!sp.found) 
        return 14;

#line 52 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (sp.left.len != 2) 
        return 15;

#line 55 "sharp-test/c_superset_probes/p221_std_include.sp"
    Str key = str_from_lit("answer");

#line 56 "sharp-test/c_superset_probes/p221_std_include.sp"
    HashMapEntry__Str__int entry;

#line 57 "sharp-test/c_superset_probes/p221_std_include.sp"
    entry.key = key;

#line 58 "sharp-test/c_superset_probes/p221_std_include.sp"
    entry.value = 42;

#line 59 "sharp-test/c_superset_probes/p221_std_include.sp"
    entry.state = 1;

#line 60 "sharp-test/c_superset_probes/p221_std_include.sp"
    if (entry.value != 42) 
        return 16;

#line 62 "sharp-test/c_superset_probes/p221_std_include.sp"
    return 0;
}

#line 11 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) void Vec__int__push(Vec__int * this, int val) {

#line 12 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 15 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->len = this->len + 1;
}


#line 49 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) long Vec__int__size(Vec__int * this) {

#line 50 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    return this->len;
}


#line 29 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) int Vec__int__get(Vec__int * this, long i) {

#line 30 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    return this->data[i];
}


#line 73 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) void Vec__int__destroy(Vec__int * this) {

#line 74 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    free(this->data);

#line 75 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->data = (int *)0;

#line 76 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->len = 0;

#line 77 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->cap = 0;
}


#line 11 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) void Vec__float__push(Vec__float * this, float val) {

#line 12 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        float * nd = (float *)realloc(this->data, sizeof(float) * new_cap);

#line 15 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->len = this->len + 1;
}


#line 49 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) long Vec__float__size(Vec__float * this) {

#line 50 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    return this->len;
}


#line 44 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) float Vec__float__op_idx(Vec__float * this, long i) {

#line 45 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 46 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    return this->data[i];
}


#line 73 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
__attribute__((weak)) void Vec__float__destroy(Vec__float * this) {

#line 74 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    free(this->data);

#line 75 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->data = (float *)0;

#line 76 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->len = 0;

#line 77 "sharp-test/c_superset_probes/../../sharp/std/vec.sph"
    this->cap = 0;
}

