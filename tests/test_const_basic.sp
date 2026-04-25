/* compiler: any */
/* test_const_basic.sp - Test const parsing and codegen */

extern i32 puts(const u8* s);

i32 main() {
    const u8* msg = "hello const";
    puts(msg);
    return 0;
}
