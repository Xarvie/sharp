# sharpc C 语言超集支持 — 完整实现计划

## 一、当前状态总结

### 已支持的 C 特性（50+ 项）

| 类别 | 特性 | 测试文件 |
|------|------|---------|
| 类型系统 | SP 原生类型映射 (i8~usize, f32/f64) | test_types_all.sp |
| 类型系统 | C 类型透传 (int, long, unsigned, wchar_t, __int64) | test_types_c_passthrough.sp |
| 类型系统 | _Bool, long long, typedef 基础 | test_bool_c.sp, test_longlong.sp |
| 调用约定 | __cdecl, __stdcall, __fastcall, __unaligned | test_calling_conv.sp |
| 声明修饰 | __declspec(noreturn/dllimport) | test_declspec_*.sp |
| 内联函数 | extern __inline__, __inline__ | test_*inline*.sp |
| 变量 | const, extern, static, 全局变量 | test_const_*.sp, test_extern_simple.sp |
| 控制流 | if/else, for, while, break, continue, return | test_if.sp, test_for.sp, test_while.sp |
| 表达式 | 算术运算, 逻辑运算, 指针, 数组, 布尔 | test_arith.sp, test_ptr.sp, test_bool.sp |
| 结构体 | struct/union 定义, typedef struct, 前向声明 | test_c_struct_union.sp |
| 结构体 | struct + impl + 方法调用 | test_struct_impl.sp |
| 结构体 | 柔性数组 [1] | test_flex_array.sp |
| 泛型 | 泛型 struct + impl | test_generic.sp |
| 预处理器 | #define, #ifdef, #include, 宏展开, 条件编译 | test_token_pasting.sp, test_strict_ansi.sp |
| 预处理器 | #pragma pack, push_macro/pop_macro, GCC system_header | test_pragma_*.sp |
| 内联汇编 | __asm__ 块透传 | test_asm_basic.sp |
| SEH | __try/__except/__finally | test_seh.sp |
| 属性 | __attribute__((mode(DI))), __attribute__((format...)) | test_attribute_mode.sp, test_c_modifiers.sp |
| 内建函数 | __builtin_frame_address, __alignof__, alloca, _malloca | test_builtin_*.sp, test_alloca.sp |
| 泛型 | 泛型 struct + impl | test_generic.sp |
| 字面量 | NAN/INFINITY, MSVC 整数字面量后缀 | test_nan_infinity.sp, test_msvc_int_suffix.sp |
| 类型转换 | C 风格强转 (type)(expr) | test_cast_sentinel.sp |
| extern 块 | extern "C++" { } | test_extern_cpp.sp |
| 变参函数 | extern i32 printf(const char*, ...) | test_variadic_extern.sp |
| 调用约定+变参 | extern i32 __cdecl printf(...) | test_complex_extern.sp |
| restrict | __restrict__ 指针修饰 | test_restrict.sp |

### 不支持的 C 特性（13 项，按优先级排列）

| 优先级 | 编号 | 特性 | 测试文件 | stdio.h 中的实际例子 |
|--------|------|------|---------|---------------------|
| **P0** | §33 | 匿名结构体/联合体 | test_anonymous_struct.sp | `struct { int a; int b; };` |
| **P0** | §34 | __declspec 修饰结构体字段 | test_struct_field_declspec.sp | `char* __declspec(nothrow) _ptr;` |
| **P0** | §35 | 复杂 typedef 链 | test_complex_typedef.sp | `typedef struct X {} X, *PX;` |
| **P0** | §36 | 函数指针参数 | test_func_pointer_param.sp | `int (*compar)(const void*, const void*)` |
| **P1** | §37 | 指定初始化器 | test_designated_init.sp | `{ .x = 1, .y = 2 }` |
| **P1** | §38 | 复合字面量 | test_compound_literal.sp | `(struct Point){ .x = 1 }` |
| **P1** | §39 | 位域 | test_bitfield.sp | `u32 flag : 1;` |
| **P1** | §40 | 线程局部存储 | test_thread_local.sp | `__declspec(thread) int x;` |
| **P2** | §41 | 复杂 __attribute__ | test_complex_attribute.sp | `__attribute__((aligned(16)))` |
| **P2** | §42 | 嵌套结构体定义 | test_nested_struct.sp | `struct Outer { struct Inner {} inner; }` |
| **P2** | §43 | restrict 修饰函数参数 | test_restrict_param.sp | `char* restrict dest` |
| **P2** | §44 | 函数指针变量 | test_func_pointer.sp | `int (*fp)(int, int) = null;` |
| **P2** | §45 | __extension__ 关键字 | test_extension.sp | `__extension__ long long x;` |

---

## 二、核心设计原则

1. **Parser 是 C 解析器，不是 SP 解析器** — Parser 应该能解析完整的 C99/C11 语法
2. **SP 是 C 的超集** — 所有 C 代码都应该是合法的 SP 代码
3. **不区分 C/SP 模式** — Parser 自动识别语法，不需要模式切换
4. **生成的 C 代码必须完全等价** — cgen 输出的 C 代码应能通过任何 C 编译器

---

## 三、实现计划

### Phase 1: 结构体/联合体增强（P0 — 4 项）

**目标：** 让 Parser 能解析 stdio.h 中的结构体定义

#### 1.1 匿名结构体/联合体 (§33)

**文件修改：** `parser.c` — `parse_struct_decl()`, `parse_union_decl()`

**当前行为：** 解析器期望 `struct Name { ... };` 或 `struct Name;`

**目标行为：**
```c
// 匿名结构体作为字段
struct Outer {
    i32 x;
    struct {    // 匿名，无名称
        i32 a;
        i32 b;
    };
    union {     // 匿名，无名称
        i32 c;
        f64 d;
    };
}
```

**实现：**
- 在 `parse_struct_decl()` 中，如果 `struct` 后面直接跟 `{`（没有名称），解析为匿名结构体
- 在字段列表中，如果遇到 `struct {` 或 `union {`，递归解析并标记为匿名
- AST 新增 `ND_ANONYMOUS_STRUCT` 和 `ND_ANONYMOUS_UNION` 节点
- cgen 输出时不生成字段名，直接输出内部字段

#### 1.2 __declspec 修饰结构体字段 (§34)

**文件修改：** `parser.c` — `parse_field_decl()`

**当前行为：** 字段声明格式为 `Type field_name;`

**目标行为：**
```c
struct _iobuf {
    char* _ptr;
    i32 _cnt;
    char* __declspec(nothrow) _base;  // __declspec 在类型之后、字段名之前
    i32 _flag;
}
```

**实现：**
- 在 `parse_field_decl()` 中，解析完类型后，检查是否有 `__declspec` token
- 如果有，调用 `parse_declspec()` 捕获内容，存储在 Node.declspec 字段
- cgen 输出时：`type __declspec(content) name;`

#### 1.3 复杂 typedef 链 (§35)

**文件修改：** `parser.c` — `parse_typedef()`

**当前行为：** `typedef struct Name { ... } Alias;`（单个别名）

**目标行为：**
```c
// 多个别名
typedef struct FILE { ... } FILE, *PFILE, **PPFILE;

// 函数指针 typedef
typedef void (*sighandler_t)(i32);
typedef i32 (*cmp_fn)(const void*, const void*);
```

**实现：**
- 在 `parse_typedef()` 中，解析完第一个别名后，检查是否有 `,`
- 如果有，继续解析更多别名（可能是指针类型 `*PFILE`）
- 支持函数指针语法的 typedef：`(*name)(params)`

#### 1.4 函数指针参数 (§36)

**文件修改：** `parser.c` — `parse_param_list()`

**当前行为：** 参数格式为 `Type name`

**目标行为：**
```c
extern void qsort(void* base, size_t nmemb, size_t size,
                  i32 (*compar)(const void*, const void*));
```

**实现：**
- 在 `parse_param()` 中，解析参数时检查是否为函数指针语法
- 如果看到 `(*` 或 `(*name)`，解析为函数指针类型
- AST 新增 `ND_FUNC_PTR_PARAM` 节点

**预计工作量：** 3 天

---

### Phase 2: 表达式增强（P1 — 4 项）

#### 2.1 指定初始化器 (§37)

**文件修改：** `parser.c` — `parse_init_expr()`, `parse_primary()`

**目标行为：**
```c
struct Point p = { .x = 1, .y = 2 };
int arr[3] = { [1] = 5, [2] = 10 };
```

**实现：**
- 在 `parse_init_expr()` 中，解析 `{` 后的内容
- 如果遇到 `.field =` 语法，解析为指定初始化器
- 如果遇到 `[index] =` 语法，解析为数组指定初始化器
- AST 新增 `ND_DESIGNATED_INIT` 节点

#### 2.2 复合字面量 (§38)

**文件修改：** `parser.c` — `parse_primary()`

**目标行为：**
```c
struct Point* p = &(struct Point){ .x = 1, .y = 2 };
```

**实现：**
- 在 `parse_primary()` 中，如果遇到 `(Type){`，解析为复合字面量
- AST 新增 `ND_COMPOUND_LIT` 节点

#### 2.3 位域 (§39)

**文件修改：** `parser.c` — `parse_field_decl()`

**目标行为：**
```c
struct Flags {
    u32 a : 1;
    u32 b : 2;
    u32 c : 4;
}
```

**实现：**
- 在 `parse_field_decl()` 中，解析完 `name;` 后，检查是否有 `:`
- 如果有，解析位宽表达式（通常是整数常量）
- AST 新增 `ND_BITFIELD` 字段或给 ND_FIELD 添加 bit_width 属性

#### 2.4 线程局部存储 (§40)

**文件修改：** `parser.c` — `parse_top_level()`, `parse_decl()`

**目标行为：**
```c
__declspec(thread) i32 tls_var1;
__thread i32 tls_var2;
```

**实现：**
- 在 `parse_top_level()` 中，检测到 `__declspec(thread)` 时，标记为 TLS 变量
- 或者检测 `__thread` 关键字
- AST 新增 `ND_TLS_DECL` 节点
- cgen 输出时：`__declspec(thread) type name;` 或 `__thread type name;`

**预计工作量：** 2 天

---

### Phase 3: 其他特性（P2 — 5 项）

#### 3.1 复杂 __attribute__ (§41)

**文件修改：** `parser.c` — `parse_attribute()`

**目标行为：**
```c
i32 x __attribute__((aligned(16), unused));
void* p __attribute__((malloc, aligned(16)));
```

**实现：**
- 扩展现有的 `skip_attribute()` 为完整的 `parse_attribute()`
- 解析属性列表中的多个属性，支持带参数的属性
- AST 新增 `ND_ATTR_COMPLEX` 节点

#### 3.2 嵌套结构体定义 (§42)

**文件修改：** `parser.c` — `parse_struct_decl()`

**目标行为：**
```c
struct Outer {
    i32 x;
    struct Inner {
        i32 a;
        i32 b;
    } inner;
}
```

**实现：**
- 在 `parse_struct_decl()` 中，字段类型可能是另一个 struct 定义
- 递归调用 `parse_struct_decl()` 解析嵌套定义
- AST 中嵌套结构体作为字段的类型

#### 3.3 restrict 修饰函数参数 (§43)

**文件修改：** `parser.c` — `parse_param_list()`

**目标行为：**
```c
extern char* strncpy(char* restrict dest, const char* restrict src, usize n);
```

**实现：**
- 在 `parse_param()` 中，解析完类型和名称后，检查是否有 `restrict`
- 如果有，标记参数为 restrict
- AST 给 ND_PARAM 添加 `is_restrict` 标志

#### 3.4 函数指针变量 (§44)

**文件修改：** `parser.c` — `parse_local_decl()`, `parse_stmt()`

**目标行为：**
```c
i32 main() {
    i32 (*fp)(i32, i32) = null;
    i32 result = (*fp)(1, 2);
    return result;
}
```

**实现：**
- 在 `parse_local_decl()` 中，检测函数指针语法 `(*name)(params)`
- 解析函数指针类型和初始化表达式
- AST 新增 `ND_FUNC_PTR_DECL` 节点

#### 3.5 __extension__ 关键字 (§45)

**文件修改：** `parser.c` — `parse_top_level()`, `parse_stmt()`

**目标行为：**
```c
__extension__ i64 x = 10000000000;
__extension__ long long y;
```

**实现：**
- 添加 `TK___EXTENSION__` token
- 在 top-level 和 statement 解析中，检测到 `__extension__` 时跳过该关键字
- 继续解析后续声明

**预计工作量：** 2 天

---

## 四、AST 扩展清单

需要在 `sharp.h` 中新增的 NodeKind：

```c
typedef enum {
    // ... existing nodes ...
    
    /* C-specific constructs */
    ND_ANONYMOUS_STRUCT,   /* anonymous struct inside struct/union */
    ND_ANONYMOUS_UNION,    /* anonymous union inside struct */
    ND_BITFIELD,           /* field : width — bit field */
    ND_DESIGNATED_INIT,    /* .field = value or [index] = value */
    ND_COMPOUND_LIT,       /* (Type){ ... } — compound literal */
    ND_FUNC_PTR_DECL,      /* type (*name)(params) = init; */
    ND_FUNC_PTR_PARAM,     /* type (*name)(params) as function parameter */
    ND_TLS_DECL,           /* __declspec(thread) / __thread variable */
    ND_ATTR_COMPLEX,       /* __attribute__((aligned(16), unused)) */
} NodeKind;
```

需要在 `Node` 结构体中新增的字段：

```c
struct Node {
    // ... existing fields ...
    int bitfield_width;       /* for ND_BITFIELD */
    bool is_restrict;         /* for ND_PARAM */
    bool is_anonymous;        /* for ND_STRUCT/ND_UNION */
    const char* init_designators; /* for ND_COMPOUND_LIT */
    // ... etc
};
```

---

## 五、验证计划

### 单元测试

每个新特性对应一个测试文件：

| 编号 | 测试文件 | 特性 | 验证标准 |
|------|---------|------|---------|
| §33 | test_anonymous_struct.sp | 匿名结构体/联合体 | C 代码生成正确，GCC 编译通过 |
| §34 | test_struct_field_declspec.sp | __declspec 字段 | C 代码生成正确 |
| §35 | test_complex_typedef.sp | 复杂 typedef 链 | C 代码生成正确 |
| §36 | test_func_pointer_param.sp | 函数指针参数 | C 代码生成正确 |
| §37 | test_designated_init.sp | 指定初始化器 | C 代码生成正确 |
| §38 | test_compound_literal.sp | 复合字面量 | C 代码生成正确 |
| §39 | test_bitfield.sp | 位域 | C 代码生成正确 |
| §40 | test_thread_local.sp | 线程局部存储 | C 代码生成正确 |
| §41 | test_complex_attribute.sp | 复杂 __attribute__ | C 代码生成正确 |
| §42 | test_nested_struct.sp | 嵌套结构体 | C 代码生成正确 |
| §43 | test_restrict_param.sp | restrict 参数 | C 代码生成正确 |
| §44 | test_func_pointer.sp | 函数指针变量 | C 代码生成正确 |
| §45 | test_extension.sp | __extension__ | C 代码生成正确 |

### 集成测试

| 测试文件 | 验证内容 |
|---------|---------|
| test_stdio_h.sp | `#include <stdio.h>` 完整解析 + 使用 printf |
| test_stdlib_h.sp | `#include <stdlib.h>` 完整解析 + 使用 malloc |
| test_math_h.sp | `#include <math.h>` 完整解析 + 使用 sin/cos |

### 回归测试

```powershell
.\run_tests.ps1
```

- 所有现有 58 个测试必须通过
- 新增 13 个测试必须通过
- 新增 3 个集成测试必须通过

---

## 六、工作量估算

| 阶段 | 特性数 | 工作量 | 优先级 |
|------|--------|--------|--------|
| Phase 1: 结构体/联合体增强 | 4 | 3 天 | P0 |
| Phase 2: 表达式增强 | 4 | 2 天 | P1 |
| Phase 3: 其他特性 | 5 | 2 天 | P2 |
| 集成测试 | 3 | 1 天 | P0 |
| **总计** | **16** | **8 天** | |

---

## 七、风险与缓解

### 风险 1: Parser 复杂度增加
**缓解：** 每个特性独立实现，有对应的测试文件验证

### 风险 2: C 语法与 SP 语法冲突
**缓解：** 统一语法解析，不需要模式切换。SP 语法是 C 的子集，所有 C 构造都可以直接解析

### 风险 3: cgen 输出不正确
**缓解：** cgen 对每个新 AST 节点都有对应的代码生成逻辑，确保输出与原输入语义等价

---

## 八、长期愿景

完成此计划后，sharpc 将成为：
1. **完整的 C 语言超集** — 能直接 `#include` 任何标准库头文件
2. **一等编程语言** — 支持所有 C 构造，无"不支持"的例外
3. **向后兼容** — 所有 C 代码都能直接在 SP 中编译
4. **向前扩展** — 在 C 基础上添加 SP 特性（impl、泛型、RAII 等）
