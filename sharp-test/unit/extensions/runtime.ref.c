
#include <stdlib.h>

#line 7 "unit/extensions/runtime.sp"
typedef struct Value Value;
struct Value {
    int data;
};

#line 10 "unit/extensions/runtime.sp"
int call_get_value(Value v);

#line 18 "unit/extensions/runtime.sp"
int Value__get_value(Value * this);

#line 21 "unit/extensions/runtime.sp"
typedef struct Buffer Buffer;
struct Buffer {
    int size;
    int buf[16];
};

#line 27 "unit/extensions/runtime.sp"
int Buffer__peek_first(Buffer * this);

#line 32 "unit/extensions/runtime.sp"
void Buffer__append(Buffer * this, int v);

#line 10 "unit/extensions/runtime.sp"
int call_get_value(Value v);

#line 13 "unit/extensions/runtime.sp"
int call_get_value(Value v) {

#line 14 "unit/extensions/runtime.sp"
    return Value__get_value(&v);
}

#line 18 "unit/extensions/runtime.sp"
int Value__get_value(Value * this) {
    return this->data;
}

#line 27 "unit/extensions/runtime.sp"
int Buffer__peek_first(Buffer * this) {

#line 28 "unit/extensions/runtime.sp"
    if (this->size > 0) 
        return this->buf[0];

#line 29 "unit/extensions/runtime.sp"
    return -1;
}

#line 32 "unit/extensions/runtime.sp"
void Buffer__append(Buffer * this, int v) {

#line 33 "unit/extensions/runtime.sp"
    if (this->size < 16) {

#line 34 "unit/extensions/runtime.sp"
        this->buf[this->size] = v;

#line 35 "unit/extensions/runtime.sp"
        this->size = this->size + 1;
    }
}

#line 40 "unit/extensions/runtime.sp"
int test_buffer() {

#line 41 "unit/extensions/runtime.sp"
    Buffer b = { 0 };

#line 42 "unit/extensions/runtime.sp"
    Buffer__append(&b, 42);

#line 43 "unit/extensions/runtime.sp"
    Buffer__append(&b, 99);

#line 44 "unit/extensions/runtime.sp"
    int pk = Buffer__peek_first(&b);

#line 45 "unit/extensions/runtime.sp"
    if (pk != 42) 
        return 21;

#line 46 "unit/extensions/runtime.sp"
    return 0;
}

#line 49 "unit/extensions/runtime.sp"
int main() {

#line 51 "unit/extensions/runtime.sp"
    Value v;
    v.data = 128;

#line 52 "unit/extensions/runtime.sp"
    int result = call_get_value(v);

#line 53 "unit/extensions/runtime.sp"
    if (result != 128) 
        return 1;

#line 56 "unit/extensions/runtime.sp"
    int direct = Value__get_value(&v);

#line 57 "unit/extensions/runtime.sp"
    if (direct != 128) 
        return 2;

#line 60 "unit/extensions/runtime.sp"
    int r = test_buffer();

#line 61 "unit/extensions/runtime.sp"
    if (r != 0) 
        return r;

#line 63 "unit/extensions/runtime.sp"
    return 0;
}
