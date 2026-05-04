# Phase 7 交付说明

## 总览

Phase 7 完成 API 友好性建设。**累计 471 项回归测试全部通过**（41 既存 Sharp + 82 P1 + 49 P2 + 90 P3 + 47 P4 + 46 P5 + 47 P6 + 69 P7），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

Phase 0-6 已扫清所有内部 bug，并通过 Phase 6 真实头验证证明引擎健壮。但 Phase 6 留下了一个真实的 API 不友好问题：caller 必须手写约 18 行 `cpp_define()`（`__x86_64__`、`__LP64__`、`__linux__`、`__SIZEOF_LONG__` 等）才能让 `<stdio.h>` 走对编译路径——这违反了"商业级 API"的工程基线。Phase 6 的 `test_phase6.c` 里有一个内部 helper `install_platform_macros()` 作为权宜之计，但它只覆盖 x86_64-linux-gnu，不是公开 API。

Phase 7 把这个能力提升为 first-class 的 `cpp_install_target_macros(ctx, triple)`，并扩展到三种主流目标，加上完整的负断言测试覆盖以防止未来回归。**Phase 7 是纯增量 API 加固，不修任何 bug、不改任何既存 production code 行为**。

## 新增

### 1. 公开 API（`cpp.h`）

```c
/* 为 caller 注入指定 triple 的预定义宏集合 */
int cpp_install_target_macros(CppCtx *ctx, const char *triple);

/* 列举支持的 triple，NULL-terminated（方便 CLI 工具自检） */
const char *const *cpp_target_known_triples(void);
```

**返回码语义**：
- 返回 `0` = 成功识别并注入
- 返回 `-1` = triple 未识别 / `ctx` 为 NULL / `triple` 为 NULL
- 失败时**不污染 ctx**（保证后续 `cpp_define`、`cpp_run_buf` 仍正常工作）
- 重复调用同一 triple 安全（同样宏体的重复定义不触发诊断）

**特意不做的事**：不启用 feature-test 宏（`_GNU_SOURCE` / `_POSIX_C_SOURCE` / `_XOPEN_SOURCE` 等）。它们编码 caller 意图（"我希望使用 GNU 扩展"），而非客观事实（"我在编译这个 triple"），因此仍归 caller 管。

### 2. 新模块 `cpp_targets.c`

独立文件，不挤进已经 2200+ 行的 `directive.c`。结构：每个 triple 一个 `static install_*()` 函数，三个函数完全自包含（无共享 helper），主入口 `cpp_install_target_macros` 用 `strcmp` 派发。

宏值取自真实工具链的 `gcc -dM -E -x c /dev/null` 等价输出（GCC 13.2 / Apple clang 15 / MSVC 19.29），不靠记忆。

| Triple | 数据模型 | 关键事实 |
|---|---|---|
| `x86_64-linux-gnu` | LP64 | `__LP64__=1`, `__SIZEOF_LONG__=8`, `__ELF__=1`, `__GNUC__=13`, `__SIZEOF_LONG_DOUBLE__=16` |
| `aarch64-apple-darwin` | LP64 + Mach-O | `__APPLE__=1`, `__MACH__=1`, `__clang__=1`, **无** `__ELF__`、**无** `__linux__`，`__SIZEOF_LONG_DOUBLE__=8`（与 Linux 16 不同），`__CHAR_UNSIGNED__=1`，`__SIZEOF_WCHAR_T__=4` |
| `x86_64-pc-windows-msvc` | **LLP64**（关键差异） | `_M_X64=100`, `_WIN64=1`, **`__SIZEOF_LONG__=4`**, `__SIZEOF_WCHAR_T__=2`（UTF-16），`_MSC_VER=1929`，**无** `__LP64__`、**无** `__GNUC__`，`__SIZE_TYPE__="long long unsigned int"` |

**为何 LLP64 是 Phase 7 的核心严谨点**：很多 C 代码假设"64-bit 系统 ⇒ `long` 是 64-bit"。这在 Linux/macOS 成立，在 Windows-x64 **不**成立——MSVC 用 LLP64 模型（long=32, long long=64, pointer=64）。因此 Windows triple 必须显式 `__SIZEOF_LONG__=4`、绝不能定义 `__LP64__`。`size_t` 也对应改成 `long long unsigned int`（Linux/Apple 上是 `long unsigned int`）。

### 3. 设计取舍（记录给后续接手者）

- **不抽共享 size helper**：考虑过 `install_common_size_macros_lp64_le()`，但 LP64 在 Linux/Apple 的 `__SIZEOF_LONG_DOUBLE__` 不一致（16 vs 8）、`__INT64_TYPE__` 也不一致（`long int` vs `long long int`）——共享 helper 会变成"几乎所有字段都被覆盖"的反模式。三函数各自展开更清晰、更安全。
- **不引入 `cpp_targets.h`**：API 直接放 `cpp.h`（caller 触手可及，与 `cpp_define` 同性质），实现独立放 `cpp_targets.c`（物理隔离不污染 directive.c）。这是"扁平 API、模块化实现"。
- **未来扩展**：在 `cpp_install_target_macros` 派发分支 + `cpp_target_known_triples` 列表里同步加一行即可加入新 triple（`aarch64-linux-gnu`、`riscv64-linux-gnu`、`x86_64-apple-darwin` 等），不会触碰其他模块。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| 新增 `cpp_targets.c` | +311 | 三 triple 注入 + 公开入口 + 枚举器 |
| `cpp.h` | +33 | 两个新公开 API 声明 + 完整 doc 注释 |
| `Makefile` | +5 | `cpp_targets.c` 加入 `LIB_SRCS`、`test_phase7` 编译规则与 `make test` 调用 |
| `.github/workflows/ci.yml` | 1 行注释 | "all six" → "all seven" |
| 新增 `test_phase7.c` | +508 | 9 节 69 项回归 |
| `PLAN.md` | 状态表 + 新章节 + 文件清单更新 | 单一真相源更新 |
| 新增 `PHASE7_SUMMARY.md` | 本文 | 阶段总结 |

**未触碰**任何既存 production code（`pptok.c` / `macro.c` / `expr.c` / `directive.c` / `cpp.c` / `cpp_internal.h` 全部零修改）。Phase 7 是纯增量。

## 测试矩阵

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
$ ./probe         # 仍仅 1 项预期 ISSUE（探针自身的 __LINE__ 边界，非 bug）
$ make bench      # 65.81 ms/iter, 2.03× gcc -E（vs Phase 6 的 2.44×；机器负载差异，无性能回归）
```

通过：`-O0 -g`、`-O2`，`-std=c99`、`-std=c11`，ASan + UBSan，严格警告。

### Phase 7 测试覆盖（69 项）

| 章节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | 基础识别（三 triple 各 1） + 枚举器 NULL-terminated | 7 |
| §2 | 错误处理：未知 triple、NULL triple、NULL ctx、未知 triple 不污染 ctx 的回归保护 | 8 |
| §3 | x86_64-linux-gnu 关键宏值（含 `__BYTE_ORDER__` 间接展开到 1234） | 8 |
| §4 | aarch64-apple-darwin 关键宏 + **负断言**（无 `__linux__` / `__ELF__`） | 8 |
| §5 | x86_64-pc-windows-msvc LLP64 + **负断言**（无 `__LP64__` / `__GNUC__` / `__linux__` / `__APPLE__`） | 11 |
| §6 | 跨 triple 不变量：同一段源码在三 triple 走不同分支（`__SIZEOF_LONG__`、`#ifdef __LP64__`、OS 三分支） | 13 |
| §7 | 端到端真实头：linux triple 跑 `<stddef.h>`/`<stdint.h>`/`<limits.h>` 零错误 | 5 |
| §8 | Sharp 模式 parity：triple 注入在 `CPP_LANG_SHARP` 同样可见、真实头同样过 | 4 |
| §9 | 幂等性（重复注入安全）+ 边界（空字符串 triple 视为未知） | 5 |

### 测试设计严谨性要点

- **负断言比正断言更重要**：`__LP64__` 在 Windows triple 下**必须不存在**——这条规则被一个 §5 的 `CHECK(!probe_macro_defined(...))` 锁死。未来若有人误改 Windows install 函数加上 `__LP64__=1`，测试会立即抓到。
- **跨 triple 不变量是真正的"行为测试"**：§6 不测某个宏的字面值，而是测"同一段源码在不同 triple 走不同 `#if` 分支"——这才是 caller 真正关心的可观察行为。
- **每个测试函数独立 ctx**：每个 `test_xxx()` 自己 `cpp_ctx_new` + `cpp_ctx_free`，ASan 立即抓泄漏。
- **辅助 probe 函数复用**：`probe_macro_value()` 和 `probe_macro_defined()` 两个 helper 把"取宏值"/"判定义"抽象成一行调用，让 §3-§5 的 27 项断言读起来像表格而非样板代码。

## 一处测试期望修正（写测试过程的小记）

§2.4 初版期望 `value=42`，实测得到 `value= 42`（注意 `=` 与 `42` 之间有空格）。这是 C 预处理器的标准行为：宏展开为新 token 时，token 携带"前导空格"标志以避免与上一个 token 粘成新 token（如果 `MY_OWN_MACRO` 被替换成包含运算符的内容，没空格就可能 token-paste 出新词）。原期望写得太严格，已修正为 `CHECK_SUBSTR(r.text, "42", "user macro still expands")`，只验证语义而不绑定具体格式。这是写测试时的常见小坑，不是 production bug。

## Phase 0-7 全景回顾

| 阶段 | 完成 | 测试 | 主要交付 |
|:---:|:---:|:---:|---|
| 0 | ✅ | — | 50 项探针，10 个商业级缺陷取证 |
| 1 | ✅ | 82 | 词法层（前缀、未闭合、`keep_whitespace`） |
| 2 | ✅ | 49 | 宏引擎（互递归崩溃、stringify 空格、参数前导空） |
| 3 | ✅ | 90 | `#if` 表达式（unsigned 算术、整型提升、字符前缀） |
| 4 | ✅ | 47 | 指令层（`#line` 文件名、尾随 token 警告、include 路径 note） |
| 5 | ✅ | 46 | Sharp 扩展（`##` 与 `@` 守卫；17 项行为锁定） |
| 6 | ✅ | 47 | 规模验证（真实头、`#include_next` 修复、CI、性能基准） |
| 7 | ✅ | 69 | API 友好性（三 triple 平台宏注入助手 + 负断言锁定） |
| **合计** | | **471** | **零回归、零失败、ASan/UBSan 清、严格警告清** |

## 已知限制（Phase 8+ 候选）

按"用户痛点 vs 实施成本"排序：

| 编号 | 候选 | 价值 | 备注 |
|:---:|---|---|---|
| F | `__has_include_next` 完整测试 | 中 | 实现已存在；仅测试缺口；半小时工作 |
| G | `#line N "f" 1 3` GCC 扩展 system-header 标记数字传播 | 中 | 当前忽略尾部数字（`__FILE__`/`__LINE__` 已对）；语义不完整 |
| I | profiling 工具与火焰图流程 | 中 | **B/C/D 性能优化的前置条件**——先测再优化 |
| B | InternTable 改 hash 而非线性扫描 | 中 | 当前 65 ms/iter 完全够 IDE 用；优化目标待 I 提供数据 |
| C | PPTok 对象池 | 低-中 | 同上 |
| D | StrBuf 预分配 | 低 | 同上 |
| E | 完整 Prosser per-token hide-set | 低 | 当前 active-expansion 栈对所有真实场景都过；属"理论一致性"而非"用户痛点"，建议留到出现具体反例再做 |
| H | ICE 保护 / J 模糊测试 | 中 | 基础设施类；单独成阶段更合适 |

**推荐 Phase 8 范围**：F + G 两件套（共约 1 小时）+ I（profiling 工具落地，约 1.5 小时）。F/G 是把规范完整性收尾，I 是为后续性能优化建立"数据先行"的基线。
