/* compiler: any */
/* Phase 5: errno_t type test */

/* errno_t type */
typedef int errno_t;

/* Function returning errno_t */
extern errno_t clearerr_s(void* stream);

/* Usage */
i32 main() {
    errno_t err = 0;
    return 0;
}
