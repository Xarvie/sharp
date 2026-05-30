/*
 * test_cast_typedef_corruption.c — Bug: cast expression using function
 * pointer typedef causes cg to emit typedefs in wrong dependency order.
 *
 * Root cause: type_refs_name() in cg.c did not handle AST_PARAM_DECL
 * nodes inside AST_TYPE_FUNC.params.  When checking if a typedef target
 * references another typedef name, it iterated over the params array
 * and called type_refs_name() on each element.  But each element is an
 * AST_PARAM_DECL node, and type_refs_name() had no case for it, falling
 * through to the default "return false" path.  This meant that typedef
 * references inside function pointer parameter lists were invisible to
 * the dependency tracker, causing cg to emit typedefs before their
 * dependencies (e.g. MyFP1 before MyRect).
 *
 * Fix: in type_refs_name()'s AST_TYPE_FUNC case, extract the type from
 * AST_PARAM_DECL nodes before recursing.
 */

typedef unsigned int Uint32;
typedef int Bool;

typedef struct MyRect {
    int x, y;
    int w, h;
} MyRect;

/* === Case 1: Original bug — fn-ptr typedef with struct tag + typedef in params === */
typedef Bool (*MyFP1)(struct MySurface *src, const MyRect *srcrect,
                       struct MySurface *dst, const MyRect *dstrect);

typedef struct {
    MyFP1 fn;
    int x;
} MyRef;

typedef void (*MyFP2)(MyRef *p);

static void f(MyRef *p) {
    (void)p;
}

MyFP2 bug_trigger = (MyFP2)f;

Uint32 test_val;
MyRect test_rect;

int test_original(void) {
    test_val = 42;
    test_rect.x = 1;
    return (int)test_val + test_rect.x;
}

/* === Case 2: Simple fn-ptr typedef with typedef param === */
typedef unsigned int Size;

typedef Size (*Sizer)(const char *s);

Sizer g_sizer = (Sizer)0;

int test_simple_fnptr_typedef(void) {
    Size sz = 0;
    (void)sz;
    return g_sizer != 0 ? 1 : 0;
}

/* === Case 3: Multiple typedef params in fn-ptr === */
typedef int Int32;
typedef long Long64;

typedef Int32 (*Converter)(Long64 val);

Converter g_conv = (Converter)0;

int test_multi_typedef_params(void) {
    return g_conv != 0 ? 1 : 0;
}

/* === Case 4: Nested fn-ptr typedef referencing typedef === */
typedef double Double64;

typedef Double64 (*MathOp)(Double64 x);

typedef MathOp (*MathFactory)(int kind);

MathFactory g_mf = (MathFactory)0;

int test_nested_fnptr_typedef(void) {
    return g_mf != 0 ? 1 : 0;
}

/* === Case 5: Fn-ptr typedef with struct pointer param === */
typedef struct Node {
    int value;
    struct Node *next;
} Node;

typedef Node *(*NodeFactory)(int val);

NodeFactory g_nf = (NodeFactory)0;

int test_struct_ptr_param(void) {
    return g_nf != 0 ? 1 : 0;
}

/* === Case 6: Cast between fn-ptr typedefs === */
typedef void (*VoidFn)(void);
typedef int (*IntFn)(void);

int test_cast_between_fnptr_typedefs(void) {
    VoidFn vf = (VoidFn)0;
    IntFn ifn = (IntFn)vf;
    (void)ifn;
    return 0;
}

/* === Case 7: Fn-ptr typedef with const typedef pointer param === */
typedef const char *CStr;

typedef CStr (*Formatter)(int val);

Formatter g_fmt = (Formatter)0;

int test_const_typedef_ptr_param(void) {
    return g_fmt != 0 ? 1 : 0;
}

/* === Case 8: Global var with cast triggers reordering === */
typedef long LongType;

typedef LongType (*LongFn)(LongType x);

static LongType identity(LongType x) { return x; }

LongFn g_long_fn = (LongFn)identity;

int test_global_cast_reorder(void) {
    return g_long_fn != 0 ? (int)g_long_fn(1) : 0;
}

/* === Case 9: Enum typedef in fn-ptr param === */
typedef enum { MODE_A, MODE_B } Mode;

typedef Mode (*ModeSelector)(int flags);

ModeSelector g_ms = (ModeSelector)0;

int test_enum_typedef_param(void) {
    return g_ms != 0 ? 1 : 0;
}

/* === Case 10: Array typedef in fn-ptr param === */
typedef int IntArr[4];

typedef IntArr *(*ArrFactory)(void);

ArrFactory g_af = (ArrFactory)0;

int test_array_typedef_param(void) {
    return g_af != 0 ? 1 : 0;
}

int main(void) {
    int r = 0;
    r += test_original();
    r += test_simple_fnptr_typedef();
    r += test_multi_typedef_params();
    r += test_nested_fnptr_typedef();
    r += test_struct_ptr_param();
    r += test_cast_between_fnptr_typedefs();
    r += test_const_typedef_ptr_param();
    r += test_global_cast_reorder();
    r += test_enum_typedef_param();
    r += test_array_typedef_param();
    return r > 0 ? 0 : 1;
}
