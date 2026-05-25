

#line 9 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
typedef struct Val Val;
struct Val {
    int x;
};

#line 14 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
int Val__double_val(Val * this);
int Val__double_val(Val * this) {
    return this->x * 2;
}

#line 17 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
int double_val(Val * v) {
    return v->x * 3;
}

#line 19 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
int main() {

#line 20 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
    Val v;
    v.x = 10;

#line 23 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
    if (Val__double_val(&v) != 20) 
        return 1;

#line 26 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
    int r = double_val(&v);

#line 27 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
    if (r != 30) 
        return 2;

#line 29 "sharp-test/c_superset_probes/p242_ext_vs_freefunc.sp"
    return 0;
}
