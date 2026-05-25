/* p41 — parenthesized declarator on extern function declaration.
 *
 * C allows redundant parens around a declarator to disambiguate
 * precedence: `int (*p)(int)` for a function pointer, or `int (foo)(int)`
 * for a plain function.  Lua's public header `lua.h` uses the latter
 * defensively as a macro-evasion idiom:
 *
 *     extern lua_State *(lua_newstate) (lua_Alloc f, void *ud);
 *
 * The parser must accept `*(IDENT)(...)` as a function declarator
 * equivalent to `*IDENT(...)` and not loop or treat it as a parameter
 * list. */
typedef struct S S;
typedef int (*F)(S *s);

extern S *(my_alloc)(F f, void *ud);
extern void (my_free)(S *s);
extern int (my_count)(void);

int main(void) { return 0; }
