
#line 2 "sharp-test/unit/cinterop/callback.ce"
typedef int (*Comparator)(const void *, const void *);

#line 4 "sharp-test/unit/cinterop/callback.ce"
int int_compare(const void * a, const void * b) {

#line 5 "sharp-test/unit/cinterop/callback.ce"
    int ia = *(const int *)a;

#line 6 "sharp-test/unit/cinterop/callback.ce"
    int ib = *(const int *)b;

#line 7 "sharp-test/unit/cinterop/callback.ce"
    if (ia < ib) 
        return -1;

#line 8 "sharp-test/unit/cinterop/callback.ce"
    if (ia > ib) 
        return 1;

#line 9 "sharp-test/unit/cinterop/callback.ce"
    return 0;
}

#line 12 "sharp-test/unit/cinterop/callback.ce"
int apply_twice(int (*fn)(int), int x) {

#line 13 "sharp-test/unit/cinterop/callback.ce"
    return fn(fn(x));
}

#line 16 "sharp-test/unit/cinterop/callback.ce"
int square(int x) {
    return x * x;
}

#line 18 "sharp-test/unit/cinterop/callback.ce"
int negate(int x) {
    return -x;
}

#line 20 "sharp-test/unit/cinterop/callback.ce"
void foreach_int(int * arr, int len, void (*fn)(int *)) {

#line 21 "sharp-test/unit/cinterop/callback.ce"
    for (int i = 0; i < len; i = i + 1) 

#line 22 "sharp-test/unit/cinterop/callback.ce"
        fn(&arr[i]);
}

#line 25 "sharp-test/unit/cinterop/callback.ce"
void add_one(int * p) {
    *p = *p + 1;
}

#line 27 "sharp-test/unit/cinterop/callback.ce"
int main() {

#line 29 "sharp-test/unit/cinterop/callback.ce"
    int vals[5] = { 5, 3, 1, 4, 2 };

#line 30 "sharp-test/unit/cinterop/callback.ce"
    int min_val = vals[0];

#line 31 "sharp-test/unit/cinterop/callback.ce"
    for (int i = 1; i < 5; i = i + 1) {

#line 32 "sharp-test/unit/cinterop/callback.ce"
        if (int_compare(&vals[i], &min_val) < 0) 

#line 33 "sharp-test/unit/cinterop/callback.ce"
            min_val = vals[i];
    }

#line 35 "sharp-test/unit/cinterop/callback.ce"
    if (min_val != 1) 
        return 1;

#line 38 "sharp-test/unit/cinterop/callback.ce"
    if (apply_twice(square, 2) != 16) 
        return 2;

#line 39 "sharp-test/unit/cinterop/callback.ce"
    if (apply_twice(negate, 5) != 5) 
        return 3;

#line 42 "sharp-test/unit/cinterop/callback.ce"
    int arr[3] = { 10, 20, 30 };

#line 43 "sharp-test/unit/cinterop/callback.ce"
    foreach_int(arr, 3, add_one);

#line 44 "sharp-test/unit/cinterop/callback.ce"
    if (arr[0] != 11) 
        return 4;

#line 45 "sharp-test/unit/cinterop/callback.ce"
    if (arr[1] != 21) 
        return 5;

#line 46 "sharp-test/unit/cinterop/callback.ce"
    if (arr[2] != 31) 
        return 6;

#line 49 "sharp-test/unit/cinterop/callback.ce"
    int (* ops[2])(int) = { square, negate };

#line 50 "sharp-test/unit/cinterop/callback.ce"
    if (ops[0](3) != 9) 
        return 7;

#line 51 "sharp-test/unit/cinterop/callback.ce"
    if (ops[1](7) != -7) 
        return 8;

#line 53 "sharp-test/unit/cinterop/callback.ce"
    return 0;
}
