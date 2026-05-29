
#line 6 "sharp-test/unit/modules/generic_inst.he"
typedef long isize;

#line 7 "sharp-test/unit/modules/generic_inst.he"
typedef unsigned long usize;

#line 9 "sharp-test/unit/modules/generic_inst.he"

#line 9 "sharp-test/unit/modules/generic_inst.he"
typedef struct RingBuf__int RingBuf__int;
struct RingBuf__int {
    int * data;
    isize cap;
    isize head;
    isize tail;
    isize count;
};



#line 17 "sharp-test/unit/modules/generic_inst.he"
void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity);

#line 41 "sharp-test/unit/modules/generic_inst.he"
long RingBuf__int__size(RingBuf__int * this);

#line 25 "sharp-test/unit/modules/generic_inst.he"
_Bool RingBuf__int__push(RingBuf__int * this, int val);

#line 33 "sharp-test/unit/modules/generic_inst.he"
_Bool RingBuf__int__pop(RingBuf__int * this, int * out);

#line 17 "sharp-test/unit/modules/generic_inst.he"

#line 25 "sharp-test/unit/modules/generic_inst.he"

#line 33 "sharp-test/unit/modules/generic_inst.he"

#line 41 "sharp-test/unit/modules/generic_inst.he"

#line 7 "sharp-test/unit/modules/generic_inst.ce"
int main() {

#line 8 "sharp-test/unit/modules/generic_inst.ce"
    int buf[8] = { 0 };

#line 10 "sharp-test/unit/modules/generic_inst.ce"
    RingBuf__int rb;

#line 11 "sharp-test/unit/modules/generic_inst.ce"
    RingBuf__int__init(&rb, buf, 8);

#line 13 "sharp-test/unit/modules/generic_inst.ce"
    if (RingBuf__int__size(&rb) != 0) 
        return 1;

#line 15 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__push(&rb, 10)) 
        return 2;

#line 16 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__push(&rb, 20)) 
        return 3;

#line 17 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__push(&rb, 30)) 
        return 4;

#line 19 "sharp-test/unit/modules/generic_inst.ce"
    if (RingBuf__int__size(&rb) != 3) 
        return 5;

#line 21 "sharp-test/unit/modules/generic_inst.ce"
    int val = 0;

#line 22 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 6;

#line 23 "sharp-test/unit/modules/generic_inst.ce"
    if (val != 10) 
        return 7;

#line 24 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 8;

#line 25 "sharp-test/unit/modules/generic_inst.ce"
    if (val != 20) 
        return 9;

#line 26 "sharp-test/unit/modules/generic_inst.ce"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 10;

#line 27 "sharp-test/unit/modules/generic_inst.ce"
    if (val != 30) 
        return 11;

#line 28 "sharp-test/unit/modules/generic_inst.ce"
    if (RingBuf__int__size(&rb) != 0) 
        return 12;

#line 30 "sharp-test/unit/modules/generic_inst.ce"
    return 0;
}

#line 17 "sharp-test/unit/modules/generic_inst.he"
__attribute__((weak)) void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity) {

#line 18 "sharp-test/unit/modules/generic_inst.he"
    this->data = buf;

#line 19 "sharp-test/unit/modules/generic_inst.he"
    this->cap = capacity;

#line 20 "sharp-test/unit/modules/generic_inst.he"
    this->head = 0;

#line 21 "sharp-test/unit/modules/generic_inst.he"
    this->tail = 0;

#line 22 "sharp-test/unit/modules/generic_inst.he"
    this->count = 0;
}


#line 41 "sharp-test/unit/modules/generic_inst.he"
__attribute__((weak)) long RingBuf__int__size(RingBuf__int * this) {
    return this->count;
}


#line 25 "sharp-test/unit/modules/generic_inst.he"
__attribute__((weak)) _Bool RingBuf__int__push(RingBuf__int * this, int val) {

#line 26 "sharp-test/unit/modules/generic_inst.he"
    if (this->count >= this->cap) 
        return 0;

#line 27 "sharp-test/unit/modules/generic_inst.he"
    this->data[this->tail] = val;

#line 28 "sharp-test/unit/modules/generic_inst.he"
    this->tail = (this->tail + 1) % this->cap;

#line 29 "sharp-test/unit/modules/generic_inst.he"
    this->count = this->count + 1;

#line 30 "sharp-test/unit/modules/generic_inst.he"
    return 1;
}


#line 33 "sharp-test/unit/modules/generic_inst.he"
__attribute__((weak)) _Bool RingBuf__int__pop(RingBuf__int * this, int * out) {

#line 34 "sharp-test/unit/modules/generic_inst.he"
    if (this->count == 0) 
        return 0;

#line 35 "sharp-test/unit/modules/generic_inst.he"
    *out = this->data[this->head];

#line 36 "sharp-test/unit/modules/generic_inst.he"
    this->head = (this->head + 1) % this->cap;

#line 37 "sharp-test/unit/modules/generic_inst.he"
    this->count = this->count - 1;

#line 38 "sharp-test/unit/modules/generic_inst.he"
    return 1;
}

