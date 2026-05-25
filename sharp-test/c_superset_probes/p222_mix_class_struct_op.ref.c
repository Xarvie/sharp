
#line 11 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
typedef struct VecC VecC;
struct VecC {
    float x;
    float y;
};

#line 16 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
struct VecS {
    float x;
    float y;
};

#line 19 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
VecC operator_add__VecC__VecS(VecC a, struct VecS b) {

#line 20 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC r;

#line 21 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    r.x = a.x + b.x;

#line 22 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    r.y = a.y + b.y;

#line 23 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    return r;
}

#line 27 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
VecC operator_add__VecS__VecC(struct VecS a, VecC b) {

#line 28 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC r;

#line 29 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    r.x = a.x + b.x;

#line 30 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    r.y = a.y + b.y;

#line 31 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    return r;
}

#line 35 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
int operator_eq__VecC__VecS(VecC a, struct VecS b) {

#line 36 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    return a.x == b.x && a.y == b.y;
}

#line 39 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
int main(void) {

#line 40 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC c1;
    c1.x = 1.0f;
    c1.y = 2.0f;

#line 42 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    struct VecS s1;
    s1.x = 10.0f;
    s1.y = 20.0f;

#line 43 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    struct VecS s2;
    s2.x = 30.0f;
    s2.y = 40.0f;

#line 46 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC cs = operator_add__VecC__VecS(c1, s1);

#line 47 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (cs.x != 11.0f) 
        return 1;

#line 48 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (cs.y != 22.0f) 
        return 2;

#line 51 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC sc = operator_add__VecS__VecC(s1, c1);

#line 52 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (sc.x != 11.0f) 
        return 3;

#line 53 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (sc.y != 22.0f) 
        return 4;

#line 56 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    VecC eq_test;
    eq_test.x = 10.0f;
    eq_test.y = 20.0f;

#line 57 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (!(operator_eq__VecC__VecS(eq_test, s1))) 
        return 5;

#line 58 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    if (operator_eq__VecC__VecS(eq_test, s2)) 
        return 6;

#line 60 "sharp-test/c_superset_probes/p222_mix_class_struct_op.sp"
    return 0;
}
