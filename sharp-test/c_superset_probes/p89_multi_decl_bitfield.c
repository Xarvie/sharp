/* p89_multi_decl_bitfield.c
 * Phase R12: comma-list bitfield declarations (linux/tcp.h pattern).
 * u32 a:1, b:1, c:1, d:29 — all share the same base type. */
struct flags {
    unsigned int a:1, b:1, c:1, reserved:29;
};
int main(void) {
    struct flags f = {0};
    f.a = 1; f.b = 0; f.c = 1;
    return (f.a + f.c == 2) ? 0 : 1;
}
