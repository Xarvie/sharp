/* p272 — Codegen: enums and named constants
 *
 * Verifies that enums generate correct integer constants in C code.
 */

enum Status {
    OK,
    WARN,
    ERR,
    FATAL = 99
};

int get_status_val(enum Status s) {
    if (s == OK)    return 0;
    if (s == WARN)  return 1;
    if (s == ERR)   return 2;
    if (s == FATAL) return 99;
    return -1;
}

int main() {
    if (get_status_val(OK) != 0)    return 1;
    if (get_status_val(WARN) != 1)  return 2;
    if (get_status_val(ERR) != 2)   return 3;
    if (get_status_val(FATAL) != 99) return 4;

    enum Status s = WARN;
    if (s != WARN) return 5;

    return 0;
}