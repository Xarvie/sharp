/*
 * Regression test: typedef alias must NOT expand the underlying struct body.
 * Bug: when a struct is defined as `typedef struct X { body } X;` and then
 * aliased as `typedef X Y;`, sharpc was incorrectly expanding the struct body.
 */

/* This sets from_inline_typedef=true on the struct definition */
typedef struct Base {
    int x;
    double y;
} Base;

/* This should emit "typedef Base Alias1;" NOT "typedef struct Base { int x; double y; } Alias1;" */
typedef Base Alias1;

/* And another level */
typedef Alias1 Alias2;

static void use_alias1(Alias1 *a) { (void)a; }
static void use_alias2(Alias2 *a) { (void)a; }

int main(void) {
    Alias1 a1;
    Alias2 a2;
    use_alias1(&a1);
    use_alias2(&a2);
    return 0;
}
