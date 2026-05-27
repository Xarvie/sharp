
#include <stdlib.h>

#line 5 "unit/const/volatile.sp"
typedef struct Data Data;
struct Data {
    int secret;
};

#line 8 "unit/const/volatile.sp"
int Data__get_val(Data * this);

#line 9 "unit/const/volatile.sp"
void Data__set_val(Data * this, int v);

#line 13 "unit/const/volatile.sp"
typedef struct Sensor Sensor;
struct Sensor {
    volatile int reading;
    const int id;
};

#line 17 "unit/const/volatile.sp"
int Sensor__get_reading(Sensor * this);

#line 18 "unit/const/volatile.sp"
int Sensor__get_id(Sensor * this);

#line 8 "unit/const/volatile.sp"
int Data__get_val(Data * this) {
    return this->secret;
}

#line 9 "unit/const/volatile.sp"
void Data__set_val(Data * this, int v) {
    this->secret = v;
}

#line 10 "unit/const/volatile.sp"
int read_val(const Data * d) {
    return Data__get_val(d);
}

#line 17 "unit/const/volatile.sp"
int Sensor__get_reading(Sensor * this) {
    return this->reading;
}

#line 18 "unit/const/volatile.sp"
int Sensor__get_id(Sensor * this) {
    return this->id;
}

#line 20 "unit/const/volatile.sp"
int main() {

#line 22 "unit/const/volatile.sp"
    Data d;

#line 23 "unit/const/volatile.sp"
    Data__set_val(&d, 128);

#line 24 "unit/const/volatile.sp"
    if (Data__get_val(&d) != 128) 
        return 1;

#line 25 "unit/const/volatile.sp"
    int v = read_val(&d);

#line 26 "unit/const/volatile.sp"
    if (v != 128) 
        return 2;

#line 29 "unit/const/volatile.sp"
    Sensor s = { 0, 42 };

#line 30 "unit/const/volatile.sp"
    if (s.id != 42) 
        return 3;

#line 31 "unit/const/volatile.sp"
    int v1 = s.reading;

#line 32 "unit/const/volatile.sp"
    int v2 = s.reading;

#line 33 "unit/const/volatile.sp"
    if (v1 != v2) 
        return 4;

#line 34 "unit/const/volatile.sp"
    const Sensor * sp = &s;

#line 35 "unit/const/volatile.sp"
    if (Sensor__get_id(sp) != 42) 
        return 5;

#line 37 "unit/const/volatile.sp"
    return 0;
}
