/* compiler: any */
/* C Feature: Function pointer declarations */
/* Expected: Parser should handle int (*fp)(int, int) syntax */

i32 main() {
    i32 (*fp)(i32, i32) = null;
    i32 result = (*fp)(1, 2);
    return result;
}
