

#line 6 "c_superset_probes/p259_goto.sp"
int main() {

#line 8 "c_superset_probes/p259_goto.sp"
    int x = 0;

#line 9 "c_superset_probes/p259_goto.sp"
    goto L_set;

#line 10 "c_superset_probes/p259_goto.sp"
    x = 99;

#line 11 "c_superset_probes/p259_goto.sp"
L_set:

#line 12 "c_superset_probes/p259_goto.sp"
    x = 42;

#line 13 "c_superset_probes/p259_goto.sp"
    if (x != 42) 
        return 1;

#line 16 "c_superset_probes/p259_goto.sp"
    int count = 0;

#line 17 "c_superset_probes/p259_goto.sp"
    int i = 0;

#line 18 "c_superset_probes/p259_goto.sp"
L_loop:

#line 19 "c_superset_probes/p259_goto.sp"
    if (i >= 5) 
        goto L_done;

#line 20 "c_superset_probes/p259_goto.sp"
    count = count + i;

#line 21 "c_superset_probes/p259_goto.sp"
    i = i + 1;

#line 22 "c_superset_probes/p259_goto.sp"
    goto L_loop;

#line 23 "c_superset_probes/p259_goto.sp"
L_done:

#line 24 "c_superset_probes/p259_goto.sp"
    if (count != 10) 
        return 2;

#line 27 "c_superset_probes/p259_goto.sp"
    int y = 5;

#line 28 "c_superset_probes/p259_goto.sp"
    goto L_skip_decl;

#line 29 "c_superset_probes/p259_goto.sp"
{

#line 30 "c_superset_probes/p259_goto.sp"
        int local = 99;

#line 31 "c_superset_probes/p259_goto.sp"
        (void)local;
    }

#line 33 "c_superset_probes/p259_goto.sp"
L_skip_decl:

#line 34 "c_superset_probes/p259_goto.sp"
    if (y != 5) 
        return 3;

#line 37 "c_superset_probes/p259_goto.sp"
    int val = 0;

#line 38 "c_superset_probes/p259_goto.sp"
    if (val == 0) 
        goto L_A;

#line 39 "c_superset_probes/p259_goto.sp"
    goto L_B;

#line 40 "c_superset_probes/p259_goto.sp"
L_A:

#line 41 "c_superset_probes/p259_goto.sp"
    val = 1;

#line 42 "c_superset_probes/p259_goto.sp"
    goto L_C;

#line 43 "c_superset_probes/p259_goto.sp"
L_B:

#line 44 "c_superset_probes/p259_goto.sp"
    val = 2;

#line 45 "c_superset_probes/p259_goto.sp"
    goto L_C;

#line 46 "c_superset_probes/p259_goto.sp"
L_C:

#line 47 "c_superset_probes/p259_goto.sp"
    if (val != 1) 
        return 4;

#line 49 "c_superset_probes/p259_goto.sp"
    return 0;
}
