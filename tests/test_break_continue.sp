/* compiler: any */
/* TDD-Stmt-5: break / continue */

i32 main() {
    i32 sum = 0;
    i32 i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 3) continue;
        if (i == 6) break;
        sum = sum + i;
    }
    return sum; /* 1+2+4+5 = 12 */
}
