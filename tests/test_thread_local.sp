/* compiler: msvc */
/* C Feature: Thread-local storage */
/* NOTE: 使用 MSVC 支持的 __declspec(thread) 语法。__thread 是 GCC 扩展，MSVC 不支持。 */

__declspec(thread) int tls_var1;

int main() {
    __declspec(thread) int tls_var2 = 0;
    return tls_var1 + tls_var2;
}