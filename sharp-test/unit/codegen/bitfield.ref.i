
#line 2 "sharp-test/unit/codegen/bitfield.ce"
struct Flags {
    unsigned int a : 1;
    unsigned int b : 3;
    unsigned int c : 4;
    unsigned int d : 8;
};

#line 9 "sharp-test/unit/codegen/bitfield.ce"
int main() {

#line 11 "sharp-test/unit/codegen/bitfield.ce"
    struct Flags f;

#line 12 "sharp-test/unit/codegen/bitfield.ce"
    f.a = 1;

#line 13 "sharp-test/unit/codegen/bitfield.ce"
    f.b = 5;

#line 14 "sharp-test/unit/codegen/bitfield.ce"
    f.c = 10;

#line 15 "sharp-test/unit/codegen/bitfield.ce"
    f.d = 200;

#line 16 "sharp-test/unit/codegen/bitfield.ce"
    if (f.a != 1) 
        return 1;

#line 17 "sharp-test/unit/codegen/bitfield.ce"
    if (f.b != 5) 
        return 2;

#line 18 "sharp-test/unit/codegen/bitfield.ce"
    if (f.c != 10) 
        return 3;

#line 19 "sharp-test/unit/codegen/bitfield.ce"
    if (f.d != 200) 
        return 4;

#line 22 "sharp-test/unit/codegen/bitfield.ce"
    f.b = 15;

#line 23 "sharp-test/unit/codegen/bitfield.ce"
    if (f.b != 7) 
        return 5;

#line 26 "sharp-test/unit/codegen/bitfield.ce"
    if (!f.a) 
        return 6;

#line 27 "sharp-test/unit/codegen/bitfield.ce"
    f.a = 0;

#line 28 "sharp-test/unit/codegen/bitfield.ce"
    if (f.a) 
        return 7;

#line 31 "sharp-test/unit/codegen/bitfield.ce"
    struct Flags g = { 0 };

#line 32 "sharp-test/unit/codegen/bitfield.ce"
    g.a = f.a;

#line 33 "sharp-test/unit/codegen/bitfield.ce"
    g.b = f.b;

#line 34 "sharp-test/unit/codegen/bitfield.ce"
    if (g.a != 0) 
        return 8;

#line 35 "sharp-test/unit/codegen/bitfield.ce"
    if (g.b != 7) 
        return 9;

#line 37 "sharp-test/unit/codegen/bitfield.ce"
    return 0;
}
