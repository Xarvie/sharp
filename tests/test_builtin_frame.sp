/* compiler: any */
/* §27 __builtin_frame_address / __alignof__ */
/* NOTE: __alignof__ 是 GCC 内建运算符，接收类型作为参数，不接收表达式。
 * 生成的 C 代码 __alignof__((void*)(&x)) 在语义上正确，
 * 但 GCC 期望 __alignof__(type) 而非 __alignof__(expr)。
 * C 代码生成正确。
 */
extern void* __builtin_frame_address(int level);
extern usize __alignof__(void* p);

i32 main() {
    i32 x = 42;
    usize align = __alignof__((void*)&x);
    return 0;
}
