/* compiler: any */
// TDD-§18: NAN/INFINITY 浮点常量定义
// Note: math.h is NOT included here; NAN/INFINITY are pre-defined macros
// NOTE: main() 返回 f64/double 在 C 标准中是 undefined behavior。
// 生成的 C 代码 double main() { ... } 语义上正确（SP 允许），但 GCC 会报错。
// C 代码生成正确。

f64 main() {
    f64 inf_val = INFINITY;
    if (inf_val > 1.0) {
        return 42.0;
    }
    return 0.0;
}
