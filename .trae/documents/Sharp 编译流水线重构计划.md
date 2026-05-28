# Sharp 编译流水线重构计划

## 背景

### 架构说明

- **sharp-cpp**: Sharp 项目的 C 预处理器，兼容完整 C11，处理所有 `.c`/`.sp`/`.sph` 文件的预处理
- **sharpc**: 编译器前端驱动器，调用 sharp-cpp 进行预处理，然后进行 lex/parse/sema/cg
- **sharpc 是 Sharp 语言的编译器前端**：将 Sharp 语言（.sp）和 C 语言（.c）转换为 C11 代码，然后通过 zig cc 编译

### 旧流水线
```
源文件 (.c/.sp)
    │
    ▼
[1] sharp-cpp 预处理展开 (cpp_run_buf)
    │  输出：展开后的 token 流 + linemarker 文本
    ▼
[2] 从 linemarker 重新收集 #include 指令 (compile_one_file L946-1018)
    │  调用 file_add_include() 注入 AST
    ▼
[3] lex → parse → scope → sema
    │
    ▼
[4] cg 代码生成
    │  收集 needed_includes（系统头文件）
    │  在输出头部注入 #include 指令
    ▼
[5] 输出带 #include 的 .c 临时文件
    │
    ▼
[6] zig cc -c → .o
```

### 旧流水线关键逻辑分析

**为什么预处理已经展开了 #include，还需要重新收集？**

答案：旧流水线输出的是"带 #include 的 .c 文件"，而不是完全展开的代码。
- cg.c 在输出头部注入 `#include "xxx"` 和 `#include <xxx>` 指令
- zig cc 收到这个 .c 文件后，会重新查找并展开这些头文件
- 这样做是因为 sharp-cpp 已经展开了头文件内容，但 cg 需要告诉 zig cc 这些内容来自哪些头文件（用于类型定义等）

### 期望新流水线
```
源文件 (.c/.sp)
    │
    ▼
[1] sharp-cpp 预处理完全展开 (cpp_run_buf)
    │  输出：展开后的 token 流 + linemarker（保留调试信息）
    ▼
[2] lex → parse → scope → sema
    │  不再从 linemarker 收集 #include
    ▼
[3] cg 代码生成
    │  输出完全展开的 C 代码
    │  不注入任何 #include 指令
    ▼
[4] 输出自包含 .i 文件（无 #include）
    │
    ▼
[5] zig cc -x c -c → .o
```

### 新流水线关键变化

1. **sharp-cpp 仍然完全展开**：所有 #include 的内容都已经被预处理展开
2. **不再收集 linemarker**：不需要从 linemarker 中提取 #include 信息
3. **cg 不注入 #include**：输出的 .i 文件是完全自包含的
4. **zig cc 直接编译 .i**：使用 `-x c` 告诉 zig cc 这是 C 代码（.i 扩展名默认是预处理器输出）

## 核心变更

### 变更 1：删除 linemarker #include 收集逻辑

**位置**: `sharpc.c::compile_one_file()` L946-1018

**说明**: 这段代码从预处理输出的 linemarker 中提取文件进入/退出信息，然后调用 `file_add_include()` 将 #include 指令注入 AST。在新流水线中，由于 cg 不再输出 #include，这段逻辑可以删除。

**删除内容**:
- L946-1018 的整个 linemarker 解析循环
- `file_add_include()` 相关调用
- 相关变量 `stk[256]`, `stkdepth`, `fname[512]` 等

### 变更 2：删除 cg.c 中的 #include 注入逻辑

**位置**: `cg.c::cg_file()` L7930-7933

**说明**: 当前 cg 在文件头部输出用户显式的 #include 指令。新流水线中，所有头文件内容已被完全展开，不需要输出 #include。

**需要修改的内容**:
- 删除 `cg_file()` L7930-7933 的 #include 输出循环
- 删除 `CgCtx.needed_includes` 字段（L105）
- 删除 `cg_set_sys_dirs()` 函数（L217-221）
- 删除 `cg_needed_includes()` 函数（L223-224）
- 删除收集 needed_includes 的逻辑（L655-659 附近）
- 删除 `cg_is_sys_path()` 函数（L135-148）
- 清理 `cg_ctx_free()` 中释放 needed_includes 的逻辑（L213）
- 更新 `sharpc.c` 中调用 `cg_set_sys_dirs()` 的代码（L1046-1047）

### 变更 3：调整输出文件扩展名

**位置**: `sharpc.c` 多处

**说明**: 旧流水线输出 `.c` 临时文件，新流水线应输出 `.i` 文件（标准预处理器输出格式）。

**需要修改的位置**:
- `make_tmp_name(inf->path, ".c")` → `make_tmp_name(inf->path, ".i")` (L1551, L1649)
- ACTION_ASSEMBLY 分支中的临时文件扩展名 (L1453)
- ACTION_COMPILE_ONLY 中的 `.c` 扩展名判断逻辑 (L1488-1489)
- ACTION_ASSEMBLY 中的 `.c` 扩展名判断 (L1453)

### 变更 4：调整 zig cc 调用参数

**位置**: `sharpc.c` 的 zig cc 调用处

**说明**: 新流水线中，zig cc 接收 `.i` 文件时需要使用 `-x c` 参数指定语言类型（因为 `.i` 扩展名默认被视为预处理器输出，zig cc 可能不对其进行进一步预处理）。

**需要修改的位置**:
- ACTION_COMPILE_ONLY 分支 (L1560-1564, L1660-1664)
- ACTION_ASSEMBLY 分支 (L1460)
- ACTION_LINK 分支 (L1660-1664)
- 添加 `-x c` 参数到 zig cc argv

### 变更 5：保留 linemarker 用于调试信息

**位置**: `sharpc.c::compile_one_file()` L895, `preprocess_one_file()` L809

**说明**: linemarker 仍然需要保留以支持调试信息（`-g` 选项）和错误报告。

**不需要修改**: 保持 `cpp_emit_linemarkers(cctx, true)`

### 变更 6：处理 `-E` 预处理选项

**位置**: `sharpc.c::preprocess_one_file()` 和 ACTION_PREPROCESS 分支

**说明**: 当用户执行 `sharpc -E` 时，应输出完全展开的 `.i` 格式（无 `#include`，但有 linemarker）。

**需要修改**: 无需修改，`preprocess_one_file()` 已经输出完全展开的代码

### 变更 7：处理 `.S` 汇编文件

**位置**: `sharpc.c` 的 `.S` 文件处理分支

**说明**: `.S` 文件（需要预处理的汇编文件）的处理逻辑不需要变更，因为它不涉及 #include 注入。

## 实施步骤

### Step 1: 删除 linemarker #include 收集逻辑

1. 删除 `compile_one_file()` L946-1018 的 linemarker 解析循环
2. 删除相关变量声明（stk, stkdepth, fname 等）
3. 确认 `file_add_include()` 不再被调用
4. 可以保留 `file_add_include()` 函数定义（在 parse.c 中），因为它是公共 API

### Step 2: 删除 cg.c 中的 #include 注入逻辑

1. 删除 `cg_file()` L7930-7933 的 #include 输出循环
2. 删除 `CgCtx.needed_includes` 字段（L105）
3. 删除 `cg_set_sys_dirs()` 函数（L217-221）
4. 删除 `cg_needed_includes()` 函数（L223-224）
5. 删除 `cg_is_sys_path()` 函数（L135-148）
6. 删除收集 needed_includes 的逻辑（L655-659 附近）
7. 清理 `cg_ctx_free()` 中释放 needed_includes 的逻辑（L213）
8. 删除 `sharpc.c` 中调用 `cg_set_sys_dirs()` 的代码（L1046-1047）

### Step 3: 修改临时文件扩展名

1. 将所有 `.c` 临时文件改为 `.i`
2. 更新相关的扩展名判断逻辑

### Step 4: 调整 zig cc 调用

1. 在 zig cc 命令中添加 `-x c` 参数
2. 确认所有 zig cc 调用点都已更新

### Step 5: 测试和验证

1. 编译 sharpc 自身
2. 运行现有测试套件
3. 验证输出的 `.i` 文件可以被 zig cc 正确编译
4. 验证 `.sp` 和 `.c` 文件都能正确处理
5. 验证 `-E`, `-c`, `-S`, link 等所有动作模式

## 风险和注意事项

1. **调试信息**: linemarker 必须保留以支持 `-g` 调试信息
2. **错误报告**: linemarker 用于错误报告中的文件/行号信息
3. **依赖生成**: `-MD`/`-MMD` 依赖文件生成不受影响（当前使用 `r.included_files`）
4. **向后兼容**: 旧流水线输出 `.c` 文件，新流水线输出 `.i` 文件，可能影响外部工具
5. **Sharp 特有逻辑**: Sharp 语言的 `@` 内省原语和特有语法不受影响（仍然通过 lex/parse/sema/cg 处理）

## 验证清单

- [ ] sharpc 编译成功
- [ ] `.c` 文件编译为 `.o` 成功
- [ ] `.sp` 文件编译为 `.o` 成功
- [ ] `-E` 选项输出正确的 `.i` 格式
- [ ] `-c` 选项输出 `.o` 文件
- [ ] `-S` 选项输出 `.s` 文件
- [ ] `-o` 指定输出文件名正确
- [ ] `-g` 调试信息正确
- [ ] `-MD`/`-MMD` 依赖文件生成正确
- [ ] `-I`/`-isystem` 包含路径正确
- [ ] `-D`/`-U` 宏定义正确
- [ ] 泛型特化正确
- [ ] 运行现有测试套件通过
