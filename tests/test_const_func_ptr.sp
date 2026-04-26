/* compiler: any */
/* C Feature: const function pointer declarations */
/* The generated C code must produce correct syntax like:
 *   i32 (* const fp)(i32, i32)
 */

i32 main() {
    i32 (* const fp)(i32, i32) = null;
    return 0;
}
