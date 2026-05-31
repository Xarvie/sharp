
#line 6 "sharp-test/unit/generics/function.ce"
int max__int(int a, int b);

#line 15 "sharp-test/unit/generics/function.ce"
void swap__int(int * a, int * b);
void swap__float(float * a, float * b);

#line 22 "sharp-test/unit/generics/function.ce"
int pass__int__double(int a, double b);

#line 27 "sharp-test/unit/generics/function.ce"
int accumulate__int(int * arr, int len);
float accumulate__float(float * arr, int len);

#line 36 "sharp-test/unit/generics/function.ce"
int abs_val__int(int v);

#line 42 "sharp-test/unit/generics/function.ce"
void fill_default__int(int * arr, int len, int val);

#line 6 "sharp-test/unit/generics/function.ce"

#line 15 "sharp-test/unit/generics/function.ce"

#line 22 "sharp-test/unit/generics/function.ce"

#line 27 "sharp-test/unit/generics/function.ce"

#line 36 "sharp-test/unit/generics/function.ce"

#line 42 "sharp-test/unit/generics/function.ce"

#line 48 "sharp-test/unit/generics/function.ce"
int main(void) {

#line 50 "sharp-test/unit/generics/function.ce"
    if (max__int(3, 5) != 5) 
        return 1;

#line 51 "sharp-test/unit/generics/function.ce"
    if (max__int(7, 2) != 7) 
        return 2;
    int a = 10, b = 20;

#line 55 "sharp-test/unit/generics/function.ce"
    swap__int(&a, &b);

#line 56 "sharp-test/unit/generics/function.ce"
    if (a != 20) 
        return 3;

#line 57 "sharp-test/unit/generics/function.ce"
    if (b != 10) 
        return 4;
    float fa = 1.0f, fb = 2.0f;

#line 61 "sharp-test/unit/generics/function.ce"
    swap__float(&fa, &fb);

#line 62 "sharp-test/unit/generics/function.ce"
    if (fa < 1.99f || fa > 2.01f) 
        return 5;

#line 63 "sharp-test/unit/generics/function.ce"
    if (fb < 0.99f || fb > 1.01f) 
        return 6;

#line 66 "sharp-test/unit/generics/function.ce"
    if (pass__int__double(1, 2.5) != 0) 
        return 7;

#line 69 "sharp-test/unit/generics/function.ce"
    int nums[4] = { 1, 2, 3, 4 };

#line 70 "sharp-test/unit/generics/function.ce"
    if (accumulate__int(nums, 4) != 10) 
        return 8;

#line 71 "sharp-test/unit/generics/function.ce"
    float fnums[3] = { 1.0f, 2.0f, 3.0f };

#line 72 "sharp-test/unit/generics/function.ce"
    if (accumulate__float(fnums, 3) < 5.99f || accumulate__float(fnums, 3) > 6.01f) 
        return 9;

#line 75 "sharp-test/unit/generics/function.ce"
    if (abs_val__int(-5) != 5) 
        return 15;

#line 76 "sharp-test/unit/generics/function.ce"
    if (abs_val__int(3) != 3) 
        return 16;

#line 77 "sharp-test/unit/generics/function.ce"
    if (abs_val__int(0) != 0) 
        return 17;

#line 80 "sharp-test/unit/generics/function.ce"
    int arr[5] = { 0, 0, 0, 0, 0 };

#line 81 "sharp-test/unit/generics/function.ce"
    fill_default__int(arr, 5, 7);

#line 82 "sharp-test/unit/generics/function.ce"
    if (arr[0] != 7 || arr[4] != 7) 
        return 18;

#line 84 "sharp-test/unit/generics/function.ce"
    return 0;
}

#line 6 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int max__int(int a, int b) {

#line 7 "sharp-test/unit/generics/function.ce"
    if (a > b) {

#line 8 "sharp-test/unit/generics/function.ce"
        return a;
    }
    else {

#line 10 "sharp-test/unit/generics/function.ce"
        return b;
    }
}


#line 15 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 16 "sharp-test/unit/generics/function.ce"
    int tmp = *a;

#line 17 "sharp-test/unit/generics/function.ce"
    *a = *b;

#line 18 "sharp-test/unit/generics/function.ce"
    *b = tmp;
}


#line 15 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) void swap__float(float * a, float * b) {

#line 16 "sharp-test/unit/generics/function.ce"
    float tmp = *a;

#line 17 "sharp-test/unit/generics/function.ce"
    *a = *b;

#line 18 "sharp-test/unit/generics/function.ce"
    *b = tmp;
}


#line 22 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int pass__int__double(int a, double b) {

#line 23 "sharp-test/unit/generics/function.ce"
    return 0;
}


#line 27 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int accumulate__int(int * arr, int len) {

#line 28 "sharp-test/unit/generics/function.ce"
    int sum = 0;

#line 29 "sharp-test/unit/generics/function.ce"
    for (int i = 0; i < len; i = i + 1) {

#line 30 "sharp-test/unit/generics/function.ce"
        sum = sum + arr[i];
    }

#line 32 "sharp-test/unit/generics/function.ce"
    return sum;
}


#line 27 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) float accumulate__float(float * arr, int len) {

#line 28 "sharp-test/unit/generics/function.ce"
    float sum = 0;

#line 29 "sharp-test/unit/generics/function.ce"
    for (int i = 0; i < len; i = i + 1) {

#line 30 "sharp-test/unit/generics/function.ce"
        sum = sum + arr[i];
    }

#line 32 "sharp-test/unit/generics/function.ce"
    return sum;
}


#line 36 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) int abs_val__int(int v) {

#line 37 "sharp-test/unit/generics/function.ce"
    if (v < 0) 
        return -v;

#line 38 "sharp-test/unit/generics/function.ce"
    return v;
}


#line 42 "sharp-test/unit/generics/function.ce"
__attribute__((weak)) void fill_default__int(int * arr, int len, int val) {

#line 43 "sharp-test/unit/generics/function.ce"
    for (int i = 0; i < len; i = i + 1) {

#line 44 "sharp-test/unit/generics/function.ce"
        arr[i] = val;
    }
}

