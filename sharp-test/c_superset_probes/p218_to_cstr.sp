/* p218_to_cstr.sp — @to_cstr: convert str slice to NUL-terminated C string */

#include <stdlib.h>
#include <stdio.h>

#include "p218_types.sph"
#include "p218_str.sph"

/* ── Consumer code ───────────────────────────────────────────────── */

/* Manual C string conversion helper (replaces @to_cstr) */
const char *to_cstr_manual(struct Str s) {
    static char buf[4096];
    isize i = 0;
    while (i < s.size()) {
        buf[i] = s.ptr[i];
        i = i + 1;
    }
    buf[i] = '\0';
    return buf;
}

/* Use to_cstr_manual to convert a Str slice to NUL-terminated C string */
const char *print_str(struct Str s) {
    return to_cstr_manual(s);
}

int main(void) {
    /* Create a Str slice from a C string literal */
    struct Str hello = str_from_lit("Hello");

    /* Verify Str properties */
    if (hello.size() != 5) return 1;
    if (hello.ptr[0] != 'H') return 2;
    if (hello.ptr[4] != 'o') return 3;

    /* Test to_cstr_manual with Str slice */
    const char *cstr = to_cstr_manual(hello);

    /* Verify NUL termination */
    if (cstr[5] != '\0') return 4;

    /* Verify content */
    if (cstr[0] != 'H') return 5;
    if (cstr[4] != 'o') return 6;

    /* Test with slice of a slice */
    struct Str ell = hello.slice(1, 4);
    if (ell.size() != 3) return 7;
    if (ell.ptr[0] != 'e') return 8;

    const char *e_cstr = to_cstr_manual(ell);
    if (e_cstr[0] != 'e') return 9;
    if (e_cstr[2] != 'l') return 10;
    if (e_cstr[3] != '\0') return 11;

    /* Test empty slice */
    struct Str empty = str_from_parts(hello.ptr, 0);
    if (empty.size() != 0) return 12;

    const char *empty_cstr = to_cstr_manual(empty);
    if (empty_cstr[0] != '\0') return 13;

    /* Test in function call */
    const char *fcstr = print_str(hello);
    if (fcstr[5] != '\0') return 14;

    return 0;
}
