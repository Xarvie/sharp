
#line 2 "sharp-test/unit/defer/defer_conditional.ce"
int log_buf[32];

#line 3 "sharp-test/unit/defer/defer_conditional.ce"
int logn = 0;

#line 4 "sharp-test/unit/defer/defer_conditional.ce"
void mark(int v) {
    if (logn < 32) {
        log_buf[logn] = v;
        logn = logn + 1;
    }
}

#line 6 "sharp-test/unit/defer/defer_conditional.ce"
int test_branch_defer(int branch) {

#line 7 "sharp-test/unit/defer/defer_conditional.ce"
    if (branch == 1) {

#line 9 "sharp-test/unit/defer/defer_conditional.ce"
        mark(1);

#line 10 "sharp-test/unit/defer/defer_conditional.ce"
        int __sharp_ret = 1;
        
#line 8 "sharp-test/unit/defer/defer_conditional.ce"
        mark(100);
        return __sharp_ret;
    }
    else {

#line 13 "sharp-test/unit/defer/defer_conditional.ce"
        mark(2);

#line 14 "sharp-test/unit/defer/defer_conditional.ce"
        int __sharp_ret = 2;
        
#line 12 "sharp-test/unit/defer/defer_conditional.ce"
        mark(200);
        return __sharp_ret;
    }
}

#line 18 "sharp-test/unit/defer/defer_conditional.ce"
int test_early_return(int x) {

#line 20 "sharp-test/unit/defer/defer_conditional.ce"
    if (x < 0) {
        int __sharp_ret = -1;
        
#line 19 "sharp-test/unit/defer/defer_conditional.ce"
        mark(50);
        return __sharp_ret;
    }

#line 21 "sharp-test/unit/defer/defer_conditional.ce"
    if (x > 100) {
        int __sharp_ret = 101;
        
#line 19 "sharp-test/unit/defer/defer_conditional.ce"
        mark(50);
        return __sharp_ret;
    }

#line 22 "sharp-test/unit/defer/defer_conditional.ce"
    mark(3);

#line 23 "sharp-test/unit/defer/defer_conditional.ce"
    int __sharp_ret = x;
    
#line 19 "sharp-test/unit/defer/defer_conditional.ce"
    mark(50);
    return __sharp_ret;
}

#line 26 "sharp-test/unit/defer/defer_conditional.ce"
int main() {

#line 28 "sharp-test/unit/defer/defer_conditional.ce"
    logn = 0;

#line 29 "sharp-test/unit/defer/defer_conditional.ce"
    int r1 = test_branch_defer(1);

#line 30 "sharp-test/unit/defer/defer_conditional.ce"
    if (r1 != 1) 
        return 1;

#line 31 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[0] != 1) 
        return 2;

#line 32 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[1] != 100) 
        return 3;

#line 34 "sharp-test/unit/defer/defer_conditional.ce"
    logn = 0;

#line 35 "sharp-test/unit/defer/defer_conditional.ce"
    int r2 = test_branch_defer(2);

#line 36 "sharp-test/unit/defer/defer_conditional.ce"
    if (r2 != 2) 
        return 4;

#line 37 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[0] != 2) 
        return 5;

#line 38 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[1] != 200) 
        return 6;

#line 41 "sharp-test/unit/defer/defer_conditional.ce"
    logn = 0;

#line 42 "sharp-test/unit/defer/defer_conditional.ce"
    int r3 = test_early_return(-5);

#line 43 "sharp-test/unit/defer/defer_conditional.ce"
    if (r3 != -1) 
        return 7;

#line 44 "sharp-test/unit/defer/defer_conditional.ce"
    if (logn != 1) 
        return 8;

#line 45 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[0] != 50) 
        return 9;

#line 47 "sharp-test/unit/defer/defer_conditional.ce"
    logn = 0;

#line 48 "sharp-test/unit/defer/defer_conditional.ce"
    int r4 = test_early_return(42);

#line 49 "sharp-test/unit/defer/defer_conditional.ce"
    if (r4 != 42) 
        return 10;

#line 50 "sharp-test/unit/defer/defer_conditional.ce"
    if (logn != 2) 
        return 11;

#line 51 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[0] != 3) 
        return 12;

#line 52 "sharp-test/unit/defer/defer_conditional.ce"
    if (log_buf[1] != 50) 
        return 13;

#line 54 "sharp-test/unit/defer/defer_conditional.ce"
    return 0;
}
