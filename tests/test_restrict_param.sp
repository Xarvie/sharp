/* compiler: any */
/* C Feature: Restrict qualifier on function parameters */
/* Expected: Parser should handle restrict keyword */

extern char* strncpy(char* restrict dest, const char* restrict src, usize n);
extern void* memcpy(void* restrict dest, const void* restrict src, usize n);

i32 main() {
    return 0;
}
