# Phase R13 — __has_attribute 属性表 + 整数限制宏 hex 修复

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓ + 35/35 gcc-diff token-identical (100%) ✓**

## 背景

R12 最后 1/35 失败源自两个 root cause（从对话重建过程中发现第三个）：

1. `__has_attribute` 始终返回 0 — glibc `<sys/cdefs.h>` 用 `__glibc_has_attribute` 包装需要它返回正确值
2. `defined(__has_attribute)` 被 `resolve_defined` 当 undefined 宏处理 → 返回 0 → glibc 走 fallback 分支输出多余代码
3. (新发现) `__INT_MAX__` 等整数限制宏用十进制 `2147483647`，而 gcc 用十六进制 `0x7fffffff` → `limits.h` 展开后 token 不一致

## 修复的 3 个 Bug

### Bug R13-1：整数限制宏表示格式不匹配 gcc

**症状**：`#include <limits.h>` 后 `INT_MAX` 展开为 `2147483647`，gcc 输出 `0x7fffffff`。

**原因**：`cpp_targets.c` 的 `__INT_MAX__`（以及 `__SCHAR_MAX__`、`__SHRT_MAX__`、`__LONG_MAX__`、`__INTMAX_MAX__`、`__UINTMAX_MAX__` 等）全部用十进制字面量。gcc 用十六进制。`limits.h` 在 `#undef INT_MAX` 后重新用 `__INT_MAX__` 展开，所以表示直接进入 token 流。

**修法**：三个 target 块（x86_64-linux-gnu、darwin/aarch64、MSVC/win32）全部改为十六进制，与 `gcc -E -dM -x c /dev/null` 输出逐字一致。

### Bug R13-2：`__has_attribute` 始终返回 0

**原因**：`expr.c` 的 `__has_attribute` 分支统一跳过参数、返回 `vsigned(0)`，没有查任何属性表。

**修法**：添加 80 个 GCC 13 属性的静态表（`gcc_attrs[]`），提取参数后去掉前后 `__` 前缀，在表中查找。命中返回 1，未命中返回 0。涵盖：
- 函数属性：`nothrow`/`leaf`/`pure`/`const`/`noreturn`/`format`/`nonnull`/`malloc`/`warn_unused_result` 等 ~65 个
- 变量/类型属性：`aligned`/`packed`/`section`/`visibility`/`may_alias`/`vector_size` 等 ~15 个

### Bug R13-3：`defined(__has_attribute)` 错误地返回 0

**原因**：`resolve_defined`（`directive.c:817`）只查 `macro_lookup`，`__has_attribute` 不是真正的宏所以返回 0。这导致 glibc 的 `#if defined __has_attribute` 进入 else 分支，将 `__glibc_has_attribute` 定义为始终返回 0 的宏——即使 `__has_attribute` 本体已经正常工作。

**修法**：在 `resolve_defined` 的标识符处理中加 `is_has_x` 检查，与 `#ifdef` 处理函数（directive.c:1052）逻辑完全对称。`__has_builtin/feature/extension/attribute/c_attribute/cpp_attribute/__has_include/__has_include_next` 全部视为隐式 defined。

## 验证矩阵

```
make test       610/610 ✓
make asan       610/610 ✓
make strict     610/610 ✓
self_smoke      20/20   零诊断 ✓
gcc-diff smoke  35/35   token-identical (100%) ✓
```

35 个测试覆盖：14 个 stress（pthread/socket/mmap/atomic/complex/locale/glob/dirent/signal/stat/termios/iconv/stdint/features）+ 12 个 stdlib 代表（assert/stdio/string/stdlib/unistd/errno/time/math/wchar/limits/inttypes/sched）+ 9 个 sharp-cpp 自身源文件。

## 关键洞察

**`__INT_MAX__` 是 hex 不是 decimal** — 这是 R12 漏掉的一个细节。gcc 所有整数类型限制宏都用十六进制，`limits.h` 通过 `#undef` + `#define INT_MAX __INT_MAX__` 让展开值直接进入 token 流。用十进制虽然语义等价，但 token-diff 会报不一致。

**`defined` 操作符在 `resolve_defined` 就被消耗** — 永远不会到达 `expr.c` 的 `__has_attribute` 分支。`#ifdef`/`#ifndef` 有单独的 handler，`#if defined(...)` 走 `resolve_defined` 这条路，两处需要分别处理。

## 累计 R-series 状态

| Target                      | Files | Pass              |
|-----------------------------|-------|-------------------|
| Lua                         | 35    | 35 / 35           |
| SQLite                      | 126   | 126 / 126         |
| zlib                        | 15    | 15 / 15           |
| mcpp testsuite (.c)         | 109   | 109 / 109         |
| Redis                       | 115   | 115 / 115         |
| Self                        | 20    | 20 / 20           |
| mcpp source                 | 10    | 10 / 10           |
| mcpp native (.t)            | 148   | 148 / 148         |
| **gcc-diff smoke (R13)**    | **35**| **35 / 35 (100%)** |
| **Total**                   | **613** | **613 / 613 (100%)** |

## R14+ 候选

1. **更大 corpus**：curl / openssl / ffmpeg headers 做更大规模 stress（预期发现新 builtins 缺失）
2. **`__has_builtin` 表**：类似 R13 的方式为 ~200 个 GCC builtins 建表，让 `__has_builtin(__builtin_FILE)` 等返回 1
3. **darwin target 完整验证**：在 macOS 上跑同等 stress 矩阵
4. **`__INTMAX_C` / `__UINTMAX_C` 函数式宏**：`c ## L` 形式，inttypes.h 中使用
