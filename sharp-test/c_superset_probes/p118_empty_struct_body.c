/* p118 — empty struct body: struct S {}
 * KNOWN-DIFFER: sharpc drops the {} and emits a forward decl
 * Expected: struct empty { };
 * Actual:   struct empty;
 */
struct empty {};
struct also_empty { };
int main(void) {
    (void)sizeof(struct empty);
    (void)sizeof(struct also_empty);
    return 0;
}
