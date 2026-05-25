#include <__stddef_ptrdiff_t.h>
#include <__stddef_size_t.h>
#include <__stddef_wchar_t.h>
#include <__stddef_max_align_t.h>


#line 6 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
constexpr int INT_SIZE = sizeof(int);

#line 7 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
constexpr int CHAR_SIZE = sizeof(char);

#line 20 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
void swap__int(int * a, int * b);

#line 27 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
void fill_array__int(int * arr, int val, int count);

#line 10 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
constexpr int element_count(int total_bytes) {

#line 11 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    return total_bytes / sizeof(int);
}

#line 15 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
constexpr int default_capacity(void) {

#line 16 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    return 256;
}

#line 20 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"

#line 27 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"

#line 35 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
int main(void) {

#line 37 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (INT_SIZE != 4) 
        return 1;

#line 38 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (CHAR_SIZE != 1) 
        return 2;

#line 41 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (element_count(1024) != 256) 
        return 3;

#line 42 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (default_capacity() != 256) 
        return 4;
    int x = 10, y = 20;

#line 46 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    swap__int(&x, &y);

#line 47 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (x != 20) 
        return 5;

#line 48 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (y != 10) 
        return 6;

#line 51 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    int arr[4];

#line 52 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    fill_array__int(arr, 42, 4);

#line 53 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (arr[0] != 42) 
        return 7;

#line 54 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    if (arr[3] != 42) 
        return 8;

#line 56 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    return 0;
}

#line 20 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
__attribute__((weak)) void swap__int(int * a, int * b) {

#line 21 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    int tmp = *a;

#line 22 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    *a = *b;

#line 23 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    *b = tmp;
}


#line 27 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
__attribute__((weak)) void fill_array__int(int * arr, int val, int count) {

#line 28 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    int i = 0;

#line 29 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
    while (i < count) {

#line 30 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
        arr[i] = val;

#line 31 "./sharp-test/c_superset_probes/p216_constexpr_generic.sp"
        i = i + 1;
    }
}

