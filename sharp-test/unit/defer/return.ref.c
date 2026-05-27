
#include <stdlib.h>

#line 4 "unit/defer/return.sp"
int seq[10];

#line 5 "unit/defer/return.sp"
int seqn = 0;

#line 9 "unit/defer/return.sp"
int counter = 0;

#line 24 "unit/defer/return.sp"
int void_ran = 0;

#line 6 "unit/defer/return.sp"
void mark(int v) {
    seq[seqn++] = v;
}

#line 10 "unit/defer/return.sp"
void bump(void) {
    counter = counter + 1;
}

#line 11 "unit/defer/return.sp"
int snapshot(void) {

#line 13 "unit/defer/return.sp"
    int __sharp_ret = counter;
    
#line 12 "unit/defer/return.sp"
    bump();
    return __sharp_ret;
}

#line 17 "unit/defer/return.sp"
void lifo(void) {
    
#line 20 "unit/defer/return.sp"
    mark(3);
    
#line 19 "unit/defer/return.sp"
    mark(2);
    
#line 18 "unit/defer/return.sp"
    mark(1);
}

#line 25 "unit/defer/return.sp"
void set_void(void) {
    void_ran = 1;
}

#line 26 "unit/defer/return.sp"
void vfunc(void) {
    
#line 27 "unit/defer/return.sp"
    set_void();
}

#line 31 "unit/defer/return.sp"
int test_multi_return(int v) {

#line 33 "unit/defer/return.sp"
    if (v > 0) {
        int __sharp_ret = 10;
        
#line 32 "unit/defer/return.sp"
        mark(10);
        return __sharp_ret;
    }

#line 34 "unit/defer/return.sp"
    int __sharp_ret = 20;
    
#line 32 "unit/defer/return.sp"
    mark(10);
    return __sharp_ret;
}

#line 38 "unit/defer/return.sp"
int test_early_return(int v) {

#line 40 "unit/defer/return.sp"
    if (v == 1) {
        int __sharp_ret = 100;
        
#line 39 "unit/defer/return.sp"
        mark(20);
        return __sharp_ret;
    }

#line 41 "unit/defer/return.sp"
    if (v == 2) {
        int __sharp_ret = 200;
        
#line 39 "unit/defer/return.sp"
        mark(20);
        return __sharp_ret;
    }

#line 42 "unit/defer/return.sp"
    int __sharp_ret = 300;
    
#line 39 "unit/defer/return.sp"
    mark(20);
    return __sharp_ret;
}

#line 46 "unit/defer/return.sp"
int test_defer_ret(int v) {

#line 48 "unit/defer/return.sp"
    if (v > 0) {
        int __sharp_ret = v;
        
#line 47 "unit/defer/return.sp"
        mark(100);
        return __sharp_ret;
    }

#line 49 "unit/defer/return.sp"
    int __sharp_ret = 0;
    
#line 47 "unit/defer/return.sp"
    mark(100);
    return __sharp_ret;
}

#line 52 "unit/defer/return.sp"
int main(void) {

#line 54 "unit/defer/return.sp"
    counter = 0;

#line 55 "unit/defer/return.sp"
    int v = snapshot();

#line 56 "unit/defer/return.sp"
    if (v != 0) 
        return 1;

#line 57 "unit/defer/return.sp"
    if (counter != 1) 
        return 2;

#line 60 "unit/defer/return.sp"
    seqn = 0;

#line 61 "unit/defer/return.sp"
    lifo();

#line 62 "unit/defer/return.sp"
    if (seqn != 3) 
        return 3;

#line 63 "unit/defer/return.sp"
    if (seq[0] != 3) 
        return 4;

#line 64 "unit/defer/return.sp"
    if (seq[1] != 2) 
        return 5;

#line 65 "unit/defer/return.sp"
    if (seq[2] != 1) 
        return 6;

#line 68 "unit/defer/return.sp"
    void_ran = 0;

#line 69 "unit/defer/return.sp"
    vfunc();

#line 70 "unit/defer/return.sp"
    if (void_ran != 1) 
        return 7;

#line 73 "unit/defer/return.sp"
    seqn = 0;

#line 74 "unit/defer/return.sp"
    int r1 = test_multi_return(1);

#line 75 "unit/defer/return.sp"
    if (r1 != 10) 
        return 8;

#line 76 "unit/defer/return.sp"
    if (seqn != 1) 
        return 9;

#line 77 "unit/defer/return.sp"
    if (seq[0] != 10) 
        return 10;

#line 79 "unit/defer/return.sp"
    seqn = 0;

#line 80 "unit/defer/return.sp"
    int r2 = test_multi_return(0);

#line 81 "unit/defer/return.sp"
    if (r2 != 20) 
        return 11;

#line 82 "unit/defer/return.sp"
    if (seqn != 1) 
        return 12;

#line 85 "unit/defer/return.sp"
    seqn = 0;

#line 86 "unit/defer/return.sp"
    int r3 = test_early_return(1);

#line 87 "unit/defer/return.sp"
    if (r3 != 100) 
        return 13;

#line 88 "unit/defer/return.sp"
    if (seqn != 1) 
        return 14;

#line 89 "unit/defer/return.sp"
    if (seq[0] != 20) 
        return 15;

#line 91 "unit/defer/return.sp"
    seqn = 0;

#line 92 "unit/defer/return.sp"
    int r4 = test_early_return(2);

#line 93 "unit/defer/return.sp"
    if (r4 != 200) 
        return 16;

#line 94 "unit/defer/return.sp"
    if (seqn != 1) 
        return 17;

#line 96 "unit/defer/return.sp"
    seqn = 0;

#line 97 "unit/defer/return.sp"
    int r5 = test_early_return(3);

#line 98 "unit/defer/return.sp"
    if (r5 != 300) 
        return 18;

#line 99 "unit/defer/return.sp"
    if (seqn != 1) 
        return 19;

#line 102 "unit/defer/return.sp"
    seqn = 0;

#line 103 "unit/defer/return.sp"
    int r6 = test_defer_ret(42);

#line 104 "unit/defer/return.sp"
    if (r6 != 42) 
        return 20;

#line 105 "unit/defer/return.sp"
    if (seqn != 1) 
        return 21;

#line 106 "unit/defer/return.sp"
    if (seq[0] != 100) 
        return 22;

#line 108 "unit/defer/return.sp"
    return 0;
}
