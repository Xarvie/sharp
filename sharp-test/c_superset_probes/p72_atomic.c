/* p72_atomic.c — GCC __atomic_* builtins.
 *
 * GCC exposes C11 atomic operations as __atomic_load_n, __atomic_store_n,
 * __atomic_compare_exchange_n, etc.  These look like normal function calls
 * (no type arguments), so the parser difficulty is limited.  The main
 * issue is whether the builtin names are recognised as valid identifiers
 * and whether the calls type-check.
 *
 * Probe: store 42, load it back, check CAS semantics.
 * Uses __ATOMIC_SEQ_CST (value = 5, defined by GCC).
 */
int main(void) {
    int val = 0;
    __atomic_store_n(&val, 42, __ATOMIC_SEQ_CST);
    int loaded = __atomic_load_n(&val, __ATOMIC_SEQ_CST);
    if (loaded != 42) return 1;
    int expected = 42;
    __atomic_compare_exchange_n(&val, &expected, 100, 0,
                                __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    loaded = __atomic_load_n(&val, __ATOMIC_SEQ_CST);
    return loaded == 100 ? 0 : 1;
}
