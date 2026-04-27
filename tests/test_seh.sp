/* compiler: msvc */
/* §15 SEH 异常处理宏 */
/* NOTE: SEH 块 __try { ... } 被原样透传到生成的 C 代码。
 * SP 类型别名 int 在 SEH 块内不会被映射为 int，因为是原始文本透传。
 * 用 MSVC 编译时，__try 是 MSVC 关键字，块内内容需为有效 C 代码。
 * __try 必须跟随 __except 或 __finally。
 */
int main() {
    __try {
        int x = 1;
    } __except(1) {
    }
    return 0;
}