/* p98_typeof_expr.c
 * Phase R15: __typeof__(expr) used as a cast or sizeof argument.
 * In expression context, __typeof__ can appear as a type-like primary. */
int arr[5];
int *ptr = arr;
/* __typeof__ as argument to sizeof — valid C extension */
int sz_arr = sizeof(__typeof__(arr));
int sz_ptr = sizeof(__typeof__(ptr));
int main(void) { return sz_arr > sz_ptr ? 0 : 1; }
