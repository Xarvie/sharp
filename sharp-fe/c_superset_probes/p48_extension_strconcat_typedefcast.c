/* p48 — Three small ISO-C / GCC idioms that block real-world headers
 * if Sharp doesn't accept them:
 *
 *   (1) `__extension__` — GCC's "next construct is non-ISO, don't warn"
 *       prefix.  glibc uses it on `typedef struct { long long ... } X;`
 *       definitions so older C89 compilers don't complain about the
 *       `long long`.  Sharp drops the keyword silently in the lexer.
 *
 *   (2) Adjacent string-literal concatenation (ISO C phase 6):
 *           const char *s = "Hello, " "World" "!";
 *       Lua's `LUA_VERSION` macro expands to a chain of these.
 *       Sharp's lexer fuses neighbours into a single STOK_STRING_LIT.
 *
 *   (3) Cast through a user typedef name with a parenthesised target:
 *           (my_int)p
 *       The cast vs paren-expr ambiguity is resolved by checking the
 *       Sharp typedef-name set populated during parsing.
 */

__extension__ typedef struct {
    long long quot;
    long long rem;
} my_lldiv_t;

typedef int my_int;

static const char *banner = "Sharp " "compiler " "v" "1";

int main(void) {
    my_lldiv_t r = { 7LL, 0LL };
    int        p = -3;
    my_int     v = (my_int)p;        /* idiom (3): typedef cast       */

    return (banner[0] != 'S')
         + (int)(r.quot - 7)
         + (int)r.rem
         + (v + 3);
}
