/* compiler: any */
/* C Feature: Complex typedef chains and function pointer typedefs */
/* Expected: Parser should handle multiple typedef aliases and function pointers */

typedef i32 errno_t, *Perrno_t, **PPerrno_t;
typedef void (*sighandler_t)(i32);
typedef i32 (*cmp_fn)(const void*, const void*);

i32 main() {
    return 0;
}
