# Phase 5 交付说明

## 总览

Phase 5 完成 Sharp 扩展打磨。**累计 355 项回归测试全部通过**（41 既存 Sharp + 82 Phase 1 + 49 Phase 2 + 90 Phase 3 + 47 Phase 4 + 46 Phase 5），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

Phase 5 探针对 Sharp 扩展跑了 17 项检测，**Sharp 行为基本扎实**——`@ident` 词法化、宏免疫、`#if @…` 延迟、嵌套延迟、`@` 在 `#define` 体中保留 `CPPT_AT_INTRINSIC` kind 等关键路径都正常工作。**1 个真 bug 修了**（`##` 与 `@` 的交互），其余 4 项是补回归测试。

## 修复

### 1. `##` 与 `@` 内省的交互（真 bug）

**病征**：
```c
#define P(a,b) a##b
P(check_, @dtor)   // 输出只有 "check_"，@dtor 被静默丢弃
```

**根因**：标准 C 的 `##` 语义要求拼接结果是合法 pp-token；`check_` ## `@dtor` 拼接成的 `check_@dtor` 重新词法化得到两个 token（`check_` + `@dtor`），按 §6.10.3.3p3 是 UB，本实现按 clang 行为返回 LHS 不变、丢弃 RHS。Sharp 中这个"丢弃"非常糟糕——`@dtor` 是 Sharp 前端必须看到的内省 token。

**Sharp 规范要求**（v0.10 §"`#` 与 `##`"）：
> 预处理器的 `#`（字符串化）和 `##`（拼接）只在 `#define` 替换列表内生效，与 `@` **无关**。

**修复**：
- `token_paste()` 加 `bool *out_keep_rhs` 出参
- 任一操作数为 `CPPT_AT_INTRINSIC` 时：发 `CPP_DIAG_WARNING`、设 `*out_keep_rhs = true`、返回 LHS 不变
- 两处调用方（参数 RHS 拼接、字面 RHS 拼接）检查 `keep_rhs`：为真则把 RHS 也 append 到结果列表，确保两个 token 都被前端看见

**修复后**：
```c
P(check_, @dtor)   // 输出 "check_ @dtor"，warning 提示
P(@dtor, _suffix)  // 输出 "@dtor _suffix"，warning 提示
```

### 2. 普通 `##` 拼接不受影响

为了不打扰 99% 的合法用例：`CAT(foo, bar)` 仍正常拼成 `foobar`，无诊断。

## 经探针确认无回归（不需修改）

下列 Sharp 行为本来就正确，Phase 5 通过测试套件锁定：

| 行为 | 状态 |
|---|:---:|
| `__SHARP__` 在 Sharp 模式定义为 1，C 模式不定义 | ✅ |
| `__SHARP_VERSION__` 是字符串字面量 `"0.10"` | ✅ |
| `__SHARP_CPP__`/`_MAJOR__`/`_MINOR__` 两模式都定义（值 1/1/0） | ✅ |
| `@ident` 在 Sharp 模式产生 `CPPT_AT_INTRINSIC` token | ✅ |
| `@ident` 在 C 模式 = `CPPT_PUNCT('@')` + `CPPT_IDENT('ident')` | ✅ |
| 裸 `@`（后无标识符）即使 Sharp 模式也是 `CPPT_PUNCT` | ✅ |
| `@has_destructor` 不被同名 `#define has_destructor 999` 替换 | ✅ |
| `@` token 作为函数式宏参数被替换时保留 `CPPT_AT_INTRINSIC` kind | ✅ |
| `@` token 在 `#define` 体内被对象式宏展开时保留 kind | ✅ |
| `#if @intrinsic(T) ... #endif` 整块原样输出（含 `@` token） | ✅ |
| `#elif @intrinsic` 同样延迟 | ✅ |
| 延迟块内的非 `@` token 也能被 Sharp 前端看到（在 `tokens[]` 数组里） | ✅ |
| 嵌套 `#if @ ... #if @ ...` 两个 `#endif` 都正确发出 | ✅ |
| 普通 `#if 1` 嵌套在外层 `#if @` 里时也走延迟路径 | ✅ |
| `#if 0` 死分支里的 `#if @` 不被延迟（不输出） | ✅ |
| `# @has_dtor` 字符串化得 `"@has_dtor"`，不告警 | ✅ |

## Spec 一致性核查

按 v0.10 spec 重新核对了 Sharp 内置宏清单（spec 行 744-745）：**只有** `__SHARP__` 和 `__SHARP_VERSION__`。原计划任务列表中的 `__SHARP_TARGET_OS__` / `__SHARP_TARGET_ARCH__` / `__SHARP_BITS__` 等**不在 spec 范围**——它们由前端编译器（不是预处理器）按目标三元组注入。本阶段未实现是正确的。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `macro.c` | +35 / -10 | `token_paste` 加 `out_keep_rhs` 参数与 Sharp `@` 守卫；两处调用方更新 |
| 新增 `test_phase5.c` | +500 | 46 项 Phase 5 回归 |

## 测试矩阵

```bash
$ make test
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Phase 5 results: 46 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试（保持向后兼容）
                                   合计 355 项

$ make asan       # 全清
$ make strict     # 无警告
```

通过：`-O0 -g`、`-O2`，`-std=c99`、`-std=c11`，ASan + UBSan，`-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror`。

### Phase 5 测试覆盖（46 项）

| 章节 | 内容 | 用例数 |
|---|---|:---:|
| §1 | `__SHARP__` / `__SHARP_VERSION__` 内置（Sharp/C 双模式） | 4 |
| §2 | `__SHARP_CPP__` 系列双模式 | 3 |
| §3 | `CPPT_AT_INTRINSIC` 词法化（Sharp/C/裸 @） | 3 |
| §4 | `@-token` 宏免疫（直接、参数位） | 2 |
| §5 | `@-token` 在 `#define` 体内保留 kind | 1 |
| §6 | `#if @` / `#elif @` 延迟、`tokens[]` 可见性 | 3 |
| §7 | 嵌套延迟（`#if @` 嵌套 + 普通 `#if 1` 嵌套） | 2 |
| §8 | `##` 与 `@` 守卫（RHS @、LHS @、纯 ## 不警） | 3 |
| §9 | `# @ident` 字符串化（不警告） | 1 |
| §10 | C 模式 parity（`@` 是 punct、不延迟、`has_destructor` 是普通宏） | 3 |
| 11 | 死分支不延迟（`#if 0 ... #if @ ... #endif #endif`） | 1 |

## 下一步：Phase 6（最终阶段）

仅剩规模验证：
- 跑通真实 `<stdio.h>` / `<stdlib.h>` / `<string.h>` 系统头
- Linux/macOS/Windows CI matrix
- 性能基准对比 `gcc -E` 在 ~10K 行头文件上的预处理时间
- 如发现实际场景的边界案例，回头补丁
