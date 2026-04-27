# 16个失败测试修复计划

## 分类

### A. C->exe 编译阶段失败（非 sharpc 问题，共 6 个）
这些是运行时 C 编译器（GCC/MSVC）的头文件或平台差异问题。

| 测试 | 问题 | 解决方案 | 优先级 |
|------|------|---------|--------|
| test_alloca.sp | 需要 `<alloca.h>` | cgen 自动添加 `#include <alloca.h>` | 低 |
| test_builtin_frame.sp | `__builtin_frame_address` GCC 内建 | 头文件平台限制，可能无法修复 | 跳过 |
| test_nan_infinity.sp | `INFINITY/NAN` 需要 `<math.h>` | cgen 自动添加 `#include <math.h>` | 低 |
| test_need_wint_t.sp | 条件宏 `__need_wint_t` | 头文件机制问题 | 低 |
| test_print.sp | 缺少 `stdio.h` | cgen 已包含，检查是否生效 | 低 |
| test_seh.sp | Windows SEH 仅 MSVC 支持 | 平台限制 | 跳过 |

### B. Parser 缺失功能（共 10 个）

#### B1. 结构体字段解析增强（3 个）
涉及文件：`parser.c` 的 `parse_struct()` 字段解析循环

| # | 测试 | 缺失特性 | 错误描述 | 工作量 |
|---|------|---------|---------|--------|
| 1 | test_anonymous_struct | 匿名 struct/union 字段 | 字段解析不支持 `struct { ... }` 内联定义 | 1 天 |
| 2 | test_nested_struct | 嵌套结构体定义 | 同上 | 同上，与 #1 一起修复 |
| 3 | test_bitfield | 位域 `:N` | 字段名后不支持 `:` 位宽 | 0.5 天 |
| 10 | test_struct_field_declspec | 字段 __declspec | `type* __declspec(...) name` | 0.5 天 |

#### B2. 表达式解析增强（3 个）
涉及文件：`parser.c` 的表达式解析

| # | 测试 | 缺失特性 | 错误描述 | 工作量 |
|---|------|---------|---------|--------|
| 5 | test_compound_literal | 复合字面量 | 不支持 `(Type){ .x=1 }` | 1.5 天 |
| 6 | test_designated_init | 指定初始化 | 初始化列表不支持 `.field = val` | 1 天 |

#### B3. 声明增强（3 个）
涉及文件：`parser.c` 的声明解析

| # | 测试 | 缺失特性 | 错误描述 | 工作量 |
|---|------|---------|---------|--------|
| 4 | test_complex_attribute | 变量后 __attribute__ | `i32 x __attribute__((...));` | 0.5 天 |
| 7 | test_extension | __extension__ 关键字 | `__extension__ i64 x = ...` | 0.25 天 |
| 8 | test_func_pointer_param | 函数指针参数 | `i32 (*compar)(void*, void*)` 在参数列表 | 1 天 |
| 10 | test_thread_local | __declspec(thread) | `__declspec(thread) i32 x` | 0.5 天 |

## 修复顺序（按依赖和复杂度）

### 第一阶段：简单声明增强（1-2 天）
1. **test_extension** — 在 `is_type_start` 添加 `TK___EXTENSION__`，`parse_type` 消费它
2. **test_thread_local** — 在 `parse_vardecl` 和 `type_start_path` 处理 `__declspec(thread)`
3. **test_complex_attribute** — 在 `parse_vardecl` 和 top-level 解析中消费 `__attribute__`

### 第二阶段：结构体字段解析增强（1-2 天）
4. **test_bitfield** — 字段解析中处理 `:N` 位宽
5. **test_struct_field_declspec** — 字段解析中处理 `__declspec(...)` 在 `*` 之后
6. **test_anonymous_struct + test_nested_struct** — 字段解析中处理 `struct/union { ... }` 内联定义

### 第三阶段：表达式解析增强（2-3 天）
7. **test_designated_init** — 初始化列表中处理 `.field = val`
8. **test_compound_literal** — 表达式解析中处理 `(Type){...}`
9. **test_func_pointer_param** — extern 参数解析中处理函数指针

### 第四阶段：C->exe 头文件修复（0.5 天）
10. **test_print / test_nan_infinity / test_alloca** — cgen 添加对应头文件

## 预期结果
- 可修复：约 13-14 个
- 不可修复（平台限制）：2-3 个
- 目标通过率：75/80 → 77/80（跳过2个平台限制）
