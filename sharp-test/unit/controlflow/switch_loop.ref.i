
#line 5 "sharp-test/unit/controlflow/switch_loop.ce"
int test_ret(int val) {

#line 6 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (val) {

#line 7 "sharp-test/unit/controlflow/switch_loop.ce"
        case 1:
        return 1;

#line 8 "sharp-test/unit/controlflow/switch_loop.ce"
        case 2:
        return 2;

#line 9 "sharp-test/unit/controlflow/switch_loop.ce"
        default:
        return 42;
    }
}

#line 14 "sharp-test/unit/controlflow/switch_loop.ce"
int for_test() {
    int i = 5, s1 = 0;

#line 16 "sharp-test/unit/controlflow/switch_loop.ce"
    for (; i > 0; i = i - 1) 
        s1 = s1 + i;

#line 17 "sharp-test/unit/controlflow/switch_loop.ce"
    if (s1 != 15) 
        return 1;
    int count = 0, j = 0;

#line 20 "sharp-test/unit/controlflow/switch_loop.ce"
    for (; ; ) {

#line 21 "sharp-test/unit/controlflow/switch_loop.ce"
        count = count + 1;

#line 22 "sharp-test/unit/controlflow/switch_loop.ce"
        j = j + 1;

#line 23 "sharp-test/unit/controlflow/switch_loop.ce"
        if (j >= 10) 
            break;
    }

#line 25 "sharp-test/unit/controlflow/switch_loop.ce"
    if (count != 10) 
        return 2;
    int power = 1, k = 0;

#line 28 "sharp-test/unit/controlflow/switch_loop.ce"
    for (; k < 4; ) {

#line 29 "sharp-test/unit/controlflow/switch_loop.ce"
        power = power * 2;

#line 30 "sharp-test/unit/controlflow/switch_loop.ce"
        k = k + 1;
    }

#line 32 "sharp-test/unit/controlflow/switch_loop.ce"
    if (power != 16) 
        return 3;
    int a = 0, b = 0;

#line 35 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int m = 0, n = 10; m < 5; m = m + 1) {

#line 36 "sharp-test/unit/controlflow/switch_loop.ce"
        a = a + m;

#line 37 "sharp-test/unit/controlflow/switch_loop.ce"
        b = b + n - m;
    }

#line 39 "sharp-test/unit/controlflow/switch_loop.ce"
    if (a != 10) 
        return 4;

#line 40 "sharp-test/unit/controlflow/switch_loop.ce"
    if (b != 40) 
        return 5;

#line 42 "sharp-test/unit/controlflow/switch_loop.ce"
    int hit = 0;

#line 43 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int p = 0; p < 0; p = p + 1) 
        hit = 1;

#line 44 "sharp-test/unit/controlflow/switch_loop.ce"
    if (hit != 0) 
        return 6;

#line 45 "sharp-test/unit/controlflow/switch_loop.ce"
    return 0;
}

#line 48 "sharp-test/unit/controlflow/switch_loop.ce"
int main() {
    int r1 = 0, x = 2;

#line 51 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (x) {

#line 52 "sharp-test/unit/controlflow/switch_loop.ce"
        case 1:
        r1 = 10;
        break;

#line 53 "sharp-test/unit/controlflow/switch_loop.ce"
        case 2:
        r1 = 20;
        break;

#line 54 "sharp-test/unit/controlflow/switch_loop.ce"
        case 3:
        r1 = 30;
        break;

#line 55 "sharp-test/unit/controlflow/switch_loop.ce"
        default:
        r1 = 0;
        break;
    }

#line 57 "sharp-test/unit/controlflow/switch_loop.ce"
    if (r1 != 20) 
        return 10;
    int r2 = 0, y = 4;

#line 60 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (y) {

#line 61 "sharp-test/unit/controlflow/switch_loop.ce"
        case 1:
        case 2:
        case 3:
        r2 = 3;
        break;

#line 62 "sharp-test/unit/controlflow/switch_loop.ce"
        case 4:
        case 5:
        r2 = 5;
        break;

#line 63 "sharp-test/unit/controlflow/switch_loop.ce"
        default:
        r2 = 0;
        break;
    }

#line 65 "sharp-test/unit/controlflow/switch_loop.ce"
    if (r2 != 5) 
        return 11;

#line 67 "sharp-test/unit/controlflow/switch_loop.ce"
    if (test_ret(1) != 1) 
        return 12;

#line 68 "sharp-test/unit/controlflow/switch_loop.ce"
    if (test_ret(2) != 2) 
        return 13;

#line 69 "sharp-test/unit/controlflow/switch_loop.ce"
    if (test_ret(99) != 42) 
        return 14;
    int r4 = 0, z = 7;

#line 72 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (z) {
        default:
        r4 = 99;
        break;
    }

#line 73 "sharp-test/unit/controlflow/switch_loop.ce"
    if (r4 != 99) 
        return 15;

#line 75 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (x) {
    }

#line 78 "sharp-test/unit/controlflow/switch_loop.ce"
    int acc = 0;

#line 79 "sharp-test/unit/controlflow/switch_loop.ce"
    int mode = 2;

#line 80 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (mode) {

#line 81 "sharp-test/unit/controlflow/switch_loop.ce"
        case 3:
        acc = acc + 100;

#line 82 "sharp-test/unit/controlflow/switch_loop.ce"
        case 2:
        acc = acc + 10;

#line 83 "sharp-test/unit/controlflow/switch_loop.ce"
        case 1:
        acc = acc + 1;

#line 84 "sharp-test/unit/controlflow/switch_loop.ce"
        break;

#line 85 "sharp-test/unit/controlflow/switch_loop.ce"
        default:
        acc = 999;
    }

#line 87 "sharp-test/unit/controlflow/switch_loop.ce"
    if (acc != 11) 
        return 16;
    int outer = 1, inner = 2, result = 0;

#line 91 "sharp-test/unit/controlflow/switch_loop.ce"
    switch (outer) {

#line 92 "sharp-test/unit/controlflow/switch_loop.ce"
        case 1:

#line 93 "sharp-test/unit/controlflow/switch_loop.ce"
        switch (inner) {

#line 94 "sharp-test/unit/controlflow/switch_loop.ce"
            case 1:
            result = 11;
            break;

#line 95 "sharp-test/unit/controlflow/switch_loop.ce"
            case 2:
            result = 12;
            break;

#line 96 "sharp-test/unit/controlflow/switch_loop.ce"
            default:
            result = 19;
            break;
        }

#line 98 "sharp-test/unit/controlflow/switch_loop.ce"
        break;

#line 99 "sharp-test/unit/controlflow/switch_loop.ce"
        case 2:
        result = 20;
        break;
    }

#line 101 "sharp-test/unit/controlflow/switch_loop.ce"
    if (result != 12) 
        return 17;
    int sum3 = 0, k2 = 10;

#line 105 "sharp-test/unit/controlflow/switch_loop.ce"
    do {

#line 106 "sharp-test/unit/controlflow/switch_loop.ce"
        if (k2 % 2 == 0) 
            sum3 = sum3 + k2;

#line 107 "sharp-test/unit/controlflow/switch_loop.ce"
        k2 = k2 - 1;
    }
    while (k2 > 0);

#line 109 "sharp-test/unit/controlflow/switch_loop.ce"
    if (sum3 != 30) 
        return 18;
    int sum = 0, i = 0;

#line 113 "sharp-test/unit/controlflow/switch_loop.ce"
    do {
        sum = sum + i;
        i = i + 1;
    }
    while (i < 5);

#line 114 "sharp-test/unit/controlflow/switch_loop.ce"
    if (sum != 10) 
        return 20;

#line 116 "sharp-test/unit/controlflow/switch_loop.ce"
    int flag = 0;

#line 117 "sharp-test/unit/controlflow/switch_loop.ce"
    do {
        flag = 1;
    }
    while (0);

#line 118 "sharp-test/unit/controlflow/switch_loop.ce"
    if (flag != 1) 
        return 21;
    int count = 0, j = 0;

#line 121 "sharp-test/unit/controlflow/switch_loop.ce"
    do {

#line 122 "sharp-test/unit/controlflow/switch_loop.ce"
        if (j == 3) 
            break;

#line 123 "sharp-test/unit/controlflow/switch_loop.ce"
        count = count + 1;

#line 124 "sharp-test/unit/controlflow/switch_loop.ce"
        j = j + 1;
    }
    while (j < 10);

#line 126 "sharp-test/unit/controlflow/switch_loop.ce"
    if (count != 3) 
        return 22;
    int sum2 = 0, k = -1;

#line 129 "sharp-test/unit/controlflow/switch_loop.ce"
    do {

#line 130 "sharp-test/unit/controlflow/switch_loop.ce"
        k = k + 1;

#line 131 "sharp-test/unit/controlflow/switch_loop.ce"
        if (k == 3) 
            continue;

#line 132 "sharp-test/unit/controlflow/switch_loop.ce"
        sum2 = sum2 + k;
    }
    while (k < 5);

#line 134 "sharp-test/unit/controlflow/switch_loop.ce"
    if (sum2 != 12) 
        return 23;

#line 137 "sharp-test/unit/controlflow/switch_loop.ce"
    int cnt2 = 0;

#line 138 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int ii = 0; ii < 5; ii = ii + 1) {

#line 139 "sharp-test/unit/controlflow/switch_loop.ce"
        for (int jj = 0; jj < 5; jj = jj + 1) {

#line 140 "sharp-test/unit/controlflow/switch_loop.ce"
            if (jj == 3) 
                break;

#line 141 "sharp-test/unit/controlflow/switch_loop.ce"
            cnt2 = cnt2 + 1;
        }
    }

#line 144 "sharp-test/unit/controlflow/switch_loop.ce"
    if (cnt2 != 15) 
        return 30;

#line 146 "sharp-test/unit/controlflow/switch_loop.ce"
    int ssum = 0;

#line 147 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int ii = 0; ii < 3; ii = ii + 1) {

#line 148 "sharp-test/unit/controlflow/switch_loop.ce"
        for (int jj = 0; jj < 3; jj = jj + 1) {

#line 149 "sharp-test/unit/controlflow/switch_loop.ce"
            if (jj == 1) 
                continue;

#line 150 "sharp-test/unit/controlflow/switch_loop.ce"
            ssum = ssum + ii + jj;
        }
    }

#line 153 "sharp-test/unit/controlflow/switch_loop.ce"
    if (ssum != 12) 
        return 31;

#line 155 "sharp-test/unit/controlflow/switch_loop.ce"
    int accum = 0;

#line 156 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int ii = 0; ii < 3; ii = ii + 1) {

#line 157 "sharp-test/unit/controlflow/switch_loop.ce"
        int jj = 3;

#line 158 "sharp-test/unit/controlflow/switch_loop.ce"
        while (jj > 0) {
            accum = accum + ii;
            jj = jj - 1;
        }
    }

#line 160 "sharp-test/unit/controlflow/switch_loop.ce"
    if (accum != 9) 
        return 32;

#line 162 "sharp-test/unit/controlflow/switch_loop.ce"
    int grid = 0;

#line 163 "sharp-test/unit/controlflow/switch_loop.ce"
    for (int row = 0; row < 4; row = row + 1) 

#line 164 "sharp-test/unit/controlflow/switch_loop.ce"
        for (int col = 0; col < 3; col = col + 1) 

#line 165 "sharp-test/unit/controlflow/switch_loop.ce"
            grid = grid + 1;

#line 166 "sharp-test/unit/controlflow/switch_loop.ce"
    if (grid != 12) 
        return 33;

#line 169 "sharp-test/unit/controlflow/switch_loop.ce"
    int fr = for_test();

#line 170 "sharp-test/unit/controlflow/switch_loop.ce"
    if (fr != 0) 
        return fr;

#line 172 "sharp-test/unit/controlflow/switch_loop.ce"
    return 0;
}
