/* TDD-Stmt-4: for loop */

i32 main() {
    i32 sum = 0;
    for (i32 i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }
    return sum; /* 0+1+2+3+4 = 10 */
}
