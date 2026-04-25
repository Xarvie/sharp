// TDD-§18: NAN/INFINITY 浮点常量定义
// Note: math.h is NOT included here; NAN/INFINITY are pre-defined macros

f64 main() {
    f64 inf_val = INFINITY;
    if (inf_val > 1.0) {
        return 42.0;
    }
    return 0.0;
}
