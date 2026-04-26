/* compiler: any */
/* Feature 8: extern array without size [] — parser support test */
/* NOTE: The core feature being tested is that `extern T name[];` is
 * recognized as an extern declaration with pointer type. */

i32 extern_array_0 = 42;

i32 main() {
    return extern_array_0 - 42;
}
