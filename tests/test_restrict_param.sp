/* compiler: any */
/* C Declarator: restrict qualifier on function parameters */
/* Expected: Parser should handle restrict keyword in parameter lists */

extern char* strncpy(char* restrict dest, const char* restrict src, long n);
extern void* memcpy(void* restrict dest, const void* restrict src, long n);
extern void* memmove(void* dest, const void* src, long n);

int main() {
    return 0;
}