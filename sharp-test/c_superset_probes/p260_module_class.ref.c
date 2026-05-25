

#line 3 "c_superset_probes/p260_mymath.sph"
typedef struct Calculator Calculator;
struct Calculator {
    int value;
};

#line 7 "c_superset_probes/p260_mymath.sph"
void Calculator__reset(Calculator * this);

#line 8 "c_superset_probes/p260_mymath.sph"
void Calculator__set(Calculator * this, int v);

#line 9 "c_superset_probes/p260_mymath.sph"
int Calculator__get(Calculator * this);

#line 10 "c_superset_probes/p260_mymath.sph"
int Calculator__add(Calculator * this, int n);

#line 7 "c_superset_probes/p260_mymath.sph"
void Calculator__reset(Calculator * this) {
    this->value = 0;
}

#line 8 "c_superset_probes/p260_mymath.sph"
void Calculator__set(Calculator * this, int v) {
    this->value = v;
}

#line 9 "c_superset_probes/p260_mymath.sph"
int Calculator__get(Calculator * this) {
    return this->value;
}

#line 10 "c_superset_probes/p260_mymath.sph"
int Calculator__add(Calculator * this, int n) {
    this->value = this->value + n;
    return this->value;
}

#line 12 "c_superset_probes/p260_mymath.sph"
int square(int x) {
    return x * x;
}

#line 13 "c_superset_probes/p260_mymath.sph"
int cube(int x) {
    return x * x * x;
}

#line 9 "c_superset_probes/p260_module_class.sp"
int main() {

#line 11 "c_superset_probes/p260_module_class.sp"
    int s = square(7);

#line 12 "c_superset_probes/p260_module_class.sp"
    if (s != 49) 
        return 1;

#line 14 "c_superset_probes/p260_module_class.sp"
    int c = cube(3);

#line 15 "c_superset_probes/p260_module_class.sp"
    if (c != 27) 
        return 2;

#line 18 "c_superset_probes/p260_module_class.sp"
    Calculator calc;

#line 19 "c_superset_probes/p260_module_class.sp"
    Calculator__reset(&calc);

#line 20 "c_superset_probes/p260_module_class.sp"
    if (Calculator__get(&calc) != 0) 
        return 3;

#line 22 "c_superset_probes/p260_module_class.sp"
    Calculator__set(&calc, 10);

#line 23 "c_superset_probes/p260_module_class.sp"
    if (Calculator__get(&calc) != 10) 
        return 4;

#line 25 "c_superset_probes/p260_module_class.sp"
    int r = Calculator__add(&calc, 5);

#line 26 "c_superset_probes/p260_module_class.sp"
    if (r != 15) 
        return 5;

#line 27 "c_superset_probes/p260_module_class.sp"
    if (Calculator__get(&calc) != 15) 
        return 6;

#line 29 "c_superset_probes/p260_module_class.sp"
    return 0;
}
