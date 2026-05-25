#include <stdbool.h>


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

#line 10 "sharp-test/c_superset_probes/p223_str_extensions.sp"
isize Str__count_byte(Str * this, char c);

#line 21 "sharp-test/c_superset_probes/p223_str_extensions.sp"
_Bool Str__contains(Str * this, Str sub);

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

#line 10 "sharp-test/c_superset_probes/p223_str_extensions.sp"
isize Str__count_byte(Str * this, char c) {

#line 11 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize n = 0;

#line 12 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize i = 0;

#line 13 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    while (i < this->len) {

#line 14 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        if (this->ptr[i] == c) 
            n = n + 1;

#line 15 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        i = i + 1;
    }

#line 17 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    return n;
}

#line 21 "sharp-test/c_superset_probes/p223_str_extensions.sp"
_Bool Str__contains(Str * this, Str sub) {

#line 22 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (sub.len > this->len) 
        return 0;

#line 23 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (sub.len == 0) 
        return 1;

#line 24 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize max_start = this->len - sub.len;

#line 25 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize start = 0;

#line 26 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    while (start <= max_start) {

#line 27 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        isize j = 0;

#line 28 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        _Bool match = 1;

#line 29 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        while (j < sub.len) {

#line 30 "sharp-test/c_superset_probes/p223_str_extensions.sp"
            if (this->ptr[start + j] != sub.ptr[j]) {

#line 31 "sharp-test/c_superset_probes/p223_str_extensions.sp"
                match = 0;

#line 32 "sharp-test/c_superset_probes/p223_str_extensions.sp"
                break;
            }

#line 34 "sharp-test/c_superset_probes/p223_str_extensions.sp"
            j = j + 1;
        }

#line 36 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        if (match) 
            return 1;

#line 37 "sharp-test/c_superset_probes/p223_str_extensions.sp"
        start = start + 1;
    }

#line 39 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    return 0;
}

#line 42 "sharp-test/c_superset_probes/p223_str_extensions.sp"
int main() {

#line 43 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str s = str_from_lit("hello world hello");

#line 46 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize n = Str__count_byte(&s, 'l');

#line 47 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (n != 5) 
        return 1;

#line 49 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    isize n2 = Str__count_byte(&s, 'z');

#line 50 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (n2 != 0) 
        return 2;

#line 53 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str world = str_from_lit("world");

#line 54 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (!Str__contains(&s, world)) 
        return 3;

#line 56 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str lo = str_from_lit("lo");

#line 57 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (!Str__contains(&s, lo)) 
        return 4;

#line 59 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str nope = str_from_lit("xyz");

#line 60 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (Str__contains(&s, nope)) 
        return 5;

#line 63 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str empty = str_from_lit("");

#line 64 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (!Str__contains(&s, empty)) 
        return 6;

#line 67 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (!Str__contains(&s, s)) 
        return 7;

#line 70 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str longer = str_from_lit("hello world hello!");

#line 71 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (Str__contains(&s, longer)) 
        return 8;

#line 74 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    Str sliced = Str__slice(&s, 6, 11);

#line 75 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    if (!Str__eq(&sliced, str_from_lit("world"))) 
        return 9;

#line 77 "sharp-test/c_superset_probes/p223_str_extensions.sp"
    return 0;
}
