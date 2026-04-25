/* compiler: any */
/* §22 alloca / __builtin_alloca */
/* NOTE: alloca() 在 MSVC 中是 intrinsic，在 GCC/Clang 中是内置函数。
 * 生成的 C 代码中 alloca 声明为 extern void*，但 GCC 需要 <alloca.h> 或 __builtin_alloca。
 * C 代码生成正确，但 GCC 编译时可能报 alloca 未声明的警告。
 */
extern void* alloca(usize size);
extern void* __builtin_alloca(usize size);

i32 main() {
    i32* p = (i32*)alloca(100);
    return 0;
}
