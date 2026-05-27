
#include <stdlib.h>

#line 13 "unit/auto/complex.sp"
typedef struct DataContainer DataContainer;
struct DataContainer {
    int storage[3];
};

#line 16 "unit/auto/complex.sp"
int DataContainer__op_idx(DataContainer * this, long i);

#line 23 "unit/auto/complex.sp"
typedef struct Data Data;
struct Data {
    int value;
};

#line 26 "unit/auto/complex.sp"
int Data__read(Data * this);

#line 27 "unit/auto/complex.sp"
void Data__init(Data * this, int v);

#line 7 "unit/auto/complex.sp"
typedef struct Value__int Value__int;
struct Value__int {
    int data;
};


typedef struct Value__float Value__float;
struct Value__float {
    float data;
};



#line 10 "unit/auto/complex.sp"
int Value__int__get(Value__int * this);
float Value__float__get(Value__float * this);

#line 16 "unit/auto/complex.sp"
int DataContainer__op_idx(DataContainer * this, long i) {
    return this->storage[i];
}

#line 19 "unit/auto/complex.sp"
int get_int() {
    return 42;
}

#line 20 "unit/auto/complex.sp"
float get_float() {
    return 3.14f;
}

#line 26 "unit/auto/complex.sp"
int Data__read(Data * this) {
    return this->value;
}

#line 27 "unit/auto/complex.sp"
void Data__init(Data * this, int v) {
    this->value = v;
}

#line 29 "unit/auto/complex.sp"
int main() {

#line 31 "unit/auto/complex.sp"
    Value__int v;

#line 32 "unit/auto/complex.sp"
    v.data = 42;

#line 33 "unit/auto/complex.sp"
    int x = Value__int__get(&v);

#line 34 "unit/auto/complex.sp"
    if (x != 42) 
        return 1;

#line 36 "unit/auto/complex.sp"
    Value__float vf;

#line 37 "unit/auto/complex.sp"
    vf.data = 7.0f;

#line 38 "unit/auto/complex.sp"
    float y = Value__float__get(&vf);

#line 39 "unit/auto/complex.sp"
    if ((int)y != 7) 
        return 2;

#line 42 "unit/auto/complex.sp"
    DataContainer dc;

#line 43 "unit/auto/complex.sp"
    dc.storage[0] = 10;

#line 44 "unit/auto/complex.sp"
    dc.storage[1] = 20;

#line 45 "unit/auto/complex.sp"
    int a = DataContainer__op_idx(&dc, 0);

#line 46 "unit/auto/complex.sp"
    int b = DataContainer__op_idx(&dc, 1);

#line 47 "unit/auto/complex.sp"
    if (a != 10) 
        return 3;

#line 48 "unit/auto/complex.sp"
    if (b != 20) 
        return 4;

#line 51 "unit/auto/complex.sp"
    int i1 = 1 + 2;

#line 52 "unit/auto/complex.sp"
    if (i1 != 3) 
        return 5;

#line 53 "unit/auto/complex.sp"
    double f1 = 1.0f + 2.0f;

#line 54 "unit/auto/complex.sp"
    if (f1 < 2.9f || f1 > 3.1f) 
        return 6;

#line 57 "unit/auto/complex.sp"
    double m1 = 1 + 2.5f;

#line 58 "unit/auto/complex.sp"
    if (m1 < 3.4f || m1 > 3.6f) 
        return 7;

#line 61 "unit/auto/complex.sp"
    int r1 = get_int();

#line 62 "unit/auto/complex.sp"
    if (r1 != 42) 
        return 8;

#line 63 "unit/auto/complex.sp"
    float r2 = get_float();

#line 64 "unit/auto/complex.sp"
    if (r2 < 3.13f || r2 > 3.15f) 
        return 9;

#line 67 "unit/auto/complex.sp"
    int t1 = 1 ? 10 : 20;

#line 68 "unit/auto/complex.sp"
    if (t1 != 10) 
        return 10;

#line 71 "unit/auto/complex.sp"
    Data d;

#line 72 "unit/auto/complex.sp"
    Data__init(&d, 77);

#line 73 "unit/auto/complex.sp"
    int v1 = Data__read(&d);

#line 74 "unit/auto/complex.sp"
    if (v1 != 77) 
        return 11;

#line 76 "unit/auto/complex.sp"
    return 0;
}

#line 10 "unit/auto/complex.sp"
__attribute__((weak)) int Value__int__get(Value__int * this) {
    return this->data;
}

__attribute__((weak)) float Value__float__get(Value__float * this) {
    return this->data;
}

