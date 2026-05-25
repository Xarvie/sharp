/* p63_attribute_stmt.c -- bare attribute spec as a statement.
 *
 *     switch (x) {
 *         case 0: x++; __attribute__((fallthrough));
 *         case 1: x++; break;
 *     }
 *
 * GCC's fallthrough marker is emitted as a statement-position
 * attribute (no following declaration).  C23 standardised this as
 * `[[fallthrough]];`; the GCC spelling remains in heavy use.
 * sqlite3.c uses it throughout the bytecode VM dispatch.
 *
 * Fix: parse_stmt accepts STOK_ATTRIBUTE followed by `;` as a
 * no-op statement (returns NULL; the block walker filters NULL).
 */
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int x = 0;
    switch (x) {
        case 0:
            x++;
            __attribute__((fallthrough));
        case 1:
            x++;
            break;
    }
    return x - 2;
}
