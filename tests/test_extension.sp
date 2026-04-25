/* compiler: gcc */
/* C Feature: __extension__ keyword */
/* Expected: Parser should handle __extension__ before declarations */

i32 main() {
    __extension__ i64 x = 10000000000;
    return 0;
}
