
#line 3 "sharp-test/unit/modules/module_advanced.he"
typedef long isize;

#line 5 "sharp-test/unit/modules/module_advanced.he"

#line 58 "sharp-test/unit/modules/module_advanced.he"
enum BufStatus {
    BUF_OK = 0,
    BUF_FULL = 1,
    BUF_EMPTY = 2,
    BUF_ERROR = -1
};

#line 5 "sharp-test/unit/modules/module_advanced.he"
typedef struct RingBuf__int RingBuf__int;
struct RingBuf__int {
    int * data;
    isize cap;
    isize head;
    isize tail;
    isize count;
};


typedef struct RingBuf__float RingBuf__float;
struct RingBuf__float {
    float * data;
    isize cap;
    isize head;
    isize tail;
    isize count;
};



#line 13 "sharp-test/unit/modules/module_advanced.he"
void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity);

#line 49 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__int__is_empty(RingBuf__int * this);

#line 47 "sharp-test/unit/modules/module_advanced.he"
long RingBuf__int__size(RingBuf__int * this);

#line 21 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__int__push(RingBuf__int * this, int val);

#line 37 "sharp-test/unit/modules/module_advanced.he"
int RingBuf__int__peek(RingBuf__int * this);

#line 29 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__int__pop(RingBuf__int * this, int * out);

#line 50 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__int__is_full(RingBuf__int * this);

#line 41 "sharp-test/unit/modules/module_advanced.he"
void RingBuf__int__clear(RingBuf__int * this);

#line 13 "sharp-test/unit/modules/module_advanced.he"
void RingBuf__float__init(RingBuf__float * this, float * buf, long capacity);

#line 21 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__float__push(RingBuf__float * this, float val);

#line 47 "sharp-test/unit/modules/module_advanced.he"
long RingBuf__float__size(RingBuf__float * this);

#line 48 "sharp-test/unit/modules/module_advanced.he"
long RingBuf__float__capacity(RingBuf__float * this);

#line 29 "sharp-test/unit/modules/module_advanced.he"
_Bool RingBuf__float__pop(RingBuf__float * this, float * out);

#line 37 "sharp-test/unit/modules/module_advanced.he"
float RingBuf__float__peek(RingBuf__float * this);

#line 52 "sharp-test/unit/modules/module_advanced.he"
int clamp_val__int(int val, int lo, int hi);
float clamp_val__float(float val, float lo, float hi);

#line 13 "sharp-test/unit/modules/module_advanced.he"

#line 21 "sharp-test/unit/modules/module_advanced.he"

#line 29 "sharp-test/unit/modules/module_advanced.he"

#line 37 "sharp-test/unit/modules/module_advanced.he"

#line 41 "sharp-test/unit/modules/module_advanced.he"

#line 47 "sharp-test/unit/modules/module_advanced.he"

#line 48 "sharp-test/unit/modules/module_advanced.he"

#line 49 "sharp-test/unit/modules/module_advanced.he"

#line 50 "sharp-test/unit/modules/module_advanced.he"

#line 52 "sharp-test/unit/modules/module_advanced.he"

#line 3 "sharp-test/unit/modules/module_advanced.ce"
int main(void) {

#line 4 "sharp-test/unit/modules/module_advanced.ce"
    int ibuf[8] = { 0 };

#line 5 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int irb;

#line 6 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__init(&irb, ibuf, 8);

#line 8 "sharp-test/unit/modules/module_advanced.ce"
    if (!RingBuf__int__is_empty(&irb)) 
        return 1;

#line 9 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__size(&irb) != 0) 
        return 2;

#line 11 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb, 10);

#line 12 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb, 20);

#line 13 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb, 30);

#line 15 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__is_empty(&irb)) 
        return 3;

#line 16 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__size(&irb) != 3) 
        return 4;

#line 17 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__peek(&irb) != 10) 
        return 5;

#line 19 "sharp-test/unit/modules/module_advanced.ce"
    int ival = 0;

#line 20 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__pop(&irb, &ival);

#line 21 "sharp-test/unit/modules/module_advanced.ce"
    if (ival != 10) 
        return 6;

#line 22 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__peek(&irb) != 20) 
        return 7;

#line 24 "sharp-test/unit/modules/module_advanced.ce"
    int ibuf2[4] = { 0 };

#line 25 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int irb2;

#line 26 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__init(&irb2, ibuf2, 4);

#line 28 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb2, 1);

#line 29 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb2, 2);

#line 30 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb2, 3);

#line 31 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb2, 4);

#line 33 "sharp-test/unit/modules/module_advanced.ce"
    if (!RingBuf__int__is_full(&irb2)) 
        return 8;

#line 34 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__push(&irb2, 5)) 
        return 9;

#line 36 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__clear(&irb2);

#line 37 "sharp-test/unit/modules/module_advanced.ce"
    if (!RingBuf__int__is_empty(&irb2)) 
        return 10;

#line 38 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__size(&irb2) != 0) 
        return 11;

#line 40 "sharp-test/unit/modules/module_advanced.ce"
    float fbuf[6] = { 0.0f };

#line 41 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float frb;

#line 42 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__init(&frb, fbuf, 6);

#line 44 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__push(&frb, 1.5f);

#line 45 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__push(&frb, 2.5f);

#line 46 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__push(&frb, 3.5f);

#line 48 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__float__size(&frb) != 3) 
        return 12;

#line 49 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__float__capacity(&frb) != 6) 
        return 13;

#line 51 "sharp-test/unit/modules/module_advanced.ce"
    float fval = 0.0f;

#line 52 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__pop(&frb, &fval);

#line 53 "sharp-test/unit/modules/module_advanced.ce"
    if (fval < 1.49f || fval > 1.51f) 
        return 14;

#line 55 "sharp-test/unit/modules/module_advanced.ce"
    float fpeek = RingBuf__float__peek(&frb);

#line 56 "sharp-test/unit/modules/module_advanced.ce"
    if (fpeek < 2.49f || fpeek > 2.51f) 
        return 15;

#line 58 "sharp-test/unit/modules/module_advanced.ce"
    int clamped_int = clamp_val__int(15, 0, 10);

#line 59 "sharp-test/unit/modules/module_advanced.ce"
    if (clamped_int != 10) 
        return 16;

#line 61 "sharp-test/unit/modules/module_advanced.ce"
    int clamped_int2 = clamp_val__int(5, 0, 10);

#line 62 "sharp-test/unit/modules/module_advanced.ce"
    if (clamped_int2 != 5) 
        return 17;

#line 64 "sharp-test/unit/modules/module_advanced.ce"
    float clamped_float = clamp_val__float(-1.0f, 0.0f, 10.0f);

#line 65 "sharp-test/unit/modules/module_advanced.ce"
    if (clamped_float < -0.01f || clamped_float > 0.01f) 
        return 18;

#line 67 "sharp-test/unit/modules/module_advanced.ce"
    enum BufStatus status = BUF_OK;

#line 68 "sharp-test/unit/modules/module_advanced.ce"
    if (status != BUF_OK) 
        return 19;

#line 70 "sharp-test/unit/modules/module_advanced.ce"
    status = BUF_FULL;

#line 71 "sharp-test/unit/modules/module_advanced.ce"
    if (status != BUF_FULL) 
        return 20;

#line 73 "sharp-test/unit/modules/module_advanced.ce"
    status = BUF_EMPTY;

#line 74 "sharp-test/unit/modules/module_advanced.ce"
    if (status != BUF_EMPTY) 
        return 21;

#line 76 "sharp-test/unit/modules/module_advanced.ce"
    int buf3[4] = { 0 };

#line 77 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int rb3;

#line 78 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__init(&rb3, buf3, 4);

#line 80 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&rb3, clamp_val__int(100, 0, 50));

#line 81 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&rb3, clamp_val__int(-10, 0, 50));

#line 82 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&rb3, clamp_val__int(25, 0, 50));
    int v1 = 0, v2 = 0, v3 = 0;

#line 85 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__pop(&rb3, &v1);

#line 86 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__pop(&rb3, &v2);

#line 87 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__pop(&rb3, &v3);

#line 88 "sharp-test/unit/modules/module_advanced.ce"
    if (v1 != 50) 
        return 22;

#line 89 "sharp-test/unit/modules/module_advanced.ce"
    if (v2 != 0) 
        return 23;

#line 90 "sharp-test/unit/modules/module_advanced.ce"
    if (v3 != 25) 
        return 24;

#line 92 "sharp-test/unit/modules/module_advanced.ce"
    float fbuf2[4] = { 0.0f };

#line 93 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float frb2;

#line 94 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__init(&frb2, fbuf2, 4);

#line 96 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__push(&frb2, clamp_val__float(3.14f, 0.0f, 2.0f));

#line 97 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__push(&frb2, clamp_val__float(1.5f, 0.0f, 2.0f));
    float fv1 = 0.0f, fv2 = 0.0f;

#line 100 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__pop(&frb2, &fv1);

#line 101 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__float__pop(&frb2, &fv2);

#line 102 "sharp-test/unit/modules/module_advanced.ce"
    if (fv1 < 1.99f || fv1 > 2.01f) 
        return 25;

#line 103 "sharp-test/unit/modules/module_advanced.ce"
    if (fv2 < 1.49f || fv2 > 1.51f) 
        return 26;

#line 105 "sharp-test/unit/modules/module_advanced.ce"
    enum BufStatus check_status;

#line 106 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__is_full(&irb2)) {

#line 107 "sharp-test/unit/modules/module_advanced.ce"
        check_status = BUF_FULL;
    }
    else 

#line 108 "sharp-test/unit/modules/module_advanced.ce"
        if (RingBuf__int__is_empty(&irb2)) {

#line 109 "sharp-test/unit/modules/module_advanced.ce"
            check_status = BUF_EMPTY;
        }
        else {

#line 111 "sharp-test/unit/modules/module_advanced.ce"
            check_status = BUF_OK;
        }

#line 113 "sharp-test/unit/modules/module_advanced.ce"
    if (check_status != BUF_EMPTY) 
        return 27;

#line 115 "sharp-test/unit/modules/module_advanced.ce"
    RingBuf__int__push(&irb2, 42);

#line 116 "sharp-test/unit/modules/module_advanced.ce"
    if (RingBuf__int__is_full(&irb2)) {

#line 117 "sharp-test/unit/modules/module_advanced.ce"
        check_status = BUF_FULL;
    }
    else 

#line 118 "sharp-test/unit/modules/module_advanced.ce"
        if (RingBuf__int__is_empty(&irb2)) {

#line 119 "sharp-test/unit/modules/module_advanced.ce"
            check_status = BUF_EMPTY;
        }
        else {

#line 121 "sharp-test/unit/modules/module_advanced.ce"
            check_status = BUF_OK;
        }

#line 123 "sharp-test/unit/modules/module_advanced.ce"
    if (check_status != BUF_OK) 
        return 28;

#line 125 "sharp-test/unit/modules/module_advanced.ce"
    return 0;
}

#line 13 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity) {

#line 14 "sharp-test/unit/modules/module_advanced.he"
    this->data = buf;

#line 15 "sharp-test/unit/modules/module_advanced.he"
    this->cap = capacity;

#line 16 "sharp-test/unit/modules/module_advanced.he"
    this->head = 0;

#line 17 "sharp-test/unit/modules/module_advanced.he"
    this->tail = 0;

#line 18 "sharp-test/unit/modules/module_advanced.he"
    this->count = 0;
}


#line 49 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__int__is_empty(RingBuf__int * this) {
    return this->count == 0;
}


#line 47 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) long RingBuf__int__size(RingBuf__int * this) {
    return this->count;
}


#line 21 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__int__push(RingBuf__int * this, int val) {

#line 22 "sharp-test/unit/modules/module_advanced.he"
    if (this->count >= this->cap) 
        return 0;

#line 23 "sharp-test/unit/modules/module_advanced.he"
    this->data[this->tail] = val;

#line 24 "sharp-test/unit/modules/module_advanced.he"
    this->tail = (this->tail + 1) % this->cap;

#line 25 "sharp-test/unit/modules/module_advanced.he"
    this->count = this->count + 1;

#line 26 "sharp-test/unit/modules/module_advanced.he"
    return 1;
}


#line 37 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) int RingBuf__int__peek(RingBuf__int * this) {

#line 38 "sharp-test/unit/modules/module_advanced.he"
    return this->data[this->head];
}


#line 29 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__int__pop(RingBuf__int * this, int * out) {

#line 30 "sharp-test/unit/modules/module_advanced.he"
    if (this->count == 0) 
        return 0;

#line 31 "sharp-test/unit/modules/module_advanced.he"
    *out = this->data[this->head];

#line 32 "sharp-test/unit/modules/module_advanced.he"
    this->head = (this->head + 1) % this->cap;

#line 33 "sharp-test/unit/modules/module_advanced.he"
    this->count = this->count - 1;

#line 34 "sharp-test/unit/modules/module_advanced.he"
    return 1;
}


#line 50 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__int__is_full(RingBuf__int * this) {
    return this->count >= this->cap;
}


#line 41 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) void RingBuf__int__clear(RingBuf__int * this) {

#line 42 "sharp-test/unit/modules/module_advanced.he"
    this->head = 0;

#line 43 "sharp-test/unit/modules/module_advanced.he"
    this->tail = 0;

#line 44 "sharp-test/unit/modules/module_advanced.he"
    this->count = 0;
}


#line 13 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) void RingBuf__float__init(RingBuf__float * this, float * buf, long capacity) {

#line 14 "sharp-test/unit/modules/module_advanced.he"
    this->data = buf;

#line 15 "sharp-test/unit/modules/module_advanced.he"
    this->cap = capacity;

#line 16 "sharp-test/unit/modules/module_advanced.he"
    this->head = 0;

#line 17 "sharp-test/unit/modules/module_advanced.he"
    this->tail = 0;

#line 18 "sharp-test/unit/modules/module_advanced.he"
    this->count = 0;
}


#line 21 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__float__push(RingBuf__float * this, float val) {

#line 22 "sharp-test/unit/modules/module_advanced.he"
    if (this->count >= this->cap) 
        return 0;

#line 23 "sharp-test/unit/modules/module_advanced.he"
    this->data[this->tail] = val;

#line 24 "sharp-test/unit/modules/module_advanced.he"
    this->tail = (this->tail + 1) % this->cap;

#line 25 "sharp-test/unit/modules/module_advanced.he"
    this->count = this->count + 1;

#line 26 "sharp-test/unit/modules/module_advanced.he"
    return 1;
}


#line 47 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) long RingBuf__float__size(RingBuf__float * this) {
    return this->count;
}


#line 48 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) long RingBuf__float__capacity(RingBuf__float * this) {
    return this->cap;
}


#line 29 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) _Bool RingBuf__float__pop(RingBuf__float * this, float * out) {

#line 30 "sharp-test/unit/modules/module_advanced.he"
    if (this->count == 0) 
        return 0;

#line 31 "sharp-test/unit/modules/module_advanced.he"
    *out = this->data[this->head];

#line 32 "sharp-test/unit/modules/module_advanced.he"
    this->head = (this->head + 1) % this->cap;

#line 33 "sharp-test/unit/modules/module_advanced.he"
    this->count = this->count - 1;

#line 34 "sharp-test/unit/modules/module_advanced.he"
    return 1;
}


#line 37 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) float RingBuf__float__peek(RingBuf__float * this) {

#line 38 "sharp-test/unit/modules/module_advanced.he"
    return this->data[this->head];
}


#line 52 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) int clamp_val__int(int val, int lo, int hi) {

#line 53 "sharp-test/unit/modules/module_advanced.he"
    if (val < lo) 
        return lo;

#line 54 "sharp-test/unit/modules/module_advanced.he"
    if (val > hi) 
        return hi;

#line 55 "sharp-test/unit/modules/module_advanced.he"
    return val;
}


#line 52 "sharp-test/unit/modules/module_advanced.he"
__attribute__((weak)) float clamp_val__float(float val, float lo, float hi) {

#line 53 "sharp-test/unit/modules/module_advanced.he"
    if (val < lo) 
        return lo;

#line 54 "sharp-test/unit/modules/module_advanced.he"
    if (val > hi) 
        return hi;

#line 55 "sharp-test/unit/modules/module_advanced.he"
    return val;
}

