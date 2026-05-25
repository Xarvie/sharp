/* p95_static_assert_toplevel.c
 * Phase R15: _Static_assert at file scope (C11 ISO 6.7.10).
 * mbedtls uses MBEDTLS_STATIC_ASSERT which expands to static_assert
 * which expands to _Static_assert at top level. */
_Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
_Static_assert(1 == 1, "always true");

typedef unsigned long ulong_t;
_Static_assert(sizeof(ulong_t) == 8, "ulong is 8 bytes on x86_64");

int main(void) { return 0; }
