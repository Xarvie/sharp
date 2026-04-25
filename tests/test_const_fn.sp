/* compiler: any */
/* test_const_fn.sp - Test const in function parameters */

extern i32 puts(const u8* s);

void print_msg(const u8* msg) {
    puts(msg);
}

i32 main() {
    const u8* hello = "hello";
    print_msg(hello);
    return 0;
}
