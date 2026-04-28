# P99 测试状态报告

## 更新日期
2026-04-29

## 总体状态
- **PASS**: 12/65 (18.5%)
- **FAIL**: 53/65 (81.5%)
  - **HANG**: 0 → 0 (全部修复！之前 28 个)
  - **PARSE**: 53 (token paste 31 + file not found 16 + 其他 6)

## 本轮修复成果

### 1. 修复 28 个 HANG（死循环）✅
- **根本原因**: 宏展开达到深度/数量限制后，未设置全局标记，导致后续每个宏调用都重复尝试展开
- **修复方案**: 
  - 添加 `g_limits_breached` 全局标志
  - `expand_list` 中增加快速路径：标志设置后直接跳过所有宏展开
  - `directive.c` 中调用 `macro_limits_breached()` 提前跳过
  - `macro_expand` 中不重复重置限制（保持全局性）
- **效果**: 28 个 HANG 全部消除，现在快速完成

### 2. 修复 token paste leading space 丢失 ✅
- **根本原因**: 宏展开后第一个 token 的 `has_leading_space` 未继承原始宏调用 token 的标志
- **修复方案**: 在 `directive.c` 中保存原始标志并传播到展开后的第一个 token
- **效果**: 简单 token paste 测试通过

## 当前失败分类

### A. token paste 复杂边缘情况（31 个文件）
- **症状**: `p99_args.h:147:99: error: token paste produces invalid token`
- **涉及文件**: p99_arith.h, p99_check.h, p99_checkable.h, p99_choice.h 等 31 个
- **分析**: 
  - 简单 `##` 已经工作（`p99_paste.h` 通过，简单测试通过）
  - 复杂场景（如 `MAC ## __VA_ARGS__` 与 `P00_NARG` 嵌套）在多 header 交互时产生无效 token
  - 单独测试 `p99_args.h` 不报错，但通过其他 header 间接包含时报错
  - 涉及 P99 的变长参数计数技巧：`P00_NARG(MAC ## __VA_ARGS__ (~) MAC ## __VA_ARGS__)`
- **根因**: 多 header 宏定义交互导致参数替换时 `##` 两侧 token 产生非预期组合
- **状态**: ⚠️ 复杂待处理

### B. 文件未找到（16 个文件）
- **症状**: `file not found: p99_generated.h` 等
- **涉及**: p99_init.h, p99_new.h 等依赖生成文件的 header
- **分析**: 
  - `p99_generated.h` 需要 Python 脚本 `configure.py` 生成
  - sharpc 的 include 路径未正确配置
- **状态**: 🔧 需要配置 include 路径 + 生成缺失文件

### C. 其他解析错误（6 个文件）
- **症状**: `invalid #include argument`, `expected ')' in #if expression`, `#elif without matching #if`
- **涉及**: p99_defarg.h, p99_compiler.h, p99_extern_inline.h 等
- **分析**: `#include P99_STRINGIFY(p99/p99_include.h)` 形式未被正确处理
- **状态**: ⚠️ 复杂待处理

## 已知工作正常
- 简单 token paste: `#define PASTE(a, b) a ## b` ✅
- 简单 variadic: `#define V(...) __VA_ARGS__` ✅
- `p99_paste.h` 全部通过
- `p99_for.h` 通过
- `p99_util.h` 通过
- `p99_default.h` 通过
- `p99_args.h` 单独编译通过

## 下一步优先级
1. **P0**: 修复 include 路径配置，减少 file not found 错误
2. **P1**: 分析 token paste 复杂边缘情况的根因（需要更精细的 debug）
3. **P2**: 修复 `#include P99_STRINGIFY(...)` 形式的解析
