# Sharp 语言规格 v1.0

> C11 完整超集 · class 方法与泛型 · 运算符 · defer · 转译到 C11 · 零运行时

***

每个特性必须通过两个验证:

1. C 无法干净表达
2. 转译到 C 后零开销

**Sharp 是什么:** C11 的严格超集，在 C11 之上增加：`struct` 与 C11 完全一致（纯数据，无方法）· `class` 带方法、运算符、泛型和自动 typedef · 模板单态化 · `defer` 资源管理 · 运算符重载 · `@` 编译期内省原语。不给新概念，只给 C 惯用法命名。

Sharp **不追踪所有权**，不引入隐式行为。代码里每一个分配、拷贝、释放都看得见。

***

## C11 完整超集保证

Sharp 是 **ISO C11 的严格超集**。`struct` 保持 C11 原始语义（纯数据布局，无方法、无泛型）。

> **任何合法的 C11 程序，不加修改地通过** **`sharpc`** **编译，生成的 C11 输出语义等价于原始 C11 源文件。**

具体保证：

| 保证项               | 说明                                                                                                 |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| **词法完整**          | 所有 C11 关键字、字面量形式、转义序列、预处理指令原样透传                                                                    |
| **语法完整**          | 所有 C11 声明符语法（含 K\&R 外）、表达式、语句、链接说明符                                                                |
| **语义等价**          | `_Atomic`、`restrict`、`register`、VLA、`_Static_assert`、`__attribute__` 等完整保留                         |
| **GCC 扩展直通**      | `__asm__`、`__attribute__`、`__int128`、`typeof`、statement-expression、computed goto 等直通               |
| **宏直通**           | `#include`、`#define`、`#if` 等预处理器行为与 C 完全一致                                                        |

***

### 值语义（和 C 完全一致）

结构体赋值、值传参、函数返回，全部是 **memcpy**，和 C 的 struct 赋值完全一致:

```sharp
Buffer a = Buffer.alloc(1024);
Buffer b = a;   // memcpy: b.ptr == a.ptr，两者共享同一块内存
```

如果一个类型持有堆资源，使用者有责任不对它做值拷贝。用 `T*` 传指针。**Sharp 容器不管理元素生命周期** — 就像 C 数组不管理元素生命周期。如果元素持有堆资源，使用者在销毁容器前自己释放。

### 泛型契约

Sharp 保证:

- 每个泛型实例化生成独立的 C 代码（单态化，monomorphization）
- 相同类型实参的实例化在全程序内**符号唯一**，由链接器去重（weak / COMDAT）
- 泛型体在所有实例化点可见时，错误在**实例化点**报告
- `@` 内省原语返回编译期常量，不产生运行时开销
- 禁止方法特化：每个泛型 struct 的方法集合唯一，不允许针对具体类型实参重新定义方法

Sharp 不保证:

- 泛型代码量上限（单态化天然会膨胀）

***

## 语言核心

| 特性                   | 理由                                                |
| -------------------- | ------------------------------------------------- |
| **C11 完整超集**         | 任何 `.c` 文件不加修改通过 `sharpc`，生成语义等价的 C11 输出          |
| `struct` 纯 C11       | 与 C11 完全一致，纯数据布局，无方法、无泛型                            |
| `class` 方法与泛型      | 带方法、运算符重载、泛型 `<T>`，自动生成 typedef                      |
| `auto` 类型推导          | 从初始化表达式推导类型，泛型长名缩写，转译后生成具体类型                      |
| `defer` 延迟执行         | 替代 `goto cleanup` 的语法糖，退出作用域时执行，代价可见              |
| 运算符重载                | 数学/向量代码必要；class 体内为糖，亦可写自由函数                     |
| 泛型 `<T>` + 单态化       | C 只能靠宏模拟                                          |
| `@` 内省原语             | 泛型约束 + 类型分派机制，封闭可枚举的 7 个原语 (1 个断言 + 6 个查询)         |

***

## Sharp 关键字与字面量扩展

除继承 C11 的全部关键字外，Sharp 新增以下词法元素：

| 词法元素       | 说明                                                |
| ---------- | ------------------------------------------------- |
| `class`    | 带方法、运算符、泛型的类型定义，自动生成 `typedef`                    |
| `defer`    | 延迟执行语句                                            |
| `operator` | 运算符重载声明                                           |
| `this`     | 方法体内隐式接收者指针，类型为 `T*` 或 `const T*`                 |
| `constexpr`| C23 编译期常量                                          |

> `class`、`defer`、`operator`、`this`、`constexpr` 为上下文敏感关键字：仅当解析器识别为 Sharp 语言特性时作关键字处理，否则保持标识符语义（C 超集保证）。

### struct 字面量

Sharp 提供 struct 字面量语法（具名字段初始化）：

```sharp
Vec3 v = Vec3 { .x = 1.0, .y = 2.0, .z = 3.0 };
Buffer b = Buffer { .ptr = malloc(n), .len = n };
```

语法：`TypeName { .field = expr, .field = expr, ... }`，C 风格 designated
initializer。字段可以省略（未列出的字段零初始化）。转译到 C 时生成
`(TypeName){ .field = expr, ... }` compound literal 形式。

Sharp 类型在代码生成时映射为对应的 C 标准类型。

### 整型类型

| 类型          | 说明                |
| ----------- | ----------------- |
| `char`      | 8 位字符/整数          |
| `short`     | 16 位短整数           |
| `int`       | 32 位整数            |
| `long`      | 32/64 位长整数（取决于平台） |
| `long long` | 64 位长整数           |

### 浮点类型

| 类型       | 说明      |
| -------- | ------- |
| `float`  | 32 位浮点数 |
| `double` | 64 位浮点数 |

### 其他类型

| 类型     | 说明   |
| ------ | ---- |
| `bool` | 布尔类型 |
| `void` | 空类型  |

### `auto` 类型推导

`auto` 从初始化表达式推导变量类型。转译到 C 时生成具体类型，无运行时开销。

```sharp
auto n = xs.size();                           // n: long
auto v = Vec3 { .x = 1, .y = 2, .z = 3 };           // v: Vec3
auto cache = HashMap<Str, Buffer>.new();       // 泛型长名缩写
```

规则：

| 写法                     | 说明                         |
| ---------------------- | -------------------------- |
| `auto x = expr;`       | `x` 的类型 = `expr` 的类型       |
| `const auto x = expr;` | `const` 修饰推导出的类型           |
| `auto* p = &x;`        | `p` 是指针，编译器验证 `expr` 确实是指针 |
| 无初始化表达式                | 编译错误 — `auto x;` 非法        |

**不做：** 函数返回类型推导、trailing return type— 函数签名必须显式。

***

## `const` 类型修饰符

`const` 是**类型限定符**，语义与 C 完全一致。`const` 是 Sharp 类型系统的一等部分，参与类型匹配、赋值检查和函数签名。

### 基本规则

1. **`const T`** **和** **`T`** **是不同类型**，但支持单向的限定转换:
   - `T` → `const T`: 隐式合法（加限定）
   - `const T` → `T`: 编译错误（丢弃限定）
2. **只读对象:** `const T x = expr;` 声明后 `x` 不能再被赋值。绕路通过指针写 `const` 对象是未定义行为。
3. **顶层 const 不参与函数类型匹配**（与 C 一致）: `void f(const int x)` 与 `void f(int x)` 是同一函数类型。这条只对**顶层**参数成立；参数中的 `const T*` 仍然参与匹配。
4. **`const T x = expr;`** **不是编译期常量**，与 C 一致。需要编译期整数常量时使用 `enum`。
5. **const 不能作为泛型类型实参的顶层限定。** `Vec<const int>` 非法，因为 const 值类型不可赋值。`Vec<const int*>` 合法（const 在 pointee 上，非顶层）。

### 指针的三种组合

| 写法                 | 含义                                      |
| ------------------ | --------------------------------------- |
| `const T* p`       | 指向 const T 的指针；`*p = ...` 非法            |
| `T* const p`       | const 指针指向 T；`p = ...` 非法，`*p = ...` 合法 |
| `const T* const p` | 两者皆 const                               |

与 C 完全一致。

### const 与泛型

const **不能**作为泛型类型实参的**顶层**限定:

```sharp
Vec<const char*> strings;       // ✅ const 在 pointee 上，非顶层
const Vec<int> v = make_vec();  // ✅ const 在变量上，非类型实参

Vec<const int> v1;              // ❌ const 修饰值类型，顶层 const
Vec<int* const> v2;             // ❌ const 修饰指针本身，顶层 const
```

判定方法：检查类型实参的 `ty_is_const()` 返回值。

| 类型实参          | `ty_is_const()`      | 合法? |
| ------------- | -------------------- | --- |
| `const int`   | true                 | ❌   |
| `const int*`  | false (const 在 base) | ✅   |
| `int* const`  | true                 | ❌   |
| `const Point` | true                 | ❌   |
| `Point*`      | false                | ✅   |

实例化时 `const T*` 和 `T*` 视为不同类型（const 在 pointee 上），各自生成独立的实例化。

### const 与方法

方法可声明 const 后缀，承诺不修改 `*this`:

```sharp
class Vec<T> {
    T*   data;
    long len;
    long cap;

    long size() const {          // const 方法: this 类型为 const Vec<T>*
        return this->len;
    }

    T at(long i) const {         // const 方法
        return this->data[i];
    }

    void push(T item) {          // 非 const: this 类型为 Vec<T>*
        ...
    }
}
```

调用规则:

- **const 方法:** `this` 类型为 `const T*`。可用于 const 或非 const 接收者
- **非 const 方法:** `this` 类型为 `T*`。仅可用于非 const 接收者；在 const 接收者上调用是编译错误
- const 方法体内不能修改 `this->...`，不能调用非 const 方法

### const 与运算符

struct 体内的运算符按其脱糖形式决定能否作用于 const 接收者:

| 运算符类别                              | 脱糖形式                              | const 接收者可用? |
| ---------------------------------- | --------------------------------- | ------------ |
| 算术 + 比较（`+`、`-`、`*`、`==`、一元 `-` 等） | `operator<op>(T this, ...)` 值传    | ✅            |
| 下标（`[]`）读取形式                       | `operator[](const T* this, long)` | ✅            |
| 下标（`[]`）写入形式                       | `operator[](T* this, long)`       | ❌            |

使用者可以同时提供 const 和非 const 两个 `[]` 重载，编译器按接收者限定分派。

### 代码生成

`const` 完整保留到生成的 C 代码中。类型匹配、赋值检查、const 方法分派在 Sharp 层完成；最终生成的 C 代码中 `const` 按原位置保留，C 编译器做最后把关。

const **不**导致泛型实例化膨胀: 顶层 const 的类型实参被拒绝；`const Vec<T>`（const 在变量上）复用同一实例化；仅 `const T*`（const 在 pointee 上）产生独立实例化。

Sharp 层的 const 错误信息直接映射回 Sharp 源码行号，不依赖 C 编译器报错。

***

## struct — 纯 C11

`struct` 与 C11 完全一致，纯数据布局，无方法、无泛型、无运算符：

```sharp
struct Point {
    float x;
    float y;
};

struct Point p = { .x = 1.0, .y = 2.0 };
```

`struct` 保持 C11 全部语义：值拷贝是 memcpy，可以取地址，可以做 `sizeof`，支持 designated initializer 和 compound literal。

---

## class — 带方法、运算符和泛型的类型

方法、关联函数、运算符、泛型全部定义在 `class` 体内。class 体既是数据布局的声明，也是该类型全部操作的命名空间。class 自动生成 `typedef`——类型名可直接使用，无需 `class` 前缀。

### 基本语法

```sharp
class Buffer {
    char* ptr;
    long  len;

    // 关联函数（无 this，通过 Buffer.alloc(n) 调用）
    Buffer alloc(long n) {
        return Buffer { .ptr = malloc(n), .len = n };
    }

    // 方法（有隐式 this，通过 b.get(i) 或 p->get(i) 调用）
    char get(long i) {
        return this->ptr[i];
    }

    // const 方法
    long length() const {
        return this->len;
    }

    // 释放资源（普通方法，手动调用）
    void destroy() {
        free(this->ptr);
        this->ptr = NULL;
        this->len = 0;
    }
}
```

### 关联函数 vs 方法

| <br /> | 关联函数              | 方法                                   |
| ------ | ----------------- | ------------------------------------ |
| `this` | 无                 | 有（隐式）                                |
| 调用语法   | `Type.func(args)` | `obj.func(args)` / `ptr->func(args)` |
| 判定规则   | 函数体内不使用 `this`    | 函数体内使用 `this`                        |

关联函数等价于 C 中的 `Type_func(args)` 约定，方法等价于 `Type_func(&obj, args)`。

### 语义规则

- class 体内声明的函数属于该类型的命名空间
- 方法的隐式 `this`: 非 const 方法中类型为 `T*`，const 方法（带 `const` 后缀）中类型为 `const T*`
- const 接收者只能调用 const 方法；非 const 接收者可调用任一
- class 的字段声明必须在方法之前（先数据后操作）
- 定义必须对使用点可见（通过 `#include` 或在同文件内定义）

### 调用语法

```sharp
Buffer b = Buffer.alloc(1024);   // 关联函数
char x = b.get(0);               // 方法，值接收者
Buffer* p = &b;
char y = p->get(0);              // 方法，指针接收者
long n = b.length();             // const 方法
b.destroy();                     // 手动释放
```

### 转译到 C

class 转译到 C 时，自动生成对应的 `typedef struct`，方法转为前缀命名函数：

```c
// class Buffer { char* ptr; long len; ... }
// →
typedef struct { char* ptr; long len; } Buffer;

// Buffer alloc(long n) { ... }
Buffer Buffer__alloc(long n) { ... }

// char get(long i) { ... }
char Buffer__get(Buffer* this, long i) { ... }

// long length() const { ... }
long Buffer__length(const Buffer* this) { ... }

// b.get(0) → Buffer__get(&b, 0)
// p->get(0) → Buffer__get(p, 0)
```

***

## defer 语句

`defer` 在当前 `{}` 块退出时执行指定的语句或代码块。它是 `goto cleanup` 的语法糖 — 行为完全显式，任何读代码的人一眼能看出什么语句在退出时执行。

### 语法

```sharp
defer EXPR;

defer {
    STATEMENTS;
}
```

### 作用域绑定

`defer` 绑定到**最近的** **`{}`** **块**（不一定是函数级别）。块退出方式包括: 自然结束、`return`、`break`、`continue`。

```sharp
void example() {
    Buffer a = Buffer.alloc(64);
    defer a.destroy();          // 注册：函数退出时执行

    if (condition) {
        Buffer b = Buffer.alloc(128);
        defer b.destroy();      // 注册：if 块退出时执行
        // if 块结束 → b.destroy() 触发
    }

    // 函数结束 → a.destroy() 触发
}
```

### 执行顺序

多个 `defer` 按 **LIFO** 逆序执行 — 后注册的先执行:

```sharp
void example() {
    Buffer a = Buffer.alloc(64);
    defer a.destroy();          // 第 1 个注册

    Buffer b = Buffer.alloc(128);
    defer b.destroy();          // 第 2 个注册

    // 退出时执行顺序: b.destroy() → a.destroy()
}
```

### 禁止嵌套 defer

`defer` 语句块内部不允许再写 `defer`:

```sharp
defer {
    cleanup_logs();
    defer reset_flags();        // ❌ 编译错误：defer 不能嵌套
}
```

### goto 限制

`goto` 不能跨越任何 `defer` 语句（不论方向）:

```sharp
// ❌ 向前跳过 defer
goto skip;
Buffer a = Buffer.alloc(64);
defer a.destroy();
skip:

// ❌ 向后跳过 defer（往回跳也跨越了 defer 的注册顺序）
Buffer b = Buffer.alloc(64);
defer b.destroy();
goto retry;
retry:

// ❌ 从外部跳入包含 defer 的块
goto inside;
{
    Buffer c = Buffer.alloc(64);
    defer c.destroy();
inside:
}
```

### break / continue

`break` 和 `continue` 安全 — 它们退出块时自然触发块内已注册的 `defer`:

```sharp
for (long i = 0; i < n; i++) {
    Buffer b = Buffer.alloc(64);
    defer b.destroy();      // break 和 continue 都会触发它

    if (b.len == 0) break;     // break → b.destroy() 触发 ✅
    if (skip)       continue;  // continue → b.destroy() 触发 ✅
}
```

### 代码生成（设计决策）

defer 翻译策略是 **inline cleanup blocks**，不使用 `goto label`：在块结尾、`return`、`break`、`continue` 之前，将已注册的 defer 体按 LIFO 顺序内联展开。不产生额外的 goto label，调试器友好。

**不保证执行的情形：** `exit()`、`abort()`、`longjmp()` 等绕过正常控制流的 C 运行时调用，defer 不保证执行——这些是 C 运行时级别的中断，Sharp 无法在翻译层拦截。

Sharp 编译器生成 **C11** 代码（`-std=c11`）。生成的 C 文件可被任意符合 C11 标准的编译器（gcc、clang、msvc）零警告编译。

***

## 运算符重载

**运算符本质上是自由函数。** class 体内的运算符声明是糖，自动把 `this` 作为第一参数。

### 支持的运算符（20 个）

| 类别  | 符号                               | <br />                                            |
| --- | -------------------------------- | :------------------------------------------------ |
| 算术  | `+`  `-`  `*`  `/`  `%`          | <br />                                            |
| 比较  | `==`  `!=`  `<`  `>`  `<=`  `>=` | <br />                                            |
| 位运算 | `&`  \`                          | `  `          ^`  `          <<`  `          >>\` |
| 下标  | `[]`                             | <br />                                            |
| 一元  | `-`（取负） `!`（逻辑非） `~`（按位取反）       | <br />                                            |

**不支持的运算符:** `=`、`+=`、`-=`、`*=`、`/=`、`%=`。

- `=` 始终是 **memcpy**，不可重载，与 C struct 赋值一致。
- 复合赋值用 `a = a + b`，写法清晰，代价可见。

### class 体内形式（糖）

```sharp
class Vec3 {
    float x; float y; float z;

    // 糖：等价于 Vec3 operator+(this, Vec3 rhs) { ... }
    Vec3 operator+(Vec3 rhs) {
        return Vec3 { .x = this->x+rhs.x, .y = this->y+rhs.y, .z = this->z+rhs.z };
    }

    float operator[](long i) const {
        return (&this->x)[i];
    }

    Vec3 operator-() {
        return Vec3 { .x = -this->x, .y = -this->y, .z = -this->z };
    }
}
```

### 自由函数形式

非对称运算符（LHS 不是本类型）必须写自由函数:

```sharp
// 标量 * 向量（LHS 是 float，非 Vec3）
Vec3 operator*(float s, Vec3 v) {
    return Vec3 { .x = s*v.x, .y = s*v.y, .z = s*v.z };
}
```

### 解析规则

表达式 `a ⊕ b` 按以下顺序查找 `operator⊕`:

1. `typeof(a)` 的 class 体内定义的运算符（若第二操作数类型匹配）
2. 全局作用域的自由函数 `operator⊕(typeof(a), typeof(b))`
3. 参数类型允许隐式转换（如 `float` → `double`）的匹配

多个匹配歧义报错。

### 转译到 C

```c
// class Vec3 内 operator+ → 自由函数
Vec3 operator_add__Vec3__Vec3(Vec3 this, Vec3 rhs) { ... }

// 自由函数 operator*(float, Vec3)
Vec3 operator_mul__float__Vec3(float s, Vec3 v) { ... }

// a[i]:
operator_index__Vec3__long(&a, i)
```

***

## 泛型

Sharp 的泛型走**单态化 +** **`@`** **内省**路线。没有 trait，没有 concept，没有编译期虚拟机。

### 设计原则

1. **类型参数是编译期替换**，不是"编译期值"。`<T>` 只是带空位的类型/函数。
2. **约束即代码**。约束不是独立的类型系统特性，它是在泛型体里用 `@static_assert` 显式化，放在函数体开头。
3. **错误推迟到实例化点**（鸭子类型默认），使用者可选地用 `@static_assert` 提前到定义点。
4. **单态化每个实例化独立**，链接期去重。

### 泛型 class

```sharp
class Vec<T> {
    T*   data;
    long len;
    long cap;
}

class Pair<A, B> {
    A first;
    B second;
}
```

实例化: `Vec<int>`、`Pair<int, float>`、`Vec<Pair<int, float>>`。

### 泛型函数

```sharp
void swap<T>(T* a, T* b) {
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

// 调用
int x = 1, y = 2;
swap<int>(&x, &y);
```

类型实参可从参数推断时允许省略: `swap(&x, &y)`。脱糖后等价于显式写 `swap<int>`。

### 泛型 class 方法

方法直接定义在泛型 class 体内，类型参数在方法内直接可用:

```sharp
class Vec<T> {
    T*   data;
    long len;
    long cap;

    Vec<T> new() {
        return Vec<T> { data: NULL, len: 0, cap: 0 };
    }

    void push(T item) {
        if (this->len == this->cap) this->grow();
        this->data[this->len++] = item;
    }

    void grow() {
        long new_cap = this->cap ? this->cap * 2 : 8;
        this->data = realloc(this->data, new_cap * sizeof(T));
        this->cap = new_cap;
    }

    long size() const {
        return this->len;
    }

    T at(long i) const {
        return this->data[i];
    }

    T operator[](long i) const {
        return this->data[i];
    }

    void clear() {
        this->len = 0;
    }

    void destroy() {
        free(this->data);
        this->data = NULL;
        this->len = 0;
        this->cap = 0;
    }
}
```

**容器不管理元素生命周期。** `destroy()` 只释放 data 数组本身。如果 `T` 持有堆资源（如 `Vec<Buffer>`），使用者在调用 `destroy()` 前要自行释放每个元素的资源:

```sharp
Vec<Buffer> files = Vec<Buffer>.new();
defer {
    for (long i = 0; i < files.size(); i++) {
        files[i].destroy();   // 手动释放每个 Buffer 的堆内存
    }
    files.destroy();          // 释放 data 数组
}
```

### 禁止方法特化

Sharp **不允许**针对具体类型实参重新定义泛型 class 的方法。每个泛型 class 的方法集合唯一，所有实例化共用同一套方法定义。类型差异统一在唯一的方法体内处理:

```sharp
// ❌ 不允许：无法为 Vec<int> 单独提供方法
// ❌ 不允许：无法为 Vec<Buffer> 单独提供清理逻辑

// ✅ 正确：所有实例化使用同一个方法体
class Vec<T> {
    void destroy() {
        free(this->data);
    }
}
```

### 约束: 鸭子类型默认 + `@static_assert` 显式检查

**默认（鸭子类型）:** 泛型体里对 `T` 的使用直接展开，错误在实例化点报告。

```sharp
T sum<T>(const Vec<T>* v) {
    T acc = v->at(0);
    for (long i = 1; i < v->size(); i++) acc = acc + v->at(i);  // 要求 T 有 operator+
    return acc;
}

sum<Matrix>(...);  // 若 Matrix 未定义 operator+，错误指向此行
```

**显式检查（推荐）:** 在函数体开头用 `@static_assert` 提前约束:

```sharp
T sum<T>(const Vec<T>* v) {
    @static_assert(@has_operator(T, +), "sum<T>: T 必须支持 operator+");
    T acc = v->at(0);
    for (long i = 1; i < v->size(); i++) acc = acc + v->at(i);
    return acc;
}
```

***

## 编译期内省（`@` 原语）

`@` 前缀是 Sharp 的**内省保留命名空间**。这些原语由编译器内建，返回编译期常量，用于泛型约束。

### 原语集合

| 原语                          | 返回            | 说明                                                                                                         |
| --------------------------- | ------------- | ---------------------------------------------------------------------------------------------------------- |
| `@static_assert(cond, msg)` | —             | 编译期断言，`cond` 必须是编译期常量，失败则中止编译并报 `msg`                                                                      |
| `@has_operator(T, <op>)`    | `bool`        | 类型 `T` 是否在 struct 体内定义了运算符 `<op>`。`<op>` 是运算符 token，如 `+`、`==`、`[]`。**只查 struct 的内部 scope**，自由函数形式的运算符不被检测 |
| `@has_method(T, name)`      | `bool`        | 类型 `T` 是否在 struct 体内定义了名为 `name` 的方法（非运算符）。`@has_operator` 的一般化（v0.13 引入）                                  |
| `@has_field(T, name)`       | `bool`        | 类型 `T` 是否是 struct 且含有名为 `name` 的数据字段（v0.13 引入）                                                              |
| `@type_name(T)`             | `const char*` | 返回 `T` 的 mangled C 名作为 NUL 结尾字符串字面量（v0.13 引入）                                                              |
| `@is_arithmetic(T)`         | `bool`        | `T` 是否是 C11 算术类型（任意整型/浮点/`char`/`bool`）（v0.13 引入）                                                          |
| `@is_pointer(T)`            | `bool`        | `T` 是否是任意形态的指针（v0.13 引入）                                                                                  |

原语集合是**封闭的**。不在上表中的 `@<标识符>` 在表达式位置是语法错误。

### `@has_method(T, name)`

`@has_operator` 的一般化:回答"T 是否是 struct,且其内部 scope 含有名为 `name` 的方法"。

```sharp
@has_method(K, hash)        // K 是否定义了 usize hash() const ?
@has_method(T, destroy)     // T 是否需要清理 ?
@has_method(T, clone)       // T 是否可深拷贝 ?
```

规则:

| 情形 | 返回 |
|------|------|
| `T` 是 struct,struct 体内有名为 `name` 的方法 | `true` |
| `T` 是 struct,但 `name` 是字段而非方法 | `false`（看 `@has_field`） |
| `T` 是 struct,但 `name` 是运算符 (`operator+` 等) | `false`（看 `@has_operator`） |
| `T` 不是 struct（内置类型、指针、enum 等） | `false` |
| 自由函数 `name(T, ...)` 存在 | `false` — 只查 struct 内部 scope |

签名不参与检查 — 名字唯一即为 `true`(Sharp 不允许同名重载,方法名在 struct 内必然唯一)。调用方写错签名时,错误在调用点报。

### `@has_field(T, name)`

回答"T 是否是 struct,且含有名为 `name` 的数据字段(非方法)"。

```sharp
// 任意"切片状"类型
@static_assert(@has_field(T, ptr) && @has_field(T, len),
               "T 必须含有 .ptr 和 .len 字段");
```

规则:

| 情形 | 返回 |
|------|------|
| `T` 是 struct,struct 体内有名为 `name` 的字段 | `true` |
| `T` 是 struct,但 `name` 是方法/运算符 | `false` |
| `T` 不是 struct | `false` |

只查顶层字段,不做递归(嵌套 struct 内的字段查不到)。

### `@type_name(T)`

返回 `T` 的 C mangled 名,类型为 `const char*`,值是 NUL 结尾的字符串字面量,落在生成的 C 代码的 `.rodata` 段。

```sharp
@type_name(int)                    →  "int"
@type_name(Vec<int>)               →  "Vec__int"
@type_name(HashMap<int, Str>)      →  "HashMap__int__Str"
@type_name(const int*)             →  "Pcint"
```

主要用途是把实例化点的类型信息带入诊断:

```sharp
@static_assert(@has_method(K, hash), @type_name(K));
// 若 K = Point 不可哈希,编译错误信息直接打印 "Point"
```

相同类型在同一编译单元内多次 `@type_name` 求值,生成的是**同一个字符串字面量**(指针相等),由 C 编译器的字符串字面量合并保证。

### `@is_arithmetic(T)`

返回 `bool`,`T` 是否是 C11 §6.2.5 定义的"算术类型":

- 整型: `char`/`signed char`/`unsigned char`,所有宽度的 `short`/`int`/`long`/`long long` 与对应 unsigned,`bool`
- 浮点: `float`/`double`/`long double`
- 不包含: 指针(看 `@is_pointer`)、struct、enum、`void`、`void*`

为何不能用 `@has_operator(T, +)` 替代: `@has_operator` 只查 struct 内部 scope,`int + int` 是 C 内置运算符,不在 scope 内。`@is_arithmetic` 填这个洞。

### `@is_pointer(T)`

返回 `bool`,`T` 是否是任意形态的指针:`T*`、`const T*`、`T* const`、`const T* const`、函数指针。

```sharp
@is_pointer(int*)              →  true
@is_pointer(const char* const) →  true
@is_pointer(void (*)(int))     →  true
@is_pointer(int)               →  false
@is_pointer(Vec<int>)          →  false
```

主要用途是把"任意指针"统一当 `usize` 处理(哈希、ID 生成、稀疏映射)。

### 复合使用模式

T1 + T2 原语合起来能用纯用户层代码写出"标量 / 指针 / struct 三路分派"的泛型函数,无需任何专用的内置分派原语:

```sharp
usize hash_any<T>(T val) {
    if (@is_arithmetic(T) || @is_pointer(T)) {
        return splitmix64((usize)val);            // 内置标量/指针走混合
    } else {
        @static_assert(@has_method(T, hash), @type_name(T));
        return val.hash();                         // 用户类型走方法
    }
}
```

三个分支编译期裁剪,生成的 C 代码只剩一条路径。

***

### 语法消歧

`#` 和 `@` 在 Sharp 中各有专属职责，互不重叠:

1. **`#`** **预处理器指令**（沿袭自 C）: 出现在**行首**（可有前导空白），如 `#include`、`#define`、`#if`
2. **`@`** **内省原语**（Sharp 新增）: 出现在**表达式位置**，如 `@has_operator(T, +)`

两套系统视觉上彻底分离。`#if SOME_MACRO` 是预处理，`@has_operator(T, +)` 是编译期查询，扫一眼即可区分。`@` 原语在语言前端的 `if` 语句中求值，不在预处理器层。

预处理器的 `#`（字符串化）和 `##`（拼接）只在 `#define` 替换列表内生效，与 `@` 无关。

### 约束即代码

`@has_operator` 和 `@static_assert` 配合有两种用法：

**约束即错误** — 不满足直接编译失败:

```sharp
// 要求 T 可加
T sum<T>(const Vec<T>* v) {
    @static_assert(@has_operator(T, +), "sum<T>: T 必须支持 operator+");
    ...
}

// 要求 T 可比较
long index_of<T>(const Vec<T>* v, T target) {
    @static_assert(@has_operator(T, ==), "index_of<T>: T 必须支持 operator==");
    for (long i = 0; i < v->size(); i++) {
        if (v->at(i) == target) return i;
    }
    return -1;
}
```

**约束即分支** — 按能力选择两套实现:

```sharp
T sum<T>(const Vec<T>* v) {
    if (@has_operator(T, +)) {
        T acc = v->at(0);
        for (long i = 1; i < v->size(); i++) acc = acc + v->at(i);
        return acc;
    } else {
        return T{};    // T 没有 operator+，提供备用实现
    }
}
```

| 需求              | 写法                                              |
| --------------- | ----------------------------------------------- |
| 约束即错误：不满足直接编译失败 | `@static_assert(@has_operator(T, +), "msg")`    |
| 约束即分支：两套实现按能力选择 | `if (@has_operator(T, +)) { ... } else { ... }` |

### Dead branch 不做类型检查

当 `if` 条件是编译期常量 `@` 原语(任意 `@has_operator`/`@has_method`/`@has_field`/`@is_arithmetic`/`@is_pointer`,以及由它们组合而成的 `&&`/`||`/`!` 表达式) 时，编译器不对 dead branch 做类型检查（只做语法检查），与 C++ `if constexpr` 语义一致：

```sharp
// T = Vec2（有 operator+）时：
//   → 执行 true branch
//   → else 里的 `return T{}` 不做类型检查

// T = Buffer（没有 operator+）时：
//   → 执行 else branch
//   → if body 里的 `acc + v->at(i)` 不做类型检查
//     即使 Buffer 没有 operator+ 也不报错
```

| 条件                             | then branch | else branch |
| ------------------------------ | ----------- | ----------- |
| 运行期表达式                         | 检查          | 检查          |
| 任一 `@` 查询原语为 `true`            | **检查**      | 不检查         |
| 任一 `@` 查询原语为 `false`           | 不检查         | **检查**      |

两个 branch 都必须**语法合法**（能 parse），dead branch 的语义错误被忽略。

### 设计边界

Sharp 的 `@` **不是** Zig 的 `comptime`:

- 不支持在编译期执行任意使用者代码
- 不支持编译期 I/O、编译期循环
- `@` 原语只做**查询**（返回常量），不做**执行**
- 原语集合**封闭可枚举**(当前 7 个) — 不开放用户扩展

**求值时机：** 所有查询原语在**泛型实例化阶段**求值——即类型参数被具体类型绑定之后。在泛型定义体内、类型参数尚未确定时，原语不立即求值，而是随每次实例化各自独立计算。这是"约束即代码"能工作的基础：同一个泛型函数体对不同的实参类型可以走不同的分支。

***

## 预处理器 (Preprocessor)

Sharp 内置一个完整的 C11 预处理器（`cpp/` 模块），在词法分析之前运行。它与 C 预处理器完全兼容。Sharp 使用 `#include` 引入其他 Sharp 源文件（`.sp` / `.sph`），预处理器在所有源文件中统一工作，宏行为与 C 完全一致。

### 翻译阶段

按 ISO C11 §5.1.1.2 执行以下六个阶段:

| 阶段 | 操作                              |
| -- | ------------------------------- |
| 1  | 三字符序列替换（trigraph，`??=` → `#` 等） |
| 2  | 反斜杠-换行拼接（`\` + `\n` 删除）         |
| 3  | 分词为预处理令牌（pp-token）              |
| 4  | 指令执行 + 宏展开                      |
| 5  | 字符/字符串编码转换                      |
| 6  | 相邻字符串字面量拼接                      |

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

### 内置宏

| 宏                   | 值                   | 说明                     |
| ------------------- | ------------------- | ---------------------- |
| `__FILE__`          | `"path/to/file.sp"` | 当前源文件路径                |
| `__LINE__`          | 整数                  | 当前源文件行号                |
| `__COUNTER__`       | 0, 1, 2, …          | 每次展开自增，全局唯一序列          |
| `__DATE__`          | `"Jan  1 2025"`     | 编译日期                   |
| `__TIME__`          | `"12:00:00"`        | 编译时间                   |
| `__STDC__`          | `1`                 | 符合 C11 标准              |
| `__STDC_VERSION__`  | `201112L`           | C11 版本号                |
| `__STDC_HOSTED__`   | `1`                 | 托管实现                   |
| `__SHARP__`         | `1`                 | Sharp 语言模式（仅 `.sp` 文件） |
| `__SHARP_VERSION__` | `"1.0"`            | Sharp 版本字符串            |

### `#if` 常量表达式

支持完整的 C11 整型常量表达式:

```
算术:  + - * / %
位运算: << >> & | ^ ~
逻辑:  && || !
关系:  == != < > <= >=
三目:  cond ? then : else
字符常量: 'a'  '\n'  '\xff'
整数字面量: 十进制 / 0x十六进制 / 0八进制 / 0b二进制
defined(X): X 已定义则为 1，否则为 0
```

所有运算以 `intmax_t`（至少 64 位）执行；除以零报错而非未定义行为。

### 与 C 的差异和扩展

| 特性              | Sharp 行为                                                                        |
| --------------- | ------------------------------------------------------------------------------- |
| `#pragma once`  | 原生支持，优于手写 include guard                                                         |
| `__VA_OPT__`    | 支持（C23 特性，Sharp 提前引入）                                                           |
| `#include_next` | 支持（GCC 扩展）                                                                      |
| `__COUNTER__`   | 支持（GCC 扩展）                                                                      |
| `#warning`      | 支持（GCC 扩展）                                                                      |
| 行标记（linemarker） | 预处理器输出 `# N "file"` 行，词法分析器吸收并更新位置信息                                            |
| 三字符序列           | 支持但默认不警告（纯 C 兼容需要）                                                              |
| `@` 内省原语        | 表达式位置的 `@ident(...)` 不被预处理器处理，直接传递给语言前端；编译期分支用 `if (@has_operator(...))` 在语言层处理 |

### 命令行选项

```
# 预处理器选项（gcc 兼容）
sharpc  -DNAME[=value]          # 预定义宏（等价于 #define）
sharpc  -UNAME                  # 预取消定义
sharpc  -Idir                   # 添加用户 include 搜索路径
sharpc  -isystem dir            # 添加系统 include 搜索路径
sharpc  -std=c11|c99|gnu11|...  # C 标准版本

# 输出控制
sharpc  -o output.c             # 指定输出文件（默认 stdout）

# 独立预处理器
sharp-cpp                       # 独立预处理器（与 sharpc 共享同一库）
sharp-cpp -P                    # 抑制行标记输出
sharp-cpp --sharp               # Sharp 语言模式（定义 __SHARP__）
sharp-cpp --tokens              # 将令牌列表输出到 stderr（调试用）
```

***

## 综合示例

把 struct 方法 / defer / 运算符 / 泛型 / `@` 内省串起来：

```sharp
#include <stdlib.h>   // malloc, free, realloc
#include <string.h>   // memcmp, strlen
#include <stdio.h>    // printf

// ============================================================
// Vec<T> — 泛型动态数组
// 注意：destroy() 只释放 data 数组本身，不管元素生命周期。
// 如果 T 持有堆资源（如 Vec<Buffer>），使用者在 destroy() 前自行清理元素。
// ============================================================
class Vec<T> {
    T*   data;
    long len;
    long cap;

    Vec<T> new() {
        return Vec<T> { data: NULL, len: 0, cap: 0 };
    }

    long size() const {
        return this->len;
    }

    T at(long i) const {
        return this->data[i];
    }

    T operator[](long i) const {
        return this->data[i];
    }

    void push(T item) {
        if (this->len == this->cap) this->grow();
        this->data[this->len++] = item;
    }

    void grow() {
        long new_cap = this->cap ? this->cap * 2 : 8;
        this->data = realloc(this->data, new_cap * sizeof(T));
        this->cap = new_cap;
    }

    void clear() {
        this->len = 0;
    }

    void destroy() {
        free(this->data);
        this->data = NULL;
        this->len = 0;
        this->cap = 0;
    }
}

// ============================================================
// Str — 字符串切片: (const char*, long)，纯值类型
// ============================================================
class Str {
    const char* data;
    long        len;

    Str from_cstr(const char* s) {
        return Str { data: s, len: strlen(s) };
    }

    long size() const {
        return this->len;
    }

    char operator[](long i) const {
        return this->data[i];
    }

    bool operator==(Str rhs) const {
        if (this->len != rhs.len) return false;
        return memcmp(this->data, rhs.data, this->len) == 0;
    }
}

// ============================================================
// Buffer — 持有堆内存的类型，提供 destroy()
// ============================================================
struct Buffer {
    char* ptr;
    long  len;

    Buffer alloc(long n) {
        return Buffer { ptr: malloc(n), len: n };
    }

    long size() const {
        return this->len;
    }

    char at(long i) const {
        return this->ptr[i];
    }

    void destroy() {
        free(this->ptr);
        this->ptr = NULL;
        this->len = 0;
    }
}

// ============================================================
// Vec3 — 纯值类型，运算符重载
// ============================================================
class Vec3 {
    float x;
    float y;
    float z;

    Vec3 operator+(Vec3 rhs) {
        return Vec3 { .x = this->x + rhs.x,
                      .y = this->y + rhs.y,
                      .z = this->z + rhs.z };
    }

    Vec3 operator-(Vec3 rhs) {
        return Vec3 { .x = this->x - rhs.x,
                      .y = this->y - rhs.y,
                      .z = this->z - rhs.z };
    }

    float operator[](long i) const {
        return (&this->x)[i];
    }
}

// 非对称：LHS 是 float，只能写自由函数
Vec3 operator*(float s, Vec3 v) {
    return Vec3 { .x = s * v.x, .y = s * v.y, .z = s * v.z };
}

// ============================================================
// 泛型函数，约束 T 可加
// ============================================================
T vec_sum<T>(const Vec<T>* v) {
    @static_assert(@has_operator(T, +), "vec_sum<T>: T 必须支持 operator+");
    T acc = v->at(0);
    for (long i = 1; i < v->size(); i++) acc = acc + v->at(i);
    return acc;
}

// ============================================================
// 使用
// ============================================================
void main() {
    // --- Vec<int>：纯值类型，defer 直接 destroy ---
    Vec<int> xs = Vec<int>.new();
    defer xs.destroy();

    xs.push(1); xs.push(2); xs.push(3);
    int total = vec_sum(&xs);
    printf("sum = %d\n", total);

    // --- Vec<Vec3>：纯值类型 ---
    Vec<Vec3> points = Vec<Vec3>.new();
    defer points.destroy();

    points.push(Vec3 { .x = 1, .y = 2, .z = 3 });
    points.push(Vec3 { .x = 4, .y = 5, .z = 6 });
    Vec3 u = points[0] + points[1];
    printf("u = (%.1f, %.1f, %.1f)\n", u.x, u.y, u.z);

    // --- Vec<Buffer>：元素持有堆内存，自己负责清理 ---
    Vec<Buffer> files = Vec<Buffer>.new();
    defer {
        for (long i = 0; i < files.size(); i++) {
            files[i].destroy();     // 手动清理每个 Buffer
        }
        files.destroy();            // 释放 data 数组
    }

    Buffer a = Buffer.alloc(1024);
    files.push(a);
    Buffer b = Buffer.alloc(2048);
    files.push(b);

    for (long i = 0; i < files.size(); i++) {
        printf("  [%d]: %ld bytes\n", i, files[i].len);
    }
    // defer 块逆序执行: files.destroy() → a/b.destroy()
}
```

