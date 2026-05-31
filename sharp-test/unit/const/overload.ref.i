
#line 7 "sharp-test/unit/const/overload.ce"
typedef struct Counter Counter;
struct Counter {
    long value;
};

#line 10 "sharp-test/unit/const/overload.ce"
Counter Counter__from(long start);

#line 13 "sharp-test/unit/const/overload.ce"
void Counter__inc(Counter * this);

#line 14 "sharp-test/unit/const/overload.ce"
long Counter__get(Counter * this);

#line 17 "sharp-test/unit/const/overload.ce"
typedef struct Buffer Buffer;
struct Buffer {
    int data;
};

#line 20 "sharp-test/unit/const/overload.ce"
int Buffer__read(Buffer * this);

#line 21 "sharp-test/unit/const/overload.ce"
void Buffer__write(Buffer * this, int v);

#line 10 "sharp-test/unit/const/overload.ce"
Counter Counter__from(long start) {

#line 11 "sharp-test/unit/const/overload.ce"
    Counter c;
    c.value = start;
    return c;
}

#line 13 "sharp-test/unit/const/overload.ce"
void Counter__inc(Counter * this) {
    this->value = this->value + 1;
}

#line 14 "sharp-test/unit/const/overload.ce"
long Counter__get(Counter * this) {
    return this->value;
}

#line 20 "sharp-test/unit/const/overload.ce"
int Buffer__read(Buffer * this) {
    return this->data;
}

#line 21 "sharp-test/unit/const/overload.ce"
void Buffer__write(Buffer * this, int v) {
    this->data = v;
}

#line 22 "sharp-test/unit/const/overload.ce"
int read_const(const Buffer * b) {
    return Buffer__read(b);
}

#line 24 "sharp-test/unit/const/overload.ce"
int main() {

#line 26 "sharp-test/unit/const/overload.ce"
    Counter c = Counter__from(100);

#line 27 "sharp-test/unit/const/overload.ce"
    if (Counter__get(&c) != 100) 
        return 1;

#line 28 "sharp-test/unit/const/overload.ce"
    Counter__inc(&c);

#line 29 "sharp-test/unit/const/overload.ce"
    if (Counter__get(&c) != 101) 
        return 2;

#line 30 "sharp-test/unit/const/overload.ce"
    Counter__inc(&c);

#line 31 "sharp-test/unit/const/overload.ce"
    Counter__inc(&c);

#line 32 "sharp-test/unit/const/overload.ce"
    if (Counter__get(&c) != 103) 
        return 3;

#line 35 "sharp-test/unit/const/overload.ce"
    Buffer b = { 0 };

#line 36 "sharp-test/unit/const/overload.ce"
    Buffer__write(&b, 42);

#line 37 "sharp-test/unit/const/overload.ce"
    if (Buffer__read(&b) != 42) 
        return 4;

#line 38 "sharp-test/unit/const/overload.ce"
    const Buffer cb = b;

#line 39 "sharp-test/unit/const/overload.ce"
    if (read_const(&cb) != 42) 
        return 5;

#line 42 "sharp-test/unit/const/overload.ce"
    Buffer b2 = { 0 };

#line 43 "sharp-test/unit/const/overload.ce"
    Buffer__write(&b2, 7);

#line 44 "sharp-test/unit/const/overload.ce"
    int v1 = Buffer__read(&b2);

#line 45 "sharp-test/unit/const/overload.ce"
    const Buffer * pb2 = &b2;

#line 46 "sharp-test/unit/const/overload.ce"
    int v2 = Buffer__read(pb2);

#line 47 "sharp-test/unit/const/overload.ce"
    if (v1 != 7) 
        return 6;

#line 48 "sharp-test/unit/const/overload.ce"
    if (v2 != 7) 
        return 7;

#line 51 "sharp-test/unit/const/overload.ce"
    Counter c2 = Counter__from(50);

#line 52 "sharp-test/unit/const/overload.ce"
    if (Counter__get(&c2) != 50) 
        return 8;

#line 55 "sharp-test/unit/const/overload.ce"
    Buffer cb1;
    cb1.data = 10;

#line 56 "sharp-test/unit/const/overload.ce"
    Buffer cb2;
    cb2.data = 20;

#line 57 "sharp-test/unit/const/overload.ce"
    if (Buffer__read(&cb1) != 10) 
        return 9;

#line 58 "sharp-test/unit/const/overload.ce"
    if (Buffer__read(&cb2) != 20) 
        return 10;

#line 60 "sharp-test/unit/const/overload.ce"
    return 0;
}
