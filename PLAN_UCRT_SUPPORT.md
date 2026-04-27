# MSVC UCRT 完整支持实现计划

## 目标

让 sharp 编译器直接解析 MSVC UCRT 头文件，不再依赖 TCC 头文件。

## 核心原则

- **调用约定**：作为 AST 属性存储，cgen 时正确生成
- **导入/导出属性**：解析器识别并标记符号
- **SAL 注解**：预处理器消除（空宏），与 GCC/clang 行为一致
- **`__pragma`**：词法层面跳过

## 阶段 1：调用约定系统（__CRTDECL / __cdecl / __stdcall）

### 1.1 词法分析（lexer.c）

添加 token 类型：
- `TK___CRTDECL` — `__CRTDECL`
- `TK___CLR_OR_THIS_CALL` — `__CLR_OR_THIS_CALL`

已有支持的（无需修改）：
- `TK___CDECL` — `__cdecl`
- `TK___STDCALL` — `__stdcall`
- `TK___FASTCALL` — `__fastcall`

### 1.2 AST 节点（parser.h / ast.h）

在 `Node` 结构体的函数声明节点中添加 `calling_conv` 字段（如果还没有）：
```c
typedef enum { CC_DEFAULT, CC_CDECL, CC_STDCALL, CC_FASTCALL, CC_CRTDECL } CallingConv;
```

### 1.3 解析器（parser.c）

修改 `parse_func_common()` 和 `parse_decl_specifiers()`：
- 在返回类型之前/之后识别调用约定 token
- 存储到 AST 节点的 `calling_conv` 字段
- 支持 `__CRTDECL` 映射为 `CC_CDECL`（因为 `__CRTDECL` 在 x64 上展开为 `__cdecl`）

### 1.4 代码生成（cgen.c）

修改函数声明/定义的代码生成：
- `CC_CDECL` → 无修饰（默认）
- `CC_STDCALL` → 生成 `__stdcall`
- `CC_FASTCALL` → 生成 `__fastcall`
- `CC_CRTDECL` → 生成 `__cdecl`

### 1.5 预处理器宏（main.c）

在 `apply_target_macros()` 中定义：
```c
cpp_define(cpp, "__CRTDECL", "__cdecl");
cpp_define(cpp, "__CLRCALL_PURE_OR_CDECL", "__cdecl");
```

### 验证

- `#include <stdio.h>` 能正确解析 UCRT 版本
- `printf` 等函数声明能正确生成 C 代码
- clang-cl 编译无错误

---

## 阶段 2：SAL 注解系统（空宏消除）

### 2.1 预处理器宏定义（main.c）

在 `apply_target_macros()` 中添加 ~60 个 SAL 空宏定义：

```c
/* SAL annotations - defined as empty to be compatible with UCRT headers.
 * GCC and clang use the same approach. */
cpp_define(cpp, "_Success_(x)", "");
cpp_define(cpp, "_Check_return_", "");
cpp_define(cpp, "_Check_return_opt_", "");
cpp_define(cpp, "_Check_return_wat_", "");
cpp_define(cpp, "_In_", "");
cpp_define(cpp, "_In_z_", "");
cpp_define(cpp, "_In_opt_", "");
cpp_define(cpp, "_In_opt_z_", "");
cpp_define(cpp, "_In_reads_(x)", "");
cpp_define(cpp, "_In_reads_bytes_(x)", "");
cpp_define(cpp, "_In_count_(x)", "");
cpp_define(cpp, "_Out_", "");
cpp_define(cpp, "_Out_z_", "");
cpp_define(cpp, "_Out_opt_", "");
cpp_define(cpp, "_Out_writes_(x)", "");
cpp_define(cpp, "_Out_writes_z_(x)", "");
cpp_define(cpp, "_Out_writes_opt_(x)", "");
cpp_define(cpp, "_Out_writes_to_(x,y)", "");
cpp_define(cpp, "_Out_writes_bytes_(x)", "");
cpp_define(cpp, "_Inout_", "");
cpp_define(cpp, "_Inout_opt_", "");
cpp_define(cpp, "_Inout_z_", "");
cpp_define(cpp, "_Ret_z_", "");
cpp_define(cpp, "_Ret_opt_z_", "");
cpp_define(cpp, "_Ret_writes_(x)", "");
cpp_define(cpp, "_Deref_out_", "");
cpp_define(cpp, "_Deref_out_z_", "");
cpp_define(cpp, "_Deref_opt_out_", "");
cpp_define(cpp, "_Post_", "");
cpp_define(cpp, "_Pre_", "");
cpp_define(cpp, "_Pre_z_", "");
cpp_define(cpp, "_Post_z_", "");
cpp_define(cpp, "_Post_valid_", "");
cpp_define(cpp, "_Pre_valid_", "");
cpp_define(cpp, "_Pre_writable_byte_size_(x)", "");
cpp_define(cpp, "_Pre_readable_byte_size_(x)", "");
cpp_define(cpp, "_Post_writable_byte_size_(x)", "");
cpp_define(cpp, "_Printf_format_string_", "");
cpp_define(cpp, "_Scanf_format_string_", "");
cpp_define(cpp, "_Scanf_s_format_string_", "");
cpp_define(cpp, "_Always_(x)", "");
cpp_define(cpp, "_When_(x,y)", "");
cpp_define(cpp, "_Null_terminated_", "");
cpp_define(cpp, "_NullNull_terminated_", "");
cpp_define(cpp, "_Readable_bytes_(x)", "");
cpp_define(cpp, "_Writable_bytes_(x)", "");
cpp_define(cpp, "_Outptr_", "");
cpp_define(cpp, "_Outptr_opt_", "");
cpp_define(cpp, "_Outptr_result_z_", "");
cpp_define(cpp, "_Frees_ptr_", "");
cpp_define(cpp, "_Frees_ptr_opt_", "");
cpp_define(cpp, "_At_(target, annos)", "");
cpp_define(cpp, "_Analysis_assume_(x)", "");
cpp_define(cpp, "_Return_type_success_(x)", "");
cpp_define(cpp, "_Field_range_(x,y)", "");
cpp_define(cpp, "_Field_size_(x)", "");
cpp_define(cpp, "_Field_size_opt_(x)", "");
cpp_define(cpp, "_Struct_size_bytes_(x)", "");
cpp_define(cpp, "_CRTIMP", "");
cpp_define(cpp, "_ACRTIMP", "");
cpp_define(cpp, "_DCRTIMP", "");
cpp_define(cpp, "_CRTIMP2", "");
cpp_define(cpp, "_CRTIMP2_PURE", "");
cpp_define(cpp, "_CRT_STDIO_INLINE", "__inline");
cpp_define(cpp, "_NO_CRT_STDIO_INLINE", "");
cpp_define(cpp, "__pragma(x)", "");
```

### 2.2 验证

- `#include <stdio.h>` 解析后函数参数不含 SAL 残留
- `#include <string.h>`, `<stdlib.h>`, `<wchar.h>` 都能通过
- clang-cl 编译生成的 C 代码无类型错误

---

## 阶段 3：`__declspec(dllimport/dllexport)` 增强

### 3.1 现状

已有 `TK___DECLSPEC` token 和 `parse_declspec()` 函数，但可能不完善。

### 3.2 需要检查/修复

1. 确认 `__declspec(dllimport)` 能正确解析并标记外部符号
2. 确认 `__declspec(noreturn)` 能正确标记函数
3. 确认 `__declspec(noinline)` 能正确标记函数
4. cgen.c 生成正确的 `__declspec` 前缀

### 3.3 验证

- 外部变量声明（如 `__declspec(dllimport) int _iob[]`）能正确生成
- `__declspec(noreturn)` 函数标记正确

---

## 阶段 4：`__int64` / `__int32` / `__int16` / `__int8` 类型

### 4.1 现状

可能已有支持，但需要确认。

### 4.2 需要添加

在词法分析器中添加 token 类型，在解析器的类型系统中映射：
- `__int64` → `TY_LONGLONG`
- `__int32` → `TY_INT`
- `__int16` → `TY_SHORT`
- `__int8` → `TY_CHAR`
- `unsigned __int64` → `TY_LONGLONG`（无符号）

### 4.3 验证

- UCRT 中的 `typedef unsigned __int64 size_t;` 能正确解析
- `__int64` 运算类型正确

---

## 阶段 5：切换 UCRT include 路径

### 5.1 修改 main.c

移除 TCC include 路径（或保留为 fallback），添加：
```c
/* UCRT include paths */
cpp_add_sys_include(cpp_ctx, "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\ucrt");
cpp_add_sys_include(cpp_ctx, "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.43.34808\\include");
```

### 5.2 include 搜索顺序

1. 用户 include（`-I`）
2. UCRT
3. MSVC
4. TCC（fallback）

### 5.3 验证

- 所有核心 93 测试通过
- UCRT `stdio.h`, `stdlib.h`, `string.h` 等 58 个头文件全部通过
- 生成的 C 代码用 clang-cl 编译无错误

---

## 验证策略

每个阶段完成后运行：

1. **核心测试**：`_test_all.ps1` → 93/93 通过
2. **UCRT 头文件测试**：测试 58 个 UCRT 纯 C 头文件
3. **clang-cl 编译测试**：生成代码用 clang-cl 编译
4. **运行测试**：`test_print.exe` 等输出正确

## 风险与缓解

| 风险 | 缓解 |
|------|------|
| SAL 空宏定义遗漏某些变体 | 测试时逐个添加 |
| `__pragma` 嵌套复杂 | 预处理器简单消除即可 |
| UCRT 版本差异 | 使用 `GetFileAttributes` 自动检测 |
| 现有测试破坏 | 每个阶段后跑全量测试 |

## 预估工作量

- 阶段 1：~1 小时（调用约定系统）
- 阶段 2：~2 小时（SAL 空宏定义 + 测试）
- 阶段 3：~1 小时（`__declspec` 增强）
- 阶段 4：~30 分钟（`__int64` 类型）
- 阶段 5：~30 分钟（路径切换）
- **总计：约 5 小时**
