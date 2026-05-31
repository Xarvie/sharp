
#line 13 "sharp-test/unit/auto/complex.ce"
typedef struct DataContainer DataContainer;
struct DataContainer {
    int storage[3];
};

#line 16 "sharp-test/unit/auto/complex.ce"
int DataContainer__op_idx(DataContainer * this, long i);

#line 23 "sharp-test/unit/auto/complex.ce"
typedef struct Data Data;
struct Data {
    int value;
};

#line 26 "sharp-test/unit/auto/complex.ce"
int Data__read(Data * this);

#line 27 "sharp-test/unit/auto/complex.ce"
void Data__init(Data * this, int v);

#line 7 "sharp-test/unit/auto/complex.ce"
typedef struct Value__int Value__int;
struct Value__int {
    int data;
};


typedef struct Value__float Value__float;
struct Value__float {
    float data;
};



#line 10 "sharp-test/unit/auto/complex.ce"
int Value__int__get(Value__int * this);
float Value__float__get(Value__float * this);

#line 16 "sharp-test/unit/auto/complex.ce"
int DataContainer__op_idx(DataContainer * this, long i) {
    return this->storage[i];
}

#line 19 "sharp-test/unit/auto/complex.ce"
int get_int() {
    return 42;
}

#line 20 "sharp-test/unit/auto/complex.ce"
float get_float() {
    return 3.14f;
}

#line 26 "sharp-test/unit/auto/complex.ce"
int Data__read(Data * this) {
    return this->value;
}

#line 27 "sharp-test/unit/auto/complex.ce"
void Data__init(Data * this, int v) {
    this->value = v;
}

#line 29 "sharp-test/unit/auto/complex.ce"
int main() {

#line 31 "sharp-test/unit/auto/complex.ce"
    Value__int v;

#line 32 "sharp-test/unit/auto/complex.ce"
    v.data = 42;

#line 33 "sharp-test/unit/auto/complex.ce"
    int x = Value__int__get(&v);

#line 34 "sharp-test/unit/auto/complex.ce"
    if (x != 42) 
        return 1;

#line 36 "sharp-test/unit/auto/complex.ce"
    Value__float vf;

#line 37 "sharp-test/unit/auto/complex.ce"
    vf.data = 7.0f;

#line 38 "sharp-test/unit/auto/complex.ce"
    float y = Value__float__get(&vf);

#line 39 "sharp-test/unit/auto/complex.ce"
    if ((int)y != 7) 
        return 2;

#line 42 "sharp-test/unit/auto/complex.ce"
    DataContainer dc;

#line 43 "sharp-test/unit/auto/complex.ce"
    dc.storage[0] = 10;

#line 44 "sharp-test/unit/auto/complex.ce"
    dc.storage[1] = 20;

#line 45 "sharp-test/unit/auto/complex.ce"
    int a = DataContainer__op_idx(&dc, 0);

#line 46 "sharp-test/unit/auto/complex.ce"
    int b = DataContainer__op_idx(&dc, 1);

#line 47 "sharp-test/unit/auto/complex.ce"
    if (a != 10) 
        return 3;

#line 48 "sharp-test/unit/auto/complex.ce"
    if (b != 20) 
        return 4;

#line 51 "sharp-test/unit/auto/complex.ce"
    int i1 = 1 + 2;

#line 52 "sharp-test/unit/auto/complex.ce"
    if (i1 != 3) 
        return 5;

#line 53 "sharp-test/unit/auto/complex.ce"
    double f1 = 1.0f + 2.0f;

#line 54 "sharp-test/unit/auto/complex.ce"
    if (f1 < 2.9f || f1 > 3.1f) 
        return 6;

#line 57 "sharp-test/unit/auto/complex.ce"
    double m1 = 1 + 2.5f;

#line 58 "sharp-test/unit/auto/complex.ce"
    if (m1 < 3.4f || m1 > 3.6f) 
        return 7;

#line 61 "sharp-test/unit/auto/complex.ce"
    int r1 = get_int();

#line 62 "sharp-test/unit/auto/complex.ce"
    if (r1 != 42) 
        return 8;

#line 63 "sharp-test/unit/auto/complex.ce"
    float r2 = get_float();

#line 64 "sharp-test/unit/auto/complex.ce"
    if (r2 < 3.13f || r2 > 3.15f) 
        return 9;

#line 67 "sharp-test/unit/auto/complex.ce"
    int t1 = 1 ? 10 : 20;

#line 68 "sharp-test/unit/auto/complex.ce"
    if (t1 != 10) 
        return 10;

#line 71 "sharp-test/unit/auto/complex.ce"
    Data d;

#line 72 "sharp-test/unit/auto/complex.ce"
    Data__init(&d, 77);

#line 73 "sharp-test/unit/auto/complex.ce"
    int v1 = Data__read(&d);

#line 74 "sharp-test/unit/auto/complex.ce"
    if (v1 != 77) 
        return 11;

#line 77 "sharp-test/unit/auto/complex.ce"
    int raw = 55;

#line 78 "sharp-test/unit/auto/complex.ce"
    int * rp = &raw;

#line 79 "sharp-test/unit/auto/complex.ce"
    if (*rp != 55) 
        return 12;

#line 80 "sharp-test/unit/auto/complex.ce"
    *rp = 66;

#line 81 "sharp-test/unit/auto/complex.ce"
    if (raw != 66) 
        return 13;

#line 84 "sharp-test/unit/auto/complex.ce"
    int nested = (1 + 2) * (3 + 4);

#line 85 "sharp-test/unit/auto/complex.ce"
    if (nested != 21) 
        return 14;

#line 87 "sharp-test/unit/auto/complex.ce"
    return 0;
}

#line 10 "sharp-test/unit/auto/complex.ce"
__attribute__((weak)) int Value__int__get(Value__int * this) {
    return this->data;
}

__attribute__((weak)) float Value__float__get(Value__float * this) {
    return this->data;
}

