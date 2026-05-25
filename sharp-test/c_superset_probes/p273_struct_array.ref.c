

#line 6 "c_superset_probes/p273_struct_array.sp"
typedef struct Vec2 Vec2;
struct Vec2 {
    int x;
    int y;
};

#line 11 "c_superset_probes/p273_struct_array.sp"
int main() {

#line 13 "c_superset_probes/p273_struct_array.sp"
    Vec2 points[4] = { 0 };

#line 15 "c_superset_probes/p273_struct_array.sp"
    points[0].x = 1;
    points[0].y = 2;

#line 16 "c_superset_probes/p273_struct_array.sp"
    points[1].x = 3;
    points[1].y = 4;

#line 17 "c_superset_probes/p273_struct_array.sp"
    points[2].x = 5;
    points[2].y = 6;

#line 18 "c_superset_probes/p273_struct_array.sp"
    points[3].x = 7;
    points[3].y = 8;
    int sum_x = 0, sum_y = 0;

#line 22 "c_superset_probes/p273_struct_array.sp"
    for (int i = 0; i < 4; i = i + 1) {

#line 23 "c_superset_probes/p273_struct_array.sp"
        sum_x = sum_x + points[i].x;

#line 24 "c_superset_probes/p273_struct_array.sp"
        sum_y = sum_y + points[i].y;
    }

#line 26 "c_superset_probes/p273_struct_array.sp"
    if (sum_x != 16) 
        return 1;

#line 27 "c_superset_probes/p273_struct_array.sp"
    if (sum_y != 20) 
        return 2;

#line 30 "c_superset_probes/p273_struct_array.sp"
    Vec2 partial[3] = { 0 };

#line 31 "c_superset_probes/p273_struct_array.sp"
    partial[1].x = 99;

#line 32 "c_superset_probes/p273_struct_array.sp"
    if (partial[0].x != 0) 
        return 3;

#line 33 "c_superset_probes/p273_struct_array.sp"
    if (partial[1].x != 99) 
        return 4;

#line 34 "c_superset_probes/p273_struct_array.sp"
    if (partial[2].x != 0) 
        return 5;

#line 36 "c_superset_probes/p273_struct_array.sp"
    return 0;
}
