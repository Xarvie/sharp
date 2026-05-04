# Sharp 语言前端 — 商业级实现规划 v1

> 继承自 `sharp-cpp` 项目（Phase R23 收尾, 610/610 测试 + ASan + strict + 7 个真实代码集成全过）的工程气质。
> **本项目完全由 AI 实施**——设计目标包括"代码不崩盘、AI 交接不崩盘、稳定推进"作为一等约束。
> 单一真相源 = 本文件。每阶段更新状态总览表 + 阶段细节段 + 末尾的"接力补遗"。

---

## 0. 气质继承 — 这个项目长什么样

下面这些不是建议，是**契约**。每位 AI 接手者都要在动手前内化它们；违反契约比代码 bug 更危险，因为它会破坏接力链。

### 0.1 工程纪律

1. **Phase-driven**：每阶段 = 一个明确范围 + 独立测试套件 + 一份 `PHASEN_SUMMARY.md` + 更新过的 `PLAN.md` 状态表 + 打包 tar.gz。
2. **三道质量门**（每阶段交付前必跑且必全过）：
   - `make test`：所有 `test_phaseN` 二进制 0 fail
   - `make asan`：`-fsanitize=address,undefined`，0 内存错误 0 UB
   - `make strict`：`-Werror -Wpedantic -Wshadow -Wstrict-prototypes`，0 警告
3. **基线先全过才能动代码**。当前阶段验证三道门绿后，才能开始下一阶段写代码。如果三道门没过，**先回滚到上一个 tar.gz，对比 diff**，搞清楚什么被动过。
4. **探针先行**：进入新阶段时，先写 standalone `.c` 探针逼出问题清单（"BAD/OK 宏" 模式），分类成 "真 bug" / "预期行为" / "探针自身错误" 三类，再动代码。**不要先想到改什么再改什么**——这是 cpp 项目里反复印证有效的纪律（Phase 8/9/10 都靠它推翻了"想当然的优化方向"）。
5. **根因 vs caller 修补**：发现一处"看起来该修这里"的 bug 时，先问自己 "是不是别处也有"。能用 1 处根因修替代 30 处 caller 修补，永远选根因。cpp 项目 R4 的 `keep_comments=false` 教训：本能反应去 directive.c 修，意识到根因在 pptok.c 后，~30 处 caller 都不用动。

### 0.2 代码风格

- **C11**，扁平目录，所有 `.c/.h` 在一层（不搞 `src/include/test` 三件套）。和 cpp 项目对齐。
- **按职责拆模块，不按层拆**：cpp 拆成 `pptok / macro / expr / directive / cpp / cpp_targets`，前端将拆成 `lex / ast / parse / scope / type / sema / mono / cg / sharp` 八个模块。
- **三层可见性**：`sharp.h` 公开 API；`sharp_internal.h` 内部跨模块共享；`*.c` 私有。**用 opaque struct + accessor 函数封装内部状态**（如 cpp 的 `MacroDef` / `macro_def_is_func` 模式）。
- **每个 bug 修复带 `/* Phase N: <一句话原因> */` 注释**。`grep "Phase [0-9R]+:" *.c` 必须能列出所有阶段性修复——审计与回滚靠这个。
- **错误返回 bool 或 -1 + 推诊断到 diag list**。除 OOM 外不 abort，不 longjmp。
- **Doxygen 风格 `/** */` 注释**：API 详尽但不啰嗦，每个公开函数说明 contract（参数、返回值、所有权）。
- **inline 快路径 / noinline 慢路径**：cpp 项目 Phase 9 引入的模式（`ph2_raw_get` + `ph2_raw_get_slow`），未来在前端热点也照搬。`CPP_LIKELY` / `CPP_UNLIKELY` 已在 `cpp_internal.h` 中可复用。

### 0.3 测试纪律

- **每阶段独立 `test_phaseN.c`**，使用统一 framework（沿用 cpp 项目的 `CHECK / CHECK_EQ_INT / CHECK_STR_EQ / CHECK_SUBSTR / CHECK_NO_SUBSTR`，将抽到 `test_helpers.h`）。
- **测试自洽**：每个 `test_xxx()` 函数自己 new ctx / free，无全局状态、无顺序依赖。ASan 立即抓泄漏。
- **bug 发现故事写进注释**：`directive.c` 在 R1 那 9 行注释完整保留 "sys/wait.h → sys/signal.h → 无限循环" 的发现路径。前端继承这个风格——抽象描述抵不过作案现场。
- **synthetic + real-world 双轨**：合成 fixture 测语义不变量；R 系列真实场景测覆盖度。两者不可互替（cpp 项目用 510 个合成测试都没找到 R1 的 `<>` include bug）。

### 0.4 AI 交接稳定性

这是本项目相比一般工程项目额外强调的一层。

- **`HANDOVER.md` 是给下一位 AI 的信**。结构：
  1. 项目状态一行总结（X/X 测试通过、ASan/strict 状态、最近一阶段 N）
  2. "你需要知道的最重要的 N 件事"
  3. 架构速览（30 秒读完）
  4. 已完成阶段 + 已修真 bug 表
  5. 关键约定
  6. 候选下一阶段 + 推荐第一动作
  7. **坑列表累积更新**（cpp 项目从坑 1 到坑 39，每个坑都是一次"差点踩坏的事"）
  8. 各阶段接力补遗段（每位接力人在末尾追加）
- **`PLAN.md` 是单一真相源**。状态总览表必须实时更新；每阶段细节段必须完整。本文件就是 PLAN.md 的初版。
- **`PHASEN_SUMMARY.md` 中文**。结构：状态、修复、文件改动、测试覆盖、how to reproduce、设计取舍记录。
- **每阶段打包**：`make dist` → `/mnt/user-data/outputs/sharp-fe-phaseN.tar.gz`。可回滚到任一阶段的可运行快照。
- **`str_replace` 精准修改，不允许整文件 rewrite**（除非新增）。重写文件对接力者的影响是核弹级的——上一位的所有 Phase 注释、bug 故事、设计取舍全部蒸发。

### 0.5 商业级具体标准

复刻 cpp 项目的硬指标：

- ✅ 零内存错误（ASan + UBSan 全跑全过）
- ✅ 零警告（`-Wpedantic -Wshadow -Wstrict-prototypes -Werror`）
- ✅ 跨平台 CI matrix：Linux × {gcc, clang} × {c99, c11} + macOS + Windows MSYS2
- ✅ 真实代码端到端验证（R 系列）
- ✅ 性能 benchmark + flamegraph + massif 内存分析
- ✅ 诚实的设计取舍记录（"做不到 X" 比 "假装 X 已经做到" 更可靠）

---

## 1. 项目目标与不变量

### 1.1 目标

实现 Sharp 语言（spec v0.11）的**前端**：从 `sharp-cpp` 输出的 pp-token 流，到生成可被任何 C11 编译器（gcc / clang / msvc）零警告编译的 C11 源代码。前端 + cpp 一起构成完整的 `sharpc` 命令行工具。

具体说，前端负责：

| 职责 | 模块 |
|---|---|
| 二阶段词法（pp-token → typed token，关键字识别，字面量类型化） | `lex` |
| AST 节点定义 | `ast` |
| 递归下降解析（C 完整 + Sharp 扩展） | `parse` |
| 名字解析与作用域 | `scope` |
| 类型系统（C 类型 + const + struct + 泛型类型） | `type` |
| 语义分析（类型检查、方法分派、运算符解析、defer 校验、@ 内省求值） | `sema` |
| 单态化（泛型实例化引擎） | `mono` |
| C 代码生成 | `cg` |
| 公开 API + 驱动 | `sharp` |

### 1.2 不变量（每阶段必须保持）

I1. **三道门全过**：`make test` / `make asan` / `make strict` 三者都 0 fail / 0 警告。

I2. **零运行时**。前端**不**生成任何辅助运行时库代码（没有 `__sharp_alloc`、没有 `__sharp_drop`）。所有 Sharp 特性必须脱糖为不需要额外库的纯 C11。

I3. **生成的 C 代码可被任意 C11 编译器零警告编译**。我们生成的 `.c` 配合 `-Wall -Wextra -Wpedantic` 应当 0 警告（这是验证"生成代码质量商业级"的最严格闸门）。

I4. **错误信息直达 .sp 源码行号**。所有 Sharp 层错误绑定原始 location（继承自 cpp 的 `CppLoc` 类型），不依赖 cc 报错。

I5. **`Phase N:` 注释完整可审计**：`grep "Phase [0-9R]+:" *.c` 列出所有阶段性修复。

I6. **公开 API 向后兼容**：跨阶段，`sharp.h` 中已发布的函数签名不删不改，只增。删/改需独立"破坏性变更阶段"+ 显式说明 + 双版本期。

I7. **每阶段独立可运行**：阶段 N 末尾的 tar.gz 解压后 `make test` 全过。这是 AI 交接稳定的物理基础——下一位接手时永远能看到一个绿色基线。

I8. **生成 C 代码必须能被 sharp-cpp 自身处理零诊断**。dogfooding 闭环：前端生成的 C → cpp 重新预处理 → 0 fatal/error。这等于在每次提交都跑一次"语义健全性检查"。

### 1.3 非目标（明确不做）

- **不做 LSP**：第一版只做命令行编译器；IDE 集成留作 v2 主题。
- **不做增量编译**：每次全量重译；增量是性能优化主题，留待 R 系列后期。
- **不做错误恢复后继续解析**：发现错误后正常 emit diag 然后 bail；不像 clang 那样积极恢复。这是简化决策——如果未来需要，是独立 phase。
- **不做编译期任意代码执行**：`@` 原语严格只做查询（`@has_operator` / `@static_assert`），不像 Zig 的 `comptime`。这是 spec 已明确的边界。
- **不做属性 / `__attribute__`**：前端透传给 cc。
- **不做 inline asm 解析**：前端透传 `asm(...)` 块给 cc。

---

## 2. 与 sharp-cpp 的对接

### 2.1 输入边界

前端**消费 cpp 的 `CppResult.tokens`**：一个扁平的 pp-token 数组，每个 token 带 `kind / text / len / loc`。在 Sharp 模式下还可能含 `CPPT_AT_INTRINSIC`（`@ident`）。

```c
/* 经典调用流（在 sharpc.c 里）：                           */
CppCtx *ctx = cpp_ctx_new();
cpp_set_lang_std(ctx, 201112L);
cpp_install_target_macros(ctx, "x86_64-linux-gnu");
CppResult pp = cpp_run(ctx, "input.sp", CPP_LANG_SHARP);
if (pp.error) { /* fatal */ }

SharpCtx *sctx = sharp_ctx_new();
sharp_set_target(sctx, "x86_64-linux-gnu");
SharpResult fr = sharp_compile_tokens(sctx, pp.tokens, pp.ntokens, "input.sp");
if (fr.error) { /* fatal */ }

/* fr.c_text 是生成的 C11 源码，喂给 cc 即可 */
```

### 2.2 复用 cpp 的基础设施

不重复造轮子的列表：

- `CppLoc` 类型 → 前端 AST 节点直接持有
- `CppDiag` / `CppDiagLevel` → 前端用同一份 diag 类型
- `cpp_internal.h` 的 `CppDiagArr` / `DA(T)` / `LIKELY` / `UNLIKELY` → 前端复用
- `InternTable`（cpp 内部已有的字符串 intern 表）→ 前端的标识符 / 类型名靠它做 O(1) 比较
- 测试框架宏（`CHECK` 系列）→ 抽到 `test_helpers.h` 共享

### 2.3 不依赖 cpp 内部细节

只依赖 `cpp.h` 的公开 API。`cpp_internal.h` 的访问通过显式转移到一个共享 `internal.h`（命名待定，可能就叫 `sharp_cpp_shared.h`）——避免前端和 cpp 互相侵入对方的私有空间。

---

## 3. 文件布局

```
sharp-fe/                         # 单根目录，与 sharp-cpp/ 平级
├── sharp.h                       公开 API
├── sharp.c                       公开 API 实现 + ctx 管理 + 顶层 driver
├── sharp_internal.h              内部跨模块共享类型与 helper
│
├── lex.h, lex.c                  Phase 1 — 二阶段词法（pp-token → SharpTok）
├── ast.h                         Phase 2 — AST 节点类型定义（无 .c，纯 header）
├── parse.h, parse.c              Phase 3 — 递归下降解析
├── scope.h, scope.c              Phase 4 — 作用域 / 符号表
├── type.h, type.c                Phase 5 — 类型表达式 / 限定 / 类型相等
├── sema.h, sema.c                Phase 6-10 — 语义分析（类型检查/方法/运算符/defer/@）
├── mono.h, mono.c                Phase 11 — 单态化引擎
├── cg.h, cg.c                    Phase 12 — C11 代码生成
├── cpp_targets.c                 沿用 cpp 模块
│
├── sharpc.c                      公开 CLI 驱动器（cpp + 前端 + 调用 cc）
├── sharp_fe_cli.c                前端独立 CLI（仅前端，跳过 cpp，用于测试 fixture）
│
├── test_helpers.h                共享测试框架宏
├── test_phase1.c                 lex 测试
├── test_phase2.c                 ast 测试（构建/打印）
├── test_phase3.c                 parse 测试
├── test_phase4.c                 scope 测试
├── test_phase5.c                 type 测试
├── test_phase6.c                 sema-base（类型检查 / decl / stmt）
├── test_phase7.c                 sema-method（struct 方法 / 关联函数 / const）
├── test_phase8.c                 sema-op（运算符重载与解析）
├── test_phase9.c                 sema-defer（含 goto 限制 / break/continue/return）
├── test_phase10.c                sema-intrinsic（@has_operator / @static_assert）
├── test_phase11.c                mono（泛型单态化）
├── test_phase12.c                cg（C 代码生成 + roundtrip）
│
├── test_e2e.c                    E2E 编译 + cc + 跑出正确输出
├── test_diag_corpus.c            诊断质量黄金测试
│
├── probe.c                       审计探针（Phase 0）
├── bench.c                       性能 benchmark
│
├── *_smoke.sh                    R 系列真实场景脚本（dogfooding / e2e / clang 兼容 …）
├── Makefile
├── .github/workflows/ci.yml      三平台 CI matrix
│
├── PLAN.md                       本文件
├── HANDOVER.md                   AI 接力交接信
├── PROFILING.md                  性能调查工作流
├── PHASE0_SUMMARY.md             审计阶段总结
├── PHASE1_SUMMARY.md             …
├── PHASEN_SUMMARY.md             每阶段独立总结
└── PHASE_R*_SUMMARY.md           R 系列各阶段总结
```

**理由摘要**：
- `ast.h` 纯 header，因为 AST 类型必须被 `parse / sema / mono / cg` 共同看到。这个模式与 cpp 项目里 `cpp.h` 暴露 `CppTok` 类似。
- `sema.c` 首版统一一个文件；超过 2000 行后拆 `sema_method.c / sema_op.c / sema_defer.c / sema_intrinsic.c`。早拆增加 AI 接力的认知负担，晚拆是更稳的策略。
- `sharpc.c` vs `sharp_fe_cli.c`：前者是用户面对的工具（含 cpp + 前端 + cc），后者是测试 fixture（只跑前端，输入是已经预处理过的 token 流或纯 C 源码）。这个分裂复刻 cpp 项目里 `sharp_cli.c` 作为测试 fixture 的角色定位。

---

## 4. 阶段总览

> **状态摘要（Phase S5 收尾）**：Phase 2-12 已通过 S1..S5 系列整体完成，
> 不再分别打包 `test_phase{2..12}.c`，而是合并为 `test_integration.c` /
> `test_import.c` / `test_decls.c`（参考 `PHASE_S{1,2,4,5}_SUMMARY.md`）。
> Phase R1..R10 仍待开始；S5 关闭了 C-superset 路线图，下一步建议见
> `ROADMAP.md` § 8（最近一次 PAUSED AT）。

| 阶段 | 范围 | 状态 | 测试规模 | 关键交付物 |
|---|---|:---:|:---:|---|
| **Phase 0** | 审计探针：58 探针（spec v0.10）+ spec v0.11 update（§14: import/auto）= 66 探针 | ✅ 完成 | 69 探针（44 OK / 2 ISSUE / 23 DECISION）| `probe.c` + 决策清单 |
| **Phase 1** | 二阶段词法：pp-token → SharpTok（含 import / auto 关键字）| ✅ 完成 | 222 / 222 | `lex.c/h` + `test_phase1.c` |
| **Phase 2-12** | AST / parse / scope / type / sema / mono / cg —— 通过 S1..S5 整体完成 | ✅ 完成 | integration 40 + import 25 + decls 56 = 121 项 | `ast.[ch]` `parse.[ch]` `scope.[ch]` `type.[ch]` `sema.[ch]` `cg.[ch]` `import.[ch]` |
| **S1** | ISO C declarator 模型（递归 declarator + storage classes） | ✅ 完成 | C-superset 29/41 | `PHASE_S1_SUMMARY.md` |
| **S2** | typedef + enum + union + switch | ✅ 完成 | C-superset 33/41 | `PHASE_S2_SUMMARY.md` |
| **S4** | initializer lists（含设计 init / 复合字面量）+ bit-field | ✅ 完成 | C-superset 40/41 | `PHASE_S4_SUMMARY.md` |
| **S5** | Lua 5.4.7 端到端集成 + GCC labels-as-values + float 字面量精度 + import.c leak 修复 | ✅ 完成 | 三道门 190/190 + Lua testsuite `final OK !!!` + C-superset 51/52 | `PHASE_S5_SUMMARY.md` + `build_lua_with_sharpfe.sh` |
| **Phase R1** | 第二个真实世界 C 项目（cJSON 1.7.18）—— Sharp 关键字模式分流、GCC 浮点值宏、TY_ARRAY 衰减、`T * const` 发射 | ✅ 完成 | 三道门 193/193 + cJSON 输出与 cc-only 基线**逐字节相同** + C-superset 54/55 | `PHASE_R1_SUMMARY.md` + `build_cjson_with_sharpfe.sh` |
| **Phase R2** | 第三 + 第四个真实世界 C 项目（picol、stb_image）—— `_Thread_local`、K&R 函数类型 typedef、block-scope 匿名 enum 三连环、do-while 单语句体 | ✅ 完成 | 三道门 197/197 + picol/stb_image 双双字节对齐 + Lua/cJSON 0 回归 + C-superset 58/59 | `PHASE_R2_SUMMARY.md` + `build_picol_with_sharpfe.sh` + `build_stb_image_with_sharpfe.sh` |
| **Phase R3** | 第五个真实世界 C 项目（候选：sqlite-amalgamation / mcpp / zlib），或转向 dogfooding（D1） | ⏳ 待开始 | 全部 .c 通过完整管线 + 输出对照 / 或者 sharp-fe 模块用 Sharp 重写 | `build_X_with_sharpfe.sh` 或 `dogfood_smoke.sh` |
| **Phase R3** | 错误信息黄金测试：50+ 故意写错的 .sp/.c，diag 输出与黄金答案逐字节对比 | ⏳ 待开始 | 50+ diag case | `test_diag_corpus.c` |
| **Phase R4** | 跨编译器兼容：生成 C 喂给 gcc + clang + tcc 都零警告 | ⏳ 待开始 | 上述测试 × 3 cc | `cc_compat_smoke.sh` |
| **Phase R5** | 名字改编可逆性 + 调试器友好性：gdb/lldb 能 demangle 出 `Vec<int>::push` | ⏳ 待开始 | 10 个调试场景 | `debug_smoke.sh` |
| **Phase R6** | 跨 TU 实例化去重：同 `Vec<int>` 在 5 个 TU 用，链接后只剩 1 份 | ⏳ 待开始 | 5 项 dedup case | `dedup_smoke.sh` |
| **Phase R7** | 性能 benchmark：sharpc 一个中型 .sp 耗时与 gcc 编译同等 C 代码对比 | ⏳ 待开始 | 性能数据 | `bench.c` + `PROFILING.md` |
| **Phase R8** | 真实小项目：实现一个 ~5K LoC 的 Sharp 程序（如 micro-redis 或 mini-lua），跑通 | ⏳ 待开始 | 5K LoC 程序 | `demo/` |
| **Phase R9** | 模糊测试：AFL/libFuzzer 喂随机 .sp，要求不崩、不挂、不读越界 | ⏳ 待开始 | 24h 不挂 | `fuzz/` |
| **Phase R10** | 与 sharp-cpp 联合稳定性：合并 cpp + fe 的全部 R 系列在一个 CI job 里跑 | ⏳ 待开始 | 全 CI 矩阵绿 | `.github/workflows/ci.yml` |

**累计测试目标**：~1100+ 项（合成）+ R1-R10 的真实场景验证。当前实测：
69 + 222（Phase 1，已合并入 lex 测试）+ 40 + 25 + 56 = **412 项**（不计
S 系列以外的 phase 测试因为 S 系列已替代）。Lua 5.4.7 testsuite 验证
真实世界端到端正确性。

**总规模估计**：
- 生产代码 ~10K LoC C
- 测试代码 ~7K LoC C
- 文档（中文）~6K LoC

---

## 5. 阶段详细规划

### Phase 0 — 审计探针

> **范围**：在写任何前端代码前，写 60+ 个 standalone 探针，针对 spec v0.11 每条规则尝试构造一个最小 case。读 spec → 探针 → 决策清单 → PHASE0_SUMMARY.md。

**输入**：spec v0.11 + cpp 项目（特别是 `cpp.h` 接口、`PHASE_R*_SUMMARY.md` 里的工作风格）。

**输出**：
- `probe.c`（Phase 0 探针 60+，分组：词法 / 类型 / struct / defer / 运算符 / 泛型 / @）
- `PHASE0_SUMMARY.md`（决策清单：每个探针对应哪个阶段，是真要支持的、还是 spec 边界外、还是 phase X 才覆盖）
- 设计决策清单（提交给用户审查）

**关键设计决策（拍板，进入 Phase 1 前用户审核机会）**：
1. **生成的 C 标识符前缀**：考虑 `__sharp_` 还是直接 `Type__method` 形式（spec 偏向后者，因为可读）。**初定**：直接用 spec 给的方案（`Type__method`、`operator_add__Vec3__Vec3` 等），不加前缀。
2. **defer 翻译策略**：inline cleanup blocks vs `goto __cleanup_N + label`。**初定**：inline cleanup blocks（更简单、不产生 goto label，调试器友好）。
3. **@ 内省求值时机**：解析时 vs 实例化时。**初定**：实例化时（因为 `@has_operator(T, +)` 需要 T 已知）。
4. **错误恢复**：遇 parse error 后如何继续。**初定**：sync 到下一个 `;` 或 `}`，不积极恢复（首版简化）。
5. **生成 C 的格式化**：是否做 pretty-print。**初定**：做最小可读化（缩进、换行），但不追求 clang-format 美化。AST 的 cg 阶段直接输出，不走 IR。
6. **名字改编规则**：spec 已给出（`Vec<int>` → `Vec__int`、`operator+` → `operator_add` 等）。**采纳 spec**。

**测试规模**：60+ 探针。

**验收标准**：
- 探针全部能编译通过（即使行为不对也要能运行）
- `PHASE0_SUMMARY.md` 列出每个探针的"现状"（在没做前端的情况下，cpp 单独跑结果是什么）
- 决策清单上列的 6 项设计决策（或更多）每条有明确选择 + 理由

**风险与坑**：
- 风险：探针写得太"未来"，在 Phase 0 跑不起来。**对策**：探针只用 cpp 模块跑一遍，看 token 流是否合理；不要尝试解析。
- 坑预期：`@ident` 在 cpp 里已经是 `CPPT_AT_INTRINSIC`，前端能直接看到——但 Sharp 模式下某些 C 关键字（如 `defer`）也要变 keyword token，这个 cpp 不知道。决策：cpp 不动，前端 lex 阶段做关键字识别。

**接力提示**：如果你接过来时 `make test` 不通过，先 `make probe && ./probe`——所有探针应当编译并跑出有意义输出。失败先回滚到 cpp R23 baseline（`/path/to/sharp-cpp-phaseR23.tar.gz`）。

---

### Phase 1 — 二阶段词法

> **范围**：消费 cpp 的 pp-token 流，输出 SharpTok（带 `kind / text / value / loc`）。完成关键字识别、字面量类型化、`@ident` 透传。

**关键设计**：
- `SharpTok` 的 `kind` 包括：所有 C 关键字（`if / else / while / for / return / struct / typedef / sizeof / ...`）+ Sharp 关键字（`defer`）+ punctuator 细分（`PLUS`、`STAR`、`ARROW`、`LBRACE` 等单独 enum 值，比 cpp 的笼统 `CPPT_PUNCT` 细）+ 字面量（`INT_LIT` / `FLOAT_LIT` / `CHAR_LIT` / `STRING_LIT`）+ `IDENT` + `AT_INTRINSIC`。
- 字面量值的解析在 lex 完成（不留给 parse）：整型字面量已经求值为 `int64_t / uint64_t`，浮点求值为 `double`，字符串保留 escape 解码后的字节序列。
- 关键字识别用静态 hash table（perfect hash 不必要，但 InternTable 一次 lookup 即可分派）。
- `@ident` 透传：lex 不查 spec 里的"原语集合是否封闭"——那是 sema 阶段的事。

**输出**：
- `lex.h`：`SharpTok` 类型 + `SharpTokKind` enum + `lex_run(...)` 函数
- `lex.c`：实现（约 800-1200 行）
- `test_phase1.c`：~80 项测试

**测试覆盖（章节计划）**：
1. §1 关键字识别（C 关键字 + `defer`）— 15 项
2. §2 punctuator 细分 — 12 项
3. §3 整型字面量值（含 hex/oct/bin、后缀）— 15 项
4. §4 浮点字面量值 — 8 项
5. §5 字符常量 — 6 项
6. §6 字符串字面量（含 phase 6 拼接交互）— 6 项
7. §7 `@ident` 透传（Sharp 模式）— 6 项
8. §8 错误诊断（无效 token / 整数溢出等）— 8 项
9. §9 location 保留（每个 token 的 .loc 与原 pp-token 一致）— 4 项

**验收标准**：测试 80/80 + ASan + strict 全过 + 探针中"词法相关"项全绿。

**风险与坑**：
- **坑预期 1**：cpp 输出的 token 已经过 phase 6 拼接（如果开启），lex 不要重复拼接。
- **坑预期 2**：`@ident` 的 `text` 含 `@` 字符，关键字识别要先排除 AT_INTRINSIC kind 才能去查。
- **坑预期 3**：整型字面量溢出 `int64_t` 时如何处理——参照 cpp expr.c 的做法（intmax_t + is_unsigned 标志），否则 `0xFFFFFFFFFFFFFFFF` 等会出错。

**接力提示**：lex 阶段最容易出的问题是字面量值的边界——整型后缀大小写组合、unicode escape、八进制 vs 十进制歧义。Phase 0 的探针清单里关于字面量的项必须都过。`test_phase1` 跑过后，跑一下 `make probe` 看探针清单是不是收紧了（更多 OK，更少 BAD）。

---

### Phase 2 — AST 类型定义

> **范围**：定义所有 AST 节点类型 + 构建/打印基础设施。这是后续所有阶段的"语言"，必须一次设计正确。

**关键设计**：
- AST 节点用 tagged union 模式：`AstNode { AstKind kind; union { AstFunc f; AstStruct s; AstStmt stmt; ... } u; }`。
- 节点持有 `CppLoc`（继承自 cpp）。
- 所有 list（参数、声明、语句序列）用 `DA(AstNode*)` 即 cpp 项目的 dynamic array 模式。
- AST 不持有 owns 类型表达式——`type` 模块单独管类型。AST 只持有 `TypeRef`（解析后的类型 ID）或未解析的 type-token-list（解析阶段尚未类型化时）。
- 提供 `ast_print(node, indent)` 用于调试。

**节点类型清单**（首版）：

```
顶层：
  AstFile（一个 .sp 文件）
  AstFuncDecl, AstFuncDef
  AstStructDecl, AstStructDef
  AstTypedefDecl
  AstVarDecl

类型：
  AstTypeRef（占位，type 模块解析后填充）
  AstTypeName（解析阶段的纯文本表达式）
  AstGenericArgs（<int, float>）

声明：
  AstParamDecl
  AstFieldDecl

语句：
  AstBlock, AstIf, AstWhile, AstFor, AstDoWhile
  AstReturn, AstBreak, AstContinue, AstGoto, AstLabel
  AstExprStmt, AstDeclStmt
  AstDefer, AstDeferBlock

表达式：
  AstIdent, AstIntLit, AstFloatLit, AstStringLit, AstCharLit
  AstBinOp, AstUnaryOp, AstAssign
  AstCall, AstMethodCall, AstFieldAccess（`.` / `->`）
  AstIndex, AstAddrOf, AstDeref
  AstCast, AstSizeof, AstCompoundLit, AstStructLit
  AstTernary, AstComma
  AstAtIntrinsic（@has_operator / @static_assert）
```

**输出**：`ast.h`（纯 header）+ `test_phase2.c`（~40 项：构建每种节点 + 打印 + 释放无泄漏）。

**验收标准**：
- 测试 40/40 + ASan
- `ast_print` 能打印每种节点
- 节点 free 函数不泄漏

**接力提示**：AST 设计是首版最容易做错的——一旦写多了 parse/sema 后再回头改 AstNode 是痛苦的。**进入 Phase 3 前 AST 类型必须 freeze**。如果接力到 Phase 3 后发现 AST 不够用，按 cpp 项目的纪律：**做最小补充，不重构**。等到一个独立的"AST refactor 阶段"再大改。

---

### Phase 3 — 递归下降解析

> **范围**：把 SharpTok 流解析为 AST。**不做任何语义检查**——只看语法。
> **Spec v0.11 新增**：`import` 语句（无头文件模块系统）+ `auto` 类型推导。

**关键设计**：
- 一个文件 `parse.c`（首版预计 1500-2000 行），按 spec 顺序组织：
  - module-level: `parse_import`（两遍导入：Pass 1 扫声明，Pass 2 解析函数体）
  - decl-level: `parse_struct_def`, `parse_func_def`, `parse_typedef`, `parse_auto_decl`, ...
  - stmt-level: `parse_stmt`, `parse_block`, `parse_if`, `parse_while`, `parse_for`, `parse_defer`, ...
  - expr-level: 用 Pratt parser 处理优先级（C 优先级 + Sharp 运算符）
- **`import` 处理**：`import "path.sp";` 由 parse 层读取目标文件、解析声明（不含函数体）并注入当前 scope。宏不穿透（目标文件单独过 cpp，不共享 CppCtx）。重复 import 同一路径是 no-op（路径去重 set）。循环 import 用两遍：Pass 1 注入签名，Pass 2 解析体。
- **`auto` 类型推导**：`auto x = expr;` 的类型节点在 parse 时是 `AstTypeAuto`（占位），sema 阶段填充为推断类型。`auto` 无初始化表达式是 parse error（`auto x;` 非法）。cg 阶段输出具体类型，生成 C 里不出现 `auto`。
- struct 体内方法 / 关联函数 / 运算符 都是普通函数定义，parse 阶段不区分（sema 阶段才区分）。
- 泛型 `<T>` 和模板调用 `Vec<int>` 解析有歧义（`a<b,c>(d)` 是 `(a<b), (c>(d))` 还是 `a<b,c>(d)` 模板调用？）。**对策**：Sharp 在 spec 里用 `Vec<int>` 作类型语法，但调用时类型实参可推断省略，只在显式写时才有 `<` `>`。我们用**回溯解析**：遇 `IDENT <` 时尝试解析为模板调用，失败回到比较运算符路径。
- defer 解析时记录"是否在 defer 块内"标志，遇到嵌套 defer 立即报 parse error（spec 禁止）。
- `@ident(...)` 解析为 `AstAtIntrinsic`，参数列表保留为子表达式（求值在 sema 阶段）。

**输出**：`parse.c/h` + `test_phase3.c`（~150 项）。

**测试覆盖**：
1. §1 顶层 decl（typedef / struct / func / var）— 25 项
2. §2 `import` 语句（路径解析 / 重复去重 / 循环 import）— 12 项
3. §3 `auto` 类型推导（含 `const auto` / `auto*` / 无初始化报错）— 10 项
4. §4 struct 体（字段 / 方法 / 关联函数 / 运算符 / const 方法）— 25 项
5. §5 函数体内的所有语句类型 — 30 项
6. §6 表达式（含运算符优先级 + Sharp 特殊运算符）— 35 项
7. §7 泛型解析（struct/func 定义 + 实例化语法）— 15 项
8. §8 defer 语法（含嵌套禁止）— 10 项
9. §9 错误恢复（malformed input 不崩、给合理 diag）— 10 项

**验收标准**：测试 172+/172+ + ASan + strict 全过 + 解析 spec 综合示例（v0.11 末尾含 `import` / Vec<T> / Vec3 / Buffer）成功生成 AST 不报错。

**风险与坑**：
- **坑预期**：`Vec<int>.new()` 中 `Vec<int>` 是类型，`.new` 是关联函数访问——这跟 C 的 `a.b` 不同。需要特殊处理：当 `.` 左边是 type-name（capital 开头 + 已声明 struct）时，认作关联函数访问。
- **坑预期**：`defer EXPR;` vs `defer { ... }`—— 两种语法都要解析，区分用 lookahead。
- **坑预期**：spec 5.2 提到"struct 字段声明必须在方法之前"——这是 parse 阶段就该校验，还是 sema？**决策**：parse 阶段校验（一旦在 struct 内看到方法 def，后续不允许字段 decl）。

**接力提示**：parse 阶段是前端"骨架"。这之后所有阶段都吃 AST。如果 parse 有 bug，sema 测试失败时的根因定位会非常难。**Phase 3 测试规模故意做大（150 项）**——是因为 parse bug 的成本最高。

---

### Phase 4 — 作用域 / 符号表

> **范围**：建立作用域链 + 把 AST 里的所有 `AstIdent` 解析到对应的 decl。

**关键设计**：
- 作用域类型：global / file（与 global 同层但 -static 限定）/ struct（字段 + 方法名空间）/ function / block。
- 符号表 entry：`{ name, kind (Var/Func/Type/Field/Method), decl_node, scope_id }`。
- 名字解析：从内向外查作用域链；struct 体内的方法/字段在该 struct 的 scope 内。
- **关键决策**：`Vec<T>` 的 T 是 struct scope 的"模板参数"，作用域内可见。
- forward decl 处理：`struct Vec<T>;` 注册名字但不填充字段；`struct Vec<int>;` 是显式实例化标记（spec 5.x）。

**输出**：`scope.c/h` + `test_phase4.c`（~70 项）。

**测试覆盖**：
1. §1 全局/文件作用域 — 8 项
2. §2 函数 + 块作用域嵌套 — 12 项
3. §3 struct scope（字段 + 方法 + 关联函数互相可见）— 15 项
4. §4 名字遮蔽（block 内 var 遮蔽 outer var）— 8 项
5. §5 模板参数作用域（`<T>` 在 struct 体内可见）— 6 项
6. §6 forward decl + 实例化声明 — 8 项
7. §7 名字冲突 / 重定义 diag — 8 项
8. §8 未定义符号 diag — 5 项

**验收标准**：70/70 + 综合示例所有 ident 能被解析到正确 decl。

**风险与坑**：
- **坑预期**：函数参数作用域在哪里——是 function scope 内，还是 outer block？决策：function scope（与 C 一致）。
- **坑预期**：struct 方法体内 `this` 是隐式名字——它在哪个作用域注册？决策：function scope 入口处自动注册一个 `this` symbol，类型为 `T*` 或 `const T*` 取决于方法限定。

**接力提示**：scope 模块的核心数据结构是作用域树。出 bug 时，先 dump 作用域树再 debug——`scope_print(scope_root)` 应当作为标准 debug 助手实现。

---

### Phase 5 — 类型系统

> **范围**：类型表达式 → 类型 ID（intern 表）+ 类型相等 + const 限定 + 隐式转换规则。**不含泛型替换**（那是 mono 阶段）。

**关键设计**：
- `Type` 类型用 tagged union：`{ TypeKind kind; union { TyInt i; TyPtr p; TyArr a; TyFn f; TyStruct s; TyConst c; TyParam param; }; }`。
- **类型 intern**：相同结构的类型共享同一 ID。指针/const/数组复合类型递归 intern。
- **`TyParam`** 表示泛型参数 `T`，标记 struct/func 它属于哪个泛型 decl + 索引（mono 阶段替换它们）。
- C 类型映射严格按 spec：char=8, short=16, int=32, long=32/64（按 target triple，复用 cpp_targets.c），long long=64, float=32, double=64。
- const：作为 `TyConst { Type *base }` 包裹其他类型。`ty_is_const(t)` 检查顶层 const，`ty_unconst(t)` 剥离一层。
- 类型相等：`ty_eq(a, b)` 因为 intern 退化为 `a == b`（指针相等）。

**输出**：`type.c/h` + `test_phase5.c`（~100 项）。

**测试覆盖**：
1. §1 基本类型（intmap / float / void / bool）— 10 项
2. §2 指针 / 数组 / 函数类型 — 15 项
3. §3 const 限定（顶层 / 指针 pointee / 多层）— 20 项
4. §4 struct 类型（无泛型）— 10 项
5. §5 类型 intern 唯一性 — 10 项
6. §6 类型相等 / 子类型 — 10 项
7. §7 隐式转换（`T → const T`、整型提升、float→double 等）— 15 项
8. §8 const 在泛型实参的合法性（spec 5.7 表）— 10 项

**验收标准**：100/100 + 探针清单中类型相关项全绿。

**风险与坑**：
- **坑预期**：`const T` 和 `T const` 都合法（C 语法），需要在 parse + type 都接受。
- **坑预期**：spec 表 5.7 关于"哪些类型实参顶层 const 算非法"是关键——需要严格按 `ty_is_const()` 实现，不能凭 grep `const` 字符串判定。

---

### Phase 6 — 语义分析-基础

> **范围**：声明 / 语句 / 非方法-非运算符表达式的类型检查。这是 sema 的"骨干"，后续 7-10 在它上面叠加 Sharp 特性。

**关键设计**：
- `sema.c` 入口 `sema_check_file(ast_file, scope, types) -> diag_list`。
- 类型检查是从下到上的 AST 递归：表达式节点带 `inferred_type` 字段（在 `ast.h` 已留好），sema 遍历时填充。
- 错误处理：发现类型错误 → 推 diag → 把节点 inferred_type 设为 `TY_ERROR` → 父节点遇到 TY_ERROR 时不再报错（避免级联）。

**测试覆盖（章节计划）**：
1. §1 声明类型检查（含 const 初始化、init 类型匹配）— 25 项
2. §2 stmt：return 类型匹配 / break/continue 在循环内 / goto label 存在 — 20 项
3. §3 表达式类型推断（算术 / 比较 / 逻辑 / 位运算）— 25 项
4. §4 函数调用类型检查 — 15 项
5. §5 数组 / 指针 / 字段访问 — 15 项
6. §6 赋值规则（含 const 检查）— 12 项
7. §7 错误信息质量（错误位置 / 含义清晰）— 8 项

**验收标准**：120/120+（含 auto 推断项）。

**接力提示**：Phase 6 的边界是"不碰方法不碰运算符不碰 defer 不碰 @"。比如 `vec.size()` 暂时报 unknown method（Phase 7 才接管）；`a + b` 暂时只支持基本算术（Phase 8 才接管）。**坚守边界**——每个 phase 都按职责切，违反这条 AI 接力会乱套。

---

### Phase 7 — 语义分析-方法

> **范围**：struct 内方法 + 关联函数 + const 方法分派 + 接收者类型推断。

**关键设计**：
- 方法 vs 关联函数判定：parse 阶段记录"struct 体内函数"标志；sema 看函数体是否使用 `this`，是 = 方法，否 = 关联函数（spec 4.4）。
- 调用 `obj.method(args)`：sema 把它转为隐式 `Type__method(&obj, args)` 调用——但**保留 AST 节点为 `AstMethodCall`**，cg 阶段才发出展平后的 C 调用。这让错误信息能保持 `obj.method` 形式。
- const 方法分派：const 接收者只能调 const 方法，否则 diag。
- 接收者类型推断：`p->method()` 自动 deref 一次。
- 关联函数 `Type.func()`：解析时需要知道 `Type` 是 type-name；sema 阶段确认无 `this` 使用。

**测试覆盖**：
1. §1 关联函数 vs 方法判定 — 10 项
2. §2 const 方法分派 — 15 项
3. §3 接收者类型（值 / 指针 / const 接收者）— 15 项
4. §4 调用语法（`.` / `->` / `Type.`）— 12 项
5. §5 错误 case：const 接收者调非 const 方法、非 const 接收者也合法调 const 方法 — 10 项
6. §6 spec 综合示例的方法部分 — 8 项
7. §7 error 质量 — 10 项

**接力提示**：方法分派看着简单实际坑多。最大的坑是"判定方法 vs 关联函数靠是否使用 `this`"——这要求 sema 先看完函数体所有引用才能定夺。**实现策略**：parse 阶段先标记为"待定"，sema 第一遍扫描 body 后定型。

---

### Phase 8 — 语义分析-运算符

> **范围**：struct 内运算符脱糖 + 自由函数运算符 + 解析顺序 + 隐式转换匹配。

**关键设计**：
- 表达式 `a + b` 的解析顺序（spec）：
  1. `typeof(a)` 的 struct 体内 `operator+(rhs)` 且 rhs 类型可匹配 `typeof(b)`
  2. 全局自由函数 `operator+(typeof(a), typeof(b))`
  3. 参数允许隐式转换（如 `float → double`）的匹配
- 多个匹配 → ambiguity error。
- struct 内 `R operator+(Args)` 脱糖为 `R operator+(T this, Args)`（值传 this）。const 后缀脱糖为 `R operator+(const T* this, Args)`。
- 名字改编 spec：`operator+(Vec3, Vec3)` → `operator_add__Vec3__Vec3`，`operator[](Vec<int>, long)` → `operator_index__Vec__int__long`。
- 不允许的运算符：`= += -= *=` 等，spec 已禁。

**测试覆盖**：
1. §1 struct 内运算符 — 15 项
2. §2 自由函数运算符 — 10 项
3. §3 解析优先（struct 内 vs 自由函数 vs 隐式转换）— 15 项
4. §4 ambiguity 检测 — 10 项
5. §5 const 接收者与运算符（`[]` 读 vs 写、算术运算符值传）— 12 项
6. §6 一元运算符（`-` / `!` / `~`）— 8 项
7. §7 名字改编正确性 — 10 项
8. §8 spec 综合示例的运算符部分 — 10 项

**接力提示**：运算符解析里最容易爬不出来的坑是"歧义 vs 偏好"。spec 给的优先顺序看起来明确，实际遇到 `T1 op T2` 双方都有定义时容易判错。**实现策略**：先实现"严格"模式（多匹配直接 ambiguity error），后续按 spec 4.3 给的优先级补 tie-break 规则。

---

### Phase 9 — 语义分析-defer

> **范围**：作用域绑定 + LIFO + break/continue/return 触发 + goto 限制 + 嵌套禁止。

**关键设计**：
- defer 在 sema 阶段记录两件事：
  1. defer 列表（每个 block 一个 stack，`AstBlock.defers: DA(AstDefer*)`）
  2. block 出口处需要 emit 的 defer 序列（cg 阶段用）
- goto 限制：sema 扫描所有 goto，对每个 goto 检查"源 → 目标 label 路径是否跨越 defer"。**实现策略**：每条 goto 走 AST 结构距离，遇到 defer 即拒绝。
- 嵌套禁止：parse 阶段已拒（在 defer 块内的 defer 是 syntax error）。
- break/continue 触发 defer：sema 不需要做什么——cg 阶段在 break/continue/return 处插入 defer 展开即可。

**测试覆盖**：
1. §1 单 defer 在 block 退出时触发 — 8 项
2. §2 多 defer LIFO 顺序 — 6 项
3. §3 嵌套 block 中的 defer 作用域绑定 — 8 项
4. §4 break/continue 触发 — 6 项
5. §5 goto 限制（跨 defer 报错）— 12 项
6. §6 嵌套 defer 报错 — 4 项
7. §7 spec 综合示例的 defer 部分 — 6 项

**接力提示**：defer 的 sema 实现相对纯净（cg 才是脏活）。这一阶段重点是"goto 跨 defer"的检查算法——它需要在整个函数范围内做静态分析。**实现策略**：先收集所有 defer 的 location（按 AST 位置），收集所有 goto 的源 + 目标 label location；对每对 (goto, label) 计算 AST 路径，路径中遇 defer 即 fail。

---

### Phase 10 — 语义分析-`@` 内省

> **范围**：`@has_operator(T, <op>)` + `@static_assert(cond, msg)` 的编译期常量折叠。

**关键设计**：
- spec 明确：`@` 原语集合**封闭**——只有 `has_operator` 和 `static_assert`。其他 `@xxx(...)` 报 syntax error（可能本来就在 lex 拦了，sema 这里再 double-check）。
- `@has_operator(T, +)` 求值：T 必须已知（如果 T 是泛型参数 → 推到实例化阶段；如果 T 是具体类型 → 立即求值）。返回 `bool` 编译期常量。
- `@static_assert(cond, msg)` 求值：cond 必须是编译期常量（含 `@has_operator` 的结果）；为 false 时 emit error 含 msg。
- 实例化时机：sema 这层只能处理具体类型实参的 `@`——遇到 `T` 仍未替换时挂起到 mono 阶段。

**测试覆盖**：
1. §1 `@has_operator` 各运算符 — 12 项
2. §2 `@static_assert` 真 / 假 / 复杂表达式 — 8 项
3. §3 `@` 在泛型函数体内（推到 mono）— 8 项
4. §4 `@` 在 `#if` 条件中（spec 提到 cpp 透传给 sema）— 4 项
5. §5 错误：未定义 `@xxx` 名字 — 4 项
6. §6 错误：cond 不是编译期常量 — 4 项

**接力提示**：`@` 阶段是 sema 的最后一块拼图。它依赖运算符解析（Phase 8），所以"T 是否支持 op" 等价于"在所有定义里能找到 `operator op` 重载吗"。**实现策略**：复用 Phase 8 的运算符查找函数，传"试探模式"标志（不报错，只回布尔）。

---

### Phase 11 — 单态化

> **范围**：泛型 struct / 泛型函数实例化 + 类型实参替换 + 链接器去重 + 名字改编。

**关键设计**：
- mono 引擎的入口：扫描全程序所有 `Vec<int>` 等具体类型用法，对每个产生 `(GenericDecl, TypeArgs)` 实例化任务。
- 实例化做两件事：
  1. AST 克隆：把泛型 decl 的 AST 复制一份，把所有 `TyParam` 替换为具体类型
  2. 重新跑 sema 7-10（方法 / 运算符 / defer / @）—— 因为类型现在是具体的，先前推迟的检查现在能做
- 链接器去重：用 `weak` / COMDAT 符号（cg 阶段控制），相同名字改编的 instances 由链接器自动去重一份。
- 名字改编（spec 5.10 完整规则）：`__` 分隔层级，运算符符号映射可读名（`+ → add`、`[] → index` 等）。深度优先展开。

**测试覆盖**：
1. §1 泛型 struct 实例化 — 20 项
2. §2 泛型函数实例化 + 类型实参推断 — 15 项
3. §3 嵌套泛型（`Vec<Pair<int, float>>`）— 10 项
4. §4 名字改编正确性（spec 5.10 的所有 case）— 20 项
5. §5 const 在泛型实参的判定（spec 5.7 表）— 15 项
6. §6 实例化点错误（鸭子类型默认 + 显式 `@static_assert`）— 15 项
7. §7 链接器去重（多 TU 同实例化）— 10 项
8. §8 显式实例化 + extern struct（spec 5.6）— 8 项
9. §9 spec 综合示例完整实例化 — 7 项

**接力提示**：mono 是前端最复杂的阶段，预期会发现 Phase 7-10 的 bug——因为合成类型 + 具体替换会触发先前没考虑过的边界。**实现策略**：mono 不修 7-10 的 bug，而是把发现的 case 反向写回对应 phase 的 test_phaseN.c，然后回去修。这是"探针先行"在大场景下的应用。

---

### Phase 12 — C 代码生成

> **范围**：把语义验证 + 单态化后的 AST 翻译为 C11 源代码。

**关键设计**：
- cg 是从 AST 到 C string 的递归。每个 AST 节点有对应 `cg_<node>` 函数。
- struct 方法翻译：`Buffer__alloc(long n)` 等；方法调用 `b.get(0)` 翻译为 `Buffer__get(&b, 0)`。
- defer 翻译：在 block 出口、break、continue、return 之前插入 cleanup 块（inline，逆序）。
- 运算符翻译：`a + b` 翻译为 `operator_add__Type__Type(a, b)`。struct 内运算符脱糖时取地址（如果是 const 方法）或值传（如果是非 const 算术）。
- 链接器去重：所有泛型实例化的 C 函数 emit 时带 `__attribute__((weak))` 或 `inline`（取决于 target；对 MSVC 用 `__declspec(selectany)`）。
- 输出格式：每个声明一行 + 函数体内适度缩进；不追求 clang-format 美化。
- **关键不变量**：生成的 C 文件 + 标准 cc 命令（`gcc -std=c11 -Wall -Wextra -Wpedantic -Werror`）必须零警告通过。这是 I3 不变量的物理验证。

**测试覆盖**：
1. §1 基本类型 + 函数 + struct 翻译 — 20 项
2. §2 struct 方法 + 关联函数 — 20 项
3. §3 运算符脱糖 — 20 项
4. §4 defer 翻译（含 break/continue/return 路径）— 20 项
5. §5 泛型实例化 + 名字改编 — 20 项
6. §6 const 限定保留 — 10 项
7. §7 字面量 + 表达式 + 语句 — 20 项
8. §8 综合：spec 末尾 main() 的完整翻译 — 10 项
9. §9 生成代码零警告通过 cc — 10 项（每个 case 都跑一次 `cc -c -Wall -Wextra -Wpedantic`）

**接力提示**：cg 阶段不要做任何"优化"（不要试图简化生成代码、不要 fold 常量）。AST → C 的翻译尽量"直译"，让 cc 做优化。这一规则在 R 系列发现 cg bug 时极其重要——bug 一定在某个翻译规则的实现里，而不是在某个"优化"里。

---

## 6. R 系列规划 — 真实场景集成

R 系列是"商业级"标签的真正决定因素。cpp 项目里 11 个 R phase（Lua / SQLite / zlib / mcpp / Redis / 自身 / 跨 PP / 标准 mode / native testsuite）每个都发现了合成测试发现不了的真 bug。前端的 R 系列照搬这种设计哲学。

### Phase R1 — E2E 编译 spec 综合示例

输入：spec v0.11 末尾的 `Vec<T> + Vec3 + Buffer` 综合示例。
流程：sharpc → 生成 C → cc 编译 → 跑 → 验证 stdout。
验收：跑出预期输出 `sum = 6` + `u = (5.0, 7.0, 9.0)` + `[0]: 1024 bytes` + `[1]: 2048 bytes`。

### Phase R2 — Dogfooding

策略：用 Sharp 改写 cpp 项目里 1-2 个小模块（如 `expr.c`），重编译跑通既有 600+ 测试。
意义：在真实代码上验证前端的语义正确性。Phase 12 没发现的 cg bug 会在这里浮现。
预期：发现 0-3 个 cg bug。

### Phase R3 — 错误信息黄金测试

50+ 故意写错的 .sp，每个都有"黄金 diag 输出"。
diag 文本变化（措辞改进）需手动更新黄金；diag 数量变化（少 diag 或多 diag）= 真 bug。
测试目的：保证错误信息质量稳定，不因 sema/parse refactor 退化。

### Phase R4 — 跨编译器兼容

把 Phase 12 的 100+ cg 测试 case 喂给 gcc / clang / tcc 三个编译器，每个都要 `-Werror -Wpedantic` 零警告。
预期：发现 cg 中的"GCC 接受但 clang 拒绝"细节（const 修饰位置、隐式转换警告等）。

### Phase R5 — 调试器友好性

10 个调试场景：在 sharpc 输出的 C 代码上设断点，用 gdb / lldb 查看，期望：
- `Vec<int>::push` 名字改编后能用 `info symbol` 还原读
- struct 方法能看到 `this` 指针 + 字段值
- defer 在 break 路径上能停下
验收：每场景都过。

### Phase R6 — 跨 TU 实例化去重

5 个 TU 各自 `import "vec.sp"` 然后 `Vec<int>` 用法不同。链接后 `nm` 看 `Vec__int__push` 应当只出现一次（COMDAT 去重）。

### Phase R7 — 性能 benchmark

`bench.c` 编译一个 5000 行 .sp（自动生成），与 gcc 编译同等 C 代码做 wall-clock 对比。目标：sharpc / gcc 比例 ≤ 3.0×（不追极致——cg 阶段透传给 cc 是主要时间）。

### Phase R8 — 真实小项目

一个 ~5K LoC 的 Sharp 程序——micro-redis 或 mini-lua 复刻——用 Sharp 写 + sharpc 编译 + 跑出正确行为。
意义：spec 的所有特性在真实项目压力下的综合验证。

### Phase R9 — 模糊测试

AFL / libFuzzer 喂随机 .sp 24h。要求：
- 0 崩溃
- 0 hang
- 0 ASan 报错
- 0 越界读

### Phase R10 — CI 联合稳定性

把 cpp + fe 的全部 R 系列在一个 CI job 里跑（Linux × {gcc, clang} × {c99, c11} + macOS + Windows）。
全矩阵绿 = 商业级签收。

---

## 7. AI 交接协议

每位接手者第一动作：

```bash
# 1. 验证基线
make test         # X passed, 0 failed (X = 当前累计测试数)
make asan         # 同上，0 内存错误
make strict       # 同上，0 警告

# 2. 读 HANDOVER.md → 当前阶段、坑列表、推荐下一动作
cat HANDOVER.md

# 3. 读 PLAN.md 状态总览 + 当前阶段细节段
sed -n '/状态总览/,/^---$/p' PLAN.md

# 4. 跑探针
make probe && ./probe   # 现有探针清单 + 当前 ISSUE
```

如果上述任一不通过 → **不要继续往下加功能**。先回滚到上一个 tar.gz 比对 diff，找出动过什么。

### 7.1 PHASE_SUMMARY 模板

```markdown
# Phase N — <一句话范围>

> Status: <COMPLETE / IN PROGRESS / BLOCKED>.
> <累计测试数>/<累计> 全过 + ASan + strict 全清。
> 本阶段发现真 bug X 个（如有）。

## 数字

| Metric | Value |
|---|---|
| 测试新增 | M / M passed |
| 累计测试 | <N> / <N> |
| LoC 改动 | +A / -B |
| 真 bug 修 | X |
| ASan / UBSan | 0 issue |
| strict | 0 warning |

## 修复（如有真 bug）

### Bug N — <一句话>
（背景 / 根因 / 修复 / 测试）

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `xxx.c` | +M / -N | <说明> |

## 测试覆盖

（章节列表 + 每节项数）

## 设计取舍

（明确记录 trade-off + 选择 + 理由）

## How to reproduce

```bash
make test_phaseN
./test_phaseN
```

## 给下一位 (Phase N+1+) 的接力提示

- 推荐第一动作
- 当前已知坑（更新到 HANDOVER 坑列表）
- 测试基线必须先全过

— Phase N 接力人，于 Phase N 收尾时
```

### 7.2 HANDOVER 模板（首版骨架）

`HANDOVER.md` 在 Phase 0 末尾建立首版，结构：

1. 项目状态一行
2. "你需要知道的最重要的 5 件事"
3. 架构速览
4. 当前状态 / 已修真 bug 表
5. 关键约定（修改文件 / 加新功能 / 测试 / 摘要文档）
6. 候选下一阶段
7. **坑列表**（开始时为空，每阶段累加）
8. 立即开始的检查清单
9. 推荐第一动作
10. 各阶段接力补遗段

### 7.3 坑列表起步

从 cpp 项目继承的"通用坑"先入列：

- **坑 1**：基线必须先全过——任何不过的现象都先看是不是上一位动过没改完整。
- **坑 2**：`str_replace` 精准修改——绝不允许整文件 rewrite。Phase N 注释是接力的命脉。
- **坑 3**：探针先行——HANDOVER 描述的"优化方向" / "下一动作"有 50% 概率是过时的，请用代码逼出问题再决定。
- **坑 4**：根因 vs caller 修补——能 1 处根因修就别修 30 处 caller。
- **坑 5**：用 GCC 仲裁——行为不确定时构造最小 case 用真 cc 验证。

前端独有的坑预估（Phase 1+ 会陆续添加）：

- **坑 P1.x**：cpp 输出的 token 已经过 phase 6 拼接（如果开启），lex 不要重复拼接。
- **坑 P3.x**：`Vec<int>.new()` 的 `<` 既是模板括号也是比较运算符——回溯解析需要正确。
- **坑 P7.x**：方法 vs 关联函数判定要看完整个 body 才能定夺，parse 阶段先标记"待定"。
- **坑 P11.x**：mono 阶段发现的 bug 多半在 7-10——回去补 7-10 的 test，然后修，不要在 mono 里 hack。
- **坑 P12.x**：cg 不要做"优化"——直译，让 cc 优化。

---

## 8. 防崩盘机制

### 8.1 阶段隔离

- 每阶段一个独立 tar.gz：`/mnt/user-data/outputs/sharp-fe-phaseN.tar.gz`
- tar.gz 解压后 `make test` 必须立即全过
- 任何阶段失败 → 回滚到上一个 tar.gz，diff 看动过什么

### 8.2 测试规模阈值

- 每阶段测试数 ≥ 上述总览表给出的最小值
- 如果实际跑下来 < 80% 阈值，说明阶段 scope 没做完整 — 回去补
- 累计测试数下降 → 退回上一个 tar.gz，必有 bug

### 8.3 三道质量门 自动化

`Makefile` 必须有：
```
make test    # all test_phase* + all test_e2e* + test_helpers 共享框架
make asan    # CFLAGS += -fsanitize=address,undefined -O0
make strict  # CFLAGS += -Wpedantic -Wshadow -Wstrict-prototypes -Werror
make all     # 三道门顺跑（CI 用）
```

### 8.4 CI matrix（自 Phase R10 起）

`.github/workflows/ci.yml` 三平台 × 多编译器：
- Linux: gcc + clang × c99 + c11
- macOS: clang × c11
- Windows: MSYS2 + MSVC（msvc 仅检查生成的 C 能编过，不跑前端）

### 8.5 回滚 策略

阶段 N 出问题时：
1. 立即停止 forward progress
2. `tar -xzf sharp-fe-phaseN-1.tar.gz` 解压上一阶段
3. 跑三道门确认上一阶段 baseline 仍绿
4. `diff -ru` 比对当前与上一阶段 — 找出"被动过的部分"
5. 决定：a. 仅回滚被动错的文件，保留新增；b. 整体回滚到上一阶段重做

**禁止**：在 broken 状态下"再动一动看看"。这是 cpp 项目里坚守了 23 个阶段没破过的纪律。

---

## 9. 设计决策清单（首批，待用户审查）

下面这些是我自己拍的板，但需要你审过——每条 tick / cross 一下，或给替代方案。改动会传播到对应 Phase 的细节段。

| # | 决策 | 选择 | 替代方案 |
|---|---|---|---|
| D1 | 生成 C 标识符前缀 | 无前缀，直接 `Type__method`（按 spec） | `__sharp_` 前缀（避免与用户代码冲突）|
| D2 | defer 翻译策略 | inline cleanup blocks（无 goto label） | `goto __cleanup_N + label`（更接近 spec 描述）|
| D3 | `@` 内省求值时机 | 实例化时（具体类型已知）| 解析时（更早报错，但泛型场景不可能） |
| D4 | parse 错误恢复 | sync 到 `;` / `}`，不积极恢复 | 像 clang 那样积极恢复 |
| D5 | cg 是否做 pretty-print | 最小可读化（缩进 + 换行），不追求 clang-format | 输出 minified（最小体积） |
| D6 | 名字改编规则 | 完全按 spec 5.10 | 加版本号防冲突（`Vec__int__v0_11__push`）|
| D7 | 链接器去重机制 | `weak` 符号（GCC/Clang）+ `__declspec(selectany)`（MSVC） | 显式实例化 + 单 TU 编译（链接前去重）|
| D8 | 测试规模阈值 | 累计 ~1100+ | 1500+（更严格但工时翻倍）|
| D9 | sema 拆分时机 | 单文件 sema.c 至 ~2000 行后再拆 | 一开始就拆 sema_method/op/defer/intrinsic |
| D10 | 是否单独做 IR | 不做，AST → C 直译 | 加一层 IR（更易优化但首版工时大涨）|
| D11 | 编译目标 C 标准 | 生成 C11（与 cpp 对齐）| C99 / C17 |
| D12 | 错误信息显示风格 | 复用 cpp 的 CppDiag 体系，错误位置精确到 token | 自己一套（更可控但割裂）|
| D13 | 命令行工具命名 | `sharpc`（最终 driver）+ `sharp_fe_cli`（前端 fixture） | `shc` / `spc` / 其他 |
| D14 | dogfooding（R2）改写哪个模块 | cpp 的 `expr.c`（1177 行，self-contained，类型计算多） | `pptok.c` 或 `cpp_targets.c` |

---

## 10. 立即开始 — Phase 0 检查清单

接手 Phase 0 的 AI 第一步：

```bash
# 1. 准备工作目录
mkdir -p /home/claude/sharp-fe
cd /home/claude/sharp-fe

# 2. 拷贝 sharp-cpp 作为依赖
cp -r /path/to/sharp-cpp ./sharp-cpp-vendored   # 或者用 submodule

# 3. 把本 PLAN.md 拷进根目录
cp /mnt/user-data/uploads/SHARP_FRONTEND_PLAN.md ./PLAN.md

# 4. 建立 HANDOVER.md 骨架（基于本文件第 7 节模板）
# 5. 建立 Makefile 骨架（建空 test_phase0、make probe 跑探针）
# 6. 写 probe.c — 60+ 探针，按 spec 章节分组

# 7. 跑探针
make probe && ./probe > phase0_issues.txt

# 8. 写 PHASE0_SUMMARY.md：每个 ISSUE 标注 → 真 bug / 预期 / 推延 phase
# 9. 用户审查决策清单（第 9 节 D1-D14）
# 10. 三道门绿 + 打包 → tar.gz
```

完成时长预估：4-8 小时（探针 + 决策 + 文档）。

---

## 11. 元信息

- **本文件版本**：v1（首版骨架，Phase 0 进入前由 AI 起草）
- **修订日期**：2026-05-03
- **修订者**：AI（即将进入 Phase 0 实施）
- **下一次修订**：Phase 0 收尾时（更新决策清单、补充探针发现的细节、把 Phase 0 状态从 ⏳ 改为 ✅）

---

## 附录 A — 与 sharp-cpp 项目的对照表

| 维度 | sharp-cpp | sharp-fe |
|---|---|---|
| 阶段数 | Phase 0-10 + R1-R23 = 34 | Phase 0-12 + R1-R10 = 23 |
| 累计测试 | 610 | ~1100 目标 |
| 生产 LoC | ~6.5K | ~10K 目标 |
| 测试 LoC | ~4.5K | ~7K 目标 |
| 文档 LoC | ~5.5K | ~6K 目标 |
| 真 bug 数 | 33（含 R 系列发现） | 估计 20-40（含 R 系列发现） |
| 主要风险 | 性能优化触动核心 | 泛型单态化复杂度 + cg 正确性 |
| 三道门 | test / asan / strict | 同 |
| 真实代码集成 | Lua/SQLite/zlib/Redis/mcpp/self/cross | E2E sample/dogfood/diag/cc compat/debug/dedup/perf/小项目/fuzz/CI |

## 附录 B — Phase 标号约定

- **P0-P12**：核心实现阶段（lex → parse → sema → mono → cg）
- **R1-R10**：真实场景集成阶段（与 cpp 项目 R 系列对应）
- 后续如需"性能 / 重构 / 新特性"独立编号：用 **O 系列**（Optimization）或 **F 系列**（Feature）—— 与 cpp 的 P / R 命名风格一致
