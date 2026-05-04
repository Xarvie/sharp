# Phase 9 交付说明

## 总览

Phase 9 完成"数据驱动的词法层性能优化"。**累计 508 项回归测试全部通过**（无新增测试——Phase 9 是纯性能优化，行为不变；既有 508 项覆盖正确性），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

**性能改善**：
- 单次预处理：**65.76 ms → 54.55 ms**（中位 of 3 runs，bench.c 50 iters）—— **降 17.3%**
- vs `gcc -E`：**2.14× → 1.79×** —— 接近"实时编译器"心理预期阈值的 1.5×
- gprof CPU total：3.54 s → 2.36 s —— 降 33%

Phase 8 落地了 profiling 工具链后，HANDOVER 明确建议下一位接力人**先跑数据再优化**。Phase 9 严格遵守这一纪律：**用真实热点数据指导优化**，而不是凭直觉相信 HANDOVER 推测的 P1-B（`InternTable` hash）是赢家。

## 探针先行：数据推翻直觉

容器里 `perf` 与 `valgrind` 不可用，但 **`gprof`** 可用（这正是 PROFILING.md 的"alternatives"段提到的同等工具）。跑 `gprof -b -p` 看 flat profile 后：

```
%       cumulative   self        calls       name
30.79     1.09       1.09        18958609    reader_next_tok
19.21     1.77       0.68       366521627    ph2_raw_get
14.97     2.30       0.53       366779379    ph1_get
11.02     2.69       0.39          532573    macro_lookup_is_func
 3.67     2.82       0.13           27068    process_buf
```

**关键发现**：
1. **HANDOVER 推测的 `intern_cstr` 完全不在 top 30**——HANDOVER 写"现状：InternTable 用线性 strcmp 查找；大 input 上 macro_lookup 链路慢"，但实测 `macro_lookup` 仅 1.41%，`intern_cstr` 看不到
2. **真热点在词法层字符获取链路**：`reader_next_tok` + `ph2_raw_get` + `ph1_get` 一起占 65%
3. **调用次数比例讲故事**：每个 token 触发约 19 次字符获取（366M ÷ 18.9M），这是因为 ph1（trigraph）+ ph2（line splicing）每个字符都要过一次

这是"探针先行"纪律的又一次验证：**不要相信 HANDOVER 描述的优化方向，跑数据再说**。如果 Phase 9 直接做 P1-B（intern hash），最多省 1.4%，几乎看不到改善。

## 修复（实施）

### 优化 1：`rb_peek` / `rb_peek2` / `rb_adv` 改 `static inline`

这三个是字符获取的最底层（每次 `ph1_get` 调用 1-3 次）。短小（几条机器指令），但调用密度极高（数亿次/iter）。

`rb_adv` 内部对 EOF 检查与 `'\n'` 判断都加 `CPP_UNLIKELY` 标注——实际源码绝大多数字节既不是 EOF 也不是换行，分支预测对了能省掉 pipeline stall。

### 优化 2：`ph1_get` 改 `static inline` + trigraph 路径标 UNLIKELY

trigraph（`??=` → `#` 等）在现代 C 中几乎不出现（C23 已正式移除该特性）。inline 后快速路径退化为：

```c
int c = rb_adv(rb);  /* inline */
if (UNLIKELY(c == '?' && ...)) { ...trigraph stuff... }
return c;
```

编译器可以把整个序列 fold 进 lexer 主循环，热路径只剩 5-6 条机器指令。

### 优化 3：`ph2_raw_get` 拆快慢路径（核心改造）

`ph2_raw_get` 含 while 循环（line splicing），编译器即使在 -O2 也不会把它 inline 进 lexer 主循环。但 splice（`\\\n`）在真实代码中几乎不出现——主路径就是"读一个字符，看是不是反斜杠"。

```c
/* Phase 9: split into hot inline + cold noinline */
static int ph2_raw_get_slow(CppReader *rd);  /* forward decl, noinline */

static inline int ph2_raw_get(CppReader *rd) {
    int c = ph1_get(&rd->raw);
    if (CPP_LIKELY(c != '\\')) return c;       /* hot path: ~3 instructions */
    return ph2_raw_get_slow(rd);                /* cold: rare splice handling */
}

static int ph2_raw_get_slow(CppReader *rd) {
    /* 完整保留原 splice 循环语义 */
    int c = '\\';
    while (c == '\\') { /* ...原代码... */ }
    return c;
}
```

这是经典的"hot path inline + cold path out-of-line"模式：编译器在调用方看到 `ph2_raw_get` 主体仅 4 条指令（`ph1_get` + 比较 + 分支预测命中 + 返回），可以与 lexer 主循环融合；splice 处理仍在 `ph2_raw_get_slow` 里完整保留语义。

### 配套：可移植 LIKELY/UNLIKELY 宏

`cpp_internal.h` 加：

```c
#if defined(__GNUC__) || defined(__clang__)
# define CPP_LIKELY(x)   __builtin_expect(!!(x), 1)
# define CPP_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define CPP_LIKELY(x)   (x)
# define CPP_UNLIKELY(x) (x)
#endif
```

GCC/clang 用 `__builtin_expect`，其他编译器（MSVC 等）退化为表达式本身（无优化但语义正确）。MSYS2 GCC、Apple clang 都识别——CI matrix 不破坏。

## 性能对比（中位数 of 3 runs，50 iters，bench.c）

| 指标 | Phase 8 baseline | Phase 9 后 | 改善 |
|---|---|---|---|
| Wall-clock per-iter | 65.76 ms | **54.55 ms** | **-17.3%** |
| Ratio vs `gcc -E` | 2.14× | **1.79×** | -16% |
| gprof CPU total（含 -pg 开销） | 3.54 s | 2.36 s | -33% |

Phase 9 后 gprof flat profile（top 6）：

```
%       cumulative   self        calls       name
31.36     0.74       0.74        18958609    reader_next_tok
20.34     1.22       0.48          532573    macro_lookup_is_func
13.14     1.53       0.31          697203    skip_block_comment
 6.78     1.69       0.16           27068    process_buf
 3.81     1.78       0.09          229068    emit_deferred_directive
 3.39     1.86       0.08             101    build_result
```

**`ph1_get` 和 `ph2_raw_get` 在 top 30 中完全消失**——确认成功被 inline 进调用方。`reader_next_tok` 自身的 self-time 从 1.09 s 降到 0.74 s（**降 32%**）即使它现在内含被 inline 的 ph1/ph2——说明 GCC 在 inline 后做了更激进的跨边界优化（CSE、寄存器分配、消除冗余 splice 检查等）。

新热点分布给 Phase 10+ 提供了清晰路标：`macro_lookup_is_func` 上升到 #2（20.34%），`skip_block_comment` 浮现为 #3（13.14%）。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `pptok.c` | +50 / -25 | rb_*/ph1_get inline + ph2_raw_get 快慢路径分离 |
| `cpp_internal.h` | +14 | `CPP_LIKELY` / `CPP_UNLIKELY` 可移植宏 |
| `PLAN.md` | 状态表 + 新章节 + 文件清单更新 | 单一真相源更新 |
| 新增 `PHASE9_SUMMARY.md` | 本文 | 阶段总结 |

**无新增测试套件**——Phase 9 是纯性能优化，行为不变。`make test` 508/508 + `make asan` 508/508 + `make strict` 508/508 共同保证了正确性。

**未触碰**：`macro.c` / `expr.c` / `directive.c` / `cpp.c` / `cpp_targets.c` / 任何 test_phase*.c / Makefile / CI workflow。Phase 9 影响面是最小的——两文件，可移植。

## 测试矩阵

```bash
$ make test    # 508/508 全过（无回归）
Phase 1 results: 82 passed, 0 failed
Phase 2 results: 49 passed, 0 failed
Phase 3 results: 90 passed, 0 failed
Phase 4 results: 47 passed, 0 failed
Phase 5 results: 46 passed, 0 failed
Phase 6 results: 47 passed, 0 failed, 0 skipped
Phase 7 results: 69 passed, 0 failed, 0 skipped
Phase 8 results: 37 passed, 0 failed, 0 skipped
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan    # 508/508 全过，零内存错误，零 UB
$ make strict  # 508/508 全过，零警告（-Werror -Wpedantic -Wshadow -Wstrict-prototypes）
$ make bench   # ~54.55 ms/iter, 1.79× gcc -E
```

通过：`-O0 -g`、`-O2`，`-std=c99`、`-std=c11`，ASan + UBSan，严格警告。

## 设计取舍记录

- **不写 Phase 9 专属测试套件**：纯性能优化，不引入任何新行为。如果有功能回归，508 项既有测试会立刻抓到。Phase 6 §4 `test_performance_budget`（≤500 ms/iter）作为 O(N²) 检查保留——目前 ~54 ms 离 500 ms 还有 9× headroom，未来回归会被自动捕获。
- **不依赖 GCC 专属语法**：`__builtin_expect` 包在 `CPP_LIKELY` / `CPP_UNLIKELY` 宏后；非 GCC/clang 编译器退化为表达式本身（无优化但语义正确）。CI matrix 的 MSYS2 GCC、Apple clang、Ubuntu gcc/clang 都识别。
- **不动 `reader_peekc`**：当前 `reader_peekc` 用 save-state + ph2_raw_get + restore 模式，每次 peek 都跑完整 splice 逻辑。理论可优化但风险更高（splice 语义敏感，且优化收益较小因为 peek 比 get 频率低）。Phase 9 把改动控制在最小高收益面。
- **不引入 `[[likely]]`（C23）或 `__attribute__((hot))`**：项目 baseline 是 C99/C11；引入 C23 attribute 会破坏 c99 CI job。`__builtin_expect` 在所有目标编译器上都存在，是最大公约数。

## Phase 0-9 全景回顾

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
| 9 | ✅ | 0（性能优化无新行为测试） | 词法层性能优化（gprof 数据驱动；65.76→54.55 ms，-17.3%；ratio 2.14×→1.79× gcc -E） |
| **合计** | | **508** | **零回归、零失败、ASan/UBSan 清、严格警告清；性能 1.79× gcc -E** |

## 已知限制（Phase 10+ 候选）

| 编号 | 候选 | 价值 | 备注 |
|:---:|---|---|---|
| **B'** | `macro_lookup_is_func` / `macro_lookup` 改 hash | **高（Phase 10 推荐）** | 现在是 #2 热点（20.34%）；hash 化预计再降 10-15% 至 ~46-49 ms/iter |
| `skip_block_comment` 优化 | 中（新热点） | 浮现为 #3（13.14%）；可能 SIMD / strchr 加速 |
| C | PPTok 对象池 | 低-中 | `tl_append_copy` 仅 2.54%，受益有限 |
| D | StrBuf 预分配 | 低 | `pptok_spell` 没浮现 |
| E | 完整 Prosser per-token hide-set | 低 | 当前 active-expansion 栈对所有真实场景都过；理论一致性而非用户痛点 |
| H | ICE 保护 / J 模糊测试 | 中 | 基础设施类 |

**推荐 Phase 10 范围**：B'（`macro_lookup` hash 表）。具体方法：`MacroTable` 内现有线性 array 改为 hash bucket（与 Phase 7 的 `cpp_targets.c` 同样用 strcmp 派发，只不过这次有 N=数百-数千个 macro 而不是 3 个 triple）。预计 1.5-2 小时含测试。

**也可以做**：`skip_block_comment` 用 strchr 跳过非 `*` 字符，预计 5-10% 改善（小快赢）。
