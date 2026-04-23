/* Test: #include <stdio.h> and call puts() */

#include <stdio.h>

/* puts() takes const u8* and returns i32. */
extern i32 puts(const u8* s);

fn main() {
    puts("hello");
}
