#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <__stddef_null.h>
#include <__stddef_size_t.h>
#include <__stdarg___gnuc_va_list.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 3 "sharp-test/c_superset_probes/p161_std_types.sph"
typedef long isize;

#line 4 "sharp-test/c_superset_probes/p161_std_types.sph"
typedef unsigned long usize;

#line 3 "sharp-test/c_superset_probes/p161_std_str.sph"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 8 "sharp-test/c_superset_probes/p161_std_str.sph"
isize Str__size(Str * this);

#line 12 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__eq(Str * this, Str other);

#line 22 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__starts_with(Str * this, Str prefix);

#line 32 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__ends_with(Str * this, Str suffix);

#line 43 "sharp-test/c_superset_probes/p161_std_str.sph"
Str Str__slice(Str * this, isize start, isize end);

#line 51 "sharp-test/c_superset_probes/p161_std_str.sph"
isize Str__find_byte(Str * this, char c);

#line 60 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__op_eq(Str * this, Str other);

#line 79 "sharp-test/c_superset_probes/p161_std_str.sph"
struct StrSplit {
    Str left;
    Str right;
    _Bool found;
};

#line 5 "sharp-test/c_superset_probes/p161_std_string.sph"
typedef struct String String;
struct String {
    char * ptr;
    isize len;
    isize cap;
};

#line 11 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__push_byte(String * this, char c);

#line 23 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__push_str(String * this, Str s);

#line 61 "sharp-test/c_superset_probes/p161_std_string.sph"
Str String__as_str(String * this);

#line 65 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__clear(String * this);

#line 69 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__reserve(String * this, isize n);

#line 77 "sharp-test/c_superset_probes/p161_std_string.sph"
isize String__size(String * this);

#line 81 "sharp-test/c_superset_probes/p161_std_string.sph"
isize String__capacity(String * this);

#line 85 "sharp-test/c_superset_probes/p161_std_string.sph"
_Bool String__is_empty(String * this);

#line 89 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__destroy(String * this);

#line 5 "sharp-test/c_superset_probes/p161_std_vec.sph"
typedef struct Vec_int Vec_int;
struct Vec_int {
    int * data;
    isize len;
    isize cap;
};

#line 11 "sharp-test/c_superset_probes/p161_std_vec.sph"
void Vec_int__push(Vec_int * this, int val);

#line 23 "sharp-test/c_superset_probes/p161_std_vec.sph"
int Vec_int__pop(Vec_int * this);

#line 29 "sharp-test/c_superset_probes/p161_std_vec.sph"
int Vec_int__get(Vec_int * this, isize i);

#line 34 "sharp-test/c_superset_probes/p161_std_vec.sph"
isize Vec_int__size(Vec_int * this);

#line 38 "sharp-test/c_superset_probes/p161_std_vec.sph"
_Bool Vec_int__is_empty(Vec_int * this);

#line 42 "sharp-test/c_superset_probes/p161_std_vec.sph"
void Vec_int__destroy(Vec_int * this);

#line 8 "sharp-test/c_superset_probes/p161_std_str.sph"
isize Str__size(Str * this) {

#line 9 "sharp-test/c_superset_probes/p161_std_str.sph"
    return this->len;
}

#line 12 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__eq(Str * this, Str other) {

#line 13 "sharp-test/c_superset_probes/p161_std_str.sph"
    if (this->len != other.len) 
        return 0;

#line 14 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize i = 0;

#line 15 "sharp-test/c_superset_probes/p161_std_str.sph"
    while (i < this->len) {

#line 16 "sharp-test/c_superset_probes/p161_std_str.sph"
        if (this->ptr[i] != other.ptr[i]) 
            return 0;

#line 17 "sharp-test/c_superset_probes/p161_std_str.sph"
        i = i + 1;
    }

#line 19 "sharp-test/c_superset_probes/p161_std_str.sph"
    return 1;
}

#line 22 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__starts_with(Str * this, Str prefix) {

#line 23 "sharp-test/c_superset_probes/p161_std_str.sph"
    if (prefix.len > this->len) 
        return 0;

#line 24 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize i = 0;

#line 25 "sharp-test/c_superset_probes/p161_std_str.sph"
    while (i < prefix.len) {

#line 26 "sharp-test/c_superset_probes/p161_std_str.sph"
        if (this->ptr[i] != prefix.ptr[i]) 
            return 0;

#line 27 "sharp-test/c_superset_probes/p161_std_str.sph"
        i = i + 1;
    }

#line 29 "sharp-test/c_superset_probes/p161_std_str.sph"
    return 1;
}

#line 32 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__ends_with(Str * this, Str suffix) {

#line 33 "sharp-test/c_superset_probes/p161_std_str.sph"
    if (suffix.len > this->len) 
        return 0;

#line 34 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize off = this->len - suffix.len;

#line 35 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize i = 0;

#line 36 "sharp-test/c_superset_probes/p161_std_str.sph"
    while (i < suffix.len) {

#line 37 "sharp-test/c_superset_probes/p161_std_str.sph"
        if (this->ptr[off + i] != suffix.ptr[i]) 
            return 0;

#line 38 "sharp-test/c_superset_probes/p161_std_str.sph"
        i = i + 1;
    }

#line 40 "sharp-test/c_superset_probes/p161_std_str.sph"
    return 1;
}

#line 43 "sharp-test/c_superset_probes/p161_std_str.sph"
Str Str__slice(Str * this, isize start, isize end) {

#line 44 "sharp-test/c_superset_probes/p161_std_str.sph"
    if (start < 0 || end < start || end > this->len) 
        __builtin_trap();

#line 45 "sharp-test/c_superset_probes/p161_std_str.sph"
    Str r;

#line 46 "sharp-test/c_superset_probes/p161_std_str.sph"
    r.ptr = this->ptr + start;

#line 47 "sharp-test/c_superset_probes/p161_std_str.sph"
    r.len = end - start;

#line 48 "sharp-test/c_superset_probes/p161_std_str.sph"
    return r;
}

#line 51 "sharp-test/c_superset_probes/p161_std_str.sph"
isize Str__find_byte(Str * this, char c) {

#line 52 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize i = 0;

#line 53 "sharp-test/c_superset_probes/p161_std_str.sph"
    while (i < this->len) {

#line 54 "sharp-test/c_superset_probes/p161_std_str.sph"
        if (this->ptr[i] == c) 
            return i;

#line 55 "sharp-test/c_superset_probes/p161_std_str.sph"
        i = i + 1;
    }

#line 57 "sharp-test/c_superset_probes/p161_std_str.sph"
    return -1;
}

#line 60 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool Str__op_eq(Str * this, Str other) {

#line 61 "sharp-test/c_superset_probes/p161_std_str.sph"
    return Str__eq(this, other);
}

#line 64 "sharp-test/c_superset_probes/p161_std_str.sph"
Str str_from_lit(const char * lit) {

#line 65 "sharp-test/c_superset_probes/p161_std_str.sph"
    Str s;

#line 66 "sharp-test/c_superset_probes/p161_std_str.sph"
    s.ptr = lit;

#line 67 "sharp-test/c_superset_probes/p161_std_str.sph"
    s.len = 0;

#line 68 "sharp-test/c_superset_probes/p161_std_str.sph"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 69 "sharp-test/c_superset_probes/p161_std_str.sph"
    return s;
}

#line 72 "sharp-test/c_superset_probes/p161_std_str.sph"
Str str_from_parts(const char * p, isize len) {

#line 73 "sharp-test/c_superset_probes/p161_std_str.sph"
    Str s;

#line 74 "sharp-test/c_superset_probes/p161_std_str.sph"
    s.ptr = p;

#line 75 "sharp-test/c_superset_probes/p161_std_str.sph"
    s.len = len;

#line 76 "sharp-test/c_superset_probes/p161_std_str.sph"
    return s;
}

#line 85 "sharp-test/c_superset_probes/p161_std_str.sph"
Str str_slice(Str s, isize start, isize end) {

#line 86 "sharp-test/c_superset_probes/p161_std_str.sph"
    return Str__slice(&s, start, end);
}

#line 89 "sharp-test/c_superset_probes/p161_std_str.sph"
isize str_find_byte(Str s, char c) {

#line 90 "sharp-test/c_superset_probes/p161_std_str.sph"
    return Str__find_byte(&s, c);
}

#line 93 "sharp-test/c_superset_probes/p161_std_str.sph"
_Bool str_eq(Str a, Str b) {

#line 94 "sharp-test/c_superset_probes/p161_std_str.sph"
    return Str__eq(&a, b);
}

#line 97 "sharp-test/c_superset_probes/p161_std_str.sph"
struct StrSplit str_split_once(Str s, char sep) {

#line 98 "sharp-test/c_superset_probes/p161_std_str.sph"
    isize idx = Str__find_byte(&s, sep);

#line 99 "sharp-test/c_superset_probes/p161_std_str.sph"
    if (idx < 0) {

#line 100 "sharp-test/c_superset_probes/p161_std_str.sph"
        struct StrSplit r;

#line 101 "sharp-test/c_superset_probes/p161_std_str.sph"
        r.left = s;

#line 102 "sharp-test/c_superset_probes/p161_std_str.sph"
        r.right = str_from_parts(s.ptr + s.len, 0);

#line 103 "sharp-test/c_superset_probes/p161_std_str.sph"
        r.found = 0;

#line 104 "sharp-test/c_superset_probes/p161_std_str.sph"
        return r;
    }

#line 106 "sharp-test/c_superset_probes/p161_std_str.sph"
    struct StrSplit r;

#line 107 "sharp-test/c_superset_probes/p161_std_str.sph"
    r.left = str_from_parts(s.ptr, idx);

#line 108 "sharp-test/c_superset_probes/p161_std_str.sph"
    r.right = str_from_parts(s.ptr + idx + 1, s.len - idx - 1);

#line 109 "sharp-test/c_superset_probes/p161_std_str.sph"
    r.found = 1;

#line 110 "sharp-test/c_superset_probes/p161_std_str.sph"
    return r;
}

#line 11 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__push_byte(String * this, char c) {

#line 12 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/p161_std_string.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/p161_std_string.sph"
        char * nd = (char *)realloc(this->ptr, new_cap);

#line 15 "sharp-test/c_superset_probes/p161_std_string.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->ptr = nd;

#line 17 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->ptr[this->len] = c;

#line 20 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->len = this->len + 1;
}

#line 23 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__push_str(String * this, Str s) {

#line 24 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (this->len + s.len > this->cap) {

#line 25 "sharp-test/c_superset_probes/p161_std_string.sph"
        _Bool overlaps = (s.ptr >= this->ptr && s.ptr < this->ptr + this->cap);

#line 26 "sharp-test/c_superset_probes/p161_std_string.sph"
        char * tmp = (char *)0;

#line 27 "sharp-test/c_superset_probes/p161_std_string.sph"
        isize copy_len = s.len;

#line 28 "sharp-test/c_superset_probes/p161_std_string.sph"
        if (overlaps) {

#line 29 "sharp-test/c_superset_probes/p161_std_string.sph"
            tmp = (char *)malloc(copy_len);

#line 30 "sharp-test/c_superset_probes/p161_std_string.sph"
            if (!tmp) 
                __builtin_trap();

#line 31 "sharp-test/c_superset_probes/p161_std_string.sph"
            isize i = 0;

#line 32 "sharp-test/c_superset_probes/p161_std_string.sph"
            while (i < copy_len) {

#line 33 "sharp-test/c_superset_probes/p161_std_string.sph"
                tmp[i] = s.ptr[i];

#line 34 "sharp-test/c_superset_probes/p161_std_string.sph"
                i = i + 1;
            }

#line 36 "sharp-test/c_superset_probes/p161_std_string.sph"
            s.ptr = tmp;
        }

#line 38 "sharp-test/c_superset_probes/p161_std_string.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 39 "sharp-test/c_superset_probes/p161_std_string.sph"
        while (new_cap < this->len + copy_len) 
            new_cap = new_cap * 2;

#line 40 "sharp-test/c_superset_probes/p161_std_string.sph"
        char * nd = (char *)realloc(this->ptr, new_cap);

#line 41 "sharp-test/c_superset_probes/p161_std_string.sph"
        if (!nd) {
            if (tmp) 
                free(tmp);
            __builtin_trap();
        }

#line 42 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->ptr = nd;

#line 43 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->cap = new_cap;

#line 44 "sharp-test/c_superset_probes/p161_std_string.sph"
        isize i = 0;

#line 45 "sharp-test/c_superset_probes/p161_std_string.sph"
        while (i < copy_len) {

#line 46 "sharp-test/c_superset_probes/p161_std_string.sph"
            this->ptr[this->len + i] = s.ptr[i];

#line 47 "sharp-test/c_superset_probes/p161_std_string.sph"
            i = i + 1;
        }

#line 49 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->len = this->len + copy_len;

#line 50 "sharp-test/c_superset_probes/p161_std_string.sph"
        if (tmp) 
            free(tmp);
    }
    else {

#line 52 "sharp-test/c_superset_probes/p161_std_string.sph"
        isize i = 0;

#line 53 "sharp-test/c_superset_probes/p161_std_string.sph"
        while (i < s.len) {

#line 54 "sharp-test/c_superset_probes/p161_std_string.sph"
            this->ptr[this->len + i] = s.ptr[i];

#line 55 "sharp-test/c_superset_probes/p161_std_string.sph"
            i = i + 1;
        }

#line 57 "sharp-test/c_superset_probes/p161_std_string.sph"
        this->len = this->len + s.len;
    }
}

#line 61 "sharp-test/c_superset_probes/p161_std_string.sph"
Str String__as_str(String * this) {

#line 62 "sharp-test/c_superset_probes/p161_std_string.sph"
    return str_from_parts(this->ptr, this->len);
}

#line 65 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__clear(String * this) {

#line 66 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->len = 0;
}

#line 69 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__reserve(String * this, isize n) {

#line 70 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (n <= this->cap) 
        return;

#line 71 "sharp-test/c_superset_probes/p161_std_string.sph"
    char * nd = (char *)realloc(this->ptr, n);

#line 72 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (!nd) 
        __builtin_trap();

#line 73 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->ptr = nd;

#line 74 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->cap = n;
}

#line 77 "sharp-test/c_superset_probes/p161_std_string.sph"
isize String__size(String * this) {

#line 78 "sharp-test/c_superset_probes/p161_std_string.sph"
    return this->len;
}

#line 81 "sharp-test/c_superset_probes/p161_std_string.sph"
isize String__capacity(String * this) {

#line 82 "sharp-test/c_superset_probes/p161_std_string.sph"
    return this->cap;
}

#line 85 "sharp-test/c_superset_probes/p161_std_string.sph"
_Bool String__is_empty(String * this) {

#line 86 "sharp-test/c_superset_probes/p161_std_string.sph"
    return this->len == 0;
}

#line 89 "sharp-test/c_superset_probes/p161_std_string.sph"
void String__destroy(String * this) {

#line 90 "sharp-test/c_superset_probes/p161_std_string.sph"
    free(this->ptr);

#line 91 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->ptr = (char *)0;

#line 92 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->len = 0;

#line 93 "sharp-test/c_superset_probes/p161_std_string.sph"
    this->cap = 0;
}

#line 96 "sharp-test/c_superset_probes/p161_std_string.sph"
String string_from_str(Str s) {

#line 97 "sharp-test/c_superset_probes/p161_std_string.sph"
    String r;

#line 98 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (s.len == 0) {

#line 99 "sharp-test/c_superset_probes/p161_std_string.sph"
        r.ptr = (char *)0;

#line 100 "sharp-test/c_superset_probes/p161_std_string.sph"
        r.len = 0;

#line 101 "sharp-test/c_superset_probes/p161_std_string.sph"
        r.cap = 0;

#line 102 "sharp-test/c_superset_probes/p161_std_string.sph"
        return r;
    }

#line 104 "sharp-test/c_superset_probes/p161_std_string.sph"
    r.len = s.len;

#line 105 "sharp-test/c_superset_probes/p161_std_string.sph"
    r.cap = s.len;

#line 106 "sharp-test/c_superset_probes/p161_std_string.sph"
    char * nd = (char *)malloc(r.cap);

#line 107 "sharp-test/c_superset_probes/p161_std_string.sph"
    if (!nd) 
        __builtin_trap();

#line 108 "sharp-test/c_superset_probes/p161_std_string.sph"
    r.ptr = nd;

#line 109 "sharp-test/c_superset_probes/p161_std_string.sph"
    isize i = 0;

#line 110 "sharp-test/c_superset_probes/p161_std_string.sph"
    while (i < s.len) {

#line 111 "sharp-test/c_superset_probes/p161_std_string.sph"
        r.ptr[i] = s.ptr[i];

#line 112 "sharp-test/c_superset_probes/p161_std_string.sph"
        i = i + 1;
    }

#line 114 "sharp-test/c_superset_probes/p161_std_string.sph"
    return r;
}

#line 117 "sharp-test/c_superset_probes/p161_std_string.sph"
String string_from_parts(const char * sptr, isize slen) {

#line 118 "sharp-test/c_superset_probes/p161_std_string.sph"
    Str tmp = str_from_parts(sptr, slen);

#line 119 "sharp-test/c_superset_probes/p161_std_string.sph"
    return string_from_str(tmp);
}

#line 11 "sharp-test/c_superset_probes/p161_std_vec.sph"
void Vec_int__push(Vec_int * this, int val) {

#line 12 "sharp-test/c_superset_probes/p161_std_vec.sph"
    if (this->len >= this->cap) {

#line 13 "sharp-test/c_superset_probes/p161_std_vec.sph"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 14 "sharp-test/c_superset_probes/p161_std_vec.sph"
        int * nd = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 15 "sharp-test/c_superset_probes/p161_std_vec.sph"
        if (!nd) 
            __builtin_trap();

#line 16 "sharp-test/c_superset_probes/p161_std_vec.sph"
        this->data = nd;

#line 17 "sharp-test/c_superset_probes/p161_std_vec.sph"
        this->cap = new_cap;
    }

#line 19 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->data[this->len] = val;

#line 20 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->len = this->len + 1;
}

#line 23 "sharp-test/c_superset_probes/p161_std_vec.sph"
int Vec_int__pop(Vec_int * this) {

#line 24 "sharp-test/c_superset_probes/p161_std_vec.sph"
    if (this->len == 0) 
        __builtin_trap();

#line 25 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->len = this->len - 1;

#line 26 "sharp-test/c_superset_probes/p161_std_vec.sph"
    return this->data[this->len];
}

#line 29 "sharp-test/c_superset_probes/p161_std_vec.sph"
int Vec_int__get(Vec_int * this, isize i) {

#line 30 "sharp-test/c_superset_probes/p161_std_vec.sph"
    if (i < 0 || i >= this->len) 
        __builtin_trap();

#line 31 "sharp-test/c_superset_probes/p161_std_vec.sph"
    return this->data[i];
}

#line 34 "sharp-test/c_superset_probes/p161_std_vec.sph"
isize Vec_int__size(Vec_int * this) {

#line 35 "sharp-test/c_superset_probes/p161_std_vec.sph"
    return this->len;
}

#line 38 "sharp-test/c_superset_probes/p161_std_vec.sph"
_Bool Vec_int__is_empty(Vec_int * this) {

#line 39 "sharp-test/c_superset_probes/p161_std_vec.sph"
    return this->len == 0;
}

#line 42 "sharp-test/c_superset_probes/p161_std_vec.sph"
void Vec_int__destroy(Vec_int * this) {

#line 43 "sharp-test/c_superset_probes/p161_std_vec.sph"
    free(this->data);

#line 44 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->data = (int *)0;

#line 45 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->len = 0;

#line 46 "sharp-test/c_superset_probes/p161_std_vec.sph"
    this->cap = 0;
}

#line 18 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
int main(void) {

#line 20 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Str hello = str_from_lit("hello");

#line 21 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Str world = str_from_lit("world");

#line 22 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Str hello2 = str_from_lit("hello");

#line 25 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    isize hello_len = Str__size(&hello);

#line 26 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    _Bool hello_eq = Str__eq(&hello, hello2);

#line 27 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    _Bool eq_test = str_eq(hello, world);

#line 29 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("hello.size()=%zd\n", hello_len);

#line 30 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("hello.eq(hello2)=%d\n", hello_eq);

#line 31 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("str_eq(hello,world)=%d\n", eq_test);

#line 34 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    String s = string_from_str(hello);

#line 35 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    isize s_size = String__size(&s);

#line 36 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    String__push_byte(&s, '!');

#line 37 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    isize s_size2 = String__size(&s);

#line 38 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    String__destroy(&s);

#line 40 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("string.size()=%zd\n", s_size);

#line 41 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("string after push_byte=%zd\n", s_size2);

#line 44 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Vec_int v;

#line 45 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    v.data = ((void *)0);

#line 46 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    v.len = 0;

#line 47 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    v.cap = 0;

#line 49 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Vec_int__push(&v, 10);

#line 50 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Vec_int__push(&v, 20);

#line 51 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Vec_int__push(&v, 30);

#line 52 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    isize v_size = Vec_int__size(&v);

#line 53 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    int v_last = Vec_int__pop(&v);

#line 54 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    int v_first = Vec_int__get(&v, 0);

#line 55 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    Vec_int__destroy(&v);

#line 57 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("vec.size()=%zd vec.last=%d vec.first=%d\n", v_size, v_last, v_first);

#line 60 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (hello_len != 5) 
        return 1;

#line 61 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (!hello_eq) 
        return 2;

#line 62 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (eq_test) 
        return 3;

#line 63 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (s_size != 5) 
        return 4;

#line 64 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (s_size2 != 6) 
        return 5;

#line 65 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (v_size != 3) 
        return 6;

#line 66 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (v_last != 30) 
        return 7;

#line 67 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    if (v_first != 10) 
        return 8;

#line 69 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    printf("P161_STDLIB_PASS\n");

#line 70 "sharp-test/c_superset_probes/p161_stdlib_integration.sp"
    return 0;
}
