/* compiler: any */
/* C Feature: Variadic function declarations */
/* Expected: Parser should handle ... in function parameters */

extern int printf(const char* fmt, ...);
extern int scanf(const char* fmt, ...);
extern int snprintf(char* buf, long n, const char* fmt, ...);

int main() {
    return 0;
}