/* compiler: any */
/* C Declarator: Function pointer variable declaration */
/* Expected: Parser should handle int (*fp)(int, int) syntax */

i32 main() {
    i32 (*fp)(i32, i32) = null;
    return 0;
}
