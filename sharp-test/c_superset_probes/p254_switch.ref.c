

#line 7 "c_superset_probes/p254_switch.sp"
int test_ret(int val) {

#line 8 "c_superset_probes/p254_switch.sp"
    switch (val) {

#line 9 "c_superset_probes/p254_switch.sp"
        case 1:
        return 1;

#line 10 "c_superset_probes/p254_switch.sp"
        case 2:
        return 2;

#line 11 "c_superset_probes/p254_switch.sp"
        default:
        return 42;
    }
}

#line 15 "c_superset_probes/p254_switch.sp"
int main() {

#line 17 "c_superset_probes/p254_switch.sp"
    int r1 = 0;

#line 18 "c_superset_probes/p254_switch.sp"
    int x = 2;

#line 19 "c_superset_probes/p254_switch.sp"
    switch (x) {

#line 20 "c_superset_probes/p254_switch.sp"
        case 1:
        r1 = 10;
        break;

#line 21 "c_superset_probes/p254_switch.sp"
        case 2:
        r1 = 20;
        break;

#line 22 "c_superset_probes/p254_switch.sp"
        case 3:
        r1 = 30;
        break;

#line 23 "c_superset_probes/p254_switch.sp"
        default:
        r1 = 0;
        break;
    }

#line 25 "c_superset_probes/p254_switch.sp"
    if (r1 != 20) 
        return 1;

#line 28 "c_superset_probes/p254_switch.sp"
    int r2 = 0;

#line 29 "c_superset_probes/p254_switch.sp"
    int y = 4;

#line 30 "c_superset_probes/p254_switch.sp"
    switch (y) {

#line 31 "c_superset_probes/p254_switch.sp"
        case 1:

#line 32 "c_superset_probes/p254_switch.sp"
        case 2:

#line 33 "c_superset_probes/p254_switch.sp"
        case 3:
        r2 = 3;
        break;

#line 34 "c_superset_probes/p254_switch.sp"
        case 4:

#line 35 "c_superset_probes/p254_switch.sp"
        case 5:
        r2 = 5;
        break;

#line 36 "c_superset_probes/p254_switch.sp"
        default:
        r2 = 0;
        break;
    }

#line 38 "c_superset_probes/p254_switch.sp"
    if (r2 != 5) 
        return 2;

#line 40 "c_superset_probes/p254_switch.sp"
    if (test_ret(1) != 1) 
        return 3;

#line 41 "c_superset_probes/p254_switch.sp"
    if (test_ret(2) != 2) 
        return 4;

#line 42 "c_superset_probes/p254_switch.sp"
    if (test_ret(99) != 42) 
        return 5;

#line 45 "c_superset_probes/p254_switch.sp"
    int r4 = 0;

#line 46 "c_superset_probes/p254_switch.sp"
    int z = 7;

#line 47 "c_superset_probes/p254_switch.sp"
    switch (z) {

#line 48 "c_superset_probes/p254_switch.sp"
        default:
        r4 = 99;
        break;
    }

#line 50 "c_superset_probes/p254_switch.sp"
    if (r4 != 99) 
        return 6;

#line 53 "c_superset_probes/p254_switch.sp"
    switch (x) {
    }

#line 55 "c_superset_probes/p254_switch.sp"
    return 0;
}
