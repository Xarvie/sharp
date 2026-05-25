/* p128 — function pointer array parameter: fn(void(*fns[])(void))
 * KNOWN-DIFFER: sharpc reverses the declarator order
 * Expected: void dispatch ( void ( * fns [ ] ) ( void ) , int n )
 * Actual:   void dispatch ( void ( * ) ( void ) fns [ ] , int n )
 */
void dispatch(void(*fns[])(void), int n) {
    for (int i = 0; i < n; i++) if (fns[i]) fns[i]();
}
int main(void) {
    void(*a[])(void) = {0, 0};
    dispatch(a, 2);
    return 0;
}
