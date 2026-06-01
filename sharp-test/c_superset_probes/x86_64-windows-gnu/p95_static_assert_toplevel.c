/* p95_static_assert_toplevel.c (x86_64-windows-gnu variant)
 * Phase R15: _Static_assert at file scope (C11 ISO 6.7.10).
 * On Windows LLP64, unsigned long is 4 bytes. */
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
_Static_assert(1 == 1, "always true");

typedef unsigned long ulong_t;
_Static_assert(sizeof(ulong_t) == 4, "ulong is 4 bytes on LLP64");

int main(void) { return 0; }
