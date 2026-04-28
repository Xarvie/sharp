# C 类型声明器重构计划

## 现状分析

### 当前架构的三个函数

```
emit_type(t)          — 无名字，输出类型字符串
emit_type_core(t)     — 核心类型输出（递归时调用）
emit_type_with_name(t, name) — 有名字，输出 `类型 名字`
```

**根本缺陷**：采用"左到右"的字符串拼接模型，而 C 语言的声明器语法是**中心环绕模型**（dangling modifiers）。

### C 声明语法的本质

C 语言中，`*` 的位置不是"追在类型后面"，而是"环绕在名字周围"：

```
int* p              → int *p          （* 在名字左边）
int (*fp)(int)      → int (*fp)(int)  （* 嵌入括号，名字在中间）
void (*(*pp)(int))(char) → 返回值 + (*(*pp)(int)) + (char)  （多层嵌套）
int a[10][20]       → int a[10][20]   （[] 在名字右边）
int (*ap[10])(void) → int (*ap[10])(void) （数组+函数指针混合）
```

C 声明的语法结构是：**specifier + declarator**：

```
specifier:  int, void, struct Foo, const volatile, ...
declarator: * declarator
          | declarator [N]
          | declarator ( params )
          | ( declarator )
          | identifier
```

`*` 必须向**内**层绑定到名字，`[]` 和 `()` 向**外**层绑定到名字。

### 当前架构的已知缺陷

| 场景 | Type 结构 | 当前输出 | 正确输出 |
|------|-----------|----------|----------|
| 函数指针参数 | `TY_PTR(base=TY_FUNC)` | `int (*name)(void)` | ✅ 正确 |
| typedef 函数指针 + 额外指针 | `TY_PTR(base=TY_NAMED("_PVFV"))` | `void(*)(void)* name` | `_PVFV* name` |
| 多层函数指针 | `TY_PTR(base=TY_PTR(base=TY_FUNC))` | `void(*)(void)* * name` | `void (*(*name)(void))` |
| 数组指针 | `TY_PTR(base=TY_ARRAY)` | `int* name[10]` | `int (*name)[10]` |
| const 函数指针 | `TY_PTR(base=TY_FUNC, is_const)` | `int (*name const)(void)` | `int (* const name)(void)` |

### 当前 hack 的问题

现在用了一个黑名单（排除 `va_list`, `wchar_t` 等）来临时修复 `_PVFV*` 问题，但这：
1. 是经验性的，无法覆盖所有情况
2. 对 struct/union typedef 也会触发（如 `FILE*` 变成 `FILE*` 碰巧对了，但路径不对）
3. 不能处理 `int (*(*pp)(int))(void)` 这种多层嵌套

## 重构方案

### 核心思想

实现**真正的 C 声明器解析器**：将 `Type` 树拆分为 **specifier** 和 **declarator**，然后按 C 语法规则渲染。

### 数据结构

新增 `Declarator` 结构，表示一个声明器的分解：

```c
typedef enum DeclKind {
    DECL_IDENT,        /* 叶子：标识符 */
    DECL_PTR,          /* * base */
    DECL_ARRAY,        /* base [size] */
    DECL_FUNC,         /* base (params) */
    DECL_GROUP,        /* (base) — 分组 */
} DeclKind;

typedef struct Declarator {
    DeclKind     kind;
    struct Declarator* base;     /* 内层声明器 */
    const char*  name;           /* 标识符名（DECL_IDENT） */
    int          array_size;     /* 数组维度（DECL_ARRAY） */
    Type**       func_params;    /* 函数参数（DECL_FUNC） */
    int          nfunc_params;
    bool         func_variadic;
    bool         is_const;       /* 该层的 const 修饰 */
} Declarator;
```

### 算法

```
1. 将 Type 树转换为 Declarator 树
   - TY_PTR → DECL_PTR
   - TY_ARRAY → DECL_ARRAY
   - TY_FUNC → DECL_FUNC
   - TY_NAMED（typedef 名） → 保留为 specifier，不展开
   - TY_NAMED（struct/union/enum 名） → 保留为 specifier
   - 原始类型 (int, void, ...) → specifier

2. 渲染 Declarator 树
   - 先输出 specifier
   - 从外层到内层遍历 declarator，决定每个 * 的位置
   - 遇到 DECL_PTR：输出 `*`（const 时 `* const`）
   - 遇到 DECL_ARRAY：输出 `[size]`
   - 遇到 DECL_FUNC：输出 `(params)`
   - 遇到需要分组的场景：用 `( ... )` 包裹

3. 核心规则：指针必须分组
   - 如果外层是 PTR，内层是 FUNC/ARRAY → 需要括号
   - 如果外层是 PTR，内层是 PTR → 不需要括号（***p）
```

### 文件变更

#### `src/cgen.c` — 完全重写类型发射模块

**删除**：
- `emit_type(G* g, Type* t)` — 旧实现
- `emit_type_core(G* g, Type* t)` — 旧实现
- `emit_type_with_name(G* g, Type* t, const char* name)` — 旧实现
- `emit_type_with_name_unaligned(G* g, Type* t, const char* name)` — 旧实现

**新增**：

```c
/* Declarator tree — represents a C declarator for rendering. */
typedef struct Declarator Declarator;

struct Declarator {
    DeclKind    kind;
    Declarator* base;
    const char* name;       /* only for DECL_IDENT */
    int         array_size; /* only for DECL_ARRAY */
    Type**      func_params;/* only for DECL_FUNC */
    int         nfunc_params;
    bool        func_variadic;
    bool        is_const;   /* const at this level */
};

/* Build a declarator tree from a Type.
 * - TY_PTR → DECL_PTR
 * - TY_ARRAY → DECL_ARRAY
 * - TY_FUNC → DECL_FUNC
 * - TY_NAMED / primitives → specifier only (no declarator)
 *
 * Crucially: TY_NAMED (typedef names) are NOT expanded.
 * The typedef name itself becomes the specifier. */
static Declarator* build_declarator(G* g, Type* t);

/* Emit just the specifier part (int, void, struct Foo, typedef_name). */
static void emit_specifier(G* g, Type* t);

/* Render the full declaration: `specifier declarator` */
static void emit_decl(G* g, Type* t, const char* name);

/* Render a declarator subtree at the current grouping level.
 * If `need_parens` is true, wrap in parentheses. */
static void emit_declarator(G* g, Declarator* d, const char* name, bool need_parens);

/* Free a declarator tree (allocated from g->arena). */
static void declarator_free(Declarator* d);
```

### 实现细节

#### build_declarator

```c
static Declarator* build_declarator(G* g, Type* t) {
    if (!t) return mk_decl(DECL_IDENT, NULL);

    Type* rt = resolve_type(g, t);
    if (!rt) return mk_decl(DECL_IDENT, NULL);

    switch (rt->kind) {
    case TY_PTR: {
        /* Don't expand typedef names — keep the name as specifier.
         * Only build PTR declarator if base is NOT a TY_NAMED
         * (which would be a typedef name we should preserve). */
        Type* base = rt->base;
        if (base && base->kind == TY_NAMED && base->ntargs == 0) {
            /* This is `ptr-to-typedef-name`. The typedef name is the
             * specifier, and we have one level of pointer on top. */
            Declarator* inner = mk_decl(DECL_IDENT, NULL);
            inner->name = base->name;  /* embed typedef name in declarator */
            Declarator* outer = mk_decl(DECL_PTR, inner);
            outer->is_const = rt->is_const;
            return outer;
        }
        Declarator* inner = build_declarator(g, base);
        Declarator* outer = mk_decl(DECL_PTR, inner);
        outer->is_const = rt->is_const;
        return outer;
    }
    case TY_ARRAY: {
        Declarator* inner = build_declarator(g, rt->base);
        Declarator* outer = mk_decl(DECL_ARRAY, inner);
        outer->array_size = rt->array_size;
        return outer;
    }
    case TY_FUNC: {
        Declarator* inner = build_declarator(g, rt->base);
        Declarator* outer = mk_decl(DECL_FUNC, inner);
        outer->func_params = rt->func_params;
        outer->nfunc_params = rt->nfunc_params;
        outer->func_variadic = rt->func_variadic;
        return outer;
    }
    case TY_NAMED:
    case TY_VOID: case TY_BOOL: case TY_CHAR: case TY_SHORT:
    case TY_INT: case TY_LONG: case TY_LONGLONG:
    case TY_FLOAT: case TY_DOUBLE:
    case TY_BITFIELD:
        /* These are specifiers, not declarators. */
        return NULL;
    }
    return NULL;
}
```

#### emit_decl

```c
static void emit_decl(G* g, Type* t, const char* name) {
    Type* rt = resolve_type(g, t);
    Declarator* d = build_declarator(g, rt);

    if (!d) {
        /* Pure specifier, no declarator */
        emit_specifier(g, t);
        if (name) sb_printf(&g->out, " %s", name);
        return;
    }

    /* Check if the declarator's innermost part is a TY_NAMED that
     * we embedded as the identifier (typedef name used as specifier). */
    if (d->kind == DECL_PTR && d->base && d->base->kind == DECL_IDENT && d->base->name) {
        /* This is `ptr-to-typedef`: emit "name*" directly */
        sb_puts(&g->out, d->base->name);
        sb_putc(&g->out, '*');
        if (d->is_const) sb_puts(&g->out, " const");
        return;
    }

    /* Find the specifier type (innermost non-declarator type) */
    Type* spec_type = t;
    while (spec_type) {
        Type* inner = NULL;
        if (spec_type->kind == TY_PTR || spec_type->kind == TY_ARRAY)
            inner = spec_type->base;
        else if (spec_type->kind == TY_FUNC)
            inner = spec_type->base;
        if (!inner) break;
        spec_type = inner;
    }

    emit_specifier(g, spec_type);
    emit_declarator(g, d, name, false);
}
```

#### emit_declarator

```c
static void emit_declarator(G* g, Declarator* d, const char* name, bool need_parens) {
    if (!d) {
        if (name) sb_puts(&g->out, name);
        return;
    }

    /* Determine if this level needs parentheses */
    bool needs_parens = need_parens;
    if (d->kind == DECL_PTR && d->base) {
        /* Pointer to array or function needs parens: (*name)[], (*name)() */
        if (d->base->kind == DECL_ARRAY || d->base->kind == DECL_FUNC)
            needs_parens = true;
    }

    if (needs_parens) sb_putc(&g->out, '(');

    switch (d->kind) {
    case DECL_IDENT:
        if (name) sb_puts(&g->out, name);
        else if (d->name) sb_puts(&g->out, d->name);
        break;

    case DECL_PTR:
        emit_declarator(g, d->base, name, false);
        sb_putc(&g->out, '*');
        if (d->is_const) sb_puts(&g->out, " const");
        break;

    case DECL_ARRAY:
        emit_declarator(g, d->base, name, false);
        sb_printf(&g->out, "[%d]", d->array_size);
        break;

    case DECL_FUNC:
        emit_declarator(g, d->base, name, false);
        sb_putc(&g->out, '(');
        for (int i = 0; i < d->nfunc_params; i++) {
            if (i) sb_puts(&g->out, ", ");
            emit_decl(g, d->func_params[i], NULL);
        }
        if (d->func_variadic) {
            if (d->nfunc_params > 0) sb_puts(&g->out, ", ");
            sb_puts(&g->out, "...");
        }
        sb_putc(&g->out, ')');
        break;

    case DECL_GROUP:
        emit_declarator(g, d->base, name, true);
        break;
    }

    if (needs_parens) sb_putc(&g->out, ')');
}
```

### 测试场景

| 测试用例 | Type 结构 | 期望输出 |
|----------|-----------|----------|
| `int x` | `TY_INT` | `int x` |
| `int* p` | `TY_PTR(TY_INT)` | `int* p` |
| `int (*fp)(int)` | `TY_PTR(TY_FUNC(TY_INT, [TY_INT]))` | `int (*fp)(int)` |
| `_PVFV* p` | `TY_PTR(TY_NAMED("_PVFV"))` | `_PVFV* p` |
| `void (*(*pp)(int))(void)` | `TY_PTR(TY_FUNC(TY_PTR(TY_FUNC(TY_VOID, [])), [TY_INT]))` | `void (*(*pp)(int))(void)` |
| `int a[10]` | `TY_ARRAY(TY_INT, 10)` | `int a[10]` |
| `int (*ap[10])(void)` | `TY_ARRAY(TY_PTR(TY_FUNC(TY_INT, [])), 10)` | `int (*ap[10])(void)` |
| `const int* p` | `TY_PTR(TY_INT, is_const)` | `const int* p` |
| `int* const p` | `TY_PTR(TY_INT, is_const)` | `int* const p` |

### 影响范围

| 调用点 | 替换为 |
|--------|--------|
| `emit_type(g, t)` | `emit_decl(g, t, NULL)` |
| `emit_type_with_name(g, t, name)` | `emit_decl(g, t, name)` |
| `emit_type_core(g, t)` | `emit_specifier(g, t)`（部分场景） |

### 风险缓解

1. **先写单元测试**：创建一个 `test_cdecl.c` 测试文件，包含所有已知场景，验证输出正确性
2. **回归测试**：运行 `run_tests.py` 确保 158/161 不 regression
3. **clang 验证**：运行 `verify_ucrt.py` 确认 13/15 或更好的 clang 通过率
4. **逐步替换**：先实现新模块，再逐个调用点替换

## 后续工作

- 清理旧的 `emit_type` / `emit_type_core` / `emit_type_with_name` / `emit_type_with_name_unaligned`
- 处理 `__unaligned` 修饰符（目前在 `emit_type_with_name_unaligned` 中有特殊逻辑）
- 处理 bitfield 的 declarator
