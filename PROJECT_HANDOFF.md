# Sharp Compiler (sharpc) 项目交接文档

> 最后更新: 2026-04-28 | 当前版本: 0.4 | 测试通过率: 158/161 (UCRT)

---

## 一、项目概述

**Sharp** 是一门实验性系统编程语言，语法类似 C + Rust。它有自己的编译器 `sharpc`，能将 Sharp 源码 (`.sp`) 编译为 C 代码，再通过 C 编译器（GCC/Clang/MSVC）生成可执行文件。

### 语言特性
- C 风格语法 + Rust 风格 `impl` 块
- 结构体/联合体/枚举
- 泛型（C++ 风格模板，通过 TCC 运行时编译）
- RAII（`drop` 机制）
- 内联汇编
- 宏系统（`#define`、`#include` 等）
- C 互操作（`extern`、`#include` C 头文件）
- 类型别名（`i32` → `int32_t`, `f64` → `double` 等）

### 代码规模
- 核心源码：9 个 `.c` 文件 + 4 个 `.h` 文件 + 5 个预处理器文件
- 总代码量：约 11,300 行 C 代码
- 测试：80 个测试文件（`tests/*.sp`）

---

## 二、目录结构

```
sharp/
├── src/                    # 编译器核心源码
│   ├── sharp.h             #   主头文件：Token、AST、Node 定义
│   ├── types.h             #   类型系统声明
│   ├── ast.h               #   AST 节点访问器/构造器
│   ├── hir.h               #   高级中间表示声明
│   ├── lexer.c             #   词法分析器（Token 化）
│   ├── parser.c            #   语法分析器（AST 构建）
│   ├── sema.c              #   语义分析（类型检查、作用域）
│   ├── lower.c             #   降低到 HIR
│   ├── hir.c               #   高级中间表示
│   ├── cgen.c              #   C 代码生成
│   ├── main.c              #   主入口（编译管线调度）
│   ├── util.c              #   工具函数（字符串缓冲、诊断等）
│   ├── types.c             #   类型系统实现
│   └── preproc/            #  SP 预处理器（C 风格宏）
│       ├── pptok.c/h       #   预处理器 Token 化
│       ├── macro.c/h       #   宏定义/展开
│       ├── expr.c/h        #   预处理器表达式求值
│       ├── directive.c/h   #   指令处理 (#if/#include 等)
│       └── cpp.c/h         #   预处理器主逻辑
├── tests/                  # 测试文件
│   ├── *.sp                #   Sharp 源码测试
│   ├── *.c                 #   生成的 C 代码（用于调试）
│   ├── *.exe               #   编译后的可执行文件
│   └── negative/           #   负向测试（应报错的代码）
├── third_party/            # 第三方依赖
│   └── tcc/                #   TinyCC（嵌入式 C 编译器，用于运行时编译）
├── CMakeLists.txt          # CMake 构建配置
├── run_tests.ps1           # PowerShell 测试脚本
├── sp_compiler_tdd.md      # 编译器 TDD 规范
└── sharp-lang-spec-v0_7.md # 语言规范
```

---

## 三、编译方法

### 环境要求
- **Windows** + **MSYS2**（提供 GCC/Clang 工具链）
- **CMake**（版本 ≥ 3.10）
- **Clang**（推荐 `C:\msys64\clang64\bin\clang.exe`，用于编译 sharpc 本身）
- **GCC** 和 **MSVC**（用于运行测试）

### 构建步骤

```powershell
# 1. 配置（使用 clang 编译 sharpc 本身）
C:\msys64\clang64\bin\cmake.exe -B cmake-build-debug `
    -D CMAKE_C_COMPILER="C:\msys64\clang64\bin\clang.exe"

# 2. 编译
C:\msys64\clang64\bin\cmake.exe --build cmake-build-debug --target sharpc -j 30

# 3. 产物位置
cmake-build-debug\sharpc.exe
```

### 常见问题
- **Ninja "GetOverlappedResult" 错误**：Windows Ninja 并发问题。解决：删除 `cmake-build-debug` 目录后重新配置，或使用 `Unix Makefiles` 生成器。
- **MSVC 编译测试时找不到 `stdint.h`**：需要正确的 VS 开发环境变量。运行 `vcvars64.bat` 后再测试。

---

## 四、测试方法

```powershell
# 运行全部 80 个测试
.\run_tests.ps1

# 输出示例：
# ==============================
# PASS: 79   FAIL: 1
# TOTAL: 80
# ==============================
```

### 测试脚本逻辑（`run_tests.ps1`）
1. 遍历 `tests/*.sp` 文件
2. 读取文件头部的 `/* compiler: xxx */` 注释确定目标编译器
3. `sharpc` 编译 `.sp` → `.c`
4. C 编译器编译 `.c` → `.exe`
5. 运行 `.exe` 检查退出码

### 单个测试调试
```powershell
# 编译 SP → C（输出到 stdout）
.\cmake-build-debug\sharpc.exe tests\test_example.sp -

# 编译 SP → C 文件（不链接）
.\cmake-build-debug\sharpc.exe tests\test_example.sp -o tests\test_example.c -no-link

# 查看生成的 C 代码
cat tests\test_example.c

# 用 GCC 编译
C:\msys64\mingw64\bin\gcc.exe tests\test_example.c -o tests\test_example.exe

# 用 Clang 编译
C:\msys64\clang64\bin\clang.exe tests\test_example.c -o tests\test_example.exe

# 运行
.\tests\test_example.exe; Write-Host "Exit: $LASTEXITCODE"
```

---

## 五、运行方法

```powershell
# 编译并运行单个 .sp 文件
.\cmake-build-debug\sharpc.exe examples\hello.sp -o hello.c -no-link
C:\msys64\mingw64\bin\gcc.exe hello.c -o hello.exe
.\hello.exe

# 或直接管道输出 C 代码
.\cmake-build-debug\sharpc.exe examples\hello.sp -
```

---

## 六、编译器架构

编译管线按以下顺序执行：

```
.sp 文件 → 预处理器 → 词法分析 → 语法分析 → 语义分析 → 降低(HIR) → C 代码生成 → .c 文件
              ↑            ↑           ↑            ↑           ↑              ↑
           preproc/     lexer.c     parser.c     sema.c     lower.c        cgen.c
           sharp-cpp                                         hir.c
```

### 各模块职责

| 模块 | 文件 | 职责 |
|------|------|------|
| **预处理器** | `preproc/` | 宏展开、`#include`、`#if/#ifdef/#define` |
| **词法分析** | `lexer.c` | 源码 → Token 流（关键字、标识符、运算符等） |
| **语法分析** | `parser.c` | Token 流 → AST（`Node` 树） |
| **语义分析** | `sema.c` | 类型检查、作用域管理、变量/函数声明验证 |
| **HIR** | `hir.c`, `lower.c` | 高级中间表示（优化/变换用） |
| **C 代码生成** | `cgen.c` | AST → C 源代码 |
| **类型系统** | `types.c`, `types.h` | 类型定义、类型比较、类型字符串化 |
| **工具函数** | `util.c` | 字符串缓冲（`sb_*`）、诊断（`diag_*`） |

### AST 节点类型（`sharp.h` 中的 `NodeKind`）

关键节点类型：
- `ND_PROGRAM` — 根节点，包含所有顶层声明
- `ND_FUNC_DECL` — 函数声明
- `ND_STRUCT_DECL` / `ND_UNION_DECL` — 结构体/联合体声明
- `ND_BLOCK` — 语句块
- `ND_VARDECL` — 局部变量声明
- `ND_IF` / `ND_WHILE` / `ND_FOR` — 控制流
- `ND_RETURN` — 返回语句
- `ND_CALL` — 函数调用
- `ND_BINOP` / `ND_UNOP` — 二元/一元运算
- `ND_SEH` — SEH 异常处理（`__try/__except/__finally`）
- `ND_DECL_SPEC` — `__declspec` 局部变量声明
- `ND_EXTERN_DECL` — `extern` 函数声明
- `ND_EXTERN_VAR` — `extern` 变量声明
- `ND_TYPEDEF_DECL` — `typedef` 声明
- `ND_CAST` — 类型转换 `(Type)expr`
- `ND_PRINT` / `ND_PRINTLN` — 内置输出函数
- `ND_DROP` — RAII drop 调用

---

## 七、当前状态（158/161 UCRT 头文件通过）

### 测试结果

| 指标 | 结果 |
|------|------|
| UCRT 头文件测试 | 158 PASS / 0 FAIL / 3 SKIP / 161 TOTAL |
| sharpc 解析所有 UCRT 头文件 | 15/15 全部通过 |
| clang C11 语法验证 | 13/15 通过 |

### 已知限制（跳过测试）

| 测试 | 状态 | 原因 |
|------|------|------|
| `test_tcc_basetsd_h` | ⏭ SKIP | TCC basetsd.h 已知兼容问题 |
| `test_tcc_basetyps_h` | ⏭ SKIP | TCC basetyps.h 已知兼容问题 |
| `test_tcc_file_h` | ⏭ SKIP | TCC file.h 已知兼容问题 |

### clang 验证已知问题

| 头文件 | 问题 | 原因 |
|--------|------|------|
| `corecrt_wctype.h` | `unknown type name 'wctype_t'` | 需要 `<wchar.h>` 提供 `wctype_t` 定义，Windows SDK 依赖问题 |
| `mbctype.h` | `unknown type name 'wctype_t'` | 同上 |

### 最近修复的功能（按时间倒序）

1. **匿名 union/struct AST 顺序修复** — 匿名 union/struct 定义现在在 typedef 引用之前输出
2. **匿名 enum 定义** — `typedef enum { ... } Name;` 现在完整输出 enum 定义
3. **函数指针 typedef codegen** — `_PVFV*` 不再展开为 `void(*)(void)*`，保留为 `_PVFV*`
4. **SAL 内部宏定义** — 添加 `_SA_annotes0/1/2/3`、`_SAL_nop_impl_`、`_VCRT_*` 等约 50 个 SAL 宏
5. **Windows SDK UCRT include 路径** — 添加 VC include + Windows Kits UCRT fallback 路径
6. **Enum typedef 前向声明** — 枚举定义在引用它的 typedef 之前输出

---

## 八、关键开发指南

### 添加新的 AST 节点类型
1. 在 `src/sharp.h` 的 `NodeKind` 枚举中添加新类型
2. 在 `Node` 结构体中添加需要的字段（如 `try_body`, `except_expr` 等）
3. 在 `src/parser.c` 中添加解析逻辑
4. 在 `src/sema.c` 的 `tc_stmt` 函数中添加语义分析
5. 在 `src/cgen.c` 的 `emit_stmt_core` 函数中添加 C 代码生成

### 添加新的 Token/关键字
1. 在 `src/sharp.h` 的 `TokKind` 枚举中添加
2. 在 `src/lexer.c` 的关键字查找表中注册
3. 在 `src/parser.c` 中添加对应的解析逻辑

### 调试技巧
```powershell
# 查看 sharpc 生成的 C 代码
.\cmake-build-debug\sharpc.exe test.sp -

# 如果 sema 报错，检查 src/sema.c 中的 tc_* 函数
# 如果 parser 报错，检查 src/parser.c 中的解析逻辑
# 如果生成的 C 代码不正确，检查 src/cgen.c 中的 emit_* 函数

# 强制重新编译（解决缓存问题）
Remove-Item cmake-build-debug\CMakeFiles\sharpc.dir\src\*.c.obj -Force
C:\msys64\clang64\bin\cmake.exe --build cmake-build-debug --target sharpc -j 30
```

### Node 字段使用约定
- `children`/`nchildren` — 子节点列表（`ND_BLOCK`, `ND_PROGRAM`）
- `lhs`/`rhs` — 二元运算的左右操作数
- `cond`/`then_b`/`else_b` — if 语句的条件/真分支/假分支
- `body` — 循环体/函数体
- `init`/`update` — for 循环的初始化和更新
- `declared_type`/`name` — 变量/函数声明的类型和名称
- `try_body`/`except_expr`/`except_body`/`finally_body` — SEH 节点专用字段
- `declspec` — `__declspec` 内容（如 `"thread"`, `"dllimport"`）

---

## 九、测试文件约定

### `/* compiler: xxx */` 注释
每个 `.sp` 测试文件头部可以指定目标编译器：
- `/* compiler: any */` — 任何编译器（默认）
- `/* compiler: gcc */` — 仅 GCC/Clang
- `/* compiler: msvc */` — 仅 MSVC

### 测试退出码
测试通过的标准是程序退出码等于预期值。测试脚本会从测试文件名推断预期退出码，或默认为 0。

---

## 十、Git 历史

最新提交：
```
608a35a fix: 修复 SEH 块内 SP 代码不被解析、添加 __builtin_frame_address 支持
471e403 __declspec
1a0f9ca __declspec
5ebaa8f __declspec
7607e03 __declspec
```

---

## 十一、常见问题排查

| 问题 | 排查方法 |
|------|----------|
| sharpc 编译后行为没变化 | 确认 `.obj` 文件被重新编译，强制删除后重建 |
| "call to undeclared function" | 检查 `sema.c` 中的 `tc_call` 函数，确认函数名查找逻辑 |
| 生成的 C 代码类型不对 | 检查 `cgen.c` 中的 `emit_type` / `emit_vardecl_inline` 函数 |
| 测试失败但 C 代码看起来正确 | 可能是 C 编译器问题，尝试用另一个编译器（GCC ↔ Clang） |
| `strncmp("__builtin_", 10)` 注意长度 | 字符串 `"__builtin_"` 是 10 个字符，不是 12 |

---

*本文档由 AI 助手生成，用于项目交接。如有疑问，请查阅源码或语言规范文档。*
