/*
 * p279_fwd_struct_ptr_typedef.c — Forward-declared struct pointer typedef.
 *
 * Regression test for: scope.c fails to register forward-declared struct tags
 * when the typedef target is wrapped in AST_TYPE_PTR.
 * e.g. typedef struct Foo * FooPtr;
 * The inner struct tag "Foo" must be forward-registered as SYM_TYPE so that
 * subsequent uses of FooPtr resolve correctly.
 */
typedef struct ForwardDecl * ForwardPtr;

ForwardPtr p1 = (ForwardPtr)0;

int main(void) {
    ForwardPtr p2 = p1;
    (void)p2;
    return 0;
}