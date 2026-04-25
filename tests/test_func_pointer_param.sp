/* compiler: any */
/* C Feature: Function pointer parameters in extern declarations */
/* Expected: Parser should handle function pointer types as function parameters */

extern void qsort(void* base, usize nmemb, usize size,
                  i32 (*compar)(const void*, const void*));

i32 main() {
    return 0;
}
