
#line 35 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 53 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 58 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 87 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 5 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Triple__int__float__long Triple__int__float__long;
struct Triple__int__float__long {
    int first;
    float second;
    long third;
};


typedef struct Triple__int__int__int Triple__int__int__int;
struct Triple__int__int__int {
    int first;
    int second;
    int third;
};


typedef struct Triple__int__long__float Triple__int__long__float;
struct Triple__int__long__float {
    int first;
    long second;
    float third;
};



#line 35 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Buffer__int Buffer__int;
struct Buffer__int {
    int data[8];
    int count;
};


typedef struct Buffer__float Buffer__float;
struct Buffer__float {
    float data[8];
    int count;
};



#line 58 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 53 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Pair__int__int Pair__int__int;
struct Pair__int__int {
    int first;
    int second;
};


typedef struct Wrapper__int Wrapper__int;
struct Wrapper__int {
    Pair__int__int inner;
    int tag;
};



#line 58 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 53 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Pair__float__int Pair__float__int;
struct Pair__float__int {
    float first;
    int second;
};


typedef struct Wrapper__float Wrapper__float;
struct Wrapper__float {
    Pair__float__int inner;
    int tag;
};



#line 87 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Container__int Container__int;
struct Container__int {
    int value;
    int ref_count;
};


typedef struct Container__float Container__float;
struct Container__float {
    float value;
    int ref_count;
};



#line 5 "sharp-test/unit/generics/generic_struct_advanced.ce"
typedef struct Triple__long__long__long Triple__long__long__long;
struct Triple__long__long__long {
    long first;
    long second;
    long third;
};



#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__int__int__set_first(Triple__int__int__int * this, int val);

#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__int__int__set_second(Triple__int__int__int * this, int val);

#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__int__int__set_third(Triple__int__int__int * this, int val);

#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Triple__int__int__int__get_first(Triple__int__int__int * this);

#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Triple__int__int__int__get_second(Triple__int__int__int * this);

#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Triple__int__int__int__get_third(Triple__int__int__int * this);

#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__long__float__set_first(Triple__int__long__float * this, int val);

#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__long__float__set_second(Triple__int__long__float * this, long val);

#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__int__long__float__set_third(Triple__int__long__float * this, float val);

#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Triple__int__long__float__get_first(Triple__int__long__float * this);

#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
long Triple__int__long__float__get_second(Triple__int__long__float * this);

#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
float Triple__int__long__float__get_third(Triple__int__long__float * this);

#line 40 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Buffer__int__append(Buffer__int * this, int val);

#line 49 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Buffer__int__len(Buffer__int * this);

#line 45 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Buffer__int__get(Buffer__int * this, int idx);

#line 40 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Buffer__float__append(Buffer__float * this, float val);

#line 49 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Buffer__float__len(Buffer__float * this);

#line 45 "sharp-test/unit/generics/generic_struct_advanced.ce"
float Buffer__float__get(Buffer__float * this, int idx);

#line 71 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Wrapper__int__set_wrapped(Wrapper__int * this, int val);

#line 75 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Wrapper__int__set_tag(Wrapper__int * this, int t);

#line 79 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Wrapper__int__set_inner_second(Wrapper__int * this, int v);

#line 63 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Wrapper__int__get_wrapped(Wrapper__int * this);

#line 67 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Wrapper__int__get_tag(Wrapper__int * this);

#line 83 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Wrapper__int__get_inner_second(Wrapper__int * this);

#line 71 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Wrapper__float__set_wrapped(Wrapper__float * this, float val);

#line 75 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Wrapper__float__set_tag(Wrapper__float * this, int t);

#line 63 "sharp-test/unit/generics/generic_struct_advanced.ce"
float Wrapper__float__get_wrapped(Wrapper__float * this);

#line 67 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Wrapper__float__get_tag(Wrapper__float * this);

#line 96 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Container__int__set_value(Container__int * this, int val);

#line 104 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Container__int__inc_ref(Container__int * this);

#line 100 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Container__int__get_ref_count(Container__int * this);

#line 92 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Container__int__get_value(Container__int * this);

#line 108 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Container__int__clone_value(Container__int * this);

#line 96 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Container__float__set_value(Container__float * this, float val);

#line 104 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Container__float__inc_ref(Container__float * this);

#line 92 "sharp-test/unit/generics/generic_struct_advanced.ce"
float Container__float__get_value(Container__float * this);

#line 100 "sharp-test/unit/generics/generic_struct_advanced.ce"
int Container__float__get_ref_count(Container__float * this);

#line 108 "sharp-test/unit/generics/generic_struct_advanced.ce"
float Container__float__clone_value(Container__float * this);

#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__long__long__long__set_first(Triple__long__long__long * this, long val);

#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__long__long__long__set_second(Triple__long__long__long * this, long val);

#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
void Triple__long__long__long__set_third(Triple__long__long__long * this, long val);

#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
long Triple__long__long__long__get_first(Triple__long__long__long * this);

#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
long Triple__long__long__long__get_second(Triple__long__long__long * this);

#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
long Triple__long__long__long__get_third(Triple__long__long__long * this);

#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 40 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 45 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 49 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 63 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 67 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 71 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 75 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 79 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 83 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 92 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 96 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 100 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 104 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 108 "sharp-test/unit/generics/generic_struct_advanced.ce"

#line 112 "sharp-test/unit/generics/generic_struct_advanced.ce"
int main() {

#line 114 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__float__long t;

#line 115 "sharp-test/unit/generics/generic_struct_advanced.ce"
    t.first = 10;

#line 116 "sharp-test/unit/generics/generic_struct_advanced.ce"
    t.second = 3.14f;

#line 117 "sharp-test/unit/generics/generic_struct_advanced.ce"
    t.third = 999L;

#line 118 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (t.first != 10) 
        return 1;

#line 119 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (t.second < 3.13f || t.second > 3.15f) 
        return 2;

#line 120 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (t.third != 999L) 
        return 3;

#line 123 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__int__int tiii;

#line 124 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__int__int__set_first(&tiii, 1);

#line 125 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__int__int__set_second(&tiii, 2);

#line 126 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__int__int__set_third(&tiii, 3);

#line 127 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__int__int__get_first(&tiii) != 1) 
        return 4;

#line 128 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__int__int__get_second(&tiii) != 2) 
        return 5;

#line 129 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__int__int__get_third(&tiii) != 3) 
        return 6;

#line 132 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__long__float tmix;

#line 133 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__long__float__set_first(&tmix, 42);

#line 134 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__long__float__set_second(&tmix, 100L);

#line 135 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__int__long__float__set_third(&tmix, 2.5f);

#line 136 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__long__float__get_first(&tmix) != 42) 
        return 7;

#line 137 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__long__float__get_second(&tmix) != 100L) 
        return 8;

#line 138 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__int__long__float__get_third(&tmix) < 2.49f || Triple__int__long__float__get_third(&tmix) > 2.51f) 
        return 9;

#line 141 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int buf;

#line 142 "sharp-test/unit/generics/generic_struct_advanced.ce"
    buf.count = 0;

#line 143 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int__append(&buf, 10);

#line 144 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int__append(&buf, 20);

#line 145 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int__append(&buf, 30);

#line 146 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__len(&buf) != 3) 
        return 10;

#line 147 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__get(&buf, 0) != 10) 
        return 11;

#line 148 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__get(&buf, 1) != 20) 
        return 12;

#line 149 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__get(&buf, 2) != 30) 
        return 13;

#line 152 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__float fbuf;

#line 153 "sharp-test/unit/generics/generic_struct_advanced.ce"
    fbuf.count = 0;

#line 154 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__float__append(&fbuf, 1.1f);

#line 155 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__float__append(&fbuf, 2.2f);

#line 156 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__float__len(&fbuf) != 2) 
        return 14;

#line 157 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__float__get(&fbuf, 0) < 1.0f || Buffer__float__get(&fbuf, 0) > 1.2f) 
        return 15;

#line 160 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__int w;

#line 161 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__int__set_wrapped(&w, 77);

#line 162 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__int__set_tag(&w, 5);

#line 163 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__int__set_inner_second(&w, 99);

#line 164 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Wrapper__int__get_wrapped(&w) != 77) 
        return 16;

#line 165 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Wrapper__int__get_tag(&w) != 5) 
        return 17;

#line 166 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Wrapper__int__get_inner_second(&w) != 99) 
        return 18;

#line 169 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (w.inner.first != 77) 
        return 19;

#line 170 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (w.inner.second != 99) 
        return 20;

#line 173 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__float wf;

#line 174 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__float__set_wrapped(&wf, 3.14f);

#line 175 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Wrapper__float__set_tag(&wf, 1);

#line 176 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Wrapper__float__get_wrapped(&wf) < 3.13f || Wrapper__float__get_wrapped(&wf) > 3.15f) 
        return 21;

#line 177 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Wrapper__float__get_tag(&wf) != 1) 
        return 22;

#line 180 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__int ci;

#line 181 "sharp-test/unit/generics/generic_struct_advanced.ce"
    ci.ref_count = 0;

#line 182 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__int__set_value(&ci, 42);

#line 183 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__int__inc_ref(&ci);

#line 184 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__int__inc_ref(&ci);

#line 185 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__int__get_ref_count(&ci) != 2) 
        return 23;

#line 188 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__int__get_value(&ci) != 42) 
        return 24;

#line 189 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__int__clone_value(&ci) != 42) 
        return 25;

#line 192 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__float cf;

#line 193 "sharp-test/unit/generics/generic_struct_advanced.ce"
    cf.ref_count = 0;

#line 194 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__float__set_value(&cf, 2.71f);

#line 195 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Container__float__inc_ref(&cf);

#line 196 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__float__get_value(&cf) < 2.70f || Container__float__get_value(&cf) > 2.72f) 
        return 26;

#line 197 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__float__get_ref_count(&cf) != 1) 
        return 27;

#line 198 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Container__float__clone_value(&cf) < 2.70f || Container__float__clone_value(&cf) > 2.72f) 
        return 28;

#line 201 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int buf2;

#line 202 "sharp-test/unit/generics/generic_struct_advanced.ce"
    buf2.count = 0;

#line 203 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int__append(&buf2, 100);

#line 204 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Buffer__int__append(&buf2, 200);

#line 205 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__get(&buf2, 0) != 100) 
        return 29;

#line 206 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Buffer__int__get(&buf2, 1) != 200) 
        return 30;

#line 209 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__long__long__long tlll;

#line 210 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__long__long__long__set_first(&tlll, 10L);

#line 211 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__long__long__long__set_second(&tlll, 20L);

#line 212 "sharp-test/unit/generics/generic_struct_advanced.ce"
    Triple__long__long__long__set_third(&tlll, 30L);

#line 213 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__long__long__long__get_first(&tlll) != 10L) 
        return 31;

#line 214 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__long__long__long__get_second(&tlll) != 20L) 
        return 32;

#line 215 "sharp-test/unit/generics/generic_struct_advanced.ce"
    if (Triple__long__long__long__get_third(&tlll) != 30L) 
        return 33;

#line 217 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return 0;
}

#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__int__int__set_first(Triple__int__int__int * this, int val) {

#line 24 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->first = val;
}


#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__int__int__set_second(Triple__int__int__int * this, int val) {

#line 28 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->second = val;
}


#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__int__int__set_third(Triple__int__int__int * this, int val) {

#line 32 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->third = val;
}


#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_first(Triple__int__int__int * this) {

#line 12 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->first;
}


#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_second(Triple__int__int__int * this) {

#line 16 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->second;
}


#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Triple__int__int__int__get_third(Triple__int__int__int * this) {

#line 20 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->third;
}


#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__long__float__set_first(Triple__int__long__float * this, int val) {

#line 24 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->first = val;
}


#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__long__float__set_second(Triple__int__long__float * this, long val) {

#line 28 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->second = val;
}


#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__int__long__float__set_third(Triple__int__long__float * this, float val) {

#line 32 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->third = val;
}


#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Triple__int__long__float__get_first(Triple__int__long__float * this) {

#line 12 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->first;
}


#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) long Triple__int__long__float__get_second(Triple__int__long__float * this) {

#line 16 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->second;
}


#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) float Triple__int__long__float__get_third(Triple__int__long__float * this) {

#line 20 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->third;
}


#line 40 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Buffer__int__append(Buffer__int * this, int val) {

#line 41 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->data[this->count] = val;

#line 42 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->count = this->count + 1;
}


#line 49 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Buffer__int__len(Buffer__int * this) {

#line 50 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->count;
}


#line 45 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Buffer__int__get(Buffer__int * this, int idx) {

#line 46 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->data[idx];
}


#line 40 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Buffer__float__append(Buffer__float * this, float val) {

#line 41 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->data[this->count] = val;

#line 42 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->count = this->count + 1;
}


#line 49 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Buffer__float__len(Buffer__float * this) {

#line 50 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->count;
}


#line 45 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) float Buffer__float__get(Buffer__float * this, int idx) {

#line 46 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->data[idx];
}


#line 71 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Wrapper__int__set_wrapped(Wrapper__int * this, int val) {

#line 72 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->inner.first = val;
}


#line 75 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Wrapper__int__set_tag(Wrapper__int * this, int t) {

#line 76 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->tag = t;
}


#line 79 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Wrapper__int__set_inner_second(Wrapper__int * this, int v) {

#line 80 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->inner.second = v;
}


#line 63 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Wrapper__int__get_wrapped(Wrapper__int * this) {

#line 64 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->inner.first;
}


#line 67 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Wrapper__int__get_tag(Wrapper__int * this) {

#line 68 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->tag;
}


#line 83 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Wrapper__int__get_inner_second(Wrapper__int * this) {

#line 84 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->inner.second;
}


#line 71 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Wrapper__float__set_wrapped(Wrapper__float * this, float val) {

#line 72 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->inner.first = val;
}


#line 75 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Wrapper__float__set_tag(Wrapper__float * this, int t) {

#line 76 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->tag = t;
}


#line 63 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) float Wrapper__float__get_wrapped(Wrapper__float * this) {

#line 64 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->inner.first;
}


#line 67 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Wrapper__float__get_tag(Wrapper__float * this) {

#line 68 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->tag;
}


#line 96 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Container__int__set_value(Container__int * this, int val) {

#line 97 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->value = val;
}


#line 104 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Container__int__inc_ref(Container__int * this) {

#line 105 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->ref_count = this->ref_count + 1;
}


#line 100 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Container__int__get_ref_count(Container__int * this) {

#line 101 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->ref_count;
}


#line 92 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Container__int__get_value(Container__int * this) {

#line 93 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->value;
}


#line 108 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Container__int__clone_value(Container__int * this) {

#line 109 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->value;
}


#line 96 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Container__float__set_value(Container__float * this, float val) {

#line 97 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->value = val;
}


#line 104 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Container__float__inc_ref(Container__float * this) {

#line 105 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->ref_count = this->ref_count + 1;
}


#line 92 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) float Container__float__get_value(Container__float * this) {

#line 93 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->value;
}


#line 100 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) int Container__float__get_ref_count(Container__float * this) {

#line 101 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->ref_count;
}


#line 108 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) float Container__float__clone_value(Container__float * this) {

#line 109 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->value;
}


#line 23 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__long__long__long__set_first(Triple__long__long__long * this, long val) {

#line 24 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->first = val;
}


#line 27 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__long__long__long__set_second(Triple__long__long__long * this, long val) {

#line 28 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->second = val;
}


#line 31 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) void Triple__long__long__long__set_third(Triple__long__long__long * this, long val) {

#line 32 "sharp-test/unit/generics/generic_struct_advanced.ce"
    this->third = val;
}


#line 11 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) long Triple__long__long__long__get_first(Triple__long__long__long * this) {

#line 12 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->first;
}


#line 15 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) long Triple__long__long__long__get_second(Triple__long__long__long * this) {

#line 16 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->second;
}


#line 19 "sharp-test/unit/generics/generic_struct_advanced.ce"
__attribute__((weak)) long Triple__long__long__long__get_third(Triple__long__long__long * this) {

#line 20 "sharp-test/unit/generics/generic_struct_advanced.ce"
    return this->third;
}

