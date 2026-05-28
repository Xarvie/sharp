/* p86_atomic_qualifier.c
 * Phase R12: _Atomic type qualifier — C11 atomics.
 * _Atomic T is treated as T (transparent qualifier).
 * Tests both qualifier form (_Atomic T) and type-specifier form (_Atomic(T)). */

/* ── _Atomic T qualifier form ──────────────────────────────────── */
_Atomic int gx;
typedef _Atomic int atomic_int_t;
static atomic_int_t counter;
struct s { _Atomic int val; int pad; };

/* ── _Atomic(T) type-specifier form (C11 6.7.2.4) ─────────────── */
typedef _Atomic(_Bool) atomic_bool_t;
typedef _Atomic(char)  atomic_char_t;
typedef _Atomic(int)   atomic_int2_t;
typedef _Atomic(long)  atomic_long_t;

/* ── _Atomic(T) variable declaration ───────────────────────────── */
_Atomic(int)            g_atomic_int;

/* ── _Atomic(T) function return type ───────────────────────────── */
_Atomic(int) get_atomic_value(void);

/* ── _Atomic(T*) pointer ───────────────────────────────────────── */
_Atomic(int*) atomic_ptr;

/* ── _Atomic(T) in struct ──────────────────────────────────────── */
struct atomic_struct {
    _Atomic(int) a;
    int _Atomic  b;   /* _Atomic after type */
};

/* ── _Atomic(T) block scope ────────────────────────────────────── */
void test_block_scope(void) {
    _Atomic(int) local_atomic;
}

/* ── Main — exercise all forms ─────────────────────────────────── */
int main(void) {
    _Atomic int x = 5;
    gx = 3;
    counter = 7;
    struct s obj;
    obj.val = 1;

    g_atomic_int = 1;
    struct atomic_struct as;
    as.a = 2;
    as.b = 3;
    test_block_scope();

    return (x + gx + counter + obj.val + g_atomic_int + as.a + as.b == 22) ? 0 : 1;
}