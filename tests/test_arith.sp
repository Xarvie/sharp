/* compiler: any */
/* TDD-Expr-2: arithmetic (+ - * / %) and comparison */

int main() {
    int a = 7;
    int b = 3;
    int c = a + b;   /* 10 */
    int d = a - b;   /* 4 */
    int e = a * b;   /* 21 */
    int f = a / b;   /* 2 */
    int g = a % b;   /* 1 */
    if (c == 10 && d == 4 && e == 21 && f == 2 && g == 1) {
        return 1;
    }
    return 0;
}