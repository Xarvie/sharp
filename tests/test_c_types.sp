/* Phase 1: C type extension tests */

/* __int64 */
typedef __int64 my_int64;

/* wchar_t */
typedef wchar_t my_wchar;

/* __time32_t / __time64_t */
typedef long __time32_t;
typedef long long __time64_t;

/* Usage */
i32 main() {
    my_int64 a = 0;
    my_wchar wc = 0;
    __time64_t t = 0;
    return 0;
}
