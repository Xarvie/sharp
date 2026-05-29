/*
 * test_enum_ptr.c — Bug: enum pointer type preserved in function pointer typedef.
 *
 * sharpc used to collapse enum types to int in function pointer typedefs,
 * causing `typedef void (*func_ptr)(MyEnum*)` to be emitted as `void (*)(int *)`
 * while the function declaration kept `void (MyEnum *)`.  These are incompatible
 * pointer-to-function types in C.
 *
 * Root cause: ty_from_ast returned ty_int(ts) for all enum types, losing the
 * enum tag identity needed for type matching.
 *
 * Fix: introduced TY_ENUM type that preserves enum tag identity, so both the
 * function pointer typedef and the function declaration produce matching types.
 */

/* === Case 1: Anonymous enum with typedef alias (original bug) === */
typedef enum {
    VAL_A,
    VAL_B
} MyEnum;

void func(MyEnum* p);

void test_anon_enum_fnptr(void) {
    typedef void (*func_ptr)(MyEnum*);
    func_ptr p = func;   /* must compile without type mismatch */
    (void)p;
}

/* === Case 2: Named enum with typedef alias === */
typedef enum Color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} Color;

void set_color(Color* c);

void test_named_enum_fnptr(void) {
    typedef void (*color_fn)(Color*);
    color_fn fn = set_color;
    (void)fn;
}

/* === Case 3: Pure enum tag (no typedef) === */
enum Status {
    STATUS_OK,
    STATUS_ERR
};

void handle_status(enum Status* s);

void test_enum_tag_fnptr(void) {
    typedef void (*status_fn)(enum Status*);
    status_fn fn = handle_status;
    (void)fn;
}

/* === Case 4: Enum value parameter (not pointer) === */
typedef enum Mode {
    MODE_READ,
    MODE_WRITE
} Mode;

void set_mode(Mode m);

void test_enum_value_fnptr(void) {
    typedef void (*mode_fn)(Mode);
    mode_fn fn = set_mode;
    (void)fn;
}

/* === Case 5: Multiple enum pointer parameters === */
typedef enum Flag {
    FLAG_A = 1,
    FLAG_B = 2
} Flag;

void merge_flags(Flag* a, Flag* b, Flag* out);

void test_multi_enum_ptr_fnptr(void) {
    typedef void (*merge_fn)(Flag*, Flag*, Flag*);
    merge_fn fn = merge_flags;
    (void)fn;
}

/* === Case 6: Enum pointer return type === */
typedef enum Result {
    RES_SUCCESS,
    RES_FAIL
} Result;

Result* get_result(void);

void test_enum_ptr_return_fnptr(void) {
    typedef Result* (*result_fn)(void);
    result_fn fn = get_result;
    (void)fn;
}

/* === Case 7: Mixed enum and non-enum parameters === */
typedef enum Level {
    LEVEL_LOW,
    LEVEL_HIGH
} Level;

void process(Level* lvl, int count, char* buf);

void test_mixed_params_fnptr(void) {
    typedef void (*process_fn)(Level*, int, char*);
    process_fn fn = process;
    (void)fn;
}

/* === Case 8: Enum pointer in nested function pointer typedef === */
typedef enum Action {
    ACTION_START,
    ACTION_STOP
} Action;

typedef void (*callback_t)(Action*);

void register_callback(callback_t cb);

void test_nested_fnptr(void) {
    typedef void (*register_fn)(callback_t);
    register_fn fn = register_callback;
    (void)fn;
}

/* === Case 9: Typedef enum with explicit underlying values === */
typedef enum Priority {
    PRIO_LOW   = 10,
    PRIO_MED   = 20,
    PRIO_HIGH  = 30,
    PRIO_CRIT  = 40
} Priority;

void schedule(Priority* p);

void test_explicit_enum_fnptr(void) {
    typedef void (*sched_fn)(Priority*);
    sched_fn fn = schedule;
    (void)fn;
}

/* === Case 10: Const enum pointer parameter === */
typedef enum Kind {
    KIND_INT,
    KIND_STR
} Kind;

void inspect_kind(const Kind* k);

void test_const_enum_ptr_fnptr(void) {
    typedef void (*inspect_fn)(const Kind*);
    inspect_fn fn = inspect_kind;
    (void)fn;
}

/* === Case 11: Enum basic operations (init, assign, compare, return) === */
typedef enum {
    OP_VAL_A = 0,
    OP_VAL_B = 1
} OpEnum;

OpEnum get_enum(void) {
    return 0;
}

void test_enum_init(void) {
    OpEnum e = 0;
    (void)e;
}

void test_enum_assign(void) {
    OpEnum e = OP_VAL_A;
    e = 1;
    (void)e;
}

void test_enum_compare(void) {
    OpEnum e = OP_VAL_A;
    if (e == 0) {}
    if (e != 1) {}
}

void test_enum_return(void) {
    OpEnum e = OP_VAL_A;
    int i = e;
    (void)i;
}

/* Stub implementations to satisfy the linker */
void func(MyEnum* p) {}
void set_color(Color* c) {}
void handle_status(enum Status* s) {}
void set_mode(Mode m) {}
void merge_flags(Flag* a, Flag* b, Flag* out) {}
Result* get_result(void) { return 0; }
void process(Level* lvl, int count, char* buf) {}
void register_callback(callback_t cb) {}
void schedule(Priority* p) {}
void inspect_kind(const Kind* k) {}
