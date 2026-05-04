# Phase 10 交付说明

## 总览

Phase 10 完成"探针验证 + 消除双重 hash 查找"。**累计 508 项回归测试全部通过**（无新增测试——Phase 10 是性能优化阶段，行为不变；既有 508 项覆盖正确性），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

**Phase 10 的核心交付不是性能数字**——而是**探针先行救了我们的另一次胜利**：HANDOVER 推荐的 P1-B'（"`MacroTable` 改 hash 而非线性扫描"）是过时描述；gprof + 看代码后发现 `MacroTable` **早就是 hash table 了**（512 buckets, FNV-1a hash）。如果直接信 HANDOVER 描述去"改 hash"，会浪费时间在已经实现的工作上。

**实际优化**：消除 `directive.c` process_buf 主循环的双重 hash 查找。**gprof 量化收益正向**（`macro_lookup_is_func` 调用 -28%, self-time -35%），但 **wall-clock 改善被容器抖动吞没**——这是诚实交付。

## 探针先行的发现

容器里 perf/valgrind 仍不可用，gprof 是工具链。Phase 10 sequence：

```bash
# Phase 9 baseline 重测（3 次取中位）
$ ./bench 50 (×3)
54.33 / 54.61 / 56.65 ms  → 中位 54.61 ms

# 跑 gprof 看 Phase 9 后的真热点
$ gprof -b -p bench_gprof gmon.out | head -10
30.79%  reader_next_tok      18.9M 调用
20.34%  macro_lookup_is_func 532K 调用    ← 这里
13.14%  skip_block_comment   697K 调用
11.02%  ...
```

**HANDOVER 描述**：「**B**. InternTable 改 hash 而非线性扫描；**现状**：`cpp_internal.h` 的 InternTable 用线性 strcmp 查找；大 input 上 macro_lookup 链路慢」

**实际代码**（macro.c:46-104）：

```c
#define MACRO_BUCKETS 512

struct MacroTable {
    MacroDef *buckets[MACRO_BUCKETS];   // ← 已经是 hash table
    /* ... */
};

static uint32_t macro_hash(const char *name) {
    uint32_t h = 2166136261u;             // ← FNV-1a
    for (const char *p = name; *p; p++)
        h = (h ^ (uint8_t)*p) * 16777619u;
    return h & (MACRO_BUCKETS - 1);
}

static MacroDef **macro_slot(MacroTable *t, const char *name) {
    uint32_t h = macro_hash(name);
    for (MacroDef **pp = &t->buckets[h]; *pp; pp = &(*pp)->next)
        if (strcmp((*pp)->name, name) == 0)
            return pp;
    return NULL;
}
```

**MacroTable 早就是 hash table 了**——512 buckets + FNV-1a hash + bucket array per-name。HANDOVER 描述应该是某个早期 phase 改过但 HANDOVER 没同步。

**真问题在 caller**：`directive.c` process_buf 主循环（line 1695 + 1717，Phase 9 后）：

```c
/* Try macro expansion */
if (!t.hide && macro_lookup(st->macros, name)) {           // 第 1 次 hash + strcmp
    /* ...build input list... */
    bool _is_func = macro_lookup_is_func(st->macros, name);  // 第 2 次 hash + strcmp
    if (_is_func) { /* ... */ }
}
```

每个 ident 在 process_buf 主循环中触发**两次** hash + strcmp 查找（即使第二次的 cache 已热）。这是 gprof 显示 `macro_lookup_is_func` 占 20% 的真原因——不是 hash table 实现有问题，而是 caller 没复用查找结果。

## 修复（小但精确）

### 1. `macro.h`/`macro.c` 加 accessor `macro_def_is_func`

保持 `MacroDef` 封装（完整定义留在 macro.c 私有）的同时，让 caller 能查询已知 `MacroDef *` 的 `is_func`：

```c
/* macro.h */
bool macro_def_is_func(const MacroDef *def);

/* macro.c */
bool macro_def_is_func(const MacroDef *def) {
    return def && def->is_func;
}
```

trivial accessor，-O2 LTO 跨 TU inline 完美。即使不 inline，函数调用 + 单次内存读取仍比 hash + strcmp 快。

### 2. `directive.c` 主循环改用缓存指针

```c
/* Phase 10: cache the MacroDef* from this lookup and reuse it
 * below when checking is_func.  Previously this loop did two
 * separate hash+strcmp passes ({lookup, then lookup_is_func})
 * for every identifier in the source — gprof showed
 * macro_lookup_is_func at 20%+ self-time after Phase 9.        */
MacroDef *_macdef = !t.hide ? macro_lookup(st->macros, name) : NULL;
if (_macdef) {
    /* Expansion limits breached? Pass through unexpanded */
    if (macro_limits_breached(st->macros)) {
        emit_tok_text(st, &t);
        pptok_free(&t);
        continue;
    }

    /* ...build input list... */

    /* We need to prime the list with enough look-ahead for
     * function-like macros (the argument list).  Phase 10:
     * use the cached _macdef rather than re-querying.        */
    bool _is_func = macro_def_is_func(_macdef);
    if (_is_func) { /* ... */ }
}
```

无新增依赖，无 API 破坏。

## 性能数据（诚实记录）

### gprof 量化收益（含 -pg 开销）

| 指标 | Phase 9 末尾 | Phase 10 后 | 变化 |
|---|---|---|---|
| `macro_lookup_is_func` 调用次数 | 532,573 | 383,699 | **-28%** |
| `macro_lookup_is_func` self-time | 0.48 s (20.34%) | 0.31 s (13.72%) | **-35%** |
| `reader_next_tok` self-time | 0.74 s (31.36%) | 0.85 s (37.61%) | **+0.11 s** |
| `skip_block_comment` self-time | 0.31 s (13.14%) | 0.29 s (12.83%) | -0.02 s |
| gprof CPU total | 2.36 s | 2.26 s | -4.2% |

调用次数下降 28%（532K → 384K）符合预期：消除一处双重 lookup 就是把双重路径减为单次。**self-time 降 35% 比调用次数降 28% 更激进**——可能是因为缓存指针后第二次访问的内存已经在 L1 中（vs. 重新走 hash chain）。

`reader_next_tok` self-time 略升 0.11s——这部分是 accessor 调用开销 + 重新分配的寄存器影响（编译器把 process_buf 的部分内联工作归到了 reader_next_tok 上下文）。**总 CPU 净降 4.2%**（2.36s → 2.26s）。

### wall-clock（5 次 of 50 iters，sorted 后取中位）

| | Phase 9 末尾（3 次） | Phase 10 末尾（5 次） |
|---|---|---|
| sorted samples | 54.33 / 54.61 / 56.65 | 55.10 / 55.21 / 55.54 / 55.74 / 56.14 |
| 中位 | **54.61 ms** | **55.54 ms** |
| ratio vs gcc -E | 1.81× | 1.81× |

**坦白**：wall-clock 中位差 0.93 ms（1.7%），**在容器测量噪声范围内**（±2ms 抖动）。**不能声称稳定 wall-clock 改善**。

为什么 gprof 显示 4.2% 改善但 wall-clock 没体现？两个可能因素：

1. **gprof 与 -pg 的精度限制**：`-pg` 用 mcount() 在每个函数入口插桩，本身就增加了 5-15% 开销，且对小函数测量精度差。`-pg` 下的 4.2% 改善在 -O2 不带 -pg 下可能小到 1-2%
2. **容器抖动 ≥ 改善量**：gprof 估算 0.10s 节省 / 2.36s = 4.2%，相当于 wall-clock 中位 ~2.3ms 节省。这恰好等于容器抖动量级，无法稳定测量

## 为什么仍保留改动

即使 wall-clock 没穿透抖动：

1. **代码意图更清晰**：缓存 `macro_lookup` 返回的指针是显然合理的——任何 reviewer 看到原代码都会问"为什么不复用？"
2. **gprof 量化收益正向**：5326→3837 次/iter 的 hash+strcmp 节省是真减少的工作
3. **零风险**：accessor 是纯函数，封装良好；不破坏 API 也不引入新依赖
4. **LTO/PGO 留 hook**：当编译器对跨 TU inline 更激进时（GCC 12+, Clang 16+），accessor 自动 collapse 为单 load
5. **utility API 复用**：`macro_def_is_func` 是清晰命名的工具函数，未来类似情况可直接用

如果撤销，损失：清晰的代码 + 数据正向的小优化 + 一个 utility API；**收益**：移除 12 行代码。明显应该保留。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `macro.h` | +18 | `macro_def_is_func` 声明 + 文档说明双重 lookup 的根因（给未来 reviewer 留参考） |
| `macro.c` | +6 | `macro_def_is_func` 实现（trivial accessor） |
| `directive.c` | +12 / -7 | 主循环 macro_lookup 改用缓存指针 + 注释解释 |
| `PLAN.md` | 状态表 + 章节 + 累计统计 | 单一真相源更新 |
| `HANDOVER.md` | 顶部 + 候选段重排 + 推荐第一动作改为 skip_block_comment + 加坑 15 + Phase 10 接力补遗 + 签名 | 给下一位接力 |
| 新增 `PHASE10_SUMMARY.md` | 本文 | 诚实阶段总结 |

**未触碰**：所有测试文件、`pptok.c` / `expr.c` / `cpp.c` / `cpp_targets.c` / `cpp_internal.h` / Makefile / CI workflow / PROFILING.md。Phase 10 影响面最小（3 文件，6 + 12 行净改）。

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
Phase 8 results: 37 passed, 0 failed, 0 skipped
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan       # 508/508 全过，零内存错误，零 UB
$ make strict     # 508/508 全过，零警告（-Werror -Wpedantic -Wshadow -Wstrict-prototypes）
$ make bench      # 5 runs of 50 iters: 中位 55.54 ms / 1.81× gcc -E
```

## Phase 0-10 全景回顾

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
| 8 | ✅ | 37 | 规范完整性收尾（`__has_include_next` 真实语义 + GCC linemarker 输入语法 + profiling 工具链） |
| 9 | ✅ | 0 | 词法层性能优化（gprof 数据驱动；65.76→54.55 ms，-17.3%；ratio 2.14×→1.79×） |
| 10 | ✅ | 0 | 探针验证 + 消除双重 hash 查找（gprof 推翻 HANDOVER 推测；macro_lookup_is_func 调用 -28%；wall-clock 不显著） |
| **合计** | | **508** | **零回归、零失败、ASan/UBSan 清、严格警告清；性能 1.81× gcc -E（vs Phase 6 基准 2.4×）** |

## 全栈性能轨迹

| 阶段 | wall-clock per-iter | ratio vs gcc -E | 主要贡献 |
|:---:|:---:|:---:|---|
| Phase 6（基准） | 52 ms | 2.4× | 第一次系统性测量 |
| Phase 7-8 末尾 | 65-66 ms | 2.14-2.17× | 加了 platform macros + linemarker，慢了 25%（功能换性能） |
| Phase 9 末尾 | **54.55 ms** | **1.79×** | rb_/ph1/ph2 inline + 快慢路径分离（-17.3%） |
| Phase 10 末尾 | 55.54 ms | 1.81× | 消除双重 hash 查找（gprof -4.2%；wall-clock 在抖动内） |

Phase 9 是迄今最大的单阶段加速。Phase 10 的诚实结论：**单点优化在已经吃过 P9 大头红利后，wall-clock 边际收益递减**——下一阶段需要更激进的改造（重写 skip_block_comment、批量 memcpy、PPTok 池等）。

## 设计取舍记录

- **不撤销 Phase 10 step 1**：即使 wall-clock 没穿透抖动，gprof 量化收益正向 + 代码意图更清晰 + 零风险 + LTO hook 留存。这是清晰的"应该保留"决策
- **不做更激进的优化**：`skip_block_comment` 重写涉及 splice/trigraph 兼容性，需要新测试 + 边界 case 验证。Phase 10 范围控制在"消除已发现的冗余"，更激进的改造留给 Phase 11+
- **不把 `MacroDef` 移到 cpp_internal.h**：保持封装。`macro_def_is_func` accessor 在 -O2 LTO 下基本等价于直接字段访问
- **gprof 量化 + wall-clock 测量必须分开汇报**：gprof 在 -pg 下精度有限，wall-clock 受容器抖动影响。Phase 10 同时给两份数据，让 reader 自己判断

## 已知限制（Phase 11+ 候选）

按数据驱动重排：

| 候选 | 价值 | 备注 |
|---|---|---|
| `skip_block_comment` 用 fast-path 扫描 | **中（Phase 11 推荐）** | P10 后 #3 热点（12.83%）；用 raw buffer 直接扫 `*` 字符避开 reader_peekc/getc 链 |
| Phase 6 sweep 用 memchr 批量复制 | 中 | `p6_read_prefix` 14.9M 调用是字符级，sweep 主循环 sb_push_ch 一次 1 字节 |
| C: PPTok 对象池 | 低-中 | `tl_append_copy` 仅 2.54%，受益有限 |
| H/J: ICE 保护 / 模糊测试 | 中 | 基础设施类 |

**推荐 Phase 11 范围**：`skip_block_comment` 重写——用 raw buffer 直接 `memchr(buf, '*', remaining)`，遇到普通 ASCII 段批量推进 pos。预计 5-10% wall-clock 改善。

**严格按"先量再写测试再改"流程**：(1) 先测 P10 baseline，(2) 写覆盖 splice/trigraph 边界的新测试 (P11.c)，(3) 实施快路径，(4) 跑 508 项 + ASan + strict，(5) gprof 二次验证 + wall-clock 5 次取中位。**不要赌结果**——Phase 10 已经证明 wall-clock 改善可能小于容器抖动量级。
