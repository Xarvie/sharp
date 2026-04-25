/* compiler: gcc */
/* §18 INFINITY 浮点常量 */
/* NOTE: INFINITY 是 C 标准宏，由 GCC/Clang 提供。
 * 测试重点是 INFINITY 在生成的 C 代码中可用。
 */

i32 main() {
    f64 inf_val = INFINITY;
    if (inf_val > 1.0) {
        return 42;
    }
    return 0;
}
