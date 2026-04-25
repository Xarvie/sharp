/* compiler: any */
// TDD-§21: 编译期静态断言老式写法
// 使用数组 typedef 实现编译期断言（C89 兼容写法）

typedef char __static_assert_i32[1];

i32 main() {
    return 42;
}
