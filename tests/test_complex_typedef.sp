/* compiler: any */
/* C Declarator: Complex typedef chains and function pointer typedefs */
/* Expected: Parser should handle multiple typedef aliases and function pointers */

/* Simple function pointer typedef */
typedef void (*sighandler_t)(int);

/* Function pointer with multiple params */
typedef int (*cmp_fn)(const void*, const void*);

/* Multiple aliases on same line */
typedef int errno_t, *Perrno_t, **PPerrno_t;

/* Typedef for array type */
typedef int int_array[10];

int main() {
    return 0;
}