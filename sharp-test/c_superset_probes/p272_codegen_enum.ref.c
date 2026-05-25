

#line 6 "c_superset_probes/p272_codegen_enum.sp"
enum Status {
    OK,
    WARN,
    ERR,
    FATAL = 99
};

#line 13 "c_superset_probes/p272_codegen_enum.sp"
int get_status_val(enum Status s) {

#line 14 "c_superset_probes/p272_codegen_enum.sp"
    if (s == OK) 
        return 0;

#line 15 "c_superset_probes/p272_codegen_enum.sp"
    if (s == WARN) 
        return 1;

#line 16 "c_superset_probes/p272_codegen_enum.sp"
    if (s == ERR) 
        return 2;

#line 17 "c_superset_probes/p272_codegen_enum.sp"
    if (s == FATAL) 
        return 99;

#line 18 "c_superset_probes/p272_codegen_enum.sp"
    return -1;
}

#line 21 "c_superset_probes/p272_codegen_enum.sp"
int main() {

#line 22 "c_superset_probes/p272_codegen_enum.sp"
    if (get_status_val(OK) != 0) 
        return 1;

#line 23 "c_superset_probes/p272_codegen_enum.sp"
    if (get_status_val(WARN) != 1) 
        return 2;

#line 24 "c_superset_probes/p272_codegen_enum.sp"
    if (get_status_val(ERR) != 2) 
        return 3;

#line 25 "c_superset_probes/p272_codegen_enum.sp"
    if (get_status_val(FATAL) != 99) 
        return 4;

#line 27 "c_superset_probes/p272_codegen_enum.sp"
    enum Status s = WARN;

#line 28 "c_superset_probes/p272_codegen_enum.sp"
    if (s != WARN) 
        return 5;

#line 30 "c_superset_probes/p272_codegen_enum.sp"
    return 0;
}
