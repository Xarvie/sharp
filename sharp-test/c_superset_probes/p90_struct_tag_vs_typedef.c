/* p90_struct_tag_vs_typedef.c
 * Phase R13: struct tag and non-struct typedef with the same name.
 * C allows `typedef enum {...} Foo` and `struct Foo {...}` to coexist in
 * separate namespaces.  `struct Foo ts` must resolve to the struct,
 * while bare `Foo` in a field type resolves to the enum/int typedef. */
typedef enum { STATE_A = 0, STATE_B = 1 } my_state;

struct my_state {
    int x;
    int y;
    my_state tag;   /* field: uses the enum typedef → int */
};

int main(void) {
    struct my_state s;
    s.x = 10;
    s.y = 20;
    s.tag = STATE_B;
    return (s.x + s.y == 30 && s.tag == STATE_B) ? 0 : 1;
}
