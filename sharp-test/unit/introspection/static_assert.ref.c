#include <stdbool.h>

#include <stdlib.h>

#line 6 "unit/introspection/static_assert.sp"
typedef long isize;

#line 7 "unit/introspection/static_assert.sp"
typedef unsigned long usize;

#line 10 "unit/introspection/static_assert.sp"
typedef struct A A;
struct A {
    int x;
};

#line 11 "unit/introspection/static_assert.sp"
typedef struct B B;
struct B {
    int x;
};

#line 14 "unit/introspection/static_assert.sp"
void * malloc(unsigned long);

#line 15 "unit/introspection/static_assert.sp"
void * calloc(unsigned long, unsigned long);

#line 16 "unit/introspection/static_assert.sp"
void * realloc(void *, unsigned long);

#line 17 "unit/introspection/static_assert.sp"
void free(void *);

#line 18 "unit/introspection/static_assert.sp"
void * memset(void *, int, unsigned long);

#line 21 "unit/introspection/static_assert.sp"
typedef struct Kt Kt;
struct Kt {
    int id;
};

#line 24 "unit/introspection/static_assert.sp"
usize Kt__hash(Kt * this);

#line 27 "unit/introspection/static_assert.sp"
_Bool Kt__op_eq(Kt * this, Kt other);

#line 31 "unit/introspection/static_assert.sp"
struct MapEntry {
    Kt key;
    int value;
    unsigned char state;
};

#line 37 "unit/introspection/static_assert.sp"
typedef struct Map Map;
struct Map {
    struct MapEntry * entries;
    isize cap;
    isize len;
};

#line 43 "unit/introspection/static_assert.sp"
_Bool Map__insert(Map * this, Kt key, int value);

#line 68 "unit/introspection/static_assert.sp"
int * Map__get(Map * this, Kt key);

#line 80 "unit/introspection/static_assert.sp"
void Map__destroy(Map * this);

#line 14 "unit/introspection/static_assert.sp"
void * malloc(unsigned long);

#line 15 "unit/introspection/static_assert.sp"
void * calloc(unsigned long, unsigned long);

#line 16 "unit/introspection/static_assert.sp"
void * realloc(void *, unsigned long);

#line 17 "unit/introspection/static_assert.sp"
void free(void *);

#line 18 "unit/introspection/static_assert.sp"
void * memset(void *, int, unsigned long);

#line 24 "unit/introspection/static_assert.sp"
usize Kt__hash(Kt * this) {

#line 25 "unit/introspection/static_assert.sp"
    return (usize)this->id * (usize)2654435761U;
}

#line 27 "unit/introspection/static_assert.sp"
_Bool Kt__op_eq(Kt * this, Kt other) {

#line 28 "unit/introspection/static_assert.sp"
    return this->id == other.id;
}

#line 43 "unit/introspection/static_assert.sp"
_Bool Map__insert(Map * this, Kt key, int value) {

#line 44 "unit/introspection/static_assert.sp"
    /* @static_assert */ 1;

#line 45 "unit/introspection/static_assert.sp"
    /* @static_assert */ 1;

#line 47 "unit/introspection/static_assert.sp"
    if (this->cap == 0) {

#line 48 "unit/introspection/static_assert.sp"
        this->cap = 8;

#line 49 "unit/introspection/static_assert.sp"
        this->entries = (struct MapEntry *)calloc(8, sizeof(struct MapEntry));

#line 50 "unit/introspection/static_assert.sp"
        if (!this->entries) 
            return 0;
    }

#line 52 "unit/introspection/static_assert.sp"
    usize h = Kt__hash(&key);

#line 53 "unit/introspection/static_assert.sp"
    isize i = (isize)(h & (usize)(this->cap - 1));

#line 54 "unit/introspection/static_assert.sp"
    while (this->entries[i].state == 1) {

#line 55 "unit/introspection/static_assert.sp"
        if (Kt__op_eq(&this->entries[i].key, key)) {

#line 56 "unit/introspection/static_assert.sp"
            this->entries[i].value = value;

#line 57 "unit/introspection/static_assert.sp"
            return 0;
        }

#line 59 "unit/introspection/static_assert.sp"
        i = (i + 1) & (this->cap - 1);
    }

#line 61 "unit/introspection/static_assert.sp"
    this->entries[i].key = key;

#line 62 "unit/introspection/static_assert.sp"
    this->entries[i].value = value;

#line 63 "unit/introspection/static_assert.sp"
    this->entries[i].state = 1;

#line 64 "unit/introspection/static_assert.sp"
    this->len = this->len + 1;

#line 65 "unit/introspection/static_assert.sp"
    return 1;
}

#line 68 "unit/introspection/static_assert.sp"
int * Map__get(Map * this, Kt key) {

#line 69 "unit/introspection/static_assert.sp"
    if (this->cap == 0) 
        return (int *)0;

#line 70 "unit/introspection/static_assert.sp"
    usize h = Kt__hash(&key);

#line 71 "unit/introspection/static_assert.sp"
    isize i = (isize)(h & (usize)(this->cap - 1));

#line 72 "unit/introspection/static_assert.sp"
    while (this->entries[i].state != 0) {

#line 73 "unit/introspection/static_assert.sp"
        if (this->entries[i].state == 1 && Kt__op_eq(&this->entries[i].key, key)) 

#line 74 "unit/introspection/static_assert.sp"
            return &this->entries[i].value;

#line 75 "unit/introspection/static_assert.sp"
        i = (i + 1) & (this->cap - 1);
    }

#line 77 "unit/introspection/static_assert.sp"
    return (int *)0;
}

#line 80 "unit/introspection/static_assert.sp"
void Map__destroy(Map * this) {
    free(this->entries);
    this->entries = (struct MapEntry *)0;
    this->cap = 0;
    this->len = 0;
}

#line 82 "unit/introspection/static_assert.sp"
int main() {

#line 84 "unit/introspection/static_assert.sp"
    /* @static_assert */ 1;

#line 85 "unit/introspection/static_assert.sp"
    /* @static_assert */ 1;

#line 86 "unit/introspection/static_assert.sp"
    /* @static_assert */ 1;

#line 87 "unit/introspection/static_assert.sp"

#line 88 "unit/introspection/static_assert.sp"
    if (!/* @has_field */ 0) {
    }
    else {
        return 98;
    }

#line 91 "unit/introspection/static_assert.sp"
    Map m = { 0 };

#line 92 "unit/introspection/static_assert.sp"
    Kt k1;
    k1.id = 1;

#line 93 "unit/introspection/static_assert.sp"
    Kt k2;
    k2.id = 2;

#line 94 "unit/introspection/static_assert.sp"
    Kt k3;
    k3.id = 3;

#line 96 "unit/introspection/static_assert.sp"
    if (!Map__insert(&m, k1, 100)) 
        return 1;

#line 97 "unit/introspection/static_assert.sp"
    if (!Map__insert(&m, k2, 200)) 
        return 2;

#line 98 "unit/introspection/static_assert.sp"
    if (!Map__insert(&m, k3, 300)) 
        return 3;

#line 100 "unit/introspection/static_assert.sp"
    int * v1 = Map__get(&m, k1);
    if (!v1 || *v1 != 100) 
        return 4;

#line 101 "unit/introspection/static_assert.sp"
    int * v2 = Map__get(&m, k2);
    if (!v2 || *v2 != 200) 
        return 5;

#line 102 "unit/introspection/static_assert.sp"
    int * v3 = Map__get(&m, k3);
    if (!v3 || *v3 != 300) 
        return 6;

#line 104 "unit/introspection/static_assert.sp"
    Kt miss;
    miss.id = 99;

#line 105 "unit/introspection/static_assert.sp"
    if (Map__get(&m, miss) != (int *)0) 
        return 7;

#line 108 "unit/introspection/static_assert.sp"
    if (Map__insert(&m, k2, 222)) 
        return 8;

#line 109 "unit/introspection/static_assert.sp"
    int * v2b = Map__get(&m, k2);

#line 110 "unit/introspection/static_assert.sp"
    if (!v2b || *v2b != 222) 
        return 9;

#line 112 "unit/introspection/static_assert.sp"
    Map__destroy(&m);

#line 113 "unit/introspection/static_assert.sp"
    return 0;
}
