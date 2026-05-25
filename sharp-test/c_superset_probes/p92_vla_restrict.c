/* p92_vla_restrict.c
 * Phase R14: C99 VLA/parameter-array qualifiers inside [...].
 * `void f(char *v[restrict])` — __restrict / restrict / static inside
 * array subscript are type qualifiers, not size expressions. */
void copy(char *dst[restrict], const char *src[restrict], int n) {
    (void)dst; (void)src; (void)n;
}
void sized(int buf[static 10]) { (void)buf; }
int main(void) { return 0; }
