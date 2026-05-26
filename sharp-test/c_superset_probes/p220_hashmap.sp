/*
 * p220 — HashMap end-to-end test.  Exercises every v0.13 primitive:
 *   - @has_method / @has_operator for compile-time hashability check
 *   - @static_assert as the gatekeeper inside generic methods
 *   - Per-instance specialization with method dispatch (key.hash(),
 *     entries[i].key == key)
 *   - Generic struct field-type recursion (HashMapEntry<K,V> inside
 *     HashMap<K,V>'s buffer)
 *
 * Avoids stdio so the test runs under sharpc's zig-cc-link path without
 * needing multiarch glibc headers.  Result via exit code.
 */
#include <stdbool.h>

typedef long isize;
typedef unsigned long usize;

/* libc decls inline — avoids depending on multiarch glibc header paths. */
void* malloc(unsigned long);
void* calloc(unsigned long, unsigned long);
void* realloc(void*, unsigned long);
void  free(void*);
void* memset(void*, int, unsigned long);

/* ── A key type with hash() and operator== ──────────────────────────── */
class Point {
    int x;
    int y;
}

usize Point.hash(this) const {
    usize a = (usize)this->x;
    usize b = (usize)this->y;
    return (a * (usize)2654435761U) ^ (b * (usize)40503U);
}

bool Point.operator==(this, Point other) const {
    return this->x == other.x && this->y == other.y;
}

/* ── HashMap (inlined from sharp/std/hashmap.sp) ─────────────────────── */
struct HashMapEntry<K, V> {
    K              key;
    V              value;
    unsigned char  state;        /* 0=EMPTY, 1=OCCUPIED, 2=TOMBSTONE */
}

class HashMapCursor<K, V> {
    HashMapEntry<K, V>* entries;
    isize cap;
    isize idx;                   /* -1 = before-first */
}

bool HashMapCursor<K, V>.valid(this) const {
    return this->idx >= 0 && this->idx < this->cap;
}

K* HashMapCursor<K, V>.key(this) const {
    if (!this->valid()) return (K*)0;
    return &this->entries[this->idx].key;
}

V* HashMapCursor<K, V>.value(this) const {
    if (!this->valid()) return (V*)0;
    return &this->entries[this->idx].value;
}

void HashMapCursor<K, V>.next(this) {
    this->_advance();
}

void HashMapCursor<K, V>._advance(this) {
    this->idx = this->idx + 1;
    while (this->idx < this->cap &&
           this->entries[this->idx].state != (unsigned char)1) {
        this->idx = this->idx + 1;
    }
}

class HashMap<K, V> {
    HashMapEntry<K, V>* entries;
    isize cap;
    isize len;
    isize tombstones;
}

bool HashMap<K, V>.insert(this, K key, V value) {
    @static_assert(@has_method(K, hash),
        "HashMap key type must define `usize hash() const`");
    @static_assert(@has_operator(K, ==),
        "HashMap key type must define `bool operator==(K) const`");

    this->_grow_if_needed();
    isize i = this->_probe_for_insert(key);
    bool is_new = (this->entries[i].state != (unsigned char)1);
    this->entries[i].key   = key;
    this->entries[i].value = value;
    this->entries[i].state = (unsigned char)1;
    if (is_new) this->len = this->len + 1;
    return is_new;
}

V* HashMap<K, V>.get(this, K key) {
    @static_assert(@has_method(K, hash), "K needs hash()");
    @static_assert(@has_operator(K, ==), "K needs operator==");
    if (this->cap == 0) return (V*)0;
    isize i = this->_probe_for_lookup(key);
    if (i < 0) return (V*)0;
    return &this->entries[i].value;
}

bool HashMap<K, V>.remove(this, K key) {
    @static_assert(@has_method(K, hash), "K needs hash()");
    @static_assert(@has_operator(K, ==), "K needs operator==");
    if (this->cap == 0) return false;
    isize i = this->_probe_for_lookup(key);
    if (i < 0) return false;
    this->entries[i].state = (unsigned char)2;
    this->len = this->len - 1;
    this->tombstones = this->tombstones + 1;
    return true;
}

bool HashMap<K, V>.contains(this, K key) {
    return this->get(key) != (V*)0;
}

isize HashMap<K, V>.size(this) const { return this->len; }
bool HashMap<K, V>.is_empty(this) const { return this->len == 0; }

void HashMap<K, V>.clear(this) {
    if (this->cap > 0 && this->entries != (HashMapEntry<K,V>*)0) {
        memset(this->entries, 0,
               (unsigned long)this->cap * sizeof(HashMapEntry<K, V>));
    }
    this->len = 0;
    this->tombstones = 0;
}

void HashMap<K, V>.destroy(this) {
    free(this->entries);
    this->entries = (HashMapEntry<K, V>*)0;
    this->cap = 0;
    this->len = 0;
    this->tombstones = 0;
}

HashMapCursor<K, V> HashMap<K, V>.iter(this) const {
    HashMapCursor<K, V> c;
    c.entries = this->entries;
    c.cap     = this->cap;
    c.idx     = -1;
    c._advance();
    return c;
}

isize HashMap<K, V>._probe_for_insert(this, K key) const {
    usize h = key.hash();
    isize mask = this->cap - 1;
    isize i = (isize)(h & (usize)mask);
    isize first_tomb = -1;
    while (true) {
        unsigned char st = this->entries[i].state;
        if (st == (unsigned char)0) {
            return first_tomb >= 0 ? first_tomb : i;
        }
        if (st == (unsigned char)2) {
            if (first_tomb < 0) first_tomb = i;
        } else {
            if (this->entries[i].key == key) return i;
        }
        i = (i + 1) & mask;
    }
}

isize HashMap<K, V>._probe_for_lookup(this, K key) const {
    usize h = key.hash();
    isize mask = this->cap - 1;
    isize i = (isize)(h & (usize)mask);
    while (true) {
        unsigned char st = this->entries[i].state;
        if (st == (unsigned char)0) return -1;
        if (st == (unsigned char)1 && this->entries[i].key == key) return i;
        i = (i + 1) & mask;
    }
}

void HashMap<K, V>._grow_if_needed(this) {
    isize used = this->len + this->tombstones;
    if ((used + 1) * 4 <= this->cap * 3 && this->cap > 0) return;
    isize new_cap = this->cap > 0 ? this->cap * 2 : 8;
    this->_rehash(new_cap);
}

void HashMap<K, V>._rehash(this, isize new_cap) {
    HashMapEntry<K, V>* nbuf = (HashMapEntry<K, V>*)
        calloc((unsigned long)new_cap, sizeof(HashMapEntry<K, V>));
    if (!nbuf) __builtin_trap();

    isize old_cap = this->cap;
    HashMapEntry<K, V>* old_buf = this->entries;

    this->entries = nbuf;
    this->cap = new_cap;
    this->len = 0;
    this->tombstones = 0;

    if (old_buf != (HashMapEntry<K, V>*)0) {
        isize j = 0;
        while (j < old_cap) {
            if (old_buf[j].state == (unsigned char)1) {
                isize i = this->_probe_for_insert(old_buf[j].key);
                this->entries[i].key   = old_buf[j].key;
                this->entries[i].value = old_buf[j].value;
                this->entries[i].state = (unsigned char)1;
                this->len = this->len + 1;
            }
            j = j + 1;
        }
        free(old_buf);
    }
}

/* ── Tests ───────────────────────────────────────────────────────────── */
int main() {
    HashMap<Point, int> m = {0};

    /* {0} init */
    if (m.size() != 0) return 1;
    if (!m.is_empty()) return 2;

    /* Initial inserts (cap allocated lazily) */
    Point p1; p1.x = 1; p1.y = 2;
    Point p2; p2.x = 3; p2.y = 4;
    Point p3; p3.x = 5; p3.y = 6;

    if (!m.insert(p1, 100)) return 3;
    if (!m.insert(p2, 200)) return 4;
    if (!m.insert(p3, 300)) return 5;
    if (m.size() != 3) return 6;

    int* v1 = m.get(p1); if (!v1 || *v1 != 100) return 10;
    int* v2 = m.get(p2); if (!v2 || *v2 != 200) return 11;
    int* v3 = m.get(p3); if (!v3 || *v3 != 300) return 12;

    /* Lookup miss */
    Point miss; miss.x = 99; miss.y = 99;
    if (m.get(miss) != (int*)0) return 13;
    if (m.contains(miss)) return 14;

    /* Overwrite */
    if (m.insert(p2, 222)) return 20;
    if (m.size() != 3) return 21;
    int* v2b = m.get(p2);
    if (!v2b || *v2b != 222) return 22;

    /* Remove + tombstone + reinsert */
    if (!m.remove(p1)) return 30;
    if (m.size() != 2) return 31;
    if (m.contains(p1)) return 32;
    if (m.remove(p1)) return 33;
    if (!m.insert(p1, 111)) return 34;
    if (m.size() != 3) return 35;
    int* v1b = m.get(p1);
    if (!v1b || *v1b != 111) return 36;

    /* Grow past initial capacity */
    int i = 0;
    while (i < 16) {
        Point p; p.x = 1000 + i; p.y = 2000 + i;
        if (!m.insert(p, 5000 + i)) return 40 + i;
        i = i + 1;
    }
    if (m.size() != 19) return 60;

    /* Verify originals after growth */
    int* v1c = m.get(p1); if (!v1c || *v1c != 111) return 61;
    int* v2c = m.get(p2); if (!v2c || *v2c != 222) return 62;
    int* v3c = m.get(p3); if (!v3c || *v3c != 300) return 63;

    /* Iteration: sum = 111+222+300 + (5000+5001+..+5015) = 633 + 80120 = 80753 */
    int sum = 0;
    int count = 0;
    HashMapCursor<Point, int> c = m.iter();
    while (c.valid()) {
        sum = sum + *c.value();
        count = count + 1;
        c.next();
    }
    if (count != 19) return 70;
    if (sum != 80753) return 71;

    /* clear() */
    m.clear();
    if (m.size() != 0) return 80;
    if (m.contains(p1)) return 81;
    if (m.cap == 0) return 82;

    if (!m.insert(p1, 999)) return 83;
    if (m.size() != 1) return 84;
    int* v1d = m.get(p1);
    if (!v1d || *v1d != 999) return 85;

    m.destroy();
    if (m.size() != 0) return 90;
    if (m.cap != 0) return 91;

    return 0;
}
