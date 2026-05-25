/*
 * p161_stdlib_integration.sp — Consumer for stdlib integration test (includes std.str, std.string).
 *
 * Tests P1-1: imports std.str and std.string
 * Tests P1-2: uses structs, methods, and functions from both
 */
/* FIXED(e2e): missing str_eq function definition in stdlib integration headers */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "p161_std_types.sph"
#include "p161_std_str.sph"
#include "p161_std_string.sph"
#include "p161_std_vec.sph"

/* ── Consumer code ───────────────────────────────────────────────── */

int main(void) {
    /* Test Str struct and methods from std.str */
    Str hello = str_from_lit("hello");
    Str world = str_from_lit("world");
    Str hello2 = str_from_lit("hello");

    /* Test Str methods */
    isize hello_len = hello.size();
    bool hello_eq = hello.eq(hello2);
    bool eq_test = str_eq(hello, world);

    printf("hello.size()=%zd\n", hello_len);
    printf("hello.eq(hello2)=%d\n", hello_eq);
    printf("str_eq(hello,world)=%d\n", eq_test);

    /* Test String struct and methods from std.string */
    String s = string_from_str(hello);
    isize s_size = s.size();
    s.push_byte('!');
    isize s_size2 = s.size();
    s.destroy();

    printf("string.size()=%zd\n", s_size);
    printf("string after push_byte=%zd\n", s_size2);

    /* Test Vec<int> struct and methods from std.vec */
    Vec_int v;
    v.data = NULL;
    v.len = 0;
    v.cap = 0;

    v.push(10);
    v.push(20);
    v.push(30);
    isize v_size = v.size();
    int v_last = v.pop();
    int v_first = v.get(0);
    v.destroy();

    printf("vec.size()=%zd vec.last=%d vec.first=%d\n", v_size, v_last, v_first);

    /* Verify expected values */
    if (hello_len != 5) return 1;
    if (!hello_eq) return 2;
    if (eq_test) return 3;
    if (s_size != 5) return 4;
    if (s_size2 != 6) return 5;
    if (v_size != 3) return 6;
    if (v_last != 30) return 7;
    if (v_first != 10) return 8;

    printf("P161_STDLIB_PASS\n");
    return 0;
}
