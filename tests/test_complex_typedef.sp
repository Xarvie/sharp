/* compiler: any */
/* C Declarator: Complex typedef chains and function pointer typedefs */
/* Expected: Parser should handle multiple typedef aliases and function pointers */

/* Simple function pointer typedef */
typedef void (*sighandler_t)(i32);

/* Function pointer with multiple params */
typedef i32 (*cmp_fn)(const void*, const void*);

/* Multiple aliases on same line */
typedef i32 errno_t, *Perrno_t, **PPerrno_t;

/* Typedef for array type */
typedef i32 int_array[10];

i32 main() {
    return 0;
}
