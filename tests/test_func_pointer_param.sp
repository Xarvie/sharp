/* compiler: any */
/* C Feature: Function pointer parameters in extern declarations */
/* Expected: Parser should handle function pointer types as function parameters */

extern void qsort(void* base, long nmemb, long size,
                  int (*compar)(const void*, const void*));

int main() {
    return 0;
}