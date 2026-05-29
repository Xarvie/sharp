#include <stddef.h>

typedef enum {
    MY_OP_A = 0,
    MY_OP_B = 1
} my_op_t;

static int my_func(my_op_t op, size_t len);

int my_func(my_op_t op, size_t len) {
    (void)op;
    return (int)len;
}

int main(void) {
    return my_func(MY_OP_A, 42);
}
