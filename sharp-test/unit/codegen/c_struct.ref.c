#include <stdlib.h>
#include <__stddef_null.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>

#include <stdlib.h>

#line 7 "unit/codegen/c_struct.sp"
typedef long isize;

#line 10 "unit/codegen/c_struct.sp"
struct CIface {
    void * userdata;
    int refcount;
    struct CIface * next;
};

#line 29 "unit/codegen/c_struct.sp"
typedef struct Str Str;
struct Str {
    const char * ptr;
    isize len;
};

#line 33 "unit/codegen/c_struct.sp"
isize Str__size(Str * this);

#line 56 "unit/codegen/c_struct.sp"
typedef struct Vec_int Vec_int;
struct Vec_int {
    int * data;
    isize len;
    isize cap;
};

#line 61 "unit/codegen/c_struct.sp"
void Vec_int__push(Vec_int * this, int val);

#line 70 "unit/codegen/c_struct.sp"
int Vec_int__get(Vec_int * this, isize i);

#line 71 "unit/codegen/c_struct.sp"
isize Vec_int__size(Vec_int * this);

#line 72 "unit/codegen/c_struct.sp"
void Vec_int__destroy(Vec_int * this);

#line 16 "unit/codegen/c_struct.sp"
void reset_refcount(struct CIface * c) {

#line 17 "unit/codegen/c_struct.sp"
    c->refcount = 0;

#line 18 "unit/codegen/c_struct.sp"
    c->next = (struct CIface *)0;
}

#line 20 "unit/codegen/c_struct.sp"
struct CIface * create_c_iface(void) {

#line 21 "unit/codegen/c_struct.sp"
    struct CIface * p = (struct CIface *)malloc(sizeof(struct CIface));

#line 22 "unit/codegen/c_struct.sp"
    p->userdata = 0;

#line 23 "unit/codegen/c_struct.sp"
    p->refcount = 0;

#line 24 "unit/codegen/c_struct.sp"
    p->next = 0;

#line 25 "unit/codegen/c_struct.sp"
    return p;
}

#line 33 "unit/codegen/c_struct.sp"
isize Str__size(Str * this) {
    return this->len;
}

#line 34 "unit/codegen/c_struct.sp"
struct Str str_from_lit(const char * lit) {

#line 35 "unit/codegen/c_struct.sp"
    struct Str s;

#line 36 "unit/codegen/c_struct.sp"
    s.ptr = lit;

#line 37 "unit/codegen/c_struct.sp"
    s.len = 0;

#line 38 "unit/codegen/c_struct.sp"
    while (lit[s.len] != 0) 
        s.len = s.len + 1;

#line 39 "unit/codegen/c_struct.sp"
    return s;
}

#line 41 "unit/codegen/c_struct.sp"
struct Str str_from_parts(const char * p, isize len) {

#line 42 "unit/codegen/c_struct.sp"
    struct Str s;

#line 43 "unit/codegen/c_struct.sp"
    s.ptr = p;

#line 44 "unit/codegen/c_struct.sp"
    s.len = len;

#line 45 "unit/codegen/c_struct.sp"
    return s;
}

#line 47 "unit/codegen/c_struct.sp"
const char * to_cstr_manual(struct Str s) {

#line 48 "unit/codegen/c_struct.sp"
    static char buf[4096];

#line 49 "unit/codegen/c_struct.sp"
    isize i = 0;

#line 50 "unit/codegen/c_struct.sp"
    while (i < Str__size(&s)) {
        buf[i] = s.ptr[i];
        i = i + 1;
    }

#line 51 "unit/codegen/c_struct.sp"
    buf[i] = '\0';

#line 52 "unit/codegen/c_struct.sp"
    return buf;
}

#line 61 "unit/codegen/c_struct.sp"
void Vec_int__push(Vec_int * this, int val) {

#line 62 "unit/codegen/c_struct.sp"
    if (this->len >= this->cap) {

#line 63 "unit/codegen/c_struct.sp"
        isize nc = (this->cap == 0) ? 8 : this->cap * 2;

#line 64 "unit/codegen/c_struct.sp"
        this->data = (int *)realloc(this->data, nc * sizeof(int));

#line 65 "unit/codegen/c_struct.sp"
        this->cap = nc;
    }

#line 67 "unit/codegen/c_struct.sp"
    this->data[this->len] = val;

#line 68 "unit/codegen/c_struct.sp"
    this->len = this->len + 1;
}

#line 70 "unit/codegen/c_struct.sp"
int Vec_int__get(Vec_int * this, isize i) {
    return this->data[i];
}

#line 71 "unit/codegen/c_struct.sp"
isize Vec_int__size(Vec_int * this) {
    return this->len;
}

#line 72 "unit/codegen/c_struct.sp"
void Vec_int__destroy(Vec_int * this) {
    free(this->data);
}

#line 74 "unit/codegen/c_struct.sp"
int main(void) {

#line 76 "unit/codegen/c_struct.sp"
    struct CIface * c = create_c_iface();

#line 77 "unit/codegen/c_struct.sp"
    reset_refcount(c);

#line 78 "unit/codegen/c_struct.sp"
    free(c);

#line 81 "unit/codegen/c_struct.sp"
    struct Str hello = str_from_lit("Hello");

#line 82 "unit/codegen/c_struct.sp"
    if (Str__size(&hello) != 5) 
        return 1;

#line 83 "unit/codegen/c_struct.sp"
    const char * cstr = to_cstr_manual(hello);

#line 84 "unit/codegen/c_struct.sp"
    if (cstr[5] != '\0') 
        return 2;

#line 85 "unit/codegen/c_struct.sp"
    if (cstr[0] != 'H') 
        return 3;

#line 86 "unit/codegen/c_struct.sp"
    struct Str empty = str_from_parts(hello.ptr, 0);

#line 87 "unit/codegen/c_struct.sp"
    if (Str__size(&empty) != 0) 
        return 4;

#line 90 "unit/codegen/c_struct.sp"
    Vec_int vi;

#line 91 "unit/codegen/c_struct.sp"
    vi.data = ((void *)0);
    vi.len = 0;
    vi.cap = 0;

#line 92 "unit/codegen/c_struct.sp"
    Vec_int__push(&vi, 42);

#line 93 "unit/codegen/c_struct.sp"
    Vec_int__push(&vi, 73);

#line 94 "unit/codegen/c_struct.sp"
    if (Vec_int__size(&vi) != 2) 
        return 5;

#line 95 "unit/codegen/c_struct.sp"
    if (Vec_int__get(&vi, 0) != 42) 
        return 6;

#line 96 "unit/codegen/c_struct.sp"
    if (Vec_int__get(&vi, 1) != 73) 
        return 7;

#line 97 "unit/codegen/c_struct.sp"
    Vec_int__destroy(&vi);

#line 99 "unit/codegen/c_struct.sp"
    return 0;
}
