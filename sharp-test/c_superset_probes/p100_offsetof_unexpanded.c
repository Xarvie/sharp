/* p100 — offsetof() used without #include (gperf pattern).
 * When a file has no #include, offsetof is not expanded by CPP and
 * reaches the parser as a bare identifier; sharpc treats it like
 * __builtin_offsetof and collects the balanced-paren payload verbatim. */
#include <stddef.h>
struct Point { int x; int y; };
/* offsetof(struct Point, y) must equal sizeof(int) == 4 on all targets */
int main(void) {
    int off = (int)offsetof(struct Point, y);
    return off == (int)sizeof(int) ? 0 : 1;
}
