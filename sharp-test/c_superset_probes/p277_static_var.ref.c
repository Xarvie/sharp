

#line 6 "c_superset_probes/p277_static_var.sp"
int g_counter = 0;

#line 8 "c_superset_probes/p277_static_var.sp"
int get_next() {

#line 9 "c_superset_probes/p277_static_var.sp"
    static int local = 100;

#line 10 "c_superset_probes/p277_static_var.sp"
    local = local + 1;

#line 11 "c_superset_probes/p277_static_var.sp"
    return local;
}

#line 14 "c_superset_probes/p277_static_var.sp"
void increment_global() {

#line 15 "c_superset_probes/p277_static_var.sp"
    g_counter = g_counter + 1;
}

#line 18 "c_superset_probes/p277_static_var.sp"
int main() {

#line 20 "c_superset_probes/p277_static_var.sp"
    g_counter = 0;

#line 21 "c_superset_probes/p277_static_var.sp"
    increment_global();

#line 22 "c_superset_probes/p277_static_var.sp"
    increment_global();

#line 23 "c_superset_probes/p277_static_var.sp"
    increment_global();

#line 24 "c_superset_probes/p277_static_var.sp"
    if (g_counter != 3) 
        return 1;

#line 27 "c_superset_probes/p277_static_var.sp"
    int a = get_next();

#line 28 "c_superset_probes/p277_static_var.sp"
    int b = get_next();

#line 29 "c_superset_probes/p277_static_var.sp"
    int c = get_next();

#line 30 "c_superset_probes/p277_static_var.sp"
    if (a != 101) 
        return 2;

#line 31 "c_superset_probes/p277_static_var.sp"
    if (b != 102) 
        return 3;

#line 32 "c_superset_probes/p277_static_var.sp"
    if (c != 103) 
        return 4;

#line 34 "c_superset_probes/p277_static_var.sp"
    return 0;
}
