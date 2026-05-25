/* p120 — function pointer to function returning pointer to array
 * KNOWN-DIFFER: sharpc drops the outer [N] in the return type
 * Expected: int (*(*fp)(void))[4];
 * Actual:   int *(*fp)(void);
 */
int (*(*fp)(void))[4];
int (*(*get_handler(int))(int))[8];
int main(void) { (void)fp; return 0; }
