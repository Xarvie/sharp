
#include <stdlib.h>

#line 7 "unit/const/overload.sp"
typedef struct Counter Counter;
struct Counter {
    long value;
};

#line 10 "unit/const/overload.sp"
Counter Counter__from(long start);

#line 13 "unit/const/overload.sp"
void Counter__inc(Counter * this);

#line 14 "unit/const/overload.sp"
long Counter__get(Counter * this);

#line 17 "unit/const/overload.sp"
typedef struct Buffer Buffer;
struct Buffer {
    int data;
};

#line 20 "unit/const/overload.sp"
int Buffer__read(Buffer * this);

#line 21 "unit/const/overload.sp"
void Buffer__write(Buffer * this, int v);

#line 10 "unit/const/overload.sp"
Counter Counter__from(long start) {

#line 11 "unit/const/overload.sp"
    Counter c;
    c.value = start;
    return c;
}

#line 13 "unit/const/overload.sp"
void Counter__inc(Counter * this) {
    this->value = this->value + 1;
}

#line 14 "unit/const/overload.sp"
long Counter__get(Counter * this) {
    return this->value;
}

#line 20 "unit/const/overload.sp"
int Buffer__read(Buffer * this) {
    return this->data;
}

#line 21 "unit/const/overload.sp"
void Buffer__write(Buffer * this, int v) {
    this->data = v;
}

#line 22 "unit/const/overload.sp"
int read_const(const Buffer * b) {
    return Buffer__read(b);
}

#line 24 "unit/const/overload.sp"
int main() {

#line 26 "unit/const/overload.sp"
    Counter c = Counter__from(100);

#line 27 "unit/const/overload.sp"
    if (Counter__get(&c) != 100) 
        return 1;

#line 28 "unit/const/overload.sp"
    Counter__inc(&c);

#line 29 "unit/const/overload.sp"
    if (Counter__get(&c) != 101) 
        return 2;

#line 30 "unit/const/overload.sp"
    Counter__inc(&c);

#line 31 "unit/const/overload.sp"
    Counter__inc(&c);

#line 32 "unit/const/overload.sp"
    if (Counter__get(&c) != 103) 
        return 3;

#line 35 "unit/const/overload.sp"
    Buffer b = { 0 };

#line 36 "unit/const/overload.sp"
    Buffer__write(&b, 42);

#line 37 "unit/const/overload.sp"
    if (Buffer__read(&b) != 42) 
        return 4;

#line 38 "unit/const/overload.sp"
    const Buffer cb = b;

#line 39 "unit/const/overload.sp"
    if (read_const(&cb) != 42) 
        return 5;

#line 41 "unit/const/overload.sp"
    return 0;
}
