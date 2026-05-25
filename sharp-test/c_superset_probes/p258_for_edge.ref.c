
#include <stdlib.h>

#line 6 "sharp-test/c_superset_probes/p258_for_edge.sp"
int main() {

#line 8 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int i = 5;

#line 9 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int s1 = 0;

#line 10 "sharp-test/c_superset_probes/p258_for_edge.sp"
    for (; i > 0; i = i - 1) {

#line 11 "sharp-test/c_superset_probes/p258_for_edge.sp"
        s1 = s1 + i;
    }

#line 13 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (s1 != 15) 
        return 1;

#line 16 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int count = 0;

#line 17 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int j = 0;

#line 18 "sharp-test/c_superset_probes/p258_for_edge.sp"
    for (; ; ) {

#line 19 "sharp-test/c_superset_probes/p258_for_edge.sp"
        count = count + 1;

#line 20 "sharp-test/c_superset_probes/p258_for_edge.sp"
        j = j + 1;

#line 21 "sharp-test/c_superset_probes/p258_for_edge.sp"
        if (j >= 10) 
            break;
    }

#line 23 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (count != 10) 
        return 2;

#line 26 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int power = 1;

#line 27 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int k = 0;

#line 28 "sharp-test/c_superset_probes/p258_for_edge.sp"
    for (; k < 4; ) {

#line 29 "sharp-test/c_superset_probes/p258_for_edge.sp"
        power = power * 2;

#line 30 "sharp-test/c_superset_probes/p258_for_edge.sp"
        k = k + 1;
    }

#line 32 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (power != 16) 
        return 3;
    int a = 0, b = 0;

#line 36 "sharp-test/c_superset_probes/p258_for_edge.sp"
    for (int m = 0, n = 10; m < 5; m = m + 1) {

#line 37 "sharp-test/c_superset_probes/p258_for_edge.sp"
        a = a + m;

#line 38 "sharp-test/c_superset_probes/p258_for_edge.sp"
        b = b + n - m;
    }

#line 41 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (a != 10) 
        return 4;

#line 42 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (b != 40) 
        return 5;

#line 45 "sharp-test/c_superset_probes/p258_for_edge.sp"
    int hit = 0;

#line 46 "sharp-test/c_superset_probes/p258_for_edge.sp"
    for (int p = 0; p < 0; p = p + 1) {

#line 47 "sharp-test/c_superset_probes/p258_for_edge.sp"
        hit = 1;
    }

#line 49 "sharp-test/c_superset_probes/p258_for_edge.sp"
    if (hit != 0) 
        return 6;

#line 51 "sharp-test/c_superset_probes/p258_for_edge.sp"
    return 0;
}
