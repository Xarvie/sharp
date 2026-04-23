/* test_const_ptr.sp - Test const pointer variants */

extern i32 puts(const u8* s);

i32 main() {
    const u8* msg1 = "hello const";
    puts(msg1);
    return 0;
}
