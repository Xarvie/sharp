#include <stdbool.h>


#line 12 "sharp-test/c_superset_probes/p230_zero_init.sp"
typedef struct Pair Pair;
struct Pair {
    int a;
    int b;
};

#line 17 "sharp-test/c_superset_probes/p230_zero_init.sp"
typedef struct Container Container;
struct Container {
    int val;
    int * ptr;
    Pair inner;
};

#line 23 "sharp-test/c_superset_probes/p230_zero_init.sp"
int main() {

#line 25 "sharp-test/c_superset_probes/p230_zero_init.sp"
    int x = { 0 };

#line 26 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (x != 0) 
        return 1;

#line 28 "sharp-test/c_superset_probes/p230_zero_init.sp"
    float f = { 0 };

#line 29 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (f != 0.0f) 
        return 2;

#line 31 "sharp-test/c_superset_probes/p230_zero_init.sp"
    _Bool b = { 0 };

#line 32 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (b != 0) 
        return 3;

#line 35 "sharp-test/c_superset_probes/p230_zero_init.sp"
    Container c = { 0 };

#line 36 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (c.val != 0) 
        return 4;

#line 37 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (c.ptr != 0) 
        return 5;

#line 38 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (c.inner.a != 0) 
        return 6;

#line 39 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (c.inner.b != 0) 
        return 7;

#line 42 "sharp-test/c_superset_probes/p230_zero_init.sp"
    int * np = { 0 };

#line 43 "sharp-test/c_superset_probes/p230_zero_init.sp"
    if (np != 0) 
        return 8;

#line 45 "sharp-test/c_superset_probes/p230_zero_init.sp"
    return 0;
}
