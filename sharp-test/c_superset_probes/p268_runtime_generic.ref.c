

#line 3 "c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 8 "c_superset_probes/p268_runtime_generic.sp"

#line 8 "c_superset_probes/p268_runtime_generic.sp"
typedef struct Pair__int Pair__int;
struct Pair__int {
    int first;
    int second;
};


typedef struct Pair__float Pair__float;
struct Pair__float {
    float first;
    float second;
};



#line 13 "c_superset_probes/p268_runtime_generic.sp"
void Pair__int__swap(Pair__int * this);
void Pair__float__swap(Pair__float * this);

#line 19 "c_superset_probes/p268_runtime_generic.sp"
int main() {

#line 20 "c_superset_probes/p268_runtime_generic.sp"
    Pair__int p;

#line 21 "c_superset_probes/p268_runtime_generic.sp"
    p.first = 10;

#line 22 "c_superset_probes/p268_runtime_generic.sp"
    p.second = 20;

#line 24 "c_superset_probes/p268_runtime_generic.sp"
    if (p.first != 10) 
        return 1;

#line 25 "c_superset_probes/p268_runtime_generic.sp"
    if (p.second != 20) 
        return 2;

#line 27 "c_superset_probes/p268_runtime_generic.sp"
    Pair__int__swap(&p);

#line 29 "c_superset_probes/p268_runtime_generic.sp"
    if (p.first != 20) 
        return 3;

#line 30 "c_superset_probes/p268_runtime_generic.sp"
    if (p.second != 10) 
        return 4;

#line 33 "c_superset_probes/p268_runtime_generic.sp"
    Pair__float pf;

#line 34 "c_superset_probes/p268_runtime_generic.sp"
    pf.first = 1.5f;

#line 35 "c_superset_probes/p268_runtime_generic.sp"
    pf.second = 2.5f;

#line 37 "c_superset_probes/p268_runtime_generic.sp"
    if (pf.first != 1.5f) 
        return 5;

#line 38 "c_superset_probes/p268_runtime_generic.sp"
    if (pf.second != 2.5f) 
        return 6;

#line 40 "c_superset_probes/p268_runtime_generic.sp"
    Pair__float__swap(&pf);

#line 42 "c_superset_probes/p268_runtime_generic.sp"
    if (pf.first != 2.5f) 
        return 7;

#line 43 "c_superset_probes/p268_runtime_generic.sp"
    if (pf.second != 1.5f) 
        return 8;

#line 45 "c_superset_probes/p268_runtime_generic.sp"
    return 0;
}

#line 13 "c_superset_probes/p268_runtime_generic.sp"
__attribute__((weak)) void Pair__int__swap(Pair__int * this) {

#line 14 "c_superset_probes/p268_runtime_generic.sp"
    int tmp = this->first;

#line 15 "c_superset_probes/p268_runtime_generic.sp"
    this->first = this->second;

#line 16 "c_superset_probes/p268_runtime_generic.sp"
    this->second = tmp;
}


#line 13 "c_superset_probes/p268_runtime_generic.sp"
__attribute__((weak)) void Pair__float__swap(Pair__float * this) {

#line 14 "c_superset_probes/p268_runtime_generic.sp"
    float tmp = this->first;

#line 15 "c_superset_probes/p268_runtime_generic.sp"
    this->first = this->second;

#line 16 "c_superset_probes/p268_runtime_generic.sp"
    this->second = tmp;
}

