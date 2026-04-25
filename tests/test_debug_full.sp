extern i32 puts(const u8* s);
i32 main() {
    const u8* msg = "hello const";
    puts(msg);
    return 0;
}
