/* p84_vararg_fnptr_type.c
 * Phase R10: variadic function pointer types emit `...` not `void`.
 * `char *(*fn)(const char *,...)` must compile and be callable. */
typedef char *(*PrintFn)(const char *, ...);
static char g_buf[32];
static char *my_fmt(const char *s, ...) {
    int i = 0;
    while (s[i] && i < 31) { g_buf[i] = s[i]; i++; }
    g_buf[i] = 0;
    return g_buf;
}
int main(void) {
    PrintFn fn = my_fmt;
    char *r = fn("ok");
    return (r[0] == 'o' && r[1] == 'k') ? 0 : 1;
}
