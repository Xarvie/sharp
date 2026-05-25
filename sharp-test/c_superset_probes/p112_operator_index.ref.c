

#line 2 "sharp-test/c_superset_probes/p112_operator_index.sp"
typedef struct Arr Arr;
struct Arr {
    int data[4];
};

#line 5 "sharp-test/c_superset_probes/p112_operator_index.sp"
int Arr__op_idx(Arr * this, int i);
int Arr__op_idx(Arr * this, int i) {
    return this->data[i];
}

#line 7 "sharp-test/c_superset_probes/p112_operator_index.sp"
int main(void) {

#line 8 "sharp-test/c_superset_probes/p112_operator_index.sp"
    Arr a;

#line 9 "sharp-test/c_superset_probes/p112_operator_index.sp"
    a.data[0] = 10;
    a.data[1] = 20;
    a.data[2] = 30;
    a.data[3] = 40;

#line 10 "sharp-test/c_superset_probes/p112_operator_index.sp"
    if (Arr__op_idx(&a, 0) != 10) 
        return 1;

#line 11 "sharp-test/c_superset_probes/p112_operator_index.sp"
    if (Arr__op_idx(&a, 2) != 30) 
        return 2;

#line 12 "sharp-test/c_superset_probes/p112_operator_index.sp"
    return 0;
}
