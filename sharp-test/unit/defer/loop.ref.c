
#include <stdlib.h>

#line 4 "unit/defer/loop.sp"
int log_buf[20];

#line 5 "unit/defer/loop.sp"
int logn = 0;

#line 6 "unit/defer/loop.sp"
void mark(int v) {
    log_buf[logn++] = v;
}

#line 8 "unit/defer/loop.sp"
int main(void) {

#line 10 "unit/defer/loop.sp"
    logn = 0;

#line 11 "unit/defer/loop.sp"
    int i = 0;

#line 12 "unit/defer/loop.sp"
    while (i < 5) {

#line 14 "unit/defer/loop.sp"
        i = i + 1;

#line 15 "unit/defer/loop.sp"
        if (i == 3) {
            
#line 13 "unit/defer/loop.sp"
            mark(7);
            break;
        }
                mark(7);
    }

#line 17 "unit/defer/loop.sp"
    if (logn != 3) 
        return 1;

#line 18 "unit/defer/loop.sp"
    if (log_buf[0] != 7 || log_buf[1] != 7 || log_buf[2] != 7) 
        return 2;

#line 21 "unit/defer/loop.sp"
    logn = 0;

#line 22 "unit/defer/loop.sp"
    int sum = 0;

#line 23 "unit/defer/loop.sp"
    for (int j = 0; j < 4; j++) {

#line 25 "unit/defer/loop.sp"
        if (j == 2) {
            
#line 24 "unit/defer/loop.sp"
            mark(9);
            continue;
        }

#line 26 "unit/defer/loop.sp"
        sum = sum + j;
        
#line 24 "unit/defer/loop.sp"
        mark(9);
    }

#line 28 "unit/defer/loop.sp"
    if (logn != 4) 
        return 3;

#line 29 "unit/defer/loop.sp"
    if (sum != 4) 
        return 4;

#line 32 "unit/defer/loop.sp"
    logn = 0;

#line 33 "unit/defer/loop.sp"
    for (int k = 0; k < 3; k = k + 1) {
        
#line 34 "unit/defer/loop.sp"
        mark(k);
    }

#line 36 "unit/defer/loop.sp"
    if (logn != 3) 
        return 5;

#line 38 "unit/defer/loop.sp"
    return 0;
}
