/* p55_thread_local.c — C11 _Thread_local storage-class.
 *
 * Per ISO C11 6.7.1, `_Thread_local` is a storage-class specifier
 * orthogonal to `static` / `extern` (it may combine with either, but
 * not with `auto` / `register` / `typedef`).  Without a sharp-fe
 * carrier flag for it, parse rejects the keyword with "missing type
 * specifier" and the entire declaration is dropped on the floor.
 *
 * stb_image's `static _Thread_local const char *stbi__g_failure_reason;`
 * is the canonical real-world case — without this fix the entire
 * stb_image translation unit fails to parse.
 *
 * Single-threaded test: use the variable as a normal global.  The
 * thread-local semantics are architectural (the C compiler emits real
 * TLS storage); from sharp-fe's perspective the keyword must
 * round-trip into the generated C verbatim.
 */
static _Thread_local const char *err_reason = "no error";

int main(void) {
    /* compare first byte of the literal: 'n' == 0x6E == 110 */
    return err_reason[0] == 'n' ? 0 : 1;
}
