/* p69_fnptr_array.c -- array of function pointers needs nested declarator.
 *
 * ISO C grammar requires the declarator-nested form
 *
 *     ret (* name[N])(args)
 *
 * for arrays whose elements are function-pointers.  The flat form
 * `ret (*)(args) name[N]` (or with a stray const, `ret (*)(args)
 * const name[N]`) is rejected by C compilers.
 *
 * Lua's lualib.c registers all built-in module loaders via
 *
 *     static const lua_CFunction searchers[] = { ... };
 *
 * where lua_CFunction is `int (*)(lua_State *)`.  The const sits on
 * the pointer (the elements are immutable function pointers, not
 * pointers to const).
 *
 * Pre-R3 cg_decl array branch emitted `cg_type(elem)` then ` name[]`,
 * yielding the broken flat form.  The fix detects PTR-to-FUNC element
 * type (peeling an optional surrounding const) and switches to the
 * nested shape with `* const` placement when const is present.
 */
typedef int (*cb_fn)(int);

static int a(int x) { return x + 1; }
static int b(int x) { return x + 2; }
static int c(int x) { return x + 3; }

/* the pattern that broke */
static const cb_fn handlers[] = { a, b, c };

int main(void) {
    int sum = 0;
    for (int i = 0; i < 3; i++) sum += handlers[i](10);
    return sum - 36;     /* (10+1)+(10+2)+(10+3) = 36 */
}
