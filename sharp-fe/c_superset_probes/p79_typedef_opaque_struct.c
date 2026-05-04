/* p79_typedef_opaque_struct.c
 * Phase R8: `typedef struct FooStruct Foo;` where FooStruct has no body
 * in the current translation unit (opaque forward pointer pattern).
 * This is used throughout Brotli's public API headers.  sharpc must
 * register the struct tag so that pointer-to-opaque works. */
typedef struct OpaqueStruct Opaque;
/* Use the opaque pointer — must not produce "unknown type" */
Opaque *make(void *p) { return (Opaque *)p; }
int     uses(Opaque *p) { return p != (void*)0; }
int main(void) {
    /* Can take the address and pass through, just not dereference. */
    int x = 99;
    Opaque *op = make(&x);
    return uses(op) ? 0 : 1;
}
