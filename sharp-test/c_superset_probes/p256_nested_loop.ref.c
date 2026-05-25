

#line 6 "c_superset_probes/p256_nested_loop.sp"
int main() {

#line 8 "c_superset_probes/p256_nested_loop.sp"
    int count = 0;

#line 9 "c_superset_probes/p256_nested_loop.sp"
    for (int i = 0; i < 5; i = i + 1) {

#line 10 "c_superset_probes/p256_nested_loop.sp"
        for (int j = 0; j < 5; j = j + 1) {

#line 11 "c_superset_probes/p256_nested_loop.sp"
            if (j == 3) 
                break;

#line 12 "c_superset_probes/p256_nested_loop.sp"
            count = count + 1;
        }
    }

#line 16 "c_superset_probes/p256_nested_loop.sp"
    if (count != 15) 
        return 1;

#line 19 "c_superset_probes/p256_nested_loop.sp"
    int sum = 0;

#line 20 "c_superset_probes/p256_nested_loop.sp"
    for (int i = 0; i < 3; i = i + 1) {

#line 21 "c_superset_probes/p256_nested_loop.sp"
        for (int j = 0; j < 3; j = j + 1) {

#line 22 "c_superset_probes/p256_nested_loop.sp"
            if (j == 1) 
                continue;

#line 23 "c_superset_probes/p256_nested_loop.sp"
            sum = sum + i + j;
        }
    }

#line 29 "c_superset_probes/p256_nested_loop.sp"
    if (sum != 12) 
        return 2;

#line 32 "c_superset_probes/p256_nested_loop.sp"
    int accum = 0;

#line 33 "c_superset_probes/p256_nested_loop.sp"
    for (int i = 0; i < 3; i = i + 1) {

#line 34 "c_superset_probes/p256_nested_loop.sp"
        int j = 3;

#line 35 "c_superset_probes/p256_nested_loop.sp"
        while (j > 0) {

#line 36 "c_superset_probes/p256_nested_loop.sp"
            accum = accum + i;

#line 37 "c_superset_probes/p256_nested_loop.sp"
            j = j - 1;
        }
    }

#line 41 "c_superset_probes/p256_nested_loop.sp"
    if (accum != 9) 
        return 3;

#line 44 "c_superset_probes/p256_nested_loop.sp"
    int grid = 0;

#line 45 "c_superset_probes/p256_nested_loop.sp"
    for (int row = 0; row < 4; row = row + 1) {

#line 46 "c_superset_probes/p256_nested_loop.sp"
        for (int col = 0; col < 3; col = col + 1) {

#line 47 "c_superset_probes/p256_nested_loop.sp"
            grid = grid + 1;
        }
    }

#line 50 "c_superset_probes/p256_nested_loop.sp"
    if (grid != 12) 
        return 4;

#line 52 "c_superset_probes/p256_nested_loop.sp"
    return 0;
}
