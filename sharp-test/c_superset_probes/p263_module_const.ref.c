

#line 7 "c_superset_probes/p263_const.sph"
constexpr int compute_limit(int base);

#line 3 "c_superset_probes/p263_const.sph"
const int MAX_SIZE = 256;

#line 4 "c_superset_probes/p263_const.sph"
const int MIN_SIZE = 16;

#line 5 "c_superset_probes/p263_const.sph"
const float PI = 3.14f;

#line 11 "c_superset_probes/p263_const.sph"
constexpr int LIMIT = compute_limit(100);

#line 7 "c_superset_probes/p263_const.sph"
constexpr int compute_limit(int base) {

#line 8 "c_superset_probes/p263_const.sph"
    return base * 2 + 10;
}

#line 8 "c_superset_probes/p263_module_const.sp"
int main() {

#line 9 "c_superset_probes/p263_module_const.sp"
    if (MAX_SIZE != 256) 
        return 1;

#line 10 "c_superset_probes/p263_module_const.sp"
    if (MIN_SIZE != 16) 
        return 2;

#line 11 "c_superset_probes/p263_module_const.sp"
    if (LIMIT != 210) 
        return 3;

#line 14 "c_superset_probes/p263_module_const.sp"
    int arr[MAX_SIZE];

#line 15 "c_superset_probes/p263_module_const.sp"
    if (sizeof(arr) != sizeof(int) * 256) 
        return 4;

#line 18 "c_superset_probes/p263_module_const.sp"
    if (PI < 3.13f || PI > 3.15f) 
        return 5;

#line 20 "c_superset_probes/p263_module_const.sp"
    return 0;
}
