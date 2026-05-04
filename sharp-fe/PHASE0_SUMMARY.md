# Phase 0 — 审计探针

> Status: **COMPLETE**
> 58 个探针全部跑通（37 OK / 1 ISSUE / 20 DECISION），三道质量门全过。
> Phase 1 (lex) 可立即开始。

## 数字

| Metric | Value |
|---|---|
| 探针总数 | 58 |
| OK | 37 |
| ISSUE | 1 |
| DECISION | 20 |
| ASan / UBSan | 0 issue |
| strict (-Werror -Wpedantic -Wshadow -Wstrict-prototypes) | 0 warning |
| 新增文件 | 7 |

## 关键发现

### ISSUE P1.8 — `>>` 是单个 rshift token

**场景**：`Vec<Vec<int>>` 的末尾 `>>`。

**事实**：cpp 严格按照 C11 词法规则把 `>>` 当作单个 right-shift 运算符 token（`CPPT_PUNCT(">>")`)。这在 C 上下文是完全正确的，但在 Sharp 泛型参数列表的上下文里 `>>` 应该是两个独立的 `>`。

**解决方案**（DECISION P9.5 已记录）：Phase 3 parse 的 `parse_generic_args()` 函数遇到 `PUNCT(">>")` 时：
1. 消费该 token
2. 在 parser 内部维护一个"剩余 `>` 计数"变量（`pending_gt = 1`），让下一层 `parse_generic_args()` 用它来关闭

**影响阶段**：Phase 3（parse）。不影响 Phase 1 (lex) 和 Phase 2 (AST)。

### DECISION 摘要

| ID | 类别 | 核心决策 |
|---|---|---|
| P9.1-P9.4 | 名字改编 | 完全按 spec §5.10，`Type__method`，`operator_add__T1__T2` |
| P9.5 | `>>` 拆分 | parse_generic_args 内 token-splitting，不修 cpp |
| P10.1-P10.5 | defer 翻译 | inline cleanup blocks，LIFO，return-temp 模式，goto ban |
| P11.1-P11.5 | cg 架构 | 无 IR，直译 AST→C，weak 符号去重，#line 标记 |
| P12.1-P12.3 | const 类型系统 | ty_is_const() 检查顶层，泛型实参不允许顶层 const |
| P13.1-P13.2 | 公开 API | sharp_compile_tokens/file，复用 CppDiag 类型 |

## 文件改动

| 文件 | 状态 | 说明 |
|---|:---:|---|
| `probe.c` | 新增 | 58 探针（§1-§13，8 个 CPP 类 section + 5 个 DECISION 类 section）|
| `Makefile` | 新增 | probe / test / asan / strict / dist / clean |
| `sharp.h` | 新增 | 公开 API 骨架（SharpCtx, SharpResult, 6 个函数声明）|
| `sharp.c` | 新增 | ctx new/free + compile_* stub + sharp_print_diags |
| `sharp_internal.h` | 新增 | FeDiagArr, FE_ERROR/WARNING/NOTE/FATAL 宏, SharpCtx 布局 |
| `test_helpers.h` | 新增 | CHECK / CHECK_EQ_INT / CHECK_STR_EQ / CHECK_SUBSTR / CHECK_NO_SUBSTR |
| `PLAN.md` | 新增 | 22 阶段完整规划（Phase 0-12 + R1-R10）|
| `HANDOVER.md` | 新增 | AI 接力文档 |
| `PHASE0_SUMMARY.md` | 新增 | 本文 |

## How to reproduce

```bash
cd /home/claude/sharp-fe
make test    # 58 probes: 37 OK, 1 ISSUE, 20 DECISION
make asan    # 同上，ASan+UBSan 零报错
make strict  # 同上，-Werror 零警告
```

## 设计取舍

- **ISSUE P1.8 不修 cpp**：cpp 的 `>>` 行为在 C 上下文是完全正确的。修 cpp 会破坏 cpp 项目的 610/610 测试，得不偿失。正确做法是在 parse 阶段处理，这也是 C++ 编译器的历史选择。

- **20 个 DECISION 探针而不是文档**：DECISION 探针让设计决策成为机器可验证的"第一类公民"——如果未来某个 phase 的 cg 实现与 P11.2 的方案不一致，`grep "DECISION P11.2" PHASE0_SUMMARY.md` 立刻揭示冲突。纯文档做不到这个。

## 给下一位（Phase 1）的接力提示

- Phase 1 scope 与 ISSUE P1.8 无关（lex 不处理泛型参数）——可以直接开始。
- 测试先行：先写 `test_phase1.c` 里的 §1-§9 测试用例，再写 lex.c 实现。
- `SharpTokKind` enum 是整个前端的"词汇表"，一旦在 Phase 1 定型，后续阶段不能随意增删 kind——这是 API 向后兼容要求 I6 在 lex 层的体现。

— Phase 0 接力人
