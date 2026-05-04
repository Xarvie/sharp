/* p62_anon_bitfield.c -- ISO C99 anonymous bit-field for padding.
 *
 *     struct S { int :32; ... };
 *
 * Per C99 6.7.2.1 paragraph 12, a bit-field declaration with no
 * declarator (just a colon and a width) is permitted and reserves
 * the specified number of bits without making them addressable by
 * name.  sqlite3.c uses this for reserved struct padding.
 *
 * Pre-R3 sharp-fe rejected with "field declaration is missing a
 * name", and even if accepted, multiple `int :N;` fields would
 * collide in the struct scope under the empty-string name.  The
 * fix is two-step: parse synthesises an empty name when the
 * declarator is absent and the next token is a colon, and
 * scope.build_struct skips registration of empty-name fields.
 */
struct S {
    int x;
    int :16;        /* anonymous padding */
    int :8;
    int y;
    int :32;        /* and another */
};

int main(void) {
    struct S s;
    s.x = 7;
    s.y = 11;
    return s.x + s.y - 18;
}
