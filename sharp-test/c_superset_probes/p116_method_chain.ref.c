

#line 2 "sharp-test/c_superset_probes/p116_method_chain.sp"
typedef struct Sb Sb;
struct Sb {
    int v;
};

#line 5 "sharp-test/c_superset_probes/p116_method_chain.sp"
Sb Sb__add(Sb * this, int x);

#line 6 "sharp-test/c_superset_probes/p116_method_chain.sp"
Sb Sb__mul(Sb * this, int x);

#line 7 "sharp-test/c_superset_probes/p116_method_chain.sp"
int Sb__val(Sb * this);

#line 5 "sharp-test/c_superset_probes/p116_method_chain.sp"
Sb Sb__add(Sb * this, int x) {
    this->v = this->v + x;
    return *this;
}

#line 6 "sharp-test/c_superset_probes/p116_method_chain.sp"
Sb Sb__mul(Sb * this, int x) {
    this->v = this->v * x;
    return *this;
}

#line 7 "sharp-test/c_superset_probes/p116_method_chain.sp"
int Sb__val(Sb * this) {
    return this->v;
}

#line 9 "sharp-test/c_superset_probes/p116_method_chain.sp"
int main(void) {

#line 10 "sharp-test/c_superset_probes/p116_method_chain.sp"
    Sb s;
    s.v = 1;

#line 11 "sharp-test/c_superset_probes/p116_method_chain.sp"
    if (Sb__val((__extension__(({ Sb __sharp_chain0 = (Sb__mul((__extension__(({ Sb __sharp_chain1 = (Sb__add(&s, 3)); &__sharp_chain1; }))), 2)); &__sharp_chain0; })))) != 8) 
        return 1;

#line 12 "sharp-test/c_superset_probes/p116_method_chain.sp"
    Sb t;
    t.v = 5;

#line 13 "sharp-test/c_superset_probes/p116_method_chain.sp"
    if (Sb__val((__extension__(({ Sb __sharp_chain2 = (Sb__add(&t, 1)); &__sharp_chain2; })))) != 6) 
        return 2;

#line 14 "sharp-test/c_superset_probes/p116_method_chain.sp"
    return 0;
}
