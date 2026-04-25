# SP→C 编译器：编译期特性 TDD 测试手册

> **约定**
> - 每个测试用例由三部分组成：**SP 源码输入** → **期望 C 输出** → **验证方法**
> - SP 额外类型映射规则（全局适用）：
>
> | SP 类型 | 输出 C 类型 |
> |---------|------------|
> | `i8`    | `int8_t`   |
> | `i16`   | `int16_t`  |
> | `i32`   | `int32_t`  |
> | `i64`   | `int64_t`  |
> | `u8`    | `uint8_t`  |
> | `u16`   | `uint16_t` |
> | `u32`   | `uint32_t` |
> | `u64`   | `uint64_t` |
> | `usize` | `size_t`   |
> | `isize` | `ptrdiff_t`|
> | `char`  | `char`     |
> | `int`   | `int`      |
> | (标准 C 类型) | 原样透传 |
>
> - 测试结构：`sp_compile(input) == expected_c_output`
> - 编译验证：期望输出的 C 代码必须能被 `gcc -std=c11 -Wall -Wextra -pedantic` 无错编译

---

## 目录

1. [SP 原生类型映射](#1-sp-原生类型映射)
2. [调用约定关键字](#2-调用约定关键字)
3. [`__declspec` 扩展](#3-__declspec-扩展)
4. [MSVC 整数类型关键字](#4-msvc-整数类型关键字)
5. [MSVC 整数字面量后缀 `i8`/`i16`/`i32`/`i64`](#5-msvc-整数字面量后缀)
6. [`__inline__` / `static __inline` / `__CRT_INLINE`](#6-__inline__--static-__inline--__crt_inline)
7. [`__attribute__` 集合](#7-__attribute__-集合)
8. [GCC 内联汇编 `__asm__`](#8-gcc-内联汇编-__asm__)
9. [`#pragma push_macro` / `pop_macro`](#9-pragma-push_macro--pop_macro)
10. [`#pragma pack`](#10-pragma-pack)
11. [`_Bool` 类型](#11-_bool-类型)
12. [`__restrict__` 关键字](#12-__restrict__-关键字)
13. [编译器内建宏类型 `__SIZE_TYPE__` 等](#13-编译器内建宏类型)
14. [`extern "C++"` 块](#14-extern-c-块)
15. [SEH 异常处理宏](#15-seh-异常处理宏)
16. [`va_list` 多架构实现](#16-va_list-多架构实现)
17. [`sizeof` 类型派发宏（`fpclassify`/`signbit`）](#17-sizeof-类型派发宏)
18. [`NAN`/`INFINITY` 浮点常量定义](#18-naninfinity-浮点常量定义)
19. [强转指针哨兵值](#19-强转指针哨兵值)
20. [`#pragma GCC system_header`](#20-pragma-gcc-system_header)
21. [编译期静态断言老式写法](#21-编译期静态断言老式写法)
22. [`alloca` / `__builtin_alloca`](#22-alloca--__builtin_alloca)
23. [`__need_wint_t` 条件 typedef 模式](#23-__need_wint_t-条件-typedef-模式)
24. [柔性数组成员变种 `[1]`](#24-柔性数组成员变种-1)
25. [`L##` 宽字符串 token 拼接](#25-l-宽字符串-token-拼接)
26. [`__attribute__((mode(DI)))`](#26-__attributemodedie)
27. [`__builtin_frame_address` / `__alignof__`](#27-__builtin_frame_address--__alignof__)
28. [`__STRICT_ANSI__` 条件编译](#28-__strict_ansi__-条件编译)
29. [`_malloca` / `_freea` 智能栈堆分配](#29-_malloca--_freea-智能栈堆分配)
30. [双层宏展开 token-pasting](#30-双层宏展开-token-pasting)

---

## 总 TDD 测试映射表

> **图例：** ✅ 已完成 📝 有正文用例（无独立测试文件） ❌ 测试文件编译失败 ⬜ 待实现

| 测试文件 | TDD 编号 | 覆盖特性 | 状态 |
|---|---|---|---|
| test_const_basic.sp | TDD-Const-1 | const 局部变量声明 | ✅ |
| test_const_fn.sp | TDD-Const-2 | const 函数参数 | ✅ |
| test_const_ptr.sp | TDD-Const-3 | const 指针变体 | ✅ |
| test_c_inline.sp | TDD-Inline-1 | extern __inline__ + C int 函数 | ✅ |
| test_c_misc.sp | TDD-Typedef-2 | typedef int errno_t + 使用 | ✅ |
| test_c_modifiers.sp | TDD-DeclSpec-2 / TDD-Attribute-1 | __declspec(dllimport) / __attribute__((format...)) | ✅ |
| test_c_struct_union.sp | TDD-Struct-1~3 | union、struct 前向声明、typedef struct {} | ✅ |
| test_c_types.sp | TDD-CType-1~3 | __int64、wchar_t、long long | ✅ |
| test_c_va_list.sp | TDD-VA-1 | va_list typedef + extern __inline__ | ✅ |
| test_extern_inline.sp | TDD-Inline-2 | extern inline function with body | ✅ |
| test_extern_inline2.sp | TDD-Inline-3 | extern __inline__ function | ✅ |
| test_extern_simple.sp | TDD-Extern-1 | extern 函数声明（无 body） | ✅ |
| test_inline_noextern.sp | TDD-Inline-4 | __inline__ 无 extern | ✅ |
| test_inline_simple.sp | TDD-Inline-5 | extern __inline__ | ✅ |
| test_inline_var.sp | TDD-Inline-6 | __inline__ 局部变量（跳过修饰符） | ✅ |
| test_int_var.sp | TDD-CType-4 | int 基本类型变量 | ✅ |
| test_longlong.sp | TDD-CType-5 | typedef long long | ✅ |
| test_print.sp | TDD-Stmt-1 | print / println built-in | ✅ |
| test_while.sp | TDD-Stmt-2 | while 循环 | ✅ |
| test_if.sp | TDD-Stmt-3 | if / else | ✅ |
| test_for.sp | TDD-Stmt-4 | for 循环 | ✅ |
| test_struct_impl.sp | TDD-Struct-4 | struct + impl + 方法调用 | ✅ |
| test_ptr.sp | TDD-Expr-1 | 指针、取地址、解引用 | ✅ |
| test_break_continue.sp | TDD-Stmt-5 | break / continue | ✅ |
| test_arith.sp | TDD-Expr-2 | 算术与比较运算 | ✅ |
| test_bool.sp | TDD-Expr-4 | bool 字面量与逻辑运算 | ✅ |
| test_array.sp | TDD-Expr-3 | 数组声明与索引 | ✅ |
| test_generic.sp | TDD-Generic-1 | 泛型 struct + impl | ✅ |
| test_types_all.sp | TDD-1.1~1.3 | SP 原生类型映射（i8~usize 等）| ✅ |
| test_types_c_passthrough.sp | TDD-1.4 | 标准 C 类型原样透传 | ✅ |
| test_calling_conv.sp | TDD-2.1~2.4 | 调用约定关键字（__cdecl/__stdcall/__fastcall/__unaligned）| ✅ |
| `test_declspec_noreturn.sp` | TDD-3.1 | `__declspec(noreturn)` 函数声明 | ✅ |
| `test_declspec_dllimport.sp` | TDD-3.2 | `__declspec(dllimport)` 外部变量声明 | ✅ |
| `test_msvc_int_suffix.sp` | §5 | MSVC 整数字面量后缀 `i8`/`i16`/`i32`/`i64` | ✅ |
| `test_asm_basic.sp` | §8 | GCC 内联汇编 `__asm__` | ✅ |
| `test_pragma_push_pop.sp` | §9 | `#pragma push_macro` / `pop_macro` | ✅ |
| `test_pragma_pack.sp` | §10 | `#pragma pack` | ✅ |
| `test_bool_c.sp` | §11 | `_Bool` 类型 | ✅ |
| `test_restrict.sp` | §12 | `__restrict__` 关键字 | ✅ |
| `test_builtin_macro_types.sp` | §13 | 编译器内建宏类型 `__SIZE_TYPE__` 等 | ✅ |
| `test_extern_cpp.sp` | §14 | `extern "C++"` 块 | ✅ |
| `test_seh.sp` | §15 | SEH 异常处理宏 | ✅ |
| `test_sizeof_dispatch.sp` | §17 | `sizeof` 类型派发宏（`fpclassify`/`signbit`）| ✅ |
| `test_nan_infinity.sp` | §18 | `NAN`/`INFINITY` 浮点常量定义 | ✅ |
| `test_cast_sentinel.sp` | §19 | 强转指针哨兵值 | ✅ |
| `test_pragma_system_header.sp` | §20 | `#pragma GCC system_header` | ✅ |
| `test_static_assert_old.sp` | §21 | 编译期静态断言老式写法 | ✅ |
| `test_alloca.sp` | §22 | `alloca` / `__builtin_alloca` | ✅ |
| `test_need_wint_t.sp` | §23 | `__need_wint_t` 条件 typedef 模式 | ✅ |
| `test_flex_array.sp` | §24 | 柔性数组成员变种 `[1]` | ✅ |
| `test_wide_string.sp` | §25 | `L##` 宽字符串 token 拼接 | ✅ |
| `test_attribute_mode.sp` | §26 | `__attribute__((mode(DI)))` | ✅ |
| `test_builtin_frame.sp` | §27 | `__builtin_frame_address` / `__alignof__` | ✅ |
| `test_strict_ansi.sp` | §28 | `__STRICT_ANSI__` 条件编译 | ✅ |
| `test_malloca.sp` | §29 | `_malloca` / `_freea` 智能栈堆分配 | ✅ |
| `test_token_pasting.sp` | §30 | 双层宏展开 token-pasting | ✅ |
| `test_complex_extern.sp` | §31 | 复杂 extern（调用约定+变参） | ✅ |
| `test_variadic_extern.sp` | §32 | 变参函数声明（`...`） | ✅ |
| `test_anonymous_struct.sp` | §33 | 匿名结构体/联合体 | ❌ |
| `test_struct_field_declspec.sp` | §34 | `__declspec` 修饰结构体字段 | ❌ |
| `test_complex_typedef.sp` | §35 | 复杂 typedef 链（`typedef struct X {} X, *PX;`） | ❌ |
| `test_func_pointer_param.sp` | §36 | 函数指针参数（`int (*compar)(const void*, const void*)`） | ❌ |
| `test_designated_init.sp` | §37 | 指定初始化器（`.field = value`） | ❌ |
| `test_compound_literal.sp` | §38 | 复合字面量（`(Type){ ... }`） | ❌ |
| `test_bitfield.sp` | §39 | 位域（`unsigned int x : 3;`） | ❌ |
| `test_thread_local.sp` | §40 | 线程局部存储（`__declspec(thread)`） | ❌ |
| `test_complex_attribute.sp` | §41 | 复杂 `__attribute__`（`aligned(16)`, `packed` 等） | ❌ |
| `test_nested_struct.sp` | §42 | 嵌套结构体定义（`struct Outer { struct Inner { ... } inner; }`） | ❌ |
| `test_restrict_param.sp` | §43 | `restrict` 修饰函数参数 | ❌ |
| `test_func_pointer.sp` | §44 | 函数指针变量声明（`int (*fp)(int, int);`） | ❌ |
| `test_extension.sp` | §45 | `__extension__` 关键字 | ❌ |

---

## 已支持的 C 特性清单

> 以下特性已通过测试验证，可以正常使用。

| 编号 | 特性 | 测试文件 | 说明 |
|------|------|---------|------|
| §1~§7 | SP 原生类型映射 | test_types_all.sp | i8~usize, f32/f64, bool, char, void |
| §1 | C 类型透传 | test_types_c_passthrough.sp | int, long, unsigned 等原样透传 |
| §2.1~2.4 | 调用约定 | test_calling_conv.sp | __cdecl, __stdcall, __fastcall, __unaligned |
| §3.1 | __declspec(noreturn) | test_declspec_noreturn.sp | 函数声明 |
| §3.2 | __declspec(dllimport) | test_declspec_dllimport.sp | 外部变量声明 |
| §5 | MSVC 整数字面量后缀 | test_msvc_int_suffix.sp | i8/i16/i32/i64 |
| §8 | GCC 内联汇编 | test_asm_basic.sp | __asm__ 块透传 |
| §9 | push_macro/pop_macro | test_pragma_push_pop.sp | #pragma push_macro("X") |
| §10 | #pragma pack | test_pragma_pack.sp | #pragma pack(push, 1) |
| §11 | _Bool 类型 | test_bool_c.sp | C99 _Bool |
| §12 | __restrict__ 关键字 | test_restrict.sp | 指针修饰符 |
| §13 | 编译器内建宏 | test_builtin_macro_types.sp | __SIZE_TYPE__ 等 |
| §14 | extern "C++" 块 | test_extern_cpp.sp | extern "C++" { ... } |
| §15 | SEH 异常处理 | test_seh.sp | __try/__except/__finally |
| §17 | sizeof 类型派发 | test_sizeof_dispatch.sp | fpclassify/signbit 宏 |
| §18 | NAN/INFINITY | test_nan_infinity.sp | 浮点常量 |
| §19 | 强转指针哨兵值 | test_cast_sentinel.sp | (type*)(expr) |
| §20 | #pragma GCC system_header | test_pragma_system_header.sp | 抑制警告 |
| §21 | _Static_assert | test_static_assert_old.sp | 编译期断言 |
| §22 | alloca | test_alloca.sp | 栈分配 |
| §23 | __need_wint_t 模式 | test_need_wint_t.sp | 条件 typedef |
| §24 | 柔性数组 [1] | test_flex_array.sp | struct { int data[1]; } |
| §25 | L## 宽字符串 | test_wide_string.sp | 宽字符常量 |
| §26 | __attribute__((mode(DI))) | test_attribute_mode.sp | GCC 模式属性 |
| §27 | __builtin_frame_address | test_builtin_frame.sp | __alignof__ |
| §28 | __STRICT_ANSI__ | test_strict_ansi.sp | 条件编译 |
| §29 | _malloca/_freea | test_malloca.sp | Windows 栈分配 |
| §30 | 双层宏展开 | test_token_pasting.sp | ## token pasting |
| §31 | 复杂 extern（调用约定+变参） | test_complex_extern.sp | extern i32 __cdecl printf(...) |
| §32 | 变参函数声明 | test_variadic_extern.sp | extern i32 printf(const char*, ...) |

---

## 不支持的 C 特性清单

> 以下特性尚未实现，测试文件已创建但编译失败。优先级按 stdio.h 等标准库头文件中的实际需求排列。

### P0 — 阻塞标准库头文件支持

| 编号 | 特性 | 测试文件 | 错误信息 | stdio.h 中的例子 |
|------|------|---------|---------|----------------|
| §33 | 匿名结构体/联合体 | test_anonymous_struct.sp | `expected field name (got token 'i32')` | `struct { int a; int b; };` 作为字段 |
| §34 | __declspec 修饰结构体字段 | test_struct_field_declspec.sp | `expected field name` | `char* __declspec(nothrow) _ptr;` |
| §35 | 复杂 typedef 链 | test_complex_typedef.sp | 解析失败 | `typedef struct X {} X, *PX, **PPX;` |
| §36 | 函数指针参数 | test_func_pointer_param.sp | 解析失败 | `int (*compar)(const void*, const void*)` |

### P1 — 重要但非阻塞

| 编号 | 特性 | 测试文件 | 错误信息 | 例子 |
|------|------|---------|---------|------|
| §37 | 指定初始化器 | test_designated_init.sp | 解析失败 | `{ .x = 1, .y = 2 }` |
| §38 | 复合字面量 | test_compound_literal.sp | 解析失败 | `(struct Point){ .x = 1 }` |
| §39 | 位域 | test_bitfield.sp | 解析失败 | `u32 flag : 1;` |
| §40 | 线程局部存储 | test_thread_local.sp | 解析失败 | `__declspec(thread) int x;` |

### P2 — 低优先级

| 编号 | 特性 | 测试文件 | 错误信息 | 例子 |
|------|------|---------|---------|------|
| §41 | 复杂 __attribute__ | test_complex_attribute.sp | 解析失败 | `__attribute__((aligned(16)))` |
| §42 | 嵌套结构体定义 | test_nested_struct.sp | 解析失败 | `struct Outer { struct Inner {} inner; }` |
| §43 | restrict 修饰参数 | test_restrict_param.sp | 解析失败 | `char* restrict dest` |
| §44 | 函数指针变量 | test_func_pointer.sp | 解析失败 | `int (*fp)(int, int) = null;` |
| §45 | __extension__ 关键字 | test_extension.sp | 解析失败 | `__extension__ long long x;` |

---

## 实现路线图

### Phase 1: 结构体/联合体增强（P0）
- 实现 §33 匿名结构体/联合体
- 实现 §34 __declspec 修饰结构体字段
- 实现 §42 嵌套结构体定义
- 预计工作量：2 天

### Phase 2: 函数声明增强（P0）
- 实现 §35 复杂 typedef 链
- 实现 §36 函数指针参数
- 实现 §44 函数指针变量
- 预计工作量：2 天

### Phase 3: 表达式增强（P1）
- 实现 §37 指定初始化器
- 实现 §38 复合字面量
- 实现 §39 位域
- 预计工作量：2 天

### Phase 4: 其他特性（P1/P2）
- 实现 §40 线程局部存储
- 实现 §41 复杂 __attribute__
- 实现 §43 restrict 修饰参数
- 实现 §45 __extension__ 关键字
- 预计工作量：2 天

### Phase 5: 集成测试
- 创建 `test_stdio_h.sp` 验证 `#include <stdio.h>` 完整解析
- 创建 `test_stdlib_h.sp` 验证 `#include <stdlib.h>` 完整解析
- 回归测试：确保所有现有测试仍通过
- 预计工作量：1 天

---

---

## 1. SP 原生类型映射

### TDD-1.1：基本整数类型

```sp
// SP 输入
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```

**期望 C 输出：**
```c
#include <stdint.h>
int32_t add(int32_t a, int32_t b) {
    return a + b;
}
```

**验证：**
```bash
# 1. 编译期检查
gcc -std=c11 -Wall -c output.c

# 2. 类型尺寸断言（可嵌入输出 C 文件末尾验证）
_Static_assert(sizeof(int32_t) == 4, "i32 must be 4 bytes");
```

---

### TDD-1.2：无符号类型

```sp
// SP 输入
fn copy(dst: *u8, src: *u8, n: usize) -> usize {
    let i: usize = 0;
    return i;
}
```

**期望 C 输出：**
```c
#include <stdint.h>
#include <stddef.h>
size_t copy(uint8_t* dst, uint8_t* src, size_t n) {
    size_t i = 0;
    return i;
}
```

**验证：**
```bash
gcc -std=c11 -Wall -c output.c
_Static_assert(sizeof(size_t) == sizeof(void*), "usize must be pointer-sized");
```

---

### TDD-1.3：全类型映射一览

```sp
// SP 输入 — 类型映射冒烟测试
let a: i8   = 0;
let b: i16  = 0;
let c: i32  = 0;
let d: i64  = 0;
let e: u8   = 0;
let f: u16  = 0;
let g: u32  = 0;
let h: u64  = 0;
let i: usize = 0;
let j: isize = 0;
let k: char  = 'x';
let l: int   = 0;
```

**期望 C 输出：**
```c
#include <stdint.h>
#include <stddef.h>
int8_t   a = 0;
int16_t  b = 0;
int32_t  c = 0;
int64_t  d = 0;
uint8_t  e = 0;
uint16_t f = 0;
uint32_t g = 0;
uint64_t h = 0;
size_t   i = 0;
ptrdiff_t j = 0;
char     k = 'x';
int      l = 0;
```

**验证（逐字节尺寸断言）：**
```c
_Static_assert(sizeof(int8_t)   == 1, "i8");
_Static_assert(sizeof(int16_t)  == 2, "i16");
_Static_assert(sizeof(int32_t)  == 4, "i32");
_Static_assert(sizeof(int64_t)  == 8, "i64");
_Static_assert(sizeof(uint8_t)  == 1, "u8");
_Static_assert(sizeof(uint16_t) == 2, "u16");
_Static_assert(sizeof(uint32_t) == 4, "u32");
_Static_assert(sizeof(uint64_t) == 8, "u64");
```

---

### TDD-1.4：标准 C 类型原样透传

```sp
// SP 输入 — 标准 C 类型不应被修改
unsigned long x;
long long y;
double z;
void* p;
```

**期望 C 输出（原样）：**
```c
unsigned long x;
long long y;
double z;
void* p;
```

**验证：** 输出字符串与输入完全相同（忽略空白规范化）

---

## 2. 调用约定关键字

### TDD-2.1：`__cdecl`

```sp
// SP 输入
int __cdecl my_func(int a, int b);
```

**期望 C 输出（Windows x86-32）：**
```c
int __attribute__((__cdecl__)) my_func(int a, int b);
```

**期望 C 输出（Windows x86-64 / Linux，cdecl 无意义）：**
```c
int my_func(int a, int b);
```

**验证：**
```bash
# x86-32 Windows
gcc -m32 -std=c11 -Wall -c output.c

# x86-64：__cdecl 被消除，无 attribute
grep -v '__cdecl' output.c | gcc -std=c11 -Wall -c -xc -
```

---

### TDD-2.2：`__stdcall`

```sp
// SP 输入
int __stdcall WinApi(int x);
```

**期望 C 输出（32-bit）：**
```c
int __attribute__((__stdcall__)) WinApi(int x);
```

**期望 C 输出（64-bit，stdcall 无意义，忽略）：**
```c
int WinApi(int x);
```

**验证：**
```bash
gcc -std=c11 -c output.c
# 确认函数签名可链接
```

---

### TDD-2.3：`__fastcall`

```sp
// SP 输入
void __fastcall fast_fn(int a, int b);
```

**期望 C 输出：**
```c
void __attribute__((fastcall)) fast_fn(int a, int b);
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-2.4：`__unaligned`

```sp
// SP 输入
__unaligned int* ptr;
```

**期望 C 输出：**
```c
int* __attribute__((packed)) ptr;
/* 注意：packed 在变量上语义近似，完全等价需目标平台确认 */
```

**验证：**
```bash
gcc -std=gnu11 -c output.c
```

---

## 3. `__declspec` 扩展

### TDD-3.1：`__declspec(noreturn)`

```sp
// SP 输入
__declspec(noreturn) void die(int code);
```

**期望 C 输出：**
```c
__attribute__((noreturn)) void die(int code);
```

**验证：**
```c
// 可编译验证：调用后无需 return
__attribute__((noreturn)) void die(int code);
int foo(void) {
    die(1);
    // 不写 return，编译器不应警告 -Wreturn-type
}
```
```bash
gcc -std=c11
```

---

### TDD-3.2：`__declspec(dllimport)` 变量声明

```sp
// SP 输入
extern __declspec(dllimport) int external_value;
```

**期望 C 输出（透传，原样保留）：**
```c
extern __declspec(dllimport) int external_value;
```

**验证：**
```bash
# 通过 SP 编译器编译成功即视为通过
sharpc test_c_modifiers.sp -o test_c_modifiers.exe
```

---

## 4. MSVC 整数类型关键字

### TDD-4.1：`__int64`

```sp
// SP 输入
typedef __int64 my_int64;

i32 main() {
    my_int64 a = 0;
    return 0;
}
```

**期望 C 输出：**
```c
typedef __int64 my_int64;

int32_t main() {
    my_int64 a = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-4.2：`wchar_t`

```sp
// SP 输入
typedef wchar_t my_wchar;

i32 main() {
    my_wchar wc = 0;
    return 0;
}
```

**期望 C 输出：**
```c
typedef wchar_t my_wchar;

int32_t main() {
    my_wchar wc = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-4.3：`long long`

```sp
// SP 输入
typedef long long my_ll;

i32 main() {
    my_ll x = 0;
    return 0;
}
```

**期望 C 输出：**
```c
typedef long long my_ll;

int32_t main() {
    my_ll x = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-4.4：标准 C `int` 变量

```sp
// SP 输入
i32 main() {
    int x = 0;
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    int x = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 5. `const` 支持

### TDD-Const-1：const 局部变量

```sp
// SP 输入
extern i32 puts(const u8* s);

i32 main() {
    const u8* msg = "hello const";
    puts(msg);
    return 0;
}
```

**期望 C 输出：**
```c
extern int32_t puts(const uint8_t* s);

int32_t main() {
    const uint8_t* msg = "hello const";
    puts(msg);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Const-2：const 函数参数

```sp
// SP 输入
extern i32 puts(const u8* s);

void print_msg(const u8* msg) {
    puts(msg);
}

i32 main() {
    const u8* hello = "hello";
    print_msg(hello);
    return 0;
}
```

**期望 C 输出：**
```c
extern int32_t puts(const uint8_t* s);

void print_msg(const uint8_t* msg) {
    puts(msg);
}

int32_t main() {
    const uint8_t* hello = "hello";
    print_msg(hello);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Const-3：const 指针变体

```sp
// SP 输入
extern i32 puts(const u8* s);

i32 main() {
    const u8* msg1 = "hello const";
    puts(msg1);
    return 0;
}
```

**期望 C 输出：**
```c
extern int32_t puts(const uint8_t* s);

int32_t main() {
    const uint8_t* msg1 = "hello const";
    puts(msg1);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 6. `__inline__` / `extern __inline__` 支持

### TDD-Inline-1：`extern __inline__` + C `int` 函数

```sp
// SP 输入
int add(int a, int b) {
    return a + b;
}
i32 main() { return add(1, 2); }
```

**期望 C 输出：**
```c
int add(int a, int b) {
    return a + b;
}
int32_t main() { return add(1, 2); }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Inline-2：`extern inline` with body

```sp
// SP 输入
extern int add(int a, int b) {
    return a + b;
}
i32 main() { return add(1, 2); }
```

**期望 C 输出：**
```c
extern int add(int a, int b) {
    return a + b;
}
int32_t main() { return add(1, 2); }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Inline-3：`extern __inline__` 函数

```sp
// SP 输入
extern __inline__ int add(int a, int b) {
    return a + b;
}
i32 main() { return 0; }
```

**期望 C 输出：**
```c
extern __inline__ int add(int a, int b) {
    return a + b;
}
int32_t main() { return 0; }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Inline-4：`__inline__` 无 extern

```sp
// SP 输入
__inline__ int add(int a, int b) { return a + b; }
i32 main() { return 0; }
```

**期望 C 输出：**
```c
__inline__ int add(int a, int b) { return a + b; }
int32_t main() { return 0; }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Inline-5：`extern __inline__`

```sp
// SP 输入
extern __inline__ int add(int a, int b) {
    return a + b;
}
i32 main() { return 0; }
```

**期望 C 输出：**
```c
extern __inline__ int add(int a, int b) {
    return a + b;
}
int32_t main() { return 0; }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Inline-6：`__inline__` 局部变量（修饰符跳过）

```sp
// SP 输入
i32 main() {
    __inline__ int x = 0;
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    __inline__ int x = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 7. `__attribute__` 支持

### TDD-Attribute-1：函数声明后 `__attribute__((format(...)))`

```sp
// SP 输入
extern void log_msg(const char* msg) __attribute__((format(printf, 1, 2)));
```

**期望 C 输出（透传）：**
```c
extern void log_msg(const char* msg) __attribute__((format(printf, 1, 2)));
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 8. struct / union 支持

### TDD-Struct-1：union 声明与使用

```sp
// SP 输入
union Value {
    int i;
    double d;
    char* s;
};

i32 main() {
    union Value v;
    return 0;
}
```

**期望 C 输出：**
```c
union Value {
    int i;
    double d;
    char* s;
};

int32_t main() {
    union Value v;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Struct-2：struct 前向声明

```sp
// SP 输入
struct Node;

struct Node {
    int data;
    struct Node* next;
};

i32 main() {
    struct Node n;
    return 0;
}
```

**期望 C 输出：**
```c
struct Node;

struct Node {
    int data;
    struct Node* next;
};

int32_t main() {
    struct Node n;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

### TDD-Struct-3：匿名 struct typedef

```sp
// SP 输入
typedef struct {
    int x;
    int y;
} Point;

i32 main() {
    Point p;
    return 0;
}
```

**期望 C 输出：**
```c
typedef struct __anon_struct_0 {
    int x;
    int y;
} Point;

int32_t main() {
    Point p;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 9. `extern` 函数声明

### TDD-Extern-1：无 body 的 extern 函数声明

```sp
// SP 输入
extern int add(int a, int b);
i32 main() { return 0; }
```

**期望 C 输出：**
```c
extern int add(int a, int b);
int32_t main() { return 0; }
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 10. `typedef` 扩展

### TDD-Typedef-2：`typedef int errno_t`

```sp
// SP 输入
typedef int errno_t;

extern errno_t clearerr_s(void* stream);

i32 main() {
    errno_t err = 0;
    return 0;
}
```

**期望 C 输出：**
```c
typedef int errno_t;

extern errno_t clearerr_s(void* stream);

int32_t main() {
    errno_t err = 0;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 11. `va_list` 支持

### TDD-VA-1：`va_list` typedef + `extern __inline__`

```sp
// SP 输入
typedef void* va_list;

extern int vprintf(const char* format, va_list args);

extern __inline__ int add(int a, int b) {
    return a + b;
}

i32 main() {
    va_list args;
    i32 x = add(1, 2);
    return 0;
}
```

**期望 C 输出：**
```c
typedef void* va_list;

extern int vprintf(const char* format, va_list args);

extern __inline__ int add(int a, int b) {
    return a + b;
}

int32_t main() {
    va_list args;
    int32_t x = add(1, 2);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 19. 强转指针哨兵值

### §19.1：`(void*)(-1)` 哨兵值

```sp
// SP 输入 — C 风格的类型强转，常用于指针哨兵值
i32 main() {
    void* p = (void*)(-1);
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    void* p = (void*)(-1);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -Wall -c output.c
```

---

### §19.2：`(i32*)(0xFFFFFFFF)` 无符号常量强转

```sp
// SP 输入 — 强转整数常量到指针类型
i32 main() {
    i32* ip = (i32*)(0xFFFFFFFF);
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t* ip = (int32_t*)4294967295;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -Wall -c output.c
```

---

### §19.3：完整测试用例

```sp
/* §19 强转指针哨兵值 */
i32 main() {
    void* p = (void*)(-1);
    i32* ip = (i32*)(0xFFFFFFFF);
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    void* p = (void*)(-1);
    int32_t* ip = (int32_t*)4294967295;
    return 0;
}
```

**验证：**
```bash
sharpc test_cast_sentinel.sp -o test_cast_sentinel.exe
test_cast_sentinel.exe && echo exit=0
```

---

## 12. print / println built-in

### TDD-Stmt-1：print / println

```sp
// SP 输入
i32 main() {
    print(42);
    println("hello");
    return 0;
}
```

**期望 C 输出（简化，实际依赖 stdio.h）：**
```c
int32_t main() {
    printf("%d", 42);
    printf("%s\n", "hello");
    return 0;
}
```

**验证：**
```bash
sharpc test_print.sp -o test_print.exe
# 编译成功即视为通过
```

---

## 13. while 循环

### TDD-Stmt-2：while loop

```sp
// SP 输入
i32 main() {
    i32 sum = 0;
    i32 i = 0;
    while (i < 5) {
        sum = sum + i;
        i = i + 1;
    }
    return sum; /* 0+1+2+3+4 = 10 */
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t sum = 0;
    int32_t i = 0;
    while (i < 5) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
```

**验证：**
```bash
sharpc test_while.sp -o test_while.exe
test_while.exe && echo exit=10
```

---

## 14. if / else

### TDD-Stmt-3：if / else

```sp
// SP 输入
i32 main() {
    i32 x = 5;
    if (x > 3) {
        return 1;
    } else {
        return 0;
    }
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t x = 5;
    if (x > 3) {
        return 1;
    } else {
        return 0;
    }
}
```

**验证：**
```bash
sharpc test_if.sp -o test_if.exe
test_if.exe && echo exit=1
```

---

## 15. for 循环

### TDD-Stmt-4：for loop

```sp
// SP 输入
i32 main() {
    i32 sum = 0;
    for (i32 i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }
    return sum; /* 0+1+2+3+4 = 10 */
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t sum = 0;
    for (int32_t i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}
```

**验证：**
```bash
sharpc test_for.sp -o test_for.exe
test_for.exe && echo exit=10
```

---

## 16. struct + impl + 方法调用

### TDD-Struct-4：struct + impl + method call

```sp
// SP 输入
struct Vec2 {
    i32 x;
    i32 y;
};

impl Vec2 {
    i32 sum() {
        return self.x + self.y;
    }
}

i32 main() {
    Vec2 v;
    v.x = 3;
    v.y = 4;
    return v.sum(); /* 7 */
}
```

**期望 C 输出（大致，方法名会 mangle）：**
```c
typedef struct Vec2 {
    int32_t x;
    int32_t y;
} Vec2;

int32_t Vec2_sum(Vec2* self) {
    return self->x + self->y;
}

int32_t main() {
    Vec2 v;
    v.x = 3;
    v.y = 4;
    return Vec2_sum(&v);
}
```

**验证：**
```bash
sharpc test_struct_impl.sp -o test_struct_impl.exe
test_struct_impl.exe && echo exit=7
```

---

## 17. 指针、取地址、解引用

### TDD-Expr-1：pointer, address-of, dereference

```sp
// SP 输入
i32 main() {
    i32 x = 42;
    i32* p = &x;
    return *p; /* 42 */
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t x = 42;
    int32_t* p = &x;
    return *p;
}
```

**验证：**
```bash
sharpc test_ptr.sp -o test_ptr.exe
test_ptr.exe && echo exit=42
```

---

## 18. break / continue

### TDD-Stmt-5：break / continue

```sp
// SP 输入
i32 main() {
    i32 sum = 0;
    i32 i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 3) continue;
        if (i == 6) break;
        sum = sum + i;
    }
    return sum; /* 1+2+4+5 = 12 */
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t sum = 0;
    int32_t i = 0;
    while (i < 10) {
        i = i + 1;
        if (i == 3) continue;
        if (i == 6) break;
        sum = sum + i;
    }
    return sum;
}
```

**验证：**
```bash
sharpc test_break_continue.sp -o test_break_continue.exe
test_break_continue.exe && echo exit=12
```

---

## 19. 算术与比较运算

### TDD-Expr-2：arithmetic and comparison

```sp
// SP 输入
i32 main() {
    i32 a = 7;
    i32 b = 3;
    i32 c = a + b;   /* 10 */
    i32 d = a - b;   /* 4 */
    i32 e = a * b;   /* 21 */
    i32 f = a / b;   /* 2 */
    i32 g = a % b;   /* 1 */
    if (c == 10 && d == 4 && e == 21 && f == 2 && g == 1) {
        return 1;
    }
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t a = 7;
    int32_t b = 3;
    int32_t c = a + b;
    int32_t d = a - b;
    int32_t e = a * b;
    int32_t f = a / b;
    int32_t g = a % b;
    if (c == 10 && d == 4 && e == 21 && f == 2 && g == 1) {
        return 1;
    }
    return 0;
}
```

**验证：**
```bash
sharpc test_arith.sp -o test_arith.exe
test_arith.exe && echo exit=1
```

---

## 20. bool 字面量与逻辑运算

### TDD-Expr-4：bool literals and logic operators

```sp
// SP 输入
i32 main() {
    bool t = true;
    bool f = false;
    if (t && !f) {
        return 1;
    }
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    _Bool t = 1;
    _Bool f = 0;
    if (t && !f) {
        return 1;
    }
    return 0;
}
```

**验证：**
```bash
sharpc test_bool.sp -o test_bool.exe
test_bool.exe && echo exit=1
```

---

## 21. Array declaration and indexing

### TDD-Expr-3：array declaration and indexing

```sp
// SP 输入
i32 main() {
    i32 arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    return arr[0] + arr[3]; /* 5 */
}
```

**期望 C 输出：**
```c
int32_t main() {
    int32_t arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    return arr[0] + arr[3];
}
```

**验证：**
```bash
sharpc test_array.sp -o test_array.exe
test_array.exe && echo exit=5
```

---

## 22. generic struct + impl

### TDD-Generic-1：generic struct with method

```sp
// SP 输入
struct Option<T> {
    value: T;
    is_some: bool;
};

impl Option<i32> {
    fn unwrap(self) -> i32 {
        return self.value;
    }
}

i32 main() {
    Option<i32> opt;
    opt.value = 42;
    opt.is_some = true;
    return opt.unwrap();
}
```

**期望 C 输出（大致，泛型会被 monomorphize）：**
```c
typedef struct Option_i32 {
    int32_t value;
    _Bool is_some;
} Option_i32;

int32_t Option_i32_unwrap(Option_i32 self) {
    return self.value;
}

int32_t main() {
    Option_i32 opt;
    opt.value = 42;
    opt.is_some = 1;
    return Option_i32_unwrap(opt);
}
```

**验证：**
```bash
sharpc test_generic.sp -o test_generic.exe
test_generic.exe && echo exit=42
```

---

## 23. MSVC 整数字面量后缀

### TDD-MSVC-Suffix-1：`i8`/`i16`/`i32`/`i64` 后缀

```sp
// SP 输入
i32 main() {
    i64 a = 100i64;
    i32 b = 50i32;
    i16 c = 25i16;
    i8  d = 10i8;
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    int64_t a = 100LL;
    int32_t b = 50;
    int16_t c = 25;
    int8_t  d = 10;
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```

---

## 24. `_Bool` 类型

### §11：`_Bool` 类型映射

```sp
// SP 输入
i32 main() {
    bool b = true;
    if (b) {
        return 1;
    }
    return 0;
}
```

**期望 C 输出：**
```c
int32_t main() {
    _Bool b = 1;
    if (b) {
        return 1;
    }
    return 0;
}
```

**验证：**
```bash
sharpc test_bool_c.sp -o test_bool_c.exe
test_bool_c.exe && echo exit=1
```

---

## 25. `NAN`/`INFINITY` 浮点常量定义

### §18：浮点常量

```sp
// SP 输入
extern double NAN;
extern double INFINITY;

i32 main() {
    double x = NAN;
    double y = INFINITY;
    return 0;
}
```

**期望 C 输出：**
```c
extern double NAN;
extern double INFINITY;

int32_t main() {
    double x = NAN;
    double y = INFINITY;
    return 0;
}
```

**验证：**
```bash
sharpc test_nan_infinity.sp -o test_nan_infinity.exe
```

---

## 26. `#pragma GCC system_header`

### §20：系统头标记

```sp
// SP 输入
#pragma GCC system_header

i32 main() {
    return 0;
}
```

**期望 C 输出（透传 #pragma）：**
```c
#pragma GCC system_header

int32_t main() {
    return 0;
}
```

**验证：**
```bash
sharpc test_pragma_system_header.sp -o test_pragma_system_header.exe
```

---

## 27. 编译期静态断言老式写法

### §21：`_Static_assert`

```sp
// SP 输入
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

i32 main() {
    return 0;
}
```

**期望 C 输出（透传）：**
```c
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");

int32_t main() {
    return 0;
}
```

**验证：**
```bash
sharpc test_static_assert_old.sp -o test_static_assert_old.exe
```

---

## 28. `__STRICT_ANSI__` 条件编译

### §28：条件编译

```sp
// SP 输入
#ifndef __STRICT_ANSI__
extern int _setmode(int fd, int mode);
#endif

i32 main() {
    return 0;
}
```

**期望 C 输出（透传预处理器指令）：**
```c
#ifndef __STRICT_ANSI__
extern int _setmode(int fd, int mode);
#endif

int32_t main() {
    return 0;
}
```

**验证：**
```bash
sharpc test_strict_ansi.sp -o test_strict_ansi.exe
```

---

## 29. `va_list` 多架构实现（高级）

### §16：`va_list` 在不同架构下的实现

```sp
// SP 输入
typedef void* va_list;

extern int vprintf(const char* format, va_list args);

extern __inline__ int add(int a, int b) {
    return a + b;
}

i32 main() {
    va_list args;
    i32 x = add(1, 2);
    return 0;
}
```

**期望 C 输出：**
```c
typedef void* va_list;

extern int vprintf(const char* format, va_list args);

extern __inline__ int add(int a, int b) {
    return a + b;
}

int32_t main() {
    va_list args;
    int32_t x = add(1, 2);
    return 0;
}
```

**验证：**
```bash
gcc -std=c11 -c output.c
```
