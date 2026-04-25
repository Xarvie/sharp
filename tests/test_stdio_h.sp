/* compiler: any */
/* Test: #include <stdio.h> support + putc/fgetc/fprintf/fscanf etc. */
#include <stdio.h>

i32 main() {
    /* Test FILE pointer */
    FILE* fp = null;

    /* Test basic I/O functions */
    putc(65, stdout);

    /* Test fprintf */
    fprintf(stdout, "hello %d\n", 42);

    /* Test fscanf prototype */
    i32 val = 0;
    /* fscanf(fp, "%d", &val); */

    return 0;
}
