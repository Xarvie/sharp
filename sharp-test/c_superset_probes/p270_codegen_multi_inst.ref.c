

#line 3 "c_superset_probes/../../sharp/std/types.sph"
typedef long isize;

#line 4 "c_superset_probes/../../sharp/std/types.sph"
typedef unsigned long usize;

#line 9 "c_superset_probes/p270_codegen_multi_inst.sp"

#line 9 "c_superset_probes/p270_codegen_multi_inst.sp"
typedef struct Wrap__int Wrap__int;
struct Wrap__int {
    int value;
};


typedef struct Wrap__float Wrap__float;
struct Wrap__float {
    float value;
};



#line 13 "c_superset_probes/p270_codegen_multi_inst.sp"
void Wrap__int__set(Wrap__int * this, int v);

#line 14 "c_superset_probes/p270_codegen_multi_inst.sp"
int Wrap__int__get(Wrap__int * this);

#line 13 "c_superset_probes/p270_codegen_multi_inst.sp"
void Wrap__float__set(Wrap__float * this, float v);

#line 14 "c_superset_probes/p270_codegen_multi_inst.sp"
float Wrap__float__get(Wrap__float * this);

#line 16 "c_superset_probes/p270_codegen_multi_inst.sp"
int main() {

#line 17 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__int wi;

#line 18 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__int__set(&wi, 42);

#line 19 "c_superset_probes/p270_codegen_multi_inst.sp"
    if (Wrap__int__get(&wi) != 42) 
        return 1;

#line 21 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__float wf;

#line 22 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__float__set(&wf, 3.14f);

#line 23 "c_superset_probes/p270_codegen_multi_inst.sp"
    if (Wrap__float__get(&wf) != 3.14f) 
        return 2;

#line 26 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__int wi2;

#line 27 "c_superset_probes/p270_codegen_multi_inst.sp"
    Wrap__int__set(&wi2, 99);

#line 28 "c_superset_probes/p270_codegen_multi_inst.sp"
    if (Wrap__int__get(&wi2) != 99) 
        return 3;

#line 30 "c_superset_probes/p270_codegen_multi_inst.sp"
    return 0;
}

#line 13 "c_superset_probes/p270_codegen_multi_inst.sp"
__attribute__((weak)) void Wrap__int__set(Wrap__int * this, int v) {
    this->value = v;
}


#line 14 "c_superset_probes/p270_codegen_multi_inst.sp"
__attribute__((weak)) int Wrap__int__get(Wrap__int * this) {
    return this->value;
}


#line 13 "c_superset_probes/p270_codegen_multi_inst.sp"
__attribute__((weak)) void Wrap__float__set(Wrap__float * this, float v) {
    this->value = v;
}


#line 14 "c_superset_probes/p270_codegen_multi_inst.sp"
__attribute__((weak)) float Wrap__float__get(Wrap__float * this) {
    return this->value;
}

