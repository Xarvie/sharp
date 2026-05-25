/* p117 — pointer-to-array typedef: typedef T (*name)[N]
 * KNOWN-DIFFER: sharpc drops the [N] suffix, emitting typedef T *name
 * Expected: typedef char (*pa)[256];
 * Actual:   typedef char *pa;
 */
typedef char (*pa)[256];
pa get_buf(void) { return 0; }
typedef int (*matrix_row)[4];
matrix_row get_row(void) { return 0; }
int main(void) { return 0; }
