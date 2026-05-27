
#include <stdlib.h>

#line 9 "unit/export/basic.sp"

#line 10 "unit/export/basic.sp"

#line 4 "unit/export/basic.sp"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 12 "unit/export/basic.sp"
int test_export_int() {
    int x = 5, y = 10;

#line 14 "unit/export/basic.sp"
    swap__int(&x, &y);

#line 15 "unit/export/basic.sp"
    return x - 10 + y - 5;
}

#line 17 "unit/export/basic.sp"
int test_export_float() {
    float a = 1.0f, b = 2.0f;

#line 19 "unit/export/basic.sp"
    swap__float(&a, &b);

#line 20 "unit/export/basic.sp"
    return (int)(a + b - 3.0f);
}

#line 22 "unit/export/basic.sp"
int test_export_gfunc() {
    int x = 1, y = 2;

#line 24 "unit/export/basic.sp"
    swap__int(&x, &y);

#line 25 "unit/export/basic.sp"
    return x - 2 + y - 1;
}

#line 28 "unit/export/basic.sp"
int main(void) {

#line 29 "unit/export/basic.sp"
    int r1 = test_export_int();

#line 30 "unit/export/basic.sp"
    if (r1 != 0) 
        return r1;

#line 31 "unit/export/basic.sp"
    int r2 = test_export_float();

#line 32 "unit/export/basic.sp"
    if (r2 != 0) 
        return r2;

#line 33 "unit/export/basic.sp"
    int r3 = test_export_gfunc();

#line 34 "unit/export/basic.sp"
    if (r3 != 0) 
        return r3;

#line 35 "unit/export/basic.sp"
    return 0;
}

#line 4 "unit/export/basic.sp"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 5 "unit/export/basic.sp"
    int tmp = *a;

#line 6 "unit/export/basic.sp"
    *a = *b;

#line 7 "unit/export/basic.sp"
    *b = tmp;
}


#line 4 "unit/export/basic.sp"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 5 "unit/export/basic.sp"
    float tmp = *a;

#line 6 "unit/export/basic.sp"
    *a = *b;

#line 7 "unit/export/basic.sp"
    *b = tmp;
}

