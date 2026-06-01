
#line 2 "sharp-test/unit/codegen/funcptr.ce"
int add(int a, int b) {
    return a + b;
}

#line 3 "sharp-test/unit/codegen/funcptr.ce"
int sub(int a, int b) {
    return a - b;
}

#line 4 "sharp-test/unit/codegen/funcptr.ce"
int mul(int a, int b) {
    return a * b;
}

#line 6 "sharp-test/unit/codegen/funcptr.ce"
int apply(int (*fn)(int, int), int a, int b) {

#line 7 "sharp-test/unit/codegen/funcptr.ce"
    return fn(a, b);
}

#line 11 "sharp-test/unit/codegen/funcptr.ce"
int (*get_op(int idx))(int, int) {

#line 12 "sharp-test/unit/codegen/funcptr.ce"
    if (idx == 0) 
        return add;

#line 13 "sharp-test/unit/codegen/funcptr.ce"
    if (idx == 1) 
        return sub;

#line 14 "sharp-test/unit/codegen/funcptr.ce"
    return mul;
}

#line 17 "sharp-test/unit/codegen/funcptr.ce"
int main() {

#line 19 "sharp-test/unit/codegen/funcptr.ce"
    int (*op)(int, int) = add;

#line 20 "sharp-test/unit/codegen/funcptr.ce"
    if (op(3, 4) != 7) 
        return 1;

#line 23 "sharp-test/unit/codegen/funcptr.ce"
    op = sub;

#line 24 "sharp-test/unit/codegen/funcptr.ce"
    if (op(10, 3) != 7) 
        return 2;

#line 25 "sharp-test/unit/codegen/funcptr.ce"
    op = mul;

#line 26 "sharp-test/unit/codegen/funcptr.ce"
    if (op(3, 4) != 12) 
        return 3;

#line 29 "sharp-test/unit/codegen/funcptr.ce"
    if (apply(add, 5, 3) != 8) 
        return 4;

#line 30 "sharp-test/unit/codegen/funcptr.ce"
    if (apply(mul, 5, 3) != 15) 
        return 5;

#line 33 "sharp-test/unit/codegen/funcptr.ce"
    int (* ops[3])(int, int) = { add, sub, mul };

#line 34 "sharp-test/unit/codegen/funcptr.ce"
    if (ops[0](10, 5) != 15) 
        return 6;

#line 35 "sharp-test/unit/codegen/funcptr.ce"
    if (ops[1](10, 5) != 5) 
        return 7;

#line 36 "sharp-test/unit/codegen/funcptr.ce"
    if (ops[2](10, 5) != 50) 
        return 8;

#line 39 "sharp-test/unit/codegen/funcptr.ce"
    if (get_op(0)(7, 3) != 10) 
        return 9;

#line 40 "sharp-test/unit/codegen/funcptr.ce"
    if (get_op(1)(7, 3) != 4) 
        return 10;

#line 42 "sharp-test/unit/codegen/funcptr.ce"
    return 0;
}
