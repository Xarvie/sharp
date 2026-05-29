typedef enum {
    MY_OP_ENCRYPT = 0,
    MY_OP_DECRYPT = 1
} my_op_t;

static void callee(my_op_t op);

static void caller(void) {
    callee(MY_OP_ENCRYPT);
}

static void callee(my_op_t op) {
    (void)op;
}
