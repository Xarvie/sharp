# Phase 8 交付说明

## 总览

Phase 8 完成"规范完整性收尾"。**累计 508 项回归测试全部通过**（41 既存 Sharp + 82 P1 + 49 P2 + 90 P3 + 47 P4 + 46 P5 + 47 P6 + 69 P7 + 37 P8），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

Phase 7 接力时 HANDOVER 把候选优先级重排，把 F（`__has_include_next` 测试）/ G（`#line N "f" 1 3` GCC 扩展）/ I（profiling 工具链）三件标记为 P0。Phase 8 把三件一起做掉。

**重要发现（探针先行的价值）**：F 和 G 在调研阶段都从"加点测试 / 扩展解析"升级为 **真 bug 修复**。这次 Phase 8 共修复 **2 个新真 bug**，外加完整的 profiling 工具链。**累计真 bug 修复数从 12 个升到 14 个**。

## 修复

### 1. `__has_include_next` 真实语义（真 bug）

**病征**：HANDOVER 描述"实现存在但没有专门测试"。**实测代码**：

```c
(void)is_next; /* __has_include_next needs extra logic, for now same */
```

`__has_include_next` 是个 stub，与 `__has_include` 完全等价。当头文件作者写 `#if __has_include_next(<foo.h>)` 想检测 chain 中下一份 foo.h 时，得到的总是与 `__has_include(<foo.h>)` 相同的答案——基本等于功能不工作。

**根因**：实现时 `is_next` 这条 logic 路径被推迟，注释明确说"for now same"。Phase 6 修复 `#include_next` 时把 skip-by-directory 逻辑写进了 `find_include_ex`，但 `cpp_has_include` 没跟着升级。

**修复**：

- `cpp_has_include` 升级为 `cpp_has_include(st, name, is_system, is_next, current_file)`，当 `is_next=true` 时计算 `skip_dir = dirname(current_file)`，遍历搜索路径时跳过该目录及之前的所有目录，从下一个开始查找——与 `find_include_ex` 同源
- 边界处理：如果 `current_file` 没有目录部分（虚拟名如 `<test>`），优雅降级为普通 `__has_include` 语义
- `resolve_has_include` / `resolve_all_has_include` 接受 `current_file` 参数；`handle_if` / `handle_elif` 把 `loc.file` 沿调用链传递（不引入全局 state）

**验证通过**：

- 同名 header 在两个 `-I` 目录中，`__has_include_next` 返回 1（能找到下一份）
- 同名 header 仅在当前目录，`__has_include_next` 返回 0（当前目录被跳过）
- 字符串引号 `"foo.h"` 与尖括号 `<foo.h>` 两种形式都工作
- 虚拟文件名时优雅降级
- `__has_include` 自身保持原有行为（无回归）

### 2. GCC linemarker 输入语法 `# N "file" [F1 [F2]]`（真 bug）

**病征**：HANDOVER 描述"尾部 1/3 数字被忽略（行为正确）；语义不完整"。**实测**：

```c
const char *src = "# 100 \"renamed.c\"\nx = __LINE__\n";
/* 输出：x = 2，并报 error: unknown directive: # */
```

派发器只识别 `#<ident>` 形式（`if (kw.kind == CPPT_IDENT)`），`# N "file"` 这种 GCC linemarker 直接报 unknown directive。这是双向不对称——**sharp-cpp 自己输出的 `# 1 "<file>"` 不能被同一个 sharp-cpp 重新读回来**。

**根因**：`#line` 关键字形式与 `# N` 数字形式在 GCC/clang 是同源的（同一个内部 handler），但 sharp-cpp 实现时只接了关键字分支。

**修复**：

- 派发器识别 `kw.kind == CPPT_PP_NUMBER`：合成 `dname="line"` + `tl_prepend(line, kw)`，让现有的 `handle_line` 统一处理两种形式
- `handle_line` 解析尾部最多两个 GCC flag 数字（1=push, 2=pop, 3=system header, 4=extern "C"），范围外的数字静默忽略（GCC 行为）
- 新增 `emit_linemarker_ex(st, line, file, flag1, flag2)` 把 flags 输出为 `# N "file" 1 3` 形式；保留旧 `emit_linemarker(...)` 三参数 wrapper，旧 4 个 caller 站点零修改
- `tl_prepend` helper 加入 `cpp_internal.h`（与 `tl_append` 同性质，12 行扩展，零副作用）

**验证通过**：

- `# 100 "renamed.c"` 不再报 unknown directive；`__LINE__` 正确变为 100，`__FILE__` 正确变为 `"renamed.c"`
- `# 1 "/usr/include/stdio.h" 1 3` round-trip：输入解析 + 输出保留两个 flags
- 范围外 flag（如 99）静默忽略，输出不出现
- 不带 flag 的 `# N "file"` 输出无尾随空白
- `#line` 关键字形式不破坏（Phase 4 修复保留）；`#line` 也兼容尾部 flag

### 3. profiling 工具链（开发体验加固）

Phase 8 之前性能测量靠 `make bench`（30 迭代，给一个 ratio 数字）。Phase 9+ 候选 B/C/D 是性能优化三件套——但凭直觉优化危险，需要数据驱动。

Makefile 新增三个 target：

| Target | 用途 | 依赖 |
|---|---|---|
| `make profile` | `perf record + perf report`，跑 200 迭代，打印 top 20 self-time 函数 | linux-perf |
| `make flamegraph` | 上面再加一步：调用 [Brendan Gregg 的 FlameGraph](https://github.com/brendangregg/FlameGraph) 渲染交互 SVG | linux-perf + FlameGraph |
| `make massif` | `valgrind --tool=massif` 测堆峰值与分配热点 | valgrind |

全部强制重新构建到独立的 `profile_bin`，开 `-fno-omit-frame-pointer` 确保 DWARF 栈展开准确。`PROFILE_ITERS=N` 可调（默认 200）；`FLAMEGRAPH_DIR=...` 可指向 FlameGraph 仓库 checkout。

新建 `PROFILING.md` 详细说明：工具安装、读 perf report / 读 flamegraph / 读 massif 的指南、方法学清单（禁用 turbo、绑核、三次取中位数）、跨平台 alternatives（macOS Instruments / Windows VS Profiler / 跨平台 samply）。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `directive.c` | +110 / -50 | F + G 两项 production 修复 |
| `cpp_internal.h` | +12 | `tl_prepend` helper |
| 新增 `test_phase8.c` | +500 | 7 节 37 项回归（F + G + Sharp parity） |
| `Makefile` | +50 | `test_phase8` build/test/clean，新增 profile/flamegraph/massif 三个 target |
| 新增 `PROFILING.md` | +180 | 工具链使用文档与方法学清单 |
| `.github/workflows/ci.yml` | 1 行注释 | "all seven" → "all eight" |
| `PLAN.md` | 状态表 + 新章节 + 文件清单更新 | 单一真相源更新 |
| 新增 `PHASE8_SUMMARY.md` | 本文 | 阶段总结 |

**Phase 8 注释锚点统计**：`directive.c` 16 处 `Phase 8:`，`cpp_internal.h` 1 处 `Phase 8:`。`grep "Phase 8:" *.c *.h` 一键审计。

未触碰：`pptok.c` / `macro.c` / `expr.c` / `cpp.c` / `cpp_targets.c` 全部零修改。Phase 8 影响面集中在 directive.c 与新增文件。

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
                                   合计 508 项

$ make asan       # 全清，零内存错误，零 UB
$ make strict     # -Werror -Wpedantic -Wshadow -Wstrict-prototypes，无警告
$ ./probe         # 仍仅 1 项预期 ISSUE（探针自身的 __LINE__ 边界）
$ make profile_bin && ./profile_bin 5    # 66.69 ms/iter, 2.17× gcc -E（无回归）
```

通过：`-O0 -g`、`-O2`，`-std=c99`、`-std=c11`，ASan + UBSan，严格警告。

### Phase 8 测试覆盖（37 项）

| 章节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | F：`__has_include_next` 跳过当前目录（同名 header 在 a/b 两目录、字符串/尖括号两形式） | 9 |
| §2 | F：退化情形（虚拟文件名、无可用目录） | 4 |
| §3 | F：`__has_include` 自身无回归 | 4 |
| §4 | G：`# N "file"` 形式被识别并更新 `__LINE__` / `__FILE__` | 5 |
| §5 | G：GCC flags 1/2/3 输入解析 + 输出保留；范围外忽略；无 flag 不输出多余空白 | 8 |
| §6 | G：`#line` 关键字形式不破坏（含可选 flags） | 4 |
| §7 | Sharp 模式 parity：F + G 都能在 `CPP_LANG_SHARP` 下工作 | 3 |

### 测试设计严谨性要点

- **合成 fixture 优于依赖系统头**：F 的所有测试用 `mkdir(/tmp/p8_a_PID_N)` + `write_file` 自己造目录树，不依赖 `/usr/include`。这让 CI 在任何主机都跑得通，且诊断失败时输入完全可控。
- **每个 fixture 测试自己 cleanup**：`rm_rf(dir_a); rm_rf(dir_b); free(...)` 在测试函数末尾，ASan 立刻抓泄漏。
- **正负断言并用**：F 的"找到下一份"和"当前目录被跳过"是同一行为的两面；§1.1 同时断言 `FOUND_NEXT` 出现 + `NOT_FOUND_NEXT` 不出现，避免假阳性（如果两个分支都被错误地包含，单边断言会过）。
- **G 的 round-trip 是 spec 等价测试**：§5.2 测 `# 1 "stdio.h" 1 3` 输入后输出包含 `"\"stdio.h\" 1 3"`——这才是"真正完整支持 GCC linemarker"的硬证据。

## 一处测试期望修正（写测试过程的小记）

§4.2 与 §6.1 初版期望 `L= 500`、`L= 200`（带前导空格），实测得到 `L=500`、`L=200`（无空格）。原因：用户定义的宏（如 `#define A 1`）展开时携带"前导空格"标志（防止与上一个 token 粘成新词），但 `__LINE__` / `__FILE__` 是 builtin 动态展开路径，不走相同的安全空格逻辑。已修正测试期望，与 Phase 7 §2.4（`value=42` vs `value= 42`）的修正方向相反但同性质——都是"测试期望与实际格式不符"，不是 production bug。

## Phase 0-8 全景回顾

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
| **合计** | | **508** | **零回归、零失败、ASan/UBSan 清、严格警告清** |

**累计修复 14 个真 bug**（Phase 8 新增 2 个：`__has_include_next` stub、GCC linemarker 输入语法）。

## 已知限制（Phase 9+ 候选）

按"用户痛点 vs 实施成本"重排（**P0 已清空**——Phase 8 把 F/G/I 三项 P0 都做完了）：

| 编号 | 候选 | 价值 | 备注 |
|:---:|---|---|---|
| B | InternTable 改 hash 而非线性扫描 | 中 | **现在有 profiling 工具了**：先 `make flamegraph` 看 `intern_cstr` 实际占比，再决定 |
| C | PPTok 对象池 | 低-中 | 同上，profiling 数据驱动 |
| D | StrBuf 预分配 | 低 | 同上 |
| E | 完整 Prosser per-token hide-set | 低 | 当前 active-expansion 栈对所有真实场景都过；理论一致性而非用户痛点 |
| H | ICE 保护（OOM-safe） | 中 | 基础设施类；setjmp/longjmp 把 `cpp_xmalloc` 失败转成诊断 |
| J | 模糊测试（AFL/libFuzzer） | 中 | 基础设施类；`cpp_run_buf` 的 fuzz harness |

**推荐 Phase 9 范围**：跑一次 `make flamegraph` → 看 top 5 self-time → 选 1-2 个最大 plateau 做针对性优化（B 是当前最可能的赢家但需要数据确认）。这次"先量后改"的纪律由 Phase 8 的 I 任务奠定——**不要凭直觉优化**。
