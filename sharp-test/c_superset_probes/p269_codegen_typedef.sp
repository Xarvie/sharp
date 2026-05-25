/* p269 — Codegen: typedef resolution chain
 *
 * Verifies that typedef chains are properly resolved in generated C.
 */

typedef int MyInt;
typedef MyInt YourInt;
typedef YourInt OurInt;

int get_max() { return 2147483647; }

int main() {
    OurInt a = 100;
    MyInt  b = 200;
    int    c = a + b;
    if (c != 300) return 1;
    if (get_max() != 2147483647) return 2;
    return 0;
}