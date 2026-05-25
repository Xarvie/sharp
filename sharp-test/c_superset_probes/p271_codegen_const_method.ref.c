

#line 6 "c_superset_probes/p271_codegen_const_method.sp"
typedef struct Data Data;
struct Data {
    int secret;
};

#line 10 "c_superset_probes/p271_codegen_const_method.sp"
int Data__get_val(Data * this);

#line 11 "c_superset_probes/p271_codegen_const_method.sp"
void Data__set_val(Data * this, int v);

#line 10 "c_superset_probes/p271_codegen_const_method.sp"
int Data__get_val(Data * this) {
    return this->secret;
}

#line 11 "c_superset_probes/p271_codegen_const_method.sp"
void Data__set_val(Data * this, int v) {
    this->secret = v;
}

#line 14 "c_superset_probes/p271_codegen_const_method.sp"
int read_val(const Data * d) {

#line 15 "c_superset_probes/p271_codegen_const_method.sp"
    return Data__get_val(d);
}

#line 18 "c_superset_probes/p271_codegen_const_method.sp"
int main() {

#line 19 "c_superset_probes/p271_codegen_const_method.sp"
    Data d;

#line 20 "c_superset_probes/p271_codegen_const_method.sp"
    Data__set_val(&d, 128);

#line 21 "c_superset_probes/p271_codegen_const_method.sp"
    if (Data__get_val(&d) != 128) 
        return 1;

#line 23 "c_superset_probes/p271_codegen_const_method.sp"
    int v = read_val(&d);

#line 24 "c_superset_probes/p271_codegen_const_method.sp"
    if (v != 128) 
        return 2;

#line 26 "c_superset_probes/p271_codegen_const_method.sp"
    return 0;
}
