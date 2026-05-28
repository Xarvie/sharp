/*
 * sharpc bug: typedef forward reference in function signature
 *
 * Pattern: typedef defined BEFORE function in source order, but function
 * forward declaration (Phase 1.5) was emitted BEFORE typedef (Phase 3a).
 * The old _CHECK_TYPE_TD macro used `pos <= fpos` to skip typedefs defined
 * before the function, but this assumption is wrong because Phase 1.5 runs
 * before Phase 3a.
 *
 * Fix: move Phase 1.5 after Phase 3a so all typedefs/structs are already
 * defined when function forward declarations are emitted.
 */

/* --- Original test case: enum typedef --- */

typedef enum {
    MY_OP_ENCRYPT = 0,
    MY_OP_DECRYPT = 1
} my_op_t;

void caller(void) {
    callee(MY_OP_ENCRYPT);
}

void callee(my_op_t op) {
    (void)op;
}

/* --- Extension 1: struct typedef --- */

typedef struct {
    int x;
    int y;
} point_t;

void use_point(point_t p);

void caller_point(void) {
    use_point(early_point());
}

point_t early_point(void) {
    point_t p = { .x = 1, .y = 2 };
    return p;
}

void use_point(point_t p) {
    (void)p;
}

/* --- Extension 2: typedef in return type --- */

typedef unsigned int uint32_t;

uint32_t get_value(void);

void caller_ret(void) {
    uint32_t v = get_value();
    (void)v;
}

uint32_t get_value(void) {
    return 42;
}

/* --- Extension 3: pointer to typedef --- */

typedef int status_t;

void caller_ptr(void) {
    status_t *p = get_status();
    (void)p;
}

status_t *get_status(void) {
    static status_t s = 0;
    return &s;
}

/* --- Extension 4: array of typedef --- */

typedef int value_t;

void caller_arr(void) {
    value_t arr[4];
    fill_arr(arr, 4);
    (void)arr;
}

void fill_arr(value_t *arr, int n) {
    for (int i = 0; i < n; i++) arr[i] = i;
}

/* --- Extension 5: func ptr typedef in param --- */

typedef void (*handler_t)(int);

void caller_fp(void) {
    register_handler(my_handler);
}

void my_handler(int code) {
    (void)code;
}

void register_handler(handler_t h) {
    h(0);
}

/* --- Extension 6: typedef chain --- */

typedef int base_t;
typedef base_t mid_t;
typedef mid_t top_t;

void caller_chain(void) {
    top_t v = get_top();
    (void)v;
}

top_t get_top(void) {
    return 99;
}

/* --- Extension 7: multiple functions with mutual calls --- */

typedef int flag_t;

void alpha(void);
void beta(flag_t f);
void gamma(void);

void alpha(void) { beta(1); }
void beta(flag_t f) { gamma(); (void)f; }
void gamma(void) { alpha(); }
