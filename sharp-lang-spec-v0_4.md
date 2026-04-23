# Sharp 语言规格 v0.4

> C++ Lite · 完整c11 + struct + impl + 泛型 + RAII · 转译到 C11 · 零运行时

---

每个特性必须通过两个验证：
1. C 无法干净表达
2. 转译到 C 后零开销

**Sharp 是什么：** C + Rust 风格的 `impl` + C++ 风格的模板 + 确定性析构。

---

## 语义契约

### 值语义（和 C 完全一致）

结构体赋值、值传参、函数返回，全部是 **memcpy**，和 C 的 struct 赋值完全一致：

```csharp
Buffer a = Buffer.alloc(1024);
Buffer b = a;   // memcpy：b.ptr == a.ptr，两者共享同一块内存
```

如果一个类型独占某个资源，你有责任不对它做值拷贝。用 `T*` 传指针，或者显式定义转移语义（置空原对象）。**Sharp 不追踪所有权。**

### RAII 契约

Sharp 保证：

- 每个持有析构函数的**局部变量**（包括值形参）在离开其作用域时，析构函数被调用恰好一次
- 析构函数以声明的**逆序**触发
- `return` / `break` / `continue` 会沿路径展开所有在作用域内的析构函数

Sharp 不保证：

- 析构函数只被调用一次（值拷贝后两份都会析构）
- 资源不被双重释放（这是使用者的责任）

---

## 语言核心

| 特性 | 理由 |
|------|------|
| `struct` + `impl` | C 有 struct 但无方法绑定 |
| 析构函数 `~T()` + RAII | 确定性资源管理（见契约） |
| 运算符重载 | 数学/向量代码必要 |
| 泛型（monomorphize） | C 只能靠宏模拟 |
---

## 预处理器 (Preprocessor)

Sharp 内置一个完整的 C11 预处理器（`cpp/` 模块），在词法分析之前运行。
它与 C 预处理器完全兼容，并增加了若干 Sharp 专属扩展。

### 翻译阶段

按 ISO C11 §5.1.1.2 执行以下六个阶段：

| 阶段 | 操作 |
|------|------|
| 1 | 三字符序列替换（trigraph，`??=` → `#` 等） |
| 2 | 反斜杠-换行拼接（`\` + `\n` 删除） |
| 3 | 分词为预处理令牌（pp-token） |
| 4 | 指令执行 + 宏展开 |
| 5 | 字符/字符串编码转换 |
| 6 | 相邻字符串字面量拼接 |

### 支持的指令

```
#include "file"          用户头文件（相对当前文件目录搜索）
#include <file>          系统头文件（系统路径搜索）
#include_next "file"     从当前文件所在目录的下一个匹配项开始搜索

#define NAME             定义空宏
#define NAME value       对象式宏
#define NAME(p1,p2) body 函数式宏
#define NAME(p,...) body 可变参数宏（__VA_ARGS__ / __VA_OPT__）
#undef  NAME             取消定义

#if   expr               常量表达式条件（支持 defined()、算术、位运算、三目）
#ifdef  NAME             等价于 #if defined(NAME)
#ifndef NAME             等价于 #if !defined(NAME)
#elif expr               else-if 分支
#else                    否则分支
#endif                   结束条件块

#line N                  设置当前行号为 N
#line N "file"           同时设置文件名

#error  message          触发编译错误并中止
#warning message         触发警告（继续编译）

#pragma once             保证当前头文件只被包含一次
#pragma <other>          其他 pragma 透传到输出；可注册自定义回调处理
```

### 宏展开规则

- **对象式宏**：展开时递归替换，但对已展开的宏名施加蓝色标记（blue-painting），防止无限递归
- **函数式宏**：参数先展开，再代入替换列表；`#` 操作符将参数字符串化；`##` 操作符拼接相邻令牌
- **可变参数**：`__VA_ARGS__` 接收 `...` 实参；`__VA_OPT__(tokens)` 仅在 `__VA_ARGS__` 非空时展开
- **`defined(X)`**：在宏展开之前求值，避免参数被意外展开

### 内置宏

| 宏 | 值 | 说明 |
|----|----|------|
| `__FILE__` | `"path/to/file.sp"` | 当前源文件路径 |
| `__LINE__` | 整数 | 当前源文件行号 |
| `__COUNTER__` | 0, 1, 2, … | 每次展开自增，全局唯一序列 |
| `__DATE__` | `"Jan  1 2025"` | 编译日期 |
| `__TIME__` | `"12:00:00"` | 编译时间 |
| `__STDC__` | `1` | 符合 C11 标准 |
| `__STDC_VERSION__` | `201112L` | C11 版本号 |
| `__STDC_HOSTED__` | `1` | 托管实现 |
| `__SHARP__` | `1` | Sharp 语言模式（仅 `.sp` 文件） |
| `__SHARP_VERSION__` | `"0.4"` | Sharp 版本字符串 |

### `#if` 常量表达式

支持完整的 C11 整型常量表达式：

```
算术：  + - * / %
位运算：<< >> & | ^ ~
逻辑：  && || !
关系：  == != < > <= >=
三目：  cond ? then : else
字符常量：'a'  '\n'  '\xff'
整数字面量：十进制 / 0x十六进制 / 0八进制 / 0b二进制
defined(X)：X 已定义则为 1，否则为 0
```

所有运算以 `intmax_t`（至少 64 位）执行；除以零报错而非未定义行为。

### 与 C 的差异和扩展

| 特性 | Sharp 行为 |
|------|------------|
| `#pragma once` | 原生支持，优于手写 include guard |
| `__VA_OPT__` | 支持（C23 特性，Sharp 提前引入） |
| `#include_next` | 支持（GCC 扩展） |
| `__COUNTER__` | 支持（GCC 扩展） |
| `#warning` | 支持（GCC 扩展） |
| 行标记（linemarker） | 预处理器输出 `# N "file"` 行，词法分析器吸收并更新位置信息，错误信息反映原始源码行号 |
| 三字符序列 | 支持但默认不警告（纯 C 兼容需要） |

### 命令行选项

```
sharpc  -DNAME[=value]  # 预定义宏（等价于 #define）
sharpc  -UNAME          # 预取消定义（等价于开头 #undef）
sharpc  -Idir           # 添加用户 include 搜索路径
sharp-cpp               # 独立预处理器可执行文件（与 sharpc 共享同一库）
sharp-cpp -P            # 抑制行标记输出
sharp-cpp --sharp       # 强制 Sharp 语言模式（定义 __SHARP__）
sharp-cpp --tokens      # 将令牌列表输出到 stderr（调试用）
```

### 示例

```sharp
// 条件编译
#define PLATFORM_LINUX
#ifdef PLATFORM_LINUX
    extern i32 linux_syscall(i32 n, i64 a, i64 b);
#endif

// 防止头文件重复包含
#pragma once
struct Vec3 { i32 x; i32 y; i32 z; }

// 调试宏
#define DBG(expr) \
    printf("[%s:%d] " #expr " = %d\n", __FILE__, __LINE__, (expr))

// 可变参数宏
#define LOG(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

// 唯一 ID 生成
#define UNIQUE_NAME(prefix) prefix##__COUNTER__
```
