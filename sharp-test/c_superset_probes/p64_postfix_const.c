/* p64_postfix_const.c -- ISO C99 postfix type qualifier `T const`.
 *
 * Per C99 6.7.3, type qualifiers may appear before OR after the type
 * specifier they apply to:
 *
 *     const int *p;     // canonical
 *     int const *p;     // equivalent; commonly used in older code
 *     unsigned char const *q;
 *
 * Two parse paths needed support: parse_type (used by casts,
 * sizeof, generic-args) and is_type_start (gates whether
 * `(unsigned char const *)x` is a cast or an expression).
 * Both were prefix-only pre-R3 for IDENT bases.  sqlite3.c uses
 * `sqlite3_io_methods const *` and similar throughout.
 */
int main(void) {
    int x = 7;
    /* postfix const in a cast — the form that broke */
    int const *p = (int const *)&x;
    /* postfix const through unsigned compound, also via cast */
    unsigned char y = 'a';
    unsigned char const *q = (unsigned char const *)&y;

    return (*p == 7 && *q == 'a') ? 0 : 1;
}
