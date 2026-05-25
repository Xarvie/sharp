#include <stdbool.h>

#include <stdlib.h>

#line 16 "sharp-test/c_superset_probes/p220_hashmap.sp"
typedef long isize;

#line 17 "sharp-test/c_superset_probes/p220_hashmap.sp"
typedef unsigned long usize;

#line 20 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * malloc(unsigned long);

#line 21 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * calloc(unsigned long, unsigned long);

#line 22 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * realloc(void *, unsigned long);

#line 23 "sharp-test/c_superset_probes/p220_hashmap.sp"
void free(void *);

#line 24 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * memset(void *, int, unsigned long);

#line 27 "sharp-test/c_superset_probes/p220_hashmap.sp"
typedef struct Point Point;
struct Point {
    int x;
    int y;
};

#line 32 "sharp-test/c_superset_probes/p220_hashmap.sp"
usize Point__hash(Point * this);

#line 38 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool Point__op_eq(Point * this, Point other);

#line 43 "sharp-test/c_superset_probes/p220_hashmap.sp"

#line 49 "sharp-test/c_superset_probes/p220_hashmap.sp"

#line 81 "sharp-test/c_superset_probes/p220_hashmap.sp"

#line 81 "sharp-test/c_superset_probes/p220_hashmap.sp"

#line 43 "sharp-test/c_superset_probes/p220_hashmap.sp"
typedef struct HashMapEntry__Point__int HashMapEntry__Point__int;
struct HashMapEntry__Point__int {
    Point key;
    int value;
    unsigned char state;
};


typedef struct HashMap__Point__int HashMap__Point__int;
struct HashMap__Point__int {
    HashMapEntry__Point__int * entries;
    isize cap;
    isize len;
    isize tombstones;
};



#line 49 "sharp-test/c_superset_probes/p220_hashmap.sp"
typedef struct HashMapCursor__Point__int HashMapCursor__Point__int;
struct HashMapCursor__Point__int {
    HashMapEntry__Point__int * entries;
    isize cap;
    isize idx;
};



#line 129 "sharp-test/c_superset_probes/p220_hashmap.sp"
long HashMap__Point__int__size(HashMap__Point__int * this);

#line 130 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool HashMap__Point__int__is_empty(HashMap__Point__int * this);

#line 88 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool HashMap__Point__int__insert(HashMap__Point__int * this, Point key, int value);

#line 104 "sharp-test/c_superset_probes/p220_hashmap.sp"
int * HashMap__Point__int__get(HashMap__Point__int * this, Point key);

#line 125 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool HashMap__Point__int__contains(HashMap__Point__int * this, Point key);

#line 113 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool HashMap__Point__int__remove(HashMap__Point__int * this, Point key);

#line 149 "sharp-test/c_superset_probes/p220_hashmap.sp"
HashMapCursor__Point__int HashMap__Point__int__iter(HashMap__Point__int * this);

#line 55 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool HashMapCursor__Point__int__valid(HashMapCursor__Point__int * this);

#line 64 "sharp-test/c_superset_probes/p220_hashmap.sp"
int * HashMapCursor__Point__int__value(HashMapCursor__Point__int * this);

#line 69 "sharp-test/c_superset_probes/p220_hashmap.sp"
void HashMapCursor__Point__int__next(HashMapCursor__Point__int * this);

#line 132 "sharp-test/c_superset_probes/p220_hashmap.sp"
void HashMap__Point__int__clear(HashMap__Point__int * this);

#line 141 "sharp-test/c_superset_probes/p220_hashmap.sp"
void HashMap__Point__int__destroy(HashMap__Point__int * this);

#line 20 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * malloc(unsigned long);

#line 21 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * calloc(unsigned long, unsigned long);

#line 22 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * realloc(void *, unsigned long);

#line 23 "sharp-test/c_superset_probes/p220_hashmap.sp"
void free(void *);

#line 24 "sharp-test/c_superset_probes/p220_hashmap.sp"
void * memset(void *, int, unsigned long);

#line 32 "sharp-test/c_superset_probes/p220_hashmap.sp"
usize Point__hash(Point * this) {

#line 33 "sharp-test/c_superset_probes/p220_hashmap.sp"
    usize a = (usize)this->x;

#line 34 "sharp-test/c_superset_probes/p220_hashmap.sp"
    usize b = (usize)this->y;

#line 35 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return (a * (usize)2654435761U) ^ (b * (usize)40503U);
}

#line 38 "sharp-test/c_superset_probes/p220_hashmap.sp"
_Bool Point__op_eq(Point * this, Point other) {

#line 39 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return this->x == other.x && this->y == other.y;
}

#line 226 "sharp-test/c_superset_probes/p220_hashmap.sp"
int main() {

#line 227 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMap__Point__int m = { 0 };

#line 230 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 0) 
        return 1;

#line 231 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__is_empty(&m)) 
        return 2;

#line 234 "sharp-test/c_superset_probes/p220_hashmap.sp"
    Point p1;
    p1.x = 1;
    p1.y = 2;

#line 235 "sharp-test/c_superset_probes/p220_hashmap.sp"
    Point p2;
    p2.x = 3;
    p2.y = 4;

#line 236 "sharp-test/c_superset_probes/p220_hashmap.sp"
    Point p3;
    p3.x = 5;
    p3.y = 6;

#line 238 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__insert(&m, p1, 100)) 
        return 3;

#line 239 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__insert(&m, p2, 200)) 
        return 4;

#line 240 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__insert(&m, p3, 300)) 
        return 5;

#line 241 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 3) 
        return 6;

#line 243 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v1 = HashMap__Point__int__get(&m, p1);
    if (!v1 || *v1 != 100) 
        return 10;

#line 244 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v2 = HashMap__Point__int__get(&m, p2);
    if (!v2 || *v2 != 200) 
        return 11;

#line 245 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v3 = HashMap__Point__int__get(&m, p3);
    if (!v3 || *v3 != 300) 
        return 12;

#line 248 "sharp-test/c_superset_probes/p220_hashmap.sp"
    Point miss;
    miss.x = 99;
    miss.y = 99;

#line 249 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__get(&m, miss) != (int *)0) 
        return 13;

#line 250 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__contains(&m, miss)) 
        return 14;

#line 253 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__insert(&m, p2, 222)) 
        return 20;

#line 254 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 3) 
        return 21;

#line 255 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v2b = HashMap__Point__int__get(&m, p2);

#line 256 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!v2b || *v2b != 222) 
        return 22;

#line 259 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__remove(&m, p1)) 
        return 30;

#line 260 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 2) 
        return 31;

#line 261 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__contains(&m, p1)) 
        return 32;

#line 262 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__remove(&m, p1)) 
        return 33;

#line 263 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__insert(&m, p1, 111)) 
        return 34;

#line 264 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 3) 
        return 35;

#line 265 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v1b = HashMap__Point__int__get(&m, p1);

#line 266 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!v1b || *v1b != 111) 
        return 36;

#line 269 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int i = 0;

#line 270 "sharp-test/c_superset_probes/p220_hashmap.sp"
    while (i < 16) {

#line 271 "sharp-test/c_superset_probes/p220_hashmap.sp"
        Point p;
        p.x = 1000 + i;
        p.y = 2000 + i;

#line 272 "sharp-test/c_superset_probes/p220_hashmap.sp"
        if (!HashMap__Point__int__insert(&m, p, 5000 + i)) 
            return 40 + i;

#line 273 "sharp-test/c_superset_probes/p220_hashmap.sp"
        i = i + 1;
    }

#line 275 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 19) 
        return 60;

#line 278 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v1c = HashMap__Point__int__get(&m, p1);
    if (!v1c || *v1c != 111) 
        return 61;

#line 279 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v2c = HashMap__Point__int__get(&m, p2);
    if (!v2c || *v2c != 222) 
        return 62;

#line 280 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v3c = HashMap__Point__int__get(&m, p3);
    if (!v3c || *v3c != 300) 
        return 63;

#line 283 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int sum = 0;

#line 284 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int count = 0;

#line 285 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMapCursor__Point__int c = HashMap__Point__int__iter(&m);

#line 286 "sharp-test/c_superset_probes/p220_hashmap.sp"
    while (HashMapCursor__Point__int__valid(&c)) {

#line 287 "sharp-test/c_superset_probes/p220_hashmap.sp"
        sum = sum + *HashMapCursor__Point__int__value(&c);

#line 288 "sharp-test/c_superset_probes/p220_hashmap.sp"
        count = count + 1;

#line 289 "sharp-test/c_superset_probes/p220_hashmap.sp"
        HashMapCursor__Point__int__next(&c);
    }

#line 291 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (count != 19) 
        return 70;

#line 292 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (sum != 80753) 
        return 71;

#line 295 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMap__Point__int__clear(&m);

#line 296 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 0) 
        return 80;

#line 297 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__contains(&m, p1)) 
        return 81;

#line 298 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (m.cap == 0) 
        return 82;

#line 300 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMap__Point__int__insert(&m, p1, 999)) 
        return 83;

#line 301 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 1) 
        return 84;

#line 302 "sharp-test/c_superset_probes/p220_hashmap.sp"
    int * v1d = HashMap__Point__int__get(&m, p1);

#line 303 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!v1d || *v1d != 999) 
        return 85;

#line 305 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMap__Point__int__destroy(&m);

#line 306 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (HashMap__Point__int__size(&m) != 0) 
        return 90;

#line 307 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (m.cap != 0) 
        return 91;

#line 309 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return 0;
}

#line 129 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) long HashMap__Point__int__size(HashMap__Point__int * this) {
    return this->len;
}


#line 130 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) _Bool HashMap__Point__int__is_empty(HashMap__Point__int * this) {
    return this->len == 0;
}


#line 88 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) _Bool HashMap__Point__int__insert(HashMap__Point__int * this, Point key, int value) {

#line 89 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "HashMap key type must define `usize hash() const`");

#line 91 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "HashMap key type must define `bool operator==(K) const`");

#line 94 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMap__Point__int___grow_if_needed(this);

#line 95 "sharp-test/c_superset_probes/p220_hashmap.sp"
    isize i = HashMap__Point__int___probe_for_insert(this, key);

#line 96 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Bool is_new = (this->entries[i].state != (unsigned char)1);

#line 97 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->entries[i].key = key;

#line 98 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->entries[i].value = value;

#line 99 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->entries[i].state = (unsigned char)1;

#line 100 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (is_new) 
        this->len = this->len + 1;

#line 101 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return is_new;
}


#line 104 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) int * HashMap__Point__int__get(HashMap__Point__int * this, Point key) {

#line 105 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "K needs hash()");

#line 106 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "K needs operator==");

#line 107 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (this->cap == 0) 
        return (int *)0;

#line 108 "sharp-test/c_superset_probes/p220_hashmap.sp"
    isize i = HashMap__Point__int___probe_for_lookup(this, key);

#line 109 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (i < 0) 
        return (int *)0;

#line 110 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return &this->entries[i].value;
}


#line 125 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) _Bool HashMap__Point__int__contains(HashMap__Point__int * this, Point key) {

#line 126 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return HashMap__Point__int__get(this, key) != (int *)0;
}


#line 113 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) _Bool HashMap__Point__int__remove(HashMap__Point__int * this, Point key) {

#line 114 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "K needs hash()");

#line 115 "sharp-test/c_superset_probes/p220_hashmap.sp"
    _Static_assert(0, "K needs operator==");

#line 116 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (this->cap == 0) 
        return 0;

#line 117 "sharp-test/c_superset_probes/p220_hashmap.sp"
    isize i = HashMap__Point__int___probe_for_lookup(this, key);

#line 118 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (i < 0) 
        return 0;

#line 119 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->entries[i].state = (unsigned char)2;

#line 120 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->len = this->len - 1;

#line 121 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->tombstones = this->tombstones + 1;

#line 122 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return 1;
}


#line 149 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) HashMapCursor__Point__int HashMap__Point__int__iter(HashMap__Point__int * this) {

#line 150 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMapCursor__Point__int c;

#line 151 "sharp-test/c_superset_probes/p220_hashmap.sp"
    c.entries = this->entries;

#line 152 "sharp-test/c_superset_probes/p220_hashmap.sp"
    c.cap = this->cap;

#line 153 "sharp-test/c_superset_probes/p220_hashmap.sp"
    c.idx = -1;

#line 154 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMapCursor__Point__int___advance(&c);

#line 155 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return c;
}


#line 55 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) _Bool HashMapCursor__Point__int__valid(HashMapCursor__Point__int * this) {

#line 56 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return this->idx >= 0 && this->idx < this->cap;
}


#line 64 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) int * HashMapCursor__Point__int__value(HashMapCursor__Point__int * this) {

#line 65 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (!HashMapCursor__Point__int__valid(this)) 
        return (int *)0;

#line 66 "sharp-test/c_superset_probes/p220_hashmap.sp"
    return &this->entries[this->idx].value;
}


#line 69 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) void HashMapCursor__Point__int__next(HashMapCursor__Point__int * this) {

#line 70 "sharp-test/c_superset_probes/p220_hashmap.sp"
    HashMapCursor__Point__int___advance(this);
}


#line 132 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) void HashMap__Point__int__clear(HashMap__Point__int * this) {

#line 133 "sharp-test/c_superset_probes/p220_hashmap.sp"
    if (this->cap > 0 && this->entries != (HashMapEntry__Point__int *)0) {

#line 134 "sharp-test/c_superset_probes/p220_hashmap.sp"
        memset(this->entries, 0, (unsigned long)this->cap * sizeof(HashMapEntry__Point__int));
    }

#line 137 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->len = 0;

#line 138 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->tombstones = 0;
}


#line 141 "sharp-test/c_superset_probes/p220_hashmap.sp"
__attribute__((weak)) void HashMap__Point__int__destroy(HashMap__Point__int * this) {

#line 142 "sharp-test/c_superset_probes/p220_hashmap.sp"
    free(this->entries);

#line 143 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->entries = (HashMapEntry__Point__int *)0;

#line 144 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->cap = 0;

#line 145 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->len = 0;

#line 146 "sharp-test/c_superset_probes/p220_hashmap.sp"
    this->tombstones = 0;
}

