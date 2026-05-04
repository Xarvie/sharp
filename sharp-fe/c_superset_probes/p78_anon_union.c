/* p78_anon_union.c
 * Phase R8: anonymous union/struct member injection (ISO C11 §6.7.2.1¶15).
 * Members of an anonymous aggregate are accessible as direct members of
 * the containing struct.  Brotli's ZopfliCostModel uses this pattern. */
typedef struct Inner { int a; float b; } Inner;
typedef struct Outer {
    int before;
    union {
        int   i_val;
        float f_val;
        Inner inner;
    };
    int after;
} Outer;

int main(void) {
    Outer o;
    o.before = 10;
    o.i_val  = 42;        /* anonymous union member */
    o.after  = 20;
    if (o.before != 10)  return 1;
    if (o.i_val  != 42)  return 2;
    if (o.after  != 20)  return 3;
    o.inner.a = 7;
    if (o.inner.a != 7)  return 4;
    return 0;
}
