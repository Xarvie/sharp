/* compiler: any */
/* TDD-Stmt-2: while loop */

i32 main() {
    i32 sum = 0;
    i32 i = 0;
    while (i < 5) {
        sum = sum + i;
        i = i + 1;
    }
    return sum; /* 0+1+2+3+4 = 10 */
}
