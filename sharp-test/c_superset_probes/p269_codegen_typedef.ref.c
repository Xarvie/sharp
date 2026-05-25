

#line 6 "c_superset_probes/p269_codegen_typedef.sp"
typedef int MyInt;

#line 7 "c_superset_probes/p269_codegen_typedef.sp"
typedef MyInt YourInt;

#line 8 "c_superset_probes/p269_codegen_typedef.sp"
typedef YourInt OurInt;

#line 10 "c_superset_probes/p269_codegen_typedef.sp"
int get_max() {
    return 2147483647;
}

#line 12 "c_superset_probes/p269_codegen_typedef.sp"
int main() {

#line 13 "c_superset_probes/p269_codegen_typedef.sp"
    OurInt a = 100;

#line 14 "c_superset_probes/p269_codegen_typedef.sp"
    MyInt b = 200;

#line 15 "c_superset_probes/p269_codegen_typedef.sp"
    int c = a + b;

#line 16 "c_superset_probes/p269_codegen_typedef.sp"
    if (c != 300) 
        return 1;

#line 17 "c_superset_probes/p269_codegen_typedef.sp"
    if (get_max() != 2147483647) 
        return 2;

#line 18 "c_superset_probes/p269_codegen_typedef.sp"
    return 0;
}
