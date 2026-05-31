
#line 7 "sharp-test/unit/auto/basic.ce"
struct Point {
    int x;
    int y;
};

#line 11 "sharp-test/unit/auto/basic.ce"
struct Val {
    int x;
};

#line 5 "sharp-test/unit/auto/basic.ce"
int identity(int x) {
    return x;
}

#line 8 "sharp-test/unit/auto/basic.ce"
struct Point make_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

#line 13 "sharp-test/unit/auto/basic.ce"
int main(void) {

#line 15 "sharp-test/unit/auto/basic.ce"
    int a = 42;

#line 16 "sharp-test/unit/auto/basic.ce"
    if (a != 42) 
        return 1;

#line 17 "sharp-test/unit/auto/basic.ce"
    int b = identity(7);

#line 18 "sharp-test/unit/auto/basic.ce"
    if (b != 7) 
        return 2;

#line 19 "sharp-test/unit/auto/basic.ce"
    struct Point p = make_point(3, 4);

#line 20 "sharp-test/unit/auto/basic.ce"
    if (p.x != 3) 
        return 3;

#line 21 "sharp-test/unit/auto/basic.ce"
    if (p.y != 4) 
        return 4;

#line 22 "sharp-test/unit/auto/basic.ce"
    int sum = 0;

#line 23 "sharp-test/unit/auto/basic.ce"
    for (int i = 0; i < 5; i = i + 1) 

#line 24 "sharp-test/unit/auto/basic.ce"
        sum = sum + i;

#line 25 "sharp-test/unit/auto/basic.ce"
    if (sum != 10) 
        return 5;

#line 28 "sharp-test/unit/auto/basic.ce"
    double f = 3.14f;

#line 29 "sharp-test/unit/auto/basic.ce"
    if ((int)f != 3) 
        return 6;

#line 30 "sharp-test/unit/auto/basic.ce"
    double d = 7.0;

#line 31 "sharp-test/unit/auto/basic.ce"
    if ((int)d != 7) 
        return 7;

#line 32 "sharp-test/unit/auto/basic.ce"
    int c = 'A';

#line 33 "sharp-test/unit/auto/basic.ce"
    if (c != 65) 
        return 8;

#line 34 "sharp-test/unit/auto/basic.ce"
    const char * s = "hello";

#line 35 "sharp-test/unit/auto/basic.ce"
    if (s[0] != 'h') 
        return 9;

#line 38 "sharp-test/unit/auto/basic.ce"
    int m = 10;

#line 39 "sharp-test/unit/auto/basic.ce"
    int n = 20;

#line 40 "sharp-test/unit/auto/basic.ce"
    int sum2 = m + n;

#line 41 "sharp-test/unit/auto/basic.ce"
    if (sum2 != 30) 
        return 10;

#line 42 "sharp-test/unit/auto/basic.ce"
    float fa = 3.0f;

#line 43 "sharp-test/unit/auto/basic.ce"
    float fb = 4.0f;

#line 44 "sharp-test/unit/auto/basic.ce"
    float prod = fa * fb;

#line 45 "sharp-test/unit/auto/basic.ce"
    if ((int)prod != 12) 
        return 11;

#line 48 "sharp-test/unit/auto/basic.ce"
    int z = 42;

#line 49 "sharp-test/unit/auto/basic.ce"
    int * ptr = &z;

#line 50 "sharp-test/unit/auto/basic.ce"
    if (*ptr != 42) 
        return 12;

#line 51 "sharp-test/unit/auto/basic.ce"
    int cc = 99;

#line 52 "sharp-test/unit/auto/basic.ce"
    if (cc != 99) 
        return 13;

#line 53 "sharp-test/unit/auto/basic.ce"
    int cond = 1;

#line 54 "sharp-test/unit/auto/basic.ce"
    if (cond) {

#line 55 "sharp-test/unit/auto/basic.ce"
        int inside = 77;

#line 56 "sharp-test/unit/auto/basic.ce"
        if (inside != 77) 
            return 14;
    }

#line 60 "sharp-test/unit/auto/basic.ce"
    int arr[4] = { 10, 20, 30, 40 };

#line 61 "sharp-test/unit/auto/basic.ce"
    int elem = arr[2];

#line 62 "sharp-test/unit/auto/basic.ce"
    if (elem != 30) 
        return 15;

#line 65 "sharp-test/unit/auto/basic.ce"
    struct Point pt;
    pt.x = 100;
    pt.y = 200;

#line 66 "sharp-test/unit/auto/basic.ce"
    int px = pt.x;

#line 67 "sharp-test/unit/auto/basic.ce"
    if (px != 100) 
        return 16;

#line 69 "sharp-test/unit/auto/basic.ce"
    return 0;
}
