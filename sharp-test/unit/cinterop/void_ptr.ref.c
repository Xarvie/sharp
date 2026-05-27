#include <stdbool.h>

#include <stdlib.h>

#line 7 "unit/cinterop/void_ptr.sp"
typedef struct AnyBox AnyBox;
struct AnyBox {
    void * ptr;
    int tag;
};

#line 11 "unit/cinterop/void_ptr.sp"
int AnyBox__as_int(AnyBox * this);

#line 16 "unit/cinterop/void_ptr.sp"
float AnyBox__as_float(AnyBox * this);

#line 21 "unit/cinterop/void_ptr.sp"
void AnyBox__set_int(AnyBox * this, int * p);

#line 25 "unit/cinterop/void_ptr.sp"
void AnyBox__set_float(AnyBox * this, float * p);

#line 32 "unit/cinterop/void_ptr.sp"
typedef struct Button Button;
struct Button {
    int id;
    void (*on_click)(int id);
};

#line 36 "unit/cinterop/void_ptr.sp"
void Button__click(Button * this);

#line 39 "unit/cinterop/void_ptr.sp"
void Button__set_callback(Button * this, void (*cb)(int));

#line 42 "unit/cinterop/void_ptr.sp"
typedef int (*IntUnary)(int);

#line 11 "unit/cinterop/void_ptr.sp"
int AnyBox__as_int(AnyBox * this) {

#line 12 "unit/cinterop/void_ptr.sp"
    int * ip = (int *)this->ptr;

#line 13 "unit/cinterop/void_ptr.sp"
    if (this->ptr) 
        return *ip;

#line 14 "unit/cinterop/void_ptr.sp"
    return 0;
}

#line 16 "unit/cinterop/void_ptr.sp"
float AnyBox__as_float(AnyBox * this) {

#line 17 "unit/cinterop/void_ptr.sp"
    float * fp = (float *)this->ptr;

#line 18 "unit/cinterop/void_ptr.sp"
    if (this->ptr) 
        return *fp;

#line 19 "unit/cinterop/void_ptr.sp"
    return 0.0f;
}

#line 21 "unit/cinterop/void_ptr.sp"
void AnyBox__set_int(AnyBox * this, int * p) {

#line 22 "unit/cinterop/void_ptr.sp"
    this->ptr = (void *)p;

#line 23 "unit/cinterop/void_ptr.sp"
    this->tag = 1;
}

#line 25 "unit/cinterop/void_ptr.sp"
void AnyBox__set_float(AnyBox * this, float * p) {

#line 26 "unit/cinterop/void_ptr.sp"
    this->ptr = (void *)p;

#line 27 "unit/cinterop/void_ptr.sp"
    this->tag = 2;
}

#line 29 "unit/cinterop/void_ptr.sp"
_Bool ptr_equals(void * a, void * b) {
    return a == b;
}

#line 36 "unit/cinterop/void_ptr.sp"
void Button__click(Button * this) {

#line 37 "unit/cinterop/void_ptr.sp"
    if (this->on_click) 
        this->on_click(this->id);
}

#line 39 "unit/cinterop/void_ptr.sp"
void Button__set_callback(Button * this, void (*cb)(int)) {

#line 40 "unit/cinterop/void_ptr.sp"
    this->on_click = cb;
}

#line 43 "unit/cinterop/void_ptr.sp"
IntUnary make_doubler() {
    return 0;
}

#line 45 "unit/cinterop/void_ptr.sp"
void cb_a(int id) {
    (void)id;
}

#line 46 "unit/cinterop/void_ptr.sp"
void cb_b(int id) {
    (void)id;
}

#line 48 "unit/cinterop/void_ptr.sp"
int main() {

#line 50 "unit/cinterop/void_ptr.sp"
    AnyBox box;

#line 51 "unit/cinterop/void_ptr.sp"
    int iv = 123;

#line 52 "unit/cinterop/void_ptr.sp"
    AnyBox__set_int(&box, &iv);

#line 53 "unit/cinterop/void_ptr.sp"
    if (AnyBox__as_int(&box) != 123) 
        return 1;

#line 55 "unit/cinterop/void_ptr.sp"
    float fv = 3.14f;

#line 56 "unit/cinterop/void_ptr.sp"
    AnyBox__set_float(&box, &fv);

#line 57 "unit/cinterop/void_ptr.sp"
    float rf = AnyBox__as_float(&box);

#line 58 "unit/cinterop/void_ptr.sp"
    if (rf < 3.13f || rf > 3.15f) 
        return 2;

#line 60 "unit/cinterop/void_ptr.sp"
    if (!ptr_equals(&iv, &iv)) 
        return 3;

#line 61 "unit/cinterop/void_ptr.sp"
    if (ptr_equals(&iv, &fv)) 
        return 4;

#line 63 "unit/cinterop/void_ptr.sp"
    AnyBox empty = { 0 };

#line 64 "unit/cinterop/void_ptr.sp"
    if (AnyBox__as_int(&empty) != 0) 
        return 5;

#line 67 "unit/cinterop/void_ptr.sp"
    Button b;

#line 68 "unit/cinterop/void_ptr.sp"
    b.id = 1;

#line 69 "unit/cinterop/void_ptr.sp"
    Button__set_callback(&b, cb_a);

#line 70 "unit/cinterop/void_ptr.sp"
    Button__click(&b);

#line 71 "unit/cinterop/void_ptr.sp"
    b.id = 2;

#line 72 "unit/cinterop/void_ptr.sp"
    Button__set_callback(&b, cb_b);

#line 73 "unit/cinterop/void_ptr.sp"
    Button__click(&b);

#line 74 "unit/cinterop/void_ptr.sp"
    Button b2 = { 0 };

#line 75 "unit/cinterop/void_ptr.sp"
    Button__click(&b2);

#line 77 "unit/cinterop/void_ptr.sp"
    return 0;
}
