/* p68_fn_returning_fnptr.c -- function whose return type is a function pointer.
 *
 * ISO C requires this to be expressed as a *nested* declarator:
 *
 *     int (*lua_atpanic(lua_State *L, int (*panicf)(lua_State *)))
 *                      (lua_State *);
 *
 * The function name with its outer-args goes INSIDE the (*...) of
 * the function-pointer declarator.  The flat form
 *
 *     int (*)(lua_State *) lua_atpanic(...);
 *
 * is rejected by C compilers.
 *
 * Lua exports half a dozen of these (`lua_atpanic`, `lua_tocfunction`,
 * `lua_getallocf`, ...).  Pre-R3 cg's free-function emission called
 * cg_type(ret_t); that worked when ret_t was a normal type but
 * produced the broken flat form for TY_PTR(TY_FUNC).
 *
 * Fix: detect TY_PTR ending in TY_FUNC at the function-prototype /
 * definition emission point; emit `inner_ret (*` opener, then the
 * normal name + outer-args, then close with `)` + `(inner_args)`.
 */

/* a Lua-style "atpanic" — takes a fn-ptr, returns a fn-ptr */
typedef int (*panic_fn)(int);

static int default_panic(int x) { return x; }

/* The pattern under test.  Both prototype and definition shapes need
 * the nested declarator. */
panic_fn lua_atpanic(panic_fn p);

panic_fn lua_atpanic(panic_fn p) {
    if (p) return p;
    return default_panic;
}

int main(void) {
    panic_fn pf = lua_atpanic(default_panic);
    return pf(0) - 0;
}
