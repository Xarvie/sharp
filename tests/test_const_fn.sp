/* compiler: any */
/* test_const_fn.sp - Test const in function parameters */

extern int puts(const char* s);

void print_msg(const char* msg) {
    puts(msg);
}

int main() {
    const char* hello = "hello";
    print_msg(hello);
    return 0;
}