
#line 2 "sharp-test/unit/codegen/union.ce"
union Data {
    int i;
    float f;
    char c;
};

#line 10 "sharp-test/unit/codegen/union.ce"
struct __anon_struct_0 {
    int lo;
    int hi;
};

#line 8 "sharp-test/unit/codegen/union.ce"
union Value {
    long long ll;
    struct {
    int lo;
    int hi;
    };
};

#line 14 "sharp-test/unit/codegen/union.ce"
union Data transform(union Data d) {

#line 15 "sharp-test/unit/codegen/union.ce"
    d.i = d.i * 2;

#line 16 "sharp-test/unit/codegen/union.ce"
    return d;
}

#line 19 "sharp-test/unit/codegen/union.ce"
int main() {

#line 21 "sharp-test/unit/codegen/union.ce"
    union Data d;

#line 22 "sharp-test/unit/codegen/union.ce"
    d.i = 42;

#line 23 "sharp-test/unit/codegen/union.ce"
    if (d.i != 42) 
        return 1;

#line 24 "sharp-test/unit/codegen/union.ce"
    d.f = 3.14f;

#line 25 "sharp-test/unit/codegen/union.ce"
    d.c = 'A';

#line 26 "sharp-test/unit/codegen/union.ce"
    if (d.c != 'A') 
        return 2;

#line 29 "sharp-test/unit/codegen/union.ce"
    if (sizeof(union Data) < sizeof(int)) 
        return 3;

#line 30 "sharp-test/unit/codegen/union.ce"
    if (sizeof(union Data) < sizeof(float)) 
        return 4;

#line 33 "sharp-test/unit/codegen/union.ce"
    union Data arr[3];

#line 34 "sharp-test/unit/codegen/union.ce"
    arr[0].i = 1;

#line 35 "sharp-test/unit/codegen/union.ce"
    arr[1].f = 2.0f;

#line 36 "sharp-test/unit/codegen/union.ce"
    arr[2].c = 'Z';

#line 37 "sharp-test/unit/codegen/union.ce"
    if (arr[0].i != 1) 
        return 5;

#line 38 "sharp-test/unit/codegen/union.ce"
    if (arr[2].c != 'Z') 
        return 6;

#line 41 "sharp-test/unit/codegen/union.ce"
    union Data input;

#line 42 "sharp-test/unit/codegen/union.ce"
    input.i = 21;

#line 43 "sharp-test/unit/codegen/union.ce"
    union Data output = transform(input);

#line 44 "sharp-test/unit/codegen/union.ce"
    if (output.i != 42) 
        return 7;

#line 46 "sharp-test/unit/codegen/union.ce"
    return 0;
}
