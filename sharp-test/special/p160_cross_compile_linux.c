/*
 * p160_cross_compile_linux.c — Cross-compile probe: include Linux headers on Windows.
 *
 * Tests:
 *   1. Include Linux-specific headers (unistd.h, errno.h, etc.)
 *   2. Use Linux-specific types and constants
 *   3. Compile with --target x86_64-linux-gnu on Windows
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

/* Test Linux-specific types */
void test_types(void) {
    pid_t pid = 0;
    uid_t uid = 0;
    gid_t gid = 0;
    ssize_t ss = 0;
    mode_t mode = 0;
    off_t off = 0;
    (void)pid; (void)uid; (void)gid; (void)ss; (void)mode; (void)off;
}

/* Test Linux-specific constants */
int test_constants(void) {
    int e = 0;
    e = EPERM;
    e = ENOENT;
    e = EAGAIN;
    e = EINTR;
    e = EACCES;
    e = EBUSY;
    e = EEXIST;
    (void)e;
    return 0;
}

/* Test Linux functions */
void test_linux_api(void) {
    int fd = open("/dev/null", O_RDONLY);
    if (fd >= 0) {
        close(fd);
    }
    (void)strlen;
}

int main(void) {
    test_types();
    test_constants();
    test_linux_api();
    return 0;
}
