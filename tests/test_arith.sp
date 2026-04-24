/* TDD-Expr-2: arithmetic (+ - * / %) and comparison */

i32 main() {
    i32 a = 7;
    i32 b = 3;
    i32 c = a + b;   /* 10 */
    i32 d = a - b;   /* 4 */
    i32 e = a * b;   /* 21 */
    i32 f = a / b;   /* 2 */
    i32 g = a % b;   /* 1 */
    if (c == 10 && d == 4 && e == 21 && f == 2 && g == 1) {
        return 1;
    }
    return 0;
}
