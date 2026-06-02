
#line 7 "sharp-test/unit/cinterop/void_ptr.ce"
typedef struct AnyBox AnyBox;
struct AnyBox {
    void * ptr;
    int tag;
};

#line 11 "sharp-test/unit/cinterop/void_ptr.ce"
int AnyBox__as_int(AnyBox * this);

#line 16 "sharp-test/unit/cinterop/void_ptr.ce"
float AnyBox__as_float(AnyBox * this);

#line 21 "sharp-test/unit/cinterop/void_ptr.ce"
void AnyBox__set_int(AnyBox * this, int * p);

#line 25 "sharp-test/unit/cinterop/void_ptr.ce"
void AnyBox__set_float(AnyBox * this, float * p);

#line 32 "sharp-test/unit/cinterop/void_ptr.ce"
typedef struct Button Button;
struct Button {
    int id;
    void (*on_click)(int id);
};

#line 36 "sharp-test/unit/cinterop/void_ptr.ce"
void Button__click(Button * this);

#line 39 "sharp-test/unit/cinterop/void_ptr.ce"
void Button__set_callback(Button * this, void (*cb)(int));

#line 42 "sharp-test/unit/cinterop/void_ptr.ce"
typedef int (*IntUnary)(int);

#line 11 "sharp-test/unit/cinterop/void_ptr.ce"
int AnyBox__as_int(AnyBox * this) {

#line 12 "sharp-test/unit/cinterop/void_ptr.ce"
    int * ip = (int *)this->ptr;

#line 13 "sharp-test/unit/cinterop/void_ptr.ce"
    if (this->ptr) 
        return *ip;

#line 14 "sharp-test/unit/cinterop/void_ptr.ce"
    return 0;
}

#line 16 "sharp-test/unit/cinterop/void_ptr.ce"
float AnyBox__as_float(AnyBox * this) {

#line 17 "sharp-test/unit/cinterop/void_ptr.ce"
    float * fp = (float *)this->ptr;

#line 18 "sharp-test/unit/cinterop/void_ptr.ce"
    if (this->ptr) 
        return *fp;

#line 19 "sharp-test/unit/cinterop/void_ptr.ce"
    return 0.0f;
}

#line 21 "sharp-test/unit/cinterop/void_ptr.ce"
void AnyBox__set_int(AnyBox * this, int * p) {

#line 22 "sharp-test/unit/cinterop/void_ptr.ce"
    this->ptr = (void *)p;

#line 23 "sharp-test/unit/cinterop/void_ptr.ce"
    this->tag = 1;
}

#line 25 "sharp-test/unit/cinterop/void_ptr.ce"
void AnyBox__set_float(AnyBox * this, float * p) {

#line 26 "sharp-test/unit/cinterop/void_ptr.ce"
    this->ptr = (void *)p;

#line 27 "sharp-test/unit/cinterop/void_ptr.ce"
    this->tag = 2;
}

#line 29 "sharp-test/unit/cinterop/void_ptr.ce"
bool ptr_equals(void * a, void * b) {
    return a == b;
}

#line 36 "sharp-test/unit/cinterop/void_ptr.ce"
void Button__click(Button * this) {

#line 37 "sharp-test/unit/cinterop/void_ptr.ce"
    if (this->on_click) 
        this->on_click(this->id);
}

#line 39 "sharp-test/unit/cinterop/void_ptr.ce"
void Button__set_callback(Button * this, void (*cb)(int)) {

#line 40 "sharp-test/unit/cinterop/void_ptr.ce"
    this->on_click = cb;
}

#line 43 "sharp-test/unit/cinterop/void_ptr.ce"
IntUnary make_doubler() {
    return 0;
}

#line 45 "sharp-test/unit/cinterop/void_ptr.ce"
int double_it(int x) {
    return x * 2;
}

#line 47 "sharp-test/unit/cinterop/void_ptr.ce"
void cb_a(int id) {
    (void)id;
}

#line 48 "sharp-test/unit/cinterop/void_ptr.ce"
void cb_b(int id) {
    (void)id;
}

#line 50 "sharp-test/unit/cinterop/void_ptr.ce"
int main() {

#line 52 "sharp-test/unit/cinterop/void_ptr.ce"
    AnyBox box;

#line 53 "sharp-test/unit/cinterop/void_ptr.ce"
    int iv = 123;

#line 54 "sharp-test/unit/cinterop/void_ptr.ce"
    AnyBox__set_int(&box, &iv);

#line 55 "sharp-test/unit/cinterop/void_ptr.ce"
    if (AnyBox__as_int(&box) != 123) 
        return 1;

#line 57 "sharp-test/unit/cinterop/void_ptr.ce"
    float fv = 3.14f;

#line 58 "sharp-test/unit/cinterop/void_ptr.ce"
    AnyBox__set_float(&box, &fv);

#line 59 "sharp-test/unit/cinterop/void_ptr.ce"
    float rf = AnyBox__as_float(&box);

#line 60 "sharp-test/unit/cinterop/void_ptr.ce"
    if (rf < 3.13f || rf > 3.15f) 
        return 2;

#line 62 "sharp-test/unit/cinterop/void_ptr.ce"
    if (!ptr_equals(&iv, &iv)) 
        return 3;

#line 63 "sharp-test/unit/cinterop/void_ptr.ce"
    if (ptr_equals(&iv, &fv)) 
        return 4;

#line 65 "sharp-test/unit/cinterop/void_ptr.ce"
    AnyBox empty = { 0 };

#line 66 "sharp-test/unit/cinterop/void_ptr.ce"
    if (AnyBox__as_int(&empty) != 0) 
        return 5;

#line 69 "sharp-test/unit/cinterop/void_ptr.ce"
    Button b;

#line 70 "sharp-test/unit/cinterop/void_ptr.ce"
    b.id = 1;

#line 71 "sharp-test/unit/cinterop/void_ptr.ce"
    Button__set_callback(&b, cb_a);

#line 72 "sharp-test/unit/cinterop/void_ptr.ce"
    Button__click(&b);

#line 73 "sharp-test/unit/cinterop/void_ptr.ce"
    b.id = 2;

#line 74 "sharp-test/unit/cinterop/void_ptr.ce"
    Button__set_callback(&b, cb_b);

#line 75 "sharp-test/unit/cinterop/void_ptr.ce"
    Button__click(&b);

#line 76 "sharp-test/unit/cinterop/void_ptr.ce"
    Button b2 = { 0 };

#line 77 "sharp-test/unit/cinterop/void_ptr.ce"
    Button__click(&b2);

#line 80 "sharp-test/unit/cinterop/void_ptr.ce"
    int ia = 100;

#line 81 "sharp-test/unit/cinterop/void_ptr.ce"
    float fa = 2.5f;

#line 82 "sharp-test/unit/cinterop/void_ptr.ce"
    void * ptrs[2];

#line 83 "sharp-test/unit/cinterop/void_ptr.ce"
    ptrs[0] = (void *)&ia;

#line 84 "sharp-test/unit/cinterop/void_ptr.ce"
    ptrs[1] = (void *)&fa;

#line 85 "sharp-test/unit/cinterop/void_ptr.ce"
    int * pi = (int *)ptrs[0];

#line 86 "sharp-test/unit/cinterop/void_ptr.ce"
    float * pf = (float *)ptrs[1];

#line 87 "sharp-test/unit/cinterop/void_ptr.ce"
    if (*pi != 100) 
        return 6;

#line 88 "sharp-test/unit/cinterop/void_ptr.ce"
    if (*pf < 2.49f || *pf > 2.51f) 
        return 7;

#line 91 "sharp-test/unit/cinterop/void_ptr.ce"
    int (*fn)(int) = double_it;

#line 92 "sharp-test/unit/cinterop/void_ptr.ce"
    if (fn(5) != 10) 
        return 8;

#line 93 "sharp-test/unit/cinterop/void_ptr.ce"
    if (fn(21) != 42) 
        return 9;

#line 95 "sharp-test/unit/cinterop/void_ptr.ce"
    return 0;
}
