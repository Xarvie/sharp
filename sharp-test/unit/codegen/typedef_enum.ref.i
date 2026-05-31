
#line 5 "sharp-test/unit/codegen/typedef_enum.ce"
typedef int MyInt;

#line 6 "sharp-test/unit/codegen/typedef_enum.ce"
typedef MyInt YourInt;

#line 7 "sharp-test/unit/codegen/typedef_enum.ce"
typedef YourInt OurInt;

#line 12 "sharp-test/unit/codegen/typedef_enum.ce"
enum Status {
    OK,
    WARN,
    ERR,
    FATAL = 99
};

#line 27 "sharp-test/unit/codegen/typedef_enum.ce"
typedef enum Color {
    RED,
    GREEN,
    BLUE
} ColorT;

#line 9 "sharp-test/unit/codegen/typedef_enum.ce"
int get_max() {
    return 2147483647;
}

#line 19 "sharp-test/unit/codegen/typedef_enum.ce"
int get_status_val(enum Status s) {

#line 20 "sharp-test/unit/codegen/typedef_enum.ce"
    if (s == OK) 
        return 0;

#line 21 "sharp-test/unit/codegen/typedef_enum.ce"
    if (s == WARN) 
        return 1;

#line 22 "sharp-test/unit/codegen/typedef_enum.ce"
    if (s == ERR) 
        return 2;

#line 23 "sharp-test/unit/codegen/typedef_enum.ce"
    if (s == FATAL) 
        return 99;

#line 24 "sharp-test/unit/codegen/typedef_enum.ce"
    return -1;
}

#line 28 "sharp-test/unit/codegen/typedef_enum.ce"
ColorT get_color(int idx) {

#line 29 "sharp-test/unit/codegen/typedef_enum.ce"
    switch (idx) {

#line 30 "sharp-test/unit/codegen/typedef_enum.ce"
        case 0:
        return RED;

#line 31 "sharp-test/unit/codegen/typedef_enum.ce"
        case 1:
        return GREEN;

#line 32 "sharp-test/unit/codegen/typedef_enum.ce"
        case 2:
        return BLUE;

#line 33 "sharp-test/unit/codegen/typedef_enum.ce"
        default:
        return RED;
    }
}

#line 37 "sharp-test/unit/codegen/typedef_enum.ce"
int main() {

#line 39 "sharp-test/unit/codegen/typedef_enum.ce"
    OurInt a = 100;

#line 40 "sharp-test/unit/codegen/typedef_enum.ce"
    MyInt b = 200;

#line 41 "sharp-test/unit/codegen/typedef_enum.ce"
    int c = a + b;

#line 42 "sharp-test/unit/codegen/typedef_enum.ce"
    if (c != 300) 
        return 1;

#line 43 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_max() != 2147483647) 
        return 2;

#line 46 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_status_val(OK) != 0) 
        return 10;

#line 47 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_status_val(WARN) != 1) 
        return 11;

#line 48 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_status_val(ERR) != 2) 
        return 12;

#line 49 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_status_val(FATAL) != 99) 
        return 13;

#line 51 "sharp-test/unit/codegen/typedef_enum.ce"
    enum Status s = WARN;

#line 52 "sharp-test/unit/codegen/typedef_enum.ce"
    if (s != WARN) 
        return 14;

#line 55 "sharp-test/unit/codegen/typedef_enum.ce"
    ColorT clr = get_color(1);

#line 56 "sharp-test/unit/codegen/typedef_enum.ce"
    if (clr != GREEN) 
        return 15;

#line 57 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_color(0) != RED) 
        return 16;

#line 58 "sharp-test/unit/codegen/typedef_enum.ce"
    if (get_color(99) != RED) 
        return 17;

#line 61 "sharp-test/unit/codegen/typedef_enum.ce"
    OurInt x = 50;

#line 62 "sharp-test/unit/codegen/typedef_enum.ce"
    YourInt y = 25;

#line 63 "sharp-test/unit/codegen/typedef_enum.ce"
    OurInt z = x - y;

#line 64 "sharp-test/unit/codegen/typedef_enum.ce"
    if (z != 25) 
        return 18;

#line 66 "sharp-test/unit/codegen/typedef_enum.ce"
    return 0;
}
