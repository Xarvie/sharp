# 全面修复计划：C 解析逻辑严谨性修复

## 问题清单

### P0（高）：数组维度在 parse_declarator 中被丢弃
### P1（中）：wchar_t 硬编码特殊处理违反关注点分离
### P2（低）：builtin_types 与 is_crt_sdk_typedef 列表重叠
### P3（低）：is_system_header_path 路径覆盖不完整

---

## 修复 1（P0）：parse_declarator 中数组维度保留

### 问题分析

**当前行为**：`parse_declarator_internal`（parser.c:757-770）中，数组后缀 `[N]` 被消费但维度表达式立即丢弃（`(void)dim;`），类型统一降级为指针。

```c
/* 当前代码 */
} else {
    Node* dim = parse_expr(p);
    (void)dim;  /* BUG: 维度被丢弃 */
    expect(p, TK_RBRACKET, "expected ']' after array dimension");
    base = type_ptr(p->arena, base);
}
```

**影响范围**：
- `typedef int arr[10];` → 生成 `typedef int* arr;` ❌ 应为 `typedef int arr[10];`
- `typedef char* str_arr[5];` → 维度丢失 ❌

**根本原因**：`parse_declarator_internal` 同时被用于两种场景：
1. **函数参数解析**（抽象模式 abstract=true）— 数组衰减为指针是正确的
2. **typedef/变量声明解析**（非抽象模式 abstract=false）— 数组维度应保留

### 修复方案

**方案 A：修改 parse_declarator_internal 签名，传递维度回调或输出参数**

在 `parse_declarator_internal` 中添加 `Node** out_array_dim` 参数，当遇到数组后缀时保存维度表达式，而不是丢弃。调用方决定如何处理维度。

```c
/* 新签名 */
static Type* parse_declarator_internal(P* p, Type* base, const char** out_name,
                                       bool abstract, Node** out_array_dim);
```

修改数组处理逻辑：

```c
if (s.kind == TK_LBRACKET) {
    lex_next(p->lex);
    if (accept(p, TK_RBRACKET)) {
        /*  Unsized array [] */
        if (!abstract && out_array_dim) {
            *out_array_dim = NULL;  /* 标记为 unsized */
        }
        base = type_ptr(p->arena, base);
    } else {
        Node* dim = parse_expr(p);
        if (!abstract && out_array_dim) {
            *out_array_dim = dim;  /* 保存维度，不丢弃 */
        }
        expect(p, TK_RBRACKET, "expected ']' after array dimension");
        /* 如果 abstract 或没有输出参数，降级为指针（向后兼容） */
        if (abstract || !out_array_dim) {
            base = type_ptr(p->arena, base);
        }
        /* 如果不降级，保持 base 不变，由外层处理维度 */
    }
}
```

**调用点修改**：

1. **typedef 外层路径**（L2871-2897）：
   ```c
   Node* dim = NULL;
   const char* tname = NULL;
   Type* ty = parse_declarator(p, base, &tname, &dim);  /* 新增 dim 输出 */
   /* 外层不再重复检查 [N] — 已由 parse_declarator 处理 */
   ```

2. **函数参数解析**（L787-789）：
   ```c
   Type* pty = parse_decl_specifiers(p);
   const char* pname;
   pty = parse_declarator_internal(p, pty, &pname, true, NULL);  /* abstract=true, 不关心维度 */
   ```

3. **嵌套匿名结构体字段**（L2669-2674）：
   ```c
   Type* fbase = parse_type(p);
   const char* fname = NULL;
   Node* fdim = NULL;
   Type* fty = parse_declarator_internal(p, fbase, &fname, false, &fdim);
   ```

4. **变量声明路径**（L3297-3300）：
   需要类似修改。

### 回归测试设计

**新增测试文件**：`test_typedef_array_dims.sp`

```sp
/* 测试 typedef 数组维度保留 */

/* 简单数组 typedef */
typedef int int_array[10];

/* 指针数组 typedef */
typedef char* str_arr[5];

/* 多维数组 typedef */
typedef int matrix[3][4];

/* 函数指针数组 */
typedef void (*handler_arr[8])(int);

int main() {
    int_array a;
    str_arr s;
    return 0;
}
```

**预期生成的 C 代码**：
```c
typedef int int_array[10];
typedef char* str_arr[5];
typedef int matrix[3][4];
typedef void (*handler_arr[8])(int);
```

---

## 修复 2（P1）：删除 wchar_t 硬编码特殊处理

### 问题分析

**当前代码**（parser.c:454-456）：
```c
} else if (t.kind == TK_IDENT && lex_ident_is(t, "wchar_t")) {
    lex_next(p->lex);
    base = type_named(p->arena, "wchar_t");
```

**问题**：解析层硬编码了 `wchar_t` 作为特殊类型名，违反了解析层/语义层关注点分离原则。

**为什么当前不影响结果**：`sema_resolve_type`（sema.c:285）会检查 `st->typedefs` 并解析 `wchar_t` 到 `int`。所以语义结果是正确的。

**架构风险**：如果未来要支持其他自定义类型别名（如 `typedef unsigned long my_wchar; typedef my_wchar wc;`），解析层不应该需要知道每一个别名。

### 修复方案

**方案 A：完全删除 wchar_t 特殊处理**

删除 L454-456 的 `wchar_t` 分支，让 `wchar_t` 走 `TK_IDENT` 分支（L529）：

```c
/* 删除后，wchar_t 会走到这里 */
} else if (t.kind == TK_IDENT) {
    /* Check if it's a known typedef (includes wchar_t registered by sema pass 4c) */
    const char* ident = arena_strndup(p->arena, t.start, t.len);
    Type* resolved = sema_resolve_type(g.st, ident, NULL);
    if (resolved && resolved->kind != TY_NAMED) {
        /* Already resolved to a primitive — use it directly */
        base = resolved;
        lex_next(p->lex);
    } else {
        /* Not yet resolved — treat as named type */
        base = type_named(p->arena, ident);
        lex_next(p->lex);
    }
}
```

**但等等**：这个方案有问题。`parse_type` 在 Parser 阶段被调用，此时 `g.st`（语义符号表）尚未初始化（sema 在 Parser 之后运行）。所以不能在 parse_type 中调用 sema_resolve_type。

**方案 B：保留 wchar_t 解析层处理，但添加注释说明原因**

这是更务实的方案。原因：
1. `wchar_t` 是 C 标准规定的特殊类型名（C99 §7.17）
2. 它在 Parser 阶段就需要被识别，因为系统头文件中的代码需要正确的类型解析
3. sema pass 4c 的 wchar_t 注册是为了**用户代码**中的 wchar_t 解析
4. 解析层的 wchar_t 处理是为了**系统头文件**中出现的 wchar_t

```c
/* wchar_t is a C-standard typedef for an integer type. We handle it
 * specially in the parser because:
 *   (a) System headers (corecrt.h, wchar.h) use wchar_t extensively
 *       before sema pass 4c runs, so we need parser-level recognition.
 *   (b) User code may still define `typedef int wchar_t;` for sharpc's
 *       own type system — sema pass 4c handles that separately.
 * This is not a violation of parser/semantic separation — wchar_t is
 * a built-in type in C, similar to int/char, just with an unusual name. */
} else if (t.kind == TK_IDENT && lex_ident_is(t, "wchar_t")) {
    lex_next(p->lex);
    base = type_named(p->arena, "wchar_t");
```

**结论**：保留当前实现，添加详细注释说明原因。这是一个**架构决策**而非 bug。

---

## 修复 3（P2）：合并 builtin_types 到 is_crt_sdk_typedef

### 问题分析

`size_t` 和 `ptrdiff_t` 出现在两个地方：
1. `is_crt_sdk_typedef` skip list（cgen.c:1645-1660）
2. `emit_typedef_decl` builtin_types 数组（cgen.c:1677-1679）

这导致维护者需要知道两个列表的关系。

### 修复方案

将 C 编译器内建类型（不能被用户重新 typedef 的）与普通 CRT typedef 分开，但统一在 `emit_typedef_decl` 的一个条件中处理：

```c
/* Phase C1: Typedef emission filtering.
 *
 * Rule 1: System header typedefs with CRT names → skip
 *   (provided by the C runtime, don't re-emit)
 *
 * Rule 2: C compiler builtin types → always skip
 *   (size_t, ptrdiff_t, nullptr_t, max_align_t are compiler intrinsics
 *    that cannot be re-typed in C source, even by user code)
 */
static void emit_typedef_decl(G* g, Node* d) {
    /* Rule 1: System header CRT typedefs */
    if (is_system_header_path(d->source_file) && is_crt_sdk_typedef(d->name)) {
        return;
    }

    /* Rule 2: C compiler builtin types (from __SIZE_TYPE__ etc.) */
    if (is_c_compiler_builtin_type(d->name)) {
        return;
    }

    /* User-defined typedef — emit normally */
    ...
}

static bool is_c_compiler_builtin_type(const char* name) {
    static const char* builtins[] = {
        "size_t", "ptrdiff_t", "nullptr_t", "max_align_t",
        NULL
    };
    for (int i = 0; builtins[i]; i++)
        if (strcmp(name, builtins[i]) == 0) return true;
    return false;
}
```

这样职责清晰：
- `is_crt_sdk_typedef`：系统头文件中的类型名列表（va_list, wchar_t, 各种 intN_t）
- `is_c_compiler_builtin_type`：C 编译器内建类型（不能被用户重新 typedef）

---

## 修复 4（P3）：is_system_header_path 路径覆盖

### 问题分析

当前路径模式覆盖：
- ✅ TCC/MinGW (tcc/include, mingw, msys64, /usr/include)
- ✅ Windows SDK (Windows Kits, Microsoft Visual Studio)

缺失：
- ❌ Cygwin (`/cygdrive/`, `/usr/include/w32api`)
- ❌ 自定义 SDK 安装路径
- ❌ LLVM/Clang 自带 headers (`lib/clang/*/include`)

### 修复方案

添加更多路径模式：

```c
static inline bool is_system_header_path(const char* path) {
    if (!path) return false;
    /* TCC / MinGW / MSYS / Cygwin */
    if (strstr(path, "tcc/include") != NULL) return true;
    if (strstr(path, "third_party\\tcc\\include") != NULL) return true;
    if (strstr(path, "third_party/tcc/include") != NULL) return true;
    if (strstr(path, "mingw") != NULL) return true;
    if (strstr(path, "msys64") != NULL) return true;
    if (strstr(path, "/usr/include") != NULL) return true;
    if (strstr(path, "/cygdrive/") != NULL) return true;
    if (strstr(path, "w32api") != NULL) return true;
    /* Windows SDK / UCRT */
    if (strstr(path, "Windows Kits") != NULL) return true;
    if (strstr(path, "Microsoft Visual Studio") != NULL) return true;
    /* LLVM / Clang built-in headers */
    if (strstr(path, "lib/clang/") != NULL) return true;
    if (strstr(path, "lib\\clang\\") != NULL) return true;
    return false;
}
```

**注意**：这是一个渐进式改进。任何新发现的系统头文件路径都可以通过添加一条规则来覆盖。

---

## 执行顺序与工作量

| 步骤 | 修改文件 | 预估行数 | 风险 |
|------|---------|---------|------|
| 1. 修复 parse_declarator 数组维度 | parser.c, sharp.h | ~40 行 | **高** — 影响所有 declarator 解析 |
| 2. 修复 7 处调用点 | parser.c | ~20 行 | **中** — 需要同步更新 |
| 3. 添加 wchar_t 注释 | parser.c | +8 行 | **低** — 纯文档 |
| 4. 重构 builtin_types | cgen.c | ~10 行 | **低** — 纯重构 |
| 5. 扩展系统头路径 | sharp.h | +4 行 | **低** — 纯配置 |
| 6. 新增测试 | tests/ | ~60 行 | — |

---

## 回归测试矩阵

| 测试 | 验证目标 | 类型 |
|------|---------|------|
| 现有 159 测试 | 无回归 | 自动化 |
| test_typedef_array_dims.sp | 数组维度保留 | 新增 |
| test_typedef_array_of_fnptr.sp | 函数指针数组 | 新增 |
| test_typedef_multidim.sp | 多维数组 | 新增 |
| test_user_crt_compound.sp | 用户 CRT 名+复合类型（已存在）| 已有 |
