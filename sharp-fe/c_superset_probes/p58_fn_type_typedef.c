/* p58_fn_type_typedef.c -- K&R-style function-type typedef.
 *
 * ISO C admits two distinct typedef shapes for callable callback
 * types.  The modern form is a function-POINTER typedef
 * (typedef void parens-star-FnPtr-parens parens-int-parens semicolon).
 * The K&R-classic form is a function-TYPE typedef (no parens, no
 * star): typedef void Fn parens-int-parens semicolon.
 *
 * Both are interchangeable when used through Fn-star or FnPtr -- both
 * denote pointer-to-function.  cJSON (R1 target) used the modern
 * pointer form throughout; stb's
 *     typedef void stbi_write_func(void *, void *, int);
 * is the second form, with usage as stbi_write_func-star everywhere.
 *
 * Before the fix, parse_top_decl saw the K&R form and mis-classified
 * it as a function prototype, never registering Fn as a typedef.
 * All later uses then failed with "unknown type 'Fn'".
 */
typedef void Fn(int);

static int captured = 0;

static void cb(int v) { captured = v; }

int main(void) {
    Fn *p = cb;
    p(42);
    return captured - 42;
}
