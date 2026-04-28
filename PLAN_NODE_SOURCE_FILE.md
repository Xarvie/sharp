# 设计文档：Node 来源追踪系统

## 1. 动机

当前编译器在 Codegen 阶段无法区分 AST 节点的来源（用户源码 vs 系统头文件），
导致 `emit_typedef_decl` 使用脆弱的启发式规则（类型名是否简单）来决定是否跳过
CRT typedef。这在商业级产品中不可接受——正确的行为应该是基于节点的**来源文件**
来判断。

## 2. 架构设计

### 2.1 核心概念

每个 AST Node 携带其来源文件的完整路径。来源信息在 Parser 创建 Node 时从
Lexer 的 `filename` 字段捕获，之后永不改变。

```
用户 .sp 文件 → p->lex->filename = "c:/Users/.../tests/test_feat6_wchar.sp"
系统头文件   → p->lex->filename = "C:/Program Files/.../ucrt/wchar.h"
```

### 2.2 数据流

```
Lexer (filename 字段)
    │
    ▼
Parser.mk() → Node.source_file = p->lex->filename
    │
    ▼
AST (每个 Node 自带来源)
    │
    ▼
Codegen → is_user_source(node->source_file) → 决定策略
```

## 3. 修改清单

### 3.1 `src/sharp.h` — Node 结构体

**位置**: [sharp.h:L350-426](file:///c:\Users\ftp\Desktop\sharp\src\sharp.h#L350-L426)

**修改**: 在 `struct Node` 中添加 `source_file` 字段

```c
struct Node {
    NodeKind     kind;
    Type*        type;
    int          line;

    /* Phase C1: Source file tracking — identifies whether this node
     * originated from user source code or a system/SDK header.
     * Set by parser's mk() from p->lex->filename at creation time. */
    const char*  source_file;    /* full path, NULL if unavailable */

    /* children list (ND_PROGRAM, ND_BLOCK) */
    Node**       children;
    ...
```

**兼容性**: 零破坏性。新增字段在结构体头部，不影响已有字段的偏移量。
所有现有代码只需在 `mk()` 中设置即可。

### 3.2 `src/parser.c` — mk() 函数

**位置**: [parser.c:L26-31](file:///c:\Users\ftp\Desktop\sharp\src\parser.c#L26-L31)

**修改**: 在 `mk()` 中自动设置 `source_file`

```c
static Node* mk(P* p, NodeKind k, int line) {
    Node* n = ARENA_NEW(p->arena, Node);
    n->kind = k;
    n->line = line;
    n->source_file = p->lex->filename;  /* Phase C1: track source */
    return n;
}
```

**影响**: 所有通过 `mk()` 创建的 Node（即**所有** AST 节点）自动获得
来源信息。无需逐个修改 7 处 `ND_TYPEDEF_DECL` 创建点。

### 3.3 `src/cgen.c` — 统一系统头检测

**新建函数**: `is_system_header(const char* path)`

提取当前散落在 `parser.c` 中 6 处的重复检测逻辑为一个统一函数：

```c
/* Phase C1: Unified system header detection.
 * Returns true if the given file path belongs to a system/SDK header.
 * Used by both parser (inline function skip) and codegen (typedef skip). */
static bool is_system_header(const char* path) {
    if (!path) return false;
    /* TCC/MinGW include paths */
    if (strstr(path, "tcc/include") != NULL) return true;
    if (strstr(path, "third_party\\tcc\\include") != NULL) return true;
    if (strstr(path, "third_party/tcc/include") != NULL) return true;
    if (strstr(path, "mingw") != NULL) return true;
    if (strstr(path, "msys64") != NULL) return true;
    if (strstr(path, "/usr/include") != NULL) return true;
    /* Windows SDK / UCRT */
    if (strstr(path, "Windows Kits") != NULL) return true;
    if (strstr(path, "Microsoft Visual Studio") != NULL) return true;
    return false;
}
```

**位置**: 放在 `is_crt_sdk_typedef` 之前。

### 3.4 `src/cgen.c` — 重写 `emit_typedef_decl`

**位置**: [cgen.c:1683-1710](file:///c:\Users\ftp\Desktop\sharp\src\cgen.c#L1683-L1710)

**新逻辑**: 基于来源判断，完全删除 `type_is_c_primitive` 启发式

```c
static void emit_typedef_decl(G* g, Node* d) {
    /* Phase C1: Skip typedefs from system headers that have names in the
     * CRT skip list. User-defined typedefs (from .sp files) are ALWAYS
     * emitted, regardless of name. */
    if (is_system_header(d->source_file) && is_crt_sdk_typedef(d->name)) {
        return;  /* system typedef — skip */
    }

    /* User-defined typedef — emit normally */
    Type* dt = d->declared_type;
    if (dt && dt->kind == TY_NAMED) {
        SymStruct* ss = sema_find_struct(g->st, dt->name);
        if (ss) {
            sb_printf(&g->out, "typedef struct %s %s", dt->name, d->name);
            if (d->lhs) {
                sb_putc(&g->out, '[');
                emit_expr(g, d->lhs);
                sb_putc(&g->out, ']');
            }
            sb_puts(&g->out, ";\n");
            return;
        }
    }
    sb_puts(&g->out, "typedef ");
    emit_type_with_name(g, dt, d->name);
    if (d->lhs) {
        sb_putc(&g->out, '[');
        emit_expr(g, d->lhs);
        sb_putc(&g->out, ']');
    }
    sb_puts(&g->out, ";\n");
}
```

**对比旧逻辑**:

| 场景 | 旧逻辑（启发式） | 新逻辑（来源追踪） |
|------|-----------------|-------------------|
| `typedef int wchar_t;` (用户 .sp) | 类型名 "int" 简单 → 生成 ✓ | source_file 是用户路径 → 生成 ✓ |
| `typedef unsigned long long size_t;` (系统头) | 类型名复合 → 跳过 ✓ | source_file 是系统路径 → 跳过 ✓ |
| `typedef unsigned long long my_type;` (用户 .sp) | 类型名复合 → 跳过 ✗ **BUG** | source_file 是用户路径 → 生成 ✓ |

**新逻辑完全消除了类型名启发式的局限性**。

### 3.5 删除 `type_is_c_primitive` 函数

**位置**: [cgen.c:1662-1679](file:///c:\Users\ftp\Desktop\sharp\src\cgen.c#L1662-L1679)

此函数是启发式方案的产物，新方案中不再需要。完全删除。

### 3.6 `src/parser.c` — 统一系统头检测调用

**位置**: [parser.c:3232-3238](file:///c:\Users\ftp\Desktop\sharp\src\parser.c#L3232-L3238)

将 Parser 中的内联系统头检测替换为调用 `is_system_header()`。
但注意 `is_system_header` 在 cgen.c 中，需要决定是放在 shared 文件还是声明 extern。

**方案选择**: 将 `is_system_header` 放在 `sharp.h` 中作为 inline 函数，
或在 `cgen.c` 中定义并在 `parser.c` 中声明 extern。

最优方案：放在 `sharp.h` 中作为 `static inline` 函数，两处都可使用：

```c
/* sharp.h */
static inline bool is_system_header_path(const char* path) {
    if (!path) return false;
    if (strstr(path, "tcc/include") != NULL) return true;
    if (strstr(path, "third_party\\tcc\\include") != NULL) return true;
    if (strstr(path, "third_party/tcc/include") != NULL) return true;
    if (strstr(path, "mingw") != NULL) return true;
    if (strstr(path, "msys64") != NULL) return true;
    if (strstr(path, "/usr/include") != NULL) return true;
    if (strstr(path, "Windows Kits") != NULL) return true;
    if (strstr(path, "Microsoft Visual Studio") != NULL) return true;
    return false;
}
```

**好处**: 单一来源定义，零重复代码。Parser 和 Codegen 共享同一实现。

### 3.7 `src/parser.c` — 替换内联系统头检测

查找并替换所有以下模式：

```c
/* 旧代码（出现在 6 处） */
const char* cur_file = p->lex->filename;
bool is_system_header = (cur_file && (strstr(cur_file, "tcc/include") != NULL ||
                                       ... ));
```

替换为：

```c
/* 新代码 */
bool from_system = is_system_header_path(p->lex->filename);
```

需要替换的位置：
1. `parser.c` 中 `extern __inline__` body skip 逻辑 (L3231-3238)
2. 其他 5 处重复检测（需全面搜索确认）

## 4. 风险与缓解

### 4.1 Arena 内存增加

每个 Node 增加 8 bytes (64-bit 指针)。以典型 UCRT 头文件 ~5000 节点计算，
额外内存约 40 KB — 完全可接受。

### 4.2 向后兼容性

`Node` 结构体是内部数据结构，不暴露给外部 API。零兼容性风险。

### 4.3 系统头检测的完备性

`is_system_header_path` 的路径匹配覆盖：
- Windows SDK (Windows Kits, Microsoft Visual Studio)
- MinGW/MSYS (mingw, msys64, /usr/include)
- TCC (tcc/include, third_party/tcc/include)

如果未来引入新的 SDK 路径，只需在此函数中添加一条规则。
比当前每个 call site 各自维护 6 条规则要容易得多。

## 5. 测试计划

### 5.1 回归测试

运行 `python run_tests.py` — 必须保持 158/161 PASS（当前基线）。

### 5.2 边界测试

新增测试用例验证边界情况：

```c
/* test_source_file_user_struct.sp */
/* 用户代码定义系统同名的 struct */
typedef struct { int x; } _locale_t;
int main() { return 0; }
```
→ 生成的 C 代码应包含 `typedef struct { int x; } _locale_t;`

```c
/* test_source_file_user_compound.sp */
/* 用户代码使用复合类型 */
typedef unsigned long long uint64;
int main() { return 0; }
```
→ 生成的 C 代码应包含 `typedef unsigned long long uint64;`
（旧启发式会错误跳过这个）

### 5.3 验证系统头 typedef 仍被跳过

```c
/* test_source_file_system_skip.sp */
/* auto */
#include <stddef.h>
int main() { return 0; }
```
→ 生成的 C 代码不应重复定义 `size_t`

## 6. 修改文件汇总

| 文件 | 修改类型 | 行数估计 |
|------|---------|---------|
| `src/sharp.h` | 添加 `source_file` 字段 + `is_system_header_path()` | +15 行 |
| `src/parser.c` | 修改 `mk()` + 替换 6 处内联检测 | ~10 行修改 |
| `src/cgen.c` | 重写 `emit_typedef_decl` + 删除 `type_is_c_primitive` | ~5 行修改 |
| `tests/` | 新增 2-3 个边界测试 | +50 行 |

**总计**: ~80 行改动，4 个文件。

## 7. 执行顺序

1. `sharp.h` — 添加 `source_file` 字段
2. `sharp.h` — 添加 `is_system_header_path()` 函数
3. `parser.c` — 修改 `mk()` 设置 `source_file`
4. `parser.c` — 替换内联系统头检测
5. `cgen.c` — 删除 `type_is_c_primitive`
6. `cgen.c` — 重写 `emit_typedef_decl`
7. 编译验证
8. 运行回归测试
9. 新增边界测试
