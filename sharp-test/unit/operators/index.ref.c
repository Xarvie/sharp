
#include <stdlib.h>

#line 4 "unit/operators/index.sp"
typedef struct Arr Arr;
struct Arr {
    int data[4];
};

#line 7 "unit/operators/index.sp"
int Arr__op_idx(Arr * this, int i);
int Arr__op_idx(Arr * this, int i) {
    return this->data[i];
}

#line 9 "unit/operators/index.sp"
int main(void) {

#line 10 "unit/operators/index.sp"
    Arr a;

#line 11 "unit/operators/index.sp"
    a.data[0] = 10;
    a.data[1] = 20;
    a.data[2] = 30;
    a.data[3] = 40;

#line 12 "unit/operators/index.sp"
    if (Arr__op_idx(&a, 0) != 10) 
        return 1;

#line 13 "unit/operators/index.sp"
    if (Arr__op_idx(&a, 2) != 30) 
        return 2;

#line 14 "unit/operators/index.sp"
    return 0;
}
