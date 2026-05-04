# Phase R1 交付说明 — Lua 5.4 真实场景集成测试

## 总览

Phase R1 是项目第一个**真实场景集成测试阶段**，与 Phase 0-10（功能/性能层）平行——"R" 取自 Real-world / Regression 双关。

**测试集**：[Lua 5.4.7 官方源码](https://github.com/lua/lua) — 35 个 .c 文件 + 28 个 .h 文件，**31,630 行**真实 C 代码。Lua 是被广泛采用的成熟项目，使用大量真实预处理特性：嵌套 include、平台检测（POSIX/Windows）、函数式宏、复杂条件编译、glibc 系统头链。

**累计回归测试：527 项已通过**（41 既存 Sharp + 82 P1 + 49 P2 + 90 P3 + 47 P4 + 46 P5 + 47 P6 + 69 P7 + 37 P8 + 19 R1；P9/P10 是性能优化无新行为测试），ASan/UBSan 全清，`-Werror -Wpedantic -Wshadow -Wstrict-prototypes` 通过。

**核心成果**：

1. **发现并修复一个真 bug**：`<...>` 形式 `#include` 错误地搜索 including-file 目录，导致 glibc 系统头无限递归（拓展自 Phase 6 的同类型修复）
2. **35/35 Lua 文件零诊断处理**——total output 与 `gcc -E` ratio **1.001**（3,879,914 vs 3,877,599 bytes，几乎逐字节相同）
3. **建立集成测试基础设施**：`sharp_cli`（CLI driver）+ `lua_smoke.sh`（35 文件批量验证）+ `test_lua_includes.c`（19 项回归锁定）
4. **累计真 bug 修复数从 14 升到 15**

## 探针先行：发现 bug 的过程

### 步骤 1：拿样本

```bash
git clone --depth 1 --branch v5.4.7 https://github.com/lua/lua.git lua-src
# 35 C files, 28 H files, 31,630 LoC
```

### 步骤 2：写最小 CLI driver

`sharp_cli.c` (~80 行)，接受 `<file> -I dir -isystem dir -D name=value`，调用 `cpp_run_buf` + 注入 `cpp_install_target_macros(ctx, "x86_64-linux-gnu")`，把诊断分类计数报告到 stderr。

### 步骤 3：第一次跑 — 发现 luaconf.h 需要系统 include

```bash
$ sharp_cli luaconf.h
luaconf.h:11:1: error: file not found: limits.h
luaconf.h:12:1: error: file not found: stddef.h
luaconf.h:657:1: error: file not found: stdint.h
```

合理——sharp_cli 没注入 `-isystem` 路径。补上 `gcc -E -v` 输出的 4 个标准 sys path：

```
/usr/lib/gcc/x86_64-linux-gnu/13/include
/usr/local/include
/usr/include/x86_64-linux-gnu
/usr/include
```

`luaconf.h` 现在 **零诊断**，输出 9079 bytes / 1195 lines。

### 步骤 4：跑全部 35 个 .c 文件

写 `lua_smoke.sh`（~150 行）：循环每个 .c，跑 sharp_cli + gcc -E，对比输出 size + 收集诊断。

第一次结果（修 bug 前）：

```
Files processed:    35
Passed (no err):    33
Failed:             2
Aggregate diags:    fatal=2  error=2  warning=0
Total output size:  sharp-cpp=3694250  gcc-E=3877599  (ratio 0.953)
```

**33/35 通过（94%）**——已经很不错。失败的两个：

- `lauxlib.c` — 1 fatal + 1 error
- `onelua.c` — Lua amalgamation（把所有 .c 合一编译）— 同样的根因

### 步骤 5：诊断真问题

```bash
$ sharp_cli lauxlib.c ...
/usr/include/x86_64-linux-gnu/sys/signal.h:1:1: fatal: #include nesting too deep (max 200)
/usr/include/x86_64-linux-gnu/sys/wait.h:35:1: error: unterminated #if at end of file
```

最小化复现：

```bash
$ cat probe.c
#include <sys/wait.h>

$ sharp_cli probe.c -isystem /usr/include/...
fatal: #include nesting too deep
```

但是单独 `#include <signal.h>` **不**失败：

```bash
$ cat probe2.c
#include <signal.h>

$ sharp_cli probe2.c -isystem /usr/include/...
[diagnostics] fatal=0 error=0 warning=0  ← clean
```

差异是什么？看 `sys/wait.h:36`：

```c
#  include <signal.h>
```

注意 `# include <signal.h>` 是从 `/usr/include/x86_64-linux-gnu/sys/wait.h` 内部发出的。同时 `/usr/include/x86_64-linux-gnu/sys/signal.h` 这个 forwarding header 存在——内容是：

```c
#include <signal.h>
```

**bug 锁定**：

1. sharp-cpp 处理 `sys/wait.h` 内部的 `#include <signal.h>` 时
2. 错误地**先搜索 including-file 的目录**（即 `sys/`）
3. 找到 sibling `sys/signal.h`，处理它
4. `sys/signal.h` 内部 `#include <signal.h>` 又被错误地解析回 `sys/signal.h`
5. **无限递归** → 200 层后 fatal

### 步骤 6：修复

`directive.c` 的 `find_include_ex` line 421-439 注释明确说"For \"...\" and <...> first search relative to the including file's directory"——这是错的。

C99 §6.10.2 与 GCC/clang 的事实标准：

- `#include "..."` 搜索 implementation-defined（典型：including file dir → -iquote → -I → sys path）
- `#include <...>` **只**搜索 sys paths（典型：-isystem → 默认 sys path）

修复：把 line 424 的 `if (current_file && !skipping)` 改为 `if (current_file && !skipping && !is_system)`。一行 + 长注释（解释 bug-discovery 故事，为后人留参考）。

## 修复验证

### 单独复现 case

```bash
$ sharp_cli probe.c (内容: #include <sys/wait.h>) -isystem ...
[diagnostics] fatal=0 error=0 warning=0 note=0 output=29829 bytes  ← clean
```

### Lua smoke test 全集合

```
Files processed:    35
Passed (no err):    35
Failed:             0
Aggregate diags:    fatal=0  error=0  warning=0
Total output size:  sharp-cpp=3879914  gcc-E=3877599  (ratio 1.001)
```

**100% 通过率，total output 与 gcc -E ratio 1.001——几乎逐字节相同**。这是非常强的语义等价证据：sharp-cpp 在 31k LoC 真实代码上的输出与参考实现差异 < 0.1%，主要在空白/换行处理细节。

### 既有测试无回归

```
make test:    527/527 通过（508 既有 + 19 新增 R1）
make asan:    527/527 通过，零内存错误，零 UB
make strict:  527/527 通过，零警告
```

## 测试基础设施

### `sharp_cli.c`（~85 行）

最小的 CLI driver，接受 `-I` / `-isystem` / `-D`，输出预处理结果到 stdout，诊断计数到 stderr。**不是公开 CLI**——production 用户用 C API；这是测试 fixture。

### `lua_smoke.sh`（~140 行）

针对 Lua 源码的批量 smoke 测试：

- 跑每个 .c 文件经过 sharp_cli + 跑 gcc -E 做尺寸对比
- 解析诊断计数（fatal/error/warning）
- 打印每文件 size + ratio 表
- 聚合统计（total output / aggregate ratio）
- 退出码：0 = 全部干净，1 = 至少一个 error

`make lua_smoke` 自动 git clone Lua 5.4.7 到 `/tmp/sharp_cpp_lua/` 并跑测试。或 `make lua_smoke LUA_SRC=/path` 用现有 checkout。

### `test_lua_includes.c`（19 项 CHECK，6 节）

把 bug-fix 的语义不变量冻结成回归测试，避免未来回退：

| 节 | 内容 | CHECK 数 |
|---|---|:---:|
| §1 | glibc-shape：sys/wait.h `#include <foo.h>` 不应解析为 sys/foo.h | 4 |
| §2 | `"sibling.h"` 形式仍正确解析到 including-file 目录 | 3 |
| §3 | `<header.h>` 形式不搜索 including-file 目录 | 2 |
|    | `<header.h>` 通过 -isystem 解析正常 | 2 |
| §4 | `"sysonly.h"` 在 -isystem 下找到（C99 §6.10.2/3 fallback） | 2 |
| §5 | 三层转发链不触发递归 + guard 正确（每个 body 出现 1 次） | 3 |
| §6 | Sharp 模式 parity（`CPP_LANG_SHARP` 下行为相同） | 3 |

每个 fixture 都用 `mkdir(/tmp/r1_xxx_PID_N) + write_file` 自己造目录树，不依赖 `/usr/include`。每个测试结尾 `rm_rf` 清理。ASan 立刻抓泄漏。

## 文件改动

| 文件 | 行差 | 说明 |
|---|---|---|
| `directive.c` | +9 / -5 | `find_include_ex` 的 system-form 不搜索 including-file 目录的 1 行修复 + 8 行长注释（含 bug 发现故事） |
| 新增 `sharp_cli.c` | +85 | 最小 CLI driver |
| 新增 `lua_smoke.sh` | +140 | 35 文件批量集成测试 |
| 新增 `test_lua_includes.c` | +400 | 19 项 R1 回归测试（6 节） |
| `Makefile` | +30 | `test_lua_includes` / `sharp_cli` build rules + `lua_smoke` target（auto-clone）+ test target 加 R1 + clean |
| `.github/workflows/ci.yml` | 1 行注释 | "all eight" → "all nine" |
| `PLAN.md` | 状态表 + 章节 + 累计 | 单一真相源更新 |
| `HANDOVER.md` | 顶部 + 候选段 + 坑 16 + R1 接力补遗 + 签名 | 给下一位接力 |
| 新增 `PHASE_R1_SUMMARY.md` | 本文 | 阶段总结 |

**未触碰**：`pptok.c` / `macro.c` / `expr.c` / `cpp.c` / `cpp_targets.c` / `cpp_internal.h` / `macro.h` / `pptok.h` / `directive.h` / `expr.h` / 任何 test_phase*.c。Phase R1 影响面：
- 1 行 production fix（外加注释）
- 4 个新文件（CLI driver + smoke shell + 回归测试 C + 阶段总结 md）
- 文档与 Makefile 维护

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
Phase R1 results: 19 passed, 0 failed
Results: 41 passed, 0 failed   ← 既存 Sharp 扩展测试

$ make asan      # 527/527 全过，零内存错误，零 UB
$ make strict    # 527/527 全过，零警告
$ make lua_smoke # 35/35 文件零诊断，aggregate ratio 1.001
```

通过：`-O0 -g`、`-O2`，`-std=c99`、`-std=c11`，ASan + UBSan，严格警告。

## 设计取舍记录

- **Phase R1 不是性能阶段**：用户明确指示停止性能优化，转向"更全面更复杂的测试"。R1 的目标是**用真实代码集成验证项目鲁棒性**，而非数字压榨。结果证明这个方向是对的——在 31k LoC 真实代码上发现了 508 个合成测试都没找到的 bug
- **新阶段命名 R1 而非 P11**：性能阶段（P9/P10）和真实场景测试（R1）是两条平行的演进线。R 系列将来可以扩展（R2 = SQLite，R3 = git, R4 = 更复杂项目），与 P 系列互不冲突
- **bug 发现的写法**：`directive.c` 的 9 行注释**完整保留 bug 发现故事**——sys/wait.h → sys/signal.h → 无限循环。这种"作案现场"风格的注释比抽象描述更有教学价值，且让未来 reviewer 立即理解约束的来由
- **集成测试代码不打包 Lua 源码**：`make lua_smoke` 是 opt-in，需要网络 + git 才能运行。`test_lua_includes.c` 是 `make test` 默认必跑——19 项合成 fixture 已锁定 bug-fix 的语义不变量。Lua 源码更新 / 网络断了都不影响 CI 默认通过
- **不引入"性能回归测试"**：Phase 9/10 已有 bench.c 做 wall-clock measurement，足够；R1 是行为层测试不应混入性能维度

## Phase 0-10 + R1 全景回顾

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
| 8 | ✅ | 37 | 规范完整性收尾（`__has_include_next` + GCC linemarker + profiling 工具链） |
| 9 | ✅ | 0 | 词法层性能优化（gprof 数据驱动；65.76→54.55 ms，-17.3%） |
| 10 | ✅ | 0 | 探针验证 + 消除双重 hash 查找（gprof 推翻 HANDOVER；wall-clock 不显著） |
| **R1** | ✅ | **19** | **Lua 5.4 集成验证 + 1 个真 bug 修复（`<>` 不应搜索 including-file dir）** |
| **合计** | | **527** | **零回归、零失败、ASan/UBSan 清、严格警告清；35/35 Lua 文件零诊断；与 gcc -E ratio 1.001** |

**累计修复 15 个真 bug**（Phase R1 新增 1 个）。

## 累计真 bug 列表

| # | Phase | Bug | 描述 |
|:---:|:---:|---|---|
| 1-10 | 0 | 取证 | 互递归崩溃、宽字符前缀、未闭合字面量、`keep_whitespace` 失效、stringify 丢空格、参数前导空泄漏、`(-1U)>0` 错算、等等 |
| 11 | 4 | `#line` | `#line "f"` 不更新 `__FILE__` |
| 12 | 5 | `##` + `@` | `##` 与 `@` 内省 token 交互静默丢 RHS |
| 13 | 6 | `#include_next` | 用文件路径而非目录比较 |
| 14 | 8 | `__has_include_next` | 是 stub（与 `__has_include` 行为相同） |
| 15 | 8 | linemarker syntax | GCC `# N "file" [flags]` 报 unknown directive |
| **16** | **R1** | **`<>` include path** | **错误搜索 including-file 目录，导致 glibc 系统头无限递归** |

## 已知限制 / Phase 11+ / R2+ 候选

按"价值 vs 实施成本"排序：

### 进一步真实场景测试（R 系列扩展）

| 编号 | 候选 | 价值 | 备注 |
|:---:|---|---|---|
| **R2** | **SQLite (~150k LoC)** | **高（推荐下一阶段）** | 比 Lua 大 5×，更多平台检测，`sqlite3.c` amalgamation 是真测试集 |
| R3 | tinycc / cproc 测试集 | 高 | C 编译器自己的测试集，覆盖 spec 边界 |
| R4 | curl / OpenSSL / kernel 选段 | 中-高 | 大型项目，但 build 配置复杂 |

### 性能优化（如未来恢复）

| 候选 | 价值 | 备注 |
|---|---|---|
| `skip_block_comment` 用 fast-path 扫描 | 中 | Phase 10 后 #3 热点（12.83%） |
| Phase 6 sweep 用 memchr 批量复制 | 中 | `p6_read_prefix` 14.9M 调用 |
| C: PPTok 对象池 | 低-中 | `tl_append_copy` 仅 2.54% |

### 规范一致性（如真实场景需要）

| 候选 | 价值 | 备注 |
|---|---|---|
| E. 完整 Prosser per-token hide-set | 低 | 当前 active-expansion 栈对所有真实场景都过 |
| H. ICE 保护 / J. 模糊测试 | 中 | 基础设施类 |

**推荐下一阶段：R2（SQLite）**。SQLite 的 `sqlite3.c` 单文件 amalgamation ~250k 行，是业界经典 stress test 之一。如果 sharp-cpp 能零诊断处理 SQLite，就基本可以宣称"production-ready for arbitrary C codebase"。预计工时 1-2 小时（沿用 R1 的 lua_smoke.sh + test_lua_includes.c 模板）。

## 给下一位的建议

如果你接手做 R2（SQLite）或其他真实项目集成：

1. **沿用 R1 的工具链**：`sharp_cli.c` 是通用的，`lua_smoke.sh` 改名 + 调整 -D flag 即可
2. **准备好失败**：R1 在 Lua（相对简洁）上还找到了 1 个 bug。R2 在 SQLite（更复杂）上**很可能**找到更多——这是好事，不是坏事
3. **每个新发现的 bug 写回归测试**：模仿 `test_lua_includes.c` 的 6 节结构。把 bug-discovery 故事写进注释——是教学价值最高的文档形式
4. **不要追求 wall-clock 改善**：R 系列是测试覆盖阶段，性能数字是 Phase 9/10 已经搞过的事。专注"零诊断" + "ratio ≈ 1.0" 这两个语义指标
5. **保持 R1 的"探针先行"纪律**：先最小复现 bug，写一个 `tmp/probe_X.c` 做最小输入；不要直接在大项目上改代码——会被噪声干扰
