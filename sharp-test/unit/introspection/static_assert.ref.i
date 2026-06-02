
#line 6 "sharp-test/unit/introspection/static_assert.ce"
typedef long isize;

#line 7 "sharp-test/unit/introspection/static_assert.ce"
typedef unsigned long usize;

#line 10 "sharp-test/unit/introspection/static_assert.ce"
typedef struct A A;
struct A {
    int x;
};

#line 11 "sharp-test/unit/introspection/static_assert.ce"
typedef struct B B;
struct B {
    int x;
};

#line 14 "sharp-test/unit/introspection/static_assert.ce"
void * malloc(unsigned long);

#line 15 "sharp-test/unit/introspection/static_assert.ce"
void * calloc(unsigned long, unsigned long);

#line 16 "sharp-test/unit/introspection/static_assert.ce"
void * realloc(void *, unsigned long);

#line 17 "sharp-test/unit/introspection/static_assert.ce"
void free(void *);

#line 18 "sharp-test/unit/introspection/static_assert.ce"
void * memset(void *, int, unsigned long);

#line 21 "sharp-test/unit/introspection/static_assert.ce"
typedef struct Kt Kt;
struct Kt {
    int id;
};

#line 24 "sharp-test/unit/introspection/static_assert.ce"
usize Kt__hash(Kt * this);

#line 27 "sharp-test/unit/introspection/static_assert.ce"
bool Kt__op_eq(Kt * this, Kt other);

#line 31 "sharp-test/unit/introspection/static_assert.ce"
struct MapEntry {
    Kt key;
    int value;
    unsigned char state;
};

#line 37 "sharp-test/unit/introspection/static_assert.ce"
typedef struct Map Map;
struct Map {
    struct MapEntry * entries;
    isize cap;
    isize len;
};

#line 43 "sharp-test/unit/introspection/static_assert.ce"
bool Map__insert(Map * this, Kt key, int value);

#line 68 "sharp-test/unit/introspection/static_assert.ce"
int * Map__get(Map * this, Kt key);

#line 80 "sharp-test/unit/introspection/static_assert.ce"
void Map__destroy(Map * this);

#line 14 "sharp-test/unit/introspection/static_assert.ce"
void * malloc(unsigned long);

#line 15 "sharp-test/unit/introspection/static_assert.ce"
void * calloc(unsigned long, unsigned long);

#line 16 "sharp-test/unit/introspection/static_assert.ce"
void * realloc(void *, unsigned long);

#line 17 "sharp-test/unit/introspection/static_assert.ce"
void free(void *);

#line 18 "sharp-test/unit/introspection/static_assert.ce"
void * memset(void *, int, unsigned long);

#line 24 "sharp-test/unit/introspection/static_assert.ce"
usize Kt__hash(Kt * this) {

#line 25 "sharp-test/unit/introspection/static_assert.ce"
    return (usize)this->id * (usize)2654435761U;
}

#line 27 "sharp-test/unit/introspection/static_assert.ce"
bool Kt__op_eq(Kt * this, Kt other) {

#line 28 "sharp-test/unit/introspection/static_assert.ce"
    return this->id == other.id;
}

#line 43 "sharp-test/unit/introspection/static_assert.ce"
bool Map__insert(Map * this, Kt key, int value) {

#line 44 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 45 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 47 "sharp-test/unit/introspection/static_assert.ce"
    if (this->cap == 0) {

#line 48 "sharp-test/unit/introspection/static_assert.ce"
        this->cap = 8;

#line 49 "sharp-test/unit/introspection/static_assert.ce"
        this->entries = (struct MapEntry *)calloc(8, sizeof(struct MapEntry));

#line 50 "sharp-test/unit/introspection/static_assert.ce"
        if (!this->entries) 
            return false;
    }

#line 52 "sharp-test/unit/introspection/static_assert.ce"
    usize h = Kt__hash(&key);

#line 53 "sharp-test/unit/introspection/static_assert.ce"
    isize i = (isize)(h & (usize)(this->cap - 1));

#line 54 "sharp-test/unit/introspection/static_assert.ce"
    while (this->entries[i].state == 1) {

#line 55 "sharp-test/unit/introspection/static_assert.ce"
        if (Kt__op_eq(&this->entries[i].key, key)) {

#line 56 "sharp-test/unit/introspection/static_assert.ce"
            this->entries[i].value = value;

#line 57 "sharp-test/unit/introspection/static_assert.ce"
            return false;
        }

#line 59 "sharp-test/unit/introspection/static_assert.ce"
        i = (i + 1) & (this->cap - 1);
    }

#line 61 "sharp-test/unit/introspection/static_assert.ce"
    this->entries[i].key = key;

#line 62 "sharp-test/unit/introspection/static_assert.ce"
    this->entries[i].value = value;

#line 63 "sharp-test/unit/introspection/static_assert.ce"
    this->entries[i].state = 1;

#line 64 "sharp-test/unit/introspection/static_assert.ce"
    this->len = this->len + 1;

#line 65 "sharp-test/unit/introspection/static_assert.ce"
    return true;
}

#line 68 "sharp-test/unit/introspection/static_assert.ce"
int * Map__get(Map * this, Kt key) {

#line 69 "sharp-test/unit/introspection/static_assert.ce"
    if (this->cap == 0) 
        return (int *)0;

#line 70 "sharp-test/unit/introspection/static_assert.ce"
    usize h = Kt__hash(&key);

#line 71 "sharp-test/unit/introspection/static_assert.ce"
    isize i = (isize)(h & (usize)(this->cap - 1));

#line 72 "sharp-test/unit/introspection/static_assert.ce"
    while (this->entries[i].state != 0) {

#line 73 "sharp-test/unit/introspection/static_assert.ce"
        if (this->entries[i].state == 1 && Kt__op_eq(&this->entries[i].key, key)) 

#line 74 "sharp-test/unit/introspection/static_assert.ce"
            return &this->entries[i].value;

#line 75 "sharp-test/unit/introspection/static_assert.ce"
        i = (i + 1) & (this->cap - 1);
    }

#line 77 "sharp-test/unit/introspection/static_assert.ce"
    return (int *)0;
}

#line 80 "sharp-test/unit/introspection/static_assert.ce"
void Map__destroy(Map * this) {
    free(this->entries);
    this->entries = (struct MapEntry *)0;
    this->cap = 0;
    this->len = 0;
}

#line 82 "sharp-test/unit/introspection/static_assert.ce"
int main() {

#line 84 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 85 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 86 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 87 "sharp-test/unit/introspection/static_assert.ce"

#line 88 "sharp-test/unit/introspection/static_assert.ce"
    if (!/* @has_field */ 0) {
    }
    else {
        return 98;
    }

#line 91 "sharp-test/unit/introspection/static_assert.ce"
    Map m = { 0 };

#line 92 "sharp-test/unit/introspection/static_assert.ce"
    Kt k1;
    k1.id = 1;

#line 93 "sharp-test/unit/introspection/static_assert.ce"
    Kt k2;
    k2.id = 2;

#line 94 "sharp-test/unit/introspection/static_assert.ce"
    Kt k3;
    k3.id = 3;

#line 96 "sharp-test/unit/introspection/static_assert.ce"
    if (!Map__insert(&m, k1, 100)) 
        return 1;

#line 97 "sharp-test/unit/introspection/static_assert.ce"
    if (!Map__insert(&m, k2, 200)) 
        return 2;

#line 98 "sharp-test/unit/introspection/static_assert.ce"
    if (!Map__insert(&m, k3, 300)) 
        return 3;

#line 100 "sharp-test/unit/introspection/static_assert.ce"
    int * v1 = Map__get(&m, k1);
    if (!v1 || *v1 != 100) 
        return 4;

#line 101 "sharp-test/unit/introspection/static_assert.ce"
    int * v2 = Map__get(&m, k2);
    if (!v2 || *v2 != 200) 
        return 5;

#line 102 "sharp-test/unit/introspection/static_assert.ce"
    int * v3 = Map__get(&m, k3);
    if (!v3 || *v3 != 300) 
        return 6;

#line 104 "sharp-test/unit/introspection/static_assert.ce"
    Kt miss;
    miss.id = 99;

#line 105 "sharp-test/unit/introspection/static_assert.ce"
    if (Map__get(&m, miss) != (int *)0) 
        return 7;

#line 108 "sharp-test/unit/introspection/static_assert.ce"
    if (Map__insert(&m, k2, 222)) 
        return 8;

#line 109 "sharp-test/unit/introspection/static_assert.ce"
    int * v2b = Map__get(&m, k2);

#line 110 "sharp-test/unit/introspection/static_assert.ce"
    if (!v2b || *v2b != 222) 
        return 9;

#line 112 "sharp-test/unit/introspection/static_assert.ce"
    Map__destroy(&m);

#line 115 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 116 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 117 "sharp-test/unit/introspection/static_assert.ce"
    /* @static_assert */ 1;

#line 119 "sharp-test/unit/introspection/static_assert.ce"
    return 0;
}
