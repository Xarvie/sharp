/* compiler: any */
/* C Feature: Variadic function declarations */
/* Expected: Parser should handle ... in function parameters */

extern i32 printf(const char* fmt, ...);
extern i32 scanf(const char* fmt, ...);
extern i32 snprintf(char* buf, usize n, const char* fmt, ...);

i32 main() {
    return 0;
}
