# Phase 3 交付说明

## 总览

Phase 3 完成 `#if` 常量表达式求值器加固。**累计 262 项回归测试全部通过**（41 既存 Sharp + 82 Phase 1 + 49 Phase 2 + 90 Phase 3），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

## 修复（基于 Phase 0 探针取证）

### 1. `#if` 中的无符号算术（核心 issue）

**病征**：`#if (-1U) > 0` 取假，应取真。

**根因**：`expr.c` 求值器在 `intmax_t` 上运算，`parse_int_literal` 解析了 `is_unsigned` 但被调用方 `eval_primary` 把这个标志直接丢弃。所有运算路径都按有符号执行，`-1U` 被当成 `-1`，后续比较自然落到有符号语义。

**修复**：把整个求值器从 `ival = intmax_t` 重构成 `Value { intmax_t v; bool is_unsigned; }`：

- 每个 `eval_*` 函数返回 `Value`
- 二元运算前调用 `usual_conv()`（C11 §6.3.1.8 通用算术转换）：任一操作数无符号则两边都按无符号
- 一元 `-` 在无符号上做 `0u - x` 模运算回绕
- 一元 `~` 保留操作数的无符号性
- 一元 `!` 与 `&&`、`||` 返回有符号 0/1
- 比较运算（`<`、`>`、`<=`、`>=`、`==`、`!=`）用 `usual_conv` 后选 `uintmax_t` 或 `intmax_t` 对应的比较，结果是有符号 0/1
- 移位（`<<`、`>>`）保留**左**操作数的类型（C11 §6.5.7p3），右操作数仅作为移位量；移位量被 mask 到 63 防 UB
- 位运算 `&`、`|`、`^`：用 `usual_conv` 后做位运算（位模式不变，只改类型解释）
- 三目 `?:`：返回选中分支的类型
- 公有 API `cpp_eval_if_expr` 返回类型仍是 `intmax_t`（取 `.v`），向后兼容

### 2. 整型字面量后缀更鲁棒

**修复**：
- 后缀解析改为单循环：`u/U/l/L` 任意顺序、任意组合都接受（`UL`、`LU`、`ULL`、`LLU`、`uLL` 等都能识别）
- 非十进制字面量（`0x...`、`0...`、`0b...`）当高位置位时按 C11 §6.4.4.1p5 自动提升为无符号——例：`0xFFFFFFFFFFFFFFFF > 0` 取真

### 3. 字符常量前缀感知

**修复**：`parse_char_const` 看前缀决定有/无符号：
- `'a'`：有符号（int 类型）
- `L'a'`：有符号（wchar_t，本实现用 signed）
- `u'a'`、`U'a'`、`u8'a'`：无符号（`char16_t`/`char32_t`/`char` C23）
- 多字符常量 `'AB'`、`'ABC'`：按 GCC 风格大端打包（`'A'<<8|'B'`）

### 4. 转义序列扩充

**修复**：原代码缺 `\?` 转义；新版本支持完整集合：
- 命名：`\n \t \r \\ \' \" \a \b \f \v \0 \?`
- 八进制：`\NNN`（最多 3 位）
- 十六进制：`\xHH...`（任意位数）

### 5. 移位量 mask（防 UB）

**修复**：移位量大于等于 64 在 C 中是未定义行为；现在 mask 到 `[0, 63]` 范围内，避免 sanitizer 报警。

### 6. 短路求值正确

**保留并强化**：`1 || (1/0)` 中 `1/0` 不被求值，不报错——`eval_logand`/`eval_logor` 用 `ec_skip_expr` 跳过死分支。

### 7. 重复函数声明合并

`directive.c` 和 `expr.c` 之前各有一份不同实现的 `parse_int_literal_pub`（前者更弱，不识别 `0b`、不全识别后缀组合）。本阶段统一到 `expr.c` 的强实现，删除 `directive.c` 的版本。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `expr.c` | +200 / -100 | Value 类型化、整型/字符常量解析重写、所有运算用 usual_conv |
| `directive.c` | -22 | 删除重复的 `parse_int_literal_pub` |
| 新增 `test_phase3.c` | +500 | 90 项 Phase 3 回归测试 |

## 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试
                                   合计 262 项

$ make asan       # 全清，无内存错误，无 UB
$ make strict     # -Werror -Wpedantic -Wshadow -Wstrict-prototypes，无警告
```

通过：
- `-O0 -g`、`-O2`，`-std=c99`、`-std=c11`
- ASan + UBSan
- 严格警告（含 `-Werror`）

### Phase 3 测试覆盖（90 项）

| 章节 | 内容 | 用例数 |
|---|---|---|
| §1 | 无符号算术（一元负、加、减、wraparound） | 4 |
| §2 | 通用算术转换（混合 signed/unsigned 比较、链式） | 2 |
| §3 | 整型后缀（U、L、LL、UL/LU/ULL/LLU 组合） | 7 |
| §4 | 字面量基数（hex 高位提升、小 hex、八进制、二进制） | 7 |
| §5 | 字符常量前缀（无、L、u、U、u8） | 5 |
| §6 | 多字符常量（双字、三字） | 2 |
| §7 | 转义序列（命名 10 种、八进制、十六进制） | 16 |
| §8 | 移位（左移、无符号右移逻辑、有符号右移算术） | 7 |
| §9 | 除/模零（产生诊断、短路避开） | 3 |
| §10 | 逻辑运算（&& 4 项、\|\| 4 项、! 4 项） | 12 |
| §11 | 三目（基础、嵌套） | 4 |
| §12 | 位运算（& \| ^ ~） | 5 |
| §13 | 逗号运算 | 3 |
| §14 | `defined()`（带括号、不带括号、不预展开参数） | 4 |
| §15 | Sharp 模式 parity | 1 |
| 边界 | hex 高位、`0xFFFFFFFFFFFFFFFF`、`(-1U)>>1` 等 | 多处穿插 |

## Phase 0 全部 ISSUE 关闭

至此 Phase 0 探针发现的 10 个商业级缺陷全部修复，剩 1 项是探针自身的预期错误（不是真 bug）：

| # | 缺陷 | 状态 |
|---|---|:---:|
| 1 | 互递归崩溃 | ✅ Phase 2 修复 |
| 2-6 | 词法层 bug | ✅ Phase 1 修复 |
| 7 | `keep_whitespace` | ✅ Phase 1 修复 |
| 8-9 | 宏展开 bug | ✅ Phase 2 修复 |
| 10 | `(-1U)>0` | ✅ **Phase 3 修复** |

## 下一步

Phase 0 取证发现的缺陷已悉数关闭。后续阶段（4/5/6）转向更主动的加固：

- **Phase 4**：`#include` 路径归一、跨包含 linemarker 正确性、`#error`/`#warning` 中的宏展开、诊断 note/hint 风格
- **Phase 5**：Sharp 扩展打磨（`#elif @…`、`@`/`#`/`##` 交互、内置宏完备性）
- **Phase 6**：规模验证（真实 `<stdio.h>`/`<stdlib.h>`、跨平台 CI、性能基准）
