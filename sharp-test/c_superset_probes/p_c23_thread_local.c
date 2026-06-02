/* C23 thread_local keyword — must be parsed as storage class specifier.
 * Tests: thread_local before type in variable declarations (global only). */

thread_local int tl_global;
thread_local _Atomic int tl_atomic;

int main(void) {
    return tl_global;
}
