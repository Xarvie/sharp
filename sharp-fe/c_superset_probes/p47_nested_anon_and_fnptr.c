/* p47 — Nested anonymous struct/union as a struct/union field, plus
 * function-pointer-as-field declarators.
 *
 * Three real-world C idioms in one probe:
 *
 *   (1) Anonymous struct nested inside a union, with a member name:
 *           union U { int v; struct { int x, y; } pt; };
 *       Sharp parses the inner `struct { ... }` via parse_type's
 *       inline-struct path; the synthesised tag is queued to
 *       pending_decls so it surfaces at file scope.
 *
 *   (2) Anonymous union nested inside a struct, with a member name:
 *           struct S { int kind; union { int i; double d; } v; };
 *       Mirror image of (1); used by glibc's <bits/mbstate_t.h> and
 *       Lua's lobject.h tagged-value union.
 *
 *   (3) Function-pointer field with paren-around-name declarator:
 *           struct Vtbl { void (*step)(int); int (*query)(void); };
 *       glibc's <bits/types/sigevent_t.h> declares its _sigev_thread
 *       struct's _function field exactly this way.  Sharp's struct
 *       body field path now uses parse_declarator so this parses
 *       correctly.
 */

union U {
    int v;
    struct { int x, y; } pt;
};

struct S {
    int kind;
    union { int i; double d; } v;
};

struct Vtbl {
    void (*step)(int);
    int  (*query)(void);
};

static void v_step(int x)        { (void)x; }
static int  v_query(void)        { return 42; }

int main(void) {
    union U u = { .v = 1 };
    u.pt.x = 3; u.pt.y = 4;

    struct S s; s.kind = 0; s.v.i = 7;

    struct Vtbl vt = { v_step, v_query };
    vt.step(0);

    return (u.pt.x + u.pt.y - 7)
         + (s.v.i - 7)
         + (vt.query() - 42);
}
