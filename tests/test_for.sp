/* compiler: any */
/* TDD-Stmt-4: for loop */

int main() {
    int sum = 0;
    for (int i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }
    return sum; /* 0+1+2+3+4 = 10 */
}