#include <stdbool.h>


#line 3 "c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 5 "c_superset_probes/p262_ringbuf.sph"

#line 5 "c_superset_probes/p262_ringbuf.sph"
typedef struct RingBuf__int RingBuf__int;
struct RingBuf__int {
    int * data;
    isize cap;
    isize head;
    isize tail;
    isize count;
};



#line 13 "c_superset_probes/p262_ringbuf.sph"
void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity);

#line 37 "c_superset_probes/p262_ringbuf.sph"
long RingBuf__int__size(RingBuf__int * this);

#line 21 "c_superset_probes/p262_ringbuf.sph"
_Bool RingBuf__int__push(RingBuf__int * this, int val);

#line 29 "c_superset_probes/p262_ringbuf.sph"
_Bool RingBuf__int__pop(RingBuf__int * this, int * out);

#line 9 "c_superset_probes/p262_module_generic.sp"
int main() {

#line 10 "c_superset_probes/p262_module_generic.sp"
    int buf[8] = { 0 };

#line 12 "c_superset_probes/p262_module_generic.sp"
    RingBuf__int rb;

#line 13 "c_superset_probes/p262_module_generic.sp"
    RingBuf__int__init(&rb, buf, 8);

#line 15 "c_superset_probes/p262_module_generic.sp"
    if (RingBuf__int__size(&rb) != 0) 
        return 1;

#line 17 "c_superset_probes/p262_module_generic.sp"
    _Bool ok = RingBuf__int__push(&rb, 10);

#line 18 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 2;

#line 19 "c_superset_probes/p262_module_generic.sp"
    ok = RingBuf__int__push(&rb, 20);

#line 20 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 3;

#line 21 "c_superset_probes/p262_module_generic.sp"
    ok = RingBuf__int__push(&rb, 30);

#line 22 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 4;

#line 24 "c_superset_probes/p262_module_generic.sp"
    if (RingBuf__int__size(&rb) != 3) 
        return 5;

#line 26 "c_superset_probes/p262_module_generic.sp"
    int val = 0;

#line 27 "c_superset_probes/p262_module_generic.sp"
    ok = RingBuf__int__pop(&rb, &val);

#line 28 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 6;

#line 29 "c_superset_probes/p262_module_generic.sp"
    if (val != 10) 
        return 7;

#line 31 "c_superset_probes/p262_module_generic.sp"
    ok = RingBuf__int__pop(&rb, &val);

#line 32 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 8;

#line 33 "c_superset_probes/p262_module_generic.sp"
    if (val != 20) 
        return 9;

#line 35 "c_superset_probes/p262_module_generic.sp"
    ok = RingBuf__int__pop(&rb, &val);

#line 36 "c_superset_probes/p262_module_generic.sp"
    if (!ok) 
        return 10;

#line 37 "c_superset_probes/p262_module_generic.sp"
    if (val != 30) 
        return 11;

#line 39 "c_superset_probes/p262_module_generic.sp"
    if (RingBuf__int__size(&rb) != 0) 
        return 12;

#line 41 "c_superset_probes/p262_module_generic.sp"
    return 0;
}

#line 13 "c_superset_probes/p262_ringbuf.sph"
__attribute__((weak)) void RingBuf__int__init(RingBuf__int * this, int * buf, long capacity) {

#line 14 "c_superset_probes/p262_ringbuf.sph"
    this->data = buf;

#line 15 "c_superset_probes/p262_ringbuf.sph"
    this->cap = capacity;

#line 16 "c_superset_probes/p262_ringbuf.sph"
    this->head = 0;

#line 17 "c_superset_probes/p262_ringbuf.sph"
    this->tail = 0;

#line 18 "c_superset_probes/p262_ringbuf.sph"
    this->count = 0;
}


#line 37 "c_superset_probes/p262_ringbuf.sph"
__attribute__((weak)) long RingBuf__int__size(RingBuf__int * this) {
    return this->count;
}


#line 21 "c_superset_probes/p262_ringbuf.sph"
__attribute__((weak)) _Bool RingBuf__int__push(RingBuf__int * this, int val) {

#line 22 "c_superset_probes/p262_ringbuf.sph"
    if (this->count >= this->cap) 
        return 0;

#line 23 "c_superset_probes/p262_ringbuf.sph"
    this->data[this->tail] = val;

#line 24 "c_superset_probes/p262_ringbuf.sph"
    this->tail = (this->tail + 1) % this->cap;

#line 25 "c_superset_probes/p262_ringbuf.sph"
    this->count = this->count + 1;

#line 26 "c_superset_probes/p262_ringbuf.sph"
    return 1;
}


#line 29 "c_superset_probes/p262_ringbuf.sph"
__attribute__((weak)) _Bool RingBuf__int__pop(RingBuf__int * this, int * out) {

#line 30 "c_superset_probes/p262_ringbuf.sph"
    if (this->count == 0) 
        return 0;

#line 31 "c_superset_probes/p262_ringbuf.sph"
    *out = this->data[this->head];

#line 32 "c_superset_probes/p262_ringbuf.sph"
    this->head = (this->head + 1) % this->cap;

#line 33 "c_superset_probes/p262_ringbuf.sph"
    this->count = this->count - 1;

#line 34 "c_superset_probes/p262_ringbuf.sph"
    return 1;
}

