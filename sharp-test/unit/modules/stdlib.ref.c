#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <__stddef_null.h>
#include <__stddef_size_t.h>
#include <__stdarg___gnuc_va_list.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 9 "unit/modules/stdlib.sp"
typedef long isize;

#line 12 "unit/modules/stdlib.sp"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 16 "unit/modules/stdlib.sp"
isize Str__size(Str * this);

#line 17 "unit/modules/stdlib.sp"
_Bool Str__eq(Str * this, Str other);

#line 31 "unit/modules/stdlib.sp"
typedef struct String String;
struct String {
    char * ptr;
    isize len;
    isize cap;
};

#line 36 "unit/modules/stdlib.sp"
void String__push_byte(String * this, char c);

#line 46 "unit/modules/stdlib.sp"
isize String__size(String * this);

#line 47 "unit/modules/stdlib.sp"
void String__destroy(String * this);

#line 58 "unit/modules/stdlib.sp"
typedef struct Vec_int Vec_int;
struct Vec_int {
    int * data;
    isize len;
    isize cap;
};

#line 63 "unit/modules/stdlib.sp"
void Vec_int__push(Vec_int * this, int val);

#line 73 "unit/modules/stdlib.sp"
int Vec_int__pop(Vec_int * this);

#line 78 "unit/modules/stdlib.sp"
int Vec_int__get(Vec_int * this, isize i);

#line 79 "unit/modules/stdlib.sp"
isize Vec_int__size(Vec_int * this);

#line 80 "unit/modules/stdlib.sp"
void Vec_int__destroy(Vec_int * this);

#line 16 "unit/modules/stdlib.sp"
isize Str__size(Str * this) {
    return this->len;
}

#line 17 "unit/modules/stdlib.sp"
_Bool Str__eq(Str * this, Str other) {

#line 18 "unit/modules/stdlib.sp"
    if (this->len != other.len) 
        return 0;

#line 19 "unit/modules/stdlib.sp"
    for (long i = 0; i < this->len; i = i + 1) 

#line 20 "unit/modules/stdlib.sp"
        if (this->ptr[i] != other.ptr[i]) 
            return 0;

#line 21 "unit/modules/stdlib.sp"
    return 1;
}

#line 23 "unit/modules/stdlib.sp"
Str str_from_lit(const char * lit) {

#line 24 "unit/modules/stdlib.sp"
    Str s;
    s.ptr = lit;
    s.len = 0;

#line 25 "unit/modules/stdlib.sp"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 26 "unit/modules/stdlib.sp"
    return s;
}

#line 28 "unit/modules/stdlib.sp"
_Bool str_eq(Str a, Str b) {
    return Str__eq(&a, b);
}

#line 36 "unit/modules/stdlib.sp"
void String__push_byte(String * this, char c) {

#line 37 "unit/modules/stdlib.sp"
    if (this->len >= this->cap) {

#line 38 "unit/modules/stdlib.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 39 "unit/modules/stdlib.sp"
        this->ptr = (char *)realloc(this->ptr, new_cap);

#line 40 "unit/modules/stdlib.sp"
        if (!this->ptr) 
            __builtin_trap();

#line 41 "unit/modules/stdlib.sp"
        this->cap = new_cap;
    }

#line 43 "unit/modules/stdlib.sp"
    this->ptr[this->len] = c;

#line 44 "unit/modules/stdlib.sp"
    this->len = this->len + 1;
}

#line 46 "unit/modules/stdlib.sp"
isize String__size(String * this) {
    return this->len;
}

#line 47 "unit/modules/stdlib.sp"
void String__destroy(String * this) {
    free(this->ptr);
    this->ptr = (char *)0;
    this->len = 0;
    this->cap = 0;
}

#line 48 "unit/modules/stdlib.sp"
String string_from_str(Str s) {

#line 49 "unit/modules/stdlib.sp"
    String r;

#line 50 "unit/modules/stdlib.sp"
    r.len = s.len;
    r.cap = s.len;

#line 51 "unit/modules/stdlib.sp"
    r.ptr = (char *)malloc(r.cap);

#line 52 "unit/modules/stdlib.sp"
    if (!r.ptr && r.cap > 0) 
        __builtin_trap();

#line 53 "unit/modules/stdlib.sp"
    for (long i = 0; i < s.len; i = i + 1) 
        r.ptr[i] = s.ptr[i];

#line 54 "unit/modules/stdlib.sp"
    return r;
}

#line 63 "unit/modules/stdlib.sp"
void Vec_int__push(Vec_int * this, int val) {

#line 64 "unit/modules/stdlib.sp"
    if (this->len >= this->cap) {

#line 65 "unit/modules/stdlib.sp"
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;

#line 66 "unit/modules/stdlib.sp"
        this->data = (int *)realloc(this->data, sizeof(int) * new_cap);

#line 67 "unit/modules/stdlib.sp"
        if (!this->data) 
            __builtin_trap();

#line 68 "unit/modules/stdlib.sp"
        this->cap = new_cap;
    }

#line 70 "unit/modules/stdlib.sp"
    this->data[this->len] = val;

#line 71 "unit/modules/stdlib.sp"
    this->len = this->len + 1;
}

#line 73 "unit/modules/stdlib.sp"
int Vec_int__pop(Vec_int * this) {

#line 74 "unit/modules/stdlib.sp"
    if (this->len == 0) 
        __builtin_trap();

#line 75 "unit/modules/stdlib.sp"
    this->len = this->len - 1;

#line 76 "unit/modules/stdlib.sp"
    return this->data[this->len];
}

#line 78 "unit/modules/stdlib.sp"
int Vec_int__get(Vec_int * this, isize i) {
    return this->data[i];
}

#line 79 "unit/modules/stdlib.sp"
isize Vec_int__size(Vec_int * this) {
    return this->len;
}

#line 80 "unit/modules/stdlib.sp"
void Vec_int__destroy(Vec_int * this) {
    free(this->data);
    this->data = (int *)0;
    this->len = 0;
    this->cap = 0;
}

#line 82 "unit/modules/stdlib.sp"
int main(void) {

#line 84 "unit/modules/stdlib.sp"
    Str hello = str_from_lit("hello");

#line 85 "unit/modules/stdlib.sp"
    Str world = str_from_lit("world");

#line 86 "unit/modules/stdlib.sp"
    Str hello2 = str_from_lit("hello");

#line 88 "unit/modules/stdlib.sp"
    isize hello_len = Str__size(&hello);

#line 89 "unit/modules/stdlib.sp"
    if (hello_len != 5) 
        return 1;

#line 90 "unit/modules/stdlib.sp"
    if (!Str__eq(&hello, hello2)) 
        return 2;

#line 91 "unit/modules/stdlib.sp"
    if (str_eq(hello, world)) 
        return 3;

#line 94 "unit/modules/stdlib.sp"
    String s = string_from_str(hello);

#line 95 "unit/modules/stdlib.sp"
    isize s_size = String__size(&s);

#line 96 "unit/modules/stdlib.sp"
    String__push_byte(&s, '!');

#line 97 "unit/modules/stdlib.sp"
    isize s_size2 = String__size(&s);

#line 98 "unit/modules/stdlib.sp"
    if (s_size != 5) 
        return 4;

#line 99 "unit/modules/stdlib.sp"
    if (s_size2 != 6) 
        return 5;

#line 100 "unit/modules/stdlib.sp"
    String__destroy(&s);

#line 103 "unit/modules/stdlib.sp"
    Vec_int v;

#line 104 "unit/modules/stdlib.sp"
    v.data = ((void *)0);

#line 105 "unit/modules/stdlib.sp"
    v.len = 0;

#line 106 "unit/modules/stdlib.sp"
    v.cap = 0;

#line 107 "unit/modules/stdlib.sp"
    Vec_int__push(&v, 10);

#line 108 "unit/modules/stdlib.sp"
    Vec_int__push(&v, 20);

#line 109 "unit/modules/stdlib.sp"
    Vec_int__push(&v, 30);

#line 110 "unit/modules/stdlib.sp"
    isize v_size = Vec_int__size(&v);

#line 111 "unit/modules/stdlib.sp"
    int v_last = Vec_int__pop(&v);

#line 112 "unit/modules/stdlib.sp"
    int v_first = Vec_int__get(&v, 0);

#line 113 "unit/modules/stdlib.sp"
    if (v_size != 3) 
        return 6;

#line 114 "unit/modules/stdlib.sp"
    if (v_last != 30) 
        return 7;

#line 115 "unit/modules/stdlib.sp"
    if (v_first != 10) 
        return 8;

#line 116 "unit/modules/stdlib.sp"
    Vec_int__destroy(&v);

#line 118 "unit/modules/stdlib.sp"
    return 0;
}
