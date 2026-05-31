/*
 * test_scope_find_typedef.c — Regression test for scope_find_typedef
 * scope chain walking bug.
 *
 * Root cause: scope_find_typedef() used scope_lookup() to find the first
 * matching symbol, then iterated only within that one scope's hash bucket
 * via sym->next.  This meant if a non-typedef symbol with the target name
 * existed in a closer scope, the typedef in a parent scope was never found
 * because sym->next does not cross scope boundaries.
 *
 * Fix: reimplement scope_find_typedef() to walk the scope chain directly
 * (like scope_lookup_ex), checking each scope's bucket for a typedef
 * symbol with the given name.
 *
 * This test exercises the two cg.c call sites:
 *   1. Cast expression with typedef pointer (line ~2869)
 *   2. Function pointer / ptr-to-array return type typedef (line ~4342)
 */

/* === Case 1: typedef for function pointer return type ===
 * When a function returns a function-pointer type via a typedef,
 * cg uses scope_find_typedef to detect the alias and emit the
 * flat typedef form instead of expanding the nested declarator. */

typedef int (*IntFn)(int);

IntFn get_int_fn(void) {
    return (IntFn)0;
}

int test_fnptr_return_typedef(void) {
    IntFn fn = get_int_fn();
    (void)fn;
    return fn == 0 ? 1 : 0;
}

/* === Case 2: typedef for ptr-to-array return type ===
 * jmp_buf-style pattern: typedef for array, function returns pointer to it.
 * scope_find_typedef must find the typedef to emit the flat form. */

typedef int BufTag[1];

BufTag *get_buf(void) {
    static BufTag b;
    b[0] = 42;
    return &b;
}

int test_ptr_to_array_typedef(void) {
    BufTag *p = get_buf();
    return (*p)[0];
}

/* === Case 3: cast expression with typedef pointer ===
 * (panic_fn pattern from lua) — cast to typedef'd fn-ptr. */

typedef void (*PanicFn)(int);

static void my_panic(int code) { (void)code; }

PanicFn g_panic = (PanicFn)my_panic;

int test_cast_typedef_ptr(void) {
    return g_panic != 0 ? 1 : 0;
}

/* === Case 4: typedef chain (typedef of typedef) ===
 * Ensures scope_find_typedef walks the bucket to find the
 * first typedef even when another typedef of the same base
 * name exists later in the chain. */

typedef int BaseT;
typedef BaseT MidT;
typedef MidT TopT;

TopT get_top(void) {
    return 99;
}

int test_typedef_chain(void) {
    TopT v = get_top();
    return (int)v;
}

/* === Case 5: struct typedef in fn-ptr return === */

typedef struct { int x; int y; } Point;

typedef Point (*PointFactory)(int x, int y);

Point make_point(int x, int y) {
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

PointFactory pf = make_point;

int test_struct_typedef_fnptr_return(void) {
    Point p = pf(3, 4);
    return p.x + p.y;
}

/* === Case 6: enum typedef in fn-ptr return === */

typedef enum { RED, GREEN, BLUE } Color;

typedef Color (*ColorMixer)(Color a, Color b);

Color mix_colors(Color a, Color b) {
    (void)a;
    return b;
}

ColorMixer cm = mix_colors;

int test_enum_typedef_fnptr_return(void) {
    Color c = cm(RED, BLUE);
    return (int)c;
}

/* === Case 7: const typedef pointer in cast === */

typedef const char *CStr;

CStr g_str = (CStr)"hello";

int test_const_typedef_ptr_cast(void) {
    return g_str != 0 ? 1 : 0;
}

/* === Case 8: typedef for fn-ptr with struct ptr param in return === */

typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef Node *(*NodeFactory)(int val);

Node *make_node(int val) {
    static Node n;
    n.value = val;
    n.next = 0;
    return &n;
}

NodeFactory nf = make_node;

int test_struct_ptr_typedef_fnptr_return(void) {
    Node *n = nf(7);
    return n->value;
}

/* === Case 9: multiple typedefs with overlapping names in same bucket ===
 * Hash collisions can place different-named symbols in the same bucket.
 * scope_find_typedef must correctly skip non-matching names via strcmp
 * and still find the typedef. */

typedef long LongT;
typedef int IntT;

LongT get_long(void) {
    return 100L;
}

int test_hash_collision_typedefs(void) {
    LongT v = get_long();
    return (int)v;
}

/* === Case 10: typedef for void fn-ptr in cast (lua atpanic pattern) === */

typedef void (*Handler)(void);

static void default_handler(void) { }

Handler g_handler = (Handler)default_handler;

int test_void_fnptr_typedef_cast(void) {
    return g_handler != 0 ? 1 : 0;
}

/* === Case 11: ptr-to-array typedef with struct element === */

typedef struct { int a; int b; } Pair;

typedef Pair PairArr[2];

PairArr *get_pairs(void) {
    static PairArr arr;
    arr[0].a = 1;
    arr[0].b = 2;
    arr[1].a = 3;
    arr[1].b = 4;
    return &arr;
}

int test_struct_ptr_to_array_typedef(void) {
    PairArr *p = get_pairs();
    return (*p)[0].a + (*p)[1].b;
}

/* === Case 12: nested fn-ptr typedef (factory returning factory) === */

typedef int (*Op)(int, int);
typedef Op (*OpFactory)(const char *name);

static int add_op(int a, int b) { return a + b; }

Op make_op(const char *name) {
    (void)name;
    return add_op;
}

OpFactory of = make_op;

int test_nested_fnptr_factory(void) {
    Op op = of("add");
    return op(10, 20);
}

int main(void) {
    int r = 0;
    r += test_fnptr_return_typedef();
    r += test_ptr_to_array_typedef();
    r += test_cast_typedef_ptr();
    r += test_typedef_chain();
    r += test_struct_typedef_fnptr_return();
    r += test_enum_typedef_fnptr_return();
    r += test_const_typedef_ptr_cast();
    r += test_struct_ptr_typedef_fnptr_return();
    r += test_hash_collision_typedefs();
    r += test_void_fnptr_typedef_cast();
    r += test_struct_ptr_to_array_typedef();
    r += test_nested_fnptr_factory();
    return r > 0 ? 0 : 1;
}
