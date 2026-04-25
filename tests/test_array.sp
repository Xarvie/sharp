/* compiler: any */
/* TDD-Expr-3: array declaration and indexing */

i32 main() {
    i32 arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    return arr[0] + arr[3]; /* 5 */
}
