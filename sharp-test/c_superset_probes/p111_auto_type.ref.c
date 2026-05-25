

#line 9 "sharp-test/c_superset_probes/p111_auto_type.sp"
struct Point {
    int x;
    int y;
};

#line 7 "sharp-test/c_superset_probes/p111_auto_type.sp"
int identity(int x) {
    return x;
}

#line 10 "sharp-test/c_superset_probes/p111_auto_type.sp"
struct Point make_point(int x, int y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}

#line 12 "sharp-test/c_superset_probes/p111_auto_type.sp"
int main(void) {

#line 13 "sharp-test/c_superset_probes/p111_auto_type.sp"
    int a = 42;

#line 14 "sharp-test/c_superset_probes/p111_auto_type.sp"
    if (a != 42) 
        return 1;

#line 16 "sharp-test/c_superset_probes/p111_auto_type.sp"
    int b = identity(7);

#line 17 "sharp-test/c_superset_probes/p111_auto_type.sp"
    if (b != 7) 
        return 2;

#line 19 "sharp-test/c_superset_probes/p111_auto_type.sp"
    struct Point p = make_point(3, 4);

#line 20 "sharp-test/c_superset_probes/p111_auto_type.sp"
    if (p.x != 3) 
        return 3;

#line 21 "sharp-test/c_superset_probes/p111_auto_type.sp"
    if (p.y != 4) 
        return 4;

#line 24 "sharp-test/c_superset_probes/p111_auto_type.sp"
    int sum = 0;

#line 25 "sharp-test/c_superset_probes/p111_auto_type.sp"
    for (int i = 0; i < 5; i = i + 1) 

#line 26 "sharp-test/c_superset_probes/p111_auto_type.sp"
        sum = sum + i;

#line 27 "sharp-test/c_superset_probes/p111_auto_type.sp"
    if (sum != 10) 
        return 5;

#line 29 "sharp-test/c_superset_probes/p111_auto_type.sp"
    return 0;
}
