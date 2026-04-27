/* compiler: any */
/* §27 __builtin_frame_address / __alignof__ */

extern void* __builtin_frame_address(int level);
extern long __alignof__(int x);

int main() {
    int x = 42;
    long align = __alignof__(x);
    return 0;
}