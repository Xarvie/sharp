

#line 6 "c_superset_probes/p266_runtime_defer.sp"
int seq[10];

#line 7 "c_superset_probes/p266_runtime_defer.sp"
int seqn = 0;

#line 8 "c_superset_probes/p266_runtime_defer.sp"
void mark(int v) {
    seq[seqn++] = v;
}

#line 10 "c_superset_probes/p266_runtime_defer.sp"
int test_defer_return(int v) {

#line 12 "c_superset_probes/p266_runtime_defer.sp"
    if (v > 0) {
        int __sharp_ret = v;
        
#line 11 "c_superset_probes/p266_runtime_defer.sp"
        mark(100);
        return __sharp_ret;
    }

#line 13 "c_superset_probes/p266_runtime_defer.sp"
    int __sharp_ret = 0;
    
#line 11 "c_superset_probes/p266_runtime_defer.sp"
    mark(100);
    return __sharp_ret;
}

#line 16 "c_superset_probes/p266_runtime_defer.sp"
void test_defer_lifo(void) {
    
#line 19 "c_superset_probes/p266_runtime_defer.sp"
    mark(3);
    
#line 18 "c_superset_probes/p266_runtime_defer.sp"
    mark(2);
    
#line 17 "c_superset_probes/p266_runtime_defer.sp"
    mark(1);
}

#line 22 "c_superset_probes/p266_runtime_defer.sp"
int main() {

#line 24 "c_superset_probes/p266_runtime_defer.sp"
    seqn = 0;

#line 25 "c_superset_probes/p266_runtime_defer.sp"
    int r = test_defer_return(42);

#line 26 "c_superset_probes/p266_runtime_defer.sp"
    if (r != 42) 
        return 1;

#line 27 "c_superset_probes/p266_runtime_defer.sp"
    if (seqn != 1) 
        return 2;

#line 28 "c_superset_probes/p266_runtime_defer.sp"
    if (seq[0] != 100) 
        return 3;

#line 31 "c_superset_probes/p266_runtime_defer.sp"
    seqn = 0;

#line 32 "c_superset_probes/p266_runtime_defer.sp"
    test_defer_lifo();

#line 33 "c_superset_probes/p266_runtime_defer.sp"
    if (seqn != 3) 
        return 4;

#line 34 "c_superset_probes/p266_runtime_defer.sp"
    if (seq[0] != 3) 
        return 5;

#line 35 "c_superset_probes/p266_runtime_defer.sp"
    if (seq[1] != 2) 
        return 6;

#line 36 "c_superset_probes/p266_runtime_defer.sp"
    if (seq[2] != 1) 
        return 7;

#line 39 "c_superset_probes/p266_runtime_defer.sp"
    seqn = 0;

#line 40 "c_superset_probes/p266_runtime_defer.sp"
    for (int i = 0; i < 3; i = i + 1) {
        
#line 41 "c_superset_probes/p266_runtime_defer.sp"
        mark(i);
    }

#line 44 "c_superset_probes/p266_runtime_defer.sp"
    if (seqn != 3) 
        return 8;

#line 46 "c_superset_probes/p266_runtime_defer.sp"
    return 0;
}
