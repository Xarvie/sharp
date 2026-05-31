
#line 4 "sharp-test/unit/defer/return.ce"
int seq[10];

#line 5 "sharp-test/unit/defer/return.ce"
int seqn = 0;

#line 9 "sharp-test/unit/defer/return.ce"
int counter = 0;

#line 24 "sharp-test/unit/defer/return.ce"
int void_ran = 0;

#line 53 "sharp-test/unit/defer/return.ce"
int inner_ran = 0;

#line 54 "sharp-test/unit/defer/return.ce"
int outer_ran = 0;

#line 6 "sharp-test/unit/defer/return.ce"
void mark(int v) {
    seq[seqn++] = v;
}

#line 10 "sharp-test/unit/defer/return.ce"
void bump(void) {
    counter = counter + 1;
}

#line 11 "sharp-test/unit/defer/return.ce"
int snapshot(void) {

#line 13 "sharp-test/unit/defer/return.ce"
    int __sharp_ret = counter;
    
#line 12 "sharp-test/unit/defer/return.ce"
    bump();
    return __sharp_ret;
}

#line 17 "sharp-test/unit/defer/return.ce"
void lifo(void) {
    
#line 20 "sharp-test/unit/defer/return.ce"
    mark(3);
    
#line 19 "sharp-test/unit/defer/return.ce"
    mark(2);
    
#line 18 "sharp-test/unit/defer/return.ce"
    mark(1);
}

#line 25 "sharp-test/unit/defer/return.ce"
void set_void(void) {
    void_ran = 1;
}

#line 26 "sharp-test/unit/defer/return.ce"
void vfunc(void) {
    
#line 27 "sharp-test/unit/defer/return.ce"
    set_void();
}

#line 31 "sharp-test/unit/defer/return.ce"
int test_multi_return(int v) {

#line 33 "sharp-test/unit/defer/return.ce"
    if (v > 0) {
        int __sharp_ret = 10;
        
#line 32 "sharp-test/unit/defer/return.ce"
        mark(10);
        return __sharp_ret;
    }

#line 34 "sharp-test/unit/defer/return.ce"
    int __sharp_ret = 20;
    
#line 32 "sharp-test/unit/defer/return.ce"
    mark(10);
    return __sharp_ret;
}

#line 38 "sharp-test/unit/defer/return.ce"
int test_early_return(int v) {

#line 40 "sharp-test/unit/defer/return.ce"
    if (v == 1) {
        int __sharp_ret = 100;
        
#line 39 "sharp-test/unit/defer/return.ce"
        mark(20);
        return __sharp_ret;
    }

#line 41 "sharp-test/unit/defer/return.ce"
    if (v == 2) {
        int __sharp_ret = 200;
        
#line 39 "sharp-test/unit/defer/return.ce"
        mark(20);
        return __sharp_ret;
    }

#line 42 "sharp-test/unit/defer/return.ce"
    int __sharp_ret = 300;
    
#line 39 "sharp-test/unit/defer/return.ce"
    mark(20);
    return __sharp_ret;
}

#line 46 "sharp-test/unit/defer/return.ce"
int test_defer_ret(int v) {

#line 48 "sharp-test/unit/defer/return.ce"
    if (v > 0) {
        int __sharp_ret = v;
        
#line 47 "sharp-test/unit/defer/return.ce"
        mark(100);
        return __sharp_ret;
    }

#line 49 "sharp-test/unit/defer/return.ce"
    int __sharp_ret = 0;
    
#line 47 "sharp-test/unit/defer/return.ce"
    mark(100);
    return __sharp_ret;
}

#line 55 "sharp-test/unit/defer/return.ce"
void inner_func() {
    
#line 56 "sharp-test/unit/defer/return.ce"
{
        inner_ran = 1;
    }
}

#line 58 "sharp-test/unit/defer/return.ce"
void outer_func() {

#line 60 "sharp-test/unit/defer/return.ce"
    inner_func();
    
#line 59 "sharp-test/unit/defer/return.ce"
{
        outer_ran = 1;
    }
}

#line 64 "sharp-test/unit/defer/return.ce"
int cond_defer(int x) {

#line 66 "sharp-test/unit/defer/return.ce"
    if (x > 10) {
        int __sharp_ret = 1;
        
#line 65 "sharp-test/unit/defer/return.ce"
        mark(50);
        return __sharp_ret;
    }

#line 67 "sharp-test/unit/defer/return.ce"
    if (x < 0) {
        int __sharp_ret = -1;
        
#line 65 "sharp-test/unit/defer/return.ce"
        mark(50);
        return __sharp_ret;
    }

#line 68 "sharp-test/unit/defer/return.ce"
    int __sharp_ret = 0;
    
#line 65 "sharp-test/unit/defer/return.ce"
    mark(50);
    return __sharp_ret;
}

#line 71 "sharp-test/unit/defer/return.ce"
int main(void) {

#line 73 "sharp-test/unit/defer/return.ce"
    counter = 0;

#line 74 "sharp-test/unit/defer/return.ce"
    int v = snapshot();

#line 75 "sharp-test/unit/defer/return.ce"
    if (v != 0) 
        return 1;

#line 76 "sharp-test/unit/defer/return.ce"
    if (counter != 1) 
        return 2;

#line 79 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 80 "sharp-test/unit/defer/return.ce"
    lifo();

#line 81 "sharp-test/unit/defer/return.ce"
    if (seqn != 3) 
        return 3;

#line 82 "sharp-test/unit/defer/return.ce"
    if (seq[0] != 3) 
        return 4;

#line 83 "sharp-test/unit/defer/return.ce"
    if (seq[1] != 2) 
        return 5;

#line 84 "sharp-test/unit/defer/return.ce"
    if (seq[2] != 1) 
        return 6;

#line 87 "sharp-test/unit/defer/return.ce"
    void_ran = 0;

#line 88 "sharp-test/unit/defer/return.ce"
    vfunc();

#line 89 "sharp-test/unit/defer/return.ce"
    if (void_ran != 1) 
        return 7;

#line 92 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 93 "sharp-test/unit/defer/return.ce"
    int r1 = test_multi_return(1);

#line 94 "sharp-test/unit/defer/return.ce"
    if (r1 != 10) 
        return 8;

#line 95 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 9;

#line 96 "sharp-test/unit/defer/return.ce"
    if (seq[0] != 10) 
        return 10;

#line 98 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 99 "sharp-test/unit/defer/return.ce"
    int r2 = test_multi_return(0);

#line 100 "sharp-test/unit/defer/return.ce"
    if (r2 != 20) 
        return 11;

#line 101 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 12;

#line 104 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 105 "sharp-test/unit/defer/return.ce"
    int r3 = test_early_return(1);

#line 106 "sharp-test/unit/defer/return.ce"
    if (r3 != 100) 
        return 13;

#line 107 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 14;

#line 108 "sharp-test/unit/defer/return.ce"
    if (seq[0] != 20) 
        return 15;

#line 110 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 111 "sharp-test/unit/defer/return.ce"
    int r4 = test_early_return(2);

#line 112 "sharp-test/unit/defer/return.ce"
    if (r4 != 200) 
        return 16;

#line 113 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 17;

#line 115 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 116 "sharp-test/unit/defer/return.ce"
    int r5 = test_early_return(3);

#line 117 "sharp-test/unit/defer/return.ce"
    if (r5 != 300) 
        return 18;

#line 118 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 19;

#line 121 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 122 "sharp-test/unit/defer/return.ce"
    int r6 = test_defer_ret(42);

#line 123 "sharp-test/unit/defer/return.ce"
    if (r6 != 42) 
        return 20;

#line 124 "sharp-test/unit/defer/return.ce"
    if (seqn != 1) 
        return 21;

#line 125 "sharp-test/unit/defer/return.ce"
    if (seq[0] != 100) 
        return 22;

#line 128 "sharp-test/unit/defer/return.ce"
    inner_ran = 0;

#line 129 "sharp-test/unit/defer/return.ce"
    outer_ran = 0;

#line 130 "sharp-test/unit/defer/return.ce"
    outer_func();

#line 131 "sharp-test/unit/defer/return.ce"
    if (inner_ran != 1) 
        return 23;

#line 132 "sharp-test/unit/defer/return.ce"
    if (outer_ran != 1) 
        return 24;

#line 135 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 136 "sharp-test/unit/defer/return.ce"
    int cr1 = cond_defer(15);

#line 137 "sharp-test/unit/defer/return.ce"
    if (cr1 != 1) 
        return 25;

#line 138 "sharp-test/unit/defer/return.ce"
    if (seqn != 1 || seq[0] != 50) 
        return 26;

#line 140 "sharp-test/unit/defer/return.ce"
    seqn = 0;

#line 141 "sharp-test/unit/defer/return.ce"
    int cr2 = cond_defer(5);

#line 142 "sharp-test/unit/defer/return.ce"
    if (cr2 != 0) 
        return 27;

#line 143 "sharp-test/unit/defer/return.ce"
    if (seqn != 1 || seq[0] != 50) 
        return 28;

#line 145 "sharp-test/unit/defer/return.ce"
    return 0;
}
