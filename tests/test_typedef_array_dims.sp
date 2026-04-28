/* compiler: any */
/* Test: Typedef array dimension preservation — verifies that parse_declarator
 * correctly preserves array dimensions in typedef declarations instead of
 * incorrectly decaying them to pointers. */

/* Simple fixed-size array typedef */
typedef int int_array[10];

/* Pointer array — array of pointers */
typedef char* str_arr[5];

/* Array typedef used in variable declaration */
typedef double vec3[3];

int main() {
    int_array a;
    str_arr s;
    vec3 v;
    return 0;
}
