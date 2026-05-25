/* p82_nested_fnptr_decl.c
 * Phase R10: doubly-nested function-pointer variable declaration and cast.
 * void (*(*x)(void*,const char*))(void) — pointer to function taking
 * (void*,const char*) and returning pointer to function taking (void). */
static void inner_fn(void) {}

static void (*outer_factory(void *p, const char *s))(void) {
    (void)p; (void)s;
    return inner_fn;
}

int main(void) {
    void (*(*x)(void*, const char*))(void);
    x = outer_factory;
    void (*fn)(void) = x((void*)0, "test");
    fn();
    return 0;
}
