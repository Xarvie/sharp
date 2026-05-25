#include <string.h>
#include <stdio.h>
int test(void) {
    char *s = strndup("hello", 3);
    puts(s);
    return 0;
}