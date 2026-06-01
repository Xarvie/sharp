
#line 3 "sharp-test/unit/controlflow/nested_loop.ce"
int main() {

#line 5 "sharp-test/unit/controlflow/nested_loop.ce"
    int mat[3][3] = { { 0 } };

#line 6 "sharp-test/unit/controlflow/nested_loop.ce"
    for (int i = 0; i < 3; i = i + 1) 

#line 7 "sharp-test/unit/controlflow/nested_loop.ce"
        for (int j = 0; j < 3; j = j + 1) 

#line 8 "sharp-test/unit/controlflow/nested_loop.ce"
            mat[i][j] = i * 3 + j;

#line 9 "sharp-test/unit/controlflow/nested_loop.ce"
    if (mat[0][0] != 0) 
        return 1;

#line 10 "sharp-test/unit/controlflow/nested_loop.ce"
    if (mat[1][2] != 5) 
        return 2;

#line 11 "sharp-test/unit/controlflow/nested_loop.ce"
    if (mat[2][2] != 8) 
        return 3;
    int found_i = -1, found_j = -1;

#line 15 "sharp-test/unit/controlflow/nested_loop.ce"
    for (int i = 0; i < 3; i = i + 1) {

#line 16 "sharp-test/unit/controlflow/nested_loop.ce"
        for (int j = 0; j < 3; j = j + 1) {

#line 17 "sharp-test/unit/controlflow/nested_loop.ce"
            if (mat[i][j] == 5) {

#line 18 "sharp-test/unit/controlflow/nested_loop.ce"
                found_i = i;

#line 19 "sharp-test/unit/controlflow/nested_loop.ce"
                found_j = j;

#line 20 "sharp-test/unit/controlflow/nested_loop.ce"
                break;
            }
        }
    }

#line 24 "sharp-test/unit/controlflow/nested_loop.ce"
    if (found_i != 1 || found_j != 2) 
        return 4;

#line 27 "sharp-test/unit/controlflow/nested_loop.ce"
    int odd_sum = 0;

#line 28 "sharp-test/unit/controlflow/nested_loop.ce"
    for (int i = 0; i < 10; i = i + 1) {

#line 29 "sharp-test/unit/controlflow/nested_loop.ce"
        if (i % 2 == 0) 
            continue;

#line 30 "sharp-test/unit/controlflow/nested_loop.ce"
        odd_sum = odd_sum + i;
    }

#line 32 "sharp-test/unit/controlflow/nested_loop.ce"
    if (odd_sum != 25) 
        return 5;

#line 35 "sharp-test/unit/controlflow/nested_loop.ce"
    int count = 0;

#line 36 "sharp-test/unit/controlflow/nested_loop.ce"
    int i = 0;

#line 37 "sharp-test/unit/controlflow/nested_loop.ce"
    while (i < 10) {

#line 38 "sharp-test/unit/controlflow/nested_loop.ce"
        int j = 0;

#line 39 "sharp-test/unit/controlflow/nested_loop.ce"
        while (j < 10) {

#line 40 "sharp-test/unit/controlflow/nested_loop.ce"
            count = count + 1;

#line 41 "sharp-test/unit/controlflow/nested_loop.ce"
            if (j == 2) 
                break;

#line 42 "sharp-test/unit/controlflow/nested_loop.ce"
            j = j + 1;
        }

#line 44 "sharp-test/unit/controlflow/nested_loop.ce"
        i = i + 1;
    }

#line 46 "sharp-test/unit/controlflow/nested_loop.ce"
    if (count != 30) 
        return 6;

#line 49 "sharp-test/unit/controlflow/nested_loop.ce"
    int sum = 0;

#line 50 "sharp-test/unit/controlflow/nested_loop.ce"
    int k = 0;

#line 51 "sharp-test/unit/controlflow/nested_loop.ce"
    do {

#line 52 "sharp-test/unit/controlflow/nested_loop.ce"
        if (k % 3 == 0) {
            k = k + 1;
            continue;
        }

#line 53 "sharp-test/unit/controlflow/nested_loop.ce"
        sum = sum + k;

#line 54 "sharp-test/unit/controlflow/nested_loop.ce"
        k = k + 1;
    }
    while (k < 10);

#line 56 "sharp-test/unit/controlflow/nested_loop.ce"
    if (sum != 27) 
        return 7;

#line 58 "sharp-test/unit/controlflow/nested_loop.ce"
    return 0;
}
