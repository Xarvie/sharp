
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

#line 16 "sharp-test/unit/codegen/static_var.ce"
int main() {

#line 18 "sharp-test/unit/codegen/static_var.ce"
    g_counter = 0;

#line 19 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 20 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 21 "sharp-test/unit/codegen/static_var.ce"
    increment_global();

#line 22 "sharp-test/unit/codegen/static_var.ce"
    if (g_counter != 3) 
        return 1;

#line 25 "sharp-test/unit/codegen/static_var.ce"
    int a = get_next();

#line 26 "sharp-test/unit/codegen/static_var.ce"
    int b = get_next();

#line 27 "sharp-test/unit/codegen/static_var.ce"
    int c = get_next();

#line 28 "sharp-test/unit/codegen/static_var.ce"
    if (a != 101) 
        return 2;

#line 29 "sharp-test/unit/codegen/static_var.ce"
    if (b != 102) 
        return 3;

#line 30 "sharp-test/unit/codegen/static_var.ce"
    if (c != 103) 
        return 4;

#line 32 "sharp-test/unit/codegen/static_var.ce"
    return 0;
}
