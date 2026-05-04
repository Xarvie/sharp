# Sharp 编译器架构重构计划

> 基于 sharp-lang-spec-v0.10 对齐

## 1. 目标

实现标准编译器前端流程，使 `.c` 和 `.sp` 文件都能通过同一套管线正确编译，
并补全 spec v0.10 定义的核心语言特性（泛型函数、`@` 内省、const 方法）：

```
每个输入文件 → preprocess → lex → parse → AST → sema → codegen → tmp/*.c
tmp/*.c → cl 编译 + 链接 → 最终可执行文件/库
```

## 2. 当前问题

### 2.1 架构问题（已修复）
- ~~**没有 per-file 隔离**~~ → Per-File CompilationUnit 已实现
- ~~**单文件输出**~~ → 每个文件独立输出 tmp/<basename>.c
- ~~**没有 link 阶段**~~ → 待实现

### 2.2 预处理器（已修复）
- **Linemarker 转换 bug**：`# 123` → `#line 123` 转换中 `memmove` 方向错误导致输出截断
  - 根因：`memmove(after_hash + 5, after_hash + 1, tail_len + 1)` 向前覆盖而非向后腾出空间
  - 修复：使用 `realloc` 扩展缓冲区 + `memmove(after_hash + 1 + shift, after_hash + 1, tail_len)` 向后移位

### 2.3 Parser 缺失的 C 语法（阻塞中）
- **逗号运算符**：`(expr1, expr2)` 在表达式中
- **空语句**：单独的 `;`
- **函数指针调用**：`(*_errno())` 这种模式解析失败
- **复杂 MSVC 类型**：`__declspec(dllimport)` 等声明
- **Statement expression**：`{ ... }` 作为表达式值（MSVC 内联函数中常见）
- **复杂宏展开**：`__analysis_assume` 等宏展开后产生 `!!(expr)` + if 语句组合
- **错误恢复不完整**：一个解析错误导致级联错误

当前状态：33 个 Lua 文件中，22 个通过完整管线，11 个因 parser 不支持上述语法而失败。

### 2.4 构建流程（部分完成）
- ~~缺少 tmp 目录中间文件管理~~ → 已实现
- ~~无法自动发现并编译多个 .c 文件~~ → 已实现 Per-File 循环

### 2.5 Spec v0.10 定义但未实现的特性
- **独立泛型函数**：`void swap<T>(T* a, T* b)` — Parser/Sema/Cgen 均未实现
- **泛型函数调用与类型推断**：`swap<int>(&x, &y)` / `swap(&x, &y)` — 缺失
- **`@` 内省原语**：`@has_destructor(T)`, `@has_operator(T, +)`, `@has_method(T, name)`, `@is_same(T, U)`, `@typeof(expr)`, `@static_assert(cond, msg)` — 缺失
- **const 方法后缀**：`long len() const { ... }` — 缺失
- **显式实例化声明**：`extern struct Vec<int>` — 缺失
- **泛型函数单态化**：Cgen 中无泛型函数特化代码生成

### 2.6 泛型跨文件问题（Per-File 模式下新增）
- 模板定义如何跨文件可见（file1.sp 定义 `struct Box<T>`，file2.sp 实例化 `Box<int>`）
- Monomorphization 收集的边界：每个文件独立收集还是全局汇总
- 重复特化：多文件都使用 `Box<int>` 时如何避免重复生成
- 泛型定义必须在头文件 (`.sph`) 中对实例化点可见

## 3. 开发规范

> 详见 `.trae/rules/开发规范.md`

- **预处理器必须能处理 `.sp` 和 `.c`，所有 `.c` 当作 `.sp` 来处理**
- **严禁为 `.sp` 和 `.c` 和 `.h` 做两条流水线处理，严格遵守**
- **修改特性支持后，必须为 Lua 项目进行编译测试**
- **确保 sp 预处理器永远兼容 `.c` 和 `.h` 文件**

## 4. 重构方案

### 阶段 1：修复 Parser 的 C 语法支持（阻塞性 — 未完成）

> 本阶段是后续所有阶段的前提，必须首先完成。

#### 3.1.1 支持逗号运算符
在 `parse_expr` 中已部分支持，验证括号内也能正确解析。

#### 3.1.2 支持空语句
在 `parse_stmt` 中添加空语句处理。

#### 3.1.3 修复函数指针调用解析
`(*func)()` 的解析流程修复，确保 `parse_unary` 在 cast 检查失败后正确回退到 `parse_postfix`。

#### 3.1.4 支持 __declspec 修饰
在函数声明和 extern 中识别 `__declspec(...)` 并传递给 cgen。

#### 3.1.5 支持 Statement Expression（MSVC 内联函数）
MSVC 头文件中常见 `{ int _x = ...; if (_x) ... }` 作为表达式值。
需要扩展 `parse_primary` 识别 `{` 开头的复合语句作为表达式。

#### 3.1.6 支持复杂宏展开后的语法
`__analysis_assume` 等宏展开后产生 `!!(expr)` + if 语句组合，
需要 parser 能处理这种非标准但合法的 C 语法。

#### 3.1.7 完善错误恢复
解析错误时跳过到下一个同步点（`;`、`}`、`#`），避免级联错误。

### 阶段 2：实现 Per-File 编译单元（已部分完成）

> Per-File 循环已在 main.c 中实现，但因阶段 1 未完成而无法通过 Lua 测试。

#### 设计前提
- **`.sp`、`.c`、`.h` 都是 Sharp 源文件**，语法完全相同
- Sharp 语法是 C11 的超集（+ struct 体内方法 + 泛型 + RAII + `@`内省）
- 方法全部定义在 struct 体内，无独立的 `impl` 块
- 不需要按文件类型分流，所有文件走同一套管线
- 预处理器已经是完整的 C 预处理器，处理 `#include`/`#define` 后产出统一文本

#### 3.2.1 统一编译管线
```
每个输入文件 (无论 .sp/.c/.h):
  cpp_run()        → 预处理文本 (展开所有 #include/#define, 带 linemarker)
  linemarker fix   → # 123 → #line 123 (修复后完整输出)
  lex_init()       → 吸收 linemarker, 产出 Sharp token 流
  parse_program()  → AST
  sema_build()     → SymTable (含 monos 收集)
  cgen_c()         → tmp/<basename>.c
```

#### 3.2.2 CompilationUnit 结构（已实现）
```c
typedef struct {
    const char*       input_path;     // 输入文件路径 (a.sp / util.c / foo.h)
    const char*       output_path;    // 输出 tmp/<basename>.c
    char*             pp_text;        // 预处理文本
    size_t            pp_text_len;    // 预处理文本长度
    Node*             ast;            // AST
    SymTable*         symtab;         // 文件级符号表
    TemplateRegistry* templates;      // 共享的泛型模板注册表
} CompilationUnit;
```

#### 3.2.3 TemplateRegistry（已实现）
```c
typedef struct {
    const char** names;    // 泛型模板名称（struct 或 func）
    void**       decls;    // 对应的 AST 节点
    int*         kinds;    // 模板类型（struct/func）
    int          count;
    int          cap;
} TemplateRegistry;
```
所有文件的编译单元共享同一个 TemplateRegistry，解决泛型模板定义的跨文件可见性。

#### 3.2.4 编译循环（已实现）
- 共享 CppCtx 预处理器上下文
- 所有文件通过同一管线：preprocess → lex → parse → sema → cgen
- tmp 目录管理已实现

#### 3.2.5 Monomorphization 两阶段收集
1. **Per-File 阶段**：每个文件独立运行 sema，收集自己触发的泛型实例化到本地 monos
2. **Link 汇总阶段**：汇总所有文件的 monos，按 mangled name 去重，决定哪些特化需要额外生成代码

### 阶段 3：实现独立泛型函数（Spec v0.10 核心特性）

#### 3.3.1 AST 扩展：ND_FUNC_DECL 增加 type_params 字段
在 `sharp.h` 的 `Node` 结构体中（已有 `type_params` / `ntype_params` 字段用于 struct），
确认其对 `ND_FUNC_DECL` 也可用，或在注释中明确扩展。

#### 3.3.2 Parser：解析 `func<T>(params)` 语法
修改顶层函数解析路径（`parse_program` 中 type_start_path），在函数名之后、`(` 之前检测 `<`：
```c
// 当前: ret name(params)
// 扩展: ret name<T, U>(params)
```
复用 `parse_type_params`（已有，用于 struct），将结果写入 `ND_FUNC_DECL->type_params`。

#### 3.3.3 Parser：泛型函数调用 `ident<T>(args)`
在 `parse_postfix` 的调用路径中，遇到 `ident<` 时解析为泛型调用：
```c
// 当前 parse_postfix: primary { (args) | .member | [idx] }*
// 扩展: primary 若是 ident 且后面跟 < → 解析为泛型调用
```
携带 Type* 注解到 AST 节点上，供 sema/cgen 使用。

#### 3.3.4 Parser：泛型函数类型推断调用 `ident(args)`
保留现有调用解析，在 sema 阶段通过参数类型推断 T。

#### 3.3.5 Sema：泛型函数实例化收集
扩展 `collect_from_node` 和 `tc_mk_subst` 处理泛型函数调用：
- 遇到 `swap<int>(&x, &y)` → 记录 `SymMono{ generic_name="swap", targs=[int] }`
- 遇到 `swap(&x, &y)` → 推断 T=int → 记录同上

#### 3.3.6 Sema：泛型函数类型推断
```c
Type* infer_generic_type(TC* tc, Node* func, Node** args, int nargs);
```
比对函数参数声明类型（含类型参数 T）与实际参数类型，反解 T 的具体类型。

#### 3.3.7 Cgen：泛型函数单态化代码生成
扩展 `cgen_c` 的特化循环，在 struct 特化之外增加函数特化：
```c
for each SymMono m in st->monos:
    if m->generic_name 对应一个泛型函数:
        emit_specialized_function(g, func_decl, m);
```
复用 `enter_spec_ctx` / `exit_spec_ctx` 机制（已有，用于 struct 特化）。

### 阶段 4：实现 `@` 内省原语（Spec v0.10 核心特性）

#### 3.4.1 Lexer：识别 `@` 前缀内省令牌
新增 `TK_AT_HAS_DESTRUCTOR`, `TK_AT_HAS_OPERATOR`, `TK_AT_HAS_METHOD`,
`TK_AT_IS_SAME`, `TK_AT_TYPEOF`, `TK_AT_STATIC_ASSERT` 等令牌类型，
或统一使用 `TK_AT_IDENT` + 运行时判断。

#### 3.4.2 Parser：内省原语语法
```
@has_destructor(T)        → 返回 bool 编译期常量
@has_operator(T, +)       → 返回 bool 编译期常量
@has_method(T, name)      → 返回 bool 编译期常量
@is_same(T, U)            → 返回 bool 编译期常量
@typeof(expr)             → 返回 type
@static_assert(cond, msg) → 语句，cond 失败则中止编译
```

#### 3.4.3 Preprocessor：`#if @has_destructor(T)` 支持
预处理器的 `#if` 常量表达式中允许 `@` 原语作为子表达式。
在泛型特化上下文中，`@has_destructor(T)` 需要解析为当前替换后的具体类型。

#### 3.4.4 Sema：内省原语求值
在 type-checking 阶段求值 `@` 原语，返回编译期常量或类型。
`@static_assert` 失败时通过 `diag_emit(DIAG_ERROR, ...)` 报告并中止。

#### 3.4.5 Cgen：`@` 原语代码生成
- `@has_destructor(T)` → 生成 `0` 或 `1`（编译期常量）
- `@is_same(T, U)` → 生成 `0` 或 `1`
- `@typeof(expr)` → 类型名称字符串（用于宏展开）
- `@static_assert` → 生成 `_Static_assert(cond, msg)`
- `#if @has_destructor(T)` → 预处理阶段求值为 `#if 0` 或 `#if 1`

### 阶段 5：const 方法后缀支持（Spec v0.10）

#### 3.5.1 Lexer：识别方法体后的 `const` 关键字
在 `parse_func_common` 的 `)` 之后、`{` 之前检测 `const`。

#### 3.5.2 AST：`ND_FUNC_DECL` 增加 `is_const_method` 标志

#### 3.5.3 Sema：const 方法类型检查
- const 方法的 `this` 类型为 `const T*`
- const 方法体内禁止修改 `this->...`
- const 接收者不能调用非 const 方法

#### 3.5.4 Cgen：const 方法签名生成
```c
// const 方法: long Buffer__length(const Buffer* this)
// 非 const:   char Buffer__get(Buffer* this, long i)
```

### 阶段 6：显式实例化与分离编译（Spec v0.10）

#### 3.6.1 Parser：`extern struct Vec<int>` 语法
识别 `extern struct Name<T>` → 声明，本 TU 不生成代码。
识别 `struct Name<T>;`（无 body）→ 显式实例化，本 TU 生成代码。

#### 3.6.2 Sema：显式实例化标记
为每个 `SymMono` 增加 `extern_flag`，区分"本 TU 生成" vs "外部提供"。

#### 3.6.3 Cgen：根据 extern_flag 决定是否生成特化代码

### 阶段 7：实现 Link 阶段

#### 3.7.1 收集所有 tmp/*.c 文件
#### 3.7.2 调用 cl.exe 编译每个 .c 为 .obj
#### 3.7.3 调用 link.exe 链接所有 .obj（含泛型特化去重）

### 阶段 8：集成测试

#### 3.8.1 测试 hello.sp
#### 3.8.2 测试泛型 struct + struct 体内方法
#### 3.8.3 测试独立泛型函数 `swap<T>` + 类型推断
#### 3.8.4 测试 `@has_destructor` 条件编译
#### 3.8.5 测试 const 方法
#### 3.8.6 测试 Lua 多文件完整管线编译（33/33 通过）
#### 3.8.7 测试 .c 文件直接编译
#### 3.8.8 测试显式实例化 `extern struct Vec<int>`

## 5. 实施顺序

| 阶段 | 内容 | 预计变更 | 状态 | 阻塞关系 |
|------|------|----------|------|----------|
| 1 | 修复 Parser C 语法 | parser.c | 未完成 | 无（阻塞后续） |
| 2 | Per-File 编译单元 + TemplateRegistry | main.c, sharp.h, sema.c | 部分完成 | 依赖 1 |
| 3 | 独立泛型函数 | parser.c, sharp.h, sema.c, cgen.c | 未开始 | 依赖 2 |
| 4 | `@` 内省原语 | lexer.c, parser.c, sema.c, preproc/, cgen.c | 未开始 | 依赖 3 |
| 5 | const 方法后缀 | lexer.c, parser.c, sema.c, cgen.c | 未开始 | 可与 3/4 并行 |
| 6 | 显式实例化 | parser.c, sema.c, cgen.c | 未开始 | 依赖 3 |
| 7 | Link 阶段 | main.c | 未开始 | 依赖 2 |
| 8 | 集成测试 | tests/ | 未开始 | 依赖 1-7 |

## 6. 风险和缓解

| 风险 | 缓解措施 |
|------|----------|
| Parser 修复引入新 bug | 每修复一个语法，用 hello.sp 验证不回归 |
| MSVC 头文件太复杂 | 先用 -E 模式验证预处理输出，再逐步添加支持 |
| Per-File 重构范围大 | 先实现单文件版本，再扩展多文件 |
| 泛型函数类型推断复杂 | 先从显式 `swap<int>` 开始，再实现推断 `swap()` |
| `@` 原语与预处理器交互 | 明确两阶段求值：预处理器阶段处理 `#if @...`，语义阶段处理表达式内 `@...` |
| 泛型特化跨文件重复 | 统一 mangled name + 链接器 COMDAT 去重 |
| const 方法破坏现有 API | 先实现 const 标志解析，再逐步添加检查逻辑 |

## 7. 成功标准

### 基础标准
1. `sharpc hello.sp -o hello.exe` 输出 Hello World
2. `sharpc lapi.c -o lapi.obj` 完整管线编译成功（非仅 -E 模式）
3. `sharpc lua1.sp lua2.sp -o lua.exe` 多文件编译成功
4. 所有现有 tests/ 测试通过
5. Lua 项目 33/33 文件通过完整管线编译 + cl.exe 链接

### Spec v0.10 标准
6. `struct Box<T> { T value; ...方法... }` 泛型 struct + 体内方法编译通过
7. `void swap<T>(T* a, T* b)` 独立泛型函数编译通过
8. `swap<int>(&x, &y)` 显式类型参数调用编译通过
9. `swap(&x, &y)` 类型推断调用编译通过
10. `#if @has_destructor(T)` 在泛型体内条件编译正确
11. `@static_assert(@has_operator(T, +), "msg")` 约束检查正确
12. `long len() const` const 方法编译通过，const 接收者不能调用非 const 方法
13. `extern struct Vec<int>` 显式实例化控制特化生成位置
14. `Vec<const char*>` vs `Vec<char*>` 产生不同实例化
15. 名字改编遵循 spec 规则：`Vec<int>` → `Vec__int`，`swap<int>` → `swap__int`
