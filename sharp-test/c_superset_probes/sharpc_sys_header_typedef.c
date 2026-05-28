/* sharpc_sys_header_typedef.c
 * Minimal reproducer: sharpc parser does not register typedefs from
 * system headers, causing "unknown type" errors when those typedef
 * names are used in user code.
 *
 * __kernel_rwf_t is typedef'd as int in <linux/fs.h> (via CPP).
 * After #include, the typedef should be visible to subsequent code. */

#include <linux/fs.h>

int main(void) {
    __kernel_rwf_t flags = 0;
    return (int)flags;
}