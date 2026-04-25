/* compiler: msvc */
/* §15 SEH 异常处理宏 */
/* NOTE: SEH 块 __try { ... } 被原样透传到生成的 C 代码。
 * SP 类型别名 i32 在 SEH 块内不会被映射为 int32_t，因为是原始文本透传。
 * 用 MSVC 编译时，__try 是 MSVC 关键字，块内内容需为有效 C 代码。
 * 此测试的 C 代码生成正确（__try { i32 x = 1; }），但 GCC/Clang 不支持 __try 关键字。
 */
i32 main() {
    __try {
        i32 x = 1;
    }
    return 0;
}
