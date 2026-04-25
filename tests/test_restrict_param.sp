/* compiler: any */
/* C Declarator: restrict qualifier on function parameters */
/* Expected: Parser should handle restrict keyword in parameter lists */

extern char* strncpy(char* restrict dest, const char* restrict src, usize n);
extern void* memcpy(void* restrict dest, const void* restrict src, usize n);
extern void* memmove(void* dest, const void* src, usize n);

i32 main() {
    return 0;
}
