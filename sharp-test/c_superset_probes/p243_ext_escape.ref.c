

#line 8 "sharp-test/c_superset_probes/p243_ext_escape.sp"
typedef struct Counter Counter;
struct Counter {
    int val;
    int next;
};

#line 13 "sharp-test/c_superset_probes/p243_ext_escape.sp"
Counter * Counter__inc(Counter * this);

#line 18 "sharp-test/c_superset_probes/p243_ext_escape.sp"
Counter * Counter__inc_next(Counter * this);

#line 23 "sharp-test/c_superset_probes/p243_ext_escape.sp"
int Counter__sum(Counter * this);

#line 13 "sharp-test/c_superset_probes/p243_ext_escape.sp"
Counter * Counter__inc(Counter * this) {

#line 14 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    this->val = this->val + 1;

#line 15 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    return this;
}

#line 18 "sharp-test/c_superset_probes/p243_ext_escape.sp"
Counter * Counter__inc_next(Counter * this) {

#line 19 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    this->next = this->next + 1;

#line 20 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    return this;
}

#line 23 "sharp-test/c_superset_probes/p243_ext_escape.sp"
int Counter__sum(Counter * this) {
    return this->val + this->next;
}

#line 25 "sharp-test/c_superset_probes/p243_ext_escape.sp"
int main() {

#line 26 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    Counter c = { 0 };

#line 28 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    Counter * p = Counter__inc(&c);

#line 29 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    if (p->val != 1) 
        return 1;

#line 32 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    Counter__inc(Counter__inc_next(Counter__inc(&c)));

#line 33 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    if (c.val != 3) 
        return 2;

#line 34 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    if (c.next != 1) 
        return 3;

#line 37 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    if (p != &c) 
        return 4;

#line 38 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    if (Counter__sum(p) != 4) 
        return 5;

#line 40 "sharp-test/c_superset_probes/p243_ext_escape.sp"
    return 0;
}
