

#line 9 "sharp-test/c_superset_probes/p239_method_chain.sp"
typedef struct Counter Counter;
struct Counter {
    int val;
};

#line 13 "sharp-test/c_superset_probes/p239_method_chain.sp"
Counter * Counter__inc(Counter * this);

#line 18 "sharp-test/c_superset_probes/p239_method_chain.sp"
Counter * Counter__add(Counter * this, int n);

#line 23 "sharp-test/c_superset_probes/p239_method_chain.sp"
int Counter__get(Counter * this);

#line 13 "sharp-test/c_superset_probes/p239_method_chain.sp"
Counter * Counter__inc(Counter * this) {

#line 14 "sharp-test/c_superset_probes/p239_method_chain.sp"
    this->val = this->val + 1;

#line 15 "sharp-test/c_superset_probes/p239_method_chain.sp"
    return this;
}

#line 18 "sharp-test/c_superset_probes/p239_method_chain.sp"
Counter * Counter__add(Counter * this, int n) {

#line 19 "sharp-test/c_superset_probes/p239_method_chain.sp"
    this->val = this->val + n;

#line 20 "sharp-test/c_superset_probes/p239_method_chain.sp"
    return this;
}

#line 23 "sharp-test/c_superset_probes/p239_method_chain.sp"
int Counter__get(Counter * this) {
    return this->val;
}

#line 25 "sharp-test/c_superset_probes/p239_method_chain.sp"
int main() {

#line 26 "sharp-test/c_superset_probes/p239_method_chain.sp"
    Counter c = { 0 };

#line 29 "sharp-test/c_superset_probes/p239_method_chain.sp"
    Counter__add(Counter__inc(Counter__inc(&c)), 5);

#line 31 "sharp-test/c_superset_probes/p239_method_chain.sp"
    if (Counter__get(&c) != 7) 
        return 1;

#line 34 "sharp-test/c_superset_probes/p239_method_chain.sp"
    Counter__inc(Counter__inc(Counter__inc(Counter__add(&c, 10))));

#line 35 "sharp-test/c_superset_probes/p239_method_chain.sp"
    if (Counter__get(&c) != 20) 
        return 2;

#line 37 "sharp-test/c_superset_probes/p239_method_chain.sp"
    return 0;
}
