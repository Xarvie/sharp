

#line 8 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
typedef struct Buffer Buffer;
struct Buffer {
    int data;
};

#line 13 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
int Buffer__read(Buffer * this);

#line 18 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
void Buffer__write(Buffer * this, int v);

#line 13 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
int Buffer__read(Buffer * this) {

#line 14 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    return this->data;
}

#line 18 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
void Buffer__write(Buffer * this, int v) {

#line 19 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    this->data = v;
}

#line 23 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
int read_const(const Buffer * b) {

#line 24 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    return Buffer__read(b);
}

#line 27 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
int main() {

#line 28 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    Buffer b = { 0 };

#line 29 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    Buffer__write(&b, 42);

#line 31 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    if (Buffer__read(&b) != 42) 
        return 1;

#line 34 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    const Buffer cb = b;

#line 35 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    if (read_const(&cb) != 42) 
        return 2;

#line 37 "sharp-test/c_superset_probes/p244_ext_const_overload.sp"
    return 0;
}
