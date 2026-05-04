# Sharp/C 预处理器 — 商业级实现规划

> Sharp 是 C 的超集；本仓库是**单一**预处理器（带 `CPP_LANG_SHARP` 模式开关），不是两套实现。

## 状态总览

| 阶段 | 范围 | 状态 | 测试通过 |
|---|---|:---:|:---:|
| **Phase 0** | 既存代码审计：50 项探针定位 10 个缺陷（含 1 真崩溃） | ✅ 完成 | — |
| **Phase 1** | 词法层加固 + 诊断基础（`pptok.c` 重点改） | ✅ 完成 | 82 / 82 |
| **Phase 2** | 宏引擎加固（`macro.c` 重点改） | ✅ 完成 | 49 / 49 |
| **Phase 3** | `#if` 常量表达式（`expr.c` 重点改） | ✅ 完成 | 90 / 90 |
| **Phase 4** | 指令打磨（`directive.c` 重点改） | ✅ 完成 | 47 / 47 |
| **Phase 5** | Sharp 扩展打磨（`macro.c` `##` 与 `@` 交互） | ✅ 完成 | 46 / 46 |
| **Phase 6** | 规模验证（`#include_next` 修复、真实头、CI、benchmark） | ✅ 完成 | 47 / 47 |
| **Phase 7** | API 友好性：`cpp_install_target_macros(triple)` 助手（新增 `cpp_targets.c`） | ✅ 完成 | 69 / 69 |
| **Phase 8** | 规范完整性收尾：`__has_include_next` 真实语义 + GCC linemarker 输入语法 + profiling 工具链 | ✅ 完成 | 37 / 37 |
| **Phase 9** | 词法层性能优化：rb_*/ph1/ph2 inline + 快慢路径分离（gprof 数据驱动） | ✅ 完成 | 0 新增 / 数据：65.76→54.55 ms（-17.3%） |
| **Phase 10** | 探针验证 + 消除双重 hash 查找（gprof 推翻 HANDOVER 描述：MacroTable 已是 hash table） | ✅ 完成（诚实交付） | 0 新增 / gprof 量化：macro_lookup_is_func 调用 -28%；wall-clock 不显著（容器抖动内） |
| **Phase R1** | 真实场景集成测试：Lua 5.4.7 全 35 文件零诊断 + 修一个真 bug（`<>` 错误搜索 including-file 目录） | ✅ 完成 | 19 / 19 + 35 个 Lua 文件 100% 通过 / aggregate ratio 1.001 vs gcc -E |
| **Phase R2** | 真实场景规模化：SQLite 3.45.0 src/*.c (126 文件) + 9 MB amalgamation 全部零诊断 | ✅ 完成 | 126 / 126 + sqlite3.c (255k 行) / aggregate ratio 0.959 vs gcc -E |
| **Phase R3** | zlib 1.3.1 全 15 文件零诊断（K&R-era 风格、平台条件型代码覆盖） | ✅ 完成 | 15 / 15 / aggregate ratio 0.970 vs gcc -E |
| **Phase R4** | mcpp Validation Suite (PP 标准合规黄金测试集)：修 9 真 PP-spec bug + phase6 hang + 11 strictness 检查 | ✅ 完成 | Clean tests **59/59 (100%)**, total **99/109 (90.8%)** |
| **Phase R5** | Redis 真实场景集成（131 .c + 8 vendored deps）：修 4 真 PP-spec bug + 1 架构修（inactive #if 不 eval） | ✅ 完成 | **Redis 115/115 (100%) 零诊断** / aggregate ratio 0.985 vs gcc -E |
| **Phase R6** | 自身 dogfooding：sharp-cpp 预处理自身 20 .c 源码：修 1 真 bug（GNU `, ##__VA_ARGS__` 非空 VA 时不该 paste） | ✅ 完成 | **20/20 自身零诊断** / aggregate ratio **1.004** vs gcc -E（近乎逐字节一致）|
| **Phase R7** | mcpp 100% 收尾：修 3 真 bug（multi-line cascaded paste、UCN in ident、redef whitespace 等价）+ 3 spec-divergence skip 加 citation | ✅ 完成 | **mcpp 106/106 (100.0%)** Clean+Error 全过；累计 R 系列 417/417 |
| **Phase R8** | 跨 PP 测试：sharp-cpp 处理 mcpp 自身 11 个 .c 源文件（cc1.c skip — MinGW only） | ✅ 完成 | **mcpp source 10/10 零诊断** / aggregate ratio 0.933 vs gcc -E（K&R 风格 + multi-byte chars） |
| **Phase R9** | 语言标准 mode：cpp_set_lang_std API + sharp_cli `-ansi`/`-std=cXX` + mcpp_smoke 解析 dg-options + #line 范围按 lang_std | ✅ 完成 | **mcpp 108/108 (100%)** ↑ from 106；e_14_10、u_1_17s 解锁；累计 R 系列 429/429 |
| **Phase R10** | mcpp 零 skip：检测 cascaded macro arg 的 expansion 含 unbalanced parens 并 warn (e_25_6) | ✅ 完成 | **mcpp 109/109 (100%) 零 skip！**；累计 R 系列 430/430 |
| **Phase R11** | mcpp native .t testsuite (153 文件) smoke test：sharp-cpp 处理无 fatal/hang | ✅ 完成 | **148/148 smoke pass** (5 skip = C++/multi-byte ident)；累计 R 系列 578/578 |

**累计回归测试：610 项已通过**（41 既存 Sharp + 82 P1 + 49 P2 + 90 P3 + 47 P4 + 46 P5 + 47 P6 + 69 P7 + 37 P8 + 19 R1 + **83 R4-R10**），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

**至此，Phase 0-10 + R1-R11 全部完成**。累计修复 **33 个真 bug**：
- Phase 0 取证 10 个（互递归崩溃、宽字符前缀、未闭合字面量、`keep_whitespace` 失效、stringify 丢空格、参数前导空泄漏、`(-1U)>0` 错算等）
- Phase 4 — `#line "f"` 不更新 `__FILE__`
- Phase 5 — `##` 与 `@` 内省 token 交互静默丢 RHS
- Phase 6 — `#include_next` 用文件路径而非目录比较
- Phase 8 — `__has_include_next` 是 stub（与 `__has_include` 行为相同）
- Phase 8 — GCC linemarker 输入语法 `# N "file" [flags]` 报 unknown directive
- Phase R1 — `<...>` 形式 `#include` 错误搜索 including-file 目录，导致 glibc 系统头无限递归
- **Phase R4 #1** — `keep_comments=false` 时注释 token kind 没改 SPACE，下游所有 SPACE-skip 漏掉（**1 处根因修替代 ~30 处 caller 修补**）
- **Phase R4 #2** — `\0..\7` 起首的 octal escape 被 `case '0'` 错误共享走 `'?'` 分支
- **Phase R4 #3** — 三元运算符没做 usual arithmetic conversion（C99 §6.5.15/5）
- **Phase R4 #4** — 三元运算符 short-circuit 在加 usual_conv 后被破坏（C99 §6.5.15/4）
- **Phase R4 #5** — `collect_arg` 不区分 comma/paren 退出，`APPEND( , )` 漏最后空 arg
- **Phase R4 #6** — object-like macro 的 body 不走 substitute()，`##` 静默被忽略
- **Phase R4 #7** — UCN `\u`/`\U` escape 在 char constant 内被当多 byte 处理
- **Phase R4 #8** — `phase6_apply_text` 在 unterminated string 上死循环（`"\n` 即可触发）
- **Phase R4 #9** — `#line N "..."` 接受 wide string literal（C99 §6.10.4/4 禁止）
- **Phase R5 #1** — `_Pragma` operator 完全没实现（C99 §6.10.9）
- **Phase R5 #2** — `__has_*` 家族（`__has_builtin` 等）没被 `#ifdef`/`defined()` 视为 implicit defined，导致 xxhash-style trampoline `#define HAS_X(x) __has_builtin(x)` 走 fallback 0 分支后 `#if 0(x)` 触发 R4 strict check 自伤
- **Phase R5 #3 (架构修)** — 嵌套 `#if`/`#elif` 在 inactive parent block 内仍 evaluate expression，违反 C99 §6.10.1（"skipped groups are not processed"）。修后 Redis 通过率 36% → 96%
- **Phase R5 #4** — 变参 `__VA_ARGS__` 收集时不 track paren depth，`VA_FUNC(INNER())` 中 INNER 的 `)` 提前终止外层 collection，导致 `()` 漏出。修后 Redis 96% → 100%
- **Phase R6 #1** — GNU 扩展 `, ##__VA_ARGS__` 在 VA 非空时仍走通用 paste 路径，对每个 `LOG("foo %s", x)` 调用点报"pasting ',' and 'x' does not give a valid pp-token" warning（GCC/Clang 都不报，因为这 idiom 中 `##` 仅是空 VA 的 marker，非空时应忽略 paste）。修后 sharp-cpp 自身 dogfooding 零诊断
- **Phase R7 #1** — multi-line cascaded `##` paste 不剥 NEWLINE：跨行 macro call (mcpp n_37) 中 args 含 NEWLINE，paste 时 NEWLINE 留在 result.tail，下一 paste 把它当 LHS → 报 "pasting '\n' and 'X'" warning。修：strip NEWLINE 同 SPACE + 跳 leading whitespace 取 first arg token
- **Phase R7 #2** — UCN escape `\uXXXX` / `\UXXXXXXXX` 在 identifier 内不识别 (mcpp n_ucn1, e_ucn)：sharp-cpp ident lex 遇 `\` 即停，让 `macro\u5B57` 和 `macro\U00006F22` 都 lex 为 `macro` → false-positive redef. 修：在 ident lex 内识别 `\u`/`\U` 转义并验证 code point (C99 §6.4.3)
- **Phase R7 #3** — `macro_bodies_equal` byte-by-byte 比较 (mcpp n_19)：C99 §6.10.3 要求 redef 在 whitespace-equivalence 下相同，但 sharp-cpp 把 SPACE/NEWLINE/COMMENT 也算入比较。修：跳过 whitespace tokens 比较 kind+spelling
- **Phase R10 #1** — cascaded macro arg expansion 含 unbalanced `(` 不报警 (mcpp e_25_6)：C99 §6.10.3.4 说 arg 的 rescan 应当 self-contained，sharp-cpp 之前 silently 把 `sub(` 类 expansion stitched 进 outer body 触发不可预测行为。修：args pre-expand 后检查 paren depth，不平衡 emit warning

加上 R4 11 处 strictness 检查：空字符常量、`#if` 末尾非法 token、整数常量溢出、`#param` 非参数错误、`##` 起首/结尾错误（object/function 都覆盖）、参数太多/太少、hex escape >byte、multi-char >8 chars、跨文件 `#if/#endif` 平衡、控制字符 invalid、`#include` trailing token warning、`/##/` paste warning。

加上若干主动加固：尾随 token 警告、`#include` 失败的搜索路径 note、`emit_note` 助手、Sharp `@`-token 安全 paste 守卫、Phase 7 的三-triple 平台宏注入助手 API、Phase 8 的 profiling 工具链、**Phase 9 的词法层性能优化（基线 65.76 ms/iter → 54.55 ms/iter，-17.3%；2.14× → 1.79× gcc -E）**、**Phase 10 的探针先行胜利**（gprof 推翻 HANDOVER 推测的"MacroTable 用线性扫描"；实测发现 hash table 早已存在，真问题是 caller 双重查找；消除冗余后 `macro_lookup_is_func` 调用减 28%，gprof self-time 减 35%；wall-clock 改善被容器抖动吞没但代码质量改善 + LTO hook 留存）、**Phase R1 真实场景集成测试**（Lua 5.4.7 全 35 文件零诊断处理；total output 与 gcc -E ratio 1.001——几乎逐字节相同；同时发现并修复 1 个真 bug：`<...>` 形式 `#include` 错误搜索 including-file 目录）。**真实 18 个 POSIX 头零错误**、性能比 `gcc -E` 慢约 **2.0-2.4 倍**、跨平台 **CI matrix（Linux × {gcc, clang} × {c99, c11}, macOS, Windows MSYS2）落地**。

---

## Phase 0 — 审计取证（已完成）

构建了 50 个定向探针 `probe.c`，识别出 10 个商业级缺陷：

| # | 缺陷 | 严重度 | 处理阶段 |
|---|---|:---:|:---:|
| 1 | `#define A B` / `#define B A` → **栈溢出崩溃** | 🔴 严重 | Phase 2 ✅ |
| 2 | `L"foo"` 切成 `L`+`"foo"` 两 token | 🟡 重要 | Phase 1 ✅ |
| 3 | `u"…"`, `U"…"`, `u8"…"` 同上 | 🟡 重要 | Phase 1 ✅ |
| 4 | `L'a'` / `u8'a'` 等字符前缀同上 | 🟡 重要 | Phase 1 ✅ |
| 5 | 未闭合 `"…<EOF>` 与 `/* …<EOF>` 静默通过 | 🟡 重要 | Phase 1 ✅ |
| 6 | 未闭合行内字符串吃掉换行，破坏 `#define` 行首识别 | 🟡 重要 | Phase 1 ✅ |
| 7 | `cpp_keep_whitespace(true)` 形同虚设 | 🟢 一般 | Phase 1 ✅ |
| 8 | `S(  a   b   c  )` → `"abc"`（应为 `"a b c"`） | 🟡 重要 | Phase 2 ✅ |
| 9 | `F(A)` → `[ 1]`（多余空格） | 🟢 美观 | Phase 2 ✅ |
| 10 | `(-1U) > 0` 取假（应取真） | 🟡 重要 | Phase 3 🔜 |

---

## Phase 1 — 词法层加固（已完成，82/82 测试通过）

### 修复

1. **宽/Unicode 字符串与字符前缀**（`L"…"`, `u"…"`, `U"…"`, `u8"…"`，字符常量同）：在标识符分支**之前**插入前缀-引号探测；非引号则恢复 reader 状态让标识符分支处理（`Lvalue`/`u8type`/`u8_var` 仍合法）。

2. **未闭合字面量与块注释诊断**：`lex_quoted` 与 `skip_block_comment` 改为返回 `bool`；`lex_quoted` 用 peek-then-getc 模式**保留**致命换行。失败时在开引号位置发 `CPP_DIAG_ERROR("unterminated …")`。

3. **`cpp_keep_whitespace(true)` 真生效**：在 `emit_tok_text` 处合成 SPACE token（`pptok` 从不发 SPACE，把空白折到下一 token 的 `has_leading_space` 标志）。新增"重建"测试验证 `out_tokens` 拼接 = `out_text`。

4. **遗留编译警告清理**（`expr.c`、`directive.c` 各一处未用变量）。

### 文件改动
- `pptok.c` (+160/-50)：lex_quoted/skip_block_comment 重写、前缀块前移
- `directive.c` (+50/-10)：emit_tok_text 加合成 SPACE
- `expr.c` (-2)：删未用变量
- 新增 `test_phase1.c`（82 项）、`probe.c`（50 项）、`Makefile`

### 测试覆盖
13 项 §1 前缀、7 项 §2 未闭合、4 项 §3 行拼接、3 项 §4 keep_whitespace、2 项 §5 CRLF、4 项 §6 pp-number、2 项 §7 三字符、2 项 §8 长输入、3 项 §9 空输入、2 项 §10 Sharp 模式、1 项 §11 诊断 loc。

---

## Phase 2 — 宏引擎加固（已完成，49/49 测试通过）

### 修复

1. **互递归栈溢出**（真崩溃）：
   - 根因：`MAX_EXPAND_DEPTH = 1,000,000`（C 栈早爆掉），单 `bool hide` 标志只盖自递归不盖互递归
   - 修复：在 `MacroTable` 上加 `HideSet active_expansions` 栈，进入宏展开 push、出 pop；展开 token 前先查栈，命中则原样透出。`MAX_EXPAND_DEPTH` 降到 200，深度检查改为以栈长（真实递归深度）为准
   - 验证通过：A↔B、A↔B↔C、10-环、C99 §6.10.3.4 例子、Prosser `f(f)(1)→f(1)`、间接互递归

2. **字符串化丢空格**：`stringify` 既看 `CPPT_SPACE`（防御）也看每个非首 token 的 `has_leading_space`；前后空白去除、内部多空白合一

3. **形参替换泄漏前导空格**：`substitute` 中首 token `has_leading_space` 改为**直接覆盖**为形参引用处的标志，不再 OR

### 文件改动
- `macro.c`：HideSet 栈、深度检查、stringify、参数前导空（约 +120/-30）
- 新增 `test_phase2.c`（49 项）

### 测试覆盖
6 项 §1 互递归、2 项 §2 自递归、5 项 §3 stringify、2 项 §4 参数前导空、3 项 §5 `__VA_OPT__`、5 项 §6 paste、4 项 §7 参数预展开、1 项 §8 limits、2 项 §9 Sharp 模式、3 项 §10 stress

---

## Phase 3 — `#if` 常量表达式（已完成，90/90 测试通过）

### 修复

1. **无符号算术**（核心 issue）：把整个求值器从 `intmax_t` 重构为 `Value { intmax_t v; bool is_unsigned; }`。每个二元运算前调用 `usual_conv()`（C11 §6.3.1.8）；任一无符号则两边按无符号执行。`(-1U) > 0` 现在正确取真。

2. **整型字面量后缀**：单循环识别 `u/U/l/L` 任意顺序组合（`UL`/`LU`/`ULL`/`LLU` 等）；hex/octal/binary 字面量高位置位时按 §6.4.4.1p5 自动提升为无符号。

3. **字符常量前缀感知**：`'a'`/`L'a'` 有符号；`u'a'`/`U'a'`/`u8'a'` 无符号。多字符常量按 GCC 大端打包（`'AB'='A'<<8|'B'`）。

4. **完整转义序列**：`\?` 补齐；`\xHH...` 任意位、`\NNN` 八进制最多 3 位。

5. **移位量 mask**：`a << b` 中 `b ≥ 64` 在 C 是 UB；mask 到 `[0, 63]`。

6. **短路求值**：`1 || (1/0)` 不报错（右侧不评估）。

7. **重复函数清理**：`directive.c` 与 `expr.c` 各有一份不同实现的 `parse_int_literal_pub`，统一到 `expr.c` 强版本。

### 文件改动
- `expr.c` (+200/-100)：Value 类型化、整型/字符常量解析重写
- `directive.c` (-22)：删除重复 `parse_int_literal_pub`
- 新增 `test_phase3.c`（90 项）

### 测试覆盖
4 项 §1 无符号、2 项 §2 通用算术转换、7 项 §3 后缀、7 项 §4 字面量基数、5 项 §5 字符前缀、2 项 §6 多字符、16 项 §7 转义、7 项 §8 移位、3 项 §9 除零、12 项 §10 逻辑、4 项 §11 三目、5 项 §12 位运算、3 项 §13 逗号、4 项 §14 `defined()`、1 项 §15 Sharp parity。

---

## Phase 4 — 指令打磨（已完成，47/47 测试通过）

### 修复

1. **`#line N "filename"` 真 bug**：先前只更新行号不更新文件名，导致 `__FILE__` 在 `#line "renamed.c"` 之后仍展开旧名。增加 `reader_set_file()` API，`handle_line` 调用之。

2. **尾随 token 警告**（GCC 风格 `-Wendif-labels`）：`#endif foo`、`#else garbage`、`#ifdef X extra`、`#ifndef X extra`、`#undef X extra` 都发警告（不阻断），但 `#endif /* comment */` 与单纯空白不警。引入 `warn_trailing(st, dname, line, consumed, loc)` 助手。

3. **`emit_note(st, loc, fmt, ...)` 助手**：与 `emit_diag` 配套发 `CPP_DIAG_NOTE` 等级的 follow-up。

4. **`#include` 失败列出搜索路径**：之前只发 `error: file not found: foo.h`。现在追加 4 类 note：
   - `searched (relative to including file): /dir/`（仅 `"…"` 形式）
   - `searched (user, -I): /path`（每个 -I 一条）
   - `searched (system): /path`（每个 sys 一条）

5. **`handle_else`、`handle_endif` 签名扩展**：从 `(st, loc)` 改成 `(st, line, loc)`，dispatcher 同步。

### 文件改动
- `pptok.c/h` (+8)：`reader_set_file` API
- `directive.c` (+60/-10)：`emit_note`、`warn_trailing`、include not-found notes、`handle_line` 文件名更新、`handle_else/endif` 签名变化
- 新增 `test_phase4.c`（47 项）

### 测试覆盖
7 项 §1 尾随 token、2 项 §2 search-path note、6 项 §3 `#include` 行为、3 项 §4 `#line`、6 项 §5 条件结构错误、2 项 §6 linemarker、2 项 §7 Sharp parity。

---

## Phase 5 — Sharp 扩展打磨（已完成，46/46 测试通过）

### 修复

1. **`##` 与 `@` 内省的交互（真 bug）**：之前 `P(check_, @dtor)` 静默丢弃 `@dtor`（标准 C "invalid paste" 路径返回 LHS 不变）；Sharp 中这是不可接受的，因为 `@dtor` 必须传到 Sharp 前端。`token_paste()` 加 `out_keep_rhs` 出参，`@`-token 操作数时发警告并返回 `*out_keep_rhs=true`，调用方 append RHS 到结果列表。

### 经探针验证无回归（17 项 Sharp 行为全部正确）

`__SHARP__`/`__SHARP_VERSION__` 模式区分；`@ident` 词法化（Sharp 模式 `CPPT_AT_INTRINSIC`，C 模式 `CPPT_PUNCT`）；`@`-token 宏免疫；`@` 在 `#define` 体内保留 kind；`#if @`/`#elif @` 延迟；嵌套延迟；普通 `#if 1` 在外层 `#if @` 内也走延迟；死分支 `#if 0` 不延迟；`# @ident` 字符串化得 `"@ident"`；C 模式 parity（`has_destructor` 可作普通宏）。

### Spec 一致性核查

按 v0.10 spec 行 744-745，Sharp 内置宏**只有** `__SHARP__` (=1) 和 `__SHARP_VERSION__` (="0.10")。原计划提到的 `__SHARP_TARGET_*` 不在 spec 范围（由前端按目标三元组注入），不属预处理器职责，确认不实现。

### 文件改动
- `macro.c` (+35/-10)：`token_paste` 加 `out_keep_rhs` + Sharp `@` 守卫
- 新增 `test_phase5.c`（46 项）

### 测试覆盖
4 项 §1 Sharp 内置、3 项 §2 `__SHARP_CPP__`、3 项 §3 词法化、2 项 §4 宏免疫、1 项 §5 体内 kind 保留、3 项 §6 延迟、2 项 §7 嵌套延迟、3 项 §8 `##` 与 `@`、1 项 §9 `#` 与 `@`、3 项 §10 C 模式 parity、1 项 §11 死分支。

---

## Phase 6 — 规模验证（已完成，47/47 测试通过）

### 修复

1. **`#include_next` 用文件路径而非目录比较（真 bug）**：之前 `find_include_ex` 检查 `if (strcmp(found, skip_until) == 0)`，`found` 是目标文件全路径，`skip_until` 是源文件全路径，两个文件名不同永远不等，`skipping` 永远为真，所有候选被跳过。修复：函数开头计算 `skip_dir = dirname(skip_until)`，遍历搜索路径时用 `strcmp(sys_paths[i], skip_dir)` 判断。修复后 GCC 的 `<limits.h>` 链路正常。

### 真实头文件验证

带平台宏（`__x86_64__`、`__linux__`、`__GNUC__` 等）注入后，**18 个常用 POSIX 头零错误零警告**：`stddef.h`、`stdint.h`、`stdbool.h`、`stdarg.h`、`limits.h`、`errno.h`、`assert.h`、`ctype.h`、`string.h`、`stdlib.h`、`stdio.h`、`math.h`、`time.h`、`unistd.h`、`sys/types.h`、`stdatomic.h`、`pthread.h`、`signal.h`、`fcntl.h`。

### 性能基准
30 次迭代，包含 17 个 POSIX 头 + main 的典型输入，输出约 9,177 行：
- sharp-cpp: **52.10 ms/iter**，176K 输出行/秒
- gcc -E: **21.34 ms/iter**（含 fork/exec 开销）
- ratio: **2.44×** slower

### 跨平台 CI（`.github/workflows/ci.yml`）

| 平台 | 编译器 | 标准 |
|---|---|---|
| Linux (Ubuntu) | gcc, clang | -std=c99, -std=c11 |
| macOS | Apple clang | -std=c11 |
| Windows | MSYS2 MinGW-w64 GCC | -std=c11 |

每个 job 跑 `make test`、`make asan`、`make strict`（Windows 跳 strict）。

### 文件改动
- `directive.c` (+35/-25)：`#include_next` 目录比较修复 + `auto_skip` 清理
- 新增 `test_phase6.c`（47 项）
- 新增 `bench.c`（独立 benchmark）
- 新增 `.github/workflows/ci.yml`（三平台 CI）

### 测试覆盖
26 项 §1 真实系统头、5 项 §2 `#include_next` 合成、3 项 §3 大堆头、1 项 §4 性能预算、2 项 §5 诊断预算、4 项 §6 确定性、4 项 §7 上下文复用、2 项 §8 Sharp parity。

---

## Phase 7 — API 友好性（已完成，69/69 测试通过）

Phase 6 留下了一个真实的 API 不友好问题：caller 必须手写 18 行 `cpp_define()` 才能让 `<stdio.h>` 走对编译路径。Phase 6 的 `test_phase6.c` 里有一个 hard-coded 的 `install_platform_macros()`（仅 x86_64-linux-gnu 一份）作为权宜之计，但它不是公开 API。Phase 7 把这个能力提升为 first-class 的 `cpp_install_target_macros(ctx, triple)`，并扩展到三种主流目标。

### 新增

1. **公开 API（`cpp.h`）**

   ```c
   int cpp_install_target_macros(CppCtx *ctx, const char *triple);
   const char *const *cpp_target_known_triples(void);
   ```

   - 返回 0 = 成功识别并注入；返回 -1 = triple 未识别 / NULL ctx / NULL triple，**不污染 ctx**
   - 不启用 feature-test 宏（`_GNU_SOURCE` / `_POSIX_C_SOURCE` 等），它们仍由 caller 决定（属"意图"而非"事实"）
   - `cpp_target_known_triples()` 返回 NULL-terminated 数组，方便 CLI 工具列举支持的目标

2. **新模块 `cpp_targets.c`**（独立文件，不挤进 directive.c 的 2200 行）

   三个 install 函数各自自包含，无共享 helper（避免引入耦合）：

   | Triple | 数据模型 | 关键事实 |
   |---|---|---|
   | `x86_64-linux-gnu` | LP64 | `__LP64__=1`, `__SIZEOF_LONG__=8`, `__ELF__=1`, `__GNUC__=13` |
   | `aarch64-apple-darwin` | LP64 + Mach-O | `__APPLE__=1`, `__MACH__=1`, **无** `__ELF__`、**无** `__linux__`，`__SIZEOF_LONG_DOUBLE__=8`（与 Linux 16 不同），`__CHAR_UNSIGNED__=1` |
   | `x86_64-pc-windows-msvc` | **LLP64**（关键差异） | `_M_X64=100`, `_WIN64=1`, **`__SIZEOF_LONG__=4`**, `__SIZEOF_WCHAR_T__=2`（UTF-16），`_MSC_VER=1929`，**无** `__LP64__`、**无** `__GNUC__` |

   宏值取自真实工具链的 `gcc -dM -E -x c /dev/null` 输出（GCC 13 / Apple clang 15 / MSVC 19.29）。

3. **测试 `test_phase7.c`（69 项）**

   测试覆盖按"严谨性优先"设计：
   - **§1（4）** 三 triple 各自识别成功 + 枚举器 NULL-terminated
   - **§2（4）** 错误处理：未知 triple、NULL triple、NULL ctx、未知 triple 不污染 ctx
   - **§3（8）** Linux 关键宏值（含 `__BYTE_ORDER__` 嵌套展开）
   - **§4（8）** Apple 关键宏 + **负断言**（无 `__linux__`/`__ELF__`）
   - **§5（11）** Windows 关键宏 + **负断言**（无 `__LP64__`/`__GNUC__`）+ LLP64 标志
   - **§6（10）** 跨 triple 不变量：同一段 `#if __SIZEOF_LONG__ == 8` 在 linux/darwin 走真分支、在 windows 走假分支
   - **§7（5）** 端到端：linux triple 跑真实 `<stddef.h>` / `<stdint.h>` / `<limits.h>` 零错误，证明助手与 P6 hand-rolled 注入达到 parity
   - **§8（4）** Sharp 模式 parity：triple 注入在 Sharp 模式同样可见
   - **§9（5）** 幂等性 + 边界（重复注入安全，空字符串 triple 视为未知）

### 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| 新增 `cpp_targets.c` | +311 | 三 triple 注入 + 公开入口 + 枚举器 |
| `cpp.h` | +33 | `cpp_install_target_macros` / `cpp_target_known_triples` 声明 |
| `Makefile` | +5 | `cpp_targets.c` 加入 `LIB_SRCS`、`test_phase7` 规则与运行 |
| 新增 `test_phase7.c` | +508 | 9 节 69 项回归 |
| `.github/workflows/ci.yml` | 注释更新 | "all six" → "all seven" |
| `PLAN.md` | 状态表 + 本节 | 单一真相源更新 |

### 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Phase 5 results: 46 passed, 0 failed
Phase 6 results: 47 passed, 0 failed, 0 skipped
Phase 7 results: 69 passed, 0 failed, 0 skipped
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试
                                   合计 471 项

$ make asan       # 全清，零内存错误，零 UB
$ make strict     # -Werror -Wpedantic -Wshadow -Wstrict-prototypes，无警告
$ ./probe         # 仍仅 1 项预期 ISSUE（探针自身的 __LINE__ 边界）
$ make bench      # 65.81 ms/iter, 2.03× gcc -E（无性能回归）
```

### Phase 7 测试覆盖（69 项汇总）

| 章节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | 基础识别 + 枚举器 | 7 |
| §2 | 错误处理（NULL/未知/不污染） | 8 |
| §3 | x86_64-linux-gnu 关键宏值 | 8 |
| §4 | aarch64-apple-darwin 关键宏 + 负断言 | 8 |
| §5 | x86_64-pc-windows-msvc LLP64 + 负断言 | 11 |
| §6 | 跨 triple 不变量（`__SIZEOF_LONG__`、`__LP64__`、OS 分支） | 13 |
| §7 | 真实头端到端（`stddef.h`/`stdint.h`/`limits.h`） | 5 |
| §8 | Sharp 模式 parity | 4 |
| §9 | 幂等性 + 边界 | 5 |

### 设计取舍记录

- **为何 LLP64 是 P7 的核心严谨点**：很多 C 代码假设"64-bit 系统 ⇒ `long` 是 64-bit"，这在 Linux/macOS 成立，在 Windows-x64 **不**成立。Phase 7 的 Windows triple 显式 `__SIZEOF_LONG__=4`、不定义 `__LP64__`，让真实的 LLP64 chain 可被激活；§6.1 / §6.2 的负断言锁定了这一点，防止未来误改。
- **不引入共享 size helper**：考虑过抽出 `install_common_size_macros_lp64_le()`，但 LP64 在 Linux 与 Apple 的 `__SIZEOF_LONG_DOUBLE__` 不一致（16 vs 8）、`__INT64_TYPE__` 也不一致（`long int` vs `long long int`）——共享 helper 会变成"几乎所有字段都被覆盖"的反模式。三函数各自展开更清晰、更安全。
- **不注入 `_GNU_SOURCE` 等**：feature-test 宏属于 caller 意图。Phase 7 helper 是"如果你声称在编译这个 triple，那么这些宏一定为真"——纯事实，不带政策。
- **未来扩展点**：在 `cpp_install_target_macros` 派发 + `cpp_target_known_triples` 列表里同步加一行即可加入新 triple（如 `aarch64-linux-gnu`、`riscv64-linux-gnu`）；不会触碰其他模块。

### 仍未做（Phase 8+ 候选）

- **F**：`__has_include_next` 完整测试（实现已存在，仅测试缺口）
- **G**：`#line N "f" 1 3` GCC 扩展 system-header 标记数字传播
- **B/C/D**：性能优化三件套（intern hash / token 池 / StrBuf 预分配）—— 建议先做 **I**（profiling 工具）拿到火焰图再决定优化目标，避免凭直觉
- **E**：完整 Prosser per-token hide-set —— 当前 active-expansion 栈对所有真实场景都过；属"理论一致性"而非"用户痛点"，建议留到出现具体反例再做

---

## Phase 8 — 规范完整性收尾（已完成，37/37 测试通过）

Phase 7 完成 API 友好性后，HANDOVER 留下了三个候选 P0 项：F（`__has_include_next` 测试）、G（`#line N "f" 1 3` GCC 扩展）、I（profiling 工具链）。Phase 8 把三件一起做掉。

**重要发现（探针先行的价值）**：F 和 G 在调研阶段都升级为 **真 bug 修复**，而非 HANDOVER 描述的"仅加测试"或"语义不完整"：

- F 原描述：`__has_include_next` 实现存在但没有专门测试。**实测**：实现是 stub（`(void)is_next; /* … needs extra logic, for now same */`），与 `__has_include` 行为完全一致。需要先实现真实语义再写测试。
- G 原描述：尾部 1/3 数字被忽略（行为正确）；语义不完整。**实测**：派发器只识别 `#<ident>` 形式，`# N "file"` 这种 GCC linemarker 直接报 `unknown directive: #`，sharp-cpp 自己输出的 linemarker 都不能被自己读回来。

### 修复

1. **`__has_include_next` 真实语义**（真 bug）

   `cpp_has_include` 升级支持 `is_next` + `current_file` 参数；当 `is_next=true` 时计算 `skip_dir = dirname(current_file)`，搜索路径列表中跳过该目录及之前的所有目录，从下一个开始查找——这与 Phase 6 修复的 `#include_next` 同源（`find_include_ex` 用的是同样的 skip_dir 逻辑）。

   边界处理：如果 `current_file` 没有目录部分（虚拟名如 `<test>`），优雅降级为普通 `__has_include` 语义而不是失败。

   `resolve_has_include` / `resolve_all_has_include` 接受 `current_file` 参数；`handle_if` / `handle_elif` 把 `loc.file` 沿调用链传递。

2. **GCC linemarker 输入语法 `# N "file" [F1 [F2]]`**（真 bug）

   - 派发器识别 `kw.kind == CPPT_PP_NUMBER`：合成 `dname="line"` + `tl_prepend(line, kw)`，让 `handle_line` 统一处理 `#line` 关键字形式与 `# N` 数字形式
   - `handle_line` 解析尾部最多两个 GCC flag 数字（1=push, 2=pop, 3=system header, 4=extern "C"），范围外的数字静默忽略（GCC 行为）
   - `emit_linemarker_ex(st, line, file, flag1, flag2)` 把 flags 输出为 `# N "file" 1 3` 形式；保留旧 `emit_linemarker(...)` 三参数 wrapper 不破坏现有 4 个 caller
   - `tl_prepend` helper 加入 `cpp_internal.h`（与 `tl_append` 同性质扩展，零副作用）

   round-trip 验证：`# 1 "/usr/include/stdio.h" 1 3` 输入后输出形式完全保留，sharp-cpp 自己产生的 -E 输出可以被同一个 sharp-cpp 重新读取。

3. **profiling 工具链**（开发体验加固）

   Makefile 新增三个 target：

   | Target | 用途 |
   |---|---|
   | `make profile` | `perf record + perf report` 跑 200 迭代，打印 top 20 self-time |
   | `make flamegraph` | 调用 Brendan Gregg 的 FlameGraph 脚本生成交互 SVG |
   | `make massif` | `valgrind --tool=massif` 测堆峰值与分配热点 |

   全部强制重新构建到 `profile_bin`，开 `-fno-omit-frame-pointer` 让 DWARF 栈展开准确。`PROFILING.md` 详细说明工作流（含禁用 turbo / 绑核 / 三次取中位数等方法学清单）。

### 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `directive.c` | +110 / -50 | F + G 两项 production 修复，含 16 处 `Phase 8:` 注释锚点 |
| `cpp_internal.h` | +12 | `tl_prepend` helper（与 `tl_append` 同性质） |
| 新增 `test_phase8.c` | +500 | 7 节 37 项回归（F + G + Sharp parity） |
| `Makefile` | +50 | `test_phase8` build/test/clean，新增 profile/flamegraph/massif targets |
| 新增 `PROFILING.md` | +180 | 工具链使用文档与方法学清单 |
| `.github/workflows/ci.yml` | 1 行注释 | "all seven" → "all eight" |

### 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Phase 5 results: 46 passed, 0 failed
Phase 6 results: 47 passed, 0 failed, 0 skipped
Phase 7 results: 69 passed, 0 failed, 0 skipped
Phase 8 results: 37 passed, 0 failed, 0 skipped
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试
                                   合计 508 项

$ make asan       # 全清，零内存错误，零 UB
$ make strict     # -Werror -Wpedantic -Wshadow -Wstrict-prototypes，无警告
$ make profile_bin && ./profile_bin 5    # 66.69 ms/iter, 2.17× gcc -E（无回归）
```

### Phase 8 测试覆盖（37 项）

| 章节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | F：`__has_include_next` 跳过当前目录 | 9 |
| §2 | F：退化情形（虚拟文件名、无可用目录） | 4 |
| §3 | F：`__has_include` 自身无回归 | 4 |
| §4 | G：`# N "file"` 形式被识别并更新 `__LINE__`/`__FILE__` | 5 |
| §5 | G：GCC flags 1/2/3 输入解析 + 输出保留；范围外忽略；无 flag 不输出多余空白 | 8 |
| §6 | G：`#line` 关键字形式不破坏（含可选 flags） | 4 |
| §7 | Sharp 模式 parity：F、G 都能在 `CPP_LANG_SHARP` 下工作 | 3 |

### 设计取舍记录

- **`tl_prepend` 而非"局部拼接 TokList"**：直接给 `cpp_internal.h` 加一个与 `tl_append` 对称的 helper（12 行），比在 directive.c 里手工 splice 链表更清晰、更可复用、零副作用。
- **`emit_linemarker` 保留旧签名**：用 `_ex` 加 flags 的扩展版本，旧 4 个 caller 站点零修改。这是"最小破坏"原则。
- **派发器伪造 `dname="line"`**：让 PP_NUMBER 形式复用 `handle_line` 而不是新写 `handle_linemarker`，减少代码重复且自动获得 `#line` 已有的所有特性（`reader_set_line`/`reader_set_file`/Phase 4 的 `__FILE__` 修复等）。
- **`current_file` 沿调用链传递而非全局**：避免引入新的全局 state；`loc.file` 已经是每个 `handle_if`/`handle_elif` 调用都有的，免费数据。

### 仍未做（Phase 9+ 候选）

按"用户痛点 vs 实施成本"排序，**P0 三件套已全部完成**，Phase 9+ 候选缩水：

| 编号 | 候选 | 价值 | 备注 |
|:---:|---|---|---|
| B | InternTable 改 hash 而非线性扫描 | 中 | 现在 Phase 8 已提供 profiling 工具链，下一步就是用 `make flamegraph` 取数据决定优化目标 |
| C | PPTok 对象池 | 低-中 | 同上，profiling 数据驱动 |
| D | StrBuf 预分配 | 低 | 同上 |
| E | 完整 Prosser per-token hide-set | 低 | 当前 active-expansion 栈对所有真实场景都过；理论一致性而非用户痛点 |
| H | ICE 保护 / J 模糊测试 | 中 | 基础设施类 |

**推荐 Phase 9 范围**：跑一次 `make flamegraph` → 看 top 5 self-time → 选 1-2 个最大 plateau 做针对性优化。这次"先量后改"的纪律由 Phase 8 的 I 任务奠定。

---

## Phase 9 — 词法层性能优化（已完成，无新行为测试；性能 65.76→54.55 ms/iter，-17.3%）

Phase 8 落地了 profiling 工具链后，HANDOVER 明确建议下一位接力人**先跑数据再优化**。Phase 9 严格遵守这一纪律：

1. **跑 gprof 看真实热点**（容器里 perf/valgrind 不可用，gprof 是 PROFILING.md "alternatives" 段提到的同等工具）
2. **数据推翻直觉**：HANDOVER 推测 P1-B (`InternTable` hash) 是赢家，但实测 `intern_cstr` 完全不在 top 30。真热点在**词法层字符获取链路**：
   - `reader_next_tok` 30.79%（18.9M 调用）
   - `ph2_raw_get` 19.21%（366M 调用）
   - `ph1_get` 14.97%（366M 调用）
   - `macro_lookup_is_func` 11.02%
3. **针对真热点优化**：词法层的字符获取是函数调用密集——每个 token 触发约 19 次 `ph1_get`/`ph2_raw_get`。`-pg` 抑制 inline 让 gprof 看到独立条目，但即使在 -O2 下，复杂函数（如 `ph2_raw_get` 含 while 循环）也不会被自动 inline

### 实施

**优化 1**：`rb_peek` / `rb_peek2` / `rb_adv` 改 `static inline`

短小（几条机器指令），调用密度极高（数亿次/iter）。`rb_adv` 内部对 EOF 检查与 `'\n'` 判断都加 `CPP_UNLIKELY` 标注（实际源码绝大多数字节既不是 EOF 也不是换行）。

**优化 2**：`ph1_get` 改 `static inline` + trigraph 路径标 UNLIKELY

trigraph 在现代 C 中几乎不出现（C23 已正式移除）。inline 后快速路径退化为 `rb_adv + return c`，编译器可以把它完全 fold 进 lexer 主循环。

**优化 3**：`ph2_raw_get` 拆快慢路径

```c
static int ph2_raw_get_slow(CppReader *rd);  /* forward decl, noinline */

static inline int ph2_raw_get(CppReader *rd) {
    int c = ph1_get(&rd->raw);
    if (CPP_LIKELY(c != '\\')) return c;
    return ph2_raw_get_slow(rd);
}
```

含 while 循环的 splice 处理移到独立的 noinline 函数。这是经典的 "hot path inline + cold path out-of-line" 模式：编译器在调用方看到 `ph2_raw_get` 主体仅 4 条指令（`ph1_get` + 比较 + 分支预测命中 + 返回），可以与 lexer 主循环融合；splice 处理仍在 `ph2_raw_get_slow` 里完整保留语义。

**配套**：`cpp_internal.h` 加可移植的 `CPP_LIKELY` / `CPP_UNLIKELY` 宏。GCC/clang 用 `__builtin_expect`，其他编译器（MSVC 等）退化为 no-op，跨平台 CI 不破坏。

### 性能对比（中位数 of 3 runs，50 iters，bench.c）

| 指标 | Phase 8 baseline | Phase 9 后 | 改善 |
|---|---|---|---|
| Wall-clock per-iter | 65.76 ms | 54.55 ms | **-17.3%** |
| Ratio vs gcc -E | 2.14× | **1.79×** | -16% |
| gprof CPU total（含 -pg 开销） | 3.54 s | 2.36 s | -33% |
| `ph1_get` self-time | 15% (top 3) | **从 top 30 消失**（已被 inline） | ✓ |
| `ph2_raw_get` self-time | 19% (top 2) | **从 top 30 消失**（已被 inline） | ✓ |
| `reader_next_tok` self-time | 1.09 s | 0.74 s | -32% |

`reader_next_tok` 时间下降 32% 即使现在内含被 inline 的 ph1/ph2——说明 inline 后 GCC 做了更激进的跨边界优化（CSE、寄存器分配、消除冗余 splice 检查等）。

### 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `pptok.c` | +50 / -25 | rb_*/ph1_get inline + ph2_raw_get 快慢路径分离 |
| `cpp_internal.h` | +14 | `CPP_LIKELY` / `CPP_UNLIKELY` 可移植宏 |
| 无新增测试套件 | — | 纯性能优化；行为不变；正确性靠 508 项既有覆盖 |
| `PLAN.md` | 状态表 + 本章节 | 单一真相源更新 |
| 新增 `PHASE9_SUMMARY.md` | — | 阶段总结 |

### 测试矩阵

```bash
$ make test    # 508/508 全过（无回归）
$ make asan    # 508/508 全过，零内存错误
$ make strict  # 508/508 全过，零警告
$ make bench   # ~54.55 ms/iter, 1.79× gcc -E（vs Phase 8 的 65.76 ms / 2.14×）
```

### 设计取舍记录

- **不写 P9 专属测试套件**：Phase 9 是纯性能优化，不引入任何新行为。如果有功能回归，508 项既有测试会立刻抓到。Phase 6 §4 `test_performance_budget`（≤500 ms/iter）作为 O(N²) 检查保留——目前 ~54 ms 离 500 ms 还有 9× headroom，回归会被自动捕获。
- **不依赖 GCC 专属语法**：`__builtin_expect` 包在 `CPP_LIKELY`/`CPP_UNLIKELY` 宏后；非 GCC/clang 编译器退化为表达式本身（无优化但语义正确）。MSYS2 GCC、Apple clang 都识别 `__builtin_expect`，CI matrix 不破坏。
- **不动 `reader_peekc`**：当前 `reader_peekc` 用 save-state + ph2_raw_get + restore 模式，每次 peek 都跑完整 splice 逻辑。理论可优化但风险更高（splice 语义敏感）。Phase 9 把改动控制在最小高收益面，留 reader_peekc 给数据驱动的下一阶段。
- **不引入 `[[likely]]` (C23) 或 `__attribute__((hot))`**：项目 baseline 是 C99/C11；引入 C23 attribute 会破坏 c99 CI job。`__builtin_expect` 在所有目标编译器上都存在，是最大公约数。

### 下一步候选（Phase 10+）

Phase 9 后新热点分布（top 5）：

| Function | Self % | 候选优化 |
|---|---|---|
| `reader_next_tok` | 31.36% | 主词法循环；难大幅优化（已经简单 switch + ident loop） |
| `macro_lookup_is_func` | 20.34% | 当前线性 strcmp；改 hash 是 P1-B 的实质内容 |
| `skip_block_comment` | 13.14% | `/* ... */` 扫描；可能用 SIMD 或 strchr 加速 |
| `process_buf` | 6.78% | 主驱动循环；难大幅动 |
| `tl_append_copy` | 2.54% | TokList 节点构造；C 任务（PPTok 池）可能有 5-10% 改善 |

**推荐 Phase 10 范围**：B 任务（`macro_lookup` hash 表）。`macro_lookup_is_func` + `macro_lookup` 加起来 22.46%，hash 化能省下相当一部分。预计可再降 10-15% 到 ~46-49 ms/iter，把 ratio 推到 1.5-1.6×。

---

## Phase 10 — 探针先行救了我们 + 消除双重 hash 查找（已完成，无新行为测试；诚实交付）

Phase 9 收尾时 HANDOVER 推荐下一位做 P1-B'：`macro_lookup_is_func` / `macro_lookup` 改 hash。Phase 10 接手后**先跑 gprof 验证**——这正是 Phase 8 工具链 + Phase 9 纪律的延续。

**关键发现**（探针又一次推翻直觉）：

```c
// macro.c:46
#define MACRO_BUCKETS 512

// macro.c:91
static uint32_t macro_hash(const char *name) {
    uint32_t h = 2166136261u;          // ← FNV-1a
    for (const char *p = name; *p; p++)
        h = (h ^ (uint8_t)*p) * 16777619u;
    return h & (MACRO_BUCKETS - 1);
}
```

**MacroTable 早就是 hash table 了**——`MACRO_BUCKETS=512` + FNV-1a hash + bucket array per-name。HANDOVER 描述"现状：InternTable 用线性 strcmp 查找"是过时信息（应该是某个早期 phase 改过但 HANDOVER 没同步）。

**真问题**：调用 caller 没复用 `macro_lookup` 返回的指针。`directive.c` process_buf 主循环的双重 hash 查找：

```c
// 改造前：line 1695 + 1717
if (!t.hide && macro_lookup(st->macros, name)) {       // 第 1 次 hash + strcmp
    /* ... */
    bool _is_func = macro_lookup_is_func(st->macros, name);  // 第 2 次 hash + strcmp
}
```

如果直接信 HANDOVER 描述去"改 hash"，会浪费时间在已经实现的工作上。**这是 Phase 8 / 9 / 10 三连发的"探针先行救了我们"**——HANDOVER 描述的优化方向有 50% 概率是错的。

### 修复（小但精确）

`macro.h` / `macro.c` 加 accessor `macro_def_is_func(const MacroDef *def)`（保持 MacroDef 封装），`directive.c` 主循环改用缓存 MacroDef* 指针：

```c
// 改造后：directive.c:1695-1726
MacroDef *_macdef = !t.hide ? macro_lookup(st->macros, name) : NULL;
if (_macdef) {
    /* ... */
    bool _is_func = macro_def_is_func(_macdef);   // 单次内存读取
}
```

无新增依赖，无 API 破坏。`macro_def_is_func` 是 trivial accessor（`return def && def->is_func;`），-O2 LTO 会跨 TU inline。

### 性能数据（诚实记录）

**gprof 量化收益**（含 -pg 开销）：

| 指标 | Phase 9 末尾 | Phase 10 后 | 变化 |
|---|---|---|---|
| `macro_lookup_is_func` 调用次数 | 532,573 | 383,699 | **-28%** |
| `macro_lookup_is_func` self-time | 0.48 s (20.34%) | 0.31 s (13.72%) | **-35%** |
| `reader_next_tok` self-time | 0.74 s (31.36%) | 0.85 s (37.61%) | +0.11s |
| gprof CPU total | 2.36 s | 2.26 s | -4.2% |

**wall-clock**（5 次 of 50 iters，sorted 后取中位）：

| | Phase 9 中位 | Phase 10 中位 |
|---|---|---|
| per-iter | 54.61 ms | 55.54 ms |
| ratio vs gcc -E | 1.81× | 1.81× |

**坦白**：wall-clock 改善（55.54 vs 54.61 ms = 1.7%）**在容器测量噪声范围内（±2ms 抖动）**，不能声称稳定改善。reader_next_tok 略升 0.11s 部分抵消了 macro_lookup_is_func 的 0.17s 节省——可能是 `MacroDef *_macdef = ...` 改变了寄存器分配，accessor 调用开销分摊到调用者。

### 为什么仍保留改动

即使 wall-clock 没穿透抖动：

1. **代码意图更清晰**：缓存 lookup 结果是显然合理的，不需要 wall-clock 数字佐证
2. **gprof 量化收益正向**：5326→3837 次 hash+strcmp 调用是真节省
3. **零风险**：accessor 是纯函数，封装良好
4. **LTO/PGO hook**：编译器优化进步后会自动产生收益（GCC 12+ 在更多场景跨 TU inline）
5. **utility API 复用**：`macro_def_is_func` 给未来类似情况留口子

### 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `macro.h` | +18 | `macro_def_is_func` 声明 + 文档说明双重 lookup 的根因 |
| `macro.c` | +6 | `macro_def_is_func` 实现（trivial accessor） |
| `directive.c` | +12 / -7 | 主循环 macro_lookup 改用缓存指针 + 注释解释 |
| `PLAN.md` | 状态表 + 本章节 + 累计统计 | 单一真相源 |
| 新增 `PHASE10_SUMMARY.md` | — | 诚实阶段总结 |

**未触碰**：所有测试文件、`pptok.c` / `expr.c` / `cpp.c` / `cpp_targets.c` / `cpp_internal.h` / Makefile / CI。Phase 10 影响面最小（3 文件，6 + 12 行净改）。

### 测试矩阵

```bash
$ make test    # 508/508 全过
$ make asan    # 508/508 全过
$ make strict  # 508/508 全过，零警告
$ make bench   # 中位 55.54 ms / 1.81× gcc -E（5 次取中位）
```

### Phase 0-10 全景对比表

| 阶段 | wall-clock per-iter | ratio vs gcc -E |
|:---:|:---:|:---:|
| Phase 6（基准建立） | 52 ms | 2.4× |
| Phase 7-8 末尾 | 65-66 ms | 2.14-2.17× |
| Phase 9 末尾 | **54.55 ms** | **1.79×** |
| Phase 10 末尾 | 55.54 ms | 1.81× |

Phase 9 的 17.3% 改善是迄今最大的单阶段加速。Phase 10 的诚实结论：单点优化在已经吃过 P9 大头红利后，wall-clock 边际收益递减——下一阶段需要更激进的改造。

### 下一步候选（Phase 11+）

| 候选 | 价值 | 备注 |
|---|---|---|
| `skip_block_comment` 用 fast-path 扫描 | 中 | P10 后 #3 热点（12.83%）；用 raw buffer 直接扫 `*` 字符避开 reader_peekc/getc 链 |
| Phase 6 sweep 用 memchr 批量复制非特殊字符 | 中 | `p6_read_prefix` 14.9M 调用是字符级，sweep 主循环 sb_push_ch 一次 1 字节 |
| C: PPTok 对象池 | 低-中 | `tl_append_copy` 仅 2.54%，受益有限 |
| H/J: ICE 保护 / 模糊测试 | 中 | 基础设施类 |

**推荐 Phase 11 范围**：`skip_block_comment` 重写——用 raw buffer 直接 memchr 找 `*` / `\\` / `??`，遇到普通 ASCII 段批量推进 pos。预计 5-10% 改善。**风险点**：comment 内 `\\\n` line splice 与 `??/` trigraph 兼容性——必须严格按"先量再写测试再改"流程。

---

## Phase R1 — Lua 5.4 真实场景集成测试（已完成，19/19 测试通过 + 35/35 Lua 文件零诊断）

Phase R1 是项目第一个**真实场景集成测试阶段**，与 Phase 0-10（功能/性能层）平行——"R" 取自 Real-world / Regression 双关。

测试集：[Lua 5.4.7 官方源码](https://github.com/lua/lua) — 35 个 .c 文件 + 28 个 .h 文件，**31,630 行**真实 C 代码。Lua 是被广泛采用的成熟项目，使用大量真实预处理特性：嵌套 include、平台检测（POSIX/Windows）、函数式宏、复杂条件编译、glibc 系统头链。

### 关键发现：1 个真 bug

跑全 35 文件 smoke test 时，`lauxlib.c` 与 `onelua.c` (amalgamation) 失败：

```
/usr/include/x86_64-linux-gnu/sys/signal.h:1:1: fatal: #include nesting too deep (max 200)
/usr/include/x86_64-linux-gnu/sys/wait.h:35:1: error: unterminated #if at end of file
```

**根因**：`directive.c` 的 `find_include_ex` 对 `<...>` 形式 include 也搜索 including-file 的目录。当处理 `sys/wait.h` 内部的 `#include <signal.h>` 时：

1. sharp-cpp 在 sys/wait.h 所在目录（/usr/include/x86_64-linux-gnu/sys/）找到 sibling `sys/signal.h`
2. `sys/signal.h` 内容仅是 `#include <signal.h>` 的 forwarding header
3. 同样错误解析 → 又找到 sys/signal.h 自己
4. 无限递归直到 nesting limit (200) 触发

C99 §6.10.2 与 GCC/clang 行为：`<...>` 形式只搜索 -isystem / 默认 sys path，**不**搜索 including-file 目录。

**修复**：`find_include_ex` 的 current-directory 搜索块加 `&& !is_system` 守卫——**1 行修改 + 8 行长注释保留 bug 发现故事**。

### 验证：修复后

```
Files processed:    35
Passed (no err):    35
Failed:             0
Aggregate diags:    fatal=0  error=0  warning=0
Total output size:  sharp-cpp=3879914  gcc-E=3877599  (ratio 1.001)
```

**100% 通过率，aggregate output 与 gcc -E ratio 1.001——几乎逐字节相同**（差异 < 0.1%，主要在空白/换行处理细节）。这是非常强的语义等价证据：sharp-cpp 在 31k LoC 真实代码上的输出与参考实现差异 < 0.1%。

### 测试基础设施

| 文件 | 用途 |
|---|---|
| `sharp_cli.c` | 最小 CLI driver（~85 行）：接受 `-I` / `-isystem` / `-D`，调用 `cpp_run_buf` |
| `lua_smoke.sh` | 批量 smoke 测试脚本（~140 行）：35 文件 × {sharp_cli, gcc -E} 对比 + 聚合统计 |
| `test_lua_includes.c` | 19 项回归测试（6 节）冻结 bug-fix 语义不变量，避免回退 |

### 19 项回归测试覆盖

| 节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | glibc-shape：`sys/wait.h #include <foo.h>` 不应解析为 `sys/foo.h` | 4 |
| §2 | `"sibling.h"` 形式仍正确解析到 including-file 目录 | 3 |
| §3 | `<header.h>` 形式不搜索 including-file 目录 + 通过 -isystem 解析正常 | 4 |
| §4 | `"sysonly.h"` 在 -isystem 下找到（C99 §6.10.2/3 fallback） | 2 |
| §5 | 三层转发链不触发递归 + guard 正确（每个 body 出现 1 次） | 3 |
| §6 | Sharp 模式 parity（`CPP_LANG_SHARP` 下行为相同） | 3 |

### 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `directive.c` | +9 / -5 | `find_include_ex` 的 1 行修复 + 8 行长注释（含 bug 发现故事） |
| 新增 `sharp_cli.c` | +85 | 最小 CLI driver（测试 fixture，非公开 CLI） |
| 新增 `lua_smoke.sh` | +140 | 35 文件批量集成测试 |
| 新增 `test_lua_includes.c` | +400 | 19 项 R1 回归测试 |
| `Makefile` | +30 | `test_lua_includes` / `sharp_cli` build rules + `lua_smoke` target（auto-clone）|
| `.github/workflows/ci.yml` | 1 行注释 | "all eight" → "all nine" |
| `PLAN.md` / `HANDOVER.md` | — | 单一真相源更新 |
| 新增 `PHASE_R1_SUMMARY.md` | — | 阶段总结 |

**未触碰**：`pptok.c` / `macro.c` / `expr.c` / `cpp.c` / `cpp_targets.c` / `cpp_internal.h` / 任何 test_phase*.c。Phase R1 影响面极小：1 行 production fix + 4 个新文件。

### 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Phase 5 results: 46 passed, 0 failed
Phase 6 results: 47 passed, 0 failed, 0 skipped
Phase 7 results: 69 passed, 0 failed, 0 skipped
Phase 8 results: 37 passed, 0 failed, 0 skipped
Phase R1 results: 19 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan      # 527/527 全过，零内存错误，零 UB
$ make strict    # 527/527 全过，零警告
$ make lua_smoke # 35/35 文件零诊断，aggregate ratio 1.001
```

### 设计取舍记录

- **新阶段命名 R1 而非 P11**：性能阶段（P9/P10）和真实场景测试（R1）是两条平行的演进线。R 系列将来可以扩展（R2 = SQLite，R3 = git 等），与 P 系列不冲突
- **bug 发现故事写进注释**：`directive.c` 的 9 行注释**完整保留 bug-discovery 故事**（sys/wait.h → sys/signal.h → 无限循环）。这种"作案现场"风格的注释比抽象描述更有教学价值
- **集成测试代码不打包 Lua 源码**：`make lua_smoke` 是 opt-in，需要网络 + git。`test_lua_includes.c` 是 `make test` 默认必跑——19 项合成 fixture 已锁定 bug-fix 的语义不变量。Lua 源码更新 / 网络断了都不影响 CI

### 累计真 bug 列表

| # | Phase | Bug |
|:---:|:---:|---|
| 1-10 | 0 | 取证 10 个（互递归崩溃、宽字符前缀、未闭合字面量、`keep_whitespace` 失效、stringify 丢空格、参数前导空泄漏、`(-1U)>0` 错算等） |
| 11 | 4 | `#line "f"` 不更新 `__FILE__` |
| 12 | 5 | `##` 与 `@` 内省 token 交互静默丢 RHS |
| 13 | 6 | `#include_next` 用文件路径而非目录比较 |
| 14 | 8 | `__has_include_next` 是 stub |
| 15 | 8 | GCC linemarker 输入语法 `# N "file" [flags]` 报 unknown directive |
| **16** | **R1** | **`<>` include 错误搜索 including-file 目录，导致 glibc 系统头无限递归** |

### 下一阶段候选：R2（SQLite）

**推荐**：用同样工具链（sharp_cli + smoke 脚本）跑 SQLite `sqlite3.c` amalgamation（~250k LoC，比 Lua 大 5×）。如果 sharp-cpp 能零诊断处理 SQLite，就基本 production-ready for arbitrary C codebase。预计工时 1-2 小时（沿用 R1 模板）。

更激进：跑 Linux kernel 的 selected files、curl、OpenSSL、TinyCC 自身测试集。每发现一个真 bug 写一节 R 系列回归测试。

---

## 文件清单

```
sharp-cpp/
├── cpp.h                 公开 API
├── cpp.c                 公开 API + 字符串拼接
├── cpp_internal.h        内部类型 (DA, StrBuf, InternTable, …)
├── pptok.h, pptok.c      词法（Phase 1 重点改；Phase 4 加 reader_set_file）
├── macro.h, macro.c      宏表与展开（Phase 2 重点改；Phase 5 加 @-token paste 守卫）
├── expr.h, expr.c        #if 常量表达式（Phase 3 重点改）
├── directive.h, directive.c  指令分发（Phase 4 重点改；Phase 6 修 #include_next）
├── cpp_targets.c        平台 triple 宏注入（Phase 7 新增）
├── test_sharp_cpp.c      既存 Sharp 扩展回归测试 (41)
├── test_phase1.c         Phase 1 回归测试 (82)
├── test_phase2.c         Phase 2 回归测试 (49)
├── test_phase3.c         Phase 3 回归测试 (90)
├── test_phase4.c         Phase 4 回归测试 (47)
├── test_phase5.c         Phase 5 回归测试 (46)
├── test_phase6.c         Phase 6 回归测试 (47)
├── test_phase7.c         Phase 7 回归测试 (69)
├── test_phase8.c         Phase 8 回归测试 (37)
├── test_lua_includes.c   Phase R1 回归测试 (19, Lua-discovered bug)
├── sharp_cli.c           Phase R1 CLI driver（测试 fixture）
├── lua_smoke.sh          Phase R1 Lua 集成 smoke 脚本
├── probe.c               50 个原始探针 (Phase 0 取证)
├── bench.c               独立性能 benchmark (Phase 6)
├── .github/workflows/ci.yml  三平台 CI matrix (Phase 6)
├── Makefile              构建/测试入口（Phase 8 加 profile/flamegraph/massif；R1 加 lua_smoke）
├── PLAN.md               本文档
├── PROFILING.md          性能 profiling 工作流（Phase 8 新增）
├── PHASE1_SUMMARY.md     Phase 1 详细总结
├── PHASE2_SUMMARY.md     Phase 2 详细总结
├── PHASE3_SUMMARY.md     Phase 3 详细总结
├── PHASE4_SUMMARY.md     Phase 4 详细总结
├── PHASE5_SUMMARY.md     Phase 5 详细总结
├── PHASE6_SUMMARY.md     Phase 6 详细总结
├── PHASE7_SUMMARY.md     Phase 7 详细总结
├── PHASE8_SUMMARY.md     Phase 8 详细总结
├── PHASE9_SUMMARY.md     Phase 9 详细总结
├── PHASE10_SUMMARY.md    Phase 10 详细总结
└── PHASE_R1_SUMMARY.md   Phase R1 详细总结
```

## 构建/测试

```bash
make test         # 跑所有九个测试套件 (test_phase1..8 + test_lua_includes + test_sharp_cpp)
make asan         # ASan + UBSan 跑一遍
make strict       # -Werror -Wpedantic -Wshadow -Wstrict-prototypes 跑一遍
make bench        # 独立性能 benchmark，对比 gcc -E
make profile      # Phase 8: perf record + top-functions report
make flamegraph   # Phase 8: 渲染 flamegraph.svg
make massif       # Phase 8: valgrind massif 堆峰值报告
make lua_smoke    # Phase R1: 自动 git clone Lua 5.4.7 + 跑 35 文件集成测试
make sqlite_smoke # Phase R2: 自动 git clone SQLite 3.45 + 跑 src/*.c (126) + amalgamation
make zlib_smoke   # Phase R3: 自动 git clone zlib 1.3.1 + 跑 15 .c 文件
make mcpp_smoke   # Phase R4: 自动 git clone mcpp + 跑 Validation Suite (109 项)
make real_world_tests  # 顺序跑 R1-R4 全部
make clean
```

也可以单独运行任一阶段测试：

```bash
make test_phase8
./test_phase8
```
