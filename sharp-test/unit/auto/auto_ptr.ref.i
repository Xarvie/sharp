
#line 2 "sharp-test/unit/auto/auto_ptr.ce"
int add(int a, int b) {
    return a + b;
}

#line 4 "sharp-test/unit/auto/auto_ptr.ce"
int main() {

#line 6 "sharp-test/unit/auto/auto_ptr.ce"
    int val = 42;

#line 7 "sharp-test/unit/auto/auto_ptr.ce"
    int * p = &val;

#line 8 "sharp-test/unit/auto/auto_ptr.ce"
    if (*p != 42) 
        return 1;

#line 9 "sharp-test/unit/auto/auto_ptr.ce"
    *p = 100;

#line 10 "sharp-test/unit/auto/auto_ptr.ce"
    if (val != 100) 
        return 2;

#line 13 "sharp-test/unit/auto/auto_ptr.ce"
    int (*fp)(int, int) = add;

#line 14 "sharp-test/unit/auto/auto_ptr.ce"
    int result = fp(3, 4);

#line 15 "sharp-test/unit/auto/auto_ptr.ce"
    if (result != 7) 
        return 3;

#line 18 "sharp-test/unit/auto/auto_ptr.ce"
    int arr[5] = { 10, 20, 30, 40, 50 };

#line 19 "sharp-test/unit/auto/auto_ptr.ce"
    int elem = arr[2];

#line 20 "sharp-test/unit/auto/auto_ptr.ce"
    if (elem != 30) 
        return 4;

#line 23 "sharp-test/unit/auto/auto_ptr.ce"
    struct Pair {
    int a;
    int b;
};


#line 24 "sharp-test/unit/auto/auto_ptr.ce"
    struct Pair pr;

#line 25 "sharp-test/unit/auto/auto_ptr.ce"
    pr.a = 7;
    pr.b = 13;

#line 26 "sharp-test/unit/auto/auto_ptr.ce"
    int fa = pr.a;

#line 27 "sharp-test/unit/auto/auto_ptr.ce"
    if (fa != 7) 
        return 5;

#line 30 "sharp-test/unit/auto/auto_ptr.ce"
    int x = 5;

#line 31 "sharp-test/unit/auto/auto_ptr.ce"
    int y = x;

#line 32 "sharp-test/unit/auto/auto_ptr.ce"
    int z = y;

#line 33 "sharp-test/unit/auto/auto_ptr.ce"
    if (z != 5) 
        return 6;

#line 35 "sharp-test/unit/auto/auto_ptr.ce"
    return 0;
}
