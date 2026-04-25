/* compiler: any */
/* TDD-Stmt-1: print / println built-in */
/* NOTE: print/println 是 SP 编译器内置函数，生成 printf 调用。
 * 生成的 C 代码不自动包含 <stdio.h>，GCC 编译时会报 printf 未声明的警告。
 * C 代码生成正确。
 */

i32 main() {
    print(42);
    println("hello");
    return 0;
}
