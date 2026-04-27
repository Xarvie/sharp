/* compiler: any */
/* TDD-Stmt-5: break / continue */

int main() {
    int sum = 0;
    int i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 3) continue;
        if (i == 6) break;
        sum = sum + i;
    }
    return sum; /* 1+2+4+5 = 12 */
}