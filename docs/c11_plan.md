# C11 Full Support Implementation Plan

## Goal
实现完整的C11标准语法支持,使sharpc能够编译所有符合C11标准的代码,并支持`-std=c11`命令行选项。

## Current Status
- **C89/C90**: 100% ✅
- **C99**: ~85% ✅ (缺少 `_Generic`)
- **C11**: ~60% ⚠️ (多项缺失)
- **C23**: 仅 `__VA_OPT__` ⚠️

---

## Phase 1: 命令行与预处理器增强

### 1.1 添加 `-std=c11` 等标准版本选项
**Files**: `src/main.c`, `src/preproc/cpp.h`, `src/preproc/cpp.c`
- 解析 `-std=c89`, `-std=c99`, `-std=c11`, `-std=c17`, `-std=gnu11` 等选项
- 根据标准版本设置 `__STDC_VERSION__` 宏:
  - c89/c90: `199409L`
  - c99: `199901L`
  - c11: `201112L`
  - c17/c18: `201710L`
- 根据标准版本启用/禁用某些扩展特性
- 当指定 `-std=c*` 时,定义 `__STRICT_ANSI__`
- 当指定 `-std=gnu*` 时,不定义 `__STRICT_ANSI__`

**TDD Test**:
- `test_std_c89.sp` - 验证 `-std=c89` 设置正确的宏
- `test_std_c99.sp` - 验证 `-std=c99` 设置正确的宏
- `test_std_c11.sp` - 验证 `-std=c11` 设置正确的宏
- `test_std_gnu11.sp` - 验证 `-std=gnu11` 不设置 `__STRICT_ANSI__`

### 1.2 预处理器内置宏完善
**Files**: `src/preproc/cpp.c`
- `__STDC__` → `"1"` (已有)
- `__STDC_HOSTED__` → `"1"` (已有)
- `__STDC_VERSION__` → 根据 `-std` 动态设置 (新增)
- `__STDC_ISO_10646__` → 根据平台设置 (新增)
- `__STDC_MB_MIGHT_NEQ_WC__` → `"1"` (新增)
- `__STDC_NO_ATOMICS__` → `"1"` (暂未实现_Atomic)
- `__STDC_NO_COMPLEX__` → `"1"` (暂未实现复数)
- `__STDC_NO_THREADS__` → `"1"` (暂未实现threads.h)
- `__STDC_NO_VLA__` → `"1"` (暂未实现VLA)

---

## Phase 2: C11 核心语言特性

### 2.1 `_Noreturn` 函数修饰符
**Files**: `src/lexer.c`, `src/parser.c`, `src/cgen.c`
- 在 lexer 中添加 `_Noreturn` 关键字为 `TK_NORETURN`
- 解析器识别 `_Noreturn` 修饰函数声明
- 代码生成输出 `__attribute__((noreturn))` 或保留 `_Noreturn`

**Lexer Changes**:
```c
{"_Noreturn", 9, TK_NORETURN},
```

**Parser Changes**:
- `parse_func_decl()` 识别 `_Noreturn` 前缀
- 在 `is_type_start()` 中添加 `TK_NORETURN`

**CodeGen Changes**:
- 函数声明输出时,如果有noreturn标记,输出 `__attribute__((noreturn))`

**TDD Test**: `test_noreturn.sp`
```c
_Noreturn void exit(int code);
_Noreturn void abort(void);
int main() { return 0; }
```

### 2.2 `_Alignas` / `_Alignof` 操作符
**Files**: `src/lexer.c`, `src/parser.c`, `src/types.c`, `src/cgen.c`
- 在 lexer 中添加 `_Alignas` 和 `_Alignof` 关键字
- `_Alignof(Type)` 返回类型的对齐要求(编译期常量)
- `_Alignas(N)` 作为结构体字段/变量的对齐修饰符
- 代码生成中:
  - `_Alignof` 替换为 `__alignof__` 或 `_Alignof`
  - `_Alignas` 替换为 `__attribute__((aligned(N)))`

**Lexer Changes**:
```c
{"_Alignas", 8, TK_ALIGNAS},
{"_Alignof", 8, TK_ALIGNOF},
```

**TDD Test**: `test_alignof.sp`
```c
int main() {
    int a = _Alignof(int);
    int b = _Alignof(double);
    return 0;
}
```

**TDD Test**: `test_alignas.sp`
```c
struct Aligned {
    char c;
    int x _Alignas(16);
};
int main() { return 0; }
```

### 2.3 `_Atomic` 类型限定符 (基础支持)
**Files**: `src/lexer.c`, `src/parser.c`, `src/types.c`, `src/cgen.c`
- 在 lexer 中添加 `_Atomic` 关键字
- 类型系统中增加 `TY_ATOMIC` 类型
- 代码生成输出 `_Atomic` 或 `__atomic` 前缀
- 注意: 完整的原子操作需要C11 stdatomic.h支持,这里只做语法解析

**Lexer Changes**:
```c
{"_Atomic", 7, TK_ATOMIC},
```

**TDD Test**: `test_atomic_basic.sp`
```c
_Atomic int counter = 0;
int main() {
    _Atomic int x = 1;
    return 0;
}
```

### 2.4 匿名结构体/联合体完善 (C11 §6.7.2.1)
**Files**: `src/parser.c`
- 当前已有部分支持,需要完善:
  - 结构体内部匿名 struct/union 的直接字段访问
  - typedef 中的匿名结构体

**TDD Test**: `test_anonymous_struct.sp` (已有,需修复)
```c
struct Outer {
    int x;
    struct {
        int a;
        int b;
    };
};
int main() {
    struct Outer o;
    o.a = 1;  // 直接访问匿名结构体字段
    return 0;
}
```

### 2.5 通用选择 `_Generic` (C11 §6.5.1.1)
**Files**: `src/lexer.c`, `src/parser.c`, `src/sema.c`, `src/cgen.c`
- 这是C11最难实现特性之一
- 语法: `_Generic(control-expr, type-name: expr, default: expr)`
- 在编译期根据控制表达式的类型选择对应的表达式
- 需要类型系统在编译期解析

**Lexer Changes**:
```c
{"_Generic", 8, TK_GENERIC},
```

**Parser Changes**:
- 在 `parse_primary()` 中添加 `_Generic` 解析
- 构建泛型选择AST节点 `ND_GENERIC`

**Sema Changes**:
- 在类型检查时解析 `_Generic`,根据控制表达式类型选择分支
- 移除未选中的分支

**CodeGen Changes**:
- 只输出被选中的表达式

**TDD Test**: `test_generic_selection.sp`
```c
#define type_name(X) _Generic((X), \
    int: "int", \
    double: "double", \
    default: "unknown")

int main() {
    int i = 0;
    double d = 0.0;
    const char* s1 = type_name(i);
    const char* s2 = type_name(d);
    return 0;
}
```

---

## Phase 3: 预处理器C11特性

### 3.1 `__VA_OPT__` 支持 (C23, 但GCC扩展已支持)
**Files**: `src/preproc/macro.c`
- 当前已有部分支持,需要完善
- 语法: `__VA_OPT__(content)` - 当 `__VA_ARGS__` 非空时展开content

**TDD Test**: `test_va_opt.sp`
```c
#define DEBUG_LOG(msg, ...) printf("DEBUG: " msg __VA_OPT__(, ) __VA_ARGS__)
int main() {
    DEBUG_LOG("hello");
    DEBUG_LOG("value=%d", 42);
    return 0;
}
```

### 3.2 `#pragma _Pragma` 操作符 (C11 §6.10.9)
**Files**: `src/preproc/cpp.c`
- `_Pragma("string-literal")` 作为 `#pragma` 的替代形式
- 可以在宏中使用

**TDD Test**: `test_pragma_operator.sp`
```c
#define PRAGMA(x) _Pragma(#x)
PRAGMA(message("Hello from pragma"))
int main() { return 0; }
```

### 3.3 宽字符串字面量 (C11 u"" / U"" / u8"")
**Files**: `src/lexer.c`, `src/parser.c`, `src/cgen.c`
- `u"string"` - char16_t 字符串
- `U"string"` - char32_t 字符串
- `u8"string"` - UTF-8 字符串
- `L"string"` - 宽字符串 (已有部分支持)

**Lexer Changes**:
- 识别 `u"..."`, `U"..."`, `u8"..."`, `L"..."` 前缀
- 增加 token kind: `TK_WIDE_STRING`, `TK_UTF16_STRING`, `TK_UTF32_STRING`, `TK_UTF8_STRING`

**TDD Test**: `test_wide_strings.sp`
```c
int main() {
    const char* s1 = u8"UTF-8 string";
    // L"wide string"  // depends on platform support
    return 0;
}
```

---

## Phase 4: 完善缺失的C99/C11特性

### 4.1 复合字面量完善 (C99 §6.5.2.5)
**Files**: `src/parser.c`, `src/cgen.c`
- 当前有基础支持,需要增强:
  - 支持数组复合字面量: `(int[]){1, 2, 3}`
  - 支持作为函数参数: `func((struct Point){.x=1, .y=2})`
  - 支持常量复合字面量

**TDD Test**: `test_compound_literal.sp` (已有,需修复)
```c
struct Point { int x; int y; };
int main() {
    struct Point* p = &(struct Point){ .x = 1, .y = 2 };
    int* arr = (int[]){1, 2, 3, 4};
    return 0;
}
```

### 4.2 指定初始化器完善 (C99 §6.7.8)
**Files**: `src/parser.c`, `src/cgen.c`
- 当前有基础支持,需要增强:
  - 数组指定初始化器: `[0] = 1, [2] = 3`
  - 混合使用: `{.x = 1, .y = 2}`
  - 嵌套指定初始化器

**TDD Test**: `test_designated_init.sp` (已有,需修复)
```c
struct Point { int x; int y; };
int main() {
    struct Point p = { .y = 2, .x = 1 };
    int arr[5] = {[0] = 1, [2] = 3, [4] = 5};
    return 0;
}
```

### 4.3 位域完善 (C11 §6.7.2.1)
**Files**: `src/parser.c`, `src/cgen.c`
- 当前有基础支持,需要增强:
  - 无名字段: `int : 3;` (填充)
  - 零宽度字段: `int : 0;` (强制对齐)
  - 位域地址不可取(语义检查)

**TDD Test**: `test_bitfield.sp` (已有,需修复)
```c
struct Flags {
    unsigned int a : 1;
    unsigned int b : 2;
    unsigned int   : 3;  // padding
    unsigned int c : 4;
    unsigned int : 0;    // force alignment
};
int main() { return 0; }
```

### 4.4 柔性数组成员 (C99 §6.7.2.1)
**Files**: `src/parser.c`, `src/cgen.c`
- 支持 `[]` 和 `[1]` 两种写法
- 必须是结构体最后一个字段

**TDD Test**: `test_flexible_array.sp`
```c
struct Buffer {
    int length;
    char data[];  // or char data[1];
};
int main() { return 0; }
```

---

## Phase 5: 其他C标准特性

### 5.1 `static_assert` (C11 §7.2)
**Files**: `src/preproc/cpp.c`, `src/lexer.c`
- 通过 `<assert.h>` 提供 `static_assert` 宏(映射到 `_Static_assert`)
- 当前已有 `_Static_assert` 支持,需要添加 `static_assert` 宏定义

**Preprocessor Changes**:
```c
cpp_define(cpp, "static_assert", "_Static_assert");
```

### 5.2 `typeof` / `__typeof__` (GCC扩展, C23标准化)
**Files**: `src/lexer.c`, `src/parser.c`, `src/sema.c`, `src/cgen.c`
- `typeof(expr)` 返回表达式的类型
- 用于声明相同类型的变量
- 代码生成输出 `__typeof__` 或保留 `typeof`

**Lexer Changes**:
```c
{"typeof", 6, TK_TYPEOF},
{"__typeof__", 10, TK_TYPEOF},
```

**TDD Test**: `test_typeof.sp`
```c
int main() {
    int x = 42;
    typeof(x) y = x;
    return 0;
}
```

### 5.3 变长数组 VLA (C99, C11可选)
**Files**: `src/parser.c`, `src/cgen.c`
- 支持运行时大小数组声明
- 函数参数中 `void func(int n, int arr[n])`
- **注意**: 这是可选特性,可以先标记为不支持

**TDD Test**: `test_vla.sp` (标记为跳过,因为TCC可能不支持)
```c
int main() {
    int n = 10;
    int arr[n];  // VLA
    return 0;
}
```

### 5.4 复数类型 `_Complex` (C99, C11可选)
**Files**: `src/lexer.c`, `src/parser.c`
- `_Complex float`, `_Complex double`
- **注意**: 这是可选特性,TCC可能不支持

---

## Phase 6: 测试与集成

### 6.1 创建完整的C11测试套件
为每个新增特性创建独立的测试文件:
- `test_noreturn.sp`
- `test_alignof.sp`
- `test_alignas.sp`
- `test_atomic_basic.sp`
- `test_generic_selection.sp`
- `test_va_opt.sp`
- `test_pragma_operator.sp`
- `test_wide_strings.sp`
- `test_typeof.sp`

### 6.2 更新 TDD 文档
- 更新 `sp_compiler_tdd.md` 中所有测试状态
- 添加新特性的完整测试用例
- 标记每个特性的C标准版本

### 6.3 运行回归测试
- 确保所有现有测试仍然通过
- 验证 `-std=c11` 正确设置宏

### 6.4 标准库头文件测试
- 验证更多标准库头文件可以正确解析
- 特别是 `<stdalign.h>`, `<stdatomic.h>`, `<stdnoreturn.h>`

---

## Implementation Order (Recommended)

1. **Week 1**: Phase 1 (命令行选项) + Phase 2.1 (_Noreturn)
2. **Week 2**: Phase 2.2 (_Alignas/_Alignof) + Phase 2.4 (匿名结构体)
3. **Week 3**: Phase 4.1 (复合字面量) + Phase 4.2 (指定初始化器) + Phase 4.3 (位域)
4. **Week 4**: Phase 3 (预处理器特性) + Phase 2.3 (_Atomic)
5. **Week 5**: Phase 2.5 (_Generic) - 最复杂特性
6. **Week 6**: Phase 5 (其他特性) + Phase 6 (测试集成)

---

## Key Technical Decisions

1. **`-std=c11` 行为**:
   - 仅影响预处理器宏定义
   - 不影响解析器行为(解析器始终接受所有扩展)
   - 当 `-pedantic` 时,对非标准特性发出警告

2. **`_Generic` 实现策略**:
   - 在 sema 阶段解析,不在 parser 阶段
   - 根据控制表达式类型,在编译时选择分支
   - 未选中的分支不进入 AST

3. **`_Atomic` 实现策略**:
   - 只做语法解析,不做语义检查
   - 代码生成时透传给C编译器
   - 完整的原子操作依赖目标C编译器

4. **VLA 处理**:
   - 标记为不支持(因为TCC限制)
   - 在 `-std=c11` 时定义 `__STDC_NO_VLA__`

5. **复数类型处理**:
   - 标记为不支持
   - 在 `-std=c11` 时定义 `__STDC_NO_COMPLEX__`
