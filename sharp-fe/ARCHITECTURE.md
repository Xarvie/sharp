# Sharp Frontend — 代码结构与流程

## 总体流程

```
源文件 (.c / .ce)
      │
      ▼
 ┌─────────┐   #include展开   ┌───────────┐
 │ sharpc  │ ──────────────▶ │  sharp-  │
 │ (driver)│ ◀── tokens ──── │   cpp/   │
 └─────────┘                 └───────────┘
      │
      ▼  CppResult (token数组 + 行号信息)
 ┌─────────┐
 │  lex.c  │  SharpTok[]  (.sp模式识别null/defer等关键字)
 └─────────┘
      │
      ▼  SharpTok[]
 ┌──────────┐
 │ parse.c  │  → AstNode* (AST树, 根节点 AST_FILE)
 └──────────┘
      │
      ▼  AstNode*
 ┌──────────┐
 │ scope.c  │  → Scope链 (符号表, 两遍: 先注册顶层, 再递归函数体)
 └──────────┘
      │
      ▼  Scope*
 ┌──────────┐
 │  sema.c  │  类型推断, 错误检查  (标注 AstNode.type_ref)
 └──────────┘
      │
      ▼  (标注后的 AstNode*)
 ┌──────────┐
 │   cg.c   │  代码生成 → 输出 C11 字符串
 └──────────┘
      │
      ▼  char* (C11 source)
   输出文件 / stdout
```

---

## 各模块职责

### `sharpc.c` — 驱动器

入口 `main()`，完成：
- 解析命令行（-I / -D / -o 等）
- 按顺序调用各管道阶段
- 收集并打印所有诊断信息（词法/解析/语义）
- 将 CG 输出写入文件或 stdout

**当前架构**（spec v0.13 对齐）：

`.c` 与 `.ce` 共享同一条编译管线，无分支：
- **CPP** 统一处理所有文件（`@` 固有语素始终识别）
- **`lex_run`** 始终提升 Sharp 关键字（`defer`/`operator`/`this`）
- **CG** 统一走多遍 Sharp 路径，无 C-mode 分支

---

### `sharp-cpp/` — C 预处理器

独立子模块，实现完整 C11 预处理器（`#include` / `#define` / `#if` 等）。
输出 `CppResult`：展开后的 token 数组 + 行号位置信息（`CppLoc`）。

sharpc 通过 `cpp_run_buf()` 调用，不需要写临时文件。

---

### `lex.c` — 词法分析

输入：`CppResult`（预处理后的 token 流）
输出：`SharpTok[]`

主要工作：
- 将 CPP token 映射为 `SharpTokKind` 枚举值
- 始终将 `defer`/`operator`/`this` 提升为 Sharp 关键字
- 记录每个 token 的源文件位置（`CppLoc`：file + line + col）

```c
typedef struct {
    SharpTokKind kind;
    const char  *text;   // 指向 CPP interner（不拥有）
    size_t       len;
    CppLoc       loc;
} SharpTok;
```

---

### `parse.c` — 语法分析（最大，~4600行）

输入：`SharpTok[]`
输出：`AstNode*`（根节点 `AST_FILE`）

递归下降解析器，核心入口：
```
parse_file()
  └─ 循环调用 parse_top_decl()
        ├─ parse_declaration_specifiers()  解析类型说明符序列
        ├─ parse_declarator()              解析声明符（名称+指针+数组+函数）
        ├─ parse_struct_def()              struct/union 体
        ├─ parse_enum_def()                enum 体
        ├─ parse_init_declarator_list()    逗号变量列表 int x, y, z
        └─ finish_func()                   函数定义体
              └─ parse_block()
                    └─ parse_stmt() / parse_expr()
```

**关键设计**：
- `DeclSpecs` 结构体：在解析说明符序列时积累 `is_const` / `is_volatile` / `storage` / `base_ty` 等，最后一次性构建 AST 类型节点
- `pending_decls`：解析 `typedef struct X { ... } X` 时，struct 体先入队，typedef 随后，保证 file.decls 顺序正确
- S5 提升：`scope.c` 在注册符号时将 forward decl → full body 升级

**类型 AST 节点**（均为 `AstNode*`，不是 `Type*`）：

| 节点 | 含义 |
|---|---|
| `AST_TYPE_NAME` | 裸类型名（int / MyStruct / ...）|
| `AST_TYPE_PTR` | `T *`，含 `ptr_const/volatile/restrict` |
| `AST_TYPE_CONST` | `const T` |
| `AST_TYPE_VOLATILE` | `volatile T` / `_Atomic T` |
| `AST_TYPE_ARRAY` | `T[N]` |
| `AST_TYPE_FUNC` | `Ret(Params...)` |

---

### `ast.c / ast.h` — AST 定义

定义 `AstNode` 联合体，所有节点公用一个 `AstKind kind` 字段加一个大 `union u`。

```c
struct AstNode {
    AstKind kind;
    CppLoc  loc;          // 源码位置
    void   *type_ref;     // sema 标注的 Type*（可为 NULL）
    union u {
        struct { ... } file;
        struct { ... } struct_def;
        struct { ... } func_def;
        struct { ... } var_decl;
        struct { ... } field_decl;
        // ... 约30种
    } u;
};
```

`AstVec`：动态数组（`data` / `len` / `cap`），用于 fields / params / stmts 等列表。

---

### `scope.c` — 作用域与符号表

输入：`AstNode*`（AST_FILE）
输出：`Scope*`（根作用域）

**Scope 层次**：
```
SCOPE_GLOBAL   （内置类型：int, long, char, ...）
  SCOPE_FILE   （顶层：struct / func / var / typedef）
    SCOPE_STRUCT  （字段 + 方法）
    SCOPE_FUNC    （参数 + 函数体）
      SCOPE_BLOCK （嵌套 {}）
```

每个 `Scope` 持有一个 16 桶哈希表，`Symbol` 单链表解冲突。

**两遍处理**：
1. Pass 1：遍历 `file.decls`，注册所有顶层名称（保证前向引用）
2. Pass 2：递归进入函数体，建立局部作用域

**S5 规则**（forward → body 升级）：
```
struct event;          // 先注册：Symbol.decl = 空体节点
struct event { ... };  // 后遇完整体：existing->decl = 完整体节点
```
同样适用于 enum forward decl、self-referential typedef。

---

### `type.c / type.h` — 语义类型系统

独立于 AST 类型节点的**内化 Type* 层**，用于 sema 阶段的类型推断与检查。

```c
typedef enum {
    TY_VOID, TY_BOOL, TY_CHAR, TY_SHORT, TY_INT, TY_LONG, TY_LONGLONG,
    TY_UCHAR, TY_USHORT, TY_UINT, TY_ULONG, TY_ULONGLONG,
    TY_FLOAT, TY_DOUBLE,
    TY_PTR,    // { Type *base }
    TY_ARRAY,  // { Type *base; int64_t size }
    TY_CONST,  // { Type *base }
    TY_FUNC,   // { Type *ret; Type **params; ... }
    TY_STRUCT, // { char *name; ... }
} TyKind;
```

**有意的简化**（CG 通过 AST 路径弥补）：
- 无 `TY_VOLATILE` / `TY_RESTRICT` / `TY_ATOMIC`（volatile 等在 Type* 层丢失）
- enum 折叠为 `TY_INT`
- typedef 名称解析为底层类型

`TyStore`：全局内化池，相同类型共享同一指针（`ty_ptr(base)` == `ty_ptr(base)`）。

---

### `sema.c` — 语义分析

输入：`AstNode*` + `Scope*` + `TyStore*`
输出：为每个表达式节点设置 `node->type_ref = Type*`

主要工作：
- 表达式类型推断（`sema_expr` 递归）
- 函数调用类型检查
- 赋值兼容性
- 收集语义错误到 `FeDiagArr`

sema 是**宽松的**：很多未知名称返回 `void*` 而不是硬错误（允许 C 宏展开的复杂情形）。

---

### `cg.c` — 代码生成（最大，~7600行）

输入：`AstNode*` + `Scope*` + `TyStore*`
输出：`char*`（C11 源代码字符串）

#### 统一多遍发射路径（`cg_file`）

`.c` 和 `.ce` 文件统一走同一条路径，无模式分支：

- **Phase 1**：对被前向引用的 struct/union 注入 forward decl
- **Phase 1.5**：对被前向引用的函数注入 forward decl
- **Phase 2**：收集泛型特化（generic specialization）
- **Phase 3a**：发射非函数顶层声明（struct/typedef/enum/global）
- **Phase 3.5**：发射泛型 struct 特化体
- **Phase 3.6**：发射泛型函数 forward decl
- **Phase 3b**：发射函数体和 struct 方法体
- **Phase 4**：发射泛型函数特化体

#### 类型发射：两条路径

| 函数 | 输入 | 保留信息 |
|---|---|---|
| `cg_type(ctx, Type*)` | 语义 `Type*` | 基本结构，丢失 volatile/restrict/typedef名 |
| `cg_type_from_ast(ctx, AstNode*)` | AST 类型节点 | 完整保留所有修饰符和名称 |

typedef/struct 的内联发射（`cg_typedef_c`/`cg_emit_decl_sharp`）优先走 `cg_type_from_ast`。

#### Skip 条件（防止重复发射）

- `typedef struct X{} Y`（X≠Y）→ struct 体随 typedef 内联，不单独发射
- `typedef struct X{} X`（自引用）→ 同上，inline body 检测
- 内联嵌套 struct（只对有 body 的 struct 生效）

---

### Phase G — Generic Disambiguation

Sharp generics use angle-bracket syntax (`Vec<T>`, `swap<int>(&a,&b)`) which
is syntactically ambiguous with comparison operators in C-family languages.

### Solution: Pre-scan + `generic_names` set

`parse.c::prescan_generic_defs()` performs a single O(n) token-stream pass
**before** any declarations are parsed.  It populates `PS.generic_names`
(a `TdSet` hash set) with every name that appears in a generic definition:

| Pattern | Example | Registered name |
|---|---|---|
| `struct\|union NAME <` | `struct Vec<T>` | `"Vec"` |
| `<IDENT>…IDENT(` at top level | `<T> void swap(…)` | `"swap"` |

During expression parsing, `IDENT <` is routed to `parse_generic_args()` only
when `IDENT ∈ generic_names`.  Otherwise `<` is treated as a comparison
operator — **zero speculation, zero backtracking** for non-generic names.

### New AST node: `AST_GENERIC_CALL`

`func<int>(a, b)` previously produced `AST_CAST{TYPE_GENERIC,NULL}` as a
callee placeholder inside `AST_CALL`, requiring special-casing in sema and cg.
It now produces a dedicated `AST_GENERIC_CALL` node:

```c
struct { char *name; AstVec type_args; AstVec call_args; } generic_call;
```

`Vec<int>.new()` still uses `AST_CAST{TYPE_GENERIC,NULL}` as a carrier for
the generic type expression; this is intentional — `parse_postfix` wraps it in
`AST_METHOD_CALL` for the method-dispatch path.

### Phase ordering fix

Generic function forward declarations (Phase 3.6) are now emitted **after**
generic struct specializations (Phase 3.5), ensuring that return types like
`Pair__int__int` in `Pair__int__int make_pair__int__int(...)` are already
defined when the forward declaration is emitted.

---

## 数据流总览

```
                    sharpc.c
                       │
          ┌────────────┼────────────┐
          │            │            │
      CppCtx        CppResult    include dirs
          │            │
       cpp_run()    CppToken[]
                       │
                    lex_run()
                       │
                    SharpTok[]
                       │
                    prescan_generic_defs()  ← Phase G pre-scan
                       │  (populates PS.generic_names)
                    parse_file()
                       │
                    AstNode*  ───────────────────┐
                       │                         │
                  scope_build()            scope.c 注册符号
                       │                         │
                     Scope*  ◀───────────────────┘
                       │
                  sema_check_file()
                       │
               (AstNode.type_ref 已填充)
                       │
                   cg_generate()
                       │
                    char* (C11)
                       │
                    写入输出
```

---

## 诊断系统

所有错误/警告统一用 `FeDiagArr`（动态数组，元素类型与 CPP 的 `CppDiag` 兼容）。

各阶段独立收集：
- `ld`：词法错误
- `pd`：解析错误
- `sd`：scope 错误
- `sema_d`：语义错误

`sharpc.c` 在所有阶段完成后统一遍历打印，格式：
```
file.c:10:5: error: message
```

---

## `tokcmp` 验证工具（`sharp-test/tokcmp.c`）

用于验证 sharpc 输出的 token 一致性：

```
原始 .ce/.c 文件
    │
    ├─── gcc -E ──────────────────▶  Token 流 A
    │
    └─── sharpc → 临时.c → gcc -E ▶  Token 流 B
                                         │
                                    LCS diff
                                         │
                              identical ✅ / differ ❌
```

过滤规则：只比较来自**用户文件**（非 `/usr/include` 等系统路径）的 token。

---

## 文件依赖关係

```
sharpc.c
  ├── lex.h        (SharpTok, lex_run)
  ├── parse.h      (parse_file, AstNode)
  ├── ast.h        (AstNode定义, AstKind)
  ├── scope.h      (scope_build, Symbol)
  ├── type.h       (TyStore, Type, ty_*)
  ├── sema.h       (sema_ctx_new, sema_check_file)
  ├── cg.h         (cg_ctx_new, cg_generate, cg_set_sys_dirs)
  └── sharp-cpp/   (cpp_ctx_new, cpp_run_buf, CppResult)
```

`cg.c` 是最重的文件（~7600行），包含：
- 所有 C 语法结构的发射逻辑
- 类型 AST 到字符串的完整映射
- 表达式打印（`cg_expr`）

`parse.c` 是第二重（~4600行），包含：
- 完整的 ISO C11 + Sharp 扩展声明解析
- 类型说明符序列解析（`parse_decl_specifiers`）
- 表达式解析（优先级爬升）
