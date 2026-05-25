# Sharp 标准库设计规范 v1.0

## 版本

- **Status**: 已实现 / 持续迭代
- **最后更新**: 2026-05-21
- **语法**: 使用 Extension Methods 语法 — 方法定义为顶层扩展方法，而非类体内方法；typedef 别名不支持定义扩展方法

---

## 目录

1. [总体设计原则](#1-总体设计原则)
2. [std.types — 基础类型](#3-stdtypes--基础类型)
3. [std.vec — Vec\<T\> 动态数组](#4-stdvec--vect-动态数组)
4. [std.str — Str 字符串切片](#5-stdstr--str-字符串切片)
5. [std.string — String 堆分配可变字符串](#6-stdstring--string-堆分配可变字符串)
6. [std.hashmap — HashMap\<K,V\> 哈希表](#7-stdhashmap--hashmapkv-哈希表)
7. [C23 constexpr — 编译期常量](#8-c23-constexpr--编译期常量)
8. [@to_cstr — 切片转 C 字符串](#9-to_cstr--切片转-c-字符串)
9. [互操作与使用模式](#10-互操作与使用模式)
10. [设计决策附录](#11-设计决策附录)

---

## 1. 总体设计原则

### 1.1 约束

Sharp 编译到 C11，运行时为零。这意味着：

- **没有异常**：所有错误通过返回值表达，致命错误用 `__builtin_trap()` 立即终止
- **没有隐式构造/析构**：结构体零初始化 (`{0}`) 即可用，释放由调用方显式执行
- **没有 trait / interface**：泛型是 monomorphization，不需要统一接口约定
- **没有自动 Drop**：若 `T` 本身持有堆内存（如 `String`），调用方负责先逐个释放元素再释放容器

### 1.2 类型与所有权

Sharp 中 `struct` 与 C11 完全一致（纯数据）。带方法、运算符和泛型的类型使用 `class` 关键字，`class` 自动生成 `typedef`。

> **Extension Methods**：方法不定义在 class/struct 体内部，而是通过顶层扩展方法 `RetType ClassName.method(this, args)` 语法定义。class/struct 体内部仅包含数据字段。方法调用语法 `obj.method(args)` 保持不变。
>
> **Extension Methods + typedef**：扩展方法**只能**定义在原始 class/struct 名称上（如 `Str`、`Vec<T>`），**不能**定义在 `typedef` 别名上（如 `typedef Str MyStr;` 后的 `MyStr`）。`typedef` 是透明别名，不创建新的方法命名空间——这与 C 的哲学一致。通用类型别名（`typedef Vec<int> IntVec`）完全支持，变量声明和方法调用均可正常使用。

| 类型 | 定义方式 | 所有权 | 方法定义 | 类比 |
|------|----------|--------|----------|------|
| `Vec<T>` | `class` | **拥有** `data` 指向的堆内存 | 扩展方法 | Rust `Vec<T>`, C++ `std::vector<T>` |
| `Str` | `class` | **借用**，不拥有任何内存 | 扩展方法 | Rust `&str`, Go `string` |
| `String` | `class` | **拥有** `ptr` 指向的堆内存 | 扩展方法 | Rust `String`, C++ `std::string` |
| `HashMap<K,V>` | `class` | **拥有** `entries` 指向的堆内存 | 扩展方法 | Rust `HashMap`, C++ `std::unordered_map` |

`Str` 可以指向 `String` 的内部缓冲区、字符串字面量、或任意 `const char*` 的一段区间。`String` 被释放后，所有指向它的 `Str` 悬空——由调用方保证生命周期。

### 1.3 命名约定

- 类型名：PascalCase (`Vec`, `Str`, `String`)
- 方法名：全小写下划线 (`push`, `get_ptr`, `as_str`)
- 模块名：全小写点分隔 (`std.vec`, `std.str`, `std.string`, `std.types`)
- 构造函数：`<type>_from_<source>(...)` 模式（`str_from_lit`, `string_from_str`）
- 返回布尔的方法：`is_<condition>()` （`is_empty`）
- 销毁方法：统一使用 `destroy()`

### 1.4 零初始化可用

所有容器类型要求 `{0}` 初始化后处于安全可用状态：

```sharp
Vec<int> v = {0};       // data=NULL, len=0, cap=0
v.push(42);             // 自动分配
v.destroy();            // 释放

String s = {0};         // ptr=NULL, len=0, cap=0
s.push_str(str_from_lit("hello"));
s.destroy();
```

---

## 3. std.types — 基础类型

### 声明

```sharp
```

### 类型定义

#### `isize` — 有符号指针宽度整数

平台相关的有符号整数，宽度等于指针宽度。用于长度、索引、大小等场景。

```sharp
typedef long isize;
```

> **目标平台假设**：Sharp 默认 target 为 `x86_64-linux-gnu` (LP64)，`long` 为 64 位。
> 未来若支持 32 位目标，可以通过编译器内置的 `__INTPTR_TYPE__` 或条件编译调整。
> Windows LLP64 上 `long` 是 32 位——若需支持，届时引入 compiler-probe 选择 `long long` 或 `ptrdiff_t`。

#### `usize` — 无符号指针宽度整数

```sharp
typedef unsigned long usize;
```

### 导出清单

| 符号 | 类型 | 说明 |
|------|------|------|
| `isize` | typedef | 有符号指针宽度整数 |
| `usize` | typedef | 无符号指针宽度整数 |

---

## 4. std.vec — Vec\<T\> 动态数组

### 声明

```sharp
#include "std/types;
```

### 结构体定义

```sharp
class Vec<T> {
    T*    data;
    isize len;
    isize cap;
};
```

> **注意**：所有方法为 Extension Methods，定义在类体外。

### 语义说明

`Vec<T>` 是一个拥有其元素的、堆分配的、可增长的动态数组。

- **元素存储**： `T` 数组在堆上连续分配（`malloc(sizeof(T) * cap)`）
- **长度**： `len` 表示当前有效元素数
- **容量**： `cap` 表示已分配可容纳的元素数，`len <= cap` 恒成立
- **空状态**： `{0}` 初始化为 `data=NULL, len=0, cap=0`，可安全调用任意方法
- **增长**： `push` 时若 `len >= cap` 则自动扩容

### 扩容策略

```
cap == 0  →  分配 8 个元素
cap >  0  →  cap * 2  (翻倍)
```

通过 `realloc` 实现。期望摊销 O(1) 的 `push`。

### 方法清单

#### `void Vec<T>.push(this, T val)`

在末尾追加一个元素。若 `len >= cap`，先扩容再写入。OOM 时 `__builtin_trap()`。

```sharp
void Vec<T>.push(this, T val) {
    if (this->len >= this->cap) {
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;
        T* nd = (T*)realloc(this->data, sizeof(T) * new_cap);
        if (!nd) __builtin_trap();
        this->data = nd;
        this->cap = new_cap;
    }
    this->data[this->len] = val;
    this->len = this->len + 1;
}
```

#### `T Vec<T>.pop(this)`

移除并返回末尾元素。`len == 0` 时调用 `__builtin_trap()`。

```sharp
T Vec<T>.pop(this) {
    if (this->len == 0) __builtin_trap();
    this->len = this->len - 1;
    return this->data[this->len];
}
```

不缩容——保留已分配内存供后续复用。

#### `T Vec<T>.get(this, isize i) const`

返回索引 `i` 处元素的**值拷贝**。越界调用 `__builtin_trap()`。

```sharp
T Vec<T>.get(this, isize i) const {
    if (i < 0 || i >= this->len) __builtin_trap();
    return this->data[i];
}
```

#### `T* Vec<T>.get_ptr(this, isize i)`

返回索引 `i` 处元素的**指针**，允许就地读写。越界调用 `__builtin_trap()`。

```sharp
T* Vec<T>.get_ptr(this, isize i) {
    if (i < 0 || i >= this->len) __builtin_trap();
    return &this->data[i];
}
```

使用场景：

```sharp
*v.get_ptr(3) = 99;          // 就地修改
v.get_ptr(2)->field = x;     // 修改结构体字段
```

#### `void Vec<T>.set(this, isize i, T val)`

将索引 `i` 处元素设置为 `val`。越界调用 `__builtin_trap()`。

```sharp
void Vec<T>.set(this, isize i, T val) {
    if (i < 0 || i >= this->len) __builtin_trap();
    this->data[i] = val;
}
```

#### `T Vec<T>.operator[](this, isize i) const`

下标访问，等同于 `get(i)`。

```sharp
T Vec<T>.operator[](this, isize i) const {
    if (i < 0 || i >= this->len) __builtin_trap();
    return this->data[i];
}
```

> **设计决策**：返回 `T`（值）而非 `T*`（指针）。理由：
> 1. 符合 Sharp 现有 operator[] 语义（见 p112, p152, p154 测试用例）
> 2. 对基础类型（int, float）值拷贝零开销
> 3. 指针访问走 `get_ptr`，语义显式，不会意外

#### `isize Vec<T>.size(this) const`

返回元素数量。

```sharp
isize Vec<T>.size(this) const {
    return this->len;
}
```

#### `isize Vec<T>.capacity(this) const`

返回已分配容量。

```sharp
isize Vec<T>.capacity(this) const {
    return this->cap;
}
```

#### `bool Vec<T>.is_empty(this) const`

是否为空。

```sharp
bool Vec<T>.is_empty(this) const {
    return this->len == 0;
}
```

#### `void Vec<T>.reserve(this, isize n)`

预分配容量至少为 `n`。不改变 `len`，不初始化新空间。
若 `n <= cap` 则无操作。OOM 时 `__builtin_trap()`。

```sharp
void Vec<T>.reserve(this, isize n) {
    if (n <= this->cap) return;
    T* nd = (T*)realloc(this->data, sizeof(T) * n);
    if (!nd) __builtin_trap();
    this->data = nd;
    this->cap = n;
}
```

#### `void Vec<T>.clear(this)`

逻辑清空：`len = 0`。不释放内存，不调用元素的任何清理。

```sharp
void Vec<T>.clear(this) {
    this->len = 0;
}
```

#### `void Vec<T>.destroy(this)`

释放 `data` 指向的堆内存，并将 `data`、`len`、`cap` 全部归零。
**不会**释放在元素中嵌入的指针——若 `T` = `String`，调用方需先逐个 `destroy` 每个元素的内部内存。

```sharp
void Vec<T>.destroy(this) {
    free(this->data);
    this->data = (T*)0;
    this->len  = 0;
    this->cap  = 0;
}
```

### Vec\<String\> 释放模式

由于 Sharp 没有析构器，当 `Vec<String>` 不再使用时，调用方负责：

```sharp
// 方式一：手动逐个释放
Vec<String> vs = {0};
vs.push(string_from_str(str_from_lit("alpha")));
vs.push(string_from_str(str_from_lit("beta")));

for (isize i = 0; i < vs.size(); i = i + 1)
    vs.get_ptr(i)->destroy();   // String.destroy()
vs.destroy();                    // Vec.destroy()
```

`vs.destroy()` 只释放 `data` 指针数组本身（`sizeof(String) * cap` 的缓冲区），不递归释放每个 `String` 内部持有的 `ptr`。调用方对元素内部的堆内存负责。

> **未来方向**：当 Sharp 支持 trait/interface 后，可引入 `destroy_deep()` 方法自动遍历调用元素的 `destroy()`。当前阶段保持这个约定。

### Vec 方法总览

| 方法 | 签名 | 备注 |
|------|------|------|
| `push` | `void Vec<T>.push(this, T val)` | 自动扩容，OOM → trap |
| `pop` | `T Vec<T>.pop(this)` | 越界 → trap；不缩容 |
| `get` | `T Vec<T>.get(this, isize i) const` | 返回值拷贝；越界 → trap |
| `get_ptr` | `T* Vec<T>.get_ptr(this, isize i)` | 返回指针；越界 → trap |
| `set` | `void Vec<T>.set(this, isize i, T val)` | 直接写入；越界 → trap |
| `operator[]` | `T Vec<T>.operator[](this, isize i) const` | 等同 `get(i)` |
| `size` | `isize Vec<T>.size(this) const` | 元素数 |
| `capacity` | `isize Vec<T>.capacity(this) const` | 已分配容量 |
| `is_empty` | `bool Vec<T>.is_empty(this) const` | len == 0 |
| `reserve` | `void Vec<T>.reserve(this, isize n)` | 预分配，不改 len，OOM → trap |
| `clear` | `void Vec<T>.clear(this)` | len = 0，不释放内存 |
| `destroy` | `void Vec<T>.destroy(this)` | 释放 data，全部归零 |

### 使用示例

```sharp
#include "std/vec;

// 基础使用
Vec<int> v = {0};
v.push(10);
v.push(20);
v.push(30);

int x = v[1];              // 20
*v.get_ptr(0) = 99;        // 就地修改
v.set(2, 300);

while (!v.is_empty()) {
    int val = v.pop();
    // process val
}
v.destroy();

// 预分配
Vec<MyStruct> items = {0};
items.reserve(1000);
for (isize i = 0; i < 1000; i = i + 1)
    items.push(make_item(i));
// ... use items
items.destroy();
```

---

## 5. std.str — Str 字符串切片

### 声明

```sharp
#include "std/types;
```

### 结构体定义

```sharp
class Str {
    const char* ptr;
    isize       len;
};
```

> **注意**：所有方法为 Extension Methods，定义在类体外。

### 语义说明

`Str` 是一个**非拥有**的字符串切片（胖指针）。它可以指向：

- 字符串字面量（`.rodata` 段）
- `String` 的内部缓冲区
- 任意 `char*` 指针的任意区间

`Str` 不管理任何内存，不需要 `destroy()`。但调用方必须保证在 `Str` 使用期间其 `ptr` 指向的内存有效。

**核心规则：`Str` 不以 `\0` 结尾。** 长度由 `len` 字段独立跟踪。

### 构造器

#### `Str str_from_lit(const char* lit)`

从 `\0` 结尾的 C 字面量创建 `Str`。遍历计算长度。

```sharp
Str str_from_lit(const char* lit) {
    Str s;
    s.ptr = lit;
    s.len = 0;
    while (lit[s.len] != 0) s.len = s.len + 1;
    return s;
}
```

使用：

```sharp
Str hello = str_from_lit("hello, world");
// hello.ptr = "hello, world\0" (rodata)
// hello.len = 12
```

#### `Str str_from_parts(const char* p, isize len)`

直接从指针和长度构造。调用方保证 `p` 至少包含 `len` 字节。

```sharp
Str str_from_parts(const char* p, isize len) {
    Str s;
    s.ptr = p;
    s.len = len;
    return s;
}
```

### 方法

#### `Str Str.slice(this, isize start, isize end) const`

返回子切片 `[start, end)`。O(1)，零拷贝。越界调用 `__builtin_trap()`。

```sharp
Str Str.slice(this, isize start, isize end) const {
    if (start < 0 || end < start || end > this->len) __builtin_trap();
    Str s;
    s.ptr = this->ptr + start;
    s.len = end - start;
    return s;
}
```

#### `bool Str.eq(this, Str other) const`

逐字节相等比较。同长度且所有字节相同返回 `true`。

```sharp
bool Str.eq(this, Str other) const {
    if (this->len != other.len) return false;
    isize i = 0;
    while (i < this->len) {
        if (this->ptr[i] != other.ptr[i]) return false;
        i = i + 1;
    }
    return true;
}
```

#### `bool Str.starts_with(this, Str prefix) const`

是否以 `prefix` 开头。

```sharp
bool Str.starts_with(this, Str prefix) const {
    if (prefix.len > this->len) return false;
    isize i = 0;
    while (i < prefix.len) {
        if (this->ptr[i] != prefix.ptr[i]) return false;
        i = i + 1;
    }
    return true;
}
```

#### `bool Str.ends_with(this, Str suffix) const`

是否以 `suffix` 结尾。

```sharp
bool Str.ends_with(this, Str suffix) const {
    if (suffix.len > this->len) return false;
    isize off = this->len - suffix.len;
    isize i = 0;
    while (i < suffix.len) {
        if (this->ptr[off + i] != suffix.ptr[i]) return false;
        i = i + 1;
    }
    return true;
}
```

#### `isize Str.find_byte(this, char c) const`

查找字节 `c` 首次出现的位置，返回索引。未找到返回 `-1`。

```sharp
isize Str.find_byte(this, char c) const {
    isize i = 0;
    while (i < this->len) {
        if (this->ptr[i] == c) return i;
        i = i + 1;
    }
    return -1;
}
```

### 自由函数

#### `StrSplit str_split_once(Str s, char sep)`

以第一个 `sep` 字符分割。返回 `{left, right, found}` 结构体。

```sharp
struct StrSplit {
    Str  left;
    Str  right;
    bool found;
}

StrSplit str_split_once(Str s, char sep) {
    isize idx = s.find_byte(sep);
    if (idx < 0) {
        StrSplit r;
        r.left  = s;
        r.right = str_from_parts(s.ptr + s.len, 0);
        r.found = false;
        return r;
    }
    StrSplit r;
    r.left  = str_from_parts(s.ptr, idx);
    r.right = str_from_parts(s.ptr + idx + 1, s.len - idx - 1);
    r.found = true;
    return r;
}
```

`StrSplit` 的 `left` 和 `right` 的 `ptr` 仍指向原始字符串内部——零分配。

```sharp
Str input = str_from_lit("key=value");
StrSplit pair = str_split_once(input, '=');
// pair.left.eq(str_from_lit("key"))   == true
// pair.right.eq(str_from_lit("value")) == true
// pair.found == true
```

> **设计决策**：`split_once` 是自由函数而非 Str 的方法。原因是 SMI 生成 C 代码时，
> 若 `StrSplit` 在 `Str` 之前定义，`Str` 仅为前向声明，导致 `StrSplit` 包含不完整的 `Str` 类型。
> 因此将 `StrSplit` 放在 `Str` 之后定义，`split_once` 作为自由函数。

### Str 方法总览

| 符号 | 签名 | 备注 |
|------|------|------|
| `str_from_lit` | `Str str_from_lit(const char* lit)` | 从 C 字面量构造 |
| `str_from_parts` | `Str str_from_parts(const char* p, isize len)` | 从 ptr+len 直接构造 |
| `slice` | `Str Str.slice(this, isize start, isize end) const` | O(1) 子切片 |
| `eq` | `bool Str.eq(this, Str other) const` | 逐字节比较 |
| `starts_with` | `bool Str.starts_with(this, Str prefix) const` | 前缀匹配 |
| `ends_with` | `bool Str.ends_with(this, Str suffix) const` | 后缀匹配 |
| `find_byte` | `isize Str.find_byte(this, char c) const` | 首次出现位置，-1=未找到 |
| `str_split_once` | `StrSplit str_split_once(Str s, char sep)` | 首次分割，零分配 |

### Str vs C 字符串

`Str` 不以 `\0` 结尾。若需 C 互操作（如传给 `printf("%s")`），必须通过 `String` 包装——`String.as_str()` 返回的 `Str` 也不以 `\0` 结尾，但底层缓冲区内存在 `String` 生命周期内保持有效。

```sharp
// 错误：Str 不以 \0 结尾，不能直接当 C string 用
// printf("%s", my_str.ptr);  ← 可能读过界

// 正确：通过 String 获取底层数据
String s = string_from_str(my_str);
Str view = s.as_str();
// 使用 view.ptr 和 view.len 通过 fwrite 等 API 传输
fwrite(view.ptr, 1, view.len, stdout);
s.destroy();
```

---

## 6. std.string — String 堆分配可变字符串

### 声明

```sharp
#include "std/types;
#include "std/str;
```

### 结构体定义

```sharp
class String {
    char* ptr;
    isize len;
    isize cap;
};
```

> **注意**：所有方法为 Extension Methods，定义在类体外。`string_from_str` 为自由函数（非扩展方法）。

### 语义说明

`String` 是一个**拥有**其内容的、堆分配的、可增长的字符串。

- **不以 `\0` 结尾**：内部缓冲区仅为原始字节序列，长度由 `len` 独立跟踪
- **零初始化**：`{0}` 为 `ptr=NULL, len=0, cap=0`，可安全调用任意方法
- **视图获取**：通过 `as_str()` 获取 `Str` 视图，调用方使用 `ptr` + `len` 访问数据

### 扩容策略

与 `Vec` 一致：

```
cap == 0  →  分配 8 字节
cap >  0  →  cap * 2
```

### 构造器

#### `String string_from_str(Str s)`

从 `Str` 创建拥有副本的 `String`。将 `s` 的内容拷贝到新分配的堆内存中。

```sharp
String string_from_str(Str s) {
    String r;
    r.len = s.len;
    r.cap = (s.len < 8) ? 8 : s.len;
    char* nd = (char*)malloc(r.cap);
    if (!nd) __builtin_trap();
    r.ptr = nd;
    isize i = 0;
    while (i < s.len) {
        r.ptr[i] = s.ptr[i];
        i = i + 1;
    }
    return r;
}
```

### 方法

#### `void String.push_byte(this, char c)`

追加单字节。若 `len >= cap` 则扩容。OOM 时 `__builtin_trap()`。

```sharp
void String.push_byte(this, char c) {
    if (this->len >= this->cap) {
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;
        if (new_cap <= this->len) new_cap = this->len + 1;
        char* nd = (char*)realloc(this->ptr, new_cap);
        if (!nd) __builtin_trap();
        this->ptr = nd;
        this->cap = new_cap;
    }
    this->ptr[this->len] = c;
    this->len = this->len + 1;
}
```

#### `void String.push_str(this, Str s)`

追加整个 `Str` 的内容。若容量不足则扩容后批量拷贝。OOM 时 `__builtin_trap()`。

```sharp
void String.push_str(this, Str s) {
    if (this->len + s.len > this->cap) {
        isize new_cap = (this->cap == 0) ? 8 : this->cap * 2;
        while (new_cap <= this->len + s.len) new_cap = new_cap * 2;
        char* nd = (char*)realloc(this->ptr, new_cap);
        if (!nd) __builtin_trap();
        this->ptr = nd;
        this->cap = new_cap;
    }
    isize i = 0;
    while (i < s.len) {
        this->ptr[this->len + i] = s.ptr[i];
        i = i + 1;
    }
    this->len = this->len + s.len;
}
```

#### `Str String.as_str(this) const`

返回指向同一块内存的 `Str`。O(1)，零拷贝。`String` 被 `destroy()` 后此 `Str` 悬空。

```sharp
Str String.as_str(this) const {
    return str_from_parts(this->ptr, this->len);
}
```

#### `void String.clear(this)`

逻辑清空：`len = 0`。不释放内存。

```sharp
void String.clear(this) {
    this->len = 0;
}
```

#### `void String.reserve(this, isize n)`

预分配至少 `n` 字节容量。若 `n <= cap` 则无操作。OOM 时 `__builtin_trap()`。

```sharp
void String.reserve(this, isize n) {
    if (n <= this->cap) return;
    char* nd = (char*)realloc(this->ptr, n);
    if (!nd) __builtin_trap();
    this->ptr = nd;
    this->cap = n;
}
```

#### `isize String.size(this) const`

返回字节长度。

```sharp
isize String.size(this) const {
    return this->len;
}
```

#### `isize String.capacity(this) const`

返回已分配容量。

```sharp
isize String.capacity(this) const {
    return this->cap;
}
```

#### `bool String.is_empty(this) const`

是否为空字符串。

```sharp
bool String.is_empty(this) const {
    return this->len == 0;
}
```

#### `void String.destroy(this)`

释放 `ptr` 指向的堆内存，全部字段归零。

```sharp
void String.destroy(this) {
    free(this->ptr);
    this->ptr = (char*)0;
    this->len = 0;
    this->cap = 0;
}
```

### String 方法总览

| 符号 | 签名 | 备注 |
|------|------|------|
| `string_from_str` | `String string_from_str(Str s)` | 从 Str 拷贝构造（自由函数） |
| `push_byte` | `void String.push_byte(this, char c)` | 追加单字节，自动扩容 |
| `push_str` | `void String.push_str(this, Str s)` | 追加 Str 内容，自动扩容 |
| `as_str` | `Str String.as_str(this) const` | O(1) 零拷贝视图 |
| `clear` | `void String.clear(this)` | len = 0，不释放内存 |
| `reserve` | `void String.reserve(this, isize n)` | 预分配，OOM → trap |
| `size` | `isize String.size(this) const` | 字节长度 |
| `capacity` | `isize String.capacity(this) const` | 已分配容量 |
| `is_empty` | `bool String.is_empty(this) const` | len == 0 |
| `destroy` | `void String.destroy(this)` | 释放 ptr，全部归零 |

### 使用示例

```sharp
#include "std/string;
#include "std/str;

String s = {0};

s.push_str(str_from_lit("hello"));
s.push_str(str_from_lit(", "));
s.push_str(str_from_lit("world"));

Str view = s.as_str();
// view.len == 13, view.ptr == s.ptr

Str sub = view.slice(0, 5);
// sub.eq(str_from_lit("hello")) == true

// 通过 fwrite 输出（Str 不以 \0 结尾）
fwrite(view.ptr, 1, view.len, stdout);

s.destroy();
// view, sub 全部悬空
```

---

## 8. C23 constexpr — 编译期常量

### 7.1 语义

Sharp 实现 C23 标准（N2954/N2976 变量 + N3205 TS 函数）的 `constexpr` 关键字。与 C23 完全一致：

```sharp
// constexpr 变量
constexpr int BUFFER_SIZE = 1024;

// constexpr 函数
constexpr int square(int x) {
    return x * x;
}
```

### 7.2 C23 限制

遵循 C23 规范，constexpr 函数：

| 允许 | 禁止 |
|------|------|
| 局部变量 | 递归调用 |
| 循环 (while/for) | `goto` / `continue` |
| 条件 (if/ternary) | `asm` 语句 |
| 调用其他 constexpr 函数 | 可变操作 |
| `sizeof` | |

### 7.3 编译期求值

编译器在语义分析阶段对 constexpr 表达式进行编译期折叠求值：

- **二元运算符**：`+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `&`, `|`, `^`, `<<`, `>>`
- **一元运算符**：`-`, `+`, `~`, `!`
- **三元运算符**：`cond ? then : else`
- **sizeof**：基本类型的编译期大小
- **函数调用**：参数代入求值（无递归检测）

### 7.4 与集成

constexpr 变量和函数可在同文件或通过 `#include` 引入的其他文件中使用。生成的 C 代码为 C23 兼容的 `constexpr` 声明。

```sharp
constexpr int MAX_CONNECTIONS = 128;
constexpr int TIMEOUT_MS = 5000;

constexpr int clamp(int val, int lo, int hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}
```

```sharp
#include "std/config;

int arr[MAX_CONNECTIONS];  // 编译期常量可用于数组大小
if (clamp(150, 0, 100) != 100) return 1;
```

### 7.5 生成 C 代码

constexpr 变量和函数生成 C23 兼容的 `constexpr` 关键字：

```c
constexpr int BUFFER_SIZE = 1024;
constexpr int square(int x) { return x * x; }
```

### 7.6 测试覆盖

| 测试 | 内容 |
|------|------|
| p210 | 基本变量、算术函数、sizeof |
| p211 | 嵌套函数调用、变量链 |
| p212 | 三元运算符、max/min/abs/clamp |
| p213 | 负数、一元运算符 |
| p214 | 位运算、移位 |
| p215 | sizeof、数组大小计算 |
| p216 | constexpr + 泛型函数 |
| p217 | constexpr + 全局跨文件共享 |

---

## 9. @to_cstr — 切片转 C 字符串

### 10.1 语法

```sharp
const char *cstr = @to_cstr(slice);
```

`@to_cstr(expr)` 是 Sharp 的内置宏（intrinsic），将任何具有 `.ptr` 和 `.len` 字段的 slice 类型转换为 NUL-terminated C 字符串（`const char*`）。

### 10.2 实现原理

代码生成阶段使用 GCC VLA 语句表达式（statement-expression），在栈上分配 `len+1` 字节，拷贝 slice 内容并追加 `\0`：

```c
#define __SP_MAGIC_TO_C_STR(__sp_slice__) ( ({ \
    __auto_type __src__ = (__sp_slice__); \
    const size_t __local_len__ = __src__.len; \
    char __local_vla_buf__[__local_len__ + 1]; \
    if (__local_len__ > 0) { \
        memcpy(__local_vla_buf__, __src__.ptr, __local_len__); \
    } \
    __local_vla_buf__[__local_len__] = '\0'; \
    (const char*)__local_vla_buf__; \
}) )
```

### 10.3 特性

- **零浪费**：VLA 精确分配 `len+1` 字节，不多不少
- **多调用安全**：每次调用有独立的栈上缓冲区
- **可重入**：无全局状态，天然线程安全
- **类型无关**：通过 `__auto_type` 自动推导，兼容 `Str`、`String.as_str()`、任何 `.ptr` + `.len` 结构体

### 9.4 使用示例

```sharp
#include "std/str;

// 基本使用
Str hello = str_from_lit("Hello");
const char *cstr = @to_cstr(hello);
printf("%s\n", cstr);    // 安全：cstr 以 \0 结尾

// 子切片
Str ell = hello.slice(1, 4);
const char *e = @to_cstr(ell);    // "ell\0"

// 空切片
Str empty = str_from_parts(hello.ptr, 0);
const char *n = @to_cstr(empty);  // "\0"
```

### 9.5 语义检查

- 必须恰好 1 个参数
- 返回类型：`const char*`
- 参数的 `.ptr` 和 `.len` 访问不进行检查——若结构体缺少这些字段，C 编译器会报错

### 9.6 测试

| 测试 | 内容 |
|------|------|
| p218 | 导入 std.str Str，基本转换、子切片、空切片、函数调用 |

---

## 10. 互操作与使用模式

### 10.1 Str 和 String 的关系

```
                    str_from_lit("hello")
                           │
                           ▼
                    Str {ptr, len=5}
                      │         │
          string_from_str()    slice(1,3)
              │   (拷贝)          │ (零拷贝，仍指向字面量)
              ▼                   ▼
        String {ptr, len=5}   Str {ptr+1, len=2}  → "el"
              │
          as_str()
          (零拷贝，指向 String 内部)
              │
              ▼
        Str {ptr, len=5}
```

`String.destroy()` 后，通过 `as_str()` 获取的所有 `Str` 都会悬空。这没有运行时检查——是 Sharp 无 GC 模型下的基本约束。

### 10.2 Vec\<String\> 的正确释放

```sharp
#include "std/vec;
#include "std/string;
#include "std/str;

Vec<String> lines = {0};

// 读文件，每行一个 String
// ... (模拟)
lines.push(string_from_str(str_from_lit("line1")));
lines.push(string_from_str(str_from_lit("line2")));

// 使用
for (isize i = 0; i < lines.size(); i = i + 1) {
    String* line = lines.get_ptr(i);
    Str view = line->as_str();
    // process view
}

// 释放（顺序重要：先 String 后 Vec）
for (isize i = 0; i < lines.size(); i = i + 1)
    lines.get_ptr(i)->destroy();    // String.destroy()
lines.destroy();                     // Vec.destroy()
```

### 10.3 C 互操作

Sharp 代码可以 `#include <stdlib.h>` 和 `#include <stdio.h>`（见 p154）。

```sharp
#include "std/str;
#include "std/string;
#include <stdio.h>

void print_str(Str s) {
    String temp = string_from_str(s);
    Str view = temp.as_str();
    fwrite(view.ptr, 1, view.len, stdout);
    putchar('\n');
    temp.destroy();
}
```

> 注意这里每次 `print_str` 都会分配一个临时 String 作为缓冲区。高频场景可以直接使用 `fwrite(s.ptr, 1, s.len, stdout)`。

