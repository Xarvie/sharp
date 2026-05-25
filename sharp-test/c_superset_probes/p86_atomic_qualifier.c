/* p86_atomic_qualifier.c
 * Phase R12: _Atomic type qualifier — C11 atomics.
 * _Atomic T is treated as T (transparent qualifier). */
_Atomic int gx;
typedef _Atomic int atomic_int_t;
static atomic_int_t counter;
struct s { _Atomic int val; int pad; };
int main(void) {
    _Atomic int x = 5;
    gx = 3;
    counter = 7;
    struct s obj;
    obj.val = 1;
    return (x + gx + counter + obj.val == 16) ? 0 : 1;
}
