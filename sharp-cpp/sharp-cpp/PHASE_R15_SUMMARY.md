# Phase R15 — `__has_builtin` 表 (303 项) + `__INTMAX_C` 函数式宏 + 扩展 corpus

> Status: **610/610 baseline ✓ + ASAN ✓ + strict ✓ + self_smoke 20/20 零诊断 ✓**
> **R13 stress 38/38 ✓ + R14 corpus 36/36 ✓ + R15 corpus 24/24 ✓ — 累计 gcc-diff 98/98 (100%) ✓**

## 新增功能 / 修复

### Feature R15-1：`__has_builtin` 表（303 个 GCC 13 builtins）

**背景**：R13 实现了 `__has_attribute` 表；`__has_builtin` 一直保守地返回 0。glibc / openssl / ffmpeg 头中大量用 `#if __has_builtin(__builtin_expect)` 等做优化路径选择，返回 0 会走低效 fallback。

**方法**：对 GCC 13 的 355 个候选 `__builtin_*` 名称做 `#if __has_builtin(...)` 探针，得到 303 个真实支持的 builtins（含算术溢出检测、位操作、原子操作、数学函数、字符串函数、I/O 函数全套），写入 `expr.c` 静态表。

**影响**：`__builtin_expect`/`__builtin_unreachable`/`__builtin_clz`/`__builtin_bswap*`/`__builtin_popcount` 等全部正确返回 1。

### Feature R15-2：`__INTMAX_C` / `__UINTMAX_C` 等函数式宏

**背景**：`<inttypes.h>` 中 `INTMAX_C(n)` 展开为 `n ## L`，依赖内置宏 `__INTMAX_C(c)` = `c ## L`。sharp-cpp 此前没有定义这些函数式宏，导致 `INT64_C(42)` 不展开。

**修法**：
1. 新增公开 API `cpp_define_func(ctx, "NAME(params)", "body")` — 存储方式与 `cpp_define` 相同（`D:NAME(params)=body`），`apply_cmdline_macros` 再调用 `macro_define_object`，后者通过 `macro_parse_define` 正确识别函数式宏语法。
2. 在 `install_x86_64_linux_gnu` 末尾用 `cpp_define_func` 安装 10 个 C-suffix 宏：
   `__INT8/16/32/64_C`、`__INTMAX_C`、`__UINT8/16/32/64_C`、`__UINTMAX_C`（值与 `gcc -E -dM` 逐字一致）。

### 新增 R15 Corpus（24 个文件）

| 组           | 文件数 | 示例                                        |
|-------------|-------|---------------------------------------------|
| glibc 高级   | 13    | netdb / arpa/inet / regex / sys/resource … |
| openssl 扩展 | 5     | md5 / sha / des / rc4 / blowfish            |
| libavutil    | 6     | log / opt / dict / buffer / frame / samplefmt |

## 验证矩阵

```
make test           610/610 ✓
make asan           610/610 ✓
make strict         610/610 ✓
self_smoke          20/20   零诊断 ✓
R13 stress          38/38   token-identical ✓
R14 corpus          36/36   token-identical ✓
R15 corpus (新)     24/24   token-identical ✓
累计 gcc-diff       98/98   100% ✓
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
| gcc-diff corpus R14         | 36    | 36 / 36            |
| **gcc-diff corpus R15**     | **24**| **24 / 24**        |
| **Total**                   | **676** | **676 / 676 (100%)** |

## R16+ 候选

1. **Linux 内核头 stress**：安装 `linux-headers-generic`，对 `include/linux/*.h` 做大规模批量 diff
2. **`cpp_define_func` 扩展到 darwin/win32 target**：同步安装 C-suffix 宏到其他 target 块
3. **`__builtin_*` passthrough 模式**：在输出中保留 `__builtin_expect(x,0)` 而非展开为 x（供编译器后端使用）
4. **musl libc 头 stress**：与 musl 的 `#if __has_builtin(...)` 兼容性验证
