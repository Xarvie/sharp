# Phase 1 — 二阶段词法（lex）

> Status: **COMPLETE**
> 213/213 测试通过，ASan/UBSan 零泄漏，strict 零警告。
> Phase 2 (AST 类型定义) 可立即开始。

## 数字

| Metric | Value |
|---|---|
| 新增测试 | 213 / 213 passed |
| 累计测试 | 271 (58 probes + 213 phase1) |
| lex.c LoC | 510 |
| lex.h LoC | 171 |
| test_phase1.c LoC | 589 |
| 真 bug 修 | 0（发现 1 个测试代码 bug：lf_free 漏 msg） |
| ASan / UBSan | 0 issue |
| strict | 0 warning |

## 功能覆盖

| §节 | 内容 | 测试数 |
|---|---|:---:|
| §1 | C11 全部 44 个关键字促进 + 非关键字不误报 | 90 |
| §2 | Sharp 关键字 (defer / operator / null / this) + 前缀不误报 | 16 |
| §3 | 50 种 punctuator 细分 | 50 |
| §4 | 整型字面量值（dec / hex / oct / bin）| 16 |
| §5 | 整型后缀（U / L / LL / ULL）+ 高位强制 unsigned | 11 |
| §6 | 浮点字面量值（含 hex float `0x1.8p1`）| 10 |
| §7 | 字符常量（plain / escape / hex / octal / UCN）| 19 |
| §8 | 字符串字面量透传（plain / L / u8 / empty）| 5 |
| §9 | AT_INTRINSIC 透传 + C mode 不误产 | 6 |
| §10 | 诊断健壮性（overflow / empty / whitespace-only）| 4 |
| §11 | 源码位置保留 + EOF sentinel | 6 |

## 关键设计决策

- **关键字查找**：`O(n)` 线性扫描（~50 个关键字），无 hash table。性能
  充分，留待 profiling 驱动优化（cpp 项目 Phase 9-10 教训：不要提前优化）。
- **text 借用语义**：`SharpTok.text` 借用 `CppResult._raw_text`，不 strdup。
  调用者通过 `LexFix` 或等价 RAII 结构绑定二者生命期。
- **`from_macro` 透传**：字段直接从 `CppTok` 拷贝。实测 cpp 当前不填充
  此字段（简单宏体展开时总为 false）——这是上游 known limitation，不是 lex bug。
  测试里记录了这个事实；等 cpp 实现后行为自动正确。

## 文件改动

| 文件 | 改动 | 说明 |
|---|:---:|---|
| `lex.h` | 新增 171 行 | SharpTokKind (60+ kinds) + SharpTok + API 声明 |
| `lex.c` | 新增 510 行 | keyword_kind / punct_kind / parse_int_literal / parse_float_literal / parse_char_literal / lex_run / lex_free |
| `test_phase1.c` | 新增 589 行 | 213 项测试 (§1-§11) + LexFix fixture |
| `Makefile` | +12 行 | FE_SRCS=lex.c; test_phase1 构建规则; test 目标更新 |

## 修复记录

- **测试 bug**：`lf_free()` 原版 `free(f->diags.data)` 直接释放数组，漏释放
  每个 `CppDiag.msg`（由 `fe_emit_diag` 内 `malloc` 分配）。ASan 首跑即发现。
  修复：循环 `free(f->diags.data[i].msg)` 后再 `free(f->diags.data)`。

## How to reproduce

```bash
cd /home/claude/sharp-fe
make test          # Phase 0: 58 probes + Phase 1: 213 passed, 0 failed
make asan          # 同上，零内存错误
make strict        # 同上，零警告
```

## 设计取舍

- `SharpTok` 没有存 `sharp.c` 里可用的 NUL 终止字符串（避免 strndup × ntokens
  的内存开销），代价是所有消费者必须用 `text` + `len`，不能 `printf("%s", text)`。
  这与 cpp 项目里 `CppTok.text` 的做法一致（borrowed + len）。
- `STOK_EXTERN_STRUCT` 在 lex.h 里保留了 slot，但 lex 层不产生它（保留给
  parse 阶段在 `extern struct` 上下文中合并两个 token 时使用）。

## 给下一位（Phase 2）的接力提示

- **Phase 2 scope 明确**：只建 `ast.h`（纯 header，无 .c）+ 构建/打印/free
  基础设施。测试先行：先写 `test_phase2.c`（~40 项），再实现。
- **AST 节点 freeze 是本阶段最重要的约束**：进入 Phase 3 (parse) 之后，
  `AstNode` 布局不允许随意增减字段（其他模块的 `sizeof(AstNode)` 会变），
  只能新增节点种类。
- **坑 FE-4 通用版**：凡是 DA 数组里的元素持有 heap 指针，free 时要两步：
  先 for 循环释放元素内部指针，再 free 数组本身。

— Phase 1 接力人
