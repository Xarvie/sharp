
#line 5 "sharp-test/unit/const/volatile.ce"
typedef struct Data Data;
struct Data {
    int secret;
};

#line 8 "sharp-test/unit/const/volatile.ce"
int Data__get_val(Data * this);

#line 9 "sharp-test/unit/const/volatile.ce"
void Data__set_val(Data * this, int v);

#line 13 "sharp-test/unit/const/volatile.ce"
typedef struct Sensor Sensor;
struct Sensor {
    volatile int reading;
    const int id;
};

#line 17 "sharp-test/unit/const/volatile.ce"
int Sensor__get_reading(Sensor * this);

#line 18 "sharp-test/unit/const/volatile.ce"
int Sensor__get_id(Sensor * this);

#line 8 "sharp-test/unit/const/volatile.ce"
int Data__get_val(Data * this) {
    return this->secret;
}

#line 9 "sharp-test/unit/const/volatile.ce"
void Data__set_val(Data * this, int v) {
    this->secret = v;
}

#line 10 "sharp-test/unit/const/volatile.ce"
int read_val(const Data * d) {
    return Data__get_val(d);
}

#line 17 "sharp-test/unit/const/volatile.ce"
int Sensor__get_reading(Sensor * this) {
    return this->reading;
}

#line 18 "sharp-test/unit/const/volatile.ce"
int Sensor__get_id(Sensor * this) {
    return this->id;
}

#line 20 "sharp-test/unit/const/volatile.ce"
int main() {

#line 22 "sharp-test/unit/const/volatile.ce"
    Data d;

#line 23 "sharp-test/unit/const/volatile.ce"
    Data__set_val(&d, 128);

#line 24 "sharp-test/unit/const/volatile.ce"
    if (Data__get_val(&d) != 128) 
        return 1;

#line 25 "sharp-test/unit/const/volatile.ce"
    int v = read_val(&d);

#line 26 "sharp-test/unit/const/volatile.ce"
    if (v != 128) 
        return 2;

#line 29 "sharp-test/unit/const/volatile.ce"
    Sensor s = { 0, 42 };

#line 30 "sharp-test/unit/const/volatile.ce"
    if (s.id != 42) 
        return 3;

#line 31 "sharp-test/unit/const/volatile.ce"
    int v1 = s.reading;

#line 32 "sharp-test/unit/const/volatile.ce"
    int v2 = s.reading;

#line 33 "sharp-test/unit/const/volatile.ce"
    if (v1 != v2) 
        return 4;

#line 34 "sharp-test/unit/const/volatile.ce"
    const Sensor * sp = &s;

#line 35 "sharp-test/unit/const/volatile.ce"
    if (Sensor__get_id(sp) != 42) 
        return 5;

#line 38 "sharp-test/unit/const/volatile.ce"
    s.reading = 100;

#line 39 "sharp-test/unit/const/volatile.ce"
    if (Sensor__get_reading(&s) != 100) 
        return 6;

#line 42 "sharp-test/unit/const/volatile.ce"
    const Sensor * sp2 = &s;

#line 43 "sharp-test/unit/const/volatile.ce"
    if (Sensor__get_reading(sp2) != 100) 
        return 7;

#line 44 "sharp-test/unit/const/volatile.ce"
    if (Sensor__get_id(sp2) != 42) 
        return 8;

#line 47 "sharp-test/unit/const/volatile.ce"
    Data d2;

#line 48 "sharp-test/unit/const/volatile.ce"
    Data__set_val(&d2, 999);

#line 49 "sharp-test/unit/const/volatile.ce"
    const Data * pd2 = &d2;

#line 50 "sharp-test/unit/const/volatile.ce"
    if (read_val(pd2) != 999) 
        return 9;

#line 52 "sharp-test/unit/const/volatile.ce"
    return 0;
}
