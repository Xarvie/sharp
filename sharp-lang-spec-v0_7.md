# Sharp 语言规格 v0.7

> C++ Lite · 完整 C11 + struct + impl + 泛型 + RAII · 转译到 C11 · 零运行时

---

每个特性必须通过两个验证:
1. C 无法干净表达
2. 转译到 C 后零开销

**Sharp 是什么:** C + Rust 风格的 `impl` + C++ 风格的模板 + 确定性析构 + `#` 编译期内省。

---

## 语义契约

### 值语义(和 C 完全一致)

结构体赋值、值传参、函数返回,全部是 **memcpy**,和 C 的 struct 赋值完全一致:

```sharp
Buffer a = Buffer.alloc(1024);
Buffer b = a;   // memcpy:b.ptr == a.ptr,两者共享同一块内存
```

如果一个类型独占某个资源,使用者有责任不对它做值拷贝。用 `T*` 传指针,或显式定义转移语义(置空原对象)。**Sharp 不追踪所有权。**

### RAII 契约

Sharp 保证:

- 每个持有析构函数的**局部变量**(包括值形参)在离开其作用域时,析构函数被调用恰好一次
- 析构函数以声明的**逆序**触发
- `return` / `break` / `continue` 会沿路径展开所有在作用域内的析构函数

Sharp 不保证:

- 析构函数只被调用一次(值拷贝后两份都会析构)
- 资源不被双重释放(这是使用者的责任)

### 泛型契约

Sharp 保证:

- 每个泛型实例化生成独立的 C 代码(单态化,monomorphization)
- 相同类型实参的实例化在全程序内**符号唯一**,由链接器去重(weak / COMDAT)
- 泛型体在所有实例化点可见时,错误在**实例化点**报告
- `#` 内省原语返回编译期常量,不产生运行时开销
- impl 选择在实例化点按 partial ordering 唯一决定,歧义即编译错误

Sharp 不保证:

- 泛型代码量上限(单态化天然会膨胀,使用者可用 `extern impl` 手动控制;const 不作为泛型类型实参,不额外引入膨胀)
- 跨项目 impl 的唯一性(见"孤儿规则")

---

## 语言核心

| 特性 | 理由 |
|------|------|
| `struct` + `impl` | C 有 struct 但无方法绑定;`impl` 是析构/运算符/方法的统一容器 |
| 析构函数 `~this()` + RAII | 确定性资源管理(见契约) |
| 运算符重载 | 数学/向量代码必要;本质是自由函数,`impl` 内为糖 |
| 泛型 `<T>` + 单态化 | C 只能靠宏模拟 |
| `#` 内省原语 | 泛型约束与编译期查询的统一机制 |

---

## 类型系统

Sharp 使用独立的类型名,与 C 语言的类型名不同。所有 Sharp 类型在代码生成时会被映射为对应的 C 标准类型。

### 整型类型

Sharp 使用与 C 一致的类型系统。

| 类型 | 说明 |
|------|------|
| `char` | 8 位字符/整数 |
| `short` | 16 位短整数 |
| `int` | 32 位整数 |
| `long` | 32/64 位长整数（取决于平台） |
| `long long` | 64 位长整数 |

### 浮点类型

| 类型 | 说明 |
|------|------|
| `float` | 32 位浮点数 |
| `double` | 64 位浮点数 |

### 其他类型

| 类型 | 说明 |
|------|------|
| `bool` | 布尔类型（C 标准类型） |
| `void` | 空类型 |

---

## `const` 类型修饰符

`const` 是**类型限定符**,语义与 C 完全一致。`const` 是 Sharp 类型系统的一等部分,参与类型匹配、赋值检查和函数签名。

#### 基本规则

1. **`const T` 和 `T` 是不同类型**,但支持单向的限定转换:
   - `T` → `const T`:隐式合法(加限定)
   - `const T` → `T`:编译错误(丢弃限定)
2. **只读对象:** `const T x = expr;` 声明后 `x` 不能再被赋值。绕路通过指针写 `const` 对象是未定义行为。
3. **顶层 const 不参与函数类型匹配**(与 C 一致):`void f(const int x)` 与 `void f(int x)` 是同一函数类型。这条只对**顶层**参数成立;参数中的 `const T*` 仍然参与匹配。
4. **`const T x = expr;` 不是编译期常量**,与 C 一致。需要编译期整数常量时使用 `enum`。
5. **const 不能作为泛型类型实参的顶层限定。** `Vec<const int>` 非法,因为 const 值类型不可赋值,与 C++ `std::vector<const T>` 规则一致。`Vec<const int*>` 合法(const 在 pointee 上,非顶层)。

#### 指针的三种组合

| 写法 | 含义 |
|------|------|
| `const T* p` | 指向 const T 的指针;`*p = ...` 非法 |
| `T* const p` | const 指针指向 T;`p = ...` 非法,`*p = ...` 合法 |
| `const T* const p` | 两者皆 const |

与 C 完全一致。

#### const 与泛型

const **不能**作为泛型类型实参的**顶层**限定:

```sp
Vec<const char*> strings;          // ✅ const 在 pointee 上,非顶层
const Vec<int> v = make_vec();   // ✅ const 在变量上,非类型实参

Vec<const int> v1;               // ❌ const 修饰值类型,顶层 const
Vec<int* const> v2;              // ❌ const 修饰指针本身,顶层 const
```

顶层 const 的泛型类型实参在语义上无意义且会导致不可赋值,
与 C++ `std::vector<const T>` 编译失败的规则一致。

**判定方法:** 检查类型实参的 `ty_is_const()` 返回值。

| 类型实参 | `ty_is_const()` | 合法? |
|----------|----------------|-------|
| `const int` | true | ❌ |
| `const int*` | false (const 在 base) | ✅ |
| `int* const` | true | ❌ |
| `const Point` | true | ❌ |
| `Point*` | false | ✅ |

实例化时 `const T*` 和 `T*` 视为不同类型(const 在 pointee 上),
各自生成独立的实例化。

#### const 与方法

方法可声明 const 后缀,承诺不修改 `*this`:

```sp
impl<T> Vec<T> {
    long len() const {           // const 方法:this 类型为 const Self*
        return this->len;
    }

    T at(long i) const {          // const 方法
        return this->data[i];
    }

    void push(T item) {            // 非 const:this 类型为 Self*
        ...
    }
}
```

调用规则:
- **const 方法:** `this` 类型为 `const Self*`。可用于 const 或非 const 接收者
- **非 const 方法:** `this` 类型为 `Self*`。仅可用于非 const 接收者;在 const 接收者上调用是编译错误
- const 方法体内不能修改 `this->...`,不能调用非 const 方法

#### const 与运算符

`impl` 内的运算符按其脱糖形式决定能否作用于 const 接收者:

| 运算符类别 | 脱糖形式 | const 接收者可用? |
|------------|----------|-------------------|
| 纯值类(`+`、`-`、`*`、`==`、一元 `-` 等) | `operator<op>(Self this, ...)` 值传 | ✅ |
| 复合赋值(`+=`、`-=` 等) | `operator<op>(Self* this, ...)` | ❌ |
| 下标(`[]`)读取形式 | `operator[](const Self* this, long)` | ✅ |
| 下标(`[]`)写入形式 | `operator[](Self* this, long)` | ❌ |

使用者可以同时提供 const 和非 const 两个重载,编译器按接收者限定分派。

#### const 与析构

析构函数 `~this()` **允许在 const 实例上调用**,即使析构函数本身修改资源(如 `free` 内部指针)。与 C++ 一致:销毁对象的生命周期不是"修改"。

#### 代码生成

`const` 完整保留到生成的 C 代码中。类型匹配、赋值检查、const 方法分派在 Sharp 层完成;最终生成的 C 代码中 `const` 按原位置保留,C 编译器做最后把关。

const **不**导致泛型实例化膨胀:顶层 const 的类型实参被拒绝;`const Vec<T>`(const 在变量上)复用同一实例化;仅 `const T*`(const 在 pointee 上)产生独立实例化,这是有意为之——`Vec<const char*>` 和 `Vec<char*>` 确实需要不同的行为。

Sharp 层的 const 错误信息直接映射回 Sharp 源码行号,不依赖 C 编译器报错。

---

## impl 块

`impl` 是 Sharp 的**能力扩展容器**。它把属于某个类型的操作——方法、析构、运算符——集中到一个命名空间。

### 基本语法

```sharp
struct Buffer {
    char*   ptr;
    long len;
}

impl Buffer {
    // 关联函数(无 this,类似静态方法)
    Buffer alloc(long n) {
        return Buffer { ptr: malloc(n), len: n };
    }

    // 方法(有 this,this 类型为 Self*)
    char get(long i) {
        return this->ptr[i];
    }

    // 析构函数
    ~this() {
        free(this->ptr);
    }
}
```

### `Self` 类型别名

`impl` 块内部,`Self` 指代被 impl 的类型(包括泛型实参):

```sharp
impl<T> Vec<T> {
    Self new() {
        return Self { data: null, len: 0, cap: 0 };
    }
    // Self 在此 impl 内等于 Vec<T>
}
```

### 语义规则

- `impl T { ... }` 里声明的函数属于 `T` 的命名空间
- 方法的第一个隐式参数是 `this`:非 const 方法中类型为 `Self*`,const 方法(带 `const` 后缀)中类型为 `const Self*`
- 关联函数(无 `this`)通过 `T.func(...)` 调用;方法通过 `obj.method(...)` 调用
- const 接收者只能调用 const 方法;非 const 接收者可调用任一
- 同一类型可以有多个 `impl` 块,内容合并(受 ODR 约束)
- `impl` 块可以出现在任何 `.sp` / `.sph` 文件中;定义必须对使用点通过 `#include` 可见

### 禁止形式

以下形式在语法上不合法,编译器必须拒绝:

```sharp
impl<T> T { ... }        // ❌ 全局 blanket impl
```

`impl` 的目标类型必须是具体类型构造器的应用,不能是裸类型参数。这防止命名污染、编译顺序依赖、跨 TU 行为不一致。

### 孤儿规则

Sharp 无专用模块系统。跨项目 impl 冲突通过 ODR + 链接期符号检测保证:

**规则 O-1(ODR):** 同一名字改编后的 impl 符号在多个翻译单元中出现时,所有定义必须逐字节一致。违反此规则是未定义行为,典型表现为链接冲突或运行时错误。与 C++ 模板 ODR 一致。

**规则 O-2(约定):** 库作者只为本库声明的类型提供 impl。为他人声明的类型编写 impl 即使通过编译也可能在集成时引发链接冲突。

**非强制** 的编译期警告:若 `impl` 的目标类型及其所有类型实参的主声明都不在当前翻译单元的"本项目"源文件集合内,编译器发出 `orphan-impl` 警告(可用 `-Wno-orphan-impl` 关闭)。

### 调用语法

```sharp
Buffer b = Buffer.alloc(1024);   // 关联函数
char x = b.get(0);                 // 方法
// b 离开作用域时自动调用 ~this()
```

### 转译到 C

```c
// impl Buffer { Buffer alloc(long n) { ... } }
Buffer Buffer__alloc(long n) { ... }

// impl Buffer { char get(long i) { ... } }
char Buffer__get(Buffer* this, long i) { ... }

// b.get(0) 转译为:
Buffer__get(&b, 0);
```

---

## 析构函数

析构函数 `~this()` 定义在 `impl` 块内,在变量离开作用域时自动调用。

### 规则

- 每个类型最多一个析构函数
- 析构函数无返回值、无参数(除隐式 `this`,类型为 `Self*`)
- 析构函数的调用点由编译器插入,**不能手动调用**
- 若类型 `T` 无析构函数,`~T` 在泛型上下文中展开为 no-op(编译期擦除)

### 作用域与触发时机

```sharp
void example() {
    Buffer a = Buffer.alloc(64);    // 构造
    Buffer b = Buffer.alloc(128);   // 构造

    if (err) {
        return;                     // 触发 ~b() 然后 ~a()
    }

    // 正常路径:函数结束时触发 ~b() 然后 ~a()
}
```

**析构顺序:** 与声明顺序相反(LIFO)。

**提前返回:** `return` / `break` / `continue` 自动展开沿途所有在作用域内的析构。

### 值拷贝不触发析构

```sharp
Buffer a = Buffer.alloc(64);
Buffer b = a;   // memcpy。a 和 b 共享 ptr。
// 函数结束时 ~b() 和 ~a() 都会被调用 —— 双重 free!
```

这是**使用者的责任**。Sharp 不追踪所有权,不做写时复制,不做借用检查。需要独占语义时,用指针或显式的 move 函数置空原对象。

---

## 运算符重载

**运算符本质上是自由函数。** `impl` 块内的运算符声明是糖,自动把 `this` 作为第一参数。

### 支持的运算符

| 类别 | 符号 |
|------|------|
| 算术 | `+`  `-`  `*`  `/`  `%` |
| 复合赋值 | `+=`  `-=`  `*=`  `/=`  `%=` |
| 比较 | `==`  `!=`  `<`  `>`  `<=`  `>=` |
| 位运算 | `&`  `|`  `^`  `<<`  `>>` |
| 下标 | `[]` |
| 一元 | `-`(取负) `!`(逻辑非) `~`(按位取反,**不是**析构) |

### 自由函数形式

```sharp
struct Vec3 { float x; float y; float z; }

// Vec3 + Vec3
Vec3 operator+(Vec3 a, Vec3 b) {
    return Vec3 { x: a.x+b.x, y: a.y+b.y, z: a.z+b.z };
}

// 标量 * 向量(非对称,LHS 不是 Vec3)
Vec3 operator*(float s, Vec3 v) {
    return Vec3 { x: s*v.x, y: s*v.y, z: s*v.z };
}

// 向量 * 标量
Vec3 operator*(Vec3 v, float s) {
    return Vec3 { x: v.x*s, y: v.y*s, z: v.z*s };
}
```

### `impl` 内形式(糖)

在 `impl T { ... }` 内声明的运算符,第一参数隐式为 `this`(类型 `Self*`,但调用时传 `Self`):

```sharp
impl Vec3 {
    // 糖:等价于 Vec3 operator+(Vec3 this, Vec3 rhs) { ... }
    Vec3 operator+(Vec3 rhs) {
        return Vec3 { x: this.x+rhs.x, y: this.y+rhs.y, z: this.z+rhs.z };
    }

    void operator+=(Vec3 rhs) {
        this->x += rhs.x; this->y += rhs.y; this->z += rhs.z;
    }

    float operator[](long i) {
        return (&this->x)[i];
    }

    Vec3 operator-() {
        return Vec3 { x: -this.x, y: -this.y, z: -this.z };
    }
}
```

**糖规则:** `impl T` 内的 `R operator<op>(Args)` 脱糖为 `R operator<op>(T this, Args)`(值传 `this`)。带 `const` 后缀时脱糖为 `R operator<op>(const T* this, Args)`。复合赋值类(`+=` 等)和 `[]` 的写入形式必须修改 `*this`,脱糖为 `R operator<op>(T* this, Args)`;该形式不能带 `const` 后缀。

const 接收者与运算符的详细对应见 "`const` 类型修饰符" 章节。

### 解析规则

表达式 `a ⊕ b` 按以下顺序查找 `operator⊕`:

1. `impl typeof(a)` 内的方法形式(若第二操作数类型匹配)
2. 全局作用域的自由函数 `operator⊕(typeof(a), typeof(b))`
3. 参数类型允许隐式转换(如 `float` → `double`)的匹配

多个匹配按 partial ordering 选最具体;歧义报错。

### 孤儿规则的应用

自由函数运算符受孤儿规则约束:**至少一个操作数类型的主 `struct` 声明应位于本项目**。违反此约定时编译器发出 `orphan-impl` 警告。

### 与析构的区分

`~this()` 是析构函数,不是 `operator~` 的定义。如需按位取反运算符,写 `operator~(T)`。Sharp 在语法层通过"是否带 `this` 前缀参数/是否空参"区分二者。

### 转译到 C

```c
// Vec3 + Vec3 (自由函数) 转译为:
operator_add__Vec3__Vec3(a, b)

// impl Vec3 { operator+ } 也转译为同一符号,因为脱糖后等价:
operator_add__Vec3__Vec3(a, b)

// a += b:
operator_add_assign__Vec3__Vec3(&a, b)

// a[i]:
operator_index__Vec3__long(&a, i)
```

---

## 泛型

Sharp 的泛型走**单态化 + impl 约束 + `#` 内省**路线。没有 trait,没有 concept,没有编译期虚拟机。

### 设计原则

1. **类型参数是编译期替换**,不是"编译期值"。Sharp 的类型系统和 C 的心智模型一致,`<T>` 只是带空位的类型/函数。
2. **约束即代码**。约束不是独立的类型系统特性,它是在泛型体里用 `#` 原语问编译器问题,用 `#static_assert` 或 `where` 子句显式化。
3. **错误推迟到实例化点**(鸭子类型默认),使用者可选地用 `#static_assert` 提前到定义点。
4. **单态化每个实例化独立**,链接期去重。

### 泛型结构体

```sharp
struct Vec<T> {
    T*    data;
    long len;
    long cap;
}

struct Pair<A, B> {
    A first;
    B second;
}
```

实例化:`Vec<int>`, `Pair<int, float>`, `Vec<Pair<int, float>>`。

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

类型实参可从参数推断时允许省略:`swap(&x, &y)`。脱糖后等价于显式写 `swap<int>`。

### 泛型 impl

```sharp
impl<T> Vec<T> {
    Self new() {
        return Self { data: null, len: 0, cap: 0 };
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

    ~this() {
        for (long i = 0; i < this->len; i++)
            ~T(&this->data[i]);     // 若 T 无析构,编译期擦除
        free(this->data);
    }
}
```

### impl 特化与 Partial Ordering

实例化 `T<X₁, ..., Xₙ>` 时,编译器从所有匹配的 `impl` 中选择**严格最具体**的一个。

**匹配定义:** `impl [<P,...>] T<A₁, ..., Aₙ>` 匹配实例化 `T<X₁, ..., Xₙ>`,当且仅当存在对类型参数 `P,...` 的替换 σ 使得 `σ(Aᵢ) ≡ Xᵢ` 对所有 i 成立。

**更具体定义:** impl `A` 比 impl `B` 更具体,当且仅当存在替换 τ 使得 `A` 的参数模式等于将 τ 作用于 `B` 的参数模式的结果,而反向替换不存在。换言之,`A` 的模式是 `B` 的模式的严格特化。

**选择规则:**
1. 若存在唯一最具体 impl,使用它。
2. 若存在多个最具体 impl 互不可比较(ambiguity),**在实例化点报编译错误** `ambiguous impl`。
3. 若无匹配 impl,**在实例化点报编译错误** `no matching impl`。

**示例:**

```sharp
impl<T> Vec<T>          { void clear() { ... } }   // (A) 泛型
impl    Vec<int>        { void clear() { ... } }   // (B) 具体,比 A 严格更具体

Vec<int> v;
v.clear();   // 选 B,B 严格最具体
```

```sharp
impl<T> Pair<T, int>    { ... }   // (A)
impl<T> Pair<int, T>    { ... }   // (B)

Pair<int, int> p;   // 错误:A 和 B 都匹配,互不包含 → ambiguous impl
Pair<float, int> q;   // OK:仅 A 匹配
Pair<int, float> r;   // OK:仅 B 匹配
```

**消歧:** 处理 ambiguity 案例须显式提供完全具体的 impl:

```sharp
impl Pair<int, int>     { ... }   // (C) 严格最具体
Pair<int, int> p;                  // OK:选 C
```

### 约束:鸭子类型默认 + 显式检查可选

**默认(鸭子类型):** 泛型体里对 `T` 的使用直接展开,错误在实例化点报告。

```sharp
T sum<T>(T* xs, long n) {
    T acc = xs[0];
    for (long i = 1; i < n; i++) acc = acc + xs[i];   // 要求 T 有 operator+
    return acc;
}

sum<Matrix>(...);   // 若 Matrix 未定义 operator+,错误指向此行
```

**显式检查(推荐):** 用 `#static_assert` 在定义点提前检查。

```sharp
T sum<T>(T* xs, long n) {
    #static_assert(#has_operator(T, +), "sum<T> requires T to implement operator+");
    T acc = xs[0];
    for (long i = 1; i < n; i++) acc = acc + xs[i];
    return acc;
}
```

**`where` 子句(糖):** 把 `#static_assert` 提到签名层,增强文档性。

```sharp
T sum<T>(T* xs, long n) where #has_operator(T, +) {
    T acc = xs[0];
    for (long i = 1; i < n; i++) acc = acc + xs[i];
    return acc;
}
```

`where P` 语义上等价于函数体开头的 `#static_assert(P, "where clause violated")`。**纯糖,无独立语义。**

### 显式实例化与分离编译

泛型定义必须对实例化点可见,放头文件(`.sph`)。

代码膨胀控制:

```sharp
// lib.sp:实例化 Vec<int> 到此翻译单元
impl Vec<int>;

// user.sp:声明 Vec<int> 在别处实例化,本 TU 不生成代码
extern impl Vec<int>;
```

行为与 C++ `extern template` 相同。

### 名字改编

```
Vec<int>                   → Vec__int
Pair<int, float>             → Pair__int__float
Vec<Pair<int, float>>        → Vec__Pair__int__float
Vec<int>::push             → Vec__int__push
operator+ (Vec3, Vec3)     → operator_add__Vec3__Vec3
operator[] (Vec<int>, long) → operator_index__Vec__int__long
```

规则:
- `__` 分隔层级,嵌套按深度优先展开
- 运算符符号映射为可读名(`+` → `add`, `-` → `sub`, `*` → `mul`, `/` → `div`, `%` → `mod`, `==` → `eq`, `!=` → `ne`, `<` → `lt`, `>` → `gt`, `<=` → `le`, `>=` → `ge`, `&` → `band`, `|` → `bor`, `^` → `bxor`, `<<` → `shl`, `>>` → `shr`, `[]` → `index`, 复合赋值加 `_assign` 后缀)

人类可读,调试器友好,可逆。

---

## 编译期内省 (`#` 原语)

`#` 前缀是 Sharp 的**内省保留命名空间**。这些原语由编译器内建,返回编译期常量或类型,用于泛型约束和元编程。

### 原语集合

| 原语 | 返回 | 说明 |
|------|------|------|
| `#has_operator(T, <op>)` | `bool` | 类型 `T` 是否定义了运算符 `<op>`。`<op>` 是运算符 token,如 `+`、`==`、`[]` |
| `#has_method(T, name)` | `bool` | 类型 `T` 是否定义了名为 `name` 的方法。`name` 是标识符 |
| `#has_destructor(T)` | `bool` | 类型 `T` 是否定义了析构函数 |
| `#typeof(expr)` | type | 表达式的类型,用于类型推断与别名 |
| `#is_same(T, U)` | `bool` | 类型相等判断(完全结构相等) |
| `#static_assert(cond, msg)` | — | 编译期断言,`cond` 必须是编译期常量,失败则中止编译并报 `msg` |

原语集合是**封闭的**。不在此表中的 `#<标识符>` 在表达式位置是语法错误。

### 语法消歧

`#` 在 Sharp 中有两个角色:

1. **预处理器指令**(沿袭自 C):出现在**行首**(可有前导空白),如 `#include`、`#define`、`#if`
2. **内省原语**(Sharp 新增):出现在**表达式位置**,如 `#has_operator(T, +)`

词法/语法分析根据位置区分:
- 逻辑行的第一个非空白令牌是 `#` → 预处理器指令
- 其他位置的 `#` → 内省原语(后必须紧跟原语表中的标识符和 `(`)

预处理器的 `#`(字符串化)和 `##`(拼接)只在 `#define` 替换列表内生效,不影响表达式上下文。

### 约束即代码

`#` 原语返回普通布尔值/类型,可以任意组合在 `where` 子句或 `#static_assert` 中:

```sharp
// 要求 T 可加且尺寸 ≤ 16 字节
T fast_sum<T>(T* xs, long n)
    where #has_operator(T, +) && sizeof(T) <= 16
{ ... }

// 要求两个类型相同
void copy_same<A, B>(A* dst, B* src)
    where #is_same(A, B)
{ *dst = *src; }

// 要求 T 有 push 方法
void append_all<C, T>(C* container, T* items, long n)
    where #has_method(C, push)
{
    for (long i = 0; i < n; i++) container->push(items[i]);
}
```

### 条件编译析构

`#has_destructor` 支持在泛型代码里条件性地调用析构:

```sharp
void destroy_n<T>(T* xs, long n) {
    #if #has_destructor(T)
        for (long i = 0; i < n; i++) ~T(&xs[i]);
    #endif
}
```

或更简洁地,直接用 `~T(&xs[i])`——若 `T` 无析构,编译期擦除为 no-op。两种形式等价。

### 保留规则

- **所有 `#<标识符>(...)` 在表达式位置的形式由编译器保留**。使用者不能定义同名符号作为变量、函数、类型。
- 原语集合由语言规范定义。编译器拒绝无法识别的 `#<标识符>`。

### 设计边界

Sharp 的 `#` **不是** Zig 的 `comptime`:

- 不支持在编译期执行任意使用者代码
- 不支持编译期 I/O、编译期循环执行使用者逻辑
- `#` 原语只做**查询**(返回常量),不做**执行**

需要编译期复杂逻辑时,用宏(预处理器)或外部代码生成工具。

---

## 预处理器 (Preprocessor)

Sharp 内置一个完整的 C11 预处理器(`cpp/` 模块),在词法分析之前运行。
它与 C 预处理器完全兼容,并增加了若干 Sharp 专属扩展。

### 翻译阶段

按 ISO C11 §5.1.1.2 执行以下六个阶段:

| 阶段 | 操作 |
|------|------|
| 1 | 三字符序列替换(trigraph,`??=` → `#` 等) |
| 2 | 反斜杠-换行拼接(`\` + `\n` 删除) |
| 3 | 分词为预处理令牌(pp-token) |
| 4 | 指令执行 + 宏展开 |
| 5 | 字符/字符串编码转换 |
| 6 | 相邻字符串字面量拼接 |

### 支持的指令

```
#include "file"          用户头文件(相对当前文件目录搜索)
#include <file>          系统头文件(系统路径搜索)
#include_next "file"     从当前文件所在目录的下一个匹配项开始搜索

#define NAME             定义空宏
#define NAME value       对象式宏
#define NAME(p1,p2) body 函数式宏
#define NAME(p,...) body 可变参数宏(__VA_ARGS__ / __VA_OPT__)
#undef  NAME             取消定义

#if   expr               常量表达式条件(支持 defined()、算术、位运算、三目)
#ifdef  NAME             等价于 #if defined(NAME)
#ifndef NAME             等价于 #if !defined(NAME)
#elif expr               else-if 分支
#else                    否则分支
#endif                   结束条件块

#line N                  设置当前行号为 N
#line N "file"           同时设置文件名

#error  message          触发编译错误并中止
#warning message         触发警告(继续编译)

#pragma once             保证当前头文件只被包含一次
#pragma <other>          其他 pragma 透传到输出;可注册自定义回调处理
```

`#if` 条件中支持 Sharp 内省原语作为常量表达式子项:`#if #has_destructor(T)`(仅在泛型上下文内有意义)。

### 宏展开规则

- **对象式宏**:展开时递归替换,但对已展开的宏名施加蓝色标记(blue-painting),防止无限递归
- **函数式宏**:参数先展开,再代入替换列表;`#` 操作符将参数字符串化;`##` 操作符拼接相邻令牌
- **可变参数**:`__VA_ARGS__` 接收 `...` 实参;`__VA_OPT__(tokens)` 仅在 `__VA_ARGS__` 非空时展开
- **`defined(X)`**:在宏展开之前求值,避免参数被意外展开

### 内置宏

| 宏 | 值 | 说明 |
|----|----|------|
| `__FILE__` | `"path/to/file.sp"` | 当前源文件路径 |
| `__LINE__` | 整数 | 当前源文件行号 |
| `__COUNTER__` | 0, 1, 2, … | 每次展开自增,全局唯一序列 |
| `__DATE__` | `"Jan  1 2025"` | 编译日期 |
| `__TIME__` | `"12:00:00"` | 编译时间 |
| `__STDC__` | `1` | 符合 C11 标准 |
| `__STDC_VERSION__` | `201112L` | C11 版本号 |
| `__STDC_HOSTED__` | `1` | 托管实现 |
| `__SHARP__` | `1` | Sharp 语言模式(仅 `.sp` 文件) |
| `__SHARP_VERSION__` | `"0.7"` | Sharp 版本字符串 |

### `#if` 常量表达式

支持完整的 C11 整型常量表达式:

```
算术:  + - * / %
位运算:<< >> & | ^ ~
逻辑:  && || !
关系:  == != < > <= >=
三目:  cond ? then : else
字符常量:'a'  '\n'  '\xff'
整数字面量:十进制 / 0x十六进制 / 0八进制 / 0b二进制
defined(X):X 已定义则为 1,否则为 0
```

所有运算以 `intmax_t`(至少 64 位)执行;除以零报错而非未定义行为。

### 与 C 的差异和扩展

| 特性 | Sharp 行为 |
|------|------------|
| `#pragma once` | 原生支持,优于手写 include guard |
| `__VA_OPT__` | 支持(C23 特性,Sharp 提前引入) |
| `#include_next` | 支持(GCC 扩展) |
| `__COUNTER__` | 支持(GCC 扩展) |
| `#warning` | 支持(GCC 扩展) |
| 行标记(linemarker) | 预处理器输出 `# N "file"` 行,词法分析器吸收并更新位置信息,错误信息反映原始源码行号 |
| 三字符序列 | 支持但默认不警告(纯 C 兼容需要) |
| `#` 内省原语 | 表达式位置的 `#ident(...)` 不被预处理器处理,直接作为令牌传递给语言前端 |

### 命令行选项

```
sharpc  -DNAME[=value]  # 预定义宏(等价于 #define)
sharpc  -UNAME          # 预取消定义(等价于开头 #undef)
sharpc  -Idir           # 添加用户 include 搜索路径
sharp-cpp               # 独立预处理器可执行文件(与 sharpc 共享同一库)
sharp-cpp -P            # 抑制行标记输出
sharp-cpp --sharp       # 强制 Sharp 语言模式(定义 __SHARP__)
sharp-cpp --tokens      # 将令牌列表输出到 stderr(调试用)
```

---

## 综合示例

把 `impl` / 析构 / 运算符 / 泛型 / `#` 内省串起来:

```sharp
#pragma once

// ---------- 泛型动态数组 ----------
struct Vec<T> {
    T*    data;
    long len;
    long cap;
}

impl<T> Vec<T> {
    Self new() {
        return Self { data: null, len: 0, cap: 0 };
    }

    long len() const {                     // const 方法:可用于 const 接收者
        return this->len;
    }

    T at(long i) const {                   // const 方法
        return this->data[i];
    }

    void push(T item) {                     // 非 const:修改 *this
        if (this->len == this->cap) this->grow();
        this->data[this->len++] = item;
    }

    void grow() {
        long new_cap = this->cap ? this->cap * 2 : 8;
        this->data = realloc(this->data, new_cap * sizeof(T));
        this->cap = new_cap;
    }

    T operator[](long i) const {           // const 读取形式
        return this->data[i];
    }

    ~this() {
        for (long i = 0; i < this->len; i++)
            ~T(&this->data[i]);             // 无析构的 T 被编译期擦除
        free(this->data);
    }
}

// ---------- 泛型求和,约束 T 可加;const 入参只读 ----------
T vec_sum<T>(const Vec<T>* v) where #has_operator(T, +) {
    #static_assert(#has_method(Vec<T>, len), "Vec<T> must support len()");
    T acc = v->at(0);                       // 允许:at() 是 const 方法
    for (long i = 1; i < v->len(); i++) acc = acc + v->at(i);
    return acc;
}

// ---------- Vec<int> 的特化 clear ----------
impl Vec<int> {
    void clear() {                      // (A) 比泛型 impl 更具体
        this->len = 0;                  // int 无析构,直接清零
    }
}

impl<T> Vec<T> {
    void clear() {                      // (B) 泛型兜底
        for (long i = 0; i < this->len; i++) ~T(&this->data[i]);
        this->len = 0;
    }
}

// ---------- 自由函数运算符:float * Vec3 ----------
struct Vec3 { float x; float y; float z; }

Vec3 operator*(float s, Vec3 v) {
    return Vec3 { x: s*v.x, y: s*v.y, z: s*v.z };
}

// ---------- 使用 ----------
void main() {
    Vec<int> xs = Vec<int>.new();
    xs.push(1); xs.push(2); xs.push(3);
    int s = vec_sum(&xs);                    // T 推断为 int;Vec<int>* → const Vec<int>*
    xs.clear();                              // 选 (A),严格最具体

    const Vec<int>* ro = &xs;
    long n = ro->len();                     // OK:len() 是 const 方法
    // ro->push(4);                          // 编译错误:push 非 const
    printf("sum = %d, len = %zu\n", s, n);
    // xs 离开作用域 → ~Vec<int>() → ~int 被擦除 → free(xs.data)

    Vec3 v = Vec3 { x: 1.0, y: 2.0, z: 3.0 };
    Vec3 w = 2.0 * v;                        // 自由函数 operator*
}
```
