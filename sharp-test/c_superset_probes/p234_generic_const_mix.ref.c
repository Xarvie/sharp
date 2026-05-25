

#line 8 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
typedef struct Pair__int__float Pair__int__float;
struct Pair__int__float {
    int first;
    float second;
};



#line 14 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
int Pair__int__float__first_val(Pair__int__float * this);

#line 22 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
void Pair__int__float__set_first(Pair__int__float * this, int val);

#line 18 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
float Pair__int__float__second_val(Pair__int__float * this);

#line 27 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
int read_pair(const Pair__int__float * p) {

#line 28 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    return Pair__int__float__first_val(p);
}

#line 31 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
int main() {

#line 32 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    Pair__int__float p = { 0 };

#line 33 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    Pair__int__float__set_first(&p, 42);

#line 34 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    p.second = 3.14f;

#line 36 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    if (Pair__int__float__first_val(&p) != 42) 
        return 1;

#line 37 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    if (Pair__int__float__second_val(&p) < 3.13f || Pair__int__float__second_val(&p) > 3.15f) 
        return 2;

#line 40 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    int r = read_pair(&p);

#line 41 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    if (r != 42) 
        return 3;

#line 43 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    return 0;
}

#line 14 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
__attribute__((weak)) int Pair__int__float__first_val(Pair__int__float * this) {

#line 15 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    return this->first;
}


#line 22 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
__attribute__((weak)) void Pair__int__float__set_first(Pair__int__float * this, int val) {

#line 23 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    this->first = val;
}


#line 18 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
__attribute__((weak)) float Pair__int__float__second_val(Pair__int__float * this) {

#line 19 "sharp-test/c_superset_probes/p234_generic_const_mix.sp"
    return this->second;
}

