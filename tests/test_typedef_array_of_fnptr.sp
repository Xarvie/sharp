/* compiler: any */
/* Test: Array of function pointers — verifies that parse_declarator correctly
 * handles the most complex declarator: an array of function pointers.
 * Grammar: ret (*name[N])(params) — both FUNC and ARRAY modifiers. */

/* Simple handler array */
typedef void (*handler_arr[8])(int);

/* Multiple param types */
typedef int (*callback_arr[4])(const char*, int, void*);

/* Function pointer typedef (reference, already tested but included for completeness) */
typedef double (*math_fn)(double);

int main() {
    handler_arr handlers;
    callback_arr callbacks;
    return 0;
}
