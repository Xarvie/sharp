# Phase R12 — gcc/clang `-E` 输出对比、差异修复、批量补全 GCC builtin

> Status: **610/610 baseline ✓ + 三方门槛 ✓ + 34/35 真实 C 文件与 `gcc -E` token 流逐字一致 (97.1%)**。
> sharp-cpp 内置宏从 ~95 个扩到 318 个（覆盖 gcc 401 个内置宏的 79%）。
> 累计真 bug 修复 33 个（R12 新增 6 个）；新增公开 API 2 个（`cpp_set_phase6_concat`、`cpp_set_trigraphs`）。

## 背景

R12 之前 sharp-cpp 已在 6 个真实场景目标上零诊断（Lua/SQLite/zlib/Redis/mcpp/self），但**没有逐字对比过 gcc/clang `-E` 输出**。R12 的目标：让 sharp-cpp 的 `-E` 输出在 token 层与 gcc 完全一致，使其可作为 gcc 的 drop-in 替换。

## 方法：探针先行 → 真 bug → 修 → 批量补全 → 回归

1. 写 `/home/claude/diff/tokenize.py`（粗 token 扫描，跳过 linemarker、归一空白）
2. 写 `/home/claude/diff/gcc_diff_smoke.sh`（批量对比 + 报告）
3. 从 `hello.c` 开始递增复杂度，每发现一个差异：最小化 reproducer → 实测 gcc → 修 → 回归
4. **批量补全策略**：到第 7 个 stress test 时改为系统化方案 —— 用 `gcc -E -dM -x c </dev/null` 取 gcc 全 401 个内置宏，与 sharp-cpp 已有的做差集，机器生成 233 个 `def(ctx, ...)` 行批量插入

## 修复的 6 个真 bug

### Bug R12-1：`__attribute__((...))` 被错误剥离

**症状**：`int foo(void) __attribute__((__nothrow__));` 经 sharp-cpp 后变成 `int foo(void);`，`<stddef.h>` 的 `max_align_t` 丢失 `__aligned__` 修饰。

**修法**：删 `directive.c` 35 行错误剥离代码。`__attribute__` 是编译器后端事，不是 PP 事。

### Bug R12-2：缺 GCC ABI / 版本内置宏

**症状**：sharp-cpp 输出 `__asm__("__USER_LABEL_PREFIX____isoc99_fscanf")`（垃圾汇编符号）；gcc 输出 `__asm__("" "__isoc99_fscanf")`。

**修法**：`cpp_targets.c` 加 `__USER_LABEL_PREFIX__`、`__VERSION__` 等 6 个核心 ABI 宏。

### Bug R12-3：phase 6 字符串拼接默认开启 ≠ gcc/clang 默认

**修法**：新公开 API `cpp_set_phase6_concat(ctx, on)`，默认 `false`；`build_result` 改为始终保证 `res.text` 与 `res._raw_text` 是独立堆块（避免上轮交接的 double-free）。

### Bug R12-4：宏体内 `__FILE__`/`__LINE__`/`__COUNTER__` 不被 rescan 展开

**症状**：`#define WHERE __FILE__, __LINE__\nuse(WHERE);` → sharp-cpp 输出 `use(__FILE__, __LINE__);`；gcc 输出 `use("foo.c", 4);`。glibc `<assert.h>` 的 `assert()` 宏受影响。

**修法**：新加 `emit_post_expansion_tok` helper，在 macro 展开后的 emit 循环里特殊处理；用**调用点 loc**（C99 §6.10.8.1）。

### Bug R12-5：trigraph 默认开启 ≠ gcc 默认

**修法**：新公开 API `cpp_set_trigraphs(ctx, on)`，默认 `false`；`RawBuf` 加 `bool trigraphs` 字段；`ph1_get` 检查这个 flag；reader 创建后从 ctx 同步。

### Bug R12-6 & R12-7：缺大量 GCC 内置宏

**症状**：每加一种 stdlib 头（`<stdatomic.h>` `<linux/types.h>` 等）就发现一个新的缺失宏 → token 流出现大段差异。

**修法（系统化）**：从 `gcc -E -dM -x c </dev/null` 取 gcc 全 401 个内置宏，与 sharp-cpp 已有的做差集，机器生成 223 行 `def(ctx, ...)` 批量插入到 `install_x86_64_linux_gnu`。覆盖：
- `__ATOMIC_*` (6 个) — C11 atomic memory order
- `__GCC_ATOMIC_*_LOCK_FREE` (10 个)
- `__GCC_HAVE_SYNC_COMPARE_AND_SWAP_*` (4 个)
- `__SIZEOF_INT128__`、`__SIZEOF_FLOAT80__`、`__SIZEOF_FLOAT128__`
- `__INT*_MAX__`/`__INT*_TYPE__`/`__INT*_WIDTH__`（INT8/16/32/64/MAX/PTR/FAST/LEAST 全套）
- `__UINT*_MAX__`/`__UINT*_TYPE__`/`__UINT*_WIDTH__`（UINT 全套）
- `__FLT*` `__DBL*` `__LDBL*` `__FLT128_*` `__FLT64X_*`（DECIMAL_DIG/DIG/MANT_DIG/MAX_EXP/MIN_EXP/HAS_INFINITY/HAS_QUIET_NAN/IS_IEC_60559）
- IEC 559：`__STDC_IEC_559__`、`__STDC_IEC_559_COMPLEX__`、`__GCC_IEC_559`、`__GCC_IEC_559_COMPLEX`
- `__SSE__`/`__SSE2__`/`__SSE_MATH__`/`__SSE2_MATH__`/`__MMX__`/`__FXSR__`
- `__INT_FAST*_TYPE__`、`__INT_LEAST*_TYPE__` 类型别名
- 已排除：函数式宏 `INT*_C(c)`（需要单独 `cpp_define_func` 基础设施），细化的 IEEE float `MIN/MAX/EPSILON/DENORM_MIN` 字面量（不影响 `#if` 求值）

**结果**：sharp-cpp 内置宏从 ~95 个扩到 318 个（覆盖 gcc 401 个的 79%）。

## 增强：GNU linemarker flag 1/2/3/4

旧版只发不带 flag 的 `# 1 "file"`。gcc/clang 在 system header 入栈用 `# 1 "file" 1 3 4`、出栈用 `# N "file" 2 3 4`。

**改动**：
- `find_include_ex` 加 `bool *out_found_in_sys` 出参
- `CppState` 加 `cur_file_is_sys` 字段
- 新加 `process_include_file` 在入栈处发 `# 1 "..." 1 3 4`
- `handle_include` 出栈处发 `# N "..." 2 3 4`
- `emit_linemarker_ex` 扩展为 3 个 flag 参数
- `handle_line` 解析 `#line` 时也接受第 3 个 flag

## 验证

### 三方门槛（必须维持）
| 门槛 | 通过 |
|------|------|
| `make test` | **610/610** ✓ |
| `make asan` (ASan/UBSan) | **610/610** ✓ |
| `make strict` (Werror Wpedantic Wshadow) | **610/610** ✓ |
| `self_smoke.sh` (sharp-cpp 处理自己 20 个 .c) | **20/20 零诊断** ✓ |

### gcc-diff smoke：35 个真实 C 文件 token-level 对比

| 套件 | 内容 | 通过 |
|------|------|------|
| 12 个 stdlib 代表性 case | hello + stdio/string/math/stdlib/assert/setjmp/time/stdarg | **12/12 ✓** |
| sharp-cpp 自己 9 个 .c | cpp/pptok/macro/expr/directive/cpp_targets/sharp_cli/probe/bench | **9/9 ✓** |
| 14 个 stress case | pthread/socket/mmap/atomic/complex/locale/glob/dirent/signal/stat/termios/iconv/stdint/bigexpr | **13/14 ✓** |
| **合计** | | **34/35 (97.1%)** |

唯一 fail：`stress11_signal`，因为 `__has_attribute(__indirect_return__)` —— sharp-cpp 现在保守返回 0；gcc 返回 1（识别真实 attribute）。修这个需要维护一个 gcc 全部 attribute 名单（~80 个），R13 候选。

### 与 clang 对比（baseline 一致性）

| 套件 | sharp-cpp vs clang | gcc vs clang | 备注 |
|------|---------------------|---------------|------|
| 12 stdlib cases | 3/12 一致 | 3/12 一致（**完全相同的 3 个**） | 失败原因都是 gcc/clang 内置宏（`__nothrow__`、`__leaf__`）的展开本质不同 |

**结论**：sharp-cpp 的 `-E` 输出对 clang 而言**等价于 gcc**。

## 已知遗留差异

### 1. `__has_attribute(...)` 保守返回 0

gcc 返回 1 if attribute 是 gcc 已知的；sharp-cpp 保守返回 0。glibc `<bits/indirect-return.h>` 用 `__glibc_has_attribute(__indirect_return__)` 选择 attribute → 不同分支。

**影响**：1/35 测试文件出现 token 差异（不影响下游编译，因为两个分支都生成有效 `__attribute__`）。

### 2. gcc 的 "intro" 序列

gcc `-E` 头部发 `# 0 "<built-in>"` / `# 0 "<command-line>"` / `stdc-predef.h` 隐式包含。sharp-cpp 不复刻 —— token 流不受影响。

### 3. mid-file linemarker（跳 `#if false` 后行号同步）

gcc 用 `# N "file" 3 4` 跳行号；sharp-cpp 用保留空行。token 流不受影响，sharp-cpp 输出**行数更多**但每个 token 的源行更精确。

### 4. 函数式 `__INT*_C(c)` 等宏未导入

`<stdint.h>` 的 `INT64_C(0x1234)` helper 当前不工作。需要新基础设施 `cpp_define_func`。

### 5. 浮点细化常量未导入

`__FLT_MIN__` `__FLT_MAX__` `__FLT_EPSILON__` `__FLT_DENORM_MIN__` 等浮点字面量值。这些只影响 `<float.h>` 之类的 header 输出，不影响 `#if` 求值（PP 不算浮点）。

## 文件改动

```
PHASE_R12_SUMMARY.md       (新)
cpp.h                      +cpp_set_phase6_concat, +cpp_set_trigraphs 声明
cpp_internal.h             +CppCtx::phase6_concat, +CppCtx::trigraphs,
                           +CppState::cur_file_is_sys
cpp.c                      +ctx->phase6_concat=false, +ctx->trigraphs=false 初始化
                           +setter 实体, build_result 改 phase6 可选 + 始终独立堆块
cpp_targets.c              +6 个 ABI 宏 (linux), +6 个 (darwin),
                           +223 行批量 gcc builtin (linux 全 install)
pptok.h                    +reader_set_trigraphs 声明
pptok.c                    +RawBuf::trigraphs, +CppReader::trigraphs,
                           +reader_set_trigraphs setter, ph1_get 条件判断
directive.c                −__attribute__ 剥离 (~35 行)
                           +emit_linemarker_ex 第三个 flag,
                           +find_include_ex out_found_in_sys 出参,
                           +process_include_file (新),
                           +emit_post_expansion_tok (新),
                           +macro_inv_loc 捕获,
                           ~handle_include / handle_line / process_file 等多处
test_phase1.c              ~test_prefix_string_concat 显式 set_phase6_concat
```

## 怎么复现

```bash
cd /home/claude/sharp/sharp-cpp && make test asan strict self_smoke
cd /home/claude/diff/cases    && /home/claude/diff/gcc_diff_smoke.sh *.c
cd /home/claude/diff          && /home/claude/diff/gcc_diff_smoke.sh stress*.c
cd /home/claude/sharp/sharp-cpp && /home/claude/diff/gcc_diff_smoke.sh -I. \
    cpp.c pptok.c macro.c expr.c directive.c cpp_targets.c sharp_cli.c probe.c bench.c
```

## 累计 R-series 状态

| Target                    | Files | Pass            |
|---------------------------|-------|-----------------|
| Lua                       | 35    | 35 / 35         |
| SQLite                    | 126   | 126 / 126       |
| zlib                      | 15    | 15 / 15         |
| mcpp testsuite (.c)       | 109   | 109 / 109       |
| Redis                     | 115   | 115 / 115       |
| Self                      | 20    | 20 / 20         |
| mcpp source               | 10    | 10 / 10         |
| mcpp native (.t)          | 148   | 148 / 148       |
| **gcc-diff smoke (R12)**  | **35**| **34 / 35 (97.1%) token-identical** |
| **Total**                 | **613** | **612 / 613 (99.84%)** |

## R13+ 候选

1. **维护 gcc attribute 名单**让 `__has_attribute` 准确，修最后 1/35 → 35/35
2. **darwin target 也批量补全** GCC builtin（当前只补了 linux）
3. **新增 `cpp_define_func` 基础设施**支持 `INT64_C(c) c ## L` 之类函数式 builtin
4. **可选：补完 IEEE 浮点字面量** `__FLT_MIN__` `__FLT_MAX__` 等（影响 `<float.h>`）
5. **R13 真实 corpus**：curl、openssl、ffmpeg headers 做更大规模 stress
