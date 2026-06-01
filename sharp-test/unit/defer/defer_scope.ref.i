
#line 2 "sharp-test/unit/defer/defer_scope.ce"
int log_buf[32];

#line 3 "sharp-test/unit/defer/defer_scope.ce"
int logn = 0;

#line 4 "sharp-test/unit/defer/defer_scope.ce"
void mark(int v) {
    if (logn < 32) {
        log_buf[logn] = v;
        logn = logn + 1;
    }
}

#line 6 "sharp-test/unit/defer/defer_scope.ce"
int test_scope_defer() {

#line 7 "sharp-test/unit/defer/defer_scope.ce"
    mark(1);

#line 8 "sharp-test/unit/defer/defer_scope.ce"
{

#line 10 "sharp-test/unit/defer/defer_scope.ce"
        mark(2);

#line 11 "sharp-test/unit/defer/defer_scope.ce"
{

#line 13 "sharp-test/unit/defer/defer_scope.ce"
            mark(3);
            
#line 12 "sharp-test/unit/defer/defer_scope.ce"
            mark(20);
        }

#line 15 "sharp-test/unit/defer/defer_scope.ce"
        mark(4);
        
#line 9 "sharp-test/unit/defer/defer_scope.ce"
        mark(10);
    }

#line 17 "sharp-test/unit/defer/defer_scope.ce"
    mark(5);

#line 18 "sharp-test/unit/defer/defer_scope.ce"
    return 0;
}

#line 21 "sharp-test/unit/defer/defer_scope.ce"
int test_if_defer() {

#line 22 "sharp-test/unit/defer/defer_scope.ce"
    int x = 5;

#line 23 "sharp-test/unit/defer/defer_scope.ce"
    if (x > 0) {

#line 25 "sharp-test/unit/defer/defer_scope.ce"
        mark(6);
        
#line 24 "sharp-test/unit/defer/defer_scope.ce"
        mark(30);
    }

#line 27 "sharp-test/unit/defer/defer_scope.ce"
    mark(7);

#line 28 "sharp-test/unit/defer/defer_scope.ce"
    return 0;
}

#line 31 "sharp-test/unit/defer/defer_scope.ce"
int test_for_defer() {

#line 32 "sharp-test/unit/defer/defer_scope.ce"
    for (int i = 0; i < 3; i = i + 1) {

#line 34 "sharp-test/unit/defer/defer_scope.ce"
        mark(8);
        
#line 33 "sharp-test/unit/defer/defer_scope.ce"
        mark(40 + i);
    }

#line 36 "sharp-test/unit/defer/defer_scope.ce"
    mark(9);

#line 37 "sharp-test/unit/defer/defer_scope.ce"
    return 0;
}

#line 40 "sharp-test/unit/defer/defer_scope.ce"
int main() {

#line 42 "sharp-test/unit/defer/defer_scope.ce"
    logn = 0;

#line 43 "sharp-test/unit/defer/defer_scope.ce"
    test_scope_defer();

#line 44 "sharp-test/unit/defer/defer_scope.ce"
    if (logn != 7) 
        return 1;

#line 45 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[0] != 1) 
        return 2;

#line 46 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[1] != 2) 
        return 3;

#line 47 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[2] != 3) 
        return 4;

#line 48 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[3] != 20) 
        return 5;

#line 49 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[4] != 4) 
        return 6;

#line 50 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[5] != 10) 
        return 7;

#line 51 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[6] != 5) 
        return 8;

#line 54 "sharp-test/unit/defer/defer_scope.ce"
    logn = 0;

#line 55 "sharp-test/unit/defer/defer_scope.ce"
    test_if_defer();

#line 56 "sharp-test/unit/defer/defer_scope.ce"
    if (logn != 3) 
        return 9;

#line 57 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[0] != 6) 
        return 10;

#line 58 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[1] != 30) 
        return 11;

#line 59 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[2] != 7) 
        return 12;

#line 62 "sharp-test/unit/defer/defer_scope.ce"
    logn = 0;

#line 63 "sharp-test/unit/defer/defer_scope.ce"
    test_for_defer();

#line 64 "sharp-test/unit/defer/defer_scope.ce"
    if (logn != 7) 
        return 13;

#line 65 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[0] != 8) 
        return 14;

#line 66 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[1] != 40) 
        return 15;

#line 67 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[2] != 8) 
        return 16;

#line 68 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[3] != 41) 
        return 17;

#line 69 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[4] != 8) 
        return 18;

#line 70 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[5] != 42) 
        return 19;

#line 71 "sharp-test/unit/defer/defer_scope.ce"
    if (log_buf[6] != 9) 
        return 20;

#line 73 "sharp-test/unit/defer/defer_scope.ce"
    return 0;
}
