/* p132 — __attribute__ immediately after struct/union keyword
 * KNOWN-ERROR: sharpc expects struct body '{' after tag name,
 * but __attribute__((packed)) etc. can appear between 'struct' and '{'.
 * Expected: struct __attribute__ ( ( packed ) ) ps { char a ; int b ; } ;
 * Actual:   sharpc parse error
 */
struct __attribute__((packed)) packed_s { char a; int b; char c; };
union __attribute__((aligned(8))) aligned_u { int i; double d; };
int main(void) {
    struct packed_s s;
    (void)s;
    return sizeof(struct packed_s) < sizeof(int) * 3 ? 0 : 1;
}
