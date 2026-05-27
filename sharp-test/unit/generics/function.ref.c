
#include <stdlib.h>

#line 5 "unit/generics/function.sp"
int max__int(int a, int b);

#line 14 "unit/generics/function.sp"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 21 "unit/generics/function.sp"
int pass__int__double(int a, double b);

#line 5 "unit/generics/function.sp"

#line 14 "unit/generics/function.sp"

#line 21 "unit/generics/function.sp"

#line 25 "unit/generics/function.sp"
int main(void) {

#line 27 "unit/generics/function.sp"
    if (max__int(3, 5) != 5) 
        return 1;

#line 28 "unit/generics/function.sp"
    if (max__int(7, 2) != 7) 
        return 2;
    int a = 10, b = 20;

#line 32 "unit/generics/function.sp"
    swap__int(&a, &b);

#line 33 "unit/generics/function.sp"
    if (a != 20) 
        return 3;

#line 34 "unit/generics/function.sp"
    if (b != 10) 
        return 4;
    float fa = 1.0f, fb = 2.0f;

#line 38 "unit/generics/function.sp"
    swap__float(&fa, &fb);

#line 39 "unit/generics/function.sp"
    if (fa < 1.99f || fa > 2.01f) 
        return 5;

#line 40 "unit/generics/function.sp"
    if (fb < 0.99f || fb > 1.01f) 
        return 6;

#line 43 "unit/generics/function.sp"
    if (pass__int__double(1, 2.5) != 0) 
        return 7;

#line 45 "unit/generics/function.sp"
    return 0;
}

#line 5 "unit/generics/function.sp"
__attribute__((weak)) int max__int(int a, int b) {

#line 6 "unit/generics/function.sp"
    if (a > b) {

#line 7 "unit/generics/function.sp"
        return a;
    }
    else {

#line 9 "unit/generics/function.sp"
        return b;
    }
}


#line 14 "unit/generics/function.sp"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 15 "unit/generics/function.sp"
    int tmp = *a;

#line 16 "unit/generics/function.sp"
    *a = *b;

#line 17 "unit/generics/function.sp"
    *b = tmp;
}


#line 14 "unit/generics/function.sp"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 15 "unit/generics/function.sp"
    float tmp = *a;

#line 16 "unit/generics/function.sp"
    *a = *b;

#line 17 "unit/generics/function.sp"
    *b = tmp;
}


#line 21 "unit/generics/function.sp"
__attribute__((weak)) int pass__int__double(int a, double b) {

#line 22 "unit/generics/function.sp"
    return 0;
}

