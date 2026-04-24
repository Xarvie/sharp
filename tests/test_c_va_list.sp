/* Phase 4: va_list and inline function tests */

/* va_list as a named type (from <stdarg.h>) */
typedef void* va_list;

/* Function using va_list */
extern int vprintf(const char* format, va_list args);

/* extern inline function with body (already supported in Phase 2) */
extern __inline__ int add(int a, int b) {
    return a + b;
}

/* Usage */
i32 main() {
    va_list args;
    i32 x = add(1, 2);
    return 0;
}
