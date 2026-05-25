

#line 9 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
typedef struct A A;
struct A {
    int val;
};

#line 10 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
typedef struct B B;
struct B {
    int val;
};

#line 12 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
int A__get_val(A * this);

#line 13 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
int B__get_val(B * this);

#line 14 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
void A__set_val(A * this, int v);

#line 15 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
void B__set_val(B * this, int v);

#line 12 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
int A__get_val(A * this) {
    return this->val + 100;
}

#line 13 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
int B__get_val(B * this) {
    return this->val + 200;
}

#line 14 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
void A__set_val(A * this, int v) {
    this->val = v;
}

#line 15 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
void B__set_val(B * this, int v) {
    this->val = v;
}

#line 17 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
int main() {

#line 18 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    A a = { 0 };

#line 19 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    B b = { 0 };

#line 21 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    A__set_val(&a, 1);

#line 22 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    B__set_val(&b, 2);

#line 24 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    if (A__get_val(&a) != 101) 
        return 1;

#line 25 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    if (B__get_val(&b) != 202) 
        return 2;

#line 27 "sharp-test/c_superset_probes/p241_ext_method_samename.sp"
    return 0;
}
