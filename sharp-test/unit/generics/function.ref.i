
#line 5 "sharp-test/unit/generics/function.ce"
int max__int(int a, int b);

#line 14 "sharp-test/unit/generics/function.ce"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 21 "sharp-test/unit/generics/function.ce"
int pass__int__double(int a, double b);

#line 5 "sharp-test/unit/generics/function.ce"

#line 14 "sharp-test/unit/generics/function.ce"

#line 21 "sharp-test/unit/generics/function.ce"

#line 25 "sharp-test/unit/generics/function.ce"
int main(void) {

#line 27 "sharp-test/unit/generics/function.ce"
    if (max__int(3, 5) != 5) 
        return 1;

#line 28 "sharp-test/unit/generics/function.ce"
    if (max__int(7, 2) != 7) 
        return 2;
    int a = 10, b = 20;

#line 32 "sharp-test/unit/generics/function.ce"
    swap__int(&a, &b);

#line 33 "sharp-test/unit/generics/function.ce"
    if (a != 20) 
        return 3;

#line 34 "sharp-test/unit/generics/function.ce"
    if (b != 10) 
        return 4;
    float fa = 1.0f, fb = 2.0f;

#line 38 "sharp-test/unit/generics/function.ce"
    swap__float(&fa, &fb);

#line 39 "sharp-test/unit/generics/function.ce"
    if (fa < 1.99f || fa > 2.01f) 
        return 5;

#line 40 "sharp-test/unit/generics/function.ce"
    if (fb < 0.99f || fb > 1.01f) 
        return 6;

#line 43 "sharp-test/unit/generics/function.ce"
    if (pass__int__double(1, 2.5) != 0) 
        return 7;

#line 45 "sharp-test/unit/generics/function.ce"
    return 0;
}

#line 5 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int max__int(int a, int b) {

#line 6 "sharp-test/unit/generics/function.ce"
    if (a > b) {

#line 7 "sharp-test/unit/generics/function.ce"
        return a;
    }
    else {

#line 9 "sharp-test/unit/generics/function.ce"
        return b;
    }
}


#line 14 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 15 "sharp-test/unit/generics/function.ce"
    int tmp = *a;

#line 16 "sharp-test/unit/generics/function.ce"
    *a = *b;

#line 17 "sharp-test/unit/generics/function.ce"
    *b = tmp;
}


#line 14 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 15 "sharp-test/unit/generics/function.ce"
    float tmp = *a;

#line 16 "sharp-test/unit/generics/function.ce"
    *a = *b;

#line 17 "sharp-test/unit/generics/function.ce"
    *b = tmp;
}


#line 21 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int pass__int__double(int a, double b) {

#line 22 "sharp-test/unit/generics/function.ce"
    return 0;
}

