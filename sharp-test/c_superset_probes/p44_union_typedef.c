/* p44 — `typedef union Tag { ... } Alias;` (and the self-referential
 * `typedef union Tag Tag;` shape that appears throughout libc / Lua
 * headers).
 *
 * Sharp's parser must:
 *   (a) accept `union Tag { ... }` as a tagged inline definition in any
 *       decl-specifier position (was only handled at top level before),
 *   (b) preserve the `union` keyword (vs. `struct`) through to cg —
 *       the self-referential typedef emission previously hard-coded
 *       `struct`, which broke `typedef union Tag Tag;` round-trips.
 *
 * Lua's `lobject.h` defines its tagged-value `Value` union exactly this
 * way; until both fixes were in place sharpc could not even parse
 * `lapi.c`. */

typedef union Slot {
    int   i;
    long  l;
    void *p;
} Slot;

union Outer {
    int x;
    Slot s;
};

typedef union Outer OuterAlias;

int main(void) {
    Slot s; s.i = 7;
    OuterAlias o; o.x = 9;
    return (s.i - 7) + (o.x - 9);
}
