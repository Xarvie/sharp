# Phase R14 — curl/openssl/ffmpeg corpus + `||`/`&&` 优先级修复

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓**
> **新 R14 corpus 36/36 token-identical ✓ + R13 stress 38/38 ✓**

## 背景

R13 达成 35/35 gcc-diff 100% 后，R14 目标是扩大 corpus 到真实库头文件（curl / openssl / ffmpeg）并修复暴露出的新 bug。

## 修复的 2 个 Bug

### Bug R14-1（严重）：`||` 短路时 `&&` 链被截断 — 优先级求值错误

**症状**：`#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 6` 求值为 0（应为 1），导致 ffmpeg 的 `AV_GCC_VERSION_AT_LEAST` 宏始终走 else 分支，`av_const`/`av_pure`/`av_cold` 等全部展开为空而不是 `__attribute__((...))` 。

**根本原因**：`ec_skip_expr`（`expr.c`）在 depth=0 时以 `||` 和 `&&` 双双作为停止符。`eval_logor` 短路（v != 0）时调用 `ec_skip_expr` 跳过右侧操作数，但 `ec_skip_expr` 碰到 `&&` 就停，把 `&& 0` 留在 token 流里。后续 `eval_logor` 的 while 循环看到 `&&`（不是 `||`）就退出，整个表达式求值未消耗完，`&&` 留给上层处理，导致本应为 `(1 || ...) = 1` 的结果变成 `1 && 0 = 0`。

```
# 期望求值顺序（&&绑定更紧）:
1 || 0 && 0  →  1 || (0 && 0)  →  1 || 0  →  1

# 实际（R13 之前）:
ec_skip_expr 在 && 停下，留下 "&& 0" → eval_logor 退出
外层: (1 || 0) 返回 1, 但 "&&" 还在流里 → 上层取到 "&&" 意外短路
```

**修法**：新增 `ec_skip_logand` 函数（`expr.c`），在 `ec_skip_expr` 基础上继续消耗所有 `&& <operand>` 链，直到遇到 `||`、`,`、`)` 等为止。`eval_logor` 短路路径改用 `ec_skip_logand`，`eval_logand` 短路路径保持用 `ec_skip_expr`（行为不变，因为 `&&` 就是 logand 的分隔符）。

**影响范围**：所有含 `A || B && C` 形式的 `#if` 表达式。glibc / musl / ffmpeg / Linux 内核头中大量使用此模式做版本兼容检测。

### Bug R14-2（低）：整数限制宏第三个 target 块修复

darwin/aarch64 和 MSVC/win32 target 的 `__INT_MAX__` 等在 R13 中已修，确认三处 target 块全部一致。

## 新增 R14 Corpus

| 组       | 文件数 | 示例                                          |
|---------|-------|----------------------------------------------|
| curl    | 3     | curl.h / easy.h / multi.h                    |
| openssl | 21    | ssl.h / crypto.h / evp.h / ec.h / cms.h …   |
| ffmpeg  | 7     | avcodec.h / avformat.h / avfilter.h …        |
| syscall | 5     | epoll.h / inotify.h / timerfd.h / perf_event.h |
| **总计** | **36** | **36/36 token-identical** ✓                  |

## 验证矩阵

```
make test           610/610 ✓
make asan           610/610 ✓
make strict         610/610 ✓
self_smoke          20/20   零诊断 ✓
R13 stress          38/38   token-identical ✓
R14 corpus (新)     36/36   token-identical ✓
累计 gcc-diff       74/74   100% ✓
```

## 累计 R-series 状态

| Target                      | Files | Pass               |
|-----------------------------|-------|--------------------|
| Lua                         | 35    | 35 / 35            |
| SQLite                      | 126   | 126 / 126          |
| zlib                        | 15    | 15 / 15            |
| mcpp testsuite (.c)         | 109   | 109 / 109          |
| Redis                       | 115   | 115 / 115          |
| Self                        | 20    | 20 / 20            |
| mcpp source                 | 10    | 10 / 10            |
| mcpp native (.t)            | 148   | 148 / 148          |
| gcc-diff stress R13         | 38    | 38 / 38            |
| **gcc-diff corpus R14**     | **36**| **36 / 36**        |
| **Total**                   | **652** | **652 / 652 (100%)** |

## R15+ 候选

1. **`__has_builtin` 表**：约 200 个 GCC builtins，让 `__has_builtin(__builtin_expect)` 等返回 1
2. **`__INTMAX_C` / `__UINTMAX_C`**：函数式宏 `c ## L` / `c ## UL`，`<inttypes.h>` 使用
3. **更大 corpus**：Linux 内核全套 include、glibc 内部头、LLVM libc 头
4. **`__builtin_*` passthrough 模式**：让 `__builtin_expect(x,0)` 等直接透传到输出
