
#line 5 "sharp-test/unit/codegen/typedef_enum.sp"
typedef int MyInt;

#line 6 "sharp-test/unit/codegen/typedef_enum.sp"
typedef MyInt YourInt;

#line 7 "sharp-test/unit/codegen/typedef_enum.sp"
typedef YourInt OurInt;

#line 12 "sharp-test/unit/codegen/typedef_enum.sp"
enum Status {
    OK,
    WARN,
    ERR,
    FATAL = 99
};

#line 9 "sharp-test/unit/codegen/typedef_enum.sp"
int get_max() {
    return 2147483647;
}

#line 19 "sharp-test/unit/codegen/typedef_enum.sp"
int get_status_val(enum Status s) {

#line 20 "sharp-test/unit/codegen/typedef_enum.sp"
    if (s == OK) 
        return 0;

#line 21 "sharp-test/unit/codegen/typedef_enum.sp"
    if (s == WARN) 
        return 1;

#line 22 "sharp-test/unit/codegen/typedef_enum.sp"
    if (s == ERR) 
        return 2;

#line 23 "sharp-test/unit/codegen/typedef_enum.sp"
    if (s == FATAL) 
        return 99;

#line 24 "sharp-test/unit/codegen/typedef_enum.sp"
    return -1;
}

#line 27 "sharp-test/unit/codegen/typedef_enum.sp"
int main() {

#line 29 "sharp-test/unit/codegen/typedef_enum.sp"
    OurInt a = 100;

#line 30 "sharp-test/unit/codegen/typedef_enum.sp"
    MyInt b = 200;

#line 31 "sharp-test/unit/codegen/typedef_enum.sp"
    int c = a + b;

#line 32 "sharp-test/unit/codegen/typedef_enum.sp"
    if (c != 300) 
        return 1;

#line 33 "sharp-test/unit/codegen/typedef_enum.sp"
    if (get_max() != 2147483647) 
        return 2;

#line 36 "sharp-test/unit/codegen/typedef_enum.sp"
    if (get_status_val(OK) != 0) 
        return 10;

#line 37 "sharp-test/unit/codegen/typedef_enum.sp"
    if (get_status_val(WARN) != 1) 
        return 11;

#line 38 "sharp-test/unit/codegen/typedef_enum.sp"
    if (get_status_val(ERR) != 2) 
        return 12;

#line 39 "sharp-test/unit/codegen/typedef_enum.sp"
    if (get_status_val(FATAL) != 99) 
        return 13;

#line 41 "sharp-test/unit/codegen/typedef_enum.sp"
    enum Status s = WARN;

#line 42 "sharp-test/unit/codegen/typedef_enum.sp"
    if (s != WARN) 
        return 14;

#line 44 "sharp-test/unit/codegen/typedef_enum.sp"
    return 0;
}
