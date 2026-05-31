
#line 7 "sharp-test/unit/extensions/runtime.ce"
typedef struct Value Value;
struct Value {
    int data;
};

#line 10 "sharp-test/unit/extensions/runtime.ce"
int call_get_value(Value v);

#line 18 "sharp-test/unit/extensions/runtime.ce"
int Value__get_value(Value * this);

#line 21 "sharp-test/unit/extensions/runtime.ce"
typedef struct Buffer Buffer;
struct Buffer {
    int size;
    int buf[16];
};

#line 27 "sharp-test/unit/extensions/runtime.ce"
int Buffer__peek_first(Buffer * this);

#line 32 "sharp-test/unit/extensions/runtime.ce"
void Buffer__append(Buffer * this, int v);

#line 49 "sharp-test/unit/extensions/runtime.ce"
int Value__get_value_sq(Value * this);

#line 51 "sharp-test/unit/extensions/runtime.ce"
int Buffer__sum(Buffer * this);

#line 10 "sharp-test/unit/extensions/runtime.ce"
int call_get_value(Value v);

#line 13 "sharp-test/unit/extensions/runtime.ce"
int call_get_value(Value v) {

#line 14 "sharp-test/unit/extensions/runtime.ce"
    return Value__get_value(&v);
}

#line 18 "sharp-test/unit/extensions/runtime.ce"
int Value__get_value(Value * this) {
    return this->data;
}

#line 27 "sharp-test/unit/extensions/runtime.ce"
int Buffer__peek_first(Buffer * this) {

#line 28 "sharp-test/unit/extensions/runtime.ce"
    if (this->size > 0) 
        return this->buf[0];

#line 29 "sharp-test/unit/extensions/runtime.ce"
    return -1;
}

#line 32 "sharp-test/unit/extensions/runtime.ce"
void Buffer__append(Buffer * this, int v) {

#line 33 "sharp-test/unit/extensions/runtime.ce"
    if (this->size < 16) {

#line 34 "sharp-test/unit/extensions/runtime.ce"
        this->buf[this->size] = v;

#line 35 "sharp-test/unit/extensions/runtime.ce"
        this->size = this->size + 1;
    }
}

#line 40 "sharp-test/unit/extensions/runtime.ce"
int test_buffer() {

#line 41 "sharp-test/unit/extensions/runtime.ce"
    Buffer b = { 0 };

#line 42 "sharp-test/unit/extensions/runtime.ce"
    Buffer__append(&b, 42);

#line 43 "sharp-test/unit/extensions/runtime.ce"
    Buffer__append(&b, 99);

#line 44 "sharp-test/unit/extensions/runtime.ce"
    int pk = Buffer__peek_first(&b);

#line 45 "sharp-test/unit/extensions/runtime.ce"
    if (pk != 42) 
        return 21;

#line 46 "sharp-test/unit/extensions/runtime.ce"
    return 0;
}

#line 49 "sharp-test/unit/extensions/runtime.ce"
int Value__get_value_sq(Value * this) {
    return this->data * this->data;
}

#line 51 "sharp-test/unit/extensions/runtime.ce"
int Buffer__sum(Buffer * this) {

#line 52 "sharp-test/unit/extensions/runtime.ce"
    int s = 0;

#line 53 "sharp-test/unit/extensions/runtime.ce"
    for (int i = 0; i < this->size; i = i + 1) 
        s = s + this->buf[i];

#line 54 "sharp-test/unit/extensions/runtime.ce"
    return s;
}

#line 57 "sharp-test/unit/extensions/runtime.ce"
int main() {

#line 59 "sharp-test/unit/extensions/runtime.ce"
    Value v;
    v.data = 128;

#line 60 "sharp-test/unit/extensions/runtime.ce"
    int result = call_get_value(v);

#line 61 "sharp-test/unit/extensions/runtime.ce"
    if (result != 128) 
        return 1;

#line 64 "sharp-test/unit/extensions/runtime.ce"
    int direct = Value__get_value(&v);

#line 65 "sharp-test/unit/extensions/runtime.ce"
    if (direct != 128) 
        return 2;

#line 68 "sharp-test/unit/extensions/runtime.ce"
    int r = test_buffer();

#line 69 "sharp-test/unit/extensions/runtime.ce"
    if (r != 0) 
        return r;

#line 72 "sharp-test/unit/extensions/runtime.ce"
    Value cv;
    cv.data = 7;

#line 73 "sharp-test/unit/extensions/runtime.ce"
    const Value * pcv = &cv;

#line 74 "sharp-test/unit/extensions/runtime.ce"
    if (Value__get_value_sq(pcv) != 49) 
        return 3;

#line 77 "sharp-test/unit/extensions/runtime.ce"
    Buffer b2 = { 0 };

#line 78 "sharp-test/unit/extensions/runtime.ce"
    Buffer__append(&b2, 10);

#line 79 "sharp-test/unit/extensions/runtime.ce"
    Buffer__append(&b2, 20);

#line 80 "sharp-test/unit/extensions/runtime.ce"
    Buffer__append(&b2, 30);

#line 81 "sharp-test/unit/extensions/runtime.ce"
    if (Buffer__sum(&b2) != 60) 
        return 4;

#line 83 "sharp-test/unit/extensions/runtime.ce"
    return 0;
}
