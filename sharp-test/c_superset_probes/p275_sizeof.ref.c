

#line 6 "c_superset_probes/p275_sizeof.sp"
typedef struct Vec4 Vec4;
struct Vec4 {
    int x;
    int y;
    int z;
    int w;
};

#line 13 "c_superset_probes/p275_sizeof.sp"
typedef int IntArr[10];

#line 15 "c_superset_probes/p275_sizeof.sp"
int main() {

#line 16 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(int) != 4) 
        return 1;

#line 17 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(Vec4) != 16) 
        return 2;

#line 18 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(Vec4 *) != 8) 
        return 3;

#line 21 "c_superset_probes/p275_sizeof.sp"
    int arr[5] = { 0 };

#line 22 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(arr) != 20) 
        return 4;

#line 25 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(IntArr) != 40) 
        return 5;

#line 28 "c_superset_probes/p275_sizeof.sp"
    if (sizeof(Vec4) / sizeof(int) != 4) 
        return 6;

#line 30 "c_superset_probes/p275_sizeof.sp"
    return 0;
}
