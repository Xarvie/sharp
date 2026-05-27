// 来源: p254_switch.sp, p255_do_while.sp, p256_nested_loop.sp, p258_for_edge.sp
// 控制流: switch + do-while + 嵌套循环 + for边界

// === p254: switch ===
int test_ret(int val) {
    switch (val) {
        case 1: return 1;
        case 2: return 2;
        default: return 42;
    }
}

// === p258: for edge ===
int for_test() {
    int i = 5, s1 = 0;
    for (; i > 0; i = i - 1) s1 = s1 + i;
    if (s1 != 15) return 1;  /* 5+4+3+2+1 */

    int count = 0, j = 0;
    for (;;) {
        count = count + 1;
        j = j + 1;
        if (j >= 10) break;
    }
    if (count != 10) return 2;

    int power = 1, k = 0;
    for (; k < 4;) {
        power = power * 2;
        k = k + 1;
    }
    if (power != 16) return 3;

    int a = 0, b = 0;
    for (int m = 0, n = 10; m < 5; m = m + 1) {
        a = a + m;
        b = b + n - m;
    }
    if (a != 10) return 4;
    if (b != 40) return 5;

    int hit = 0;
    for (int p = 0; p < 0; p = p + 1) hit = 1;
    if (hit != 0) return 6;
    return 0;
}

int main() {
    // --- p254: switch tests ---
    int r1 = 0, x = 2;
    switch (x) {
        case 1: r1 = 10; break;
        case 2: r1 = 20; break;
        case 3: r1 = 30; break;
        default: r1 = 0; break;
    }
    if (r1 != 20) return 10;

    int r2 = 0, y = 4;
    switch (y) {
        case 1: case 2: case 3: r2 = 3; break;
        case 4: case 5:            r2 = 5; break;
        default: r2 = 0; break;
    }
    if (r2 != 5) return 11;

    if (test_ret(1) != 1) return 12;
    if (test_ret(2) != 2) return 13;
    if (test_ret(99) != 42) return 14;

    int r4 = 0, z = 7;
    switch (z) { default: r4 = 99; break; }
    if (r4 != 99) return 15;

    switch (x) { /* empty */ }

    // --- p255: do-while tests ---
    int sum = 0, i = 0;
    do { sum = sum + i; i = i + 1; } while (i < 5);
    if (sum != 10) return 20;

    int flag = 0;
    do { flag = 1; } while (0);
    if (flag != 1) return 21;

    int count = 0, j = 0;
    do {
        if (j == 3) break;
        count = count + 1;
        j = j + 1;
    } while (j < 10);
    if (count != 3) return 22;

    int sum2 = 0, k = -1;
    do {
        k = k + 1;
        if (k == 3) continue;
        sum2 = sum2 + k;
    } while (k < 5);
    if (sum2 != 12) return 23;

    // --- p256: nested loop tests ---
    int cnt2 = 0;
    for (int ii = 0; ii < 5; ii = ii + 1) {
        for (int jj = 0; jj < 5; jj = jj + 1) {
            if (jj == 3) break;
            cnt2 = cnt2 + 1;
        }
    }
    if (cnt2 != 15) return 30;

    int ssum = 0;
    for (int ii = 0; ii < 3; ii = ii + 1) {
        for (int jj = 0; jj < 3; jj = jj + 1) {
            if (jj == 1) continue;
            ssum = ssum + ii + jj;
        }
    }
    if (ssum != 12) return 31;

    int accum = 0;
    for (int ii = 0; ii < 3; ii = ii + 1) {
        int jj = 3;
        while (jj > 0) { accum = accum + ii; jj = jj - 1; }
    }
    if (accum != 9) return 32;

    int grid = 0;
    for (int row = 0; row < 4; row = row + 1)
        for (int col = 0; col < 3; col = col + 1)
            grid = grid + 1;
    if (grid != 12) return 33;

    // --- p258: for edge tests ---
    int fr = for_test();
    if (fr != 0) return fr;

    return 0;
}