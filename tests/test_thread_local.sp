/* compiler: msvc */
/* C Feature: Thread-local storage */
/* Expected: Parser should handle __declspec(thread) and __thread */

__declspec(thread) i32 tls_var1;

i32 main() {
    __thread i32 tls_var2 = 0;
    return tls_var1 + tls_var2;
}
