/* p94_transparent_union.c
 * Phase R14: __attribute__((transparent_union)) — GCC extension.
 * When a typedef alias has this attribute, function parameters declared
 * with that type should be emitted using the first member's base type
 * (e.g. `const struct sockaddr *`) instead of the anonymous union type,
 * preventing "incompatible type" errors in downstream C compilation. */
typedef union {
    int *as_int_ptr;
    void *as_void_ptr;
} __my_ptr __attribute__((__transparent_union__));

/* This function declaration must emit `int *` not `union __anon_N` */
extern int take_int_ptr(__my_ptr p);

int main(void) { return 0; }
