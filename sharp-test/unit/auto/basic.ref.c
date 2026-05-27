
#include <stdlib.h>

#line 7 "unit/auto/basic.sp"
struct Point {
    int x;
    int y;
};

#line 11 "unit/auto/basic.sp"
struct Val {
    int x;
};

#line 5 "unit/auto/basic.sp"
int identity(int x) {
    return x;
}

#line 8 "unit/auto/basic.sp"
struct Point make_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

#line 13 "unit/auto/basic.sp"
int main(void) {

#line 15 "unit/auto/basic.sp"
    int a = 42;

#line 16 "unit/auto/basic.sp"
    if (a != 42) 
        return 1;

#line 17 "unit/auto/basic.sp"
    int b = identity(7);

#line 18 "unit/auto/basic.sp"
    if (b != 7) 
        return 2;

#line 19 "unit/auto/basic.sp"
    struct Point p = make_point(3, 4);

#line 20 "unit/auto/basic.sp"
    if (p.x != 3) 
        return 3;

#line 21 "unit/auto/basic.sp"
    if (p.y != 4) 
        return 4;

#line 22 "unit/auto/basic.sp"
    int sum = 0;

#line 23 "unit/auto/basic.sp"
    for (int i = 0; i < 5; i = i + 1) 

#line 24 "unit/auto/basic.sp"
        sum = sum + i;

#line 25 "unit/auto/basic.sp"
    if (sum != 10) 
        return 5;

#line 28 "unit/auto/basic.sp"
    double f = 3.14f;

#line 29 "unit/auto/basic.sp"
    if ((int)f != 3) 
        return 6;

#line 30 "unit/auto/basic.sp"
    double d = 7.0;

#line 31 "unit/auto/basic.sp"
    if ((int)d != 7) 
        return 7;

#line 32 "unit/auto/basic.sp"
    int c = 'A';

#line 33 "unit/auto/basic.sp"
    if (c != 65) 
        return 8;

#line 34 "unit/auto/basic.sp"
    const char * s = "hello";

#line 35 "unit/auto/basic.sp"
    if (s[0] != 'h') 
        return 9;

#line 38 "unit/auto/basic.sp"
    int m = 10;

#line 39 "unit/auto/basic.sp"
    int n = 20;

#line 40 "unit/auto/basic.sp"
    int sum2 = m + n;

#line 41 "unit/auto/basic.sp"
    if (sum2 != 30) 
        return 10;

#line 42 "unit/auto/basic.sp"
    float fa = 3.0f;

#line 43 "unit/auto/basic.sp"
    float fb = 4.0f;

#line 44 "unit/auto/basic.sp"
    float prod = fa * fb;

#line 45 "unit/auto/basic.sp"
    if ((int)prod != 12) 
        return 11;

#line 48 "unit/auto/basic.sp"
    int z = 42;

#line 49 "unit/auto/basic.sp"
    int * ptr = &z;

#line 50 "unit/auto/basic.sp"
    if (*ptr != 42) 
        return 12;

#line 51 "unit/auto/basic.sp"
    int cc = 99;

#line 52 "unit/auto/basic.sp"
    if (cc != 99) 
        return 13;

#line 53 "unit/auto/basic.sp"
    int cond = 1;

#line 54 "unit/auto/basic.sp"
    if (cond) {

#line 55 "unit/auto/basic.sp"
        int inside = 77;

#line 56 "unit/auto/basic.sp"
        if (inside != 77) 
            return 14;
    }

#line 59 "unit/auto/basic.sp"
    return 0;
}
