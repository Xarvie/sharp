/* p61_typedef_fnptr_field.c -- typedef-name as return type of a struct
 *                              field whose declarator is a function-pointer.
 *
 * sqlite3.c declares many vfs / io-method tables with this shape:
 *
 *     sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs *, const char *);
 *
 * The leading IDENT is a typedef name; the declarator in parentheses
 * is `(*field_name)` followed by a function parameter list.  Before
 * the fix, is_type_start saw `IDENT (` and didn't recognize the
 * combination as a type-start, so the entire field declaration was
 * skipped with "unexpected token in struct body".
 *
 * The fix gates `IDENT (` on the IDENT being a known typedef-name,
 * mirroring the existing IDENT-) and the new IDENT-const treatments.
 */
typedef int my_int_t;

struct S {
    my_int_t (*fn)(int);     /* the pattern that broke */
    int x;
};

static int double_it(int v) { return v * 2; }

int main(void) {
    struct S s;
    s.fn = double_it;
    s.x  = 21;
    return s.fn(s.x) - 42;
}
