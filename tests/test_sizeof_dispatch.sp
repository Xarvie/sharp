/* compiler: any */
/* §17 sizeof 类型派发宏 */
extern void fpclassify_impl(double x);
extern void fpclassify_impl_f(float x);

i32 main() {
    double x = 1.0;
    return 0;
}
