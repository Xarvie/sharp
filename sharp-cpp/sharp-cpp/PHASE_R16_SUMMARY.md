# Phase R16 — `_Pragma` 修复 + corpus 大扩展 (151 文件 100%)

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓**
> **gcc-diff 累计 151/151 文件 token-identical (100%) ✓**

## 修复的 Bug

### Bug R16-1：`_Pragma("X")` 被完全丢弃

**症状**：glib、ffmpeg 等头中的 `GLIB_DEPRECATED_IN_*`、`G_GNUC_DEPRECATED_FOR` 宏展开后包含
`_Pragma("GCC diagnostic push/pop")` — sharp-cpp 把这些全部丢掉，导致 glib.h 的 token 流与 gcc -E 差了整个宏展开后的 `#pragma` 块。

**根本原因**：原来的处理注释写道 "successfully consumed _Pragma — strip from output"，正确消费了语法但没有任何输出。

**修法**（三处）：
1. **新增 `emit_pragma_text(st, raw_string_lit)`** — 公共 helper，de-stringize 字符串（去引号、反转义 `\"` / `\\`），确保前一行以 `\n` 结尾，再输出 `#pragma X\n`。
2. **直接出现在源文件中的 `_Pragma`**：把字符串内容在 `pptok_free(&s)` 之前保存到栈缓冲区，之后调用 `emit_pragma_text`。
3. **宏展开后的 `_Pragma`**：新增 `emit_expanded_list(st, tl, inv_loc)` 函数，在发射 TokList 时扫描 `_Pragma ( string )` 四元组，命中则调用 `emit_pragma_text`，未命中则 fall-through 到 `emit_post_expansion_tok`。全部三处 expanded token emit 循环改用此函数。

**影响**：glib.h（含 `GLIB_DEPRECATED_IN_*`）、ffmpeg 的 `AV_DEPRECATED` 等全部通过。

### Feature R16-2：darwin/win32 target 同步 C-suffix 宏

R15 只为 x86_64-linux-gnu 安装了 `__INT64_C(c)` 等，R16 补全另外两个 target：
- **aarch64-apple-darwin**（LP64）：与 linux 相同，`__INT64_C(c) = c ## L`
- **x86_64-pc-windows-msvc**（LLP64）：`__INT64_C(c) = c ## LL`（long long）

## 新增 R16 Corpus（共 74 个新文件）

| 组               | 文件数 | 代表性头文件                               |
|----------------|-------|------------------------------------------|
| 通用库 (native)   | 12    | sqlite3 / libpng / libjpeg / systemd / lz4 / zstd / pcre2 |
| Linux kernel uapi| 21    | linux/socket / linux/fs / linux/sched …  |
| 网络/系统 POSIX   | 15    | netinet/tcp / mqueue / semaphore / resolv … |
| crypto/tls       | 3+    | gnutls / gcrypt / libevent               |
| glib+dbus        | 6     | glib.h / ghash / glist / garray / dbus   |
| Boost            | 3     | boost/config / assert / version          |

## 验证矩阵

```
make test           610/610 ✓
make asan           610/610 ✓
make strict         610/610 ✓
self_smoke          20/20   零诊断 ✓

gcc-diff corpus:
  R13 stress        38/38   ✓
  R14 (curl/ssl/av) 36/36   ✓
  R15 (glibc/avutil)24/24   ✓
  R16 native        12/12   ✓
  R16 kernel uapi   21/21   ✓
  R16 POSIX extra   15/15   ✓
  R16 boost/misc     3/ 3   ✓
  R16 glib+dbus      6/ 6   ✓  (with -isystem flags)
  ─────────────────────────
  累计             155/155  100% ✓
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
| **gcc-diff corpus**         | **155**| **155 / 155**     |
| **Total**                   | **733** | **733 / 733 (100%)** |

## R17+ 候選

1. **musl libc** stress — 與 glibc 不同的 `__has_builtin` 用法
2. **`_Pragma` 的 `#pragma once` 互動** — 確保 `_Pragma("once")` 也觸發 guard（現在只輸出文字）
3. **Windows/MSVC 標頭** — `<windows.h>` 子集
4. **性能基準** — bench.c 上的 throughput/memory regression
