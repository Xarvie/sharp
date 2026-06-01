
#line 13 "sharp-test/unit/const/const_return.ce"
struct Config {
    int width;
    int height;
};

#line 18 "sharp-test/unit/const/const_return.ce"
int Config__area(struct Config * this);

#line 2 "sharp-test/unit/const/const_return.ce"
const int * get_const_ptr(const int * p) {

#line 3 "sharp-test/unit/const/const_return.ce"
    return p;
}

#line 6 "sharp-test/unit/const/const_return.ce"
int sum_const_arr(const int * arr, int len) {

#line 7 "sharp-test/unit/const/const_return.ce"
    int s = 0;

#line 8 "sharp-test/unit/const/const_return.ce"
    for (int i = 0; i < len; i = i + 1) 

#line 9 "sharp-test/unit/const/const_return.ce"
        s = s + arr[i];

#line 10 "sharp-test/unit/const/const_return.ce"
    return s;
}

#line 18 "sharp-test/unit/const/const_return.ce"
int Config__area(struct Config * this) {
    return this->width * this->height;
}

#line 20 "sharp-test/unit/const/const_return.ce"
int main() {

#line 22 "sharp-test/unit/const/const_return.ce"
    int vals[3] = { 10, 20, 30 };

#line 23 "sharp-test/unit/const/const_return.ce"
    if (sum_const_arr(vals, 3) != 60) 
        return 1;

#line 26 "sharp-test/unit/const/const_return.ce"
    int x = 42;

#line 27 "sharp-test/unit/const/const_return.ce"
    const int * cp = get_const_ptr(&x);

#line 28 "sharp-test/unit/const/const_return.ce"
    if (*cp != 42) 
        return 2;

#line 31 "sharp-test/unit/const/const_return.ce"
    struct Config c;

#line 32 "sharp-test/unit/const/const_return.ce"
    c.width = 6;
    c.height = 7;

#line 33 "sharp-test/unit/const/const_return.ce"
    if (Config__area(&c) != 42) 
        return 3;

#line 36 "sharp-test/unit/const/const_return.ce"
    const int * p = vals;

#line 37 "sharp-test/unit/const/const_return.ce"
    if (*p != 10) 
        return 4;

#line 38 "sharp-test/unit/const/const_return.ce"
    p = p + 1;

#line 39 "sharp-test/unit/const/const_return.ce"
    if (*p != 20) 
        return 5;

#line 40 "sharp-test/unit/const/const_return.ce"
    p = p + 1;

#line 41 "sharp-test/unit/const/const_return.ce"
    if (*p != 30) 
        return 6;

#line 44 "sharp-test/unit/const/const_return.ce"
    const struct Config * pc = &c;

#line 45 "sharp-test/unit/const/const_return.ce"
    if (Config__area(pc) != 42) 
        return 7;

#line 47 "sharp-test/unit/const/const_return.ce"
    return 0;
}
