# Sharp C 前端完整支持实施计划

## 目标

让 Sharp 解析器能完整处理 C11 标准头文件（stdio.h、stdlib.h、string.h、stddef.h），支持 Sharp 作为 C 超集的完整语法能力。

---

## 统计分析

### 测试基准：TCC 预处理器输出

对 `stdio.h` 执行 `tcc -E` 后得到约 428 行预处理后的代码。逐行分析不支持的语法：

#### Phase 1: 类型扩展（~80 行代码）

| 特性 | 出现次数 | 示例 | 优先级 |
|------|---------|------|--------|
| `__int64` 类型 | 2 | `typedef __int64 fpos_t;` | 🔥 必须 |
| `__time32_t` / `__time64_t` | 2 | `typedef long __time32_t;` | 中 |
| `wchar_t` 类型 | 数十次 | `wchar_t * fgetws(...)` | 🔥 必须 |
| `_locale_t` 结构体指针 | ~50 | `_locale_t _Locale` | 中 |

#### Phase 2: 声明修饰符（~120 行代码）

| 特性 | 出现次数 | 示例 | 优先级 |
|------|---------|------|--------|
| `#pragma pack(push,N)` | 2 | `#pragma pack(push,8)` | 🔥 必须 |
| `#pragma pack(pop)` | 1 | `#pragma pack(pop)` | 🔥 必须 |
| `__attribute__` | ~10 | `__attribute__((format(gnu_printf,3,0)))` | 🔥 必须 |
| `extern __inline__` | 2 | `extern __inline__ int vsnprintf(...)` | 中 |
| 前向引用 struct | ~10 | `typedef struct threadlocaleinfostruct *pthreadlocinfo;` | 🔥 必须 |

#### Phase 3: 完整 struct/union 支持（~30 行代码）

| 特性 | 出现次数 | 示例 | 优先级 |
|------|---------|------|--------|
| `union` 类型 | 若干 | `union { int i; float f; }` | 中 |
| struct 内嵌匿名 union | 少量 | `_iobuf` 中的字段 | 低 |
| 位域字段 | 0 (stdio.h 无) | `int flag:1;` | 低 |

#### Phase 4: 函数特性（~50 行代码）

| 特性 | 出现次数 | 示例 | 优先级 |
|------|---------|------|--------|
| 可变参数 `va_list` | ~20 | `int printf(const char*, va_list);` | 🔥 必须 |
| 函数内联定义 | 2 | `extern __inline__ int vsnprintf(...) { return ...; }` | 中 |
| `_Static_assert` | 0 (stdio.h 无) | `_Static_assert(sizeof(int)==4, "...");` | 低 |

#### Phase 5: 其他（~30 行代码）

| 特性 | 出现次数 | 示例 | 优先级 |
|------|---------|------|--------|
| `errno_t` 类型 | ~10 | `errno_t clearerr_s(FILE*);` | 中 |
| `_off_t` / `_off64_t` | ~8 | `_off64_t ftello64(FILE*)` | 中 |
| 宽字符串字面量 `L"..."` | 若干 | `L"/"`, `L"\\"` | 中 |

### 其他头文件需要补充支持

| 头文件 | 额外特性 |
|--------|---------|
| `stdlib.h` | `__declspec`（MSVC）、`div_t`/`ldiv_t` struct、`atexit` 函数指针 |
| `string.h` | `const` 正确性、`size_t` 返回值 |
| `stdint.h` | `int8_t` 等 typedef 链、`INT64_C` 宏 |
| `stddef.h` | `offsetof` 宏、`NULL` 定义 |
| `_mingw.h` | `__CRTDECL`、`_CRTIMP`、`__MINGW_ATTRIB_NORETURN` |

---

## 实施计划

### Phase 1: 类型扩展（~80 行）
- [ ] 1.1 token.h: 新增 `TK___INT64`, `TK_WCHAR_T`, `TK___TIME32_T`, `TK___TIME64_T`
- [ ] 1.2 lexer.c: 识别新关键字
- [ ] 1.3 parser.c: `parse_c_type()` 支持新类型 → 映射为 `long long` / `unsigned short`
- [ ] 1.4 types.c: 新增 `TY_WCHAR_T` 类型
- [ ] 1.5 测试: wchar_t、__int64 typedef 解析

### Phase 2: 声明修饰符（~150 行）
- [ ] 2.1 预处理器: 支持 `#pragma pack(push,N)` / `#pragma pack(pop)` — 原样输出到 C
- [ ] 2.2 token.h: 新增 `TK___ATTRIBUTE__`, `TK___INLINE__`, `TK___DECLSPEC`
- [ ] 2.3 lexer.c: 识别 `__attribute__`、`__inline__`、`__declspec`
- [ ] 2.4 parser.c: `__attribute__((...))` → 跳过，记录为修饰符列表
- [ ] 2.5 parser.c: `extern __inline__` → 标记为 inline 函数
- [ ] 2.6 cgen.c: 生成时保留 `__attribute__` 原样输出
- [ ] 2.7 测试: __attribute__ 跳过，函数内联

### Phase 3: struct/union 完善（~50 行）
- [ ] 3.1 parser.c: `union` 声明解析（类似 struct）
- [ ] 3.2 parser.c: 前向引用 `typedef struct Tag *PtrType;`
- [ ] 3.3 测试: union typedef，前向引用

### Phase 4: 函数特性（~100 行）
- [ ] 4.1 token.h: `TK_VA_LIST`
- [ ] 4.2 parser.c: `va_list` 类型识别
- [ ] 4.3 parser.c: 函数体内联定义 `extern __inline__ int f() { return 0; }`
- [ ] 4.4 测试: va_list 参数，内联函数

### Phase 5: 其他（~50 行）
- [ ] 5.1 token.h: `TK_ERRNO_T`, `TK_OFF_T` 等
- [ ] 5.2 lexer.c: 宽字符串 `L"..."` 识别
- [ ] 5.3 parser.c: `_Static_assert` 支持
- [ ] 5.4 cgen.c: 宽字符串生成 `L"..."`
- [ ] 5.5 测试: 宽字符串、_Static_assert

---

## 风险与注意事项

1. **`#pragma` 处理**：需要预处理器支持，不能只在 parser 层
2. **`__attribute__` 参数解析**：需要跳过嵌套括号
3. **MSVC `__declspec`**：当前测试用 TCC 头文件不含 `__declspec`，但 MSVC 版本需要
4. **向后兼容**：现有 Sharp 代码不能受影响
5. **测试覆盖**：每个 phase 至少 5 个测试用例

---

## 预估总代码量

| Phase | 改动文件 | 行数 |
|-------|---------|------|
| 1. 类型扩展 | token.h, lexer.c, parser.c, types.c | ~80 |
| 2. 声明修饰符 | token.h, lexer.c, parser.c, cgen.c, cpp/ | ~150 |
| 3. struct/union | parser.c | ~50 |
| 4. 函数特性 | token.h, lexer.c, parser.c | ~100 |
| 5. 其他 | token.h, lexer.c, parser.c, cgen.c | ~50 |
| 测试 | tests/ | ~100 |
| **总计** | | **~530 行** |
