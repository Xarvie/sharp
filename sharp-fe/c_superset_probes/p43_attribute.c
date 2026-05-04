/* p43 — GCC __attribute__((...)) silently accepted in every position
 * it appears in real C headers.
 *
 * Sharp parses and discards attributes; the C compiler re-derives them
 * when it processes the cleaned C output.  Coverage:
 *   - leading attribute on a top-level decl
 *   - attribute on a function prototype (after the parameter list)
 *   - attribute on a struct field (between name and `;`)
 *   - attribute after a declarator on a variable
 *   - multiple back-to-back attributes
 */

__attribute__((unused)) static int leading_attr = 7;

extern int proto_attr(int x) __attribute__((const));

struct WithField {
    int  a;
    long long b __attribute__((aligned(16)));
    char c __attribute__((deprecated)) __attribute__((unused));
};

static int trailing_attr_var __attribute__((unused)) = 11;

int proto_attr(int x) { return x + 1; }

int main(void) {
    struct WithField w;
    w.a = 1;
    w.b = 2;
    w.c = 3;
    return (proto_attr(2) - 3)
         + (leading_attr - 7)
         + (trailing_attr_var - 11)
         + (w.a + (int)w.b + w.c - 6);
}
