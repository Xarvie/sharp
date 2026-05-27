#include <stdbool.h>

#include <stdlib.h>

#line 6 "unit/modules/generic_inst.sph"
typedef long isize;

#line 7 "unit/modules/generic_inst.sph"
typedef unsigned long usize;

#line 9 "unit/modules/generic_inst.sph"

#line 9 "unit/modules/generic_inst.sph"
typedef struct RingBuf__int RingBuf__int;
struct RingBuf__int {
    int * data;
    isize cap;
    isize head;
    isize tail;
    isize count;
};



#line 17 "unit/modules/generic_inst.sph"
void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity);

#line 41 "unit/modules/generic_inst.sph"
long RingBuf__int__size(RingBuf__int * this);

#line 25 "unit/modules/generic_inst.sph"
_Bool RingBuf__int__push(RingBuf__int * this, int val);

#line 33 "unit/modules/generic_inst.sph"
_Bool RingBuf__int__pop(RingBuf__int * this, int * out);

#line 17 "unit/modules/generic_inst.sph"

#line 25 "unit/modules/generic_inst.sph"

#line 33 "unit/modules/generic_inst.sph"

#line 41 "unit/modules/generic_inst.sph"

#line 7 "unit/modules/generic_inst.sp"
int main() {

#line 8 "unit/modules/generic_inst.sp"
    int buf[8] = { 0 };

#line 10 "unit/modules/generic_inst.sp"
    RingBuf__int rb;

#line 11 "unit/modules/generic_inst.sp"
    RingBuf__int__init(&rb, buf, 8);

#line 13 "unit/modules/generic_inst.sp"
    if (RingBuf__int__size(&rb) != 0) 
        return 1;

#line 15 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__push(&rb, 10)) 
        return 2;

#line 16 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__push(&rb, 20)) 
        return 3;

#line 17 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__push(&rb, 30)) 
        return 4;

#line 19 "unit/modules/generic_inst.sp"
    if (RingBuf__int__size(&rb) != 3) 
        return 5;

#line 21 "unit/modules/generic_inst.sp"
    int val = 0;

#line 22 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 6;

#line 23 "unit/modules/generic_inst.sp"
    if (val != 10) 
        return 7;

#line 24 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 8;

#line 25 "unit/modules/generic_inst.sp"
    if (val != 20) 
        return 9;

#line 26 "unit/modules/generic_inst.sp"
    if (!RingBuf__int__pop(&rb, &val)) 
        return 10;

#line 27 "unit/modules/generic_inst.sp"
    if (val != 30) 
        return 11;

#line 28 "unit/modules/generic_inst.sp"
    if (RingBuf__int__size(&rb) != 0) 
        return 12;

#line 30 "unit/modules/generic_inst.sp"
    return 0;
}

#line 17 "unit/modules/generic_inst.sph"
__attribute__((weak)) void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity) {

#line 18 "unit/modules/generic_inst.sph"
    this->data = buf;

#line 19 "unit/modules/generic_inst.sph"
    this->cap = capacity;

#line 20 "unit/modules/generic_inst.sph"
    this->head = 0;

#line 21 "unit/modules/generic_inst.sph"
    this->tail = 0;

#line 22 "unit/modules/generic_inst.sph"
    this->count = 0;
}


#line 41 "unit/modules/generic_inst.sph"
__attribute__((weak)) long RingBuf__int__size(RingBuf__int * this) {
    return this->count;
}


#line 25 "unit/modules/generic_inst.sph"
__attribute__((weak)) _Bool RingBuf__int__push(RingBuf__int * this, int val) {

#line 26 "unit/modules/generic_inst.sph"
    if (this->count >= this->cap) 
        return 0;

#line 27 "unit/modules/generic_inst.sph"
    this->data[this->tail] = val;

#line 28 "unit/modules/generic_inst.sph"
    this->tail = (this->tail + 1) % this->cap;

#line 29 "unit/modules/generic_inst.sph"
    this->count = this->count + 1;

#line 30 "unit/modules/generic_inst.sph"
    return 1;
}


#line 33 "unit/modules/generic_inst.sph"
__attribute__((weak)) _Bool RingBuf__int__pop(RingBuf__int * this, int * out) {

#line 34 "unit/modules/generic_inst.sph"
    if (this->count == 0) 
        return 0;

#line 35 "unit/modules/generic_inst.sph"
    *out = this->data[this->head];

#line 36 "unit/modules/generic_inst.sph"
    this->head = (this->head + 1) % this->cap;

#line 37 "unit/modules/generic_inst.sph"
    this->count = this->count - 1;

#line 38 "unit/modules/generic_inst.sph"
    return 1;
}

