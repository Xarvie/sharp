
#include <stdlib.h>

#line 5 "unit/controlflow/switch_loop.sp"
int test_ret(int val) {

#line 6 "unit/controlflow/switch_loop.sp"
    switch (val) {

#line 7 "unit/controlflow/switch_loop.sp"
        case 1:
        return 1;

#line 8 "unit/controlflow/switch_loop.sp"
        case 2:
        return 2;

#line 9 "unit/controlflow/switch_loop.sp"
        default:
        return 42;
    }
}

#line 14 "unit/controlflow/switch_loop.sp"
int for_test() {
    int i = 5, s1 = 0;

#line 16 "unit/controlflow/switch_loop.sp"
    for (; i > 0; i = i - 1) 
        s1 = s1 + i;

#line 17 "unit/controlflow/switch_loop.sp"
    if (s1 != 15) 
        return 1;
    int count = 0, j = 0;

#line 20 "unit/controlflow/switch_loop.sp"
    for (; ; ) {

#line 21 "unit/controlflow/switch_loop.sp"
        count = count + 1;

#line 22 "unit/controlflow/switch_loop.sp"
        j = j + 1;

#line 23 "unit/controlflow/switch_loop.sp"
        if (j >= 10) 
            break;
    }

#line 25 "unit/controlflow/switch_loop.sp"
    if (count != 10) 
        return 2;
    int power = 1, k = 0;

#line 28 "unit/controlflow/switch_loop.sp"
    for (; k < 4; ) {

#line 29 "unit/controlflow/switch_loop.sp"
        power = power * 2;

#line 30 "unit/controlflow/switch_loop.sp"
        k = k + 1;
    }

#line 32 "unit/controlflow/switch_loop.sp"
    if (power != 16) 
        return 3;
    int a = 0, b = 0;

#line 35 "unit/controlflow/switch_loop.sp"
    for (int m = 0, n = 10; m < 5; m = m + 1) {

#line 36 "unit/controlflow/switch_loop.sp"
        a = a + m;

#line 37 "unit/controlflow/switch_loop.sp"
        b = b + n - m;
    }

#line 39 "unit/controlflow/switch_loop.sp"
    if (a != 10) 
        return 4;

#line 40 "unit/controlflow/switch_loop.sp"
    if (b != 40) 
        return 5;

#line 42 "unit/controlflow/switch_loop.sp"
    int hit = 0;

#line 43 "unit/controlflow/switch_loop.sp"
    for (int p = 0; p < 0; p = p + 1) 
        hit = 1;

#line 44 "unit/controlflow/switch_loop.sp"
    if (hit != 0) 
        return 6;

#line 45 "unit/controlflow/switch_loop.sp"
    return 0;
}

#line 48 "unit/controlflow/switch_loop.sp"
int main() {
    int r1 = 0, x = 2;

#line 51 "unit/controlflow/switch_loop.sp"
    switch (x) {

#line 52 "unit/controlflow/switch_loop.sp"
        case 1:
        r1 = 10;
        break;

#line 53 "unit/controlflow/switch_loop.sp"
        case 2:
        r1 = 20;
        break;

#line 54 "unit/controlflow/switch_loop.sp"
        case 3:
        r1 = 30;
        break;

#line 55 "unit/controlflow/switch_loop.sp"
        default:
        r1 = 0;
        break;
    }

#line 57 "unit/controlflow/switch_loop.sp"
    if (r1 != 20) 
        return 10;
    int r2 = 0, y = 4;

#line 60 "unit/controlflow/switch_loop.sp"
    switch (y) {

#line 61 "unit/controlflow/switch_loop.sp"
        case 1:
        case 2:
        case 3:
        r2 = 3;
        break;

#line 62 "unit/controlflow/switch_loop.sp"
        case 4:
        case 5:
        r2 = 5;
        break;

#line 63 "unit/controlflow/switch_loop.sp"
        default:
        r2 = 0;
        break;
    }

#line 65 "unit/controlflow/switch_loop.sp"
    if (r2 != 5) 
        return 11;

#line 67 "unit/controlflow/switch_loop.sp"
    if (test_ret(1) != 1) 
        return 12;

#line 68 "unit/controlflow/switch_loop.sp"
    if (test_ret(2) != 2) 
        return 13;

#line 69 "unit/controlflow/switch_loop.sp"
    if (test_ret(99) != 42) 
        return 14;
    int r4 = 0, z = 7;

#line 72 "unit/controlflow/switch_loop.sp"
    switch (z) {
        default:
        r4 = 99;
        break;
    }

#line 73 "unit/controlflow/switch_loop.sp"
    if (r4 != 99) 
        return 15;

#line 75 "unit/controlflow/switch_loop.sp"
    switch (x) {
    }
    int sum = 0, i = 0;

#line 79 "unit/controlflow/switch_loop.sp"
    do {
        sum = sum + i;
        i = i + 1;
    }
    while (i < 5);

#line 80 "unit/controlflow/switch_loop.sp"
    if (sum != 10) 
        return 20;

#line 82 "unit/controlflow/switch_loop.sp"
    int flag = 0;

#line 83 "unit/controlflow/switch_loop.sp"
    do {
        flag = 1;
    }
    while (0);

#line 84 "unit/controlflow/switch_loop.sp"
    if (flag != 1) 
        return 21;
    int count = 0, j = 0;

#line 87 "unit/controlflow/switch_loop.sp"
    do {

#line 88 "unit/controlflow/switch_loop.sp"
        if (j == 3) 
            break;

#line 89 "unit/controlflow/switch_loop.sp"
        count = count + 1;

#line 90 "unit/controlflow/switch_loop.sp"
        j = j + 1;
    }
    while (j < 10);

#line 92 "unit/controlflow/switch_loop.sp"
    if (count != 3) 
        return 22;
    int sum2 = 0, k = -1;

#line 95 "unit/controlflow/switch_loop.sp"
    do {

#line 96 "unit/controlflow/switch_loop.sp"
        k = k + 1;

#line 97 "unit/controlflow/switch_loop.sp"
        if (k == 3) 
            continue;

#line 98 "unit/controlflow/switch_loop.sp"
        sum2 = sum2 + k;
    }
    while (k < 5);

#line 100 "unit/controlflow/switch_loop.sp"
    if (sum2 != 12) 
        return 23;

#line 103 "unit/controlflow/switch_loop.sp"
    int cnt2 = 0;

#line 104 "unit/controlflow/switch_loop.sp"
    for (int ii = 0; ii < 5; ii = ii + 1) {

#line 105 "unit/controlflow/switch_loop.sp"
        for (int jj = 0; jj < 5; jj = jj + 1) {

#line 106 "unit/controlflow/switch_loop.sp"
            if (jj == 3) 
                break;

#line 107 "unit/controlflow/switch_loop.sp"
            cnt2 = cnt2 + 1;
        }
    }

#line 110 "unit/controlflow/switch_loop.sp"
    if (cnt2 != 15) 
        return 30;

#line 112 "unit/controlflow/switch_loop.sp"
    int ssum = 0;

#line 113 "unit/controlflow/switch_loop.sp"
    for (int ii = 0; ii < 3; ii = ii + 1) {

#line 114 "unit/controlflow/switch_loop.sp"
        for (int jj = 0; jj < 3; jj = jj + 1) {

#line 115 "unit/controlflow/switch_loop.sp"
            if (jj == 1) 
                continue;

#line 116 "unit/controlflow/switch_loop.sp"
            ssum = ssum + ii + jj;
        }
    }

#line 119 "unit/controlflow/switch_loop.sp"
    if (ssum != 12) 
        return 31;

#line 121 "unit/controlflow/switch_loop.sp"
    int accum = 0;

#line 122 "unit/controlflow/switch_loop.sp"
    for (int ii = 0; ii < 3; ii = ii + 1) {

#line 123 "unit/controlflow/switch_loop.sp"
        int jj = 3;

#line 124 "unit/controlflow/switch_loop.sp"
        while (jj > 0) {
            accum = accum + ii;
            jj = jj - 1;
        }
    }

#line 126 "unit/controlflow/switch_loop.sp"
    if (accum != 9) 
        return 32;

#line 128 "unit/controlflow/switch_loop.sp"
    int grid = 0;

#line 129 "unit/controlflow/switch_loop.sp"
    for (int row = 0; row < 4; row = row + 1) 

#line 130 "unit/controlflow/switch_loop.sp"
        for (int col = 0; col < 3; col = col + 1) 

#line 131 "unit/controlflow/switch_loop.sp"
            grid = grid + 1;

#line 132 "unit/controlflow/switch_loop.sp"
    if (grid != 12) 
        return 33;

#line 135 "unit/controlflow/switch_loop.sp"
    int fr = for_test();

#line 136 "unit/controlflow/switch_loop.sp"
    if (fr != 0) 
        return fr;

#line 138 "unit/controlflow/switch_loop.sp"
    return 0;
}
