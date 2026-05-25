/* p127 — multi-dimensional array parameter: fn(int m[R][C])
 * KNOWN-DIFFER: sharpc drops the inner dimension, emitting int m[R]
 * Expected: void mat ( int m [ 3 ] [ 3 ] ) { ... }
 * Actual:   void mat ( int m [ 3 ] ) { ... }
 */
void mat3x3(int m[3][3]) { (void)m; }
void mat2x4(int m[2][4]) { (void)m; }
int main(void) {
    int a[3][3] = {{0}};
    int b[2][4] = {{0}};
    mat3x3(a); mat2x4(b);
    return 0;
}
