
#include <stdlib.h>

#line 7 "sharp-test/c_superset_probes/p109_defer_loop.sp"
int log_buf[16];

#line 8 "sharp-test/c_superset_probes/p109_defer_loop.sp"
int logn = 0;

#line 9 "sharp-test/c_superset_probes/p109_defer_loop.sp"
void mark(int v) {
    log_buf[logn++] = v;
}

#line 11 "sharp-test/c_superset_probes/p109_defer_loop.sp"
int main(void) {

#line 13 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    logn = 0;

#line 14 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    int i = 0;

#line 15 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    while (i < 5) {

#line 17 "sharp-test/c_superset_probes/p109_defer_loop.sp"
        i = i + 1;

#line 18 "sharp-test/c_superset_probes/p109_defer_loop.sp"
        if (i == 3) {
            
#line 16 "sharp-test/c_superset_probes/p109_defer_loop.sp"
            mark(7);
            break;
        }
                mark(7);
    }

#line 21 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    if (logn != 3) 
        return 1;

#line 22 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    if (log_buf[0] != 7 || log_buf[1] != 7 || log_buf[2] != 7) 
        return 2;

#line 25 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    logn = 0;

#line 26 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    int sum = 0;

#line 27 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    for (int j = 0; j < 4; j++) {

#line 29 "sharp-test/c_superset_probes/p109_defer_loop.sp"
        if (j == 2) {
            
#line 28 "sharp-test/c_superset_probes/p109_defer_loop.sp"
            mark(9);
            continue;
        }

#line 30 "sharp-test/c_superset_probes/p109_defer_loop.sp"
        sum = sum + j;
        
#line 28 "sharp-test/c_superset_probes/p109_defer_loop.sp"
        mark(9);
    }

#line 33 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    if (logn != 4) 
        return 3;

#line 35 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    if (sum != 4) 
        return 4;

#line 37 "sharp-test/c_superset_probes/p109_defer_loop.sp"
    return 0;
}
