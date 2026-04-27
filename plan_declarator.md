# Plan: 完整 C 声明符解析器（Full C Declarator Parser）

## 一、当前架构深度分析

### 1.1 parse_type() 的现状

当前 `parse_type()`（src/parser.c line 276-412）采用**从左到右的扁平类型解析**：

```
[修饰符] → 基础类型 → [*] → [const] → [__restrict__]
```

**问题：** C 的声明符语法不是线性的，是**中缀的**：

```c
int (*fp)(int, int);       // 函数指针：括号 + 参数列表 在中间
int (*arr[3])(void);       // 函数指针数组：数组 + 函数 交错
int (*(*p)(int))[4];       // 嵌套：多层括号/函数/数组交错
```

当前 parser 在遇到 `int (*` 时会认为 `*` 是 pointer postfix，但实际 `(` 后面是一个完整的 declarator 子树。

### 1.2 需要解析的 C 声明符完整语法（C99 §6.7.5）

```
declaration:
    declaration-specifiers init-declarator-list_opt ;

init-declarator:
    declarator [= initializer]

declarator:                    ← 核心难点
    pointer_opt direct-declarator

pointer:
    * [type-qualifier-list_opt] { * [type-qualifier-list_opt] }*

direct-declarator:
    identifier                          ← 最简单情况: name
    ( declarator )                      ← 括号包裹: (*name)
    direct-declarator [ ... ]           ← 数组修饰: name[N]
    direct-declarator ( ... )           ← 函数修饰: name(params)
```

### 1.3 当前 parser 缺失的能力

| 能力 | 状态 | 影响 | 测试用例 |
|------|------|------|---------|
| 基础类型 → 派生类型 | ✅ | `int*`, `int**` | - |
| 括号声明符 `(*name)` | ❌ | `int (*fp)(int)` 解析失败 | test_func_pointer.sp |
| 函数类型 `name(params)` | ❌ | `int (*fp)(int)` 解析失败 | test_func_pointer_param.sp |
| 数组 `[N]` | ⚠️ 部分 | 只在 vardecl 中支持 | - |
| 嵌套 `(*(*p)(int))[4]` | ❌ | 完全不支持 | - |
| restrict 在参数中 | ❌ | `char* restrict dest` 解析失败 | test_restrict_param.sp |
| 函数指针 typedef | ❌ | `typedef void (*fp)(int);` 解析失败 | test_complex_typedef.sp |

### 1.4 根本原因

**当前设计假设：** 类型声明 = `type-name` 后跟可选的 `*` 后缀。

**C 的实际语法：** 声明符是一棵**树**，根是标识符，中间节点是指针/数组/函数。

---

## 二、方案设计

### 2.1 设计原则

1. **不改变现有 `parse_type()` 的签名** — 保持返回 `Type*`
2. **新增 `parse_declarator()` 函数** — 解析声明符树
3. **声明符解析后返回 `(Type*, name)`** — 类型 + 名称
4. **与现有代码渐进式集成** — 不破坏已有功能
5. **AST 不变** — cgen 不需要改动（声明符只影响类型解析）

### 2.2 数据结构

**新增 `Declarator` 结构体：**

```c
/* 声明符解析结果 — 描述 "从标识符到基础类型的派生链" */
typedef struct {
    Type* type;           /* 最终构建的类型树 */
    const char* name;     /* 声明的标识符名称 */
    bool is_variadic;     /* 函数类型：是否变参 */
    Node** params;        /* 函数类型：参数列表 */
    int nparams;
} Declarator;
```

**Type 系统扩展：**

```c
/* TypeKind 新增 */
typedef enum {
    // ... existing ...
    TY_FUNC,     /* 函数类型: ret_type(params) */
} TypeKind;

struct Type {
    // ... existing fields ...
    /* For TY_FUNC: */
    Type* ret_type;        /* 函数返回类型 */
    Type** param_types;    /* 函数参数类型数组 */
    int nparams;
    bool is_variadic;
    bool param_has_names;  /* 参数是否有名称（用于 extern 声明）*/
};
```

### 2.3 算法：经典 C 声明符解析

```
parse_declarator(base_type):
    1. 解析指针前缀: * [const] [restrict] { * ... }*
       → 构建指针链: pointer_type

    2. 解析直接声明符（递归）:
       a. 如果是 name → 基础情况: 返回 (type=pointer_type, name=name)
       b. 如果是 ( → 递归: parse_declarator(pointer_type)
       c. 如果是 direct-declarator [N] → 数组: 构建数组类型
       d. 如果是 direct-declarator (params) → 函数: 构建函数类型

    3. 组合: 从内到外构建类型树
```

**关键洞察：** C 声明符的解析是**先指针前缀，再直接声明符**。直接声明符是递归的：

```c
int (*fp)(int, int);
//       ↑    ↑
//     指针前缀  函数修饰
//       ↖_____↙
//         组合
```

### 2.4 核心函数设计

```c
/* 解析指针前缀: * [const] [restrict] { * ... }* */
static Type* parse_pointer_prefix(P* p, Type* base);

/* 解析直接声明符: name | (declarator) | direct[N] | direct(params) */
static Declarator parse_direct_declarator(P* p, Type* pointer_type);

/* 主入口: 解析完整声明符 */
static Declarator parse_declarator(P* p, Type* base_type);

/* 从 Declarator 构建最终的 Type*（用于不需要名称的场景） */
static Type* declarator_to_type(P* p, Declarator d);
```

### 2.5 parse_direct_declarator 详细流程

```
parse_direct_declarator(pointer_type):
    token = peek()

    case TK_LPAREN:
        consume '('
        inner = parse_declarator(pointer_type)  // 递归解析括号内的声明符
        consume ')'
        // 递归解析后续的数组/函数修饰
        return parse_array_func_suffix(inner)

    case TK_IDENT:
        name = consume()
        d = Declarator { type: pointer_type, name: name }
        // 解析后续的数组/函数修饰
        return parse_array_func_suffix(d)

    case TK_STRUCT / TK_UNION:
        // 匿名结构体/联合体作为类型
        struct_type = parse_struct_decl() / parse_union_decl()
        d = Declarator { type: struct_type, name: null }
        return parse_array_func_suffix(d)

    default:
        error("expected identifier or '('")

parse_array_func_suffix(declarator):
    loop:
        if peek() == '[':
            consume '['
            if peek() != ']':
                size = parse_expr()
            consume ']'
            declarator.type = type_array(declarator.type, size)

        else if peek() == '(':
            consume '('
            params = parse_param_list()
            consume ')'
            declarator.type = type_func(declarator.type, params)
            declarator.is_variadic = params.is_variadic
            declarator.params = params.nodes
            declarator.nparams = params.count

        else:
            break  // 没有更多修饰符

    return declarator
```

### 2.6 集成点

| 当前函数 | 当前调用 | 改为调用 | 说明 |
|---------|---------|---------|------|
| `parse_struct_decl()` | `Type* ty = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | 结构体字段声明 |
| `parse_field_decl()` | `Type* ty = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | 字段声明（含位域） |
| `parse_param_list()` | `Type* ty = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | 函数参数 |
| `parse_typedef()` | `Type* base = parse_type(p)` | `Declarator d = parse_declarator(p, base)` | typedef 声明 |
| `parse_vardecl()` | `Type* ty = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | 局部变量 |
| `parse_extern_decl()` | `Type* ret = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | extern 声明 |
| `parse_local_decl()` | `Type* ty = parse_type(p)` | `Declarator d = parse_declarator(p, base_type)` | 局部声明 |

---

## 三、具体实现代码

### 3.1 文件：`src/sharp.h`

新增：
```c
/* TypeKind 扩展 */
typedef enum {
    // ... existing ...
    TY_FUNC,       /* 函数类型: ret_type(params) */
} TypeKind;

struct Type {
    // ... existing fields ...
    /* For TY_FUNC: */
    Type* ret_type;
    Type** param_types;
    int nparams;
    bool is_variadic;
};
```

### 3.2 文件：`src/types.c`

新增：
```c
Type* type_func(Arena** a, Type* ret, Type** params, int nparams, bool variadic) {
    Type* t = (Type*)arena_zalloc(a, sizeof(Type));
    t->kind = TY_FUNC;
    t->ret_type = ret;
    t->param_types = params;
    t->nparams = nparams;
    t->is_variadic = variadic;
    return t;
}
```

### 3.3 文件：`src/parser.c`

新增约 **400 行代码**：

1. **`parse_pointer_prefix()`** — 约 50 行
2. **`parse_direct_declarator()`** — 约 150 行
3. **`parse_array_func_suffix()`** — 约 80 行
4. **`parse_declarator()`** — 约 50 行
5. **`declarator_to_type()`** — 约 50 行
6. **集成到 7 个调用点** — 约 20 行改动

---

## 四、测试文件

现有 13 个测试文件中，以下将由此方案解决：

| 测试文件 | 特性 | 是否解决 |
|---------|------|---------|
| `test_func_pointer_param.sp` | 函数指针参数 | ✅ |
| `test_func_pointer.sp` | 函数指针变量 | ✅ |
| `test_complex_typedef.sp` | 函数指针 typedef | ✅ |
| `test_restrict_param.sp` | restrict 参数 | ✅ |

剩余 9 个测试文件需要其他方案（匿名结构体、位域、指定初始化器等）。

---

## 五、验证标准

| 测试用例 | 输入 | 预期输出 |
|---------|------|---------|
| 简单函数指针 | `int (*fp)(int);` | 编译通过，生成 `int (*fp)(int);` |
| 函数指针参数 | `void qsort(void*, size_t, size_t, int(*)(const void*, const void*));` | 编译通过 |
| 函数指针 typedef | `typedef void (*sighandler_t)(int);` | 编译通过 |
| restrict 参数 | `void* memcpy(void* restrict, const void* restrict, size_t);` | 编译通过 |
| 现有 54 个测试 | 不变 | 全部通过 |

---

## 六、工作量估算

| 步骤 | 工作量 | 说明 |
|------|--------|------|
| 设计 + 数据结构 | 半天 | Declarator, TY_FUNC |
| `parse_declarator()` 核心实现 | 1 天 | 递归下降解析声明符树 |
| 集成到所有调用点 | 1 天 | 7 个调用点逐一适配 |
| types.c 扩展 | 半天 | type_func |
| 测试 + 调试 | 1 天 | 测试文件 + 回归测试 |
| **总计** | **4 天** | |
