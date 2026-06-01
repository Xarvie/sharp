
#line 2 "sharp-test/unit/operators/assign.ce"
typedef struct Counter Counter;
struct Counter {
    int n;
};

#line 6 "sharp-test/unit/operators/assign.ce"
Counter Counter__op_add(Counter * this, int x);

#line 10 "sharp-test/unit/operators/assign.ce"
Counter Counter__op_sub(Counter * this, int x);

#line 6 "sharp-test/unit/operators/assign.ce"
Counter Counter__op_add(Counter * this, int x) {

#line 7 "sharp-test/unit/operators/assign.ce"
    Counter r;
    r.n = this->n + x;
    return r;
}

#line 10 "sharp-test/unit/operators/assign.ce"
Counter Counter__op_sub(Counter * this, int x) {

#line 11 "sharp-test/unit/operators/assign.ce"
    Counter r;
    r.n = this->n - x;
    return r;
}

#line 14 "sharp-test/unit/operators/assign.ce"
int main() {

#line 16 "sharp-test/unit/operators/assign.ce"
    Counter c;
    c.n = 0;

#line 17 "sharp-test/unit/operators/assign.ce"
    Counter d;
    d.n = 5;

#line 18 "sharp-test/unit/operators/assign.ce"
    c = d;

#line 19 "sharp-test/unit/operators/assign.ce"
    if (c.n != 5) 
        return 1;

#line 22 "sharp-test/unit/operators/assign.ce"
    Counter e;
    e.n = 10;

#line 23 "sharp-test/unit/operators/assign.ce"
    Counter f = Counter__op_add(&e, 5);

#line 24 "sharp-test/unit/operators/assign.ce"
    if (f.n != 15) 
        return 2;

#line 25 "sharp-test/unit/operators/assign.ce"
    if (e.n != 10) 
        return 3;

#line 28 "sharp-test/unit/operators/assign.ce"
    Counter g;
    g.n = 20;

#line 29 "sharp-test/unit/operators/assign.ce"
    Counter h = Counter__op_sub(&g, 8);

#line 30 "sharp-test/unit/operators/assign.ce"
    if (h.n != 12) 
        return 4;

#line 33 "sharp-test/unit/operators/assign.ce"
    Counter base;
    base.n = 100;

#line 34 "sharp-test/unit/operators/assign.ce"
    Counter r1 = Counter__op_add(&base, 10);

#line 35 "sharp-test/unit/operators/assign.ce"
    Counter r2 = Counter__op_sub(&r1, 5);

#line 36 "sharp-test/unit/operators/assign.ce"
    if (r2.n != 105) 
        return 5;

#line 38 "sharp-test/unit/operators/assign.ce"
    return 0;
}
