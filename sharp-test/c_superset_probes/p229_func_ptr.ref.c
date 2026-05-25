

#line 8 "sharp-test/c_superset_probes/p229_func_ptr.sp"
typedef struct Button Button;
struct Button {
    int id;
    void (*on_click)(int id);
};

#line 13 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void Button__click(Button * this);

#line 17 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void Button__set_callback(Button * this, void (*cb)(int));

#line 22 "sharp-test/c_superset_probes/p229_func_ptr.sp"
typedef int (*IntUnary)(int);

#line 13 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void Button__click(Button * this) {

#line 14 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    if (this->on_click) 
        this->on_click(this->id);
}

#line 17 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void Button__set_callback(Button * this, void (*cb)(int)) {

#line 18 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    this->on_click = cb;
}

#line 23 "sharp-test/c_superset_probes/p229_func_ptr.sp"
IntUnary make_doubler() {

#line 25 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    return 0;
}

#line 29 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void cb_a(int id) {
}

#line 30 "sharp-test/c_superset_probes/p229_func_ptr.sp"
void cb_b(int id) {
}

#line 32 "sharp-test/c_superset_probes/p229_func_ptr.sp"
int main() {

#line 33 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button b;

#line 34 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    b.id = 1;

#line 35 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button__set_callback(&b, cb_a);

#line 37 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button__click(&b);

#line 39 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    b.id = 2;

#line 40 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button__set_callback(&b, cb_b);

#line 41 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button__click(&b);

#line 44 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button b2 = { 0 };

#line 45 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    Button__click(&b2);

#line 47 "sharp-test/c_superset_probes/p229_func_ptr.sp"
    return 0;
}
