

#line 6 "c_superset_probes/p257_multi_return_defer.sp"
int seq[8];

#line 7 "c_superset_probes/p257_multi_return_defer.sp"
int seqn = 0;

#line 8 "c_superset_probes/p257_multi_return_defer.sp"
void mark(int v) {
    seq[seqn++] = v;
}

#line 11 "c_superset_probes/p257_multi_return_defer.sp"
int test_multi_return(int v) {

#line 13 "c_superset_probes/p257_multi_return_defer.sp"
    if (v > 0) {
        int __sharp_ret = 10;
        
#line 12 "c_superset_probes/p257_multi_return_defer.sp"
        mark(1);
        return __sharp_ret;
    }

#line 14 "c_superset_probes/p257_multi_return_defer.sp"
    int __sharp_ret = 20;
    
#line 12 "c_superset_probes/p257_multi_return_defer.sp"
    mark(1);
    return __sharp_ret;
}

#line 18 "c_superset_probes/p257_multi_return_defer.sp"
int test_early_return(int v) {

#line 20 "c_superset_probes/p257_multi_return_defer.sp"
    if (v == 1) {
        int __sharp_ret = 100;
        
#line 19 "c_superset_probes/p257_multi_return_defer.sp"
        mark(2);
        return __sharp_ret;
    }

#line 21 "c_superset_probes/p257_multi_return_defer.sp"
    if (v == 2) {
        int __sharp_ret = 200;
        
#line 19 "c_superset_probes/p257_multi_return_defer.sp"
        mark(2);
        return __sharp_ret;
    }

#line 22 "c_superset_probes/p257_multi_return_defer.sp"
    int __sharp_ret = 300;
    
#line 19 "c_superset_probes/p257_multi_return_defer.sp"
    mark(2);
    return __sharp_ret;
}

#line 25 "c_superset_probes/p257_multi_return_defer.sp"
int main() {

#line 27 "c_superset_probes/p257_multi_return_defer.sp"
    seqn = 0;

#line 28 "c_superset_probes/p257_multi_return_defer.sp"
    int r1 = test_multi_return(1);

#line 29 "c_superset_probes/p257_multi_return_defer.sp"
    if (r1 != 10) 
        return 1;

#line 30 "c_superset_probes/p257_multi_return_defer.sp"
    if (seqn != 1) 
        return 2;

#line 31 "c_superset_probes/p257_multi_return_defer.sp"
    if (seq[0] != 1) 
        return 3;

#line 33 "c_superset_probes/p257_multi_return_defer.sp"
    seqn = 0;

#line 34 "c_superset_probes/p257_multi_return_defer.sp"
    int r2 = test_multi_return(0);

#line 35 "c_superset_probes/p257_multi_return_defer.sp"
    if (r2 != 20) 
        return 4;

#line 36 "c_superset_probes/p257_multi_return_defer.sp"
    if (seqn != 1) 
        return 5;

#line 37 "c_superset_probes/p257_multi_return_defer.sp"
    if (seq[0] != 1) 
        return 6;

#line 40 "c_superset_probes/p257_multi_return_defer.sp"
    seqn = 0;

#line 41 "c_superset_probes/p257_multi_return_defer.sp"
    int r3 = test_early_return(1);

#line 42 "c_superset_probes/p257_multi_return_defer.sp"
    if (r3 != 100) 
        return 7;

#line 43 "c_superset_probes/p257_multi_return_defer.sp"
    if (seqn != 1) 
        return 8;

#line 44 "c_superset_probes/p257_multi_return_defer.sp"
    if (seq[0] != 2) 
        return 9;

#line 46 "c_superset_probes/p257_multi_return_defer.sp"
    seqn = 0;

#line 47 "c_superset_probes/p257_multi_return_defer.sp"
    int r4 = test_early_return(2);

#line 48 "c_superset_probes/p257_multi_return_defer.sp"
    if (r4 != 200) 
        return 10;

#line 49 "c_superset_probes/p257_multi_return_defer.sp"
    if (seqn != 1) 
        return 11;

#line 51 "c_superset_probes/p257_multi_return_defer.sp"
    seqn = 0;

#line 52 "c_superset_probes/p257_multi_return_defer.sp"
    int r5 = test_early_return(3);

#line 53 "c_superset_probes/p257_multi_return_defer.sp"
    if (r5 != 300) 
        return 12;

#line 54 "c_superset_probes/p257_multi_return_defer.sp"
    if (seqn != 1) 
        return 13;

#line 56 "c_superset_probes/p257_multi_return_defer.sp"
    return 0;
}
