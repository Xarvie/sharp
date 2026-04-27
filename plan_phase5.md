# Phase 5: 修复剩余 8 个失败测试

## 当前状态
- **通过**: 72/80
- **失败**: 8

## 失败分类

### A. sharpc C 发射问题（1 个，可修复）
| 测试 | 问题 | 根因 |
|------|------|------|
| test_func_pointer_param | C->exe 编译失败 | ND_EXTERN_DECL 参数发射时，函数指针类型和名称分离发射导致语法错误 |

### B. 平台限制测试（7 个，部分可尝试修复）
| 测试 | 问题 | 可修复性 |
|------|------|----------|
| test_alloca | GCC C->exe 失败 | 可能缺少 `<alloca.h>` 头文件 |
| test_builtin_frame | GCC C->exe 失败 | `__builtin_frame_address` GCC 不支持？ |
| test_nan_infinity | GCC C->exe 失败 | NaN/Inf 字面量生成问题 |
| test_need_wint_t | GCC C->exe 失败 | 缺少 `wint_t` 类型定义 |
| test_print | GCC C->exe 失败 | 可能链接问题 |
| test_seh | MSVC SEH 在非 MSVC 失败 | 平台限制，跳过 |
| test_thread_local | MSVC C->exe 失败 | MSVC `__declspec(thread)` 链接问题 |

## 修复计划

### Step 1: test_func_pointer_param — C 发射修复（优先级: 高）

**问题**: 
生成的 C 代码为:
```c
extern void qsort(void* base, size_t nmemb, size_t size, int32_t(*)(const void*, const void*)* compar);
```
应该为:
```c
extern void qsort(void* base, size_t nmemb, size_t size, int32_t (*compar)(const void*, const void*));
```

**根因**: 
在 cgen.c 中发射 extern 声明参数时，简单地做 `emit_type(g, param->declared_type); sb_printf(&g->out, " %s", param->name);`。对于函数指针类型 `ptr(func(i32, ptr(const void), ptr(const void)))`，`emit_type` 会发射 `int32_t(*)(const void*, const void*)`，然后加上 `* compar` 导致语法错误。

**修复方案**:
方案 A: 在 ND_PARAM 节点中存储"类型前缀"和"类型后缀+名称"信息，让 cgen 按 C 语法正确发射。
方案 B: 在 cgen 中检测函数指针类型，特殊处理发射语法。
方案 C（推荐）: 修改 cgen 的 `emit_param` 函数，在检测到函数指针类型时，提取返回值类型、参数列表和名称，按 C 语法重新组装。

**实现步骤**:
1. 在 cgen.c 中查找发射 extern 声明参数的代码
2. 添加 `emit_param` 函数，检测 `TY_FUNC` 类型
3. 对于函数指针类型，提取: return_type, param_types, is_ptr
4. 按 `return_type (*name)(param_types...)` 格式发射
5. 对于普通类型，继续用现有逻辑

### Step 2: test_alloca — 头文件修复（优先级: 中）

**问题**: GCC 编译失败
**可能原因**: 缺少 `#include <alloca.h>`
**修复**: 在生成的 C 代码中添加 alloca.h 头文件，或在 cgen 中检测 alloca 使用时自动添加

### Step 3: test_nan_infinity — 字面量修复（优先级: 中）

**问题**: GCC 编译失败
**可能原因**: NaN/Infinity 字面量生成不正确
**修复**: 检查 cgen 中 float 字面量发射，使用 `NAN`/`INFINITY` 宏

### Step 4: test_need_wint_t — 类型定义修复（优先级: 中）

**问题**: GCC 编译失败
**可能原因**: 缺少 `wint_t` 类型
**修复**: 添加 `#include <wchar.h>` 或 typedef

### Step 5: test_print — 链接修复（优先级: 低）

**问题**: GCC 编译失败
**修复**: 检查 print 相关的 C 代码生成

### Step 6: test_builtin_frame — 平台限制（跳过）

**问题**: GCC 不支持 `__builtin_frame_address`
**决策**: 跳过

### Step 7: test_seh, test_thread_local — 平台限制（跳过）

**问题**: MSVC 特有功能
**决策**: 跳过

## 预期结果
- 修复 Step 1-5 后: 约 76-77/80 通过
- 跳过 Step 6-7 后: 剩余 3-4 个平台限制失败
