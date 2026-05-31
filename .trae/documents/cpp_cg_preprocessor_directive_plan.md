# CPP 和 CG 预处理指令架构修改计划

## 问题分析

### 当前问题

用户在 `.he` 头文件中写的预处理指令（如 `#define`、`#undef`、`#pragma`）在 CG 输出时丢失了：

```c
// test.he
#define __USE_POSIX
#include <stdio.h>
#undef __USE_POSIX

void foo(void) { printf("hello\n"); }
```

CG 输出：

```c
// 只有函数定义，#define 和 #undef 丢失了
void foo(void) { printf("hello\n"); }
```

### 根因

当前架构：

```
源文件 → CPP 预处理 → Token 流 → Parser → AST → CG → C 代码
```

CPP 预处理阶段把 `#define`、`#undef` 等指令处理掉了，只留下处理后的 token。CG 阶段无法恢复这些指令。

### 为什么这是个问题

预处理指令是有语义的：
- `#define __USE_POSIX` 会影响后续 `#include <stdio.h>` 的行为
- `#pragma pack(1)` 会影响结构体布局
- `#undef` 会取消之前的宏定义

如果丢失这些指令，生成的 C 代码语义就和原文件不一致。

## 架构方案

### 方案 A：CPP 记录原始指令（推荐）

在 CPP 预处理阶段记录所有遇到的预处理指令，保留原始文本，传递给后续阶段。

```
源文件 → CPP 预处理 → Token 流 + 预处理指令列表 → Parser → AST → CG → C 代码
                                    ↓
                              记录 #define、#undef、#pragma 等原始文本
```

**实现步骤**：

1. **修改 CppResult 结构**（cpp.h）
   - 添加 `preprocessor_directives` 数组，记录每个预处理指令的：
     - 原始文本（如 `#define __USE_POSIX`）
     - 文件路径
     - 行号

2. **修改 CPP 处理逻辑**（cpp.c）
   - 在处理 `#define`、`#undef`、#pragma`、`#error`、`#warning` 等指令时
   - 记录原始指令文本到 `preprocessor_directives` 数组

3. **修改 sharpc.c**
   - 从 `CppResult.preprocessor_directives` 提取指令
   - 传递给 AST 或 CG

4. **修改 CG**（cg.c）
   - 在输出文件开头，按原始顺序输出预处理指令
   - 或者在每个 `#line` 指令前输出该位置的预处理指令

**优点**：
- 不改变 CPP 的核心逻辑
- 保留原始指令的精确文本
- 支持所有预处理指令类型

**缺点**：
- 需要修改 CPP 模块

### 方案 B：保留原始源文件文本

在 AST 中保留原始源文件的文本，CG 输出时直接复制预处理指令部分。

**缺点**：
- 需要区分哪些是预处理指令，哪些是代码
- 难以处理宏展开后的代码

### 方案 C：两阶段处理

第一阶段：只提取预处理指令
第二阶段：正常预处理和编译

**缺点**：
- 需要两次扫描源文件
- 复杂度高

## 推荐方案：方案 A

### 详细实现步骤

#### Step 1：修改 CppResult 结构

```c
// cpp.h
typedef struct {
    const char *text;      // 原始指令文本，如 "#define __USE_POSIX"
    const char *file;      // 文件路径
    int line;              // 行号
} CppDirective;

typedef struct CppResult {
    // ... 现有字段 ...
    
    CppDirective *directives;     // 预处理指令数组
    size_t ndirectives;           // 指令数量
} CppResult;
```

#### Step 2：修改 CPP 处理逻辑

在 `handle_define`、`handle_undef`、`handle_pragma` 等函数中，记录原始指令：

```c
// cpp.c
static void handle_define(CppCtx *ctx, ...) {
    // 记录原始指令
    CppDirective *d = &ctx->result.directives[ctx->result.ndirectives++];
    d->text = xstrdup(ctx->current_line);  // 如 "#define __USE_POSIX"
    d->file = xstrdup(ctx->current_file);
    d->line = ctx->current_line;
    
    // 原有处理逻辑 ...
}
```

#### Step 3：修改 sharpc.c

```c
// sharpc.c
CppResult r = cpp_preprocess(cctx, input, sys_inc, defines);

// 提取预处理指令
for (size_t i = 0; i < r.ndirectives; i++) {
    file_add_directive(ast, r.directives[i].text);
}
```

#### Step 4：修改 AST

```c
// ast.h
typedef struct AstNode {
    // ...
    struct {
        // ...
        char **user_directives;     // 预处理指令
        size_t nuser_directives;
    } file;
} AstNode;
```

#### Step 5：修改 CG

```c
// cg.c
static void cg_file(CgCtx *ctx, const AstNode *file) {
    // 输出预处理指令
    for (size_t i = 0; i < file->u.file.nuser_directives; i++) {
        cg_puts(ctx, file->u.file.user_directives[i]);
        cg_puts(ctx, "\n");
    }
    
    // 输出 #include
    for (size_t i = 0; i < file->u.file.nuser_includes; i++) {
        cg_puts(ctx, file->u.file.user_includes[i]);
        cg_puts(ctx, "\n");
    }
    
    // 输出声明和函数 ...
}
```

### 需要处理的指令类型

1. `#define` - 宏定义
2. `#undef` - 取消宏定义
3. `#pragma` - 编译器指示
4. `#error` - 错误指示
5. `#warning` - 警告指示
6. `#line` - 行号指示（可能需要特殊处理）

### 不需要处理的指令

- `#include` - 已经单独处理
- `#if`、`#ifdef`、`#ifndef`、`#else`、`#elif`、`#endif` - 条件编译，CPP 已经处理

## 执行步骤

### 第一阶段：提交当前修复

1. 提交当前代码修改（禁用 preamble、正确处理 include）
2. 推送到远程仓库

### 第二阶段：实现预处理指令保留

1. 修改 `cpp.h`：添加 `CppDirective` 结构和 `CppResult.directives` 字段
2. 修改 `cpp.c`：在处理预处理指令时记录原始文本
3. 修改 `ast.h`：添加 `user_directives` 字段
4. 修改 `sharpc.c`：从 `CppResult` 提取指令并传递给 AST
5. 修改 `cg.c`：输出预处理指令
6. 测试验证

## 测试用例

```c
// test_directive.he
#define __USE_POSIX
#include <stdio.h>
#undef __USE_POSIX

#pragma pack(push, 1)
struct Packed {
    char a;
    int b;
};
#pragma pack(pop)

void foo(void) { printf("hello\n"); }
```

期望输出：

```c
// test_directive.he.gen.c
#define __USE_POSIX
#include <stdio.h>
#undef __USE_POSIX

#pragma pack(push, 1)
struct Packed {
    char a;
    int b;
};
#pragma pack(pop)

void foo(void) { printf("hello\n"); }
```
