
#line 3 "sharp-test/unit/const/const_cast.ce"
const int * get_const(int * p) {
    return p;
}

#line 5 "sharp-test/unit/const/const_cast.ce"
int main() {

#line 7 "sharp-test/unit/const/const_cast.ce"
    int val = 5;

#line 8 "sharp-test/unit/const/const_cast.ce"
    int * mutable_p = &val;

#line 9 "sharp-test/unit/const/const_cast.ce"
    const int * const_p = mutable_p;

#line 10 "sharp-test/unit/const/const_cast.ce"
    if (*const_p != 5) 
        return 1;

#line 13 "sharp-test/unit/const/const_cast.ce"
    int cval = 99;

#line 14 "sharp-test/unit/const/const_cast.ce"
    const void * cvp = &cval;

#line 15 "sharp-test/unit/const/const_cast.ce"
    const int * cip = (const int *)cvp;

#line 16 "sharp-test/unit/const/const_cast.ce"
    if (*cip != 99) 
        return 2;

#line 19 "sharp-test/unit/const/const_cast.ce"
    int a = 1;

#line 20 "sharp-test/unit/const/const_cast.ce"
    const int * p = a ? &a : (const int *)0;

#line 21 "sharp-test/unit/const/const_cast.ce"
    if (!p) 
        return 3;

#line 22 "sharp-test/unit/const/const_cast.ce"
    if (*p != 1) 
        return 4;

#line 25 "sharp-test/unit/const/const_cast.ce"
    int arr[3] = { 10, 20, 30 };

#line 26 "sharp-test/unit/const/const_cast.ce"
    const int * cp = arr;

#line 27 "sharp-test/unit/const/const_cast.ce"
    if (*cp != 10) 
        return 5;

#line 28 "sharp-test/unit/const/const_cast.ce"
    cp = cp + 1;

#line 29 "sharp-test/unit/const/const_cast.ce"
    if (*cp != 20) 
        return 6;

#line 32 "sharp-test/unit/const/const_cast.ce"
    int x = 42;

#line 33 "sharp-test/unit/const/const_cast.ce"
    const int * rp = get_const(&x);

#line 34 "sharp-test/unit/const/const_cast.ce"
    if (*rp != 42) 
        return 7;

#line 36 "sharp-test/unit/const/const_cast.ce"
    return 0;
}
