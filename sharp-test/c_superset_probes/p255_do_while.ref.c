

#line 6 "c_superset_probes/p255_do_while.sp"
int main() {

#line 8 "c_superset_probes/p255_do_while.sp"
    int sum = 0;

#line 9 "c_superset_probes/p255_do_while.sp"
    int i = 0;

#line 10 "c_superset_probes/p255_do_while.sp"
    do {

#line 11 "c_superset_probes/p255_do_while.sp"
        sum = sum + i;

#line 12 "c_superset_probes/p255_do_while.sp"
        i = i + 1;
    }
    while (i < 5);

#line 14 "c_superset_probes/p255_do_while.sp"
    if (sum != 10) 
        return 1;

#line 17 "c_superset_probes/p255_do_while.sp"
    int flag = 0;

#line 18 "c_superset_probes/p255_do_while.sp"
    do {

#line 19 "c_superset_probes/p255_do_while.sp"
        flag = 1;
    }
    while (0);

#line 21 "c_superset_probes/p255_do_while.sp"
    if (flag != 1) 
        return 2;

#line 24 "c_superset_probes/p255_do_while.sp"
    int count = 0;

#line 25 "c_superset_probes/p255_do_while.sp"
    int j = 0;

#line 26 "c_superset_probes/p255_do_while.sp"
    do {

#line 27 "c_superset_probes/p255_do_while.sp"
        if (j == 3) 
            break;

#line 28 "c_superset_probes/p255_do_while.sp"
        count = count + 1;

#line 29 "c_superset_probes/p255_do_while.sp"
        j = j + 1;
    }
    while (j < 10);

#line 31 "c_superset_probes/p255_do_while.sp"
    if (count != 3) 
        return 3;

#line 34 "c_superset_probes/p255_do_while.sp"
    int sum2 = 0;

#line 35 "c_superset_probes/p255_do_while.sp"
    int k = -1;

#line 36 "c_superset_probes/p255_do_while.sp"
    do {

#line 37 "c_superset_probes/p255_do_while.sp"
        k = k + 1;

#line 38 "c_superset_probes/p255_do_while.sp"
        if (k == 3) 
            continue;

#line 39 "c_superset_probes/p255_do_while.sp"
        sum2 = sum2 + k;
    }
    while (k < 5);

#line 42 "c_superset_probes/p255_do_while.sp"
    if (sum2 != 12) 
        return 4;

#line 44 "c_superset_probes/p255_do_while.sp"
    return 0;
}
