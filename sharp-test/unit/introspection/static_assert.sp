// 来源: p253_static_assert.sp, p220_hashmap.sp (关键内省部分)
// @static_assert 复合条件 + 编译期内省实战(基于 HashMap)

#include <stdbool.h>

typedef long isize;
typedef unsigned long usize;

// === p253: @static_assert 复合条件 ===
class A { int x; };
class B { int x; };

// === p220: 基于内省的 HashMap (简化版) ===
void* malloc(unsigned long);
void* calloc(unsigned long, unsigned long);
void* realloc(void*, unsigned long);
void  free(void*);
void* memset(void*, int, unsigned long);

// Key type with hash() and operator==
class Kt {
    int id;
};
usize Kt.hash(this) const {
    return (usize)this->id * (usize)2654435761U;
}
bool Kt.operator==(this, Kt other) const {
    return this->id == other.id;
}

struct MapEntry {
    Kt key;
    int value;
    unsigned char state;  /* 0=EMPTY, 1=OCCUPIED */
};

class Map {
    MapEntry* entries;
    isize cap;
    isize len;
};

bool Map.insert(this, Kt key, int value) {
    @static_assert(@has_method(Kt, hash), "Map key type needs hash()");
    @static_assert(@has_operator(Kt, ==), "Map key type needs operator==");
    // simplified insert
    if (this->cap == 0) {
        this->cap = 8;
        this->entries = (MapEntry*)calloc(8, sizeof(MapEntry));
        if (!this->entries) return false;
    }
    usize h = key.hash();
    isize i = (isize)(h & (usize)(this->cap - 1));
    while (this->entries[i].state == 1) {
        if (this->entries[i].key == key) {
            this->entries[i].value = value;
            return false; // overwrite, not new
        }
        i = (i + 1) & (this->cap - 1);
    }
    this->entries[i].key = key;
    this->entries[i].value = value;
    this->entries[i].state = 1;
    this->len = this->len + 1;
    return true;
}

int* Map.get(this, Kt key) {
    if (this->cap == 0) return (int*)0;
    usize h = key.hash();
    isize i = (isize)(h & (usize)(this->cap - 1));
    while (this->entries[i].state != 0) {
        if (this->entries[i].state == 1 && this->entries[i].key == key)
            return &this->entries[i].value;
        i = (i + 1) & (this->cap - 1);
    }
    return (int*)0;
}

void Map.destroy(this) { free(this->entries); this->entries = (MapEntry*)0; this->cap = 0; this->len = 0; }

int main() {
    // --- p253: compound @static_assert ---
    @static_assert(@has_field(A, x) && @has_field(B, x), "A and B must have x");
    @static_assert(1 || 0, "1 || 0 must be true");
    @static_assert(!0, "!0 must be true");
    if (@has_field(A, missing)) { return 99; }
    if (!@has_field(A, missing)) { } else { return 98; }

    // --- p220: Map runtime test ---
    Map m = {0};
    Kt k1; k1.id = 1;
    Kt k2; k2.id = 2;
    Kt k3; k3.id = 3;

    if (!m.insert(k1, 100)) return 1;
    if (!m.insert(k2, 200)) return 2;
    if (!m.insert(k3, 300)) return 3;

    int* v1 = m.get(k1); if (!v1 || *v1 != 100) return 4;
    int* v2 = m.get(k2); if (!v2 || *v2 != 200) return 5;
    int* v3 = m.get(k3); if (!v3 || *v3 != 300) return 6;

    Kt miss; miss.id = 99;
    if (m.get(miss) != (int*)0) return 7;

    // overwrite
    if (m.insert(k2, 222)) return 8;  /* false: overwrite */
    int* v2b = m.get(k2);
    if (!v2b || *v2b != 222) return 9;

    m.destroy();
    return 0;
}