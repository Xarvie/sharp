/* §12 __restrict__ 关键字测试 */

extern i32 strlen(const i8* __restrict__ s);

i32 main() {
    i32* __restrict__ p = null;
    return 0;
}
