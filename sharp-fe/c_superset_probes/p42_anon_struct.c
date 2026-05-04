/* p42 — anonymous struct in typedef.
 *
 * `typedef struct { fields... } Name;` is the standard C idiom for
 * defining a named type whose underlying struct has no separate tag.
 * It appears throughout system headers — gcc's <stddef.h> defines
 * max_align_t this way, and most third-party C libraries follow the
 * same pattern (FILE on some platforms, divx_quot_t etc.).
 *
 * The decl-specifier path must accept `struct` followed by `{` as an
 * anonymous-struct body and produce a usable type, not an error. */

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    long long whole;
    int       fract;
} Money;

int main(void) {
    Point p = { 3, 4 };
    Money m = { 100, 50 };
    return (p.x + p.y - 7) + (int)((m.whole + m.fract) - 150);
}
