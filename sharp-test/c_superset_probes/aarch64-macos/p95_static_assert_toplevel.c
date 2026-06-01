/* p95_static_assert_toplevel.c (aarch64-macos variant)
 * On macOS LP64, unsigned long is 8 bytes. */
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
_Static_assert(1 == 1, "always true");

typedef unsigned long ulong_t;
_Static_assert(sizeof(ulong_t) == 8, "ulong is 8 bytes on LP64");

int main(void) { return 0; }
