
#line 2 "sharp-test/unit/generics/generic_func_advanced.ce"
int min_val__int(int a, int b);

#line 6 "sharp-test/unit/generics/generic_func_advanced.ce"
int max_val__int(int a, int b);

#line 10 "sharp-test/unit/generics/generic_func_advanced.ce"
void swap_vals__int(int * a, int * b);

#line 16 "sharp-test/unit/generics/generic_func_advanced.ce"
int clamp_val__int(int val, int lo, int hi);

#line 2 "sharp-test/unit/generics/generic_func_advanced.ce"

#line 6 "sharp-test/unit/generics/generic_func_advanced.ce"

#line 10 "sharp-test/unit/generics/generic_func_advanced.ce"

#line 16 "sharp-test/unit/generics/generic_func_advanced.ce"

#line 22 "sharp-test/unit/generics/generic_func_advanced.ce"
int main() {

#line 24 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (min_val__int(3, 7) != 3) 
        return 1;

#line 25 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (max_val__int(3, 7) != 7) 
        return 2;
    int a = 10, b = 20;

#line 29 "sharp-test/unit/generics/generic_func_advanced.ce"
    swap_vals__int(&a, &b);

#line 30 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (a != 20) 
        return 3;

#line 31 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (b != 10) 
        return 4;

#line 34 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (clamp_val__int(5, 0, 10) != 5) 
        return 5;

#line 35 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (clamp_val__int(-3, 0, 10) != 0) 
        return 6;

#line 36 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (clamp_val__int(15, 0, 10) != 10) 
        return 7;
    int x = 15, y = 3;

#line 40 "sharp-test/unit/generics/generic_func_advanced.ce"
    int lo = min_val__int(x, y);

#line 41 "sharp-test/unit/generics/generic_func_advanced.ce"
    int hi = max_val__int(x, y);

#line 42 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (lo != 3) 
        return 8;

#line 43 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (hi != 15) 
        return 9;

#line 44 "sharp-test/unit/generics/generic_func_advanced.ce"
    int clamped = clamp_val__int(7, lo, hi);

#line 45 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (clamped != 7) 
        return 10;

#line 47 "sharp-test/unit/generics/generic_func_advanced.ce"
    return 0;
}

#line 2 "sharp-test/unit/generics/generic_func_advanced.ce"
__attribute__((weak)) int min_val__int(int a, int b) {

#line 3 "sharp-test/unit/generics/generic_func_advanced.ce"
    return a < b ? a : b;
}


#line 6 "sharp-test/unit/generics/generic_func_advanced.ce"
__attribute__((weak)) int max_val__int(int a, int b) {

#line 7 "sharp-test/unit/generics/generic_func_advanced.ce"
    return a > b ? a : b;
}


#line 10 "sharp-test/unit/generics/generic_func_advanced.ce"
__attribute__((weak)) void swap_vals__int(int * a, int * b) {

#line 11 "sharp-test/unit/generics/generic_func_advanced.ce"
    int tmp = *a;

#line 12 "sharp-test/unit/generics/generic_func_advanced.ce"
    *a = *b;

#line 13 "sharp-test/unit/generics/generic_func_advanced.ce"
    *b = tmp;
}


#line 16 "sharp-test/unit/generics/generic_func_advanced.ce"
__attribute__((weak)) int clamp_val__int(int val, int lo, int hi) {

#line 17 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (val < lo) 
        return lo;

#line 18 "sharp-test/unit/generics/generic_func_advanced.ce"
    if (val > hi) 
        return hi;

#line 19 "sharp-test/unit/generics/generic_func_advanced.ce"
    return val;
}

