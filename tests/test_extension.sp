/* compiler: gcc */
/* C Feature: __extension__ keyword */
/* Expected: Parser should handle __extension__ before declarations */

int main() {
    __extension__ long long x = 10000000000;
    return 0;
}