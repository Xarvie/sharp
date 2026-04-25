/* compiler: gcc */
/* C Feature: Complex __attribute__ on variables */
/* Expected: Parser should handle __attribute__((aligned(16), unused)) */

i32 x __attribute__((aligned(16), unused));
i32 y __attribute__((packed));
void* z __attribute__((malloc));

i32 main() {
    return 0;
}
