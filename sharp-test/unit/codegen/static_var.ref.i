
#line 4 "sharp-test/unit/codegen/static_var.ce"
int g_counter = 0;

#line 6 "sharp-test/unit/codegen/static_var.ce"
int get_next() {

#line 7 "sharp-test/unit/codegen/static_var.ce"
    static int local = 100;

#line 8 "sharp-test/unit/codegen/static_var.ce"
    local = local + 1;

#line 9 "sharp-test/unit/codegen/static_var.ce"
    return local;
}

#line 12 "sharp-test/unit/codegen/static_var.ce"
void increment_global() {

#line 13 "sharp-test/unit/codegen/static_var.ce"
    g_counter = g_counter + 1;
}

#line 17 "sharp-test/unit/codegen/static_var.ce"
int fib_memo(int n) {

#line 18 "sharp-test/unit/codegen/static_var.ce"
    static int call_count = 0;

#line 19 "sharp-test/unit/codegen/static_var.ce"
    call_count = call_count + 1;

#line 20 "sharp-test/unit/codegen/static_var.ce"
    if (n <= 1) 
        return n;

#line 21 "sharp-test/unit/codegen/static_var.ce"
    return fib_memo(n - 1) + fib_memo(n - 2);
}

#line 25 "sharp-test/unit/codegen/static_var.ce"
int multi_static() {

#line 26 "sharp-test/unit/codegen/static_var.ce"
    static int a = 1;

#line 27 "sharp-test/unit/codegen/static_var.ce"
    static int b = 10;

#line 28 "sharp-test/unit/codegen/static_var.ce"
    static int c = 100;

#line 29 "sharp-test/unit/codegen/static_var.ce"
    a = a + 1;

#line 30 "sharp-test/unit/codegen/static_var.ce"
    b = b + 10;

#line 31 "sharp-test/unit/codegen/static_var.ce"
    c = c + 100;

#line 32 "sharp-test/unit/codegen/static_var.ce"
    return a + b + c;
}

#line 36 "sharp-test/unit/codegen/static_var.ce"
int * get_static_ptr() {

#line 37 "sharp-test/unit/codegen/static_var.ce"
    static int value = 0;

#line 38 "sharp-test/unit/codegen/static_var.ce"
    value = value + 10;

#line 39 "sharp-test/unit/codegen/static_var.ce"
    return &value;
}

#line 43 "sharp-test/unit/codegen/static_var.ce"
int use_const_static() {

#line 44 "sharp-test/unit/codegen/static_var.ce"
    static const int limit = 42;

#line 45 "sharp-test/unit/codegen/static_var.ce"
    return limit;
}

#line 48 "sharp-test/unit/codegen/static_var.ce"
int main() {

#line 50 "sharp-test/unit/codegen/static_var.ce"
    g_counter = 0;

#line 51 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 52 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 53 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 54 "sharp-test/unit/codegen/static_var.ce"
    if (g_counter != 3) 
        return 1;

#line 57 "sharp-test/unit/codegen/static_var.ce"
    int a = get_next();

#line 58 "sharp-test/unit/codegen/static_var.ce"
    int b = get_next();

#line 59 "sharp-test/unit/codegen/static_var.ce"
    int c = get_next();

#line 60 "sharp-test/unit/codegen/static_var.ce"
    if (a != 101) 
        return 2;

#line 61 "sharp-test/unit/codegen/static_var.ce"
    if (b != 102) 
        return 3;

#line 62 "sharp-test/unit/codegen/static_var.ce"
    if (c != 103) 
        return 4;

#line 65 "sharp-test/unit/codegen/static_var.ce"
    int r = fib_memo(5);

#line 66 "sharp-test/unit/codegen/static_var.ce"
    if (r != 5) 
        return 5;

#line 69 "sharp-test/unit/codegen/static_var.ce"
    int m1 = multi_static();

#line 70 "sharp-test/unit/codegen/static_var.ce"
    int m2 = multi_static();

#line 71 "sharp-test/unit/codegen/static_var.ce"
    int m3 = multi_static();

#line 72 "sharp-test/unit/codegen/static_var.ce"
    if (m1 != 222) 
        return 6;

#line 73 "sharp-test/unit/codegen/static_var.ce"
    if (m2 != 333) 
        return 7;

#line 74 "sharp-test/unit/codegen/static_var.ce"
    if (m3 != 444) 
        return 8;

#line 77 "sharp-test/unit/codegen/static_var.ce"
    int * p1 = get_static_ptr();

#line 78 "sharp-test/unit/codegen/static_var.ce"
    int * p2 = get_static_ptr();

#line 79 "sharp-test/unit/codegen/static_var.ce"
    if (*p1 != 20) 
        return 9;

#line 80 "sharp-test/unit/codegen/static_var.ce"
    if (*p2 != 20) 
        return 10;

#line 81 "sharp-test/unit/codegen/static_var.ce"
    if (p1 != p2) 
        return 11;

#line 84 "sharp-test/unit/codegen/static_var.ce"
    if (use_const_static() != 42) 
        return 12;

#line 85 "sharp-test/unit/codegen/static_var.ce"
    if (use_const_static() != 42) 
        return 13;

#line 87 "sharp-test/unit/codegen/static_var.ce"
    return 0;
}
