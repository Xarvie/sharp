/* compiler: any */
/* §27 __builtin_frame_address / __alignof__ */

extern void* __builtin_frame_address(int level);
extern usize __alignof__(i32 x);

i32 main() {
    i32 x = 42;
    usize align = __alignof__(x);
    return 0;
}
